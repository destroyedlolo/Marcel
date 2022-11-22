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
	SA_NAMEDNOTIF = 0,
	SA_ALERT,
	SA_NOTIF
};

static enum RC_readconf readconf(uint8_t mid, const char *l, struct Section **section ){
	const char *arg;

	if((!strcmp(l,"$UnnamedNotification"))){
		if(findSectionByName("$UnnamedNotification")){
			publishLog('F', "'$UnnamedNotification' section is already defined");
			exit(EXIT_FAILURE);
		}

		struct section_namednotification *nsection = malloc(sizeof(struct section_namednotification));
		initSection( (struct Section *)nsection, mid, SA_NOTIF, "$UnnamedNotification");

		nsection->url = NULL;
		nsection->cmd = NULL;

/* utiliser postconfInit pour s'abonné au topic "Notification/#"
 * processMsg() pour les traiter.
 * Voir mod_outfile
 */
		if(cfg.verbose)	/* Be verbose if requested */
			publishLog('C', "\tEntering $UnnamedNotification section (%04x)", nsection->section.id);

		*section = (struct Section *)nsection;
		return ACCEPTED;		
#if 0
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
		initSection( (struct Section *)nsection, mid, SA_NAMEDNOTIF, strdup(arg));

		nsection->url = NULL;
		nsection->cmd = NULL;

		if(cfg.verbose)	/* Be verbose if requested */
			publishLog('C', "\tEntering $alert section '%s' (%04x)", nsection->section.uid, nsection->section.id);

		mod_alert.firstalert = nsection;
		*section = (struct Section *)nsection;
		return ACCEPTED;		
#endif
	} else if(*section){
		if((arg = striKWcmp(l,"RESTUrl="))){
			acceptSectionDirective(*section, "RESTUrl=");
			((struct section_namednotification *)(*section))->url = strdup(arg);
			assert(((struct section_namednotification *)(*section))->url);

			if(cfg.verbose)
				publishLog('C', "\t\tRESTUrl : '%s'", ((struct section_namednotification *)(*section))->url);

			return ACCEPTED;
		} else	if((arg = striKWcmp(l,"OSCmd="))){
			acceptSectionDirective(*section, "OSCmd=");
			((struct section_namednotification *)(*section))->cmd = strdup(arg);
			assert(((struct section_namednotification *)(*section))->cmd);

			if(cfg.verbose)
				publishLog('C', "\t\tOSCmd : '%s'", ((struct section_namednotification *)(*section))->cmd);

			return ACCEPTED;
		}
	}
	return REJECTED;
}

static bool acceptSDirective( uint8_t sec_id, const char *directive ){
	if(sec_id == SA_NAMEDNOTIF){
		if( !strcmp(directive, "RESTUrl=") )
			return true;	/* Accepted */
		else if( !strcmp(directive, "OSCmd=") )
			return true;	/* Accepted */
	}

	return false;
}

#if 0
static void subTopic( uint8_t mid ){
	if(mod_alert.alert_used){
		if( MQTTClient_subscribe( cfg.client, "Alert/#", 0) != MQTTCLIENT_SUCCESS ){
			publishLog('F', "Can't subscribe to 'Alert/#'");
			exit(EXIT_FAILURE);
		}
	} else if(cfg.verbose)
		publishLog('I', "Alert disabled");

	if(mod_alert.unotif_used){
		if( MQTTClient_subscribe( cfg.client, "Notification/#", 0) != MQTTCLIENT_SUCCESS ){
			publishLog('F', "Can't subscribe to 'Notification/#'");
			exit(EXIT_FAILURE);
		}
	} else if(cfg.verbose)
		publishLog('I', "Unnamed notification disabled");
	
	if(mod_alert.nnotif_used){
		if( MQTTClient_subscribe( cfg.client, "nNotification/#", 0) != MQTTCLIENT_SUCCESS ){
			publishLog('F', "Can't subscribe to 'nNotification/#'");
			exit(EXIT_FAILURE);
		}
	} else if(cfg.verbose)
		publishLog('I', "Named notification disabled");
}

static bool processMsg(const char *topic, char *payload){
	const char *arg;

	/* processing "static" topic */
	if((arg = striKWcmp(topic, "Alert/"))){
		publishLog('T', "Received alert \"%s\"", arg);
		return true;
	} else if((arg = striKWcmp(topic, "Notification/"))){
		publishLog('T', "Received alert \"%s\"", arg);
		return true;
	}
	/* processing named sessions */


	return false;
}
#endif

void InitModule( void ){
	initModule((struct Module *)&mod_alert, "mod_alert");

	mod_alert.module.readconf = readconf;
	mod_alert.module.acceptSDirective = acceptSDirective;

	registerModule( (struct Module *)&mod_alert );	/* Register the module */
}
