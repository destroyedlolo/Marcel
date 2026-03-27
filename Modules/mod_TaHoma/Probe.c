/* Query Devices (probes) from a TaHoma
 *
 * This file is part of Marcel project and is following the same
 * license rules (see LICENSE file)
 *
 * 11/03/2026 - LF - Creation
 */

#include "mod_TaHoma.h"
#include "../Marcel/MQTT_tools.h"
#include "../Marcel/CURL_helpers.h"

#include <stdlib.h>

void *processProbe (void *actx){
	struct section_Probe *s = (struct section_Probe *)actx;

		/* Sanity checks */
	if(s->device.section.inerror){
		publishLog('F', "[%s] Unconfigured device. Dying ...", s->device.section.uid);
		s->device.section.inerror = false;	/* To let SectionError() notifying */
		SectionError((struct Section *)s, true);
		pthread_exit(0);
	}

	for(bool first=true;; first=false){	/* Infinite publishing loop */
		if(isDisabled((struct Section *)s)){
#ifdef DEBUG
			if(cfg.debug)
				publishLog('d', "[%s] is disabled", s->device.section.uid);
#endif
		} else if(isDisabled(&s->device.gateway->section)){
#ifdef DEBUG
			if(cfg.debug)
				publishLog('d', "[%s] TaHoma \"%s\" is disabled", s->device.section.uid, s->device.gateway->section.uid);
#endif
		} else if( !first || s->device.section.immediate ){
			struct MemoryStruct buff = EMPTY_MEMCHUNK;
			if(callAPIDev(&s->device, NULL, NULL, &buff)){	/* Call succeeded */
				struct json_object *res= json_tokener_parse(buff.memory);
				if(json_object_is_type(res, json_type_array)){
					size_t nbr = json_object_array_length(res);	/* get the number of sub objects */
					for(size_t idx=0; idx < nbr; ++idx){
						struct json_object *obj = json_object_array_get_idx(res, idx);
						const char *n = getObjString(obj, OBJPATH( "name", NULL ));
						int type = getObjInt(obj, OBJPATH( "type", NULL ));

						if(!n || !type)	/* Bad formatted response */
							continue;

						for(struct State_definition *st = s->States; st; st = st->next){
							if(!strcmp(n, st->state)){
								if(! (st->disabled || (st->dontSimulate && cfg.simulate)) ){
									struct json_object *val = getObj(obj,  OBJPATH( "value", NULL ) );
									const char *l = json_object_to_json_string(val);

									mqttpublish(cfg.client, st->topic, strlen(l), (void *)l, st->retained );
									if(cfg.debug)
										publishLog('d', "[%s][%s] %s", s->device.section.uid, st->state, l);
								} else if(cfg.debug)
									publishLog('d', "[%s][%s] is disabled", s->device.section.uid, st->state);
								break;
							}
						}
					}
				} else {
					publishLog('E', "[%s] Incorrect response (not an array)", s->device.section.uid);
					SectionError((struct Section *)s, true);
				}

				json_object_put(res);
			} else if(cfg.debug){
				if(buff.memory)	/* We're in error but a response may have been provided */
					publishLog('d', "[%s] response \"%s\"", s->device.section.uid, buff.memory);
			}

			if(buff.memory)
				freeResponse(&buff);
		}

		if(s->device.section.sample > 0){
			struct timespec ts;
			ts.tv_sec = (time_t)s->device.section.sample;
			ts.tv_nsec = (unsigned long int)((s->device.section.sample - (time_t)s->device.section.sample) * 1e9);

			nanosleep( &ts, NULL );
		} else {
#ifdef DEBUG
			if(cfg.debug)
				publishLog('d', "[%s] is running only once", s->device.section.uid);
#endif
			break;
		}
	}

	return NULL;
}
