/* mod_dpd.h
 *
 * 	Dead Publisher Detector
 * 	Publish a message if a figure doesn't come on time
 *
 * This file is part of Marcel project and is following the same
 * license rules (see LICENSE file)
 *
 * 06/10/2022 - LF - Migrated to v8
 */

#include "mod_dpd.h"
#ifdef LUA
#	include "../mod_Lua/mod_Lua.h"
#endif

static struct module_dpd mod_dpd;

enum {
	SD_DPD = 0
};

static enum RC_readconf readconf(uint8_t mid, const char *l, struct Section **section ){
	return REJECTED;
}

void InitModule( void ){
	mod_dpd.module.name = "mod_dpd";

	mod_dpd.module.readconf = readconf;
	mod_dpd.module.acceptSDirective = NULL;
	mod_dpd.module.getSlaveFunction = NULL;
	mod_dpd.module.postconfInit = NULL;

	register_module( (struct Module *)&mod_dpd );	/* Register the module */
}
