/* mod_1wire
 *
 * Handle 1-wire probes.
 * (also suitable for other exposed as a file values)
 *
 * This file is part of Marcel project and is following the same
 * license rules (see LICENSE file)
 *
 * 11/10/2022 - LF - First version
 */

#include "mod_1wire.h"	/* module's own stuffs */
#ifdef LUA
#	include "../mod_Lua/mod_Lua.h"
#endif
#include "../Marcel/MQTT_tools.h"

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>

static struct module_1wire mod_1wire;

enum {
	ST_FFV= 0,
};

static enum RC_readconf readconf(uint8_t mid, const char *l, struct Section **section ){
	const char *arg;

	if(!strcmp(l, "RandomizeProbes")){
		if(*section){
			publishLog('F', "TestFlag can't be part of a section");
			exit(EXIT_FAILURE);
		}

		mod_1wire.randomize = true;

		if(cfg.verbose)
			publishLog('C', "\tProbes are randomized");

		return ACCEPTED;
	} else if((arg = striKWcmp(l,"*FFV="))){
		if(findSectionByName(arg)){
			publishLog('F', "Section '%s' is already defined", arg);
			exit(EXIT_FAILURE);
		}

		struct section_FFV *nsection = malloc(sizeof(struct section_FFV));	/* Allocate a new section */
		initSection( (struct Section *)nsection, mid, ST_FFV, strdup(arg));	/* Initialize shared fields */

		nsection->file = NULL;
		nsection->latch = NULL;
		nsection->offset = 0.0;
		nsection->safe85 = false;

		if(cfg.verbose)	/* Be verbose if requested */
			publishLog('C', "\tEntering FFV section '%s' (%04x)", nsection->section.uid, nsection->section.id);

		*section = (struct Section *)nsection;	/* we're now in a section */
		return ACCEPTED;
	} else if(*section){
		if((arg = striKWcmp(l,"File="))){
			assert(( (*(struct section_FFV **)section)->file = strdup(arg) ));

			if(cfg.verbose)	/* Be verbose if requested */
				publishLog('C', "\t\tFile : '%s'", (*(struct section_FFV **)section)->file);
			return ACCEPTED;
		} else if((arg = striKWcmp(l,"Offset="))){
			(*(struct section_FFV **)section)->offset = strtof(arg, NULL);

			if(cfg.verbose)	/* Be verbose if requested */
				publishLog('C', "\t\tOffset: %f", (*(struct section_FFV **)section)->offset);
			return ACCEPTED;
		}
	}

	return REJECTED;
}

static bool mh_acceptSDirective( uint8_t sec_id, const char *directive ){
	if(sec_id == ST_FFV){
		if( !strcmp(directive, "Disabled") )
			return true;	/* Accepted */
		else if( !strcmp(directive, "Immediate") )
			return true;	/* Accepted */
		else if( !strcmp(directive, "Keep") )
			return true;	/* Accepted */
		else if( !strcmp(directive, "Retained") )
			return true;	/* Accepted */
		else if( !strcmp(directive, "Sample=") )
			return true;	/* Accepted */
		else if( !strcmp(directive, "Topic=") )
			return true;	/* Accepted */
		else if( !strcmp(directive, "Func=") )
			return true;	/* Accepted */
		else if( !strcmp(directive, "File=") )
			return true;	/* Accepted */
		else if( !strcmp(directive, "Offset=") )
			return true;	/* Accepted */
	}

	return false;
}

void InitModule( void ){
	mod_1wire.module.name = "mod_1wire";	/* Identify the module */

		/* Initialize callbacks
		 * It's MANDATORY that all callbacks are initialised (even by a NULL value)
		 */
	mod_1wire.module.readconf = readconf;
	mod_1wire.module.acceptSDirective = mh_acceptSDirective;
	mod_1wire.module.getSlaveFunction = NULL;
	mod_1wire.module.postconfInit = NULL;

	mod_1wire.randomize = false;

	register_module( (struct Module *)&mod_1wire );	/* Register the module */
}
