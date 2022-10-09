/* mod_sht31
 *
 * Exposes SHT31 (Temperature/humidity probe) figures.
 *
 * This file is part of Marcel project and is following the same
 * license rules (see LICENSE file)
 *
 * 09/10/2022 - LF - First version
 */

#include "mod_sht31.h"	/* module's own stuffs */
#ifdef LUA
#	include "../mod_Lua/mod_Lua.h"
#endif

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static struct module_sht31 mod_sht31;

enum {
	ST_SHT31= 0,
};

static enum RC_readconf readconf(uint8_t mid, const char *l, struct Section **section ){
	const char *arg;

	if((arg = striKWcmp(l,"*SHT31="))){	/* Starting a section definition */
		if(findSectionByName(arg)){
			publishLog('F', "Section '%s' is already defined", arg);
			exit(EXIT_FAILURE);
		}

		struct section_sht31 *nsection = malloc(sizeof(struct section_sht31));	/* Allocate a new section */
		initSection( (struct Section *)nsection, mid, ST_SHT31, strdup(arg));	/* Initialize shared fields */

		if(cfg.verbose)	/* Be verbose if requested */
			publishLog('C', "\tEntering section '%s' (%04x)", nsection->section.uid, nsection->section.id);

		*section = (struct Section *)nsection;	/* we're now in a section */
		return ACCEPTED;
	}

	return REJECTED;
}

void InitModule( void ){
	mod_sht31.module.name = "mod_sht31";	/* Identify the module */

		/* Initialize callbacks
		 * It's MANDATORY that all callbacks are initialised (even by a NULL value)
		 */
	mod_sht31.module.readconf = readconf;
	mod_sht31.module.acceptSDirective = NULL;
	mod_sht31.module.getSlaveFunction = NULL;
	mod_sht31.module.postconfInit = NULL;

	register_module( (struct Module *)&mod_sht31 );	/* Register the module */
}
