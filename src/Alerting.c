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

static void psendSMS(const char *url, const char *title, const char *msg ){
	if(!url)
		return;

	CURL *curl;

	if((curl = curl_easy_init())){
			/* Calculate the number of replacement to do */
		size_t nbrem = 0, nbret = 0;
		const char *p = url;
		while((p = strstr(p, "%m%"))){	/* Messages */
			nbrem++;
			p += 3;	/* add '%m%' length */
		}

		p = url;
		while((p = strstr(p, "%t%"))){	/* Title */
			nbret++;
			p += 3;	/* add '%t%' length */
		}

		CURLcode res;
		char *emsg, *etitle;
		char aurl[ strlen(url) + 
			nbrem * strlen( emsg=curl_easy_escape(curl,msg,0) ) +
			nbret * strlen( etitle=curl_easy_escape(curl,title,0) ) + 1
		];

		char *d = aurl;
		p = url;

		while(*p){
			const char *what = NULL;

			if(*p =='%'){	/* Is it a token to replace ? */
				if(!strncmp(p, "%t%",3))
					what = etitle;
				else if(!strncmp(p, "%m%",3))
					what = emsg;
			}

			if(what){	/* we got a token */
				for(const char *s = what; *s; s++)
					if(*s =='"')
						*d++ = '\'';
					else
						*d++ = *s;
				p += 3;
			} else	/* not a token ... copying */
				*d++ = *p++;
		}
		*d = 0;

		curl_free(emsg);
		curl_free(etitle);

		curl_easy_setopt(curl, CURLOPT_URL, aurl);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);

		if((res = curl_easy_perform(curl)) != CURLE_OK)
			publishLog('E', "Sending SMS : %s", curl_easy_strerror(res));

		curl_easy_cleanup(curl);
	}
}

static void sendSMS( const char *title, const char *msg ){
	psendSMS( cfg.SMSurl, title, msg );
}

void pAlertCmd( const char *cmd, const char *id, const char *msg ){
	const char *p = cmd;
	size_t nbre=0;	/* # of %t% in the string */

	if(!p)
		return;

	while((p = strstr(p, "%t%"))){
		nbre++;
		p += 3;	/* add '%t%' length */
	}

	char tcmd[ strlen(cfg.AlertCmd) + nbre*strlen(id) + 1 ];
	char *d = tcmd;
	p = cmd;

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

void AlertCmd( const char *id, const char *msg ){
	pAlertCmd(cfg.AlertCmd, id, msg);
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
		publishLog('F', "Can't subscribe to 'Alert/#'");
		exit(EXIT_FAILURE);
	}
/*
	if( MQTTClient_subscribe( cfg.client, "nAlert/#", 0) != MQTTCLIENT_SUCCESS ){
		publishLog('F', "Can't subscribe to 'nAlert/#'");
		exit(EXIT_FAILURE);
	}
*/

	if( MQTTClient_subscribe( cfg.client, "Notification/#", 0) != MQTTCLIENT_SUCCESS ){
		publishLog('F', "Can't subscribe to 'Notification/#'");
		exit(EXIT_FAILURE);
	}
	if( MQTTClient_subscribe( cfg.client, "nNotification/#", 0) != MQTTCLIENT_SUCCESS ){
		publishLog('F', "Can't subscribe to 'nNotification/#'");
		exit(EXIT_FAILURE);
	}

	if(!cfg.SMSurl)
		publishLog('W', "SMS sending not configured : disabling unamed SMS sending");
	if(!cfg.AlertCmd)
		publishLog('W', "Alert's command not configured : disabling unamed external alerting");
	if(!cfg.notiflist)
		publishLog('W', "No named notification configured : disabling");
}

void SendAlert(const char *id, const char *msg, int withSMS){
	char smsg[ strlen(id) + strlen(msg) + 4 ];
	sprintf( smsg, "%s : %s", id, msg );
	if(withSMS)
		sendSMS( id, smsg );
	AlertCmd( id, msg );

	publishLog('T', "Alert sent : '%s' : '%s'", id, msg);
}

void RiseAlert(const char *id, const char *msg, int withSMS){
	struct alert *an = findalert(id);

	if(!an){	/* It's a new alert */
		SendAlert(id, msg, withSMS);
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
		sendSMS( id, smsg );

		publishLog('T', "Alert cleared for '%s'", id);

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
		sendSMS( id, smsg );
	AlertCmd( id, msg+1 );

	publishLog('T', "Alert sent : '%s' : '%s'", id, msg+1);
}

void pnNotify(const char *names, const char *title, const char *msg){
/* names : names to notify to
 * title : notification's title
 * msg : messages to send
 */
	publishLog('T', "Named Notifications : '%s' id: '%s' msg : '%s'", names, title, msg);
	char c;
	while(( c=*names++ )){
		for( struct notification *n = cfg.notiflist; n; n = n->next){
			if(c == n->id){
				if(n->url)
					psendSMS( n->url, title, msg );
				if(n->cmd)
					pAlertCmd( n->cmd, title, msg );
				break;
			}
		}
	}
}

void rcv_nnotification(const char *names, const char *msg){
	char *id = strchr(names, '/');

	if(!id){
		publishLog('W', "topic '%s' : No name provided, message ignored", names);
	} else {
		*id++ = 0;
		pnNotify(names, id, msg);
	}
}

