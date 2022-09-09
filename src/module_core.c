/* Marcel's core module
 *
 * 	08/09/2022 - LF - First version
 */

#include "module_core.h"

#include <stddef.h>
#include <sys/types.h>
#include <unistd.h>
#include <assert.h>

#include <stdio.h>
#include <string.h>

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


		/* initialise configuration */

	assert(sizeof(pid_t) == 4);	/* Otherwise, the size of defaultCID need to be changed */
	static char defaultCID[16];
	sprintf(defaultCID, "Marcel.%x", getpid());

	cfg.Broker = "tcp://localhost:1883";
	cfg.ClientID = defaultCID;
	cfg.client = NULL;
	cfg.ConLostFatal = false;

#ifdef DEBUG
	if(cfg.debug)
		printf("*d* default ClientID : '%s'\n", defaultCID);
#endif
}
