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
#include <assert.h>

static struct module_ups mod_ups;

enum {
	ST_UPS= 0
};

static enum RC_readconf readconf(uint8_t mid, const char *l, struct Section **asection ){
	struct section_ups **section = (struct section_ups **)asection;
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

		*section = nsection;	/* we're now in a section */
		return ACCEPTED;
	} else if(*section){
		if((arg = striKWcmp(l,"Host="))){
			(*section)->host = strdup(arg);
			assert( (*section)->host );

			if(cfg.verbose)
				publishLog('C', "\t\tNUT's host : '%s'", (*section)->host);

			return ACCEPTED;
		} else if((arg = striKWcmp(l,"Port="))){
			(*section)->port = atoi(arg);

			if(cfg.verbose)
				publishLog('C', "\t\tNUT's port : %u", (*section)->port);

			return ACCEPTED;
		} else if((arg = striKWcmp(l,"Var="))){
			struct var *v = malloc(sizeof(struct var));	/* New variable */
			assert(v);
			assert( (v->name = strdup( arg )) );

			v->next = (*section)->var_list;	/* add it in the list */
			(*section)->var_list = v;

			if(cfg.verbose)
				publishLog('C', "\t\tVar : '%s'", v->name);

			return ACCEPTED;
		}
	}

	return REJECTED;
}

static bool mu_acceptSDirective( uint8_t sec_id, const char *directive ){
	if(sec_id == ST_UPS){
		if( !strcmp(directive, "Disabled") )
			return true;
		else if( !strcmp(directive, "Sample=") )
			return true;
		else if( !strcmp(directive, "Topic=") )
			return true;
		else if( !strcmp(directive, "Host=") )
			return true;
		else if( !strcmp(directive, "Port=") )
			return true;
		else if( !strcmp(directive, "Var=") )
			return true;
		else if( !strcmp(directive, "Keep") )
			return true;
	}

	return false;
}

void InitModule( void ){
	mod_ups.module.name = "mod_ups";

	mod_ups.module.readconf = readconf;
	mod_ups.module.acceptSDirective = mu_acceptSDirective;
	mod_ups.module.getSlaveFunction = NULL;
	mod_ups.module.postconfInit = NULL;

	register_module( (struct Module *)&mod_ups );
}
