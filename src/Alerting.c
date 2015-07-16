/* Alerting.c
 * 	Handle SMS alerting
 *
 * This file is part of Marcel project and is following the same
 * license rules (see LICENSE file)
 *
 * 16/07/2015	- LF - First version
 */

#include "Marcel.h"
#include "Alerting.h"

#include <stdlib.h>
#include <assert.h>

struct DList alerts;

static struct alert *findalert(const char *id){
printf("*d* f:%p l:%p\n", alerts.first, alerts.last);
	return NULL;
}

void init_alerting(void){
	DLListInit( &alerts );

	if( MQTTClient_subscribe( cfg.client, "Alert/#", 0) != MQTTCLIENT_SUCCESS ){
		fputs("Can't subscribe to 'Alert/#'", stderr);
		exit(EXIT_FAILURE);
	}
}

void rcv_alert(const char *id, const char *msg){
	struct alert *al = findalert(id);
printf("*d* Alert '%s'/'%s'\n", id, msg);

	if(*msg == 'S'){	/* rise this alert */
		if(!al){	/* And it's a new one */
			assert( al = malloc( sizeof(struct alert) ) );
			assert( al->alert = strdup( id ) );

		}
	} else {	/* Alert's over */
	}
}
