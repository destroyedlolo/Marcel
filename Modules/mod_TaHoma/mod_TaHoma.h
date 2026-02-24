/* mod_TaHoma
 *
 * Steer Overkiz' TaHoma using local API
 *
 * This file is part of Marcel project and is following the same
 * license rules (see LICENSE file)
 *
 * 24/02/2025 - LF - Creation
 */

#ifndef MOD_TAHOMA_H
#define MOD_TAHOMA_H

#include "../Marcel/Module.h"
#include "../Marcel/Section.h"

/* Custom structure to store module's configuration */
struct module_TaHoma {
	struct Module module;

		/* Gateway's */
	const char *hostname;
	const char *ip;
	const char *token;
	uint16_t port;

	bool unsafe;	/* Don't verify SSL chain */
};

extern struct module_TaHoma mod_TaHoma;

#endif
