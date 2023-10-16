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
#include "mlSection.h"

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <libgen.h>
#include <assert.h>

static struct module_Lua mod_Lua_storage;	/* Module's own structure */

	/* Expose function to Lua */
static int exposeFunctions( const char *name, const struct luaL_Reg *funcs){
#if LUA_VERSION_NUM > 501
	lua_newtable(mod_Lua->L);
	luaL_setfuncs (mod_Lua->L, funcs, 0);
	lua_pushvalue(mod_Lua->L, -1);	// pluck these lines out if they offend you
	lua_setglobal(mod_Lua->L, name); // for they clobber the Holy _G
#else
	luaL_register(mod_Lua->L, name, funcs);
#endif

	return 1;
}

static int exposeObjMethods( lua_State *L, const char *name, const struct luaL_Reg *funcs){
	luaL_newmetatable(L, name);
	lua_pushstring(L, "__index");
	lua_pushvalue(L, -2);
	lua_settable(L, -3);	/* metatable.__index = metatable */

#if LUA_VERSION_NUM < 503
	/* Insert __name field if Lua < 5.3
	 * on 5.3+, it's provided out of the box
	 */
	lua_pushstring(L, name);
	lua_setfield(L, -2, "__name");
#endif

	if(funcs){	/* May be NULL if we're creating an empty metatable */
#if LUA_VERSION_NUM > 501
		luaL_setfuncs( L, funcs, 0);
#else
		luaL_register(L, NULL, funcs);
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
	pthread_mutex_lock( &mod_Lua->onefunc );
	lua_getglobal(mod_Lua->L, name);
	if( lua_type(mod_Lua->L, -1) != LUA_TFUNCTION ){
		if(lua_type(mod_Lua->L, -1) != LUA_TNIL )
			publishLog('E', "\"%s\" is not a function, a %s", name, lua_typename(mod_Lua->L, lua_type(mod_Lua->L, -1)) );
		lua_pop(mod_Lua->L, 1);
		pthread_mutex_unlock( &mod_Lua->onefunc );
		return LUA_REFNIL;
	}

	int ref = luaL_ref(mod_Lua->L,LUA_REGISTRYINDEX);	/* Get the reference to the function */

	pthread_mutex_unlock( &mod_Lua->onefunc );
	return ref;
}

static void lockState(void){
	pthread_mutex_lock( &mod_Lua->onefunc );
}

static void unlockState(void){
	pthread_mutex_unlock( &mod_Lua->onefunc );
}

static void pushNumber(const double val){
	lua_pushnumber( mod_Lua->L, val );
}

static void pushString(const char *val){
	lua_pushstring( mod_Lua->L, val );
}

static void pushFunctionId(int id){
	lua_rawgeti( mod_Lua->L, LUA_REGISTRYINDEX, id );
}

static int ml_exec(int narg, int nret){
	return lua_pcall( mod_Lua->L, narg, nret, 0);
}

static void ml_pop(int idx){
	return lua_pop( mod_Lua->L, idx );
}

static const char *getStringFromStack(int idx){
	return lua_tostring( mod_Lua->L, idx );
}

static bool getBooleanFromStack(int idx){
	return lua_toboolean( mod_Lua->L, idx );
}

static enum RC_readconf ml_readconf(uint8_t mid, const char *l, struct Section **section ){
	const char *arg;

	if((arg = striKWcmp(l,"UserFuncScript="))){	/* script to load */
		if(*section){
			publishLog('F', "UserFuncScript can't be part of a section");
			exit(EXIT_FAILURE);
		}

		mod_Lua->script = strdup(arg);
		assert(mod_Lua->script);

		if(cfg.verbose)	/* Be verbose if requested */
			publishLog('C', "\tUser functions definition script : %s", mod_Lua->script);

		return ACCEPTED;
	} else if(*section){
		if((arg = striKWcmp(l, "Func="))){
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
	lua_close(mod_Lua->L);
}

static void ml_postconfInit( uint8_t mid ){
	if(mod_Lua->script){
		char rp[ PATH_MAX ];
		if( realpath( mod_Lua->script, rp ) ){
			lua_pushstring(mod_Lua->L, basename(rp) );
			lua_setglobal(mod_Lua->L, "MARCEL_SCRIPT");

			lua_pushstring(mod_Lua->L, dirname(rp) );
			lua_setglobal(mod_Lua->L, "MARCEL_SCRIPT_DIR");

			if(cfg.verbose){
				lua_pushinteger(mod_Lua->L, 1 );
				lua_setglobal(mod_Lua->L, "MARCEL_VERBOSE");
			}

#ifdef DEBUG
			if(cfg.debug){
				lua_pushinteger(mod_Lua->L, 1 );
				lua_setglobal(mod_Lua->L, "MARCEL_DEBUG");
			}
#endif
		} else {
			publishLog('F', "realpath(%s) : %s", mod_Lua->script, strerror( errno ));
			exit(EXIT_FAILURE);
		}

		int err = luaL_loadfile(mod_Lua->L, mod_Lua->script) || lua_pcall(mod_Lua->L, 0, 0, 0);
		if(err){
			publishLog('F', "'%s' : %s", mod_Lua->script, lua_tostring(mod_Lua->L, -1));
			exit(EXIT_FAILURE);
		}

	}
}

void InitModule( void ){
	initModule((struct Module *)&mod_Lua_storage, "mod_Lua_storage");

	mod_Lua_storage.module.readconf = ml_readconf;
	mod_Lua_storage.module.postconfInit = ml_postconfInit;

	mod_Lua_storage.script = NULL;
	mod_Lua_storage.exposeFunctions = exposeFunctions;
	mod_Lua_storage.findUserFunc = findUserFunc;
	mod_Lua_storage.lockState = lockState;
	mod_Lua_storage.unlockState = unlockState;
	mod_Lua_storage.pushNumber = pushNumber;
	mod_Lua_storage.pushString = pushString;
	mod_Lua_storage.pushFunctionId = pushFunctionId;
	mod_Lua_storage.exec = ml_exec;
	mod_Lua_storage.pop = ml_pop;
	mod_Lua_storage.getStringFromStack = getStringFromStack;
	mod_Lua_storage.getBooleanFromStack = getBooleanFromStack;
	mod_Lua_storage.exposeObjMethods = exposeObjMethods;
	mod_Lua_storage.initSectionSharedMethods = initSectionSharedMethods;
	mod_Lua_storage.pushSectionObject = pushSectionObject;

	registerModule( (struct Module *)&mod_Lua_storage );	/* Register the module */

	mod_Lua_storage.L = luaL_newstate();		/* opens Lua */
	luaL_openlibs( mod_Lua_storage.L );	/* and it's libraries */

	atexit(clean_lua);

	mod_Lua = &mod_Lua_storage;

		/* Expose some functions in Lua */
	exposeFunctions("Marcel", MarcelLib);
}
