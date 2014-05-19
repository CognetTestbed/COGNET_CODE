
/*
 * The olsr.org Optimized Link-State Routing daemon(olsrd)
 * Copyright (c) 2004, Andreas Tonnesen(andreto@olsr.org)
 *                     includes code by Bruno Randolf
 *                     includes code by Andreas Lopatic
 *                     includes code by Sven-Ola Tuecke
 *                     includes code by Lorenz Schori
 *                     includes bugs by Markus Kittenberger
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * * Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in
 *   the documentation and/or other materials provided with the
 *   distribution.
 * * Neither the name of olsr.org, olsrd nor the names of its
 *   contributors may be used to endorse or promote products derived
 *   from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * Visit http://www.olsr.org for more information.
 *
 * If you find this software useful feel free to make a donation
 * to the project. For more information see the website or contact
 * the copyright holders.
 *
 */

/*
 * Dynamic linked library for the olsr.org olsr daemon
 */


#include <sys/types.h>
#include <sys/socket.h>
#if !defined _WIN32
#include <sys/select.h>
#endif /* !defined _WIN32 */
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <time.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <assert.h>

#include "ipcalc.h"
#include "olsr.h"
#include "builddata.h"
#include "olsr_types.h"
#include "neighbor_table.h"
#include "two_hop_neighbor_table.h"
#include "mpr_selector_set.h"
#include "tc_set.h"
#include "hna_set.h"
#include "mid_set.h"
#include "link_set.h"
#include "net_olsr.h"
#include "lq_plugin.h"
#include "common/autobuf.h"
#include "gateway.h"

#include "olsrd_txtinfo.h"
#include "olsrd_plugin.h"

#ifdef _WIN32
#define close(x) closesocket(x)
#endif /* _WIN32 */

static int ipc_socket;

/* IPC initialization function */
static int plugin_ipc_init(void);

static void send_info(unsigned int /*send_what*/, int /*socket*/);

static void ipc_action(int, void *, unsigned int);

static void ipc_print_neigh(struct autobuf *, bool);

static void ipc_print_link(struct autobuf *);

static void ipc_print_routes(struct autobuf *);

static void ipc_print_topology(struct autobuf *);

static void ipc_print_hna(struct autobuf *);

static void ipc_print_mid(struct autobuf *);

static void ipc_print_gateway(struct autobuf *);

static void ipc_print_config(struct autobuf *);

static void ipc_print_interface(struct autobuf *);

static void ipc_print_sgw(struct autobuf *);

#define TXT_IPC_BUFSIZE 256

#define SIW_NEIGH 0x0001
#define SIW_LINK 0x0002
#define SIW_ROUTE 0x0004
#define SIW_HNA 0x0008
#define SIW_MID 0x0010
#define SIW_TOPO 0x0020
#define SIW_GATEWAY 0x0040
#define SIW_INTERFACE 0x0080
#define SIW_CONFIG 0x0100
#define SIW_2HOP 0x0200
#define SIW_VERSION 0x0400
#define SIW_SGW 0x0800

/* ALL = neigh link route hna mid topo */
#define SIW_ALL 0x003F

#define MAX_CLIENTS 3

static char *outbuffer[MAX_CLIENTS];
static size_t outbuffer_size[MAX_CLIENTS];
static size_t outbuffer_written[MAX_CLIENTS];
static int outbuffer_socket[MAX_CLIENTS];
static int outbuffer_count = 0;

static struct timer_entry *writetimer_entry;

/**
 *Do initialization here
 *
 *This function is called by the my_init
 *function in uolsrd_plugin.c
 */
int
olsrd_plugin_init(void)
{
  /* Initial IPC value */
  ipc_socket = -1;

  plugin_ipc_init();
  return 1;
}

/**
 * destructor - called at unload
 */
void
olsr_plugin_exit(void)
{
  if (ipc_socket != -1)
    close(ipc_socket);
}

static int
plugin_ipc_init(void)
{
  union olsr_sockaddr sst;
  uint32_t yes = 1;
  socklen_t addrlen;

  /* Init ipc socket */
  if ((ipc_socket = socket(olsr_cnf->ip_version, SOCK_STREAM, 0)) == -1) {
#ifndef NODEBUG
    olsr_printf(1, "(TXTINFO) socket()=%s\n", strerror(errno));
#endif /* NODEBUG */
    return 0;
  } else {
    if (setsockopt(ipc_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&yes, sizeof(yes)) < 0) {
#ifndef NODEBUG
      olsr_printf(1, "(TXTINFO) setsockopt()=%s\n", strerror(errno));
#endif /* NODEBUG */
      return 0;
    }
#if (defined __FreeBSD__ || defined __FreeBSD_kernel__) && defined SO_NOSIGPIPE
    if (setsockopt(ipc_socket, SOL_SOCKET, SO_NOSIGPIPE, (char *)&yes, sizeof(yes)) < 0) {
      perror("SO_REUSEADDR failed");
      return 0;
    }
#endif /* (defined __FreeBSD__ || defined __FreeBSD_kernel__) && defined SO_NOSIGPIPE */
    /* Bind the socket */

    /* complete the socket structure */
    memset(&sst, 0, sizeof(sst));
    if (olsr_cnf->ip_version == AF_INET) {
      sst.in4.sin_family = AF_INET;
      addrlen = sizeof(struct sockaddr_in);
#ifdef SIN6_LEN
      sst.in4.sin_len = addrlen;
#endif /* SIN6_LEN */
      sst.in4.sin_addr.s_addr = txtinfo_listen_ip.v4.s_addr;
      sst.in4.sin_port = htons(ipc_port);
    } else {
      sst.in6.sin6_family = AF_INET6;
      addrlen = sizeof(struct sockaddr_in6);
#ifdef SIN6_LEN
      sst.in6.sin6_len = addrlen;
#endif /* SIN6_LEN */
      sst.in6.sin6_addr = txtinfo_listen_ip.v6;
      sst.in6.sin6_port = htons(ipc_port);
    }

    /* bind the socket to the port number */
    if (bind(ipc_socket, &sst.in, addrlen) == -1) {
#ifndef NODEBUG
      olsr_printf(1, "(TXTINFO) bind()=%s\n", strerror(errno));
#endif /* NODEBUG */
      return 0;
    }

    /* show that we are willing to listen */
    if (listen(ipc_socket, 1) == -1) {
#ifndef NODEBUG
      olsr_printf(1, "(TXTINFO) listen()=%s\n", strerror(errno));
#endif /* NODEBUG */
      return 0;
    }

    /* Register with olsrd */
    add_olsr_socket(ipc_socket, &ipc_action, NULL, NULL, SP_PR_READ);

#ifndef NODEBUG
    olsr_printf(2, "(TXTINFO) listening on port %d\n", ipc_port);
#endif /* NODEBUG */
  }
  return 1;
}

static void
ipc_action(int fd, void *data __attribute__ ((unused)), unsigned int flags __attribute__ ((unused)))
{
  union olsr_sockaddr pin;

  char addr[INET6_ADDRSTRLEN];
  fd_set rfds;
  struct timeval tv;
  unsigned int send_what = 0;
  int ipc_connection;

  socklen_t addrlen = sizeof(pin);

  if (outbuffer_count >= MAX_CLIENTS) {
    return;
  }

  if ((ipc_connection = accept(fd, &pin.in, &addrlen)) == -1) {
#ifndef NODEBUG
    olsr_printf(1, "(TXTINFO) accept()=%s\n", strerror(errno));
#endif /* NODEBUG */
    return;
  }

  tv.tv_sec = tv.tv_usec = 0;
  if (olsr_cnf->ip_version == AF_INET) {
    if (inet_ntop(olsr_cnf->ip_version, &pin.in4.sin_addr, addr, INET6_ADDRSTRLEN) == NULL)
      addr[0] = '\0';
    if (!ip4equal(&pin.in4.sin_addr, &txtinfo_accept_ip.v4) && txtinfo_accept_ip.v4.s_addr != INADDR_ANY) {
#ifdef TXTINFO_ALLOW_LOCALHOST
      if (pin.in4.sin_addr.s_addr != INADDR_LOOPBACK) {
#endif /* TXTINFO_ALLOW_LOCALHOST */
        olsr_printf(1, "(TXTINFO) From host(%s) not allowed!\n", addr);
        close(ipc_connection);
        return;
#ifdef TXTINFO_ALLOW_LOCALHOST
      }
#endif /* TXTINFO_ALLOW_LOCALHOST */
    }
  } else {
    if (inet_ntop(olsr_cnf->ip_version, &pin.in6.sin6_addr, addr, INET6_ADDRSTRLEN) == NULL)
      addr[0] = '\0';
    /* Use in6addr_any (::) in olsr.conf to allow anybody. */
    if (!ip6equal(&in6addr_any, &txtinfo_accept_ip.v6) && !ip6equal(&pin.in6.sin6_addr, &txtinfo_accept_ip.v6)) {
      olsr_printf(1, "(TXTINFO) From host(%s) not allowed!\n", addr);
      close(ipc_connection);
      return;
    }
  }

#ifndef NODEBUG
  olsr_printf(2, "(TXTINFO) Connect from %s\n", addr);
#endif /* NODEBUG */

  /* purge read buffer to prevent blocking on linux */
  FD_ZERO(&rfds);
  FD_SET((unsigned int)ipc_connection, &rfds);  /* Win32 needs the cast here */
  if (0 <= select(ipc_connection + 1, &rfds, NULL, NULL, &tv)) {
    char requ[128];
    ssize_t s = recv(ipc_connection, (void *)&requ, sizeof(requ), 0);   /* Win32 needs the cast here */
    if (0 < s) {
      requ[s] = 0;
      /* To print out neighbours only on the Freifunk Status
       * page the normal output is somewhat lengthy. The
       * header parsing is sufficient for standard wget.
       */
      if (0 != strstr(requ, "/neighbours")) send_what = SIW_NEIGH | SIW_LINK;
      else {
        /* print out every combinations of requested tabled
         * 3++ letter abbreviations are matched */
        if (0 != strstr(requ, "/all")) send_what = SIW_ALL;
        else { /*already included in /all*/
          if (0 != strstr(requ, "/nei")) send_what |= SIW_NEIGH;
          if (0 != strstr(requ, "/lin")) send_what |= SIW_LINK;
          if (0 != strstr(requ, "/rou")) send_what |= SIW_ROUTE;
          if (0 != strstr(requ, "/hna")) send_what |= SIW_HNA;
          if (0 != strstr(requ, "/mid")) send_what |= SIW_MID;
          if (0 != strstr(requ, "/top")) send_what |= SIW_TOPO;
        }
        if (0 != strstr(requ, "/gat")) send_what |= SIW_GATEWAY;
        if (0 != strstr(requ, "/con")) send_what |= SIW_CONFIG;
        if (0 != strstr(requ, "/int")) send_what |= SIW_INTERFACE;
        if (0 != strstr(requ, "/2ho")) send_what |= SIW_2HOP;
        if (0 != strstr(requ, "/ver")) send_what |= SIW_VERSION;
        if (0 != strstr(requ, "/sgw")) send_what |= SIW_SGW;
      }
    }
    if ( send_what == 0 ) send_what = SIW_ALL;
  }

  send_info(send_what, ipc_connection);
}

static void
ipc_print_neigh(struct autobuf *abuf, bool list_2hop)
{
  struct ipaddr_str buf1;
  struct neighbor_entry *neigh;
  struct neighbor_2_list_entry *list_2;
  int thop_cnt;

  abuf_puts(abuf, "Table: Neighbors\nIP address\tSYM\tMPR\tMPRS\tWill.");
  if (list_2hop) abuf_puts(abuf,"\n\t2hop interface adrress\n");
  else abuf_puts(abuf, "\t2 Hop Neighbors\n");

  /* Neighbors */
  OLSR_FOR_ALL_NBR_ENTRIES(neigh) {
    abuf_appendf(abuf, "%s\t%s\t%s\t%s\t%d\t", olsr_ip_to_string(&buf1, &neigh->neighbor_main_addr), (neigh->status == SYM) ? "YES" : "NO",
              neigh->is_mpr ? "YES" : "NO", olsr_lookup_mprs_set(&neigh->neighbor_main_addr) ? "YES" : "NO", neigh->willingness);
    thop_cnt = 0;

    for (list_2 = neigh->neighbor_2_list.next; list_2 != &neigh->neighbor_2_list; list_2 = list_2->next) {
      if (list_2hop) abuf_appendf(abuf, "\t%s\n", olsr_ip_to_string(&buf1, &list_2->neighbor_2->neighbor_2_addr));
      else thop_cnt++;
    }
    if (!list_2hop) {
      abuf_appendf(abuf, "%d\n", thop_cnt);
    }
  }
  OLSR_FOR_ALL_NBR_ENTRIES_END(neigh);
  abuf_puts(abuf, "\n");
}

static void
ipc_print_link(struct autobuf *abuf)
{
  struct ipaddr_str buf1, buf2;
  struct lqtextbuffer lqbuffer1, lqbuffer2;

  struct link_entry *my_link = NULL;

#ifdef ACTIVATE_VTIME_TXTINFO
  abuf_puts(abuf, "Table: Links\nLocal IP\tRemote IP\tVTime\tLQ\tNLQ\tCost\n");
#else /* ACTIVATE_VTIME_TXTINFO */
  abuf_puts(abuf, "Table: Links\nLocal IP\tRemote IP\tHyst.\tLQ\tNLQ\tCost\n");
#endif /* ACTIVATE_VTIME_TXTINFO */

  /* Link set */
  OLSR_FOR_ALL_LINK_ENTRIES(my_link) {
#ifdef ACTIVATE_VTIME_TXTINFO
    int diff = (unsigned int)(my_link->link_timer->timer_clock - now_times);

    abuf_appendf(abuf, "%s\t%s\t%d.%03d\t%s\t%s\t\n", olsr_ip_to_string(&buf1, &my_link->local_iface_addr),
              olsr_ip_to_string(&buf2, &my_link->neighbor_iface_addr),
              diff/1000, abs(diff%1000),
              get_link_entry_text(my_link, '\t', &lqbuffer1),
              get_linkcost_text(my_link->linkcost, false, &lqbuffer2));
#else /* ACTIVATE_VTIME_TXTINFO */
    abuf_appendf(abuf, "%s\t%s\t0.00\t%s\t%s\t\n", olsr_ip_to_string(&buf1, &my_link->local_iface_addr),
              olsr_ip_to_string(&buf2, &my_link->neighbor_iface_addr),
              get_link_entry_text(my_link, '\t', &lqbuffer1),
              get_linkcost_text(my_link->linkcost, false, &lqbuffer2));
#endif /* ACTIVATE_VTIME_TXTINFO */
  } OLSR_FOR_ALL_LINK_ENTRIES_END(my_link);

  abuf_puts(abuf, "\n");
}

static void
ipc_print_routes(struct autobuf *abuf)
{
  struct ipaddr_str buf1, buf2;
  struct rt_entry *rt;
  struct lqtextbuffer lqbuffer;

  abuf_puts(abuf, "Table: Routes\nDestination\tGateway IP\tMetric\tETX\tInterface\n");

  /* Walk the route table */
  OLSR_FOR_ALL_RT_ENTRIES(rt) {
    abuf_appendf(abuf, "%s/%d\t%s\t%d\t%s\t%s\t\n", olsr_ip_to_string(&buf1, &rt->rt_dst.prefix), rt->rt_dst.prefix_len,
              olsr_ip_to_string(&buf2, &rt->rt_best->rtp_nexthop.gateway), rt->rt_best->rtp_metric.hops,
              get_linkcost_text(rt->rt_best->rtp_metric.cost, true, &lqbuffer),
              if_ifwithindex_name(rt->rt_best->rtp_nexthop.iif_index));
  } OLSR_FOR_ALL_RT_ENTRIES_END(rt);

  abuf_puts(abuf, "\n");

}

static void
ipc_print_topology(struct autobuf *abuf)
{
  struct tc_entry *tc;

#ifdef ACTIVATE_VTIME_TXTINFO
  abuf_puts(abuf, "Table: Topology\nDest. IP\tLast hop IP\tLQ\tNLQ\tCost\tVTime\n");
#else /* ACTIVATE_VTIME_TXTINFO */
  abuf_puts(abuf, "Table: Topology\nDest. IP\tLast hop IP\tLQ\tNLQ\tCost\n");
#endif /* ACTIVATE_VTIME_TXTINFO */

  /* Topology */
  OLSR_FOR_ALL_TC_ENTRIES(tc) {
    struct tc_edge_entry *tc_edge;
    OLSR_FOR_ALL_TC_EDGE_ENTRIES(tc, tc_edge) {
      if (tc_edge->edge_inv) {
        struct ipaddr_str dstbuf, addrbuf;
        struct lqtextbuffer lqbuffer1, lqbuffer2;
#ifdef ACTIVATE_VTIME_TXTINFO
        uint32_t vt = tc->validity_timer != NULL ? (tc->validity_timer->timer_clock - now_times) : 0;
        int diff = (int)(vt);
        abuf_appendf(abuf, "%s\t%s\t%s\t%s\t%d.%03d\n", olsr_ip_to_string(&dstbuf, &tc_edge->T_dest_addr),
            olsr_ip_to_string(&addrbuf, &tc->addr),
            get_tc_edge_entry_text(tc_edge, '\t', &lqbuffer1),
            get_linkcost_text(tc_edge->cost, false, &lqbuffer2),
            diff/1000, diff%1000);
#else /* ACTIVATE_VTIME_TXTINFO */
        abuf_appendf(abuf, "%s\t%s\t%s\t%s\n", olsr_ip_to_string(&dstbuf, &tc_edge->T_dest_addr), olsr_ip_to_string(&addrbuf, &tc->addr),
                  get_tc_edge_entry_text(tc_edge, '\t', &lqbuffer1), get_linkcost_text(tc_edge->cost, false, &lqbuffer2));
#endif /* ACTIVATE_VTIME_TXTINFO */
      }
    } OLSR_FOR_ALL_TC_EDGE_ENTRIES_END(tc, tc_edge);
  } OLSR_FOR_ALL_TC_ENTRIES_END(tc);

  abuf_puts(abuf, "\n");
}

static void
ipc_print_hna(struct autobuf *abuf)
{
  struct ip_prefix_list *hna;
  struct hna_entry *tmp_hna;
  struct hna_net *tmp_net;
  struct ipaddr_str buf, mainaddrbuf;

#ifdef ACTIVATE_VTIME_TXTINFO
  abuf_puts(abuf, "Table: HNA\nDestination\tGateway\tVTime\n");
#else /* ACTIVATE_VTIME_TXTINFO */
  abuf_puts(abuf, "Table: HNA\nDestination\tGateway\n");
#endif /* ACTIVATE_VTIME_TXTINFO */

  /* Announced HNA entries */
  if (olsr_cnf->ip_version == AF_INET) {
    for (hna = olsr_cnf->hna_entries; hna != NULL; hna = hna->next) {
      abuf_appendf(abuf, "%s/%d\t%s\n", olsr_ip_to_string(&buf, &hna->net.prefix), hna->net.prefix_len,
                olsr_ip_to_string(&mainaddrbuf, &olsr_cnf->main_addr));
    }
  } else {
    for (hna = olsr_cnf->hna_entries; hna != NULL; hna = hna->next) {
      abuf_appendf(abuf, "%s/%d\t%s\n", olsr_ip_to_string(&buf, &hna->net.prefix), hna->net.prefix_len,
                olsr_ip_to_string(&mainaddrbuf, &olsr_cnf->main_addr));
    }
  }

  /* HNA entries */
  OLSR_FOR_ALL_HNA_ENTRIES(tmp_hna) {

    /* Check all networks */
    for (tmp_net = tmp_hna->networks.next; tmp_net != &tmp_hna->networks; tmp_net = tmp_net->next) {
#ifdef ACTIVATE_VTIME_TXTINFO
      uint32_t vt = tmp_net->hna_net_timer != NULL ? (tmp_net->hna_net_timer->timer_clock - now_times) : 0;
      int diff = (int)(vt);
      abuf_appendf(abuf, "%s/%d\t%s\t\%d.%03d\n", olsr_ip_to_string(&buf, &tmp_net->hna_prefix.prefix),
          tmp_net->hna_prefix.prefix_len, olsr_ip_to_string(&mainaddrbuf, &tmp_hna->A_gateway_addr),
          diff/1000, abs(diff%1000));
#else /* ACTIVATE_VTIME_TXTINFO */
      abuf_appendf(abuf, "%s/%d\t%s\n", olsr_ip_to_string(&buf, &tmp_net->hna_prefix.prefix),
          tmp_net->hna_prefix.prefix_len, olsr_ip_to_string(&mainaddrbuf, &tmp_hna->A_gateway_addr));
#endif /* ACTIVATE_VTIME_TXTINFO */
    }
  }
  OLSR_FOR_ALL_HNA_ENTRIES_END(tmp_hna);

  abuf_puts(abuf, "\n");
}


#ifdef __linux__
/**
 * Construct the sgw table for a given ip version
 *
 * @param abuf the string buffer
 * @param ipv6 true for IPv6, false for IPv4
 * @param fmtv the format for printing
 */
static void sgw_ipvx(struct autobuf *abuf, bool ipv6, const char * fmth, const char * fmtv) {
  struct gateway_entry * current_gw;
  struct gw_list * list;
  struct gw_container_entry * gw;

  abuf_appendf(abuf, "Table: Smart Gateway IPv%s\n", ipv6 ? "6" : "4");

  list = ipv6 ? &gw_list_ipv6 : &gw_list_ipv4;
  if (list->count) {
    char current[2] = { 0, 0 };
    char originator[INET6_ADDRSTRLEN];
    char prefix[(INET6_ADDRSTRLEN * 2) + 1];
    uint32_t uplink = 0;
    uint32_t downlink = 0;
    char sipv4[2] = { 0, 0 };
    char sipv4nat[2] = { 0, 0 };
    char sipv6[2] = { 0, 0 };
    char if_name[IF_NAMESIZE];
    char destination[INET6_ADDRSTRLEN];
    long long unsigned int cost = 0;
    memset(originator, 0, sizeof(originator));
    memset(prefix, 0, sizeof(prefix));
    memset(if_name, 0, sizeof(if_name));
    memset(destination, 0, sizeof(destination));

    abuf_appendf(abuf, fmth, " ", "Originator", "Prefix", "Uplink", "Downlink", "IPv4", "IPv4-NAT", "IPv6", "Tunnel-Name", "Destination", "Cost");

    current_gw = olsr_get_inet_gateway(false);
    OLSR_FOR_ALL_GWS(&list->head, gw) {
      if (gw) {
        current[0] = (current_gw && (gw->gw == current_gw)) ? '*' : ' ';

        if (gw->gw) {
          inet_ntop(ipv6 ? AF_INET6 : AF_INET, &gw->gw->originator, originator, sizeof(originator));
          strncpy(prefix, olsr_ip_prefix_to_string(&gw->gw->external_prefix), sizeof(prefix));
          uplink = gw->gw->uplink;
          downlink = gw->gw->downlink;
          sipv4[0] = gw->gw->ipv4 ? 'Y' : 'N';
          sipv4nat[0] = gw->gw->ipv4nat ? 'Y' : 'N';
          sipv6[0] = gw->gw->ipv6 ? 'Y' : 'N';
        }
        if (gw->tunnel) {
          strncpy(if_name, gw->tunnel->if_name, sizeof(if_name));
          inet_ntop(ipv6 ? AF_INET6 : AF_INET, &gw->tunnel->target, destination, sizeof(destination));
        }
        if (gw->gw) {
          cost = (long long unsigned int)gw->path_cost;
        }
        abuf_appendf(abuf, fmtv, current, originator, prefix, uplink, downlink, sipv4, sipv4nat, sipv6, if_name, destination, cost);
      }
    } OLSR_FOR_ALL_GWS_END(gw);
  }
}
#endif /* __linux__ */

static void
ipc_print_sgw(struct autobuf *abuf)
{
#ifndef __linux__
  abuf_puts(abuf, "Gateway mode is only supported in linux\n");
#else

  static const char * fmth4 = "%s%-16s %-33s %-8s %-8s %-4s %-8s %-4s %-16s %-16s %s\n";
  static const char * fmtv4 = "%s%-16s %-33s %-8u %-8u %-4s %-8s %-4s %-16s %-16s %llu\n";
  static const char * fmth6 = "%s%-46s %-93s %-8s %-8s %-4s %-8s %-4s %-16s %-46s %s\n";
  static const char * fmtv6 = "%s%-46s %-93s %-8u %-8u %-4s %-8s %-4s %-16s %-46s %llu\n";

  sgw_ipvx(abuf, false, fmth4, fmtv4);
  abuf_puts(abuf, "\n");
  sgw_ipvx(abuf, true, fmth6, fmtv6);
  abuf_puts(abuf, "\n");
#endif /* __linux__ */
}

static void
ipc_print_mid(struct autobuf *abuf)
{
  int idx;
  unsigned short is_first;
  struct mid_entry *entry;
  struct mid_address *alias;
#ifdef ACTIVATE_VTIME_TXTINFO
  abuf_puts(abuf, "Table: MID\nIP address\tAlias\tVTime\n");
#else /* ACTIVATE_VTIME_TXTINFO */
  abuf_puts(abuf, "Table: MID\nIP address\tAliases\n");
#endif /* ACTIVATE_VTIME_TXTINFO */

  /* MID */
  for (idx = 0; idx < HASHSIZE; idx++) {
    entry = mid_set[idx].next;

    while (entry != &mid_set[idx]) {
#ifdef ACTIVATE_VTIME_TXTINFO
      struct ipaddr_str buf, buf2;
#else /* ACTIVATE_VTIME_TXTINFO */
      struct ipaddr_str buf;
      abuf_puts(abuf, olsr_ip_to_string(&buf, &entry->main_addr));
#endif /* ACTIVATE_VTIME_TXTINFO */
      alias = entry->aliases;
      is_first = 1;

      while (alias) {
#ifdef ACTIVATE_VTIME_TXTINFO
        uint32_t vt = alias->vtime - now_times;
        int diff = (int)(vt);

        abuf_appendf(abuf, "%s\t%s\t%d.%03d\n", 
                     olsr_ip_to_string(&buf, &entry->main_addr), 
                     olsr_ip_to_string(&buf2, &alias->alias),
                     diff/1000, abs(diff%1000));
#else /* ACTIVATE_VTIME_TXTINFO */
        abuf_appendf(abuf, "%s%s", (is_first ? "\t" : ";"), olsr_ip_to_string(&buf, &alias->alias));
#endif /* ACTIVATE_VTIME_TXTINFO */
        alias = alias->next_alias;
        is_first = 0;
      }
      entry = entry->next;
#ifndef ACTIVATE_VTIME_TXTINFO
      abuf_puts(abuf,"\n");
#endif /* ACTIVATE_VTIME_TXTINFO */
    }
  }
  abuf_puts(abuf, "\n");
}

static void
ipc_print_gateway(struct autobuf *abuf)
{
#ifndef __linux__
  abuf_puts(abuf, "Gateway mode is only supported in linux\n");
#else /* __linux__ */
  static const char IPV4[] = "ipv4";
  static const char IPV4_NAT[] = "ipv4(n)";
  static const char IPV6[] = "ipv6";
  static const char NONE[] = "-";

  struct ipaddr_str buf;
  struct gateway_entry *gw;
  struct lqtextbuffer lqbuf;

  // Status IP ETX Hopcount Uplink-Speed Downlink-Speed ipv4/ipv4-nat/- ipv6/- ipv6-prefix/-
  abuf_puts(abuf, "Table: Gateways\nStatus\tGateway IP\tETX\tHopcnt\tUplink\tDownlnk\tIPv4\tIPv6\tPrefix\n");
  OLSR_FOR_ALL_GATEWAY_ENTRIES(gw) {
    char v4 = '-', v6 = '-';
    const char *v4type = NONE, *v6type = NONE;
    struct tc_entry *tc;

    if ((tc = olsr_lookup_tc_entry(&gw->originator)) == NULL) {
      continue;
    }

    if (gw == olsr_get_inet_gateway(false)) {
      v4 = 's';
    }
    else if (gw->ipv4 && (olsr_cnf->ip_version == AF_INET || olsr_cnf->use_niit)
        && (olsr_cnf->smart_gw_allow_nat || !gw->ipv4nat)) {
      v4 = 'u';
    }

    if (gw == olsr_get_inet_gateway(true)) {
      v6 = 's';
    }
    else if (gw->ipv6 && olsr_cnf->ip_version == AF_INET6) {
      v6 = 'u';
    }

    if (gw->ipv4) {
      v4type = gw->ipv4nat ? IPV4_NAT : IPV4;
    }
    if (gw->ipv6) {
      v6type = IPV6;
    }

    abuf_appendf(abuf, "%c%c\t%s\t%s\t%d\t%u\t%u\t%s\t%s\t%s\n",
        v4, v6, olsr_ip_to_string(&buf, &gw->originator),
        get_linkcost_text(tc->path_cost, true, &lqbuf), tc->hops,
        gw->uplink, gw->downlink, v4type, v6type,
        gw->external_prefix.prefix_len == 0 ? NONE : olsr_ip_prefix_to_string(&gw->external_prefix));
  } OLSR_FOR_ALL_GATEWAY_ENTRIES_END(gw)
#endif /* __linux__ */
}

static void
ipc_print_config(struct autobuf *abuf)
{
  olsrd_write_cnf_autobuf(abuf, olsr_cnf);
}

static void
ipc_print_version(struct autobuf *abuf)
{
abuf_appendf(abuf, "Version: %s (built on %s on %s)\n", olsrd_version, build_date, build_host);
}
static void
ipc_print_interface(struct autobuf *abuf)
{
  const struct olsr_if *ifs;
  abuf_puts(abuf, "Table: Interfaces\nName\tState\tMTU\tWLAN\tSrc-Adress\tMask\tDst-Adress\n");
  for (ifs = olsr_cnf->interfaces; ifs != NULL; ifs = ifs->next) {
    const struct interface *const rifs = ifs->interf;
    abuf_appendf(abuf, "%s\t", ifs->name);
    if (!rifs) {
      abuf_puts(abuf, "DOWN\n");
      continue;
    }
    abuf_appendf(abuf, "UP\t%d\t%s\t",
               rifs->int_mtu, rifs->is_wireless ? "Yes" : "No");
 
    if (olsr_cnf->ip_version == AF_INET) {
      struct ipaddr_str addrbuf, maskbuf, bcastbuf;
      abuf_appendf(abuf, "%s\t%s\t%s\n",
                 ip4_to_string(&addrbuf, rifs->int_addr.sin_addr), ip4_to_string(&maskbuf, rifs->int_netmask.sin_addr),
                 ip4_to_string(&bcastbuf, rifs->int_broadaddr.sin_addr));
    } else {
       struct ipaddr_str addrbuf, maskbuf;
      abuf_appendf(abuf, "%s\t\t%s\n",
                 ip6_to_string(&addrbuf, &rifs->int6_addr.sin6_addr), ip6_to_string(&maskbuf, &rifs->int6_multaddr.sin6_addr));
    }
  }
  abuf_puts(abuf, "\n");
}


static void
txtinfo_write_data(void *foo __attribute__ ((unused))) {
  fd_set set;
  int result, i, j, max;
  struct timeval tv;

  FD_ZERO(&set);
  max = 0;
  for (i=0; i<outbuffer_count; i++) {
    /* And we cast here since we get a warning on Win32 */
    FD_SET((unsigned int)(outbuffer_socket[i]), &set);

    if (outbuffer_socket[i] > max) {
      max = outbuffer_socket[i];
    }
  }

  tv.tv_sec = 0;
  tv.tv_usec = 0;

  result = select(max + 1, NULL, &set, NULL, &tv);
  if (result <= 0) {
    return;
  }

  for (i=0; i<outbuffer_count; i++) {
    if (FD_ISSET(outbuffer_socket[i], &set)) {
      result = send(outbuffer_socket[i], outbuffer[i] + outbuffer_written[i], outbuffer_size[i] - outbuffer_written[i], 0);
      if (result > 0) {
        outbuffer_written[i] += result;
      }

      if (result <= 0 || outbuffer_written[i] == outbuffer_size[i]) {
        /* close this socket and cleanup*/
        close(outbuffer_socket[i]);
        free (outbuffer[i]);

        for (j=i+1; j<outbuffer_count; j++) {
          outbuffer[j-1] = outbuffer[j];
          outbuffer_size[j-1] = outbuffer_size[j];
          outbuffer_socket[j-1] = outbuffer_socket[j];
          outbuffer_written[j-1] = outbuffer_written[j];
        }
        outbuffer_count--;
      }
    }
  }
  if (outbuffer_count == 0) {
    olsr_stop_timer(writetimer_entry);
  }
}

static void
send_info(unsigned int send_what, int the_socket)
{
  struct autobuf abuf;

  abuf_init(&abuf, 4096);

  /* Print minimal http header */
  abuf_puts(&abuf, "HTTP/1.0 200 OK\n");
  abuf_puts(&abuf, "Content-type: text/plain\n\n");

  /* Print tables to IPC socket */

  /* links */
  if ((send_what & SIW_LINK) == SIW_LINK) ipc_print_link(&abuf);
  /* neighbours */
  if ((send_what & SIW_NEIGH) == SIW_NEIGH) ipc_print_neigh(&abuf,false);
  /* topology */
  if ((send_what & SIW_TOPO) == SIW_TOPO) ipc_print_topology(&abuf);
  /* hna */
  if ((send_what & SIW_HNA) == SIW_HNA) ipc_print_hna(&abuf);
  /* sgw */
  if ((send_what & SIW_SGW) == SIW_SGW) ipc_print_sgw(&abuf);
  /* mid */
  if ((send_what & SIW_MID) == SIW_MID) ipc_print_mid(&abuf);
  /* routes */
  if ((send_what & SIW_ROUTE) == SIW_ROUTE) ipc_print_routes(&abuf);
  /* gateways */
  if ((send_what & SIW_GATEWAY) == SIW_GATEWAY) ipc_print_gateway(&abuf);
  /* config */
  if ((send_what & SIW_CONFIG) == SIW_CONFIG) ipc_print_config(&abuf);
  /* interface */
  if ((send_what & SIW_INTERFACE) == SIW_INTERFACE) ipc_print_interface(&abuf);
  /* 2hop neighbour list */
  if ((send_what & SIW_2HOP) == SIW_2HOP) ipc_print_neigh(&abuf,true);
  /* version */
  if ((send_what & SIW_VERSION) == SIW_VERSION) ipc_print_version(&abuf);

  assert(outbuffer_count < MAX_CLIENTS);

  outbuffer[outbuffer_count] = olsr_malloc(abuf.len, "txt output buffer");
  outbuffer_size[outbuffer_count] = abuf.len;
  outbuffer_written[outbuffer_count] = 0;
  outbuffer_socket[outbuffer_count] = the_socket;

  memcpy(outbuffer[outbuffer_count], abuf.buf, abuf.len);
  outbuffer_count++;

  if (outbuffer_count == 1) {
    writetimer_entry = olsr_start_timer(100, 0, OLSR_TIMER_PERIODIC, &txtinfo_write_data, NULL, 0);
  }

  abuf_free(&abuf);
}

/*
 * Local Variables:
 * mode: c
 * style: linux
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 */
