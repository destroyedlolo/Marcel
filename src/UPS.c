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
	if(!ctx->topic || !ctx->section_name){
		fputs("*E* configuration error : section name or topic specified, ignoring this section\n", stderr);
		pthread_exit(0);
	}
	if(!ctx->host || !ctx->port){
		fputs("*E* configuration error : Don't know how to reach NUT\n", stderr);
		pthread_exit(0);
	}

	if(!(server = gethostbyname( ctx->host ))){
		perror( ctx->host );
		fputs("*E* Stopping this thread\n", stderr);
		pthread_exit(0);
	}

	memset( &serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons( ctx->port );
	memcpy(&serv_addr.sin_addr.s_addr,*server->h_addr_list,server->h_length);

	if(debug)
		printf("Launching a processing flow for UPS/%s\n", ctx->section_name);

	for(;;){	/* Infinite loop to process data */
		int sockfd = socket(AF_INET, SOCK_STREAM, 0);

		if(sockfd < 0){
			fprintf(stderr, "*E* Can't create socket : %s\n", strerror( errno ));
			fputs("*E* Stopping this thread\n", stderr);
			pthread_exit(0);
		}
		if(connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0){
/*AF : Send error topic */
			perror("*E* Connecting");
		} else {
			for(struct var *v = ctx->var_list; v; v = v->next){
				sprintf(l, "GET VAR %s %s\n", ctx->section_name, v->name);
				if( send(sockfd, l , strlen(l), 0) == -1 ){
/*AF : Send error topic */
					perror("*E* Sending");
				} else {
					char *ps, *pe;
					socketreadline(sockfd, l, sizeof(l));
					if(!( ps = strchr(l, '"')) || !( pe = strchr(ps+1, '"') )){
						if(debug)
							printf("*E* %s/%s : unexpected result '%s'\n", ctx->section_name, v->name, l);
					} else {
						ps++; *pe++ = 0;	/* Extract only the result */
						assert(pe - l + strlen(ctx->topic) + strlen(v->name) + 2 < MAXLINE ); /* ensure there is enough place for the topic name */
						sprintf( pe, "%s/%s", ctx->topic, v->name );
						papub( pe, strlen(ps), ps, 0 );
						if(debug)
							printf("UPS : %s -> '%s'\n", pe, ps);
					}
				}
			}
			close(sockfd);
			sleep( ctx->sample );
		}
	}
	pthread_exit(0);
}
#endif
