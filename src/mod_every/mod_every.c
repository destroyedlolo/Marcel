/* mod_every
 *
 * Repeating tasks
 *
 * This file is part of Marcel project and is following the same
 * license rules (see LICENSE file)
 *
 * 07/09/2015	- LF - First version
 * 17/05/2016	- LF - Add 'Immediate'
 * 06/06/2016	- LF - If sample is null, stop 
 * 20/08/2016	- LF - Prevent a nasty bug making system to crash if 
 * 		user function lookup is failing
 * 					----
 * 24/09/2022	- LF - Move to V8
 */

#include "mod_every.h"

#ifdef LUA
#	include "../mod_Lua/mod_Lua.h"
#else
#	error "mod_every is useless without Lua"
#endif

#include <stdlib.h>
#include <string.h>

static struct module_every mod_every;

	/* Section identifier */
enum {
	ST_EVERY = 0
};

/* ***
 * Module interface
 * ***/
static enum RC_readconf readconf(uint8_t mid, const char *l, struct Section **section ){
	const char *arg;	/* Argument of the configuration directive */

	if((arg = striKWcmp(l,"*Every="))){	/* Starting a section definition */
		if(findSectionByName(arg)){	/* Name must be unique */
			publishLog('F', "Section '%s' is already defined", arg);
			exit(EXIT_FAILURE);
		}

		struct section_every *nsection = malloc(sizeof(struct section_every));
		initSection( (struct Section *)nsection, mid, ST_EVERY, "Every");

		if(cfg.verbose)	/* Be verbose if requested */
			publishLog('C', "\tEntering section '%s' (%04x)", nsection->section.uid, nsection->section.id);

		*section = (struct Section *)nsection;
		return ACCEPTED;
	}

	 return REJECTED;
}

void InitModule( void ){
	mod_every.module.name = "mod_every";

	mod_every.module.readconf = readconf;
	mod_every.module.acceptSDirective = NULL;
	mod_every.module.getSlaveFunction = NULL;
	mod_every.module.postconfInit = NULL;

	register_module( (struct Module *)&mod_every );
}
