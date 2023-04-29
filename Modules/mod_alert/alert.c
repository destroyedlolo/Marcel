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

static void sentAlertsCounter( void ){
	if(mod_alert.countertopic){
		unsigned int nbre=0;

		for(struct alert *an = (struct alert *)mod_alert.alerts.first; an; an = (struct alert *)an->node.next)
			nbre++;

		char tmsg[31];
		sprintf(tmsg, "%u", nbre);

		mqttpublish(cfg.client, mod_alert.countertopic, strlen(tmsg), tmsg, 0);

		publishLog('T', "Alerts counter : %u", nbre);
	}
}

bool RiseAlert(const char *id, const char *msg){
	struct alert *an = findalert(id);

	if(!an){	/* Creating a new alert */
		assert( (an = malloc( sizeof(struct alert) )) );
		assert( (an->alert = strdup( id )) );
		DLAdd( &mod_alert.alerts, (struct DLNode *)an );

		publishLog('E', "[%s] New alert : %s", id, msg);

		sentAlertsCounter();
		return true;
	}
	return false;
}

bool AlertIsOver(const char *id, const char *msg){
	struct alert *an = findalert(id);

	if(an){	/* The alert exists */
		DLRemove( &mod_alert.alerts, (struct DLNode *)an );
		free( (void *)an->alert );
		free( an );

		publishLog('C', "[%s] Corrected : %s", id, msg);

		sentAlertsCounter();
		return true;
	}
	return false;
}

	/* **
	 * Modules'
	 * **/

void malert_postconfInit(struct Section *asec){
	struct section_alert *s = (struct section_alert *)asec;	/* avoid lot of casting */

	if( !s->actions.url && !s->actions.cmd ){
		publishLog('F', "[%s]Both 'RESTUrl=' and 'OSCmd=' are missing.", s->section.uid);
		exit(EXIT_FAILURE);
	}

	if(!s->section.topic){
		publishLog('F', "[%s] Topic must be set. Dying ...", s->section.uid);
		exit(EXIT_FAILURE);
	}

	if( MQTTClient_subscribe( cfg.client, s->section.topic, 0) != MQTTCLIENT_SUCCESS ){
		publishLog('F', "[%s]Can't subscribe to '%s'", s->section.uid, s->section.topic);
		exit(EXIT_FAILURE);
	}
}

bool malert_alert_processMQTT(struct Section *asec, const char *topic, char *payload){
	struct section_alert *s = (struct section_alert *)asec;	/* avoid lot of casting */
	const char *id;

	if((id = striKWcmp(topic, "Alert/"))){	/* Legacy interface, topic hardcoded */
		if(*payload == 'S' || *payload == 's' ){	/* Rise an alert */
			if(RiseAlert(id, payload+1)){
				execOSCmd(s->actions.cmd, id, payload+1);
				if(*payload == 'S')
					execRest(s->actions.url, id, payload+1);
			}
		} else {	/* Alert's over */
			if(AlertIsOver(id, payload)){
				execOSCmd(s->actions.cmd, id, payload);
				execRest(s->actions.url, id, payload);
			}
		}
		return true;
	}

	return false;
}

bool salrt_raisealert_processMQTT(struct Section *asec, const char *topic, char *payload){
	struct section_alert *s = (struct section_alert *)asec;	/* avoid lot of casting */
	const char *id;

	if(!mqtttokcmp(s->section.topic, topic, &id)){
		if(RiseAlert(id, payload)){
			execOSCmd(s->actions.cmd, id, payload);
			execRest(s->actions.url, id, payload);
		}
		return true;
	}

	return false;
}

bool salrt_correctalert_processMQTT(struct Section *asec, const char *topic, char *payload){
	struct section_alert *s = (struct section_alert *)asec;	/* avoid lot of casting */
	const char *id;

	if(!mqtttokcmp(s->section.topic, topic, &id)){
		if(AlertIsOver(id, payload)){
			execOSCmd(s->actions.cmd, id, payload);
			execRest(s->actions.url, id, payload);
		}
		return true;
	}

	return false;
}
