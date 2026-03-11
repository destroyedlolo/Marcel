/* Request Devices from a TaHoma
 *
 * This file is part of Marcel project and is following the same
 * license rules (see LICENSE file)
 *
 * 11/03/2026 - LF - Creation
 */

#include "mod_TaHoma.h"
#include "../Marcel/MQTT_tools.h"
#include "../Marcel/CURL_helpers.h"

void *processDevice(void *actx){
	struct section_Device *s = (struct section_Device *)actx;

		/* Sanity checks */
	if(!s->TaHoma){
		publishLog('F', "[%s] No TaHoma defined. Dying ...", s->section.uid);
		SectionError((struct Section *)s, true);
		pthread_exit(0);
	}

	if(!s->url){
		publishLog('F', "[%s] No URL defined. Dying ...", s->section.uid);
		SectionError((struct Section *)s, true);
		pthread_exit(0);
	}

	if(!s->States){
		publishLog('F', "[%s] No state defined. Dying ...", s->section.uid);
		SectionError((struct Section *)s, true);
		pthread_exit(0);
	}
}
