/* Marcel's core module
 *
 * 	08/09/2022 - LF - First version
 */

#include "module_core.h"

struct module_Core {
	struct module module;
} mod_Core;

static bool mc_readconf(const char *l){
	return false;
}

void init_module_core(){
	mod_Core.module.name = "mod_core";
	mod_Core.module.readconf = mc_readconf;

	register_module( (struct module *)&mod_Core );
}

