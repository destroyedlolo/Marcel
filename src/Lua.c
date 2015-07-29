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
		if(verbose)
			fprintf(stderr, "*E* \"%s\" is not a function, a %s.\n", fn, lua_typename(L, lua_type(L, -1)) );
		lua_pop(L, 1);
		return LUA_REFNIL;
	}

	int ref = luaL_ref(L,LUA_REGISTRYINDEX);	/* Get the reference to the function */

	return ref;
}

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

		pthread_mutex_init( &onefunc, NULL);
	} else if(verbose)
		puts("*W* No FuncScript defined, Lua disabled");
}
#endif
