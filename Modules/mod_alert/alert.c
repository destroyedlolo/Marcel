/* mod_alert/alert
 *
 * Handle Alerts
 *
 * This file is part of Marcel project and is following the same
 * license rules (see LICENSE file)
 *
 * 15/04/2023 - LF - First version
 */
#include "mod_alert.h"	/* module's own stuffs */
#include "../Marcel/MQTT_tools.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>

	/* **
	 * Alerts' management
	 * **/

static struct alert *findalert(const char *id){
	for(struct alert *an = (struct alert *)mod_alert.alerts.first; an; an = (struct alert *)an->node.next){
		if(!strcmp(id, an->alert))
			return an;
	}

	return NULL;
}

static bool RiseAlert(const char *id, const char *msg){
	struct alert *an = findalert(id);

	if(!an){	/* Creating a new alert */
		assert( (an = malloc( sizeof(struct alert) )) );
		assert( (an->alert = strdup( id )) );
		DLAdd( &mod_alert.alerts, (struct DLNode *)an );
		return true;
	}
	return false;
}

	/* **
	 * Modules'
	 * **/

void alert_postconfInit(struct Section *asec){
	struct section_alert *s = (struct section_alert *)asec;	/* avoid lot of casting */

	if( !s->actions.url && !s->actions.cmd ){
		publishLog('F', "[%s]Both 'RESTUrl=' and 'OSCmd=' are missing.", s->section.uid);
		exit(EXIT_FAILURE);
	}

	static char * topics[] = {
		"Alerts/#",	/* Legacy alerting */
		"Raise/#",	/* Raise an alert (new way) */
		"Correct/#"	/* Correct an alert */
	};
	static int qoss[] = { 0, 0, 0 };

	if( MQTTClient_subscribeMany( cfg.client, sizeof(topics)/sizeof(*topics), topics, qoss) != MQTTCLIENT_SUCCESS ){
		publishLog('F', "[%s]Can't subscribe to alerting topics", s->section.uid);
		exit(EXIT_FAILURE);
	}
}

bool alert_processMQTT(struct Section *asec, const char *topic, char *payload){
	struct section_alert *s = (struct section_alert *)asec;	/* avoid lot of casting */
	const char *id;

	if((id = striKWcmp(topic, "Alerts/"))){	/* Legacy interface */
		if(*payload == 'S' || *payload == 's' ){	/* Rise an alert */
/* 			RiseAlert(id, payload+1, *payload == 'S'); */
		} else	/* Alert's over */
/*			AlertIsOver(id); */
		return true;
	} else if((id = striKWcmp(topic, "Raise/"))){
		if(RiseAlert(id, payload)){
			execOSCmd(s->actions.cmd, id, payload);
			execRest(s->actions.url, id, payload);
		}
		return true;
	}

	return false;
}

