/* mod_dpd.h
 *
 * 	Dead Publisher Detector
 * 	Publish a message if a figure doesn't come on time
 *
 * This file is part of Marcel project and is following the same
 * license rules (see LICENSE file)
 *
 * 06/10/2022 - LF - Migrated to v8
 */

#include "mod_dpd.h"
#ifdef LUA
#	include "../mod_Lua/mod_Lua.h"
#endif

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>

static struct module_dpd mod_dpd;

enum {
	SD_DPD = 0
};

static enum RC_readconf readconf(uint8_t mid, const char *l, struct Section **section ){
	const char *arg;

	if((arg = striKWcmp(l,"*DPD="))){
		if(findSectionByName(arg)){
			publishLog('F', "Section '%s' is already defined", arg);
			exit(EXIT_FAILURE);
		}

		struct section_dpd *nsection = malloc(sizeof(struct section_dpd));
		initSection( (struct Section *)nsection, mid, SD_DPD, strdup(arg));

		nsection->notiftopic = NULL;
		nsection->rcv = -1;
		nsection->inerror = false;

		if(cfg.verbose)	/* Be verbose if requested */
			publishLog('C', "\tEntering section '%s' (%04x)", nsection->section.uid, nsection->section.id);

		*section = (struct Section *)nsection;
		return ACCEPTED;
	} else if(*section){
		if((arg = striKWcmp(l,"Timeout="))){
			(*section)->sample = atof(arg);

			if(cfg.verbose)
				publishLog('C', "\t\tTimeout : %lf", (*section)->sample);

			return ACCEPTED;			
		} else if((arg = striKWcmp(l,"NotificationTopic="))){
			((struct section_dpd *)(*section))->notiftopic = strdup(arg);
			assert(((struct section_dpd *)(*section))->notiftopic);

			if(cfg.verbose)
				publishLog('C', "\t\tNotification Topic : '%s'", ((struct section_dpd *)(*section))->notiftopic);

			return ACCEPTED;			
		}
	}

	return REJECTED;
}

static bool md_acceptSDirective( uint8_t sec_id, const char *directive ){
	if(sec_id == SD_DPD){
		if( !strcmp(directive, "Disabled") )
			return true;	/* Accepted */
		else if( !strcmp(directive, "Topic=") )
			return true;	/* Accepted */
		else if( !strcmp(directive, "Func=") )
			return true;	/* Accepted */
		else if( !strcmp(directive, "Sample=") )
			return true;	/* Accepted */
		else if( !strcmp(directive, "Timeout=") )
			return true;	/* Accepted */
		else if( !strcmp(directive, "NotificationTopic=") )
			return true;	/* Accepted */
	}

	return false;
}

void InitModule( void ){
	mod_dpd.module.name = "mod_dpd";

	mod_dpd.module.readconf = readconf;
	mod_dpd.module.acceptSDirective = md_acceptSDirective;
	mod_dpd.module.getSlaveFunction = NULL;
	mod_dpd.module.postconfInit = NULL;

	register_module( (struct Module *)&mod_dpd );	/* Register the module */
}
