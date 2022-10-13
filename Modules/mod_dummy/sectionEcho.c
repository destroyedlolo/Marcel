/* in order to speedup compilation it's better to split large source files in
 * multiple chunks.
 * Here, as example, dummy section processing is externalized
 *
 * This file is part of Marcel project and is following the same
 * license rules (see LICENSE file)
 */


/* ***
 * Marcel's own include 
 * ***/

#include "mod_dummy.h"	/* module's own stuffs */
#include "../Marcel/MQTT_tools.h"

/* ***
 * System's include
 * ***/

#include <stdlib.h>

/* ***
 * Let's go
 * ***/

/* Function called just after configuration reading.
 * Here, the goal is to subscribe to this section topic
 */
void st_echo_postconfInit(struct Section *asec){
	struct section_echo *s = (struct section_echo *)asec;	/* avoid lot of casting */

		/* Sanity checks
		 * As they're highlighting configuration issue, let's
		 * consider error as fatal.
		 */
	if(!s->section.topic){
		publishLog('E', "[%s] Topic must be set. Dying ...", s->section.uid);
		pthread_exit(0);
	}

		/* Subscribing */
	if(MQTTClient_subscribe( cfg.client, s->section.topic, 0 ) != MQTTCLIENT_SUCCESS ){
		publishLog('E', "Can't subscribe to '%s'", s->section.topic );
		exit( EXIT_FAILURE );
	}
}

	/* A message is arrived.
	 * If present, callbacks of sections are called to process the message
	 */
bool st_echo_processMQTT(struct Section *asec, const char *topic, char *payload ){
	struct section_echo *s = (struct section_echo *)asec;	/* avoid lot of casting */

	if(!mqtttokcmp(s->section.topic, topic)){
		publishLog('I', "[%s] %s", s->section.uid, payload);
		return true;	/* we processed the message */
	}

	return false;	/* Let's try with other sections */
}

