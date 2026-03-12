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

		/* Verify states' sanity */
	for(struct State_definition *st = s->States; st; st = st->next){
		if(!st->topic){
			publishLog('F', "[%s] State \"%s\" has no topic defined. Dying ...", s->section.uid, st->state);
			SectionError((struct Section *)s, true);
			pthread_exit(0);
		}
	}

		/* Verify TaHoma's */
	s->gateway = (struct section_TaHoma *)findSectionByName(s->TaHoma);
	if(!s->gateway || strcmp(s->gateway->section.kind, "TaHoma")){
		publishLog('F', "[%s] TaHoma \"%s\" not found. Dying ...", s->section.uid, s->TaHoma);
		SectionError((struct Section *)s, true);
		pthread_exit(0);
	}

	if(!s->gateway->hostname || !s->gateway->ip || !s->gateway->token){
		publishLog('F', "[%s] TaHoma \"%s\" misses some parameters. Dying ...", s->section.uid, s->TaHoma);
		SectionError((struct Section *)s, true);
		pthread_exit(0);
	}
}
