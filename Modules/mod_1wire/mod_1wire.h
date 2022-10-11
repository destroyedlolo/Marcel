/* mod_1wire
 *
 * Handle 1-wire probes.
 * (also suitable for other exposed as a file values)
 *
 * This file is part of Marcel project and is following the same
 * license rules (see LICENSE file)
 *
 * 11/10/2022 - LF - First version
 */

#ifndef MOD_1WIRE_H
#define MOD_1WIRE_H

#include "../Marcel/Module.h"
#include "../Marcel/Section.h"

/* Custom structure to store module's configuration */
struct module_1wire {
	struct Module module;

	bool randomize;		/* Randomize probes to avoid they are all launched at the same time */
};

#endif
