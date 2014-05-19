/*
 * gateway_default_handler.c
 *
 *  Created on: Jan 29, 2010
 *      Author: rogge
 */
#ifdef __linux__

#include "gateway_default_handler.h"

#include "gateway_costs.h"
#include "defs.h"
#include "gateway.h"
#include "lq_plugin.h"

static uint32_t gw_def_nodecount;
static uint32_t gw_def_stablecount;
static bool gw_def_choose_new_ipv4_gw;
static bool gw_def_choose_new_ipv6_gw;
static struct timer_entry *gw_def_timer;
static struct costs_weights gw_costs_weights;

/* forward declarations */
static void gw_default_init(void);
static void gw_default_cleanup(void);
static void gw_default_startup_handler(void);
static uint64_t gw_default_getcosts(struct gateway_entry *gw);
static void gw_default_choosegw_handler(bool ipv4, bool ipv6);
static void gw_default_update_handler(struct gateway_entry *);
static void gw_default_delete_handler(struct gateway_entry *);

/**
 * Callback list for the gateway (default) handler
 */
struct olsr_gw_handler gw_def_handler = {
    &gw_default_init,
    &gw_default_cleanup,
    &gw_default_startup_handler,
    &gw_default_getcosts,
    &gw_default_choosegw_handler,
    &gw_default_update_handler,
    &gw_default_delete_handler
};

/*
 * Helper functions
 */

/**
 * Calculate the threshold path cost.
 *
 * @param path_cost the path cost
 * @return the threshold path cost
 */
static inline uint64_t gw_default_calc_threshold(uint64_t path_cost) {
  if (olsr_cnf->smart_gw_thresh == 0) {
    return path_cost;
  }

  return ((path_cost * (uint64_t) olsr_cnf->smart_gw_thresh) + (uint64_t) 50) / (uint64_t) 100;
}

/**
 * Look through the gateway list and select the best gateway
 * depending on the distance to this router
 */
static void gw_default_choose_gateway(void) {
  uint64_t cost_ipv4_threshold = UINT64_MAX;
  uint64_t cost_ipv6_threshold = UINT64_MAX;
  bool cost_ipv4_threshold_valid = false;
  bool cost_ipv6_threshold_valid = false;
  struct gateway_entry *chosen_gw_ipv4 = NULL;
  struct gateway_entry *chosen_gw_ipv6 = NULL;
  uint64_t chosen_gw_ipv4_costs = UINT64_MAX;
  uint64_t chosen_gw_ipv6_costs = UINT64_MAX;
  struct gateway_entry *gw;
  bool dual = false;

  if (olsr_cnf->smart_gw_thresh) {
    /* determine the path cost thresholds */

    uint64_t cost = gw_default_getcosts(olsr_get_inet_gateway(false));
    if (cost != UINT64_MAX) {
      cost_ipv4_threshold = gw_default_calc_threshold(cost);
      cost_ipv4_threshold_valid = true;
    }

    cost = gw_default_getcosts(olsr_get_inet_gateway(true));
    if (cost != UINT64_MAX) {
      cost_ipv6_threshold = gw_default_calc_threshold(cost);
      cost_ipv6_threshold_valid = true;
    }
  }

  OLSR_FOR_ALL_GATEWAY_ENTRIES(gw) {
    uint64_t gw_cost = gw_default_getcosts(gw);

    if (gw_cost == UINT64_MAX) {
      /* never select a node with infinite costs */
      continue;
    }

    if (gw_def_choose_new_ipv4_gw) {
      bool gw_eligible_v4 = gw->ipv4
          /* && (olsr_cnf->ip_version == AF_INET || olsr_cnf->use_niit) *//* contained in gw_def_choose_new_ipv4_gw */
          && (olsr_cnf->smart_gw_allow_nat || !gw->ipv4nat);
      if (gw_eligible_v4 && gw_cost < chosen_gw_ipv4_costs
          && (!cost_ipv4_threshold_valid || (gw_cost < cost_ipv4_threshold))) {
        chosen_gw_ipv4 = gw;
        chosen_gw_ipv4_costs = gw_cost;
      }
    }

    if (gw_def_choose_new_ipv6_gw) {
      bool gw_eligible_v6 = gw->ipv6
          /* && olsr_cnf->ip_version == AF_INET6 *//* contained in gw_def_choose_new_ipv6_gw */;
      if (gw_eligible_v6 && gw_cost < chosen_gw_ipv6_costs
          && (!cost_ipv6_threshold_valid || (gw_cost < cost_ipv6_threshold))) {
        chosen_gw_ipv6 = gw;
        chosen_gw_ipv6_costs = gw_cost;
      }
    }
  } OLSR_FOR_ALL_GATEWAY_ENTRIES_END(gw)

  /* determine if we should keep looking for IPv4 and/or IPv6 gateways */
  gw_def_choose_new_ipv4_gw = gw_def_choose_new_ipv4_gw && (chosen_gw_ipv4 == NULL);
  gw_def_choose_new_ipv6_gw = gw_def_choose_new_ipv6_gw && (chosen_gw_ipv6 == NULL);

  /* determine if we are dealing with a dual stack gateway */
  dual = chosen_gw_ipv4 && (chosen_gw_ipv4 == chosen_gw_ipv6);

  if (chosen_gw_ipv4) {
    /* we are dealing with an IPv4 or dual stack gateway */
    olsr_set_inet_gateway(&chosen_gw_ipv4->originator, chosen_gw_ipv4_costs, true, dual);
  }
  if (chosen_gw_ipv6 && !dual) {
    /* we are dealing with an IPv6-only gateway */
    olsr_set_inet_gateway(&chosen_gw_ipv6->originator, chosen_gw_ipv6_costs, false, true);
  }

  if ((olsr_cnf->smart_gw_thresh == 0) && !gw_def_choose_new_ipv4_gw && !gw_def_choose_new_ipv6_gw) {
    /* stop looking for a better gateway */
    olsr_stop_timer(gw_def_timer);
    gw_def_timer = NULL;
  }
}

/**
 * Timer callback for lazy gateway selection
 *
 * @param unused unused
 */
static void gw_default_timer(void *unused __attribute__ ((unused))) {
  /* accept a 10% increase/decrease in the number of gateway nodes without triggering a stablecount reset */
  unsigned int tree100percent = tc_tree.count * 10;
  uint32_t nodecount090percent = gw_def_nodecount * 9;
  uint32_t nodecount110percent = gw_def_nodecount * 11;
  if ((tree100percent >= nodecount090percent) && (tree100percent <= nodecount110percent)) {
    gw_def_nodecount = tc_tree.count;
  }

  if (tc_tree.count == gw_def_nodecount) {
    /* the number of gateway nodes is 'stable' */
    gw_def_stablecount++;
  } else {
    /* there was a significant change in the number of gateway nodes */
    gw_def_nodecount = tc_tree.count;
    gw_def_stablecount = 0;
  }

  if (gw_def_stablecount >= olsr_cnf->smart_gw_stablecount) {
    /* the number of gateway nodes is stable enough, so we should select a new gateway now */
    gw_default_choose_gateway();
  }
}

/**
 * Lookup a new gateway
 *
 * @param ipv4 lookup new v4 gateway
 * @param ipv6 lookup new v6 gateway
 */
static void gw_default_lookup_gateway(bool ipv4, bool ipv6) {
  if (ipv4) {
    /* get a new IPv4 gateway if we use OLSRv4 or NIIT */
    gw_def_choose_new_ipv4_gw = (olsr_cnf->ip_version == AF_INET) || olsr_cnf->use_niit;
  }
  if (ipv6) {
    /* get a new IPv6 gateway if we use OLSRv6 */
    gw_def_choose_new_ipv6_gw = olsr_cnf->ip_version == AF_INET6;
  }

  if (gw_def_choose_new_ipv4_gw || gw_def_choose_new_ipv6_gw) {
    gw_default_choose_gateway();
  }
}

/*
 * Exported functions
 */

/*
 * Handler functions
 */

/**
 * initialization of default gateway handler
 */
static void gw_default_init(void) {
  /* initialize values */
  gw_def_nodecount = 0;
  gw_def_stablecount = 0;
  gw_def_choose_new_ipv4_gw = true;
  gw_def_choose_new_ipv6_gw = true;
  gw_def_timer = NULL;

  gw_costs_weights.WexitU = olsr_cnf->smart_gw_weight_exitlink_up;
  gw_costs_weights.WexitD = olsr_cnf->smart_gw_weight_exitlink_down;
  gw_costs_weights.Wetx = olsr_cnf->smart_gw_weight_etx;
  gw_costs_weights.Detx = olsr_cnf->smart_gw_divider_etx;
}

/**
 * Cleanup default gateway handler
 */
static void gw_default_cleanup(void) {
}

/**
 * Handle gateway startup
 */
static void gw_default_startup_handler(void) {
  /* reset node count */
  gw_def_nodecount = tc_tree.count;
  gw_def_stablecount = 0;

  /* get a new IPv4 gateway if we use OLSRv4 or NIIT */
  gw_def_choose_new_ipv4_gw = (olsr_cnf->ip_version == AF_INET) || olsr_cnf->use_niit;

  /* get a new IPv6 gateway if we use OLSRv6 */
  gw_def_choose_new_ipv6_gw = olsr_cnf->ip_version == AF_INET6;

  /* keep in mind we might be a gateway ourself */
  gw_def_choose_new_ipv4_gw = gw_def_choose_new_ipv4_gw && !olsr_cnf->has_ipv4_gateway;
  gw_def_choose_new_ipv6_gw = gw_def_choose_new_ipv6_gw && !olsr_cnf->has_ipv6_gateway;

  /* (re)start gateway lazy selection timer */
  olsr_set_timer(&gw_def_timer, olsr_cnf->smart_gw_period, 0, true, &gw_default_timer, NULL, 0);
}

/**
 * Called when the costs of a gateway must be determined.
 *
 * @param gw the gateway
 * @return the costs, or UINT64_MAX in case the gateway is null or has inifinite costs
 */
static uint64_t gw_default_getcosts(struct gateway_entry *gw) {
  struct tc_entry* tc;

  if (!gw) {
    return UINT64_MAX;
  }

  tc = olsr_lookup_tc_entry(&gw->originator);

  if (!tc || (tc->path_cost == ROUTE_COST_BROKEN) || (!gw->uplink || !gw->downlink)) {
    /* gateways should not exist without tc entry */
    /* do not consider nodes with an infinite ETX */
    /* do not consider nodes without bandwidth or with a uni-directional link */
    return UINT64_MAX;
  }

  /* determine the path cost */
  return gw_costs_weigh(gw_costs_weights, tc->path_cost, gw->uplink, gw->downlink);
}

/**
 * Choose a new gateway
 *
 * @param ipv4 lookup new v4 gateway
 * @param ipv6 lookup new v6 gateway
 */
static void gw_default_choosegw_handler(bool ipv4, bool ipv6) {
  gw_default_lookup_gateway(ipv4, ipv6);

  if (gw_def_choose_new_ipv4_gw || gw_def_choose_new_ipv6_gw) {
    gw_default_startup_handler();
  }
}

/**
 * Update a gateway entry
 *
 * @param gw the gateway entry
 */
static void gw_default_update_handler(struct gateway_entry *gw) {
  if (olsr_cnf->smart_gw_thresh == 0) {
    /* classical behaviour: stick with the chosen gateway unless it changes */
    bool v4changed = gw && (gw == olsr_get_inet_gateway(false))
        && (!gw->ipv4 || (gw->ipv4nat && !olsr_cnf->smart_gw_allow_nat));
    bool v6changed = gw && (gw == olsr_get_inet_gateway(true)) && !gw->ipv6;

    if (v4changed || v6changed) {
      gw_default_lookup_gateway(v4changed, v6changed);
    }
  } else {
    /* new behaviour: always pick a new gateway */
    gw_default_lookup_gateway(true, true);
  }
}

/**
 * Remove a gateway entry
 *
 * @param gw the gateway entry
 */
static void gw_default_delete_handler(struct gateway_entry *gw) {
  bool isv4 = gw && (gw == olsr_get_inet_gateway(false));
  bool isv6 = gw && (gw == olsr_get_inet_gateway(true));

  if (isv4 || isv6) {
    gw_default_lookup_gateway(isv4, isv6);
  }
}
#endif /* __linux__ */
