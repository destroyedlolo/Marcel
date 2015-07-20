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

#include <curl/curl.h>

struct DList alerts;

static void sendSMS( const char *msg ){
	CURL *curl;
		
	if(!cfg.SMSurl)
		return;

	if((curl = curl_easy_init())){
		CURLcode res;
		char *emsg;
		char aurl[ strlen(cfg.SMSurl) + strlen( emsg=curl_easy_escape(curl,msg,0) ) ];	/* room for \0 provided by the %s replacement */

		sprintf( aurl, cfg.SMSurl, emsg );
		curl_free(emsg);

		curl_easy_setopt(curl, CURLOPT_URL, aurl);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);

		if((res = curl_easy_perform(curl)) != CURLE_OK)
			fprintf(stderr, "*E* Sending SMS : %s\n", curl_easy_strerror(res));

		curl_easy_cleanup(curl);
	}
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

	if(!cfg.SMSurl && verbose)
		puts("*W* SMS sending not fully configured : disabling SMS sending");

	curl_global_init(CURL_GLOBAL_ALL);
	atexit( curl_global_cleanup );
}

void rcv_alert(const char *id, const char *msg){
	struct alert *an = findalert(id);
printf("*d* Alert '%s'/'%s' (an:%p)\n", id, msg, an);

	if(*msg == 'S'){	/* rise this alert */
		if(!an){	/* And it's a new one */
			char smsg[ strlen(id) + strlen(msg) + 3 ];	/* \0 replaces the leading msg character */
			sprintf( smsg, "%s : %s", id, msg+1 );
			sendSMS( smsg );

			assert( an = malloc( sizeof(struct alert) ) );
			assert( an->alert = strdup( id ) );

			DLAdd( &alerts, (struct DLNode *)an );
		}
	} else {	/* Alert's over */
		if(an){
			char smsg[ strlen(id) + 13];
			sprintf( smsg, "%s : recovered", id );
			sendSMS( smsg );

			DLRemove( &alerts, (struct DLNode *)an );

			free( (void *)an->alert );
			free( an );
		}
	}
}
