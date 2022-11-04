/* mod_OnOff
 *
 * Enable or disable a section
 *
 * This file is part of Marcel project and is following the same
 * license rules (see LICENSE file)
 *
 * NOTEZ-BIEN : this module doesn't define any directive or section
 */

#include "mod_OnOff.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>

static struct module_OnOff mod_OnOff;

void InitModule( void ){
	initModule((struct Module *)&mod_OnOff, "mod_OnOff");

	mod_OnOff.module.readconf = NULL;
	mod_OnOff.module.acceptSDirective = NULL;
	mod_OnOff.module.getSlaveFunction = NULL;
	mod_OnOff.module.postconfInit = NULL;
	mod_OnOff.module.processMsg = NULL;

	registerModule( (struct Module *)&mod_OnOff );
}
