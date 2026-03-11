/* mod_TaHoma
 *
 * Steer Overkiz' TaHoma using local API
 *
 * This file is part of Marcel project and is following the same
 * license rules (see LICENSE file)
 *
 * 24/02/2025 - LF - Creation
 */

#include "mod_TaHoma.h"
#ifdef LUA
#	include "../mod_Lua/mod_Lua.h"
#endif

#include <stdlib.h>
#include <assert.h>

struct module_TaHoma mod_TaHoma;

static enum RC_readconf readconf(uint8_t mid, const char *l, struct Section **section ){
	const char *arg;

	if(!strcmp(l, "RandomizeProbes")){
		if(*section){
			publishLog('F', "RandomizeProbes can't be part of a section");
			exit(EXIT_FAILURE);
		}

		mod_TaHoma.randomize = true;

		if(cfg.verbose)
			publishLog('C', "\tProbes are randomized");

		return ACCEPTED;
	} else if((arg = striKWcmp(l,"DefaultSampleDelay="))){
		/* No need to check if we are on not inside a section :
		 * despite this directive is a top level one, it can be placed
		 * anywhere : we don't know when a section definition
		 * is over
		 */

		mod_TaHoma.defaultsampletime = strtof(arg, NULL);

		if(cfg.verbose)
			publishLog('C', "\tDefault sample time : %f", mod_TaHoma.defaultsampletime);

		return ACCEPTED;
	} else if((arg = striKWcmp(l,"*TaHoma="))){	/* Create a new gateway */
		if(findSectionByName(arg)){
			publishLog('F', "Section '%s' is already defined", arg);
			exit(EXIT_FAILURE);
		}

		struct section_TaHoma *nsection = malloc(sizeof(struct section_TaHoma));	/* Allocate a new section */
		initSection( (struct Section *)nsection, mid, ST_TAHOMA, strdup(arg), "TaHoma");
		nsection->hostname = NULL;
		nsection->ip = NULL;
		nsection->token = NULL;
		nsection->port = 8443;
		nsection->unsafe = false;

		if(cfg.verbose)	/* Be verbose if requested */
			publishLog('C', "\tEntering TaHoma section '%s' (%04x)", nsection->section.uid, nsection->section.id);

		*section = (struct Section *)nsection;	/* we're now in a section */
		return ACCEPTED;
	} else if((arg = striKWcmp(l,"*Device="))){	/* Create a new Device */
		if(findSectionByName(arg)){
			publishLog('F', "Section '%s' is already defined", arg);
			exit(EXIT_FAILURE);
		}

		struct section_Device *nsection = malloc(sizeof(struct section_Device));	/* Allocate a new section */
		initSection( (struct Section *)nsection, mid, ST_DEVICE, strdup(arg), "Device");
		nsection->TaHoma= NULL;
		nsection->url = NULL;
		nsection->States = NULL;

		if(cfg.verbose)	/* Be verbose if requested */
			publishLog('C', "\tEntering Device section '%s' (%04x)", nsection->section.uid, nsection->section.id);

		*section = (struct Section *)nsection;	/* we're now in a section */
		return ACCEPTED;
	} else if(*section){
		if((arg = striKWcmp(l,"TaHoma_host="))){
			acceptSectionDirective(*section, "TaHoma_host=");
			assert(( (*(struct section_TaHoma **)section)->hostname = strdup(arg) ));

			if(cfg.verbose)	/* Be verbose if requested */
				publishLog('C', "\t\tHostname : '%s'", (*(struct section_TaHoma **)section)->hostname);
			return ACCEPTED;
		} else if((arg = striKWcmp(l,"TaHoma_address="))){
			acceptSectionDirective(*section, "TaHoma_address=");
			assert(( (*(struct section_TaHoma **)section)->ip = strdup(arg) ));

			if(cfg.verbose)	/* Be verbose if requested */
				publishLog('C', "\t\tAddress : '%s'", (*(struct section_TaHoma **)section)->ip);
			return ACCEPTED;
		} else if((arg = striKWcmp(l,"TaHoma_port="))){
			acceptSectionDirective(*section, "TaHoma_port=");
			assert(( (*(struct section_TaHoma **)section)->port = atol(arg) ));

			if(cfg.verbose)	/* Be verbose if requested */
				publishLog('C', "\t\tPort : %d", (*(struct section_TaHoma **)section)->port);
			return ACCEPTED;
		} else if((arg = striKWcmp(l,"TaHoma_token="))){
			acceptSectionDirective(*section, "TaHoma_token=");
			assert(( (*(struct section_TaHoma **)section)->token = strdup(arg) ));

			if(cfg.verbose)	/* Be verbose if requested */
				publishLog('C', "\t\tToken : '%s'", (*(struct section_TaHoma **)section)->token);
			return ACCEPTED;
		} else if((!strcmp(l,"DontVerifySSL"))){
			acceptSectionDirective(*section, "DontVerifySSL");
			(*(struct section_TaHoma **)section)->unsafe = true;

			if(cfg.verbose)	/* Be verbose if requested */
				publishLog('C', "\t\tDon't check SSL chain (unsafe mode)");
			return ACCEPTED;
		} else if((arg = striKWcmp(l,"TaHoma="))){
			acceptSectionDirective(*section, "TaHoma=");
			(*(struct section_Device **)section)->TaHoma = strdup(arg);
			assert((*(struct section_Device **)section)->TaHoma);

			if(cfg.verbose)	/* Be verbose if requested */
				publishLog('C', "\t\tTaHoma : '%s'", (*(struct section_Device **)section)->TaHoma);
			return ACCEPTED;
		} else if((arg = striKWcmp(l,"url="))){
			acceptSectionDirective(*section, "url=");
			(*(struct section_Device **)section)->url = strdup(arg);
			assert((*(struct section_Device **)section)->url);

			if(cfg.verbose)	/* Be verbose if requested */
				publishLog('C', "\t\tURL : '%s'", (*(struct section_Device **)section)->url);
			return ACCEPTED;
		} else if((arg = striKWcmp(l,"**State="))){	/* New state definition */
			acceptSectionDirective(*section, "**State=");

			struct State_definition *nstate = malloc(sizeof(struct State_definition));	/* Allocate a new state */
			nstate->state = strdup(arg);
			assert(nstate->state);
			nstate->disabled = false;
			nstate->dontSimulate = false;
			nstate->topic = NULL;
			nstate->retained = false;
			nstate->funcname = NULL;
			nstate->funcid = LUA_REFNIL;

			if(cfg.verbose)	/* Be verbose if requested */
				publishLog('C', "\t\tEntering State '%s'", nstate->state);

			nstate->next = (*(struct section_Device **)section)->States;
			(*(struct section_Device **)section)->States = nstate;

			return ACCEPTED;
		} else if((*(struct section_Device **)section)->States){
			/* Handling states' specific directives
			 * They MUST be refined here : mod_core's doesn't deal with
			 * the same structure.
			 */
			 struct State_definition *state = (*(struct section_Device **)section)->States;

			 if((arg = striKWcmp(l, "state_Topic="))){
				acceptSectionDirective( *section, "state_Topic=" );

				state->topic = strdup(arg);
				assert(state->topic);

				if(cfg.verbose)
					publishLog('C', "\t\t\tTopic : '%s'", state->topic);

				return ACCEPTED;
			}

#if 0
		} else if((arg = striKWcmp(l,"state="))){
			acceptSectionDirective(*section, "state=");
			(*(struct section_State **)section)->state = strdup(arg);
			assert((*(struct section_State **)section)->state);

			if(cfg.verbose)	/* Be verbose if requested */
				publishLog('C', "\t\tState: '%s'", (*(struct section_State **)section)->state);
			return ACCEPTED;
#endif
		}
	}

	return REJECTED;
}

static uint8_t customizePerSubSection( struct Section *section, uint8_t sec_id ){
	if(sec_id == ST_DEVICE && !!((struct section_Device *)section)->States)
		return ST_STATE;
	
	return sec_id;
}

static bool acceptSDirective( uint8_t sec_id, const char *directive ){
	if(sec_id == ST_TAHOMA){
		if( !strcmp(directive, "Disabled") )
			return true;	/* Accepted */
		else if( !strcmp(directive, "TaHoma_host=") )
			return true;
		else if( !strcmp(directive, "TaHoma_address=") )
			return true;
		else if( !strcmp(directive, "TaHoma_port=") )
			return true;
		else if( !strcmp(directive, "TaHoma_token=") )
			return true;
		else if( !strcmp(directive, "DontVerifySSL") )
			return true;
	} else if(sec_id == ST_DEVICE){
		if( !strcmp(directive, "Disabled") )
			return true;
		else if( !strcmp(directive, "DoNotSimulate") )
			return true;	/* Accepted */
		else if( !strcmp(directive, "Retained") )
			return true;	/* Accepted */
		else if( !strcmp(directive, "Immediate") )
			return true;
		else if( !strcmp(directive, "Sample=") )
			return true;
		else if( !strcmp(directive, "TaHoma=") )
			return true;	/* Accepted */
		else if( !strcmp(directive, "url=") )
			return true;	/* Accepted */
		else if( !strcmp(directive, "**State=") )
			return true;	/* Accepted */
	} else if(sec_id == ST_STATE){
			/* Despite they're having same goal
			 * I need to prepend with "state_"
			 * otherwise, it will be take in account by
			 * mod_core first (and then rejected).
			 */
		if( !strcmp(directive, "state_Topic=") )
			return true;	/* Accepted */
#if 0
		else if( !strcmp(directive, "Func=") )
			return true;	/* Accepted */
#endif
	}
	return false;
}


static ThreadedFunctionPtr getSlaveFunction(uint8_t sid){
/*
	if(sid == ST_STATE)
		return processWFDaily;
*/
	return NULL;
}

void InitModule( void ){
	initModule((struct Module *)&mod_TaHoma, "mod_TaHoma");	/* Identify the module */

		/* Initialize callbacks
		 * It's MANDATORY that all callbacks are initialised
		 */
	mod_TaHoma.module.readconf = readconf;
	mod_TaHoma.module.customeSID = customizePerSubSection;
	mod_TaHoma.module.acceptSDirective = acceptSDirective;
	mod_TaHoma.module.getSlaveFunction = getSlaveFunction;

	mod_TaHoma.randomize = false;
	mod_TaHoma.defaultsampletime = 0.0;

	registerModule( (struct Module *)&mod_TaHoma );	/* Register the module */

#ifdef LUA
	if(mod_Lua){ /* Is mod_Lua loaded ? */

			/* Expose shared methods */

			/* Expose mod_owm's own function */
	}
#endif
}
