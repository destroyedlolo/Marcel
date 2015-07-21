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

#include <lauxlib.h>	/* auxlib : usable hi-level function */
#include <lualib.h>		/* Functions to open libraries */

#include <stdlib.h>

#ifdef LUA
lua_State *L;

static void clean_lua(void){
	lua_close(L);
}

void init_Lua( void ){
	L = lua_open();		/* opens Lua */
	luaL_openlibs(L);	/* and it's libraries */
	atexit(clean_lua);

}
#endif
