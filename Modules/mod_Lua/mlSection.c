/* mlSection
 *
 * Lua's abstraction of section
 *
 * This file is part of Marcel project and is following the same
 * license rules (see LICENSE file)
 *
 * 14/10/2023 - LF - First version
 *
 *	Notez-bien : We don't have to check the section kind as methods are
 * common to all sections.
 */

#include "mod_Lua.h"
#include "mlSection.h"

static int mls_getUID(lua_State *L){
	struct Section **s = lua_touserdata(L,1);
	luaL_argcheck(L, s != NULL, 1, "Not a Section");
	
	lua_pushstring(L, (*s)->uid);

	return 1;
}

static int mls_getKind(lua_State *L){
	struct Section **s = lua_touserdata(L,1);
	if(!s)
		luaL_error(L, "Not a Section");
	
	lua_pushstring(L, (*s)->kind);

	return 1;
}

static int mls_isEnabled(lua_State *L){
	struct Section **s = lua_touserdata(L,1);
	if(!s)
		luaL_error(L, "Not a Section");
	
	lua_pushboolean(L, !(*s)->disabled);

	return 1;
}

static int mls_getCustomFigures(lua_State *L){
	struct Section **s = lua_touserdata(L,1);
	if(!s)
		luaL_error(L, "Not a Section");

	if((*s)->publishCustomFigures)
		return ((*s)->publishCustomFigures(*s));
	else
		return 0;
}

static const struct luaL_Reg mlSectionM[] = {
	{"getUID", mls_getUID},
	{"getName", mls_getUID},
	{"getKind", mls_getKind},
	{"isEnabled", mls_isEnabled},
	{"getCustomFigures", mls_getCustomFigures},
	{NULL, NULL}
};

/* Initialize Lua's sections exposition */
void initSectionSharedMethods(lua_State *L, const char *objName){
	mod_Lua->exposeObjMethods(L, objName, mlSectionM);
}

/* Create a Lua's section object */
void pushSectionObject(lua_State *L, struct Section *obj){
	struct Section **s = (struct Section **)lua_newuserdata(L, sizeof(struct Section *));
	if(!s)
		luaL_error(L, "No memory");

	*s = obj;

	luaL_getmetatable(L, obj->kind);
	lua_setmetatable(L, -2);
}
