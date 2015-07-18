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
#include <string.h>

struct DList alerts;

static void sendSMS( const char *msg ){
/* Code comes from Jerry Jeremiah's 
 * http://stackoverflow.com/questions/22077802/simple-c-example-of-doing-an-http-post-and-consuming-the-response
 */

}

static struct alert *findalert(const char *id){
printf("*d* lst - f:%p l:%p\n", alerts.first, alerts.last);
	for(struct alert *an = (struct alert *)alerts.first; an; an = (struct alert *)an->node.next){
printf("*d*\t%p p:%p n:%p\n", an, an->node.prev, an->node.next);
		if(!strcmp(id, an->alert))
			return an;
	}

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
	struct alert *an = findalert(id);
printf("*d* Alert '%s'/'%s' (an:%p)\n", id, msg, an);

	if(*msg == 'S'){	/* rise this alert */
		if(!an){	/* And it's a new one */
			assert( an = malloc( sizeof(struct alert) ) );
			assert( an->alert = strdup( id ) );

			DLAdd( &alerts, (struct DLNode *)an );
		}
	} else {	/* Alert's over */
		if(an){
			DLRemove( &alerts, (struct DLNode *)an );

			free( (void *)an->alert );
			free( an );
		}
	}
}
