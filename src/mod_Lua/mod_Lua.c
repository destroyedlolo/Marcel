/* mod_Lua
 *
 * Add Lua's plugins to Marcel
 *
 * This file is part of Marcel project and is following the same
 * license rules (see LICENSE file)
 *
 * 21/09/2022 - LF - First version
 */

#include "mod_Lua.h"	/* module's own stuffs */

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <libgen.h>
#include <assert.h>
#include <ctype.h>

static struct module_Lua mod_Lua;	/* Module's own structure */

	/* Expose function to Lua */
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

	/**
	 * @brief find user defined function
	 * @param name function name
	 * @return function identifier
	 */
static int findUserFunc( const char *name ){
	pthread_mutex_lock( &mod_Lua.onefunc );
	lua_getglobal(mod_Lua.L, name);
	if( lua_type(mod_Lua.L, -1) != LUA_TFUNCTION ){
		if(lua_type(mod_Lua.L, -1) != LUA_TNIL )
			publishLog('E', "\"%s\" is not a function, a %s", name, lua_typename(mod_Lua.L, lua_type(mod_Lua.L, -1)) );
		lua_pop(mod_Lua.L, 1);
		pthread_mutex_unlock( &mod_Lua.onefunc );
		return LUA_REFNIL;
	}

	int ref = luaL_ref(mod_Lua.L,LUA_REGISTRYINDEX);	/* Get the reference to the function */

	pthread_mutex_unlock( &mod_Lua.onefunc );
	return ref;
}

static void lockState(void){
	pthread_mutex_lock( &mod_Lua.onefunc );
}

static void unlockState(void){
	pthread_mutex_unlock( &mod_Lua.onefunc );
}

static void pushNumber(const double val){
	lua_pushnumber( mod_Lua.L, val );
}

static void pushFUnctionId(int id){
	lua_rawgeti( mod_Lua.L, LUA_REGISTRYINDEX, id );
}

static int ml_exec(int narg, int nret){
	return lua_pcall( mod_Lua.L, narg, nret, 0);
}

static void ml_pop(int idx){
	return lua_pop( mod_Lua.L, idx );
}

static const char *getStringFromStack(int idx){
	return lua_tostring( mod_Lua.L, idx );
}

static bool getBooleanFromStack(int idx){
	return lua_toboolean( mod_Lua.L, idx );
}

static enum RC_readconf ml_readconf(uint8_t mid, const char *l, struct Section **section ){
	const char *arg;

	if((arg = striKWcmp(l,"UserFuncScript="))){	/* script to load */
		if(*section){
			publishLog('F', "UserFuncScript can't be part of a section");
			exit(EXIT_FAILURE);
		}

		mod_Lua.script = strdup(arg);
		assert(mod_Lua.script);

		if(cfg.verbose)	/* Be verbose if requested */
			publishLog('C', "\tUser functions definition script : %s", mod_Lua.script);

		return ACCEPTED;
	} else if(*section){
		const char *directive = l;

		while(isspace(*directive))
			directive++;

		if((arg = striKWcmp(directive,"Func="))){
			acceptSectionDirective( *section, "Func=" );

			(*section)->funcname = strdup(arg);
			assert( (*section)->funcname );

			if(cfg.verbose)	/* Be verbose if requested */
				publishLog('C', "\t\tFunction : %s", (*section)->funcname);

			return ACCEPTED;
		}
	}

	return REJECTED;
}

/* ***
 * Module's initialisation function
 * ***/

static void clean_lua(void){
	lua_close(mod_Lua.L);
}

static void ml_postconfInit( void ){
	if(mod_Lua.script){
		char rp[ PATH_MAX ];
		if( realpath( mod_Lua.script, rp ) ){
			lua_pushstring(mod_Lua.L, basename(rp) );
			lua_setglobal(mod_Lua.L, "MARCEL_SCRIPT");

			lua_pushstring(mod_Lua.L, dirname(rp) );
			lua_setglobal(mod_Lua.L, "MARCEL_SCRIPT_DIR");

#ifdef DEBUG
			if(cfg.debug){
				lua_pushinteger(mod_Lua.L, 1 );
				lua_setglobal(mod_Lua.L, "MARCEL_DEBUG");
			}
#endif
		} else {
			publishLog('F', "realpath(%s) : %s", mod_Lua.script, strerror( errno ));
			exit(EXIT_FAILURE);
		}

		int err = luaL_loadfile(mod_Lua.L, mod_Lua.script) || lua_pcall(mod_Lua.L, 0, 0, 0);
		if(err){
			publishLog('F', "'%s' : %s", mod_Lua.script, lua_tostring(mod_Lua.L, -1));
			exit(EXIT_FAILURE);
		}

	}
}

void InitModule( void ){
	mod_Lua.module.name = "mod_Lua";

	mod_Lua.module.readconf = ml_readconf;
	mod_Lua.module.acceptSDirective = NULL;
	mod_Lua.module.getSlaveFunction = NULL;			/* No section handled */
	mod_Lua.module.postconfInit = ml_postconfInit;

	mod_Lua.script = NULL;
	mod_Lua.exposeFunctions = exposeFunctions;
	mod_Lua.findUserFunc = findUserFunc;
	mod_Lua.lockState = lockState;
	mod_Lua.unlockState = unlockState;
	mod_Lua.pushNumber = pushNumber;
	mod_Lua.pushFUnctionId = pushFUnctionId;
	mod_Lua.exec = ml_exec;
	mod_Lua.pop = ml_pop;
	mod_Lua.getStringFromStack = getStringFromStack;
	mod_Lua.getBooleanFromStack = getBooleanFromStack;

	register_module( (struct Module *)&mod_Lua );	/* Register the module */

	mod_Lua.L = luaL_newstate();		/* opens Lua */
	luaL_openlibs( mod_Lua.L );	/* and it's libraries */

	atexit(clean_lua);

		/* Expose some functions in Lua */
	exposeFunctions("Marcel", MarcelLib);
}
