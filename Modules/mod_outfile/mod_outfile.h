/* mod_outfile
 *
 * Write value to file
 *
 * This file is part of Marcel project and is following the same
 * license rules (see LICENSE file)
 *
 */

#ifndef MOD_OUTFILE_H
#define MOD_OUTFILE_H

#include "../Marcel/Module.h"
#include "../Marcel/Section.h"

struct module_outfile {
	struct Module module;
};

struct section_outfile {
	struct Section section;

	const char *file;	/* File to write to */

	bool inerror;
};

#endif
