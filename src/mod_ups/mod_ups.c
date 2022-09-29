/* mod_ups.h
 *
 * This "fake" module only shows how to create a module for Marcel
 *
 * This file is part of Marcel project and is following the same
 * license rules (see LICENSE file)
 *
 * 29/09/2022 - LF - First version
 */

#include "mod_ups.h"	/* module's own stuffs */
#ifdef LUA
#	include "../mod_Lua/mod_Lua.h"
#endif

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static struct module_ups mod_ups;

enum {
	ST_UPS= 0
};

static enum RC_readconf readconf(uint8_t mid, const char *l, struct Section **section ){
	const char *arg;

	if((arg = striKWcmp(l,"*UPS="))){	/* Starting a section definition */
		if(findSectionByName(arg)){
			publishLog('F', "Section '%s' is already defined", arg);
			exit(EXIT_FAILURE);
		}

		struct section_ups *nsection = malloc(sizeof(struct section_ups));
		initSection( (struct Section *)nsection, mid, ST_UPS, strdup(arg));

		nsection->host = NULL;
		nsection->port = 0;
		nsection->var_list = NULL;

		if(cfg.verbose)	/* Be verbose if requested */
			publishLog('C', "\tEntering section '%s' (%04x)", nsection->section.uid, nsection->section.id);

		*section = (struct Section *)nsection;	/* we're now in a section */
		return ACCEPTED;
	}

	return REJECTED;
}

void InitModule( void ){
	mod_ups.module.name = "mod_ups";

	mod_ups.module.readconf = readconf;
	mod_ups.module.acceptSDirective = NULL;
	mod_ups.module.getSlaveFunction = NULL;
	mod_ups.module.postconfInit = NULL;

	register_module( (struct Module *)&mod_ups );
}