/* mod_freebox
 *
 *  Publish FreeboxOS figures
 *
 * This file is part of Marcel project and is following the same
 * license rules (see LICENSE file)
 *
 * 24/09/2023 - LF - First version
 */

#include "mod_freebox.h"
#include "../Marcel/MQTT_tools.h"

static struct module_freebox mod_freebox;

enum {
	SFB_FREEBOX = 0
};

void InitModule( void ){
	initModule((struct Module *)&mod_freebox, "mod_freebox");

#if 0
	mod_freebox.module.readconf = readconf;
	mod_freebox.module.acceptSDirective = mfb_acceptSDirective;
	mod_freebox.module.getSlaveFunction = mfb_getSlaveFunction;
#endif

	registerModule( (struct Module *)&mod_freebox );
}
