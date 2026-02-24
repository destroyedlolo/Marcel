/* mod_TaHoma
 *
 * Steer Overkiz' TaHoma using local API
 *
 * This file is part of Marcel project and is following the same
 * license rules (see LICENSE file)
 *
 * 24/02/2025 - LF - Creation
 */

#include "mod_TaHoma.h"
#ifdef LUA
#	include "../mod_Lua/mod_Lua.h"
#endif

void InitModule( void ){
	initModule((struct Module *)&mod_TaHoma, "mod_TaHoma");	/* Identify the module */

		/* Initialize callbacks
		 * It's MANDATORY that all callbacks are initialised
		 */
	mod_TaHoma.module.readconf = readconf;
	mod_TaHoma.module.acceptSDirective = acceptSDirective;
	mod_TaHoma.module.getSlaveFunction = getSlaveFunction;

	mod_TaHoma.hostname = NULL;
	mod_TaHoma.ip = NULL;
	mod_TaHoma.token = NULL;
	mod_TaHoma.port = 8443;
	mod_TaHoma.unsafe = false;

	registerModule( (struct Module *)&mod_TaHoma );	/* Register the module */

#ifdef LUA
	if(mod_Lua){ /* Is mod_Lua loaded ? */

			/* Expose shared methods */

			/* Expose mod_owm's own function */
	}
#endif
}
