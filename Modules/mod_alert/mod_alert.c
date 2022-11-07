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

struct module_alert mod_alert;

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
			
			return ACCEPTED;	}

	return REJECTED;
}

void InitModule( void ){
	initModule((struct Module *)&mod_alert, "mod_alert");

	mod_alert.module.readconf = readconf;

	mod_alert.alert_name = 0;
	mod_alert.notif_name = 0;

	mod_alert.notiflist = NULL;
	mod_alert.alertgrouped = false;

	registerModule( (struct Module *)&mod_alert );	/* Register the module */
}
