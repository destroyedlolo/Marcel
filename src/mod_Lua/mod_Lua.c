/* mod_Lua
 *
 * Add Lua's plugins to Marcel
 *
 * 21/09/2022 - LF - First version
 */

#include "mod_Lua.h"	/* module's own stuffs */
#include "../Version.h"

#include <stdlib.h>
#include <string.h>
#include <unistd.h>


static struct module_Lua mod_Lua;	/* Module's own structure */

/* ***
 * Lua exposed functions
 * ***/

static int lmHostname(lua_State *L){
	char n[HOST_NAME_MAX];
	gethostname(n, HOST_NAME_MAX);

	lua_pushstring(L, n);
	return 1;
}

static int lmClientID(lua_State *L){
	lua_pushstring(L, cfg.ClientID);

	return 1;
}

static int lmVersion(lua_State *L){
	lua_pushstring(L, MARCEL_VERSION);

	return 1;
}

static int lmCopyright(lua_State *L){
	lua_pushstring(L, MARCEL_COPYRIGHT);

	return 1;
}

static const struct luaL_Reg MarcelLib [] = {
#if 0
	{"SendNamedMessage", lmSendNMsg},
	{"SendMessage", lmSendMsg},
	{"SendMessageSMS", lmSendMsgSMS},
	{"RiseAlert", lmRiseAlert},		/* ... and send only a mail */
	{"RiseAlertSMS", lmRiseAlertSMS},	/* ... and send both a mail and a SMS */
	{"ClearAlert", lmClearAlert},
	{"SendAlertsCounter", lmSendAlertsCounter},
	{"MQTTPublish", lmPublish},
	{"Log", lmLog},
#endif
	{"Hostname", lmHostname},
	{"ClientID", lmClientID},
	{"Version", lmVersion},
	{"Copyright", lmCopyright},
	{NULL, NULL}
};

static int exposeFunctions( const char *name, const struct luaL_Reg *funcs){
	luaL_newmetatable(mod_Lua.L, name);
	lua_pushstring(mod_Lua.L, "__index");
	lua_pushvalue(mod_Lua.L, -2);
	lua_settable(mod_Lua.L, -3);	/* metatable.__index = metatable */

/* printf("******* version %d\n", LUA_VERSION_NUM); */
#if LUA_VERSION_NUM < 503
	/* Insert __name field if Lua < 5.3
	 * on 5.3+, it's provided out of the box
	 */
	lua_pushstring(mod_Lua.L, name);
	lua_setfield(mod_Lua.L, -2, "__name");
#endif

	if(funcs){	/* May be NULL if we're creating an empty metatable */
#if LUA_VERSION_NUM > 501
		luaL_setfuncs(mod_Lua.L, funcs, 0);
#else
		luaL_register(mod_Lua.L, NULL, funcs);
#endif
	}

	return 1;
}


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

	mod_Lua.exposeFunctions = exposeFunctions;

	if(findModuleByName(mod_Lua.module.name) != (uint8_t)-1){
		publishLog('F', "Module '%s' is already loaded", mod_Lua.module.name);
		exit(EXIT_FAILURE);
	}

	register_module( (struct Module *)&mod_Lua );	/* Register the module */

	mod_Lua.L = luaL_newstate();		/* opens Lua */
	luaL_openlibs( mod_Lua.L );	/* and it's libraries */

	atexit(clean_lua);

		/* Expose some functions in Lua */
	exposeFunctions("Marcel", MarcelLib);
}
