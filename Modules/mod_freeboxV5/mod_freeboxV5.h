/* mod_freeboxV5
 *
 *  Publish Freebox v4/v5 figures (French Internet Service Provider)
 *
 * This file is part of Marcel project and is following the same
 * license rules (see LICENSE file)
 *
 * 01/10/2022 - LF - First version
 * 20/09/2023 - LF - Rename to V5
 */

#ifndef MOD_FREEBOXV5_H
#define MOD_FREEBOXV5_H

#include "../Marcel/Module.h"
#include "../Marcel/Section.h"

struct module_freeboxV5 {
	struct Module module;
};

struct section_freeboxV5 {
	struct Section section;

	bool inerror;
};
#endif
