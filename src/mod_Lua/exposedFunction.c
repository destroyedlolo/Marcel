/* mod_Lua - Exposed functions
 *
 * 24/09/2022 - LF - First version
 */

#include "mod_Lua.h"	/* module's own stuffs */
#include "../Version.h"

#include <unistd.h>

static int lmLog(lua_State *L){
	if(lua_gettop(L) != 2){
		publishLog('E', "In your Lua code, Log() requires 2 arguments : severity and message");
		return 0;
	}

	const char *sev = luaL_checkstring(L, 1);
	const char *msg = luaL_checkstring(L, 2);

	publishLog( *sev, msg );

	return 0;
}

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

const struct luaL_Reg MarcelLib [] = {
#if 0
	{"SendNamedMessage", lmSendNMsg},
	{"SendMessage", lmSendMsg},
	{"SendMessageSMS", lmSendMsgSMS},
	{"RiseAlert", lmRiseAlert},		/* ... and send only a mail */
	{"RiseAlertSMS", lmRiseAlertSMS},	/* ... and send both a mail and a SMS */
	{"ClearAlert", lmClearAlert},
	{"SendAlertsCounter", lmSendAlertsCounter},
	{"MQTTPublish", lmPublish},
#endif
	{"Log", lmLog},
	{"Hostname", lmHostname},
	{"ClientID", lmClientID},
	{"Version", lmVersion},
	{"Copyright", lmCopyright},
	{NULL, NULL}
};


