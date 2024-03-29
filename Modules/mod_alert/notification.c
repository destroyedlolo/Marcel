/* mod_alert/notification
 *
 * Handle named and unnamed notifications
 *
 * This file is part of Marcel project and is following the same
 * license rules (see LICENSE file)
 *
 * 23/11//2022 - LF - First version
 */

#include "mod_alert.h"	/* module's own stuffs */
#include "../Marcel/MQTT_tools.h"

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>


	/* ***
	 * Unamed notification
	 * ***/

void notif_postconfInit(struct Section *asec){
	struct section_unnamednotification *s = (struct section_unnamednotification *)asec;	/* avoid lot of casting */

	if( !s->actions.url && !s->actions.cmd ){
		publishLog('F', "[%s]Both 'RESTUrl=' and 'OSCmd=' are missing.", s->section.uid);
		exit(EXIT_FAILURE);
	}

		/* No need to verify if s->section.topic is set as it is hard-coded during
		 * section initialisation.
		 */
	if( MQTTClient_subscribe( cfg.client, s->section.topic, 0) != MQTTCLIENT_SUCCESS ){
		publishLog('F', "[%s]Can't subscribe to '%s'", s->section.uid, s->section.topic);
		exit(EXIT_FAILURE);
	}
}

bool notif_unnamednotification_processMQTT(struct Section *asec, const char *topic, char *payload){
	struct section_unnamednotification *s = (struct section_unnamednotification *)asec;	/* avoid lot of casting */
	const char *aid;

	if(!mqtttokcmp(s->section.topic, topic, &aid)){
		if(s->section.disabled){
#ifdef DEBUG
			if(cfg.debug)
				publishLog('d', "[%s] is disabled", s->section.uid);
#endif
			return true;
		}

		if(s->actions.cmd && *payload == 'S')
			execOSCmd(s->actions.cmd, aid, payload + (*payload == 'S' ? 1 : 0));
		if(s->actions.url)
			execRest(s->actions.url, aid, payload + (*payload == 'S' ? 1 : 0));
		return true;
	}

	return false;	/* Let's try with other sections */
}


	/* ***
	 * Named notification
	 * ***/

/**
 * @brief Search for named notification by its name
 *
 * @param name of the notification to find
 * @return pointer to the named notification or NULL if not found
 */
struct namednotification *findNamed(const char n){
	for(struct namednotification *nnotif = mod_alert.nnotif; nnotif; nnotif = nnotif->next){
		if(nnotif->name == n)
			return nnotif;
	}

	return NULL;
}

void pnNotify(const char *names, const char *title, const char *payload){
	char sec;
	while(( sec=*names++ )){
		if(sec=='/')	/* in order to process nNotification/ message */
			break;

		struct namednotification *n = findNamed(sec);
		publishLog('T', "Received named notification \"%c\" (%s) title \"%s\"", sec, n ? "found":"unknown", title);

		if(n){
			if(n->disabled){
#ifdef DEBUG
				if(cfg.debug)
					publishLog('d', "Named notification \"%c\" is disabled", n->name);
#endif
				continue;
			}

			if(n->actions.cmd)
				execOSCmd(n->actions.cmd, title, payload);
			if(n->actions.url)
				execRest(n->actions.url, title, payload);
		}
	}
}

/**
 * @brief Enable or disable a named notification
 *
 * @param name Name of the named notification
 * @param val new status (true if disable)
 * @return false if the named can't be found
 */
bool namedNNDisable(char n, bool val){
	struct namednotification *nn = findNamed(n);

	if(!nn){
		if(cfg.verbose)
			publishLog('E', "Named notification \"%c\" doesn't exist", n);
		return false;
	}

	nn->disabled = val;

	if(cfg.verbose)
		publishLog('I', "%s Named notification \"%c\"", val ? "Disabling":"Enabling", nn->name);

	publishNNStatus(nn);

	return true;
}

/**
 * @brief Publish the status of a NamedNotification
 *
 * @param nn Named notification to publish
 */
void publishNNStatus(struct namednotification *nn){
	char ttopic[ strlen(cfg.ClientID) + 27 ];	/* + "/NamedNotificationChange/?" */
	sprintf(ttopic, "%s/NamedNotificationChange/%c", cfg.ClientID, nn->name);

	char t[2] = { 
		nn->disabled ? '0':'1', 
		0
	};

	mqttpublish(cfg.client, ttopic, 1, t, false);

#ifdef DEBUG
	publishLog('d', "Named Notification \"%c\" status is \"%s\"", nn->name, t);
#endif
}

