/* mod_dummy
 *
 * This "fake" module only shows how to create a module for Marcel
 *
 * This file is part of Marcel project and is following the same
 * license rules (see LICENSE file)
 *
 * 13/09/2022 - LF - First version
 */

#ifndef MOD_DUMMY_H
#define MOD_DUMMY_H

/* Include shared modules definitions and utilities */
#include "../Module.h"
#include "../Section.h"

/* Custom structure to store module's configuration */
struct module_dummy {
		/* The first field MUST BE a "struct Module" which is containing
		 * all stuffs allowing the interface between Marcel's core and the
		 * module itself. 
		 */
	struct Module module;

		/* configuration variables
		 * They are dedicated and only available inside this module.
		 */
	int test;	/* variable containing interesting stuffs for the module */
	bool flag;
};


/* Custom structure to store a section handled by this module.
 * In this example, only one section but nothing prevent to have several :
 * in such case, they MUST have unique uid.
 */
struct section_dummy {
		/* The first field MUST BE a "struct Section" which is containing
		 * all stuffs allowing the interface between Marcel's core and the
		 * section itself. 
		 */
	struct Section section;

		/* Variables dedicated to this structure */
	int dummy;
};

#endif
