/* mod_freebox
 *
 *  Publish Freebox v4/v5 figures (French Internet Service Provider)
 *
 * This file is part of Marcel project and is following the same
 * license rules (see LICENSE file)
 *
 * 01/10/2022 - LF - First version
 */

#ifndef MOD_FREEBOX_H
#define MOD_FREEBOX_H

#include "../Module.h"
#include "../Section.h"

struct module_freebox {
	struct Module module;
};

struct section_freebox {
	struct Section section;
};
#endif
