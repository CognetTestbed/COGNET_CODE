#include "olsrdPlugin.h"

/* Plugin includes */
#include "sgwDynSpeed.h"

/* OLSRD includes */
#include "olsr.h"

/* System includes */
#include <stdbool.h>

/*
 * OLSR Entrypoints
 */

/**
 OLSR entrypoint to initialise the plugin.

 @return
 - 0 on fail
 - 1 on success
 */
int olsrd_plugin_init(void) {
	bool retval = initSgwDynSpeed();
	if (retval) {
		olsr_printf(0, "%s\n", SGWDYNSPEED_PLUGIN_NAME_LONG
#ifdef GIT_SHA
				" (" GIT_SHA ")"
#endif /* GIT_SHA */
				);
	}
	return (retval ? 1 : 0);
}

/**
 OLSR entrypoint to retrieve the interface version supported by the plugin.

 @return
 the supported interface version
 */
int olsrd_plugin_interface_version(void) {
	return SGWDYNSPEED_PLUGIN_INTERFACE_VERSION;
}

/**
 OLSR entrypoint to retrieve the plugin parameter configuration.

 @param params
 a pointer to a variable in which the function stores a pointer to the
 plugin parameter configuration
 @param size
 a pointer to a variable in which the function stores the number of rows of the
 plugin parameter configuration
 */
void olsrd_get_plugin_parameters(const struct olsrd_plugin_parameters **params, int *size) {
	*params = &plugin_parameters[0];
	*size = ARRAYSIZE(plugin_parameters);
}

/*
 * Shared Library Entrypoints
 */

/**
 Shared library entrypoint declaration for initialisation
 */
static void __attribute__ ((constructor)) sgwDynSpeed_init(void);

/**
 Shared library entrypoint declaration for destruction
 */
static void __attribute__ ((destructor)) sgwDynSpeed_fini(void);

/**
 Shared library entrypoint for initialisation
 */
static void sgwDynSpeed_init(void) {
}

/**
 Shared library entrypoint for destruction
 */
static void sgwDynSpeed_fini(void) {
	stopSgwDynSpeed();
}
