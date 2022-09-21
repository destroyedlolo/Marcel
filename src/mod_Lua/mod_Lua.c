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

static void clean_lua(void){
	lua_close(mod_Lua.L);
}

static void ml_postconfInit( void ){
puts("mod_Lua : postconfInit()");
}

void InitModule( void ){
	mod_Lua.module.name = "mod_Lua";

	mod_Lua.module.readconf = NULL;
	mod_Lua.module.acceptSDirective = NULL;
	mod_Lua.module.getSlaveFunction = NULL;
	mod_Lua.module.postconfInit = ml_postconfInit;

	if(findModuleByName(mod_Lua.module.name) != (uint8_t)-1){
		publishLog('F', "Module '%s' is already loaded", mod_Lua.module.name);
		exit(EXIT_FAILURE);
	}

	register_module( (struct Module *)&mod_Lua );	/* Register the module */

	mod_Lua.L = luaL_newstate();		/* opens Lua */
	luaL_openlibs( mod_Lua.L );	/* and it's libraries */

	atexit(clean_lua);
}
