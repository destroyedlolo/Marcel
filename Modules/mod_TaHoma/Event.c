/* Handle event from a TaHoma
 *
 * This file is part of Marcel project and is following the same
 * license rules (see LICENSE file)
 *
 * 23/03/2026 - LF - Creation
 */

#include "mod_TaHoma.h"
#include "../Marcel/MQTT_tools.h"
#include "../Marcel/CURL_helpers.h"

#include <stdlib.h>

void *processEvent(void *actx){
	struct section_Event *s = (struct section_Event *)actx;

		/* Sanity checks */
	if(s->device.section.inerror){
		publishLog('F', "[%s] Unconfigured device. Dying ...", s->device.section.uid);
		SectionError((struct Section *)s, true);
		pthread_exit(0);
	}

		/* Registering the events handler */
	struct MemoryStruct buff = EMPTY_MEMCHUNK;
 	callAPI(&s->device, "events/register", "", &buff);
	if(cfg.debug)
		publishLog('d', "[%s] response \"%s\"", s->device.section.uid, buff.memory);

	if(!buff.memory){
		publishLog('F', "[%s] Empty registering response. Dying ...", s->device.section.uid);
		SectionError((struct Section *)s, true);
		pthread_exit(0);
	}

	struct json_object *parsed_json = json_tokener_parse(buff.memory);
	const char *idobj = getObjString(parsed_json, OBJPATH( "id", NULL ));
	if(!idobj){
		json_object_put(parsed_json);
		freeResponse(&buff);
		publishLog('F', "[%s] Can't find lister's id. Dying ...", s->device.section.uid);
		SectionError((struct Section *)s, true);
		pthread_exit(0);
	}

	if(cfg.debug)
		publishLog('d', "[%s] Listener ID : \"%s\"", s->device.section.uid, idobj);

	json_object_put(parsed_json);
	freeResponse(&buff);

	char fetchreq[strlen("events//fetch") + strlen(idobj) +1];
	sprintf(fetchreq, "events/%s/fetch", idobj);
	if(cfg.debug)
		publishLog('d', "[%s] Fetching URI \"%s\"", s->device.section.uid, fetchreq);

	/*
	for(;;){
	}
*/

	char unregreq[strlen("events//unregister") + strlen(idobj) +1];
	sprintf(unregreq, "events/%s/unregister", idobj);
	if(cfg.debug)
		publishLog('d', "[%s] unregistering URI \"%s\"", s->device.section.uid, unregreq);
	
	callAPI(&s->device, unregreq, "", &buff);

	if(cfg.debug)
		publishLog('d', "[%s] response \"%s\"", s->device.section.uid, buff.memory ? buff.memory : "NULL");

	pthread_exit(0);
}
