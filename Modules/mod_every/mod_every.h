/* mod_every
 *
 * Repeating tasks
 *
 */

#ifndef MOD_EVERY_H
#define MOD_EVERY_H

#include "../Marcel/Module.h"
#include "../Marcel/Section.h"

	/* **
	 * own structures 
	 *
	 * For the moment, no customisation needed but custom strutures
	 * prepares the future :)
	 *
	 * **/
struct module_every {
	struct Module module;

};

struct section_every {
	struct Section section;
};

struct section_at {
	struct Section section;
	bool runIfOver;
};

#endif
