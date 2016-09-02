/* UPS.c
 * 	Definitions related to UPS figures handling
 *
 * This file is part of Marcel project and is following the same
 * license rules (see LICENSE file)
 *
 * 08/07/2015	- LF - start v2.0 - make source modular
 */
#ifdef UPS

#include "UPS.h"
#include "MQTT_tools.h"

#include <string.h>
#include <errno.h>
#include <assert.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

void *process_UPS(void *actx){
	struct _UPS *ctx = actx;	/* Only to avoid zillions of cast */
	char l[MAXLINE];
	struct hostent *server;
	struct sockaddr_in serv_addr;

		/* Sanity checks */
	if(!ctx->host || !ctx->port){
		publishLog('E', "[%s] configuration error : Don't know how to reach NUT", ctx->uid);
		pthread_exit(0);
	}

	if(!(server = gethostbyname( ctx->host ))){
		publishLog('E', "[%s] %s : Don't know how to reach NUT. Dying", ctx->uid, strerror(errno));
		pthread_exit(0);
	}

	memset( &serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons( ctx->port );
	memcpy(&serv_addr.sin_addr.s_addr,*server->h_addr_list,server->h_length);

	publishLog('I', "Launching a processing flow for UPS/%s", ctx->uid);

	for(;;){	/* Infinite loop to process data */
		if(ctx->disabled){
			publishLog('I', "Reading Freebox '%s' is disabled", ctx->topic);
		} else {
			int sockfd = socket(AF_INET, SOCK_STREAM, 0);
			if(sockfd < 0){
				publishLog('E', "[%s] Can't create socket : %s", ctx->topic, strerror( errno ));
				if(!ctx->keep){
					publishLog('F', "[%s] Dying", ctx->topic);
					pthread_exit(0);
				}
			} else {
				if(connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0)
					publishLog('E', "[%s] Connecting : %s", ctx->topic, strerror( errno ));
				else {
					for(struct var *v = ctx->var_list; v; v = v->next){
						sprintf(l, "GET VAR %s %s\n", ctx->uid, v->name);
						if( send(sockfd, l , strlen(l), 0) == -1 ){
							publishLog('E', "[%s] Sending : %s", ctx->topic, strerror( errno ));
						} else {
							char *ps, *pe;
							socketreadline(sockfd, l, sizeof(l));
							if(!( ps = strchr(l, '"')) || !( pe = strchr(ps+1, '"') ))
								publishLog('W', "[%s] %s : unexpected result '%s'", ctx->uid, v->name, l);
							else {
								ps++; *pe++ = 0;	/* Extract only the result */
								assert(pe - l + strlen(ctx->topic) + strlen(v->name) + 2 < MAXLINE ); /* ensure there is enough place for the topic name */
								sprintf( pe, "%s/%s", ctx->topic, v->name );
								mqttpublish( cfg.client, pe, strlen(ps), ps, 0 );
								publishLog('I', "[%s] UPS : %s -> '%s'", ctx->uid, pe, ps);
							}
						}
					}
				}
				close(sockfd);
			}
		}
		sleep( ctx->sample );
	}
	pthread_exit(0);
}
#endif
