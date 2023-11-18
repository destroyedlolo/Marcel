/* mlSection
 *
 * Lua's abstraction of section
 *
 * This file is part of Marcel project and is following the same
 * license rules (see LICENSE file)
 *
 * 14/10/2023 - LF - First version
 */

#ifndef MLSECTION_H
#define MLSECTION_H

#include "../Marcel/Section.h"

#include <lauxlib.h>	/* auxlib : usable hi-level function */

extern void initSectionSharedMethods(lua_State *, const char *);
extern void pushSectionObject(lua_State *, struct Section *);

#endif
