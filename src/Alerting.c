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
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

struct DList alerts;

static void sendSMS( const char *msg ){
/* Code comes from Jerry Jeremiah's 
 * http://stackoverflow.com/questions/22077802/simple-c-example-of-doing-an-http-post-and-consuming-the-response
 */
	struct hostent *server;
    struct sockaddr_in serv_addr;
    int sockfd, bytes, sent, received, total;

	if(!cfg.ErrorSMS.Host)
		return;

puts(msg);

	/* lookup the ip address */
	if(!(server = gethostbyname( cfg.ErrorSMS.Host ))){
		fputs("*E* Can't find SMS host : disabling SMS sending\n", stderr);
		cfg.ErrorSMS.Host = NULL;
		return;
	}

	/* create the socket */
	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		perror("SMS' Socket()");
		exit(EXIT_FAILURE);
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

	if(!cfg.ErrorSMS.Host || !cfg.ErrorSMS.Port || !cfg.ErrorSMS.Page || !cfg.ErrorSMS.Payload){
		if(debug)
			puts("*W* SMS sending not fully configured : disabling SMS sending");
		cfg.ErrorSMS.Host = NULL;
	}
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
			DLRemove( &alerts, (struct DLNode *)an );

			free( (void *)an->alert );
			free( an );
		}
	}
}
