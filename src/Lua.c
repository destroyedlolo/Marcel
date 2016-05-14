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
#include "Alerting.h"
#include "MQTT_tools.h"


#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <libgen.h>		/* dirname ... */
#include <unistd.h>		/* chdir ... */

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
		if(verbose && lua_type(L, -1) != LUA_TNIL )
			fprintf(stderr, "*E* \"%s\" is not a function, a %s.\n", fn, lua_typename(L, lua_type(L, -1)) );
		lua_pop(L, 1);
		pthread_mutex_unlock( &onefunc );
		return LUA_REFNIL;
	}

	int ref = luaL_ref(L,LUA_REGISTRYINDEX);	/* Get the reference to the function */

	pthread_mutex_unlock( &onefunc );
	return ref;
}

void execUserFuncDeadPublisher( struct _DeadPublisher *ctx, const char *topic, const char *msg){
	if(ctx->funcid != LUA_TNIL){	/* A function is defined */
		pthread_mutex_lock( &onefunc );
		lua_rawgeti( L, LUA_REGISTRYINDEX, ctx->funcid);	/* retrieves the function */
		lua_pushstring( L, topic);
		lua_pushstring( L, msg);
		if(lua_pcall( L, 2, 0, 0)){
			fprintf(stderr, "DPD / '%s' : %s\n", topic, lua_tostring(L, -1));
			lua_pop(L, 1);  /* pop error message from the stack */
			lua_pop(L, 1);  /* pop NIL from the stack */
		}
		pthread_mutex_unlock( &onefunc );
	}
}

void execUserFuncEvery( struct _Every *ctx ){
	pthread_mutex_lock( &onefunc );
	lua_rawgeti( L, LUA_REGISTRYINDEX, ctx->funcid);	/* retrieves the function */
	lua_pushstring( L, ctx->name );
	if(lua_pcall( L, 1, 0, 0)){
		fprintf(stderr, "Every / %s : %s\n", ctx->name, lua_tostring(L, -1));
		lua_pop(L, 1);  /* pop error message from the stack */
		lua_pop(L, 1);  /* pop NIL from the stack */
	}
	pthread_mutex_unlock( &onefunc );
}

static int lmSendNMsg(lua_State *L){
	if(lua_gettop(L) != 3){
		fputs("*E* In your Lua code, SendNamedMessage() requires 3 arguments : Alerts' names, title and message\n", stderr);
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
		fputs("*E* In your Lua code, SendMessage() requires 2 arguments : title and message\n", stderr);
		return 0;
	}

	const char *topic = luaL_checkstring(L, 1);
	const char *msg = luaL_checkstring(L, 2);
	SendAlert( topic, msg, 0 );
	
	return 0;
}

static int lmSendMsgSMS(lua_State *L){
	if(lua_gettop(L) != 2){
		fputs("*E* In your Lua code, SendMessageSMS() requires 2 arguments : title and message\n", stderr);
		return 0;
	}

	const char *topic = luaL_checkstring(L, 1);
	const char *msg = luaL_checkstring(L, 2);
	SendAlert( topic, msg, -1 );
	
	return 0;
}

static int lmRiseAlert(lua_State *L){
	if(lua_gettop(L) != 2){
		fputs("*E* In your Lua code, RiseAlert() requires 2 arguments : topic, message\n", stderr);
		return 0;
	}

	const char *topic = luaL_checkstring(L, 1);
	const char *msg = luaL_checkstring(L, 2);
	RiseAlert( topic, msg, 0 );
	
	return 0;
}

static int lmRiseAlertSMS(lua_State *L){
	if(lua_gettop(L) != 2){
		fputs("*E* In your Lua code, RiseAlert() requires 2 arguments : topic, message\n", stderr);
		return 0;
	}

	const char *topic = luaL_checkstring(L, 1);
	const char *msg = luaL_checkstring(L, 2);
	RiseAlert( topic, msg, -1 );
	
	return 0;
}

static int lmClearAlert(lua_State *L){
	if(lua_gettop(L) != 1){
		fputs("*E* In your Lua code, ClearAlert() requires 1 argument : topic\n", stderr);
		return 0;
	}

	const char *topic = luaL_checkstring(L, 1);
	AlertIsOver( topic );

	return 0;
}

static int lmPublish(lua_State *L){
	if(lua_gettop(L) != 2){
		fputs("*E* In your Lua code, Publish() requires 2 arguments : topic and value.\n", stderr);
		return 0;
	}

	const char *topic = luaL_checkstring(L, 1),
				*val = luaL_checkstring(L, 2);

	mqttpublish( cfg.client, topic, strlen(val), (void *)val, 0 );

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
	lua_pushstring(L, VERSION);

	return 1;
}

static const struct luaL_reg MarcelLib [] = {
	{"SendNamedMessage", lmSendNMsg},
	{"SendMessage", lmSendMsg},
	{"SendMessageSMS", lmSendMsgSMS},
	{"RiseAlert", lmRiseAlert},		/* ... and send only a mail */
	{"RiseAlertSMS", lmRiseAlertSMS},	/* ... and send both a mail and a SMS */
	{"ClearAlert", lmClearAlert},
	{"MQTTPublish", lmPublish},
	{"Hostname", lmHostname},
	{"ClientID", lmClientID},
	{"Version", lmVersion},
	{NULL, NULL}
};

void init_Lua( const char *conffile ){
	if( cfg.luascript ){
		char *copy_cf;

		assert( copy_cf = strdup(conffile) );
		if(chdir( dirname( copy_cf )) == -1){
			perror("chdir() : ");
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

		int err = luaL_loadfile(L, cfg.luascript) || lua_pcall(L, 0, 0, 0);
		if(err){
			fprintf(stderr, "*F* '%s' : %s\n", cfg.luascript, lua_tostring(L, -1));
			exit(EXIT_FAILURE);
		}

		pthread_mutex_init( &onefunc, NULL);
	} else if(verbose)
		puts("*W* No FuncScript defined, Lua disabled");
}
#endif
