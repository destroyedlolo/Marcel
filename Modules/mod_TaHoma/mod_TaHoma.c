/* mod_TaHoma
 *
 * Steer Overkiz' TaHoma using local API
 *
 * This file is part of Marcel project and is following the same
 * license rules (see LICENSE file)
 *
 * 24/02/2025 - LF - Creation
 * 25/03/2026 - LF - Redesign to optimize
 */

#include "mod_TaHoma.h"
#ifdef LUA
#	include "../mod_Lua/mod_Lua.h"
#endif

#include <curl/curl.h>
#include <stdlib.h>
#include <assert.h>

struct module_TaHoma mod_TaHoma;

static void initTaHoma(struct Section *asec){
	struct section_TaHoma *s = (struct section_TaHoma *)asec;

	s->section.inerror = true;	/* By default, we're in error */

		/* Sanity check */
	if(!s->hostname || !s->ip || !s->token){
		publishLog('E', "[%s] TaHoma misses some parameters", s->section.uid);
		return;
	}

		/* Build the url */
	s->url_len = strlen("https://:/enduser-mobile-web/1/enduserAPI/");
	s->url_len += strlen(s->ip);
	s->url_len += 5; /* port: 65535 */

	if(!(s->baseurl = malloc(s->url_len + 1))){
		publishLog('E', "[%s] Out of memory", s->section.uid);
		return;
	}
	sprintf(s->baseurl, "https://%s:%u/enduser-mobile-web/1/enduserAPI/", s->ip, s->port);
	s->url_len = strlen(s->baseurl);	/* Because the port length is unknown */

	if(cfg.debug)
		publishLog('d', "[%s] URL set to \"%s\"", s->section.uid, s->baseurl);

	s->section.inerror = false;	/* Initialisation completed */
}

static void initProbe(struct Section *asec){
	struct section_Probe *s = (struct section_Probe *)asec;

	s->device.section.inerror = true;	/* By default, we're in error */

		/* Sanity check */
	if(!s->device.TaHoma){
		publishLog('E', "[%s] No TaHoma defined", s->device.section.uid);
		return;
	}
	s->device.gateway = (struct section_TaHoma *)findSectionByName(s->device.TaHoma);
	if(!s->device.gateway || strcmp(s->device.gateway->section.kind, "TaHoma")){
		publishLog('E', "[%s] TaHoma \"%s\" not found", s->device.section.uid, s->device.TaHoma);
		return;
	}
	if(s->device.gateway->section.inerror){
		publishLog('E', "[%s] TaHoma \"%s\" is not configured", s->device.section.uid, s->device.TaHoma);
		return;
	}

	if(!s->device.url){
		publishLog('E', "[%s] No URL defined", s->device.section.uid);
		return;
	}

	if(!s->States){
		publishLog('E', "[%s] No state defined", s->device.section.uid);
		return;
	}

	for(struct State_definition *st = s->States; st; st = st->next){ /* states' sanity */
		if(!st->topic){
			publishLog('F', "[%s] State \"%s\" has no topic defined", s->device.section.uid, st->state);
			return;
		}
	}

		/* Building URL */
	CURL *curl = curl_easy_init();
	if(!curl){
		publishLog('E', "[%s] Curl init failed", s->device.section.uid);
		return;
	}
	
	char *enc = curl_easy_escape(curl, s->device.url, 0);
	if(!enc){
		publishLog('E', "[%s] curl_easy_escape failed", s->device.section.uid);
		curl_easy_cleanup(curl);
		return;
	}

	s->device.target_url = malloc(
		( 
			s->device.gateway->url_len +
			strlen("setup/devices//states") +
			strlen(enc)
		) +1);

	if(!s->device.target_url){
		publishLog('E', "[%s] No memory", s->device.section.uid);
		curl_free(enc);
		curl_easy_cleanup(curl);
		return;
	}
	sprintf((char *)s->device.target_url, "%ssetup/devices/%s/states", s->device.gateway->baseurl, enc);

	curl_free(enc);
	curl_easy_cleanup(curl);

	if(cfg.debug)
		publishLog('d', "[%s] url : \"%s\"", s->device.section.uid, s->device.target_url);
		
	s->device.section.inerror = false;	/* Initialisation completed */
}

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
	} else if((arg = striKWcmp(l,"*Probe="))){	/* Create a new probe */
		if(findSectionByName(arg)){
			publishLog('F', "Section '%s' is already defined", arg);
			exit(EXIT_FAILURE);
		}

		struct section_Probe *nsection = malloc(sizeof(struct section_Probe));	/* Allocate a new section */
		initSection( (struct Section *)nsection, mid, ST_PROBE, strdup(arg), "Probe");
		nsection->device.TaHoma= NULL;
		nsection->device.url = NULL;
		nsection->States = NULL;
		nsection->device.section.postconfInit = initProbe;
		nsection->device.section.sample = mod_TaHoma.defaultsampletime;

		if(cfg.verbose)	/* Be verbose if requested */
			publishLog('C', "\tEntering Probe section '%s' (%04x)", nsection->device.section.uid, nsection->device.section.id);

		*section = (struct Section *)nsection;	/* we're now in a section */
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
		nsection->section.postconfInit = initTaHoma;

		if(cfg.verbose)	/* Be verbose if requested */
			publishLog('C', "\tEntering TaHoma section '%s' (%04x)", nsection->section.uid, nsection->section.id);

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
			(*(struct section_Probe **)section)->device.TaHoma = strdup(arg);
			assert((*(struct section_Probe **)section)->device.TaHoma);

			if(cfg.verbose)	/* Be verbose if requested */
				publishLog('C', "\t\tTaHoma : '%s'", (*(struct section_Probe **)section)->device.TaHoma);
			return ACCEPTED;
		} else if((arg = striKWcmp(l,"url="))){
			acceptSectionDirective(*section, "url=");
			(*(struct section_Probe **)section)->device.url = strdup(arg);
			assert((*(struct section_Probe **)section)->device.url);

			if(cfg.verbose)	/* Be verbose if requested */
				publishLog('C', "\t\tURL : '%s'", (*(struct section_Probe **)section)->device.url);
			return ACCEPTED;
		}
	}

	return REJECTED;
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
	} else if(sec_id == ST_PROBE){
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
	}

	return false;
}

void InitModule( void ){
	initModule((struct Module *)&mod_TaHoma, "mod_TaHoma");	/* Identify the module */

		/* Initialize callbacks
		 * It's MANDATORY that all callbacks are initialised
		 */
	mod_TaHoma.module.readconf = readconf;
	mod_TaHoma.module.acceptSDirective = acceptSDirective;
/*
	mod_TaHoma.module.customeSID = customizePerSubSection;
	mod_TaHoma.module.getSlaveFunction = getSlaveFunction;
*/

	mod_TaHoma.randomize = false;
	mod_TaHoma.defaultsampletime = 300;

	registerModule( (struct Module *)&mod_TaHoma );	/* Register the module */

#ifdef LUA
	if(mod_Lua){ /* Is mod_Lua loaded ? */

			/* Expose shared methods */

			/* Expose mod_owm's own function */
	}
#endif

	init_Curl();
}
