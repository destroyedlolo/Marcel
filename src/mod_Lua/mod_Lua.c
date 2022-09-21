/* mod_Lua
 *
 * Add Lua's plugins to Marcel
 *
 * 21/09/2022 - LF - First version
 */

#include "mod_Lua.h"	/* module's own stuffs */

#include <stdlib.h>
#include <string.h>
#include <unistd.h>


static struct module_Lua mod_Lua;	/* Module's own structure */

/* ***
 * Module's initialisation function
 * ***/

void InitModule( void ){
	mod_Lua.module.name = "mod_Lua";

	mod_Lua.module.readconf = NULL;
	mod_Lua.module.acceptSDirective = NULL;
	mod_Lua.module.getSlaveFunction = NULL;

	if(findModuleByName(mod_Lua.module.name) != (uint8_t)-1){
		publishLog('F', "Module '%s' is already loaded", mod_Lua.module.name);
		exit(EXIT_FAILURE);
	}

	register_module( (struct Module *)&mod_Lua );	/* Register the module */


}
