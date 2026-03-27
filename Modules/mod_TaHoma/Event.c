/* Handle event from a TaHoma
 *
 * This file is part of Marcel project and is following the same
 * license rules (see LICENSE file)
 *
 * 23/03/2026 - LF - Creation
 */

/*** Event example
[
        deviceStates :
                [
                        value :
                                573
                        type :
                                1
                        name :
                                "core:CO2ConcentrationState"
                ]
        deviceURL :
                "zigbee://2095-0445-1705/58849/1#3"
        name :
                "DeviceStateChangedEvent"
]
***/

#include "mod_TaHoma.h"
#include "../Marcel/MQTT_tools.h"
#include "../Marcel/CURL_helpers.h"

#include <stdlib.h>

void *processEvent(void *actx){
	struct section_TaHoma *s = (struct section_TaHoma *)actx;

		/* Sanity checks */
	if(s->section.inerror){
		publishLog('F', "[%s] Unconfigured box. Dying ...", s->section.uid);
		SectionError((struct Section *)s, true);
		pthread_exit(0);
	}

		/* Registering the events handler */
	struct MemoryStruct buff = EMPTY_MEMCHUNK;
 	callAPIGW(s, "events/register", "", &buff);
	if(cfg.debug)
		publishLog('d', "[%s] response \"%s\"", s->section.uid, buff.memory);

	if(!buff.memory){
		publishLog('F', "[%s] Empty registering response. Dying ...", s->section.uid);
		SectionError(&s->section, true);
		pthread_exit(0);
	}

	struct json_object *parsed_json = json_tokener_parse(buff.memory);
	const char *idobj = getObjString(parsed_json, OBJPATH( "id", NULL ));
	if(!idobj){
		json_object_put(parsed_json);
		freeResponse(&buff);
		publishLog('F', "[%s] Can't find lister's id. Dying ...", s->section.uid);
		SectionError(&s->section, true);
		pthread_exit(0);
	}

	if(cfg.debug)
		publishLog('d', "[%s] Listener ID : \"%s\"", s->section.uid, idobj);

	json_object_put(parsed_json);
	freeResponse(&buff);

	char fetchreq[strlen("events//fetch") + strlen(idobj) +1];
	sprintf(fetchreq, "events/%s/fetch", idobj);
	if(cfg.debug)
		publishLog('d', "[%s] Fetching URI \"%s\"", s->section.uid, fetchreq);

	for(;;){
		callAPIGW(s, fetchreq, "", &buff);
/*		if(cfg.debug) */
			publishLog('d', "[%s] Event resp: \"%s\"", s->section.uid, buff.memory ? buff.memory : "NULL data");

		if(buff.memory){
			struct json_object *parsed_json = json_tokener_parse(buff.memory);
			if(json_object_is_type(parsed_json, json_type_array)){
				size_t nbre = json_object_array_length(parsed_json);
				for(size_t i=0; i<nbre; ++i){
					struct json_object *entry = json_object_array_get_idx(parsed_json, i);
					const char *dev = getObjString(entry, OBJPATH( "deviceURL", NULL ));
					const char *ev = getObjString(entry, OBJPATH( "name", NULL ));
					struct json_object *vals = getObj(entry,  OBJPATH( "deviceStates", NULL ) );
					if(json_object_is_type(vals, json_type_array)){
						size_t nbrev = json_object_array_length(vals);
						for(size_t j=0; j<nbrev; ++j){
							struct json_object *v = json_object_array_get_idx(vals, j);
							const char *name = getObjString(v, OBJPATH( "name", NULL ));
							const char *l = json_object_to_json_string(getObj(v,  OBJPATH( "value", NULL )));

	/*						if(cfg.debug) */
								publishLog('d', "[%s] Event e:%s u:%s n:%s v:%s", s->section.uid, ev, dev, name, l);
						}
					}
				}
			} else /* if(cfg.debug) */
				publishLog('E', "[%s] Not a JSON array", s->section.uid);

			json_object_put(parsed_json);
			freeResponse(&buff);
		}

		struct timespec ts;
		ts.tv_sec = (time_t)s->section.sample;
		ts.tv_nsec = (unsigned long int)((s->section.sample - (time_t)s->section.sample) * 1e9);

		nanosleep( &ts, NULL );
	
	}

	char unregreq[strlen("events//unregister") + strlen(idobj) +1];
	sprintf(unregreq, "events/%s/unregister", idobj);
	if(cfg.debug)
		publishLog('d', "[%s] unregistering URI \"%s\"", s->section.uid, unregreq);
	
	callAPIGW(s, unregreq, "", &buff);

	if(cfg.debug)
		publishLog('d', "[%s] response \"%s\"", s->section.uid, buff.memory ? buff.memory : "NULL");

	pthread_exit(0);
}
