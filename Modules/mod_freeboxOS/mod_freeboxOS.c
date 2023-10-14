/* mod_freeboxOS
 *
 *  Publish FreeboxOS figures
 *
 * This file is part of Marcel project and is following the same
 * license rules (see LICENSE file)
 *
 * 24/09/2023 - LF - First version
 */

#include "mod_freeboxOS.h"
#include "../Marcel/MQTT_tools.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#if 0
#include <unistd.h>
#include <errno.h>
#endif

static struct module_freeboxOS mod_freeboxOS;

enum {
	SFB_FREEBOX = 0
};

static void *process_freeboxOS(void *actx){
	struct section_freeboxOS *ctx = (struct section_freeboxOS *)actx;

		/* Sanity checks */
	if(!ctx->section.topic){
		publishLog('E', "['%s'] configuration error : no topic specified. Dying ...", ctx->section.uid);
		pthread_exit(0);
	}


	if(!ctx->section.sample){
		publishLog('E', "[%s] Sample time can't be 0. Dying ...", ctx->section.uid);
		pthread_exit(0);
	}

	puts("ok");
}

static enum RC_readconf readconf(uint8_t mid, const char *l, struct Section **asection ){
	struct section_freeboxOS **section = (struct section_freeboxOS **)asection;
	const char *arg;

	if((arg = striKWcmp(l,"*FreeboxOS="))){	/* Starting a section definition */
		if(findSectionByName(arg)){
			publishLog('F', "Section '%s' is already defined", arg);
			exit(EXIT_FAILURE);
		}

		struct section_freeboxOS *nsection = malloc(sizeof(struct section_freeboxOS));
		initSection( (struct Section *)nsection, mid, SFB_FREEBOX, strdup(arg));

		nsection->url = NULL;
		nsection->app_token = NULL;

		if(cfg.verbose)	/* Be verbose if requested */
			publishLog('C', "\tEntering FreeboxOS section '%s' (%04x)", nsection->section.uid, nsection->section.id);

		*section = nsection;	/* we're now in a section */
		return ACCEPTED;
	} else if(*section){
		if((arg = striKWcmp(l,"URL="))){
/* Not needed as only one section exists
			acceptSectionDirective(*section, "URL=");
*/
			if((*(struct section_freeboxOS **)section)->url){
				publishLog('F', "['%s'] Url defined twice", (*section)->section.uid);
				exit(EXIT_FAILURE);
			}
			assert(( (*section)->url = strdup(arg) ));

			if(cfg.verbose)	/* Be verbose if requested */
				publishLog('C', "\t\tURL : '%s'", (*section)->url);
	
			return ACCEPTED;
		} else if((arg = striKWcmp(l,"app_token="))){
/* Not needed as only one section exists
			acceptSectionDirective(*section, "app_token=");
*/
			if((*(struct section_freeboxOS **)section)->app_token){
				publishLog('F', "['%s'] app_token defined twice", (*section)->section.uid);
				exit(EXIT_FAILURE);
			}
			assert(( (*section)->app_token = strdup(arg) ));

			if(cfg.verbose)	/* Be verbose if requested */
				publishLog('C', "\t\tapp_token : '%s'", (*section)->app_token);
	
			return ACCEPTED;
		}
	}

	return REJECTED;
}

static bool mfb_acceptSDirective( uint8_t sec_id, const char *directive ){
	if(sec_id == SFB_FREEBOX){
		if( !strcmp(directive, "Disabled") )
			return true;
		else if( !strcmp(directive, "URL=") )
			return true;
		else if( !strcmp(directive, "Sample=") )
			return true;
		else if( !strcmp(directive, "Topic=") )
			return true;
		else if( !strcmp(directive, "Immediate") )
			return true;
		else if( !strcmp(directive, "Keep") )
			return true;
	}

	return false;
}

static ThreadedFunctionPtr mfb_getSlaveFunction(uint8_t sid){
	if(sid == SFB_FREEBOX)
		return process_freeboxOS;

	return NULL;
}

void InitModule( void ){
	initModule((struct Module *)&mod_freeboxOS, "mod_freeboxOS");

	mod_freeboxOS.module.readconf = readconf;
	mod_freeboxOS.module.acceptSDirective = mfb_acceptSDirective;
	mod_freeboxOS.module.getSlaveFunction = mfb_getSlaveFunction;

	registerModule( (struct Module *)&mod_freeboxOS );
}
