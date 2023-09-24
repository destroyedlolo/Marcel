/* mod_freebox
 *
 *  Publish FreeboxOS figures
 *
 * This file is part of Marcel project and is following the same
 * license rules (see LICENSE file)
 *
 * 24/09/2023 - LF - First version
 */

#ifndef MOD_FREEBOX_H
#define MOD_FREEBOX_H

#include "../Marcel/Module.h"
#include "../Marcel/Section.h"

struct module_freebox {
	struct Module module;
};

struct section_freebox {
	struct Section section;
};
#endif
