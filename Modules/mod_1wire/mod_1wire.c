/* mod_1wire
 *
 * Handle 1-wire probes.
 * (also suitable for other exposed as a file values)
 *
 * This file is part of Marcel project and is following the same
 * license rules (see LICENSE file)
 *
 * 11/10/2022 - LF - First version
 */

#include "mod_1wire.h"	/* module's own stuffs */
#ifdef LUA
#	include "../mod_Lua/mod_Lua.h"
#endif
#include "../Marcel/MQTT_tools.h"

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>

static struct module_1wire mod_1wire;

void InitModule( void ){
	mod_1wire.module.name = "mod_1wire";	/* Identify the module */

		/* Initialize callbacks
		 * It's MANDATORY that all callbacks are initialised (even by a NULL value)
		 */
	mod_1wire.module.readconf = NULL;
	mod_1wire.module.acceptSDirective = NULL;
	mod_1wire.module.getSlaveFunction = NULL;
	mod_1wire.module.postconfInit = NULL;

	register_module( (struct Module *)&mod_1wire );	/* Register the module */
}
