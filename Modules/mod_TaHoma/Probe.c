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
		} else if( (!first || s->device.section.immediate) && isDisabled((struct Section *)s) ){
			struct MemoryStruct buff = EMPTY_MEMCHUNK;
			if(callAPI(&s->device, NULL, &buff)){	/* Call succeeded */
printf("**** OK : %s\n", buff.memory);
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
								struct json_object *val = getObj(obj,  OBJPATH( "value", NULL ) );
								printf("*** Found '%s' : %s\n", n, json_object_to_json_string(val));
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
				free(buff.memory);
		}

		struct timespec ts;
		ts.tv_sec = (time_t)s->device.section.sample;
		ts.tv_nsec = (unsigned long int)((s->device.section.sample - (time_t)s->device.section.sample) * 1e9);

		nanosleep( &ts, NULL );
	}
}
