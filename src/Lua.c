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

#include <stdlib.h>
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
	lua_getglobal(L, fn);
	if( lua_type(L, -1) != LUA_TFUNCTION ){
		if(verbose && lua_type(L, -1) != LUA_TNIL )
			fprintf(stderr, "*E* \"%s\" is not a function, a %s.\n", fn, lua_typename(L, lua_type(L, -1)) );
		lua_pop(L, 1);
		return LUA_REFNIL;
	}

	int ref = luaL_ref(L,LUA_REGISTRYINDEX);	/* Get the reference to the function */

	return ref;
}

void execUserFuncTopic( struct _DeadPublisher *ctx, const char *topic, const char *msg){
	if(ctx->funcid != LUA_TNIL){	/* A function is defined */
		pthread_mutex_lock( &onefunc );
		lua_rawgeti( L, LUA_REGISTRYINDEX, ctx->funcid);	/* retrieves the function */
		lua_pushstring( L, topic);
		lua_pushstring( L, msg);
		lua_pcall( L, 2, 0, 0);
		pthread_mutex_unlock( &onefunc );
	}
}

static int lmRiseAlert(lua_State *L){
	if(lua_gettop(L) != 2){
		fputs("*E* In your Lua code, RiseAlert() requires 2 arguments : topic, message\n", stderr);
		return 0;
	}

	const char *topic = luaL_checkstring(L, 1);
	const char *msg = luaL_checkstring(L, 2);
	RiseAlert( topic, msg );
	
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

static const struct luaL_reg MarcelLib [] = {
	{"RiseAlert", lmRiseAlert},
	{"ClearAlert", lmClearAlert},
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

		int err = luaL_loadfile(L, cfg.luascript) || lua_pcall(L, 0, 0, 0);
		if(err){
			fprintf(stderr, "*F* '%s' : %s\n", cfg.luascript, lua_tostring(L, -1));
			exit(EXIT_FAILURE);
		}

		luaL_newmetatable(L, "Marcel");
		lua_pushstring(L, "__index");
		lua_pushvalue(L, -2);
		lua_settable(L, -3);	/* metatable.__index = metatable */
		luaL_register(L,"Marcel", MarcelLib);

		pthread_mutex_init( &onefunc, NULL);
	} else if(verbose)
		puts("*W* No FuncScript defined, Lua disabled");
}
#endif
