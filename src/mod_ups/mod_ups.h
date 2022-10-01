/* mod_ups.h
 *
 * Retrieves UPS figures through its NUT server
 *
 * This file is part of Marcel project and is following the same
 * license rules (see LICENSE file)
 *
 * 29/09/2022 - LF - First version
 */

#ifndef MOD_UPS_H
#define MOD_UPS_H

#include "../Module.h"
#include "../Section.h"

struct module_ups {
	struct Module module;
};


struct var {	/* Storage for var list */
	struct var *next;
	const char *name;
};

struct section_ups {
	struct Section section;

	const char *host;	/* NUT server */
	uint16_t port;		/* NUT server */

	struct var *var_list;	/* List of variables to read */
};

#endif
