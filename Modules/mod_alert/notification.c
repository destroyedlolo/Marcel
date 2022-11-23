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

void notif_postconfInit(struct Section *asec){
	struct section_namednotification *s = (struct section_namednotification *)asec;	/* avoid lot of casting */

	if( !s->url && !s->cmd ){
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
	struct section_namednotification *s = (struct section_namednotification *)asec;	/* avoid lot of casting */

	if(!mqtttokcmp(s->section.topic, topic)){
		if(s->section.disabled){
#ifdef DEBUG
			if(cfg.debug)
				publishLog('d', "[%s] is disabled", s->section.uid);
#endif
			return true;
		}

		size_t i=strlen(s->section.topic);
		assert(i>0);

		char t[i+1];
		strcpy(t, s->section.topic);
		t[--i]=0;

		const char *aid;
		assert((aid = striKWcmp(topic,t)));

		execOSCmd(s->cmd, aid, payload);
		return true;
	}

	return false;	/* Let's try with other sections */
}
