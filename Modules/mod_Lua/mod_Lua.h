/* mod_Lua
 *
 * Add Lua's plugins to Marcel
 *
 * This file is part of Marcel project and is following the same
 * license rules (see LICENSE file)
 *
 * 21/09/2022 - LF - First version
 */

#ifndef MOD_LUA_H
#define MOD_LUA_H

#include "../Marcel/Module.h"
#include "../Marcel/Section.h"

#include <lauxlib.h>	/* auxlib : usable hi-level function */
#include <lualib.h>		/* Functions to open libraries */


struct module_Lua {
	struct Module module;

	lua_State *L;
	pthread_mutex_t onefunc;	/* As using a shared state, only one func can run at a time */

	const char *script;			/* Script to load */

	struct Section *psection;	/* section iterator */

	/* ***
	 * Callbacks
	 * ***/

		/* Expose Lua functions */
	int (*exposeFunctions)(const char *name, const struct luaL_Reg *funcs);
		/* Find identifier of an user function */
	int (*findUserFunc)(const char *name);
		/* locking */
	void (*lockState)(void);
	void (*unlockState)(void);
		/* Push value on state */
	void (*pushNumber)(const double val);
	void (*pushString)(const char *val);
		/* Push on state a function by its ID */
	void (*pushFunctionId)(int functionid);
		/* Exec a function */
	int (*exec)(int narg, int nret);
		/* Pop a value from stack */
	void (*pop)(int idx);
		/* get a string from stack */
	const char *(*getStringFromStack)(int idx);
		/* get a boolean from stack */
	bool (*getBooleanFromStack)(int idx);
		/* expose methods to an object */
	int (*exposeObjMethods)(lua_State *, const char *, const struct luaL_Reg *);

		/* Expose section shared methods to a Lua's object */
	void (*initSectionSharedMethods)(lua_State *, const char *);
	void (*pushSectionObject)(lua_State *, struct Section *);

		/* Compatibility */
#	if LUA_VERSION_NUM <= 501
	void *(*testudata)(lua_State *, int, const char *);
#	endif
};

extern const struct luaL_Reg MarcelLib [];

#	if LUA_VERSION_NUM <= 501
#		define	luaL_testudata (mod_Lua->testudata)
#	endif
#endif
