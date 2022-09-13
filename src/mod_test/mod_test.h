/* mod_test
 *
 * This "fake" module only shows how to create a module for Marcel
 *
 * 13/09/2022 - LF - First version
 */

#ifndef MOD_TEST_H
#define MOD_TEST_H

/* Include shared modules definitions and utilities */
#include "../module.h"

/* Custom structure to store module's configuration */
struct module_test {
		/* The first field MUST BE a "struct module" which is containing
		 * all stuffs allowing the interface between Marcel's core and the
		 * module itself. 
		 */
	struct module module;

		/* configuration variables
		 * They are dedicated and only available inside this module.
		 */
	int test;	/* variable containing interesting stuffs for the module */
	bool flag;
};

#endif
