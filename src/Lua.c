/* Lua.c
 *
 * Implementation Lua user functions
 *
 * Cover by Creative Commons Attribution-NonCommercial 3.0 License
 * (http://creativecommons.org/licenses/by-nc/3.0/)
 *
 * 22/07/2015 LF - Creation
 */

#include "Marcel.h"
#include "Version.h"
#include "Alerting.h"
#include "MQTT_tools.h"


#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <libgen.h>		/* dirname ... */
#include <unistd.h>		/* chdir ... */
#include <errno.h>

#ifdef LUA

#include <lauxlib.h>	/* auxlib : usable hi-level function */
#include <lualib.h>		/* Functions to open libraries */

lua_State *L;
pthread_mutex_t onefunc;	/* Only one func can run at a time */

static void clean_lua(void){
	lua_close(L);
}

int findUserFunc( const char *fn ){
	pthread_mutex_lock( &onefunc );
	lua_getglobal(L, fn);
	if( lua_type(L, -1) != LUA_TFUNCTION ){
		if(lua_type(L, -1) != LUA_TNIL )
		publishLog('E', "\"%s\" is not a function, a %s", fn, lua_typename(L, lua_type(L, -1)) );
		lua_pop(L, 1);
		pthread_mutex_unlock( &onefunc );
		return LUA_REFNIL;
	}

	int ref = luaL_ref(L,LUA_REGISTRYINDEX);	/* Get the reference to the function */

	pthread_mutex_unlock( &onefunc );
	return ref;
}

void execUserFuncDeadPublisher( struct _DeadPublisher *ctx, const char *topic, const char *msg){
	if(ctx->funcid != LUA_REFNIL){	/* A function is defined */
		pthread_mutex_lock( &onefunc );
		lua_rawgeti( L, LUA_REGISTRYINDEX, ctx->funcid);	/* retrieves the function */
		lua_pushstring( L, topic);
		lua_pushstring( L, msg);
		if(lua_pcall( L, 2, 0, 0)){
			publishLog('E', "[%s] DPD / '%s' : %s", ctx->uid, topic, lua_tostring(L, -1));
			lua_pop(L, 1);  /* pop error message from the stack */
			lua_pop(L, 1);  /* pop NIL from the stack */
		}
		pthread_mutex_unlock( &onefunc );
	}
}

void execUserFuncEvery( struct _Every *ctx ){
	pthread_mutex_lock( &onefunc );
	lua_rawgeti( L, LUA_REGISTRYINDEX, ctx->funcid);	/* retrieves the function */
	lua_pushstring( L, ctx->uid );
	if(lua_pcall( L, 1, 0, 0)){
		publishLog('E', "[%s] Every : %s", ctx->uid, lua_tostring(L, -1));
		lua_pop(L, 1);  /* pop error message from the stack */
		lua_pop(L, 1);  /* pop NIL from the stack */
	}
	pthread_mutex_unlock( &onefunc );
}

void execUserFuncREST( struct _REST *ctx, char *res ){
	pthread_mutex_lock( &onefunc );
	lua_rawgeti( L, LUA_REGISTRYINDEX, ctx->funcid);	/* retrieves the function */
	if(res)
		lua_pushstring( L, res );
	else
		lua_pushnil(L);

	if(lua_pcall( L, 1, 0, 0)){
		publishLog('E', "[%s] RESTquery / %s : %s", ctx->uid, ctx->url, lua_tostring(L, -1));
		lua_pop(L, 1);  /* pop error message from the stack */
		lua_pop(L, 1);  /* pop NIL from the stack */
	}
	pthread_mutex_unlock( &onefunc );
}

void execUserFuncOutFile( struct _OutFile *ctx, const char *msg ){
	if(ctx->funcid != LUA_TNIL){	/* A function is defined */
		pthread_mutex_lock( &onefunc );
		lua_rawgeti( L, LUA_REGISTRYINDEX, ctx->funcid);	/* retrieves the function */
		lua_pushstring( L, ctx->uid );
		lua_pushstring( L, msg);
		if(lua_pcall( L, 2, 0, 0)){
			publishLog('E', "[%s] OutFile : %s", ctx->uid, lua_tostring(L, -1));
			lua_pop(L, 1);  /* pop error message from the stack */
			lua_pop(L, 1);  /* pop NIL from the stack */
		}
		pthread_mutex_unlock( &onefunc );
	}
}

bool execUserFuncFFV( struct _FFV *ctx, float val, float compensated){
/* <- val : raw value
 * <- corrected : temperature with offset applied
 */
	bool ret=true;

	if(ctx->funcid != LUA_TNIL){	/* A function is defined */
		pthread_mutex_lock( &onefunc );
		lua_rawgeti( L, LUA_REGISTRYINDEX, ctx->funcid);	/* retrieves the function */
		lua_pushstring( L, ctx->uid );
		lua_pushstring( L, ctx->topic );
		lua_pushnumber( L, val);
		lua_pushnumber( L, compensated);
		if(lua_pcall( L, 4, 1, 0)){
			publishLog('E', "[%s] FFV : %s", ctx->uid, lua_tostring(L, -1));
			lua_pop(L, 1);  /* pop error message from the stack */
			lua_pop(L, 1);  /* pop NIL from the stack */
		} else if(!lua_toboolean(L, -1))
			ret = false;
		pthread_mutex_unlock( &onefunc );
	}

	return ret;
}

void executeFailFunc( union CSection *ctx, const char *errmsg ){
puts("executeFailFunc");
	if(ctx->common.failfuncid != LUA_TNIL){	/* A function is defined */
		pthread_mutex_lock( &onefunc );
		lua_rawgeti( L, LUA_REGISTRYINDEX, ctx->common.failfuncid);	/* retrieves the function */
		lua_pushstring( L, ctx->common.uid );
		lua_pushstring( L, ctx->common.topic );
		lua_pushstring( L, errmsg);

		if(lua_pcall( L, 3, 0, 0)){
			publishLog('E', "[%s] FailFunc : %s", ctx->common.uid, lua_tostring(L, -1));
			lua_pop(L, 1);  /* pop error message from the stack */
			lua_pop(L, 1);  /* pop NIL from the stack */
		}
		pthread_mutex_unlock( &onefunc );
	}
puts("executeFailFunc ok");
}

static int lmSendNMsg(lua_State *L){
	if(lua_gettop(L) != 3){
		publishLog('E', "In your Lua code, SendNamedMessage() requires 3 arguments : Alerts' names, title and message");
		return 0;
	}

	const char *names = luaL_checkstring(L, 1);
	const char *topic = luaL_checkstring(L, 2);
	const char *msg = luaL_checkstring(L, 3);
	pnNotify( names, topic, msg );
	
	return 0;

}

static int lmSendMsg(lua_State *L){
	if(lua_gettop(L) != 2){
		publishLog('E', "In your Lua code, SendMessage() requires 2 arguments : title and message");
		return 0;
	}

	const char *topic = luaL_checkstring(L, 1);
	const char *msg = luaL_checkstring(L, 2);
	SendAlert( topic, msg, 0 );
	
	return 0;
}

static int lmSendMsgSMS(lua_State *L){
	if(lua_gettop(L) != 2){
		publishLog('E', "In your Lua code, SendMessageSMS() requires 2 arguments : title and message");
		return 0;
	}

	const char *topic = luaL_checkstring(L, 1);
	const char *msg = luaL_checkstring(L, 2);
	SendAlert( topic, msg, -1 );
	
	return 0;
}

static int lmRiseAlert(lua_State *L){
	if(lua_gettop(L) != 2){
		publishLog('E', "In your Lua code, RiseAlert() requires 2 arguments : title, message");
		return 0;
	}

	const char *topic = luaL_checkstring(L, 1);
	const char *msg = luaL_checkstring(L, 2);
	RiseAlert( topic, msg, 0 );
	
	return 0;
}

static int lmRiseAlertSMS(lua_State *L){
	if(lua_gettop(L) != 2){
		publishLog('E', "In your Lua code, RiseAlertSMS() requires 2 arguments : title, message");
		return 0;
	}

	const char *topic = luaL_checkstring(L, 1);
	const char *msg = luaL_checkstring(L, 2);
	RiseAlert( topic, msg, -1 );
	
	return 0;
}

static int lmClearAlert(lua_State *L){
	if(lua_gettop(L) != 1){
		publishLog('E', "In your Lua code, ClearAlert() requires 1 argument : title");
		return 0;
	}

	const char *topic = luaL_checkstring(L, 1);
	AlertIsOver( topic );

	return 0;
}

static int lmSendAlertsCounter(lua_State *L){
	sentAlertsCounter();

	return 0;
}

static int lmPublish(lua_State *L){
	int retain = 0;
	if(lua_gettop(L) < 2 || lua_gettop(L) > 3){
		publishLog('E', "In your Lua code, Publish() requires at least 2 arguments : topic, value and optionnaly retain");
		return 0;
	}

	const char *topic = luaL_checkstring(L, 1),
				*val = luaL_checkstring(L, 2);
	retain =  lua_toboolean(L, 3);

	mqttpublish( cfg.client, topic, strlen(val), (void *)val, retain );

	return 0;
}

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

static const struct luaL_reg MarcelLib [] = {
	{"SendNamedMessage", lmSendNMsg},
	{"SendMessage", lmSendMsg},
	{"SendMessageSMS", lmSendMsgSMS},
	{"RiseAlert", lmRiseAlert},		/* ... and send only a mail */
	{"RiseAlertSMS", lmRiseAlertSMS},	/* ... and send both a mail and a SMS */
	{"ClearAlert", lmClearAlert},
	{"SendAlertsCounter", lmSendAlertsCounter},
	{"MQTTPublish", lmPublish},
	{"Hostname", lmHostname},
	{"ClientID", lmClientID},
	{"Version", lmVersion},
	{"Copyright", lmCopyright},
	{"Log", lmLog},
	{NULL, NULL}
};

void init_Lua( const char *conffile ){
	if( cfg.luascript ){
		char *copy_cf;

		assert( copy_cf = strdup(conffile) );
		if(chdir( dirname( copy_cf )) == -1){
			publishLog('F', "chdir() : %s", strerror( errno ));
			exit(EXIT_FAILURE);
		}
		free( copy_cf );

		L = lua_open();		/* opens Lua */
		luaL_openlibs(L);	/* and it's libraries */
		atexit(clean_lua);

		luaL_newmetatable(L, "Marcel");
		lua_pushstring(L, "__index");
		lua_pushvalue(L, -2);
		lua_settable(L, -3);	/* metatable.__index = metatable */
		luaL_register(L,"Marcel", MarcelLib);

		char rp[ PATH_MAX ];
		if( realpath( cfg.luascript, rp ) ){
			lua_pushstring(L, dirname(rp) );
			lua_setglobal(L, "MARCEL_SCRIPT_DIR");
		} else {
			publishLog('F', "realpath() : %s", strerror( errno ));
			exit(EXIT_FAILURE);
		}
		
		int err = luaL_loadfile(L, cfg.luascript) || lua_pcall(L, 0, 0, 0);
		if(err){
			publishLog('F', "'%s' : %s", cfg.luascript, lua_tostring(L, -1));
			exit(EXIT_FAILURE);
		}

		pthread_mutex_init( &onefunc, NULL);
	} else 
		publishLog('W', "No FuncScript defined, Lua disabled");
}
#endif
