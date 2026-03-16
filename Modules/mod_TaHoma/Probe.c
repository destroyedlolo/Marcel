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
printf("*** Querying %d\n", callAPI(&s->device, NULL, &res));
		}

		struct timespec ts;
		ts.tv_sec = (time_t)s->device.section.sample;
		ts.tv_nsec = (unsigned long int)((s->device.section.sample - (time_t)s->device.section.sample) * 1e9);

		nanosleep( &ts, NULL );
	}
}
