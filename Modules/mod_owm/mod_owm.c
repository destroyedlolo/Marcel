/* mod_owm
 *
 * Weather forecast using OpenWeatherMap
 *  
 * This file is part of Marcel project and is following the same
 * license rules (see LICENSE file)
 *
 * 03/11/2022 - LF - Move to V8
 */

#include "mod_owm.h"

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>

struct module_owm mod_owm;

static enum RC_readconf readconf(uint8_t mid, const char *l, struct Section **section ){
	const char *arg;

	if((arg = striKWcmp(l,"APIkey="))){
		if(*section){
			publishLog('F', "RandomizeProbes can't be part of a section");
			exit(EXIT_FAILURE);
		}

		assert(( mod_owm.apikey = strdup(arg) ));

		if(cfg.verbose)
			publishLog('C', "\tAPIkey : %s", mod_owm.apikey);

		return ACCEPTED;
	} else if((arg = striKWcmp(l,"*MeteoDaily="))){
		if(findSectionByName(arg)){
			publishLog('F', "Section '%s' is already defined", arg);
			exit(EXIT_FAILURE);
		}

		struct section_OWMQuery *nsection = malloc(sizeof(struct section_OWMQuery));	/* Allocate a new section */
		initSection( (struct Section *)nsection, mid, SM_DAILY, strdup(arg));	/* Initialize shared fields */

		nsection->section.sample = DEFAULT_WEATHER_SAMPLE;
		nsection->city = NULL;
		nsection->units = "metric";
		nsection->lang = NULL;

		if(cfg.verbose)	/* Be verbose if requested */
			publishLog('C', "\tEntering Daily Weather section '%s' (%04x)", nsection->section.uid, nsection->section.id);

		*section = (struct Section *)nsection;	/* we're now in a section */
		return ACCEPTED;
	} else if(*section){
		if((arg = striKWcmp(l,"City="))){
			acceptSectionDirective(*section, "City=");
			assert(( (*(struct section_OWMQuery **)section)->city = strdup(arg) ));

			if(cfg.verbose)	/* Be verbose if requested */
				publishLog('C', "\t\tCity : '%s'", (*(struct section_OWMQuery **)section)->city);
			return ACCEPTED;
		} else if((arg = striKWcmp(l,"Units="))){
			acceptSectionDirective(*section, "Units=");
			assert(( (*(struct section_OWMQuery **)section)->units = strdup(arg) ));

			if(cfg.verbose)	/* Be verbose if requested */
				publishLog('C', "\t\tUnits : '%s'", (*(struct section_OWMQuery **)section)->units);
			return ACCEPTED;
		} else if((arg = striKWcmp(l,"Lang="))){
			acceptSectionDirective(*section, "Lang=");
			assert(( (*(struct section_OWMQuery **)section)->lang = strdup(arg) ));

			if(cfg.verbose)	/* Be verbose if requested */
				publishLog('C', "\t\tLanguage : '%s'", (*(struct section_OWMQuery **)section)->lang);
			return ACCEPTED;
		}
	}

	return REJECTED;
}

static bool acceptSDirective( uint8_t sec_id, const char *directive ){
	if(sec_id == SM_DAILY || sec_id == SM_3H){
		if( !strcmp(directive, "Disabled") )
			return true;	/* Accepted */
		else if( !strcmp(directive, "Immediate") )
			return true;	/* Accepted */
		else if( !strcmp(directive, "Retained") )
			return true;	/* Accepted */
		else if( !strcmp(directive, "Topic=") )
			return true;	/* Accepted */
		else if( !strcmp(directive, "Sample=") )
			return true;	/* Accepted */
		else if( !strcmp(directive, "City=") )
			return true;	/* Accepted */
		else if( !strcmp(directive, "Units=") )
			return true;	/* Accepted */
		else if( !strcmp(directive, "Lang=") )
			return true;	/* Accepted */
	}

	return false;
}

void InitModule( void ){
	mod_owm.module.name = "mod_owm";	/* Identify the module */

		/* Initialize callbacks
		 * It's MANDATORY that all callbacks are initialised (even by a NULL value)
		 */
	mod_owm.module.readconf = readconf;
	mod_owm.module.acceptSDirective = acceptSDirective;
	mod_owm.module.getSlaveFunction = NULL;
	mod_owm.module.postconfInit = NULL;

	mod_owm.apikey = NULL;

	register_module( (struct Module *)&mod_owm );	/* Register the module */
}
