/* Alerting.c
 * 	Handle SMS and Mail alerting
 *
 * This file is part of Marcel project and is following the same
 * license rules (see LICENSE file)
 *
 * 16/07/2015	- LF - First version
 * 24/02/2016	- LF - 4.5 - Add Notifications
 */

#include "Marcel.h"
#include "Alerting.h"

#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include <curl/curl.h>

struct DList alerts;

static void psendSMS(const char *url, const char *msg ){
	if(!url)
		return;

	CURL *curl;

	if((curl = curl_easy_init())){
		CURLcode res;
		char *emsg;
		char aurl[ strlen(url) + strlen( emsg=curl_easy_escape(curl,msg,0) ) ];	/* room for \0 provided by the %s replacement */

		sprintf( aurl, url, emsg );
		curl_free(emsg);

		curl_easy_setopt(curl, CURLOPT_URL, aurl);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);

		if((res = curl_easy_perform(curl)) != CURLE_OK)
			fprintf(stderr, "*E* Sending SMS : %s\n", curl_easy_strerror(res));

		curl_easy_cleanup(curl);
	}
}

static void sendSMS( const char *msg ){
	psendSMS( cfg.SMSurl, msg );
}

void AlertCmd( const char *id, const char *msg ){
	const char *p = cfg.AlertCmd;
	size_t nbre=0;	/* # of %t% in the string */

	if(!p)
		return;

	while((p = strstr(p, "%t%"))){
		nbre++;
		p += 3;	/* add '%t%' length */
	}

	char tcmd[ strlen(cfg.AlertCmd) + nbre*strlen(id) + 1 ];
	char *d = tcmd;
	p = cfg.AlertCmd;

	while(*p){
		if(*p =='%' && !strncmp(p, "%t%",3)){
			for(const char *s = id; *s; s++)
				if(*s =='"')
					*d++ = '\'';
				else
					*d++ = *s;
			p += 3;
		} else
			*d++ = *p++;
	}
	*d = 0;


	FILE *f = popen( tcmd, "w");
	if(!f){
		perror("popen()");
		return;
	}
	fputs(msg, f);
	fclose(f);
}

static struct alert *findalert(const char *id){
#ifdef DEBUG
printf("*d* lst - f:%p l:%p\n", alerts.first, alerts.last);
#endif
	for(struct alert *an = (struct alert *)alerts.first; an; an = (struct alert *)an->node.next){
#ifdef DEBUG
printf("*d*\t%p p:%p n:%p\n", an, an->node.prev, an->node.next);
#endif
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
/*
	if( MQTTClient_subscribe( cfg.client, "nAlert/#", 0) != MQTTCLIENT_SUCCESS ){
		fputs("Can't subscribe to 'nAlert/#'", stderr);
		exit(EXIT_FAILURE);
	}
*/

	if( MQTTClient_subscribe( cfg.client, "Notification/#", 0) != MQTTCLIENT_SUCCESS ){
		fputs("Can't subscribe to 'Notification/#'", stderr);
		exit(EXIT_FAILURE);
	}
	if( MQTTClient_subscribe( cfg.client, "nNotification/#", 0) != MQTTCLIENT_SUCCESS ){
		fputs("Can't subscribe to 'nNotification/#'", stderr);
		exit(EXIT_FAILURE);
	}

	if(!cfg.SMSurl && verbose)
		puts("*W* SMS sending not configured : disabling unamed SMS sending");
	if(!cfg.AlertCmd && verbose)
		puts("*W* Alert's command not configured : disabling unamed external alerting");
	if(!cfg.notiflist && verbose)
		puts("*W* No named notification configured : disabling");
}

void RiseAlert(const char *id, const char *msg, int withSMS){
	struct alert *an = findalert(id);

	if(!an){	/* It's a new alert */
		char smsg[ strlen(id) + strlen(msg) + 4 ];
		sprintf( smsg, "%s : %s", id, msg );
		if(withSMS)
			sendSMS( smsg );
		AlertCmd( id, msg );

		if(verbose)
			printf("*I* Alert sent : '%s' : '%s'\n", id, msg);

		assert( an = malloc( sizeof(struct alert) ) );
		assert( an->alert = strdup( id ) );
		DLAdd( &alerts, (struct DLNode *)an );
	}
}

void AlertIsOver(const char *id){
	struct alert *an = findalert(id);

	if(an){	/* The alert exists */
		char smsg[ strlen(id) + 13];
		sprintf( smsg, "%s : recovered", id );
		sendSMS( smsg );

		if(verbose)
			printf("*I* Alert cleared for '%s'\n", id);

		DLRemove( &alerts, (struct DLNode *)an );
		free( (void *)an->alert );
		free( an );
	}
}

void rcv_alert(const char *id, const char *msg){
	if(*msg == 'S' || *msg == 's' )	/* Rise an alert */
		RiseAlert(id, msg+1, *msg == 'S');
	else	/* Alert's over */
		AlertIsOver(id);
}

void rcv_notification(const char *id, const char *msg){
	char smsg[ strlen(id) + strlen(msg) + 4 ];
	sprintf( smsg, "%s : %s", id, msg+1);
	if(*msg == 'S')
		sendSMS( smsg );
	AlertCmd( id, msg+1 );

	if(verbose)
		printf("*I* Alert sent : '%s' : '%s'\n", id, msg+1);
}

void rcv_nnotification(const char *names, const char *msg){
	char *id = strchr(names, '/');

	if(!id){
		if(verbose)
			printf("*W* topic '%s' : No name provided, message ignored\n", names);
	} else {
		char c;
		*id++ = 0;
		if(verbose)
			printf("*I* Notifications named : '%s' id: '%s' msg : '%s'\n", names, id, msg);
		while(( c=*names++ )){
			for( struct notification *n = cfg.notiflist; n; n = n->next){
				if(c == n->id){
					if(n->url)
						psendSMS( n->url, msg );
					break;
				}
			}
		}
	}
}

