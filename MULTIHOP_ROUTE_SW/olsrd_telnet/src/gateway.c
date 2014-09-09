/*
 * gateway.c
 *
 *  Created on: 05.01.2010
 *      Author: henning
 */

#ifdef __linux__

#include "common/avl.h"
#include "defs.h"
#include "ipcalc.h"
#include "olsr.h"
#include "olsr_cfg.h"
#include "olsr_cookie.h"
#include "scheduler.h"
#include "kernel_routes.h"
#include "kernel_tunnel.h"
#include "net_os.h"
#include "duplicate_set.h"
#include "log.h"
#include "gateway_default_handler.h"
#include "gateway_list.h"
#include "gateway.h"

#include <assert.h>
#include <net/if.h>

/*
 * Defines for the multi-gateway script
 */

#define SCRIPT_MODE_GENERIC   "generic"
#define SCRIPT_MODE_OLSRIF    "olsrif"
#define SCRIPT_MODE_SGWSRVTUN "sgwsrvtun"
#define SCRIPT_MODE_EGRESSIF  "egressif"
#define SCRIPT_MODE_SGWTUN    "sgwtun"

/** structure that holds an interface name, mark and a pointer to the gateway that uses it */
struct interfaceName {
  char name[IFNAMSIZ]; /**< interface name */
  uint8_t mark; /**< marking */
  struct gateway_entry *gw; /**< gateway that uses this interface name */
};

/** the gateway tree */
struct avl_tree gateway_tree;

/** gateway cookie */
static struct olsr_cookie_info *gateway_entry_mem_cookie = NULL;

/** gateway container cookie */
static struct olsr_cookie_info *gw_container_entry_mem_cookie = NULL;

/** the gateway netmask for the HNA */
static uint8_t smart_gateway_netmask[sizeof(union olsr_ip_addr)];

/** the gateway handler/plugin */
static struct olsr_gw_handler *gw_handler;

/** the IPv4 gateway list */
struct gw_list gw_list_ipv4;

/** the IPv6 gateway list */
struct gw_list gw_list_ipv6;

/** the current IPv4 gateway */
static struct gw_container_entry *current_ipv4_gw;

/** the current IPv6 gateway */
static struct gw_container_entry *current_ipv6_gw;

/** interface names for smart gateway egress interfaces */
struct interfaceName * sgwEgressInterfaceNames;

/** interface names for smart gateway tunnel interfaces, IPv4 */
struct interfaceName * sgwTunnel4InterfaceNames;

/** interface names for smart gateway tunnel interfaces, IPv6 */
struct interfaceName * sgwTunnel6InterfaceNames;

/** the timer for proactive takedown */
static struct timer_entry *gw_takedown_timer;

/*
 * Forward Declarations
 */

static void olsr_delete_gateway_tree_entry(struct gateway_entry * gw, uint8_t prefixlen, bool immediate);

/*
 * Helper Functions
 */

/**
 * @return the gateway 'server' tunnel name to use
 */
static inline const char * server_tunnel_name(void) {
  return (olsr_cnf->ip_version == AF_INET ? TUNNEL_ENDPOINT_IF : TUNNEL_ENDPOINT_IF6);
}

/**
 * Convert the netmask of the HNA (in the form of an IP address) to a HNA
 * pointer.
 *
 * @param mask the netmask of the HNA (in the form of an IP address)
 * @param prefixlen the prefix length
 * @return a pointer to the HNA
 */
static inline uint8_t * hna_mask_to_hna_pointer(union olsr_ip_addr *mask, int prefixlen) {
  return (((uint8_t *)mask) + ((prefixlen+7)/8));
}

/**
 * @return true if multi-gateway mode is enabled
 */
static inline bool multi_gateway_mode(void) {
  return (olsr_cnf->smart_gw_use_count > 1);
}

/**
 * Convert an encoded 1 byte transport value (5 bits mantissa, 3 bits exponent)
 * to an uplink/downlink speed value
 *
 * @param value the encoded 1 byte transport value
 * @return the uplink/downlink speed value (in kbit/s)
 */
static uint32_t deserialize_gw_speed(uint8_t value) {
  uint32_t speed;
  uint32_t exp;

  if (!value) {
    /* 0 and 1 alias onto 0 during serialisation. We take 0 here to mean 0 and
     * not 1 (since a bandwidth of 1 is no bandwidth at all really) */
    return 0;
  }

  speed = (value >> 3) + 1;
  exp = value & 7;

  while (exp-- > 0) {
    speed *= 10;
  }
  return speed;
}

/**
 * Convert an uplink/downlink speed value into an encoded 1 byte transport
 * value (5 bits mantissa, 3 bits exponent)
 *
 * @param speed the uplink/downlink speed value (in kbit/s)
 * @return value the encoded 1 byte transport value
 */
static uint8_t serialize_gw_speed(uint32_t speed) {
  uint8_t exp = 0;

  if (speed == 0) {
    return 0;
  }

  if (speed > 320000000) {
    return 0xff;
  }

  while ((speed > 32 || (speed % 10) == 0) && exp < 7) {
    speed /= 10;
    exp++;
  }
  return ((speed - 1) << 3) | exp;
}

/**
 * Find an interfaceName struct corresponding to a certain gateway
 * (when gw != NULL) or to an empty interfaceName struct (when gw == NULL).
 *
 * @param gw the gateway to find (when not NULL), or the empty struct to find (when NULL)
 * @return a pointer to the struct, or NULL when not found
 */
static struct interfaceName * find_interfaceName(struct gateway_entry *gw) {
  struct interfaceName * sgwTunnelInterfaceNames;
  uint8_t i = 0;

  if (!multi_gateway_mode()) {
    return NULL;
  }

  assert(sgwTunnel4InterfaceNames);
  assert(sgwTunnel6InterfaceNames);

  sgwTunnelInterfaceNames = (olsr_cnf->ip_version == AF_INET) ? sgwTunnel4InterfaceNames : sgwTunnel6InterfaceNames;
  while (i < olsr_cnf->smart_gw_use_count) {
    struct interfaceName * ifn = &sgwTunnelInterfaceNames[i];
    if (ifn->gw == gw) {
      return ifn;
    }
    i++;
  }

  return NULL;
}

/**
 * Get an unused olsr ipip tunnel name for a certain gateway and store it in name.
 *
 * @param gw pointer to the gateway
 * @param name pointer to output buffer (length IFNAMSIZ)
 * @param interfaceName a pointer to the location where to store a pointer to the interfaceName struct
 */
static void get_unused_iptunnel_name(struct gateway_entry *gw, char * name, struct interfaceName ** interfaceName) {
  static uint32_t counter = 0;

  assert(gw);
  assert(name);
  assert(interfaceName);

  memset(name, 0, IFNAMSIZ);

  if (multi_gateway_mode()) {
    struct interfaceName * ifn = find_interfaceName(NULL);

    if (ifn) {
      strncpy(&name[0], &ifn->name[0], sizeof(ifn->name));
      *interfaceName = ifn;
      ifn->gw = gw;
      return;
    }

    /* do not return, fall-through to classic naming as fallback */
  }

  snprintf(name, IFNAMSIZ, "tnl_%08x", (olsr_cnf->ip_version == AF_INET) ? gw->originator.v4.s_addr : ++counter);
  *interfaceName = NULL;
}

/**
 * Set an olsr ipip tunnel name that is used by a certain gateway as unused
 *
 * @param gw pointer to the gateway
 */
static void set_unused_iptunnel_name(struct gateway_entry *gw) {
  struct interfaceName * ifn;

  if (!multi_gateway_mode()) {
    return;
  }

  assert(gw);

  ifn = find_interfaceName(gw);
  if (ifn) {
    ifn->gw = NULL;
    return;
  }
}

/**
 * Run the multi-gateway script/
 *
 * @param mode the mode (see SCRIPT_MODE_* defines)
 * @param add true to add policy routing, false to remove it
 * @param ifname the interface name (optional)
 * @param ifmark the interface mark (optional
 * @return true when successful
 */
static bool multiGwRunScript(const char * mode, bool add, const char * ifname, uint8_t * ifmark) {
  struct autobuf buf;
  int r;

  abuf_init(&buf, 1024);

  abuf_appendf(&buf, "\"%s\"", olsr_cnf->smart_gw_policyrouting_script);

  abuf_appendf(&buf, " \"%s\"", (olsr_cnf->ip_version == AF_INET) ? "ipv4" : "ipv6");

  assert(!strcmp(mode, SCRIPT_MODE_GENERIC) || !strcmp(mode, SCRIPT_MODE_OLSRIF) ||
      !strcmp(mode, SCRIPT_MODE_SGWSRVTUN) || !strcmp(mode, SCRIPT_MODE_EGRESSIF) ||
      !strcmp(mode, SCRIPT_MODE_SGWTUN));
  abuf_appendf(&buf, " \"%s\"", mode);

  abuf_appendf(&buf, " \"%s\"", add ? "add" : "del");

  if (ifname) {
    assert(!strcmp(mode, SCRIPT_MODE_OLSRIF) || !strcmp(mode, SCRIPT_MODE_SGWSRVTUN) ||
        !strcmp(mode, SCRIPT_MODE_EGRESSIF) || !strcmp(mode, SCRIPT_MODE_SGWTUN));
    abuf_appendf(&buf, " \"%s\"", ifname);
  } else {
    assert(!strcmp(mode, SCRIPT_MODE_GENERIC));
  }
  if (ifmark) {
    assert(!strcmp(mode, SCRIPT_MODE_EGRESSIF) || !strcmp(mode, SCRIPT_MODE_SGWTUN));
    assert(ifname);
    abuf_appendf(&buf, " \"%u\"", *ifmark);
  } else {
    assert(!strcmp(mode, SCRIPT_MODE_GENERIC) || !strcmp(mode, SCRIPT_MODE_OLSRIF) ||
      !strcmp(mode, SCRIPT_MODE_SGWSRVTUN));
  }

  r = system(buf.buf);

  abuf_free(&buf);

  return (r == 0);
}

/**
 * Setup generic multi-gateway iptables and ip rules
 *
 * - generic (on olsrd start/stop)
 * iptablesExecutable -t mangle -A OUTPUT -j CONNMARK --restore-mark
 *
 * @param add true to add policy routing, false to remove it
 * @return true when successful
 */
static bool multiGwRulesGeneric(bool add) {
  return multiGwRunScript(SCRIPT_MODE_GENERIC, add, NULL, NULL);
}

/**
 * Setup multi-gateway iptables and ip rules for all OLSR interfaces.
 *
 * - olsr interfaces (on olsrd start/stop)
 * iptablesExecutable -t mangle -A PREROUTING -i ${olsrInterface} -j CONNMARK --restore-mark
 *
 * @param add true to add policy routing, false to remove it
 * @return true when successful
 */
static bool multiGwRulesOlsrInterfaces(bool add) {
  bool ok = true;
  struct interface * ifn;

  for (ifn = ifnet; ifn; ifn = ifn->int_next) {
    if (!multiGwRunScript(SCRIPT_MODE_OLSRIF, add, ifn->int_name, NULL)) {
      ok = false;
      if (add) {
        return ok;
      }
    }
  }

  return ok;
}

/**
 * Setup multi-gateway iptables and ip rules for the smart gateway server tunnel.
 *
 * - sgw server tunnel interface (on olsrd start/stop)
 * iptablesExecutable -t mangle -A PREROUTING  -i tunl0 -j CONNMARK --restore-mark
 *
 * @param add true to add policy routing, false to remove it
 * @return true when successful
 */
static bool multiGwRulesSgwServerTunnel(bool add) {
  return multiGwRunScript(SCRIPT_MODE_SGWSRVTUN, add, server_tunnel_name(), NULL);
}

/**
 * Setup multi-gateway iptables and ip rules for all egress interfaces.
 *
 * - egress interfaces (on interface start/stop)
 * iptablesExecutable -t mangle -A POSTROUTING -m conntrack --ctstate NEW -o ${egressInterface} -j CONNMARK --set-mark ${egressInterfaceMark}
 * iptablesExecutable -t mangle -A INPUT       -m conntrack --ctstate NEW -i ${egressInterface} -j CONNMARK --set-mark ${egressInterfaceMark}
 * ip rule add fwmark ${egressInterfaceMark} table ${egressInterfaceMark} pref ${egressInterfaceMark}
 *
 * like table:
 * ppp0 91
 * eth0 92
 *
 * @param add true to add policy routing, false to remove it
 * @return true when successful
 */
static bool multiGwRulesEgressInterfaces(bool add) {
  bool ok = true;
  unsigned int i = 0;

  for (i = 0; i < olsr_cnf->smart_gw_egress_interfaces_count; i++) {
    struct interfaceName * ifn = &sgwEgressInterfaceNames[i];
    if (!multiGwRunScript(SCRIPT_MODE_EGRESSIF, add, ifn->name, &ifn->mark)) {
      ok = false;
      if (add) {
        return ok;
      }
    }
  }

  return ok;
}

/**
 * Setup multi-gateway iptables and ip rules for the smart gateway client tunnels.
 *
 * - sgw tunnels (on sgw tunnel start/stop)
 * iptablesExecutable -t mangle -A POSTROUTING -m conntrack --ctstate NEW -o ${sgwTunnelInterface} -j CONNMARK --set-mark ${sgwTunnelInterfaceMark}
 * ip rule add fwmark ${sgwTunnelInterfaceMark} table ${sgwTunnelInterfaceMark} pref ${sgwTunnelInterfaceMark}
 *
 * like table:
 * tnl_101 101
 * tnl_102 102
 * tnl_103 103
 * tnl_104 104
 * tnl_105 105
 * tnl_106 106
 * tnl_107 107
 * tnl_108 108
 */
static bool multiGwRulesSgwTunnels(bool add) {
  bool ok = true;
  unsigned int i = 0;

  while (i < olsr_cnf->smart_gw_use_count) {
    struct interfaceName * ifn = (olsr_cnf->ip_version == AF_INET) ? &sgwTunnel4InterfaceNames[i] : &sgwTunnel6InterfaceNames[i];
    if (!multiGwRunScript(SCRIPT_MODE_SGWTUN, add, ifn->name, &ifn->mark)) {
      ok = false;
      if (add) {
        return ok;
      }
    }

    i++;
  }

  return ok;
}

/*
 * Callback Functions
 */

/**
 * Callback for tunnel interface monitoring which will set the route into the tunnel
 * when the interface comes up again.
 *
 * @param if_index the interface index
 * @param ifh the interface
 * @param flag interface change flags
 */
static void smartgw_tunnel_monitor(int if_index __attribute__ ((unused)),
    struct interface *ifh __attribute__ ((unused)), enum olsr_ifchg_flag flag __attribute__ ((unused))) {
  return;
}

/**
 * Timer callback to remove and cleanup a gateway entry
 *
 * @param ptr
 */
static void cleanup_gateway_handler(void *ptr) {
  struct gateway_entry *gw = ptr;

  if (gw->ipv4 || gw->ipv6) {
    /* do not clean it up when it is in use */
    return;
  }

  /* remove gateway entry */
  avl_delete(&gateway_tree, &gw->node);
  olsr_cookie_free(gateway_entry_mem_cookie, gw);
}

/**
 * Remove a gateway from a gateway list.
 *
 * @param gw_list a pointer to the gateway list
 * @param ipv4 true when dealing with an IPv4 gateway / gateway list
 * @param gw a pointer to the gateway to remove from the list
 */
static void removeGatewayFromList(struct gw_list * gw_list, bool ipv4, struct gw_container_entry * gw) {
  if (gw->tunnel) {
    struct interfaceName * ifn = find_interfaceName(gw->gw);
    if (ifn) {
      olsr_os_inetgw_tunnel_route(gw->tunnel->if_index, ipv4, false, ifn->mark);
    }
    olsr_os_del_ipip_tunnel(gw->tunnel);
    set_unused_iptunnel_name(gw->gw);
    gw->tunnel = NULL;
  }
  gw->gw = NULL;
  olsr_cookie_free(gw_container_entry_mem_cookie, olsr_gw_list_remove(gw_list, gw));
}

/**
 * Remove expensive gateways from the gateway list.
 * It uses the smart_gw_takedown_percentage configuration parameter
 *
 * @param gw_list a pointer to the gateway list
 * @param ipv4 true when dealing with an IPv4 gateway / gateway list
 * @param current_gw the current gateway
 */
static void takeDownExpensiveGateways(struct gw_list * gw_list, bool ipv4, struct gw_container_entry * current_gw) {
  uint64_t current_gw_cost_boundary;

  /*
   * exit immediately when takedown is disabled, there is no current gateway, or
   * when there is only a single gateway
   */
  if ((olsr_cnf->smart_gw_takedown_percentage == 0) || (current_gw == NULL ) || (gw_list->count <= 1)) {
    return;
  }

  /* get the cost boundary */

  /* scale down because otherwise the percentage calculation can overflow */
  current_gw_cost_boundary = (current_gw->path_cost >> 2);

  if (olsr_cnf->smart_gw_takedown_percentage < 100) {
    current_gw_cost_boundary = (current_gw_cost_boundary * 100) / olsr_cnf->smart_gw_takedown_percentage;
  }

  /* loop while we still have gateways */
  while (gw_list->count > 1) {
    /* get the worst gateway */
    struct gw_container_entry * worst_gw = olsr_gw_list_get_worst_entry(gw_list);

    /* exit when it's the current gateway */
    if (worst_gw == current_gw) {
      return;
    }

    /*
     * exit when it (and further ones; the list is sorted on costs) has lower
     * costs than the boundary costs
     */
    if ((worst_gw->path_cost >> 2) < current_gw_cost_boundary) {
      return;
    }

    /* it's is too expensive: take it down */
    removeGatewayFromList(gw_list, ipv4, worst_gw);
  }
}

/**
 * Timer callback for proactive gateway takedown
 *
 * @param unused unused
 */
static void gw_takedown_timer_callback(void *unused __attribute__ ((unused))) {
  takeDownExpensiveGateways(&gw_list_ipv4, true, current_ipv4_gw);
  takeDownExpensiveGateways(&gw_list_ipv6, false, current_ipv6_gw);
}

/*
 * Main Interface
 */

/**
 * Initialize gateway system
 */
int olsr_init_gateways(void) {
  int retries = 5;

  gateway_entry_mem_cookie = olsr_alloc_cookie("gateway_entry_mem_cookie", OLSR_COOKIE_TYPE_MEMORY);
  olsr_cookie_set_memory_size(gateway_entry_mem_cookie, sizeof(struct gateway_entry));

  gw_container_entry_mem_cookie = olsr_alloc_cookie("gw_container_entry_mem_cookie", OLSR_COOKIE_TYPE_MEMORY);
  olsr_cookie_set_memory_size(gw_container_entry_mem_cookie, sizeof(struct gw_container_entry));

  avl_init(&gateway_tree, avl_comp_default);

  olsr_gw_list_init(&gw_list_ipv4, olsr_cnf->smart_gw_use_count);
  olsr_gw_list_init(&gw_list_ipv6, olsr_cnf->smart_gw_use_count);

  if (!multi_gateway_mode()) {
    sgwEgressInterfaceNames = NULL;
    sgwTunnel4InterfaceNames = NULL;
    sgwTunnel6InterfaceNames = NULL;
  } else {
    uint8_t i;
    struct sgw_egress_if * egressif;

    /* setup the egress interface name/mark pairs */
    sgwEgressInterfaceNames = olsr_malloc(sizeof(struct interfaceName) * olsr_cnf->smart_gw_egress_interfaces_count, "sgwEgressInterfaceNames");
    i = 0;
    egressif = olsr_cnf->smart_gw_egress_interfaces;
    while (egressif) {
      struct interfaceName * ifn = &sgwEgressInterfaceNames[i];
      ifn->gw = NULL;
      ifn->mark = i + olsr_cnf->smart_gw_mark_offset_egress;
      egressif->mark = ifn->mark;
      snprintf(&ifn->name[0], sizeof(ifn->name), egressif->name, egressif->mark);

      egressif = egressif->next;
      i++;
    }
    assert(i == olsr_cnf->smart_gw_egress_interfaces_count);

    /* setup the SGW tunnel name/mark pairs */
    sgwTunnel4InterfaceNames = olsr_malloc(sizeof(struct interfaceName) * olsr_cnf->smart_gw_use_count, "sgwTunnel4InterfaceNames");
    sgwTunnel6InterfaceNames = olsr_malloc(sizeof(struct interfaceName) * olsr_cnf->smart_gw_use_count, "sgwTunnel6InterfaceNames");
    for (i = 0; i < olsr_cnf->smart_gw_use_count; i++) {
      struct interfaceName * ifn = &sgwTunnel4InterfaceNames[i];
      ifn->gw = NULL;
      ifn->mark = i + olsr_cnf->smart_gw_mark_offset_tunnels;
      snprintf(&ifn->name[0], sizeof(ifn->name), "tnl_4%03u", ifn->mark);

      ifn = &sgwTunnel6InterfaceNames[i];
      ifn->gw = NULL;
      ifn->mark = i + olsr_cnf->smart_gw_mark_offset_tunnels;
      snprintf(&ifn->name[0], sizeof(ifn->name), "tnl_6%03u", ifn->mark);
    }
  }

  current_ipv4_gw = NULL;
  current_ipv6_gw = NULL;

  gw_handler = NULL;

  refresh_smartgw_netmask();

  /* initialize default gateway handler */
  gw_handler = &gw_def_handler;
  gw_handler->init();


  /*
   * There appears to be a kernel bug in some kernels (at least in the 3.0
   * Debian Squeeze kernel, but not in the Fedora 17 kernels) around
   * initialising the IPIP server tunnel (loading the IPIP module), so we retry
   * a few times before giving up
   */
  while (retries-- > 0) {
    if (!olsr_os_init_iptunnel(server_tunnel_name())) {
      retries = 5;
      break;
    }

    olsr_printf(0, "Could not initialise the IPIP server tunnel, retrying %d more times\n", retries);
  }
  if (retries <= 0) {
    return 1;
  }

  olsr_add_ifchange_handler(smartgw_tunnel_monitor);

  return 0;
}

/**
 * Startup gateway system
 */
int olsr_startup_gateways(void) {
  bool ok = true;

  if (!multi_gateway_mode()) {
    return 0;
  }

  ok = ok && multiGwRulesGeneric(true);
  ok = ok && multiGwRulesSgwServerTunnel(true);
  ok = ok && multiGwRulesOlsrInterfaces(true);
  ok = ok && multiGwRulesEgressInterfaces(true);
  ok = ok && multiGwRulesSgwTunnels(true);
  if (!ok) {
    olsr_printf(0, "Could not setup multi-gateway iptables and ip rules\n");
    olsr_shutdown_gateways();
    return 1;
  }

  if (olsr_cnf->smart_gw_takedown_percentage > 0) {
    /* start gateway takedown timer */
    olsr_set_timer(&gw_takedown_timer, olsr_cnf->smart_gw_period, 0, true, &gw_takedown_timer_callback, NULL, 0);
  }

  return 0;
}

/**
 * Shutdown gateway tunnel system
 */
void olsr_shutdown_gateways(void) {
  if (!multi_gateway_mode()) {
    return;
  }

  if (olsr_cnf->smart_gw_takedown_percentage > 0) {
    /* stop gateway takedown timer */
    olsr_stop_timer(gw_takedown_timer);
    gw_takedown_timer = NULL;
  }

  (void)multiGwRulesSgwTunnels(false);
  (void)multiGwRulesEgressInterfaces(false);
  (void)multiGwRulesOlsrInterfaces(false);
  (void)multiGwRulesSgwServerTunnel(false);
  (void)multiGwRulesGeneric(false);
}

/**
 * Cleanup gateway tunnel system
 */
void olsr_cleanup_gateways(void) {
  struct gateway_entry * tree_gw;
  struct gw_container_entry * gw;

  olsr_remove_ifchange_handler(smartgw_tunnel_monitor);

  /* remove all gateways in the gateway tree that are not the active gateway */
  OLSR_FOR_ALL_GATEWAY_ENTRIES(tree_gw) {
    if ((tree_gw != olsr_get_inet_gateway(false)) && (tree_gw != olsr_get_inet_gateway(true))) {
      olsr_delete_gateway_tree_entry(tree_gw, FORCE_DELETE_GW_ENTRY, true);
    }
  } OLSR_FOR_ALL_GATEWAY_ENTRIES_END(gw)

  /* remove all active IPv4 gateways (should be at most 1 now) */
  OLSR_FOR_ALL_GWS(&gw_list_ipv4.head, gw) {
    if (gw && gw->gw) {
      olsr_delete_gateway_entry(&gw->gw->originator, FORCE_DELETE_GW_ENTRY, true);
    }
  }
  OLSR_FOR_ALL_GWS_END(gw);

  /* remove all active IPv6 gateways (should be at most 1 now) */
  OLSR_FOR_ALL_GWS(&gw_list_ipv6.head, gw) {
    if (gw && gw->gw) {
      olsr_delete_gateway_entry(&gw->gw->originator, FORCE_DELETE_GW_ENTRY, true);
    }
  }
  OLSR_FOR_ALL_GWS_END(gw);

  /* there should be no more gateways */
  assert(!avl_walk_first(&gateway_tree));
  assert(!current_ipv4_gw);
  assert(!current_ipv6_gw);

  olsr_os_cleanup_iptunnel(server_tunnel_name());

  assert(gw_handler);
  gw_handler->cleanup();
  gw_handler = NULL;

  if (sgwEgressInterfaceNames) {
    free(sgwEgressInterfaceNames);
    sgwEgressInterfaceNames = NULL;
  }
  if (sgwTunnel4InterfaceNames) {
    free(sgwTunnel4InterfaceNames);
    sgwTunnel4InterfaceNames = NULL;
  }
  if (sgwTunnel6InterfaceNames) {
    free(sgwTunnel6InterfaceNames);
    sgwTunnel6InterfaceNames = NULL;
  }

  olsr_gw_list_cleanup(&gw_list_ipv6);
  olsr_gw_list_cleanup(&gw_list_ipv4);
  olsr_free_cookie(gw_container_entry_mem_cookie);
  olsr_free_cookie(gateway_entry_mem_cookie);
}

/**
 * Triggers the first lookup of a gateway.
 */
void olsr_trigger_inetgw_startup(void) {
  assert(gw_handler);
  gw_handler->startup();
}

/**
 * Print debug information about gateway entries
 */
#ifndef NODEBUG
void olsr_print_gateway_entries(void) {
  struct ipaddr_str buf;
  struct gateway_entry *gw;
  const int addrsize = olsr_cnf->ip_version == AF_INET ? (INET_ADDRSTRLEN - 1) : (INET6_ADDRSTRLEN - 1);

  OLSR_PRINTF(0, "\n--- %s ---------------------------------------------------- GATEWAYS\n\n", olsr_wallclock_string());
  OLSR_PRINTF(0, "%-*s %-6s %-9s %-9s %s\n",
      addrsize, "IP address", "Type", "Uplink", "Downlink", olsr_cnf->ip_version == AF_INET ? "" : "External Prefix");

  OLSR_FOR_ALL_GATEWAY_ENTRIES(gw) {
    OLSR_PRINTF(0, "%-*s %s%c%s%c%c %-9u %-9u %s\n",
        addrsize,
        olsr_ip_to_string(&buf, &gw->originator),
        gw->ipv4nat ? "" : "   ",
        gw->ipv4 ? '4' : ' ',
        gw->ipv4nat ? "(N)" : "",
        (gw->ipv4 && gw->ipv6) ? ',' : ' ',
        gw->ipv6 ? '6' : ' ',
        gw->uplink,
        gw->downlink,
        gw->external_prefix.prefix_len == 0 ? "" : olsr_ip_prefix_to_string(&gw->external_prefix));
  } OLSR_FOR_ALL_GATEWAY_ENTRIES_END(gw)
}
#endif /* NODEBUG */

/*
 * Tx Path Interface
 */

/**
 * Apply the smart gateway modifications to an outgoing HNA
 *
 * @param mask pointer to netmask of the HNA
 * @param prefixlen of the HNA
 */
void olsr_modifiy_inetgw_netmask(union olsr_ip_addr *mask, int prefixlen) {
  uint8_t *ptr = hna_mask_to_hna_pointer(mask, prefixlen);

  /* copy the current settings for uplink/downlink into the mask */
  memcpy(ptr, &smart_gateway_netmask, sizeof(smart_gateway_netmask) - prefixlen / 8);
  if (olsr_cnf->has_ipv4_gateway) {
    ptr[GW_HNA_FLAGS] |= GW_HNA_FLAG_IPV4;

    if (olsr_cnf->smart_gw_uplink_nat) {
      ptr[GW_HNA_FLAGS] |= GW_HNA_FLAG_IPV4_NAT;
    }
  }
  if (olsr_cnf->has_ipv6_gateway) {
    ptr[GW_HNA_FLAGS] |= GW_HNA_FLAG_IPV6;
  }
  if (!olsr_cnf->has_ipv6_gateway || prefixlen != ipv6_internet_route.prefix_len) {
    ptr[GW_HNA_FLAGS] &= ~GW_HNA_FLAG_IPV6PREFIX;
  }
}

/*
 * SgwDynSpeed Plugin Interface
 */

/**
 * Setup the gateway netmask
 */
void refresh_smartgw_netmask(void) {
  uint8_t *ip;

  /* clear the mask */
  memset(&smart_gateway_netmask, 0, sizeof(smart_gateway_netmask));

  if (olsr_cnf->smart_gw_active) {
    ip = (uint8_t *) &smart_gateway_netmask;

    ip[GW_HNA_FLAGS] |= GW_HNA_FLAG_LINKSPEED;
    ip[GW_HNA_DOWNLINK] = serialize_gw_speed(olsr_cnf->smart_gw_downlink);
    ip[GW_HNA_UPLINK] = serialize_gw_speed(olsr_cnf->smart_gw_uplink);

    if (olsr_cnf->ip_version == AF_INET6 && olsr_cnf->smart_gw_prefix.prefix_len > 0) {
      ip[GW_HNA_FLAGS] |= GW_HNA_FLAG_IPV6PREFIX;
      ip[GW_HNA_V6PREFIXLEN] = olsr_cnf->smart_gw_prefix.prefix_len;
      memcpy(&ip[GW_HNA_V6PREFIX], &olsr_cnf->smart_gw_prefix.prefix, 8);
    }
  }
}

/*
 * TC/SPF/HNA Interface
 */

/**
 * Checks if a HNA prefix/netmask combination is a smart gateway
 *
 * @param prefix
 * @param mask
 * @return true if is a valid smart gateway HNA, false otherwise
 */
bool olsr_is_smart_gateway(struct olsr_ip_prefix *prefix, union olsr_ip_addr *mask) {
  uint8_t *ptr;

  if (!is_prefix_inetgw(prefix)) {
    return false;
  }

  ptr = hna_mask_to_hna_pointer(mask, prefix->prefix_len);
  return ptr[GW_HNA_PAD] == 0 && ptr[GW_HNA_FLAGS] != 0;
}

/**
 * Update a gateway_entry based on a HNA
 *
 * @param originator ip of the source of the HNA
 * @param mask netmask of the HNA
 * @param prefixlen of the HNA
 * @param seqno the sequence number of the HNA
 */
void olsr_update_gateway_entry(union olsr_ip_addr *originator, union olsr_ip_addr *mask, int prefixlen, uint16_t seqno) {
  struct gw_container_entry * new_gw_in_list;
  uint8_t *ptr;
  struct gateway_entry *gw = node2gateway(avl_find(&gateway_tree, originator));

  if (!gw) {
    gw = olsr_cookie_malloc(gateway_entry_mem_cookie);
    gw->originator = *originator;
    gw->node.key = &gw->originator;

    avl_insert(&gateway_tree, &gw->node, AVL_DUP_NO);
  } else if (olsr_seqno_diff(seqno, gw->seqno) <= 0) {
    /* ignore older HNAs */
    return;
  }

  /* keep new HNA seqno */
  gw->seqno = seqno;

  ptr = hna_mask_to_hna_pointer(mask, prefixlen);
  if ((ptr[GW_HNA_FLAGS] & GW_HNA_FLAG_LINKSPEED) != 0) {
    gw->uplink = deserialize_gw_speed(ptr[GW_HNA_UPLINK]);
    gw->downlink = deserialize_gw_speed(ptr[GW_HNA_DOWNLINK]);
  } else {
    gw->uplink = 0;
    gw->downlink = 0;
  }

  gw->ipv4 = (ptr[GW_HNA_FLAGS] & GW_HNA_FLAG_IPV4) != 0;
  gw->ipv4nat = (ptr[GW_HNA_FLAGS] & GW_HNA_FLAG_IPV4_NAT) != 0;

  if (olsr_cnf->ip_version == AF_INET6) {
    gw->ipv6 = (ptr[GW_HNA_FLAGS] & GW_HNA_FLAG_IPV6) != 0;

    /* do not reset prefixlength for ::ffff:0:0 HNAs */
    if (prefixlen == ipv6_internet_route.prefix_len) {
      memset(&gw->external_prefix, 0, sizeof(gw->external_prefix));

      if ((ptr[GW_HNA_FLAGS] & GW_HNA_FLAG_IPV6PREFIX) != 0
          && memcmp(mask->v6.s6_addr, &ipv6_internet_route.prefix, olsr_cnf->ipsize) == 0) {
        /* this is the right prefix (2000::/3), so we can copy the prefix */
        gw->external_prefix.prefix_len = ptr[GW_HNA_V6PREFIXLEN];
        memcpy(&gw->external_prefix.prefix, &ptr[GW_HNA_V6PREFIX], 8);
      }
    }
  }

  /* stop cleanup timer if necessary */
  if (gw->cleanup_timer) {
    olsr_stop_timer(gw->cleanup_timer);
    gw->cleanup_timer = NULL;
  }

  /* update the costs of the gateway when it is an active gateway */
  new_gw_in_list = olsr_gw_list_find(&gw_list_ipv4, gw);
  if (new_gw_in_list) {
    assert(gw_handler);
    new_gw_in_list = olsr_gw_list_update(&gw_list_ipv4, new_gw_in_list, gw_handler->getcosts(new_gw_in_list->gw));
    assert(new_gw_in_list);
  }

  new_gw_in_list = olsr_gw_list_find(&gw_list_ipv6, gw);
  if (new_gw_in_list) {
    assert(gw_handler);
    new_gw_in_list = olsr_gw_list_update(&gw_list_ipv6, new_gw_in_list, gw_handler->getcosts(new_gw_in_list->gw));
    assert(new_gw_in_list);
  }

  /* call update handler */
  assert(gw_handler);
  gw_handler->update(gw);
}

/**
 * Delete a gateway based on the originator IP and the prefixlength of a HNA.
 * Should only be called if prefix is a smart_gw prefix or if node is removed
 * from TC set.
 *
 * @param originator
 * @param prefixlen
 * @param immediate when set to true then the gateway is removed from the
 * gateway tree immediately, else it is removed on a delayed schedule.
 */
void olsr_delete_gateway_entry(union olsr_ip_addr *originator, uint8_t prefixlen, bool immediate) {
  olsr_delete_gateway_tree_entry(node2gateway(avl_find(&gateway_tree, originator)), prefixlen, immediate);
}

/**
 * Delete a gateway entry .
 *
 * @param gw a gateway entry from the gateway tree
 * @param prefixlen
 * @param immediate when set to true then the gateway is removed from the
 * gateway tree immediately, else it is removed on a delayed schedule.
 */
static void olsr_delete_gateway_tree_entry(struct gateway_entry * gw, uint8_t prefixlen, bool immediate) {
  bool change = false;

  if (!gw) {
    return;
  }

  if (immediate && gw->cleanup_timer) {
    /* stop timer if we have to remove immediately */
    olsr_stop_timer(gw->cleanup_timer);
    gw->cleanup_timer = NULL;
  }

  if (gw->cleanup_timer == NULL || gw->ipv4 || gw->ipv6) {
    /* the gw  is not scheduled for deletion */

    if (olsr_cnf->ip_version == AF_INET && prefixlen == 0) {
      change = gw->ipv4;
      gw->ipv4 = false;
      gw->ipv4nat = false;
    } else if (olsr_cnf->ip_version == AF_INET6 && prefixlen == ipv6_internet_route.prefix_len) {
      change = gw->ipv6;
      gw->ipv6 = false;
    } else if (olsr_cnf->ip_version == AF_INET6 && prefixlen == ipv6_mappedv4_route.prefix_len) {
      change = gw->ipv4;
      gw->ipv4 = false;
      gw->ipv4nat = false;
    }

    if (prefixlen == FORCE_DELETE_GW_ENTRY || !(gw->ipv4 || gw->ipv6)) {
      struct gw_container_entry * gw_in_list;

      /* prevent this gateway from being chosen as the new gateway */
      gw->ipv4 = false;
      gw->ipv4nat = false;
      gw->ipv6 = false;

      /* handle gateway loss */
      assert(gw_handler);
      gw_handler->delete(gw);

      /* cleanup gateway if necessary */
      gw_in_list = olsr_gw_list_find(&gw_list_ipv4, gw);
      if (gw_in_list) {
        if (current_ipv4_gw && current_ipv4_gw->gw == gw) {
          olsr_os_inetgw_tunnel_route(current_ipv4_gw->tunnel->if_index, true, false, olsr_cnf->rt_table_tunnel);
          current_ipv4_gw = NULL;
        }

        if (gw_in_list->tunnel) {
          struct interfaceName * ifn = find_interfaceName(gw_in_list->gw);
          if (ifn) {
            olsr_os_inetgw_tunnel_route(gw_in_list->tunnel->if_index, true, false, ifn->mark);
          }
          olsr_os_del_ipip_tunnel(gw_in_list->tunnel);
          set_unused_iptunnel_name(gw_in_list->gw);
          gw_in_list->tunnel = NULL;
        }

        gw_in_list->gw = NULL;
        gw_in_list = olsr_gw_list_remove(&gw_list_ipv4, gw_in_list);
        olsr_cookie_free(gw_container_entry_mem_cookie, gw_in_list);
      }

      gw_in_list = olsr_gw_list_find(&gw_list_ipv6, gw);
      if (gw_in_list) {
        if (current_ipv6_gw && current_ipv6_gw->gw == gw) {
          olsr_os_inetgw_tunnel_route(current_ipv6_gw->tunnel->if_index, false, false, olsr_cnf->rt_table_tunnel);
          current_ipv6_gw = NULL;
        }

        if (gw_in_list->tunnel) {
          struct interfaceName * ifn = find_interfaceName(gw_in_list->gw);
          if (ifn) {
            olsr_os_inetgw_tunnel_route(gw_in_list->tunnel->if_index, false, false, ifn->mark);
          }
          olsr_os_del_ipip_tunnel(gw_in_list->tunnel);
          set_unused_iptunnel_name(gw_in_list->gw);
          gw_in_list->tunnel = NULL;
        }

        gw_in_list->gw = NULL;
        gw_in_list = olsr_gw_list_remove(&gw_list_ipv6, gw_in_list);
        olsr_cookie_free(gw_container_entry_mem_cookie, gw_in_list);
      }

      if (!immediate) {
        /* remove gateway entry on a delayed schedule */
        olsr_set_timer(&gw->cleanup_timer, GW_CLEANUP_INTERVAL, 0, false, cleanup_gateway_handler, gw, NULL);
      } else {
        cleanup_gateway_handler(gw);
      }

      /* when the current gateway was deleted, then immediately choose a new gateway */
      if (!current_ipv4_gw || !current_ipv6_gw) {
        assert(gw_handler);
        gw_handler->choose(!current_ipv4_gw, !current_ipv6_gw);
      }

    } else if (change) {
      assert(gw_handler);
      gw_handler->update(gw);
    }
  }
}

/**
 * Triggers a check if the one of the gateways have been lost or has an
 * ETX = infinity
 */
void olsr_trigger_gatewayloss_check(void) {
  bool ipv4 = false;
  bool ipv6 = false;

  if (current_ipv4_gw && current_ipv4_gw->gw) {
    struct tc_entry *tc = olsr_lookup_tc_entry(&current_ipv4_gw->gw->originator);
    ipv4 = (tc == NULL || tc->path_cost == ROUTE_COST_BROKEN);
  }
  if (current_ipv6_gw && current_ipv6_gw->gw) {
    struct tc_entry *tc = olsr_lookup_tc_entry(&current_ipv6_gw->gw->originator);
    ipv6 = (tc == NULL || tc->path_cost == ROUTE_COST_BROKEN);
  }

  if (ipv4 || ipv6) {
    assert(gw_handler);
    gw_handler->choose(ipv4, ipv6);
  }
}

/*
 * Gateway Plugin Functions
 */

/**
 * Sets a new internet gateway.
 *
 * @param originator ip address of the node with the new gateway
 * @param path_cost the path cost
 * @param ipv4 set ipv4 gateway
 * @param ipv6 set ipv6 gateway
 * @return true if an error happened, false otherwise
 */
bool olsr_set_inet_gateway(union olsr_ip_addr *originator, uint64_t path_cost, bool ipv4, bool ipv6) {
  struct gateway_entry *new_gw;

  ipv4 = ipv4 && (olsr_cnf->ip_version == AF_INET || olsr_cnf->use_niit);
  ipv6 = ipv6 && (olsr_cnf->ip_version == AF_INET6);
  if (!ipv4 && !ipv6) {
    return true;
  }

  new_gw = node2gateway(avl_find(&gateway_tree, originator));
  if (!new_gw) {
    /* the originator is not in the gateway tree, we can't set it as gateway */
    return true;
  }

  /* handle IPv4 */
  if (ipv4 &&
      new_gw->ipv4 &&
      (!new_gw->ipv4nat || olsr_cnf->smart_gw_allow_nat) &&
      (!current_ipv4_gw || current_ipv4_gw->gw != new_gw || current_ipv4_gw->path_cost != path_cost)) {
    /* new gw is different than the current gw, or costs have changed */

    struct gw_container_entry * new_gw_in_list = olsr_gw_list_find(&gw_list_ipv4, new_gw);
    if (new_gw_in_list) {
      /* new gw is already in the gw list */
      assert(new_gw_in_list->tunnel);
      olsr_os_inetgw_tunnel_route(new_gw_in_list->tunnel->if_index, true, true, olsr_cnf->rt_table_tunnel);
      current_ipv4_gw = olsr_gw_list_update(&gw_list_ipv4, new_gw_in_list, path_cost);
    } else {
      /* new gw is not yet in the gw list */
      char name[IFNAMSIZ];
      struct olsr_iptunnel_entry *new_v4gw_tunnel;
      struct interfaceName * interfaceName;

      if (olsr_gw_list_full(&gw_list_ipv4)) {
        /* the list is full: remove the worst active gateway */
        struct gw_container_entry* worst = olsr_gw_list_get_worst_entry(&gw_list_ipv4);
        assert(worst);

        removeGatewayFromList(&gw_list_ipv4, true, worst);
      }

      get_unused_iptunnel_name(new_gw, name, &interfaceName);
      new_v4gw_tunnel = olsr_os_add_ipip_tunnel(&new_gw->originator, true, name);
      if (new_v4gw_tunnel) {
        if (interfaceName) {
          olsr_os_inetgw_tunnel_route(new_v4gw_tunnel->if_index, true, true, interfaceName->mark);
        }
        olsr_os_inetgw_tunnel_route(new_v4gw_tunnel->if_index, true, true, olsr_cnf->rt_table_tunnel);

        new_gw_in_list = olsr_cookie_malloc(gw_container_entry_mem_cookie);
        new_gw_in_list->gw = new_gw;
        new_gw_in_list->tunnel = new_v4gw_tunnel;
        new_gw_in_list->path_cost = path_cost;
        current_ipv4_gw = olsr_gw_list_add(&gw_list_ipv4, new_gw_in_list);
      } else {
        /* adding the tunnel failed, we try again in the next cycle */
        set_unused_iptunnel_name(new_gw);
        ipv4 = false;
      }
    }
  }

  /* handle IPv6 */
  if (ipv6 &&
      new_gw->ipv6 &&
      (!current_ipv6_gw || current_ipv6_gw->gw != new_gw || current_ipv6_gw->path_cost != path_cost)) {
    /* new gw is different than the current gw, or costs have changed */

  	struct gw_container_entry * new_gw_in_list = olsr_gw_list_find(&gw_list_ipv6, new_gw);
    if (new_gw_in_list) {
      /* new gw is already in the gw list */
      assert(new_gw_in_list->tunnel);
      olsr_os_inetgw_tunnel_route(new_gw_in_list->tunnel->if_index, true, true, olsr_cnf->rt_table_tunnel);
      current_ipv6_gw = olsr_gw_list_update(&gw_list_ipv6, new_gw_in_list, path_cost);
    } else {
      /* new gw is not yet in the gw list */
      char name[IFNAMSIZ];
      struct olsr_iptunnel_entry *new_v6gw_tunnel;
      struct interfaceName * interfaceName;

      if (olsr_gw_list_full(&gw_list_ipv6)) {
        /* the list is full: remove the worst active gateway */
        struct gw_container_entry* worst = olsr_gw_list_get_worst_entry(&gw_list_ipv6);
        assert(worst);

        removeGatewayFromList(&gw_list_ipv6, false, worst);
      }

      get_unused_iptunnel_name(new_gw, name, &interfaceName);
      new_v6gw_tunnel = olsr_os_add_ipip_tunnel(&new_gw->originator, false, name);
      if (new_v6gw_tunnel) {
        if (interfaceName) {
          olsr_os_inetgw_tunnel_route(new_v6gw_tunnel->if_index, false, true, interfaceName->mark);
        }
        olsr_os_inetgw_tunnel_route(new_v6gw_tunnel->if_index, false, true, olsr_cnf->rt_table_tunnel);

        new_gw_in_list = olsr_cookie_malloc(gw_container_entry_mem_cookie);
        new_gw_in_list->gw = new_gw;
        new_gw_in_list->tunnel = new_v6gw_tunnel;
        new_gw_in_list->path_cost = path_cost;
        current_ipv6_gw = olsr_gw_list_add(&gw_list_ipv6, new_gw_in_list);
      } else {
        /* adding the tunnel failed, we try again in the next cycle */
        set_unused_iptunnel_name(new_gw);
        ipv6 = false;
      }
    }
  }

  return !ipv4 && !ipv6;
}

/**
 * @param ipv6 if set to true then the IPv6 gateway is returned, otherwise the IPv4
 * gateway is returned
 * @return a pointer to the gateway_entry of the current ipv4 internet gw or
 * NULL if not set.
 */
struct gateway_entry *olsr_get_inet_gateway(bool ipv6) {
	if (ipv6) {
		return current_ipv6_gw ? current_ipv6_gw->gw : NULL;
	}

	return current_ipv4_gw ? current_ipv4_gw->gw : NULL;
}

#endif /* __linux__ */
