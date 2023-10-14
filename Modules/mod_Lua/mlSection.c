/* mlSection
 *
 * Lua's abstraction of section
 *
 * This file is part of Marcel project and is following the same
 * license rules (see LICENSE file)
 *
 * 14/10/2023 - LF - First version
 */

#include "mod_Lua.h"
#include "mlSection.h"

static int mls_getUID(lua_State *L){
	struct Section **s = luaL_testudata(L, 1, "mlSection");

	if(!s)
		luaL_error(L, "Not a Section");

	lua_pushstring(L, (*s)->uid);

	return 1;
}

static const struct luaL_Reg mlSectionM[] = {
	{"getUID", mls_getUID},
	{"getName", mls_getUID},
	{NULL, NULL}
};

/* Initialize Lua's sections exposition */
void mlSectionInit(lua_State *L){
	exposeObjFunctions(L, "mlSection", mlSectionM);
}

/* Create a Lua's section object */
void mlSectionPush(lua_State *L, struct Section *obj){
	struct Section **s = (struct Section **)lua_newuserdata(L, sizeof(struct Section *));
	if(!s)
		luaL_error(L, "No memory");

	*s = obj;

	luaL_getmetatable(L, "mlSection");
	lua_setmetatable(L, -2);
}
