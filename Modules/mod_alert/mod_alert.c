/* mod_alert
 *
 * Handle notification and alerting
 *
 * This file is part of Marcel project and is following the same
 * license rules (see LICENSE file)
 *
 * 05/11//2022 - LF - First version
 */

#include "mod_alert.h"	/* module's own stuffs */

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>

static struct module_alert mod_alert;

enum {
	SA_ALERT = 0
};

static enum RC_readconf readconf(uint8_t mid, const char *l, struct Section **section ){
	const char *arg;

	if((arg = striKWcmp(l,"AlertName="))){
		if(*section){
			publishLog('F', "AlertName= can't be part of a section");
			exit(EXIT_FAILURE);
		}

		if(mod_alert.alert_name)
			publishLog('E', "AlertName= defined more than once. Let's continue ...");

		mod_alert.alert_name = *arg;

		if(cfg.verbose)
			publishLog('C', "\t\"/Alert\" notification's name : '%c'", mod_alert.alert_name);

		return ACCEPTED;
	} else if((arg = striKWcmp(l,"NotificationName="))){
		if(*section){
			publishLog('F', "NotificationName= can't be part of a section");
			exit(EXIT_FAILURE);
		}

		if(mod_alert.notif_name)
			publishLog('E', "NotificationName= defined more than once. Let's continue ...");

		mod_alert.notif_name = *arg;

		if(cfg.verbose)
			publishLog('C', "\t\"/Notification\" notification's name : '%c'", mod_alert.notif_name);

		return ACCEPTED;
	} else if(!strcmp(l, "AlertGrouped")){
		if(*section){
			publishLog('F', "AlertGrouped can't be part of a section");
			exit(EXIT_FAILURE);
		}

		mod_alert.alertgrouped = true;
		if(cfg.verbose)
			publishLog('C', "\tAlerts are grouped");
			
		return ACCEPTED;
	} else if((arg = striKWcmp(l,"$alert="))){
		if(strlen(arg) != 1){
			publishLog('F', "[%s] $alert's name can be ONLY 1 character long", arg);
			exit(EXIT_FAILURE);
		}
		if(findSectionByName(arg)){
			publishLog('F', "Section '%s' is already defined", arg);
			exit(EXIT_FAILURE);
		}

		struct section_namedalert *nsection = malloc(sizeof(struct section_namedalert));
		initSection( (struct Section *)nsection, mid, SA_ALERT, strdup(arg));

		nsection->url = NULL;
		nsection->cmd = NULL;

		if(cfg.verbose)	/* Be verbose if requested */
			publishLog('C', "\tEntering $alert section '%s' (%04x)", nsection->section.uid, nsection->section.id);

		mod_alert.firstalert = nsection;
		*section = (struct Section *)nsection;
		return ACCEPTED;		
	} else if(*section){
		if((arg = striKWcmp(l,"RESTUrl="))){
			acceptSectionDirective(*section, "RESTUrl=");
			((struct section_namedalert *)(*section))->url = strdup(arg);
			assert(((struct section_namedalert *)(*section))->url);

			if(cfg.verbose)
				publishLog('C', "\t\tRESTUrl : '%s'", ((struct section_namedalert *)(*section))->url);

			return ACCEPTED;
		} else	if((arg = striKWcmp(l,"OSCmd="))){
			acceptSectionDirective(*section, "OSCmd=");
			((struct section_namedalert *)(*section))->cmd = strdup(arg);
			assert(((struct section_namedalert *)(*section))->cmd);

			if(cfg.verbose)
				publishLog('C', "\t\tOSCmd : '%s'", ((struct section_namedalert *)(*section))->cmd);

			return ACCEPTED;
		}
	}

	return REJECTED;
}

static bool acceptSDirective( uint8_t sec_id, const char *directive ){
	if(sec_id == SA_ALERT){
		if( !strcmp(directive, "RESTUrl=") )
			return true;	/* Accepted */
		else if( !strcmp(directive, "OSCmd=") )
			return true;	/* Accepted */
	}

	return false;
}

void InitModule( void ){
	initModule((struct Module *)&mod_alert, "mod_alert");

	mod_alert.module.readconf = readconf;
	mod_alert.module.acceptSDirective = acceptSDirective;

	mod_alert.alert_name = 0;
	mod_alert.notif_name = 0;

	mod_alert.firstalert = NULL;
	mod_alert.alertgrouped = false;

	registerModule( (struct Module *)&mod_alert );	/* Register the module */
}
