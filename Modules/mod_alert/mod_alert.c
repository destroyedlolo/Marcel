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

enum {
	SA_ALERT=0,
	SA_NOTIF
};

static enum RC_readconf readconf(uint8_t mid, const char *l, struct Section **section ){
	const char *arg;

	if((!strcmp(l,"$unnamedNotification"))){
		if(findSectionByName("$unnamedNotification")){
			publishLog('F', "'$unnamedNotification' section is already defined");
			exit(EXIT_FAILURE);
		}

		struct section_unnamednotification *nsection = malloc(sizeof(struct section_unnamednotification));
		initSection( (struct Section *)nsection, mid, SA_NOTIF, "$unnamedNotification");

		nsection->section.topic = "Notification/#";
		nsection->actions.url = NULL;
		nsection->actions.cmd = NULL;
		nsection->section.postconfInit = notif_postconfInit;
		nsection->section.processMsg = notif_unnamednotification_processMQTT;

		if(cfg.verbose)	/* Be verbose if requested */
			publishLog('C', "\tEntering $UnnamedNotification section (%04x)", nsection->section.id);

		mod_alert.current = NULL;	/* not anymore in a named */
		*section = (struct Section *)nsection;
		return ACCEPTED;
	} else if((arg = striKWcmp(l,"$namedNotification="))){
		if(arg[1]){
			publishLog('F', "[%s] \"$namedNotification=\"'s name can be only 1 character long", arg);
			exit(EXIT_FAILURE);
		}

		if(findNamed(*arg)){
			publishLog('F', "\"$namedNotification=%c\" is already defined", *arg);
			exit(EXIT_FAILURE);
		}

		struct namednotification *nnamed = malloc(sizeof(struct namednotification));
		assert(nnamed);
		nnamed->next = mod_alert.nnotif;
		nnamed->actions.url = NULL;
		nnamed->actions.cmd = NULL;
		nnamed->name = *arg;
		nnamed->disabled = false;

		if(cfg.verbose)	/* Be verbose if requested */
			publishLog('C', "\tEntering $namedNotification '%c'", *arg);

		mod_alert.nnotif = nnamed;
		mod_alert.current = nnamed;
		*section = NULL;	/* not anymore in a section */
		return ACCEPTED;
	} if((!strcmp(l,"$alert"))){
		if(findSectionByName("$alert")){
			publishLog('F', "'$alert' section is already defined");
			exit(EXIT_FAILURE);
		}

		struct section_alert *nsection = malloc(sizeof(struct section_alert));
		initSection( (struct Section *)nsection, mid, SA_ALERT, "$alert");

			/* Module's own fields */
		nsection->actions.url = NULL;
		nsection->actions.cmd = NULL;

/*		nsection->section.topic = "Alert/#"; */
		nsection->section.postconfInit = alert_postconfInit;
		nsection->section.processMsg = alert_processMQTT;

		if(cfg.verbose)	/* Be verbose if requested */
			publishLog('C', "\tEntering $alert section (%04x)", nsection->section.id);

		mod_alert.current = NULL;	/* not anymore in a named */
		*section = (struct Section *)nsection;
		return ACCEPTED;
	} else if(*section || mod_alert.current){
		if((arg = striKWcmp(l,"RESTUrl="))){
			if(mod_alert.current){	/* Named notification */
				assert(( mod_alert.current->actions.url = strdup(arg) ));

				if(cfg.verbose)
					publishLog('C', "\t\tRESTUrl : '%s'", mod_alert.current->actions.url);

				return ACCEPTED;
			} else {	/* Unamed notification or alert */
				acceptSectionDirective(*section, "RESTUrl=");
				((struct section_unnamednotification *)(*section))->actions.url = strdup(arg);
				assert(((struct section_unnamednotification *)(*section))->actions.url);

				if(cfg.verbose)
					publishLog('C', "\t\tRESTUrl : '%s'", ((struct section_unnamednotification *)(*section))->actions.url);

				return ACCEPTED;
			}
		} else	if((arg = striKWcmp(l,"OSCmd="))){
			if(mod_alert.current){	/* Named notification */
				assert(( mod_alert.current->actions.cmd = strdup(arg) ));

				if(cfg.verbose)
					publishLog('C', "\t\tOSCmd : '%s'", mod_alert.current->actions.cmd);

				return ACCEPTED;
			} else {	/* Unamed notification or alert */
				acceptSectionDirective(*section, "OSCmd=");
				((struct section_unnamednotification *)(*section))->actions.cmd = strdup(arg);
				assert(((struct section_unnamednotification *)(*section))->actions.cmd);

				if(cfg.verbose)
					publishLog('C', "\t\tOSCmd : '%s'", ((struct section_unnamednotification *)(*section))->actions.cmd);

				return ACCEPTED;
			}
		} else if(!strcmp(l,"Disabled") && mod_alert.current){
				/* Needed as alerts/notifications are stored alone and not
				 * considered as sections.
				 */
			mod_alert.current->disabled = true;

			if(cfg.verbose)
				publishLog('C', "\t\tDisabled");

			return ACCEPTED;
		}
	}
	return REJECTED;
}

static bool acceptSDirective( uint8_t sec_id, const char *directive ){
	if(sec_id == SA_NOTIF || sec_id == SA_ALERT){
		if( !strcmp(directive, "RESTUrl=") )
			return true;	/* Accepted */
		else if( !strcmp(directive, "OSCmd=") )
			return true;	/* Accepted */
		if( !strcmp(directive, "Disabled") )
			return true;	/* Accepted */
	}

	return false;
}

/* Subscribe ONLY if named notification are used */
static void subTopic( uint8_t mid ){
	if(mod_alert.nnotif){
		if( MQTTClient_subscribe( cfg.client, "nNotification/#", 0) != MQTTCLIENT_SUCCESS ){
			publishLog('F', "Can't subscribe to 'nNotification/#'");
			exit(EXIT_FAILURE);
		}
	} else if(cfg.verbose)
		publishLog('I', "Named notification disabled");
}

static bool processMsg(const char *topic, char *payload){
	const char *arg;

	/* processing named sessions */
	if((arg = striKWcmp(topic, "nNotification/"))){	/* Mustn't include wildcard otherwise, use mqtttokcmp() */
		char *title = strchr(arg, '/');

		if(!title)
			publishLog('E', "Received named notification \"%s\" without title : ignoring", arg);
		else {
			while(arg < title){
				char sec = *arg++;
				struct namednotification *n = findNamed(sec);
				publishLog('T', "Received named notification \"%c\" (%s) title \"%s\"", sec, n ? "found":"unknown", title+1);

				if(n){
					if(n->disabled){
#ifdef DEBUG
						if(cfg.debug)
							publishLog('d', "Named notification \"%c\" is disabled", n->name);
#endif
						continue;
					}

					if(n->actions.cmd)
						execOSCmd(n->actions.cmd, title+1, payload);
					if(n->actions.url)
						execRest(n->actions.url, title+1, payload);
				}
			}
		}
		return true;	/* Even if notification are unknown, we processed the message */
	}

	return false;
}

void InitModule( void ){
	initModule((struct Module *)&mod_alert, "mod_alert");

	mod_alert.module.readconf = readconf;
	mod_alert.module.acceptSDirective = acceptSDirective;
	mod_alert.module.postconfInit = subTopic;	/* Named subscription if needed */
	mod_alert.module.processMsg = processMsg;	/* Named subscription */

	mod_alert.nnotif = NULL;
	mod_alert.current = NULL;

	DLListInit(&mod_alert.alerts);

	registerModule( (struct Module *)&mod_alert );	/* Register the module */
}
