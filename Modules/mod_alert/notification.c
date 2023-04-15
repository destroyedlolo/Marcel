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

struct namednotification *findNamed(const char n){
	for(struct namednotification *nnotif = mod_alert.nnotif; nnotif; nnotif = nnotif->next){
		if(nnotif->name == n)
			return nnotif;
	}

	return NULL;
}
