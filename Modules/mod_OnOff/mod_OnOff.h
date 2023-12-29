/* mod_OnOff
 *
 * Enable or disable a section
 *
 * This file is part of Marcel project and is following the same
 * license rules (see LICENSE file)
 *
 */

#ifndef MOD_ONOFF_H
#define MOD_ONOFF_H

#include "../Marcel/Module.h"
#include "../Marcel/Section.h"

struct module_OnOff {
	struct Module module;

	char *topic;
	char *NNtopic;
};

#endif
