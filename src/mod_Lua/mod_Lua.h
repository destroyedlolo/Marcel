/* mod_Lua
 *
 * Add Lua's plugins to Marcel
 *
 * 21/09/2022 - LF - First version
 */

#ifndef MOD_LUA_H
#define MOD_LUA_H

#include "../Module.h"

#include <lauxlib.h>	/* auxlib : usable hi-level function */
#include <lualib.h>		/* Functions to open libraries */

struct module_Lua {
	struct Module module;

	lua_State *L;
	pthread_mutex_t onefunc;	/* As using a shared state, only one func can run at a time */

	const char *script;			/* Script to load */

	/* Callbacks */
	int (*exposeFunctions)(const char *name, const struct luaL_Reg *funcs);	/* Expose Lua functions */
};

#endif
