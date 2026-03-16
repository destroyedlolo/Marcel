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
		} else if( !first || s->device.section.immediate ){
			struct MemoryStruct res = EMPTY_MEMCHUNK;
			if(callAPI(&s->device, NULL, &res)){	/* Call succeeded */
printf("**** OK : %s\n", res.memory);
			} else if(cfg.debug){
				if(res.memory)	/* We're in error but a response may have been provided */
					publishLog('d', "[%s] response \"%s\"", s->device.section.uid, res.memory);
			}

			if(res.memory)
				free(res.memory);
		}

		struct timespec ts;
		ts.tv_sec = (time_t)s->device.section.sample;
		ts.tv_nsec = (unsigned long int)((s->device.section.sample - (time_t)s->device.section.sample) * 1e9);

		nanosleep( &ts, NULL );
	}
}
