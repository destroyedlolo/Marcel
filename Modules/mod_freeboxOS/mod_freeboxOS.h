/* mod_freeboxOS
 *
 *  Publish FreeboxOS figures
 *
 * This file is part of Marcel project and is following the same
 * license rules (see LICENSE file)
 *
 * 24/09/2023 - LF - First version
 */

#ifndef MOD_FREEBOXOS_H
#define MOD_FREEBOXOS_H

#include "../Marcel/Module.h"
#include "../Marcel/Section.h"

struct module_freeboxOS {
	struct Module module;
};

struct section_freeboxOS {
	struct Section section;

	const char *url;
	const char *app_token;
};
#endif
