/* Freebox.c
 * 	Definitions related to Freebox figures handling
 *
 * This file is part of Marcel project and is following the same
 * license rules (see LICENSE file)
 *
 * 08/07/2015	- LF - start v2.0 - make source modular
 */
#ifdef FREEBOX

#include "Freebox.h"
#include "MQTT_tools.h"

#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <errno.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

void *process_Freebox(void *actx){
	struct _FreeBox *ctx = actx;	/* Only to avoid zillions of cast */

	char l[MAXLINE];
	struct hostent *server;
	struct sockaddr_in serv_addr;

		/* Sanity checks */
	if(!ctx->topic){
		fputs("*E* configuration error : no topic specified, ignoring this section\n", stderr);
		pthread_exit(0);
	}
	if(!(server = gethostbyname( FBX_HOST ))){
		perror( FBX_HOST );
		fputs("*E* Stopping this thread\n", stderr);
		pthread_exit(0);
	}

	memset( &serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons( FBX_PORT );
	memcpy(&serv_addr.sin_addr.s_addr,*server->h_addr_list,server->h_length);

	if(verbose)
		printf("Launching a processing flow for Freebox\n");

	for(;;){	/* Infinite loop to process data */
		if(ctx->disabled){
			if(verbose)
				printf("*I* Reading Freebox '%s' is disabled\n", ctx->topic);
		} else {
			int sockfd = socket(AF_INET, SOCK_STREAM, 0);
			if(sockfd < 0){
				fprintf(stderr, "*E* Can't create socket : %s\n", strerror( errno ));
				fputs("*E* Stopping this thread\n", stderr);
				pthread_exit(0);
			}
			if(connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0){
/*AF : Send error topic */
				perror("*E* Connecting");
			} else if( send(sockfd, FBX_REQ, strlen(FBX_REQ), 0) == -1 ){
/*AF : Send error topic */
				perror("*E* Sending");
			} else while( socketreadline(sockfd, l, sizeof(l)) != -1 ){
				if(strstr(l, "ATM")){
					int u, d, lm;
					if(sscanf(l+25,"%d", &d) != 1) d=-1;
					if(sscanf(l+44,"%d", &u) != 1) u=-1;

					lm = sprintf(l, "%s/DownloadATM", ctx->topic) + 2;
					assert( lm+1 < MAXLINE-10 );	/* Enough space for the response ? */
					sprintf( l+lm, "%d", d );
					mqttpublish( cfg.client, l, strlen(l+lm), l+lm, 0 );
					if(verbose)
						printf("Freebox : %s -> %s\n", l, l+lm);

					lm = sprintf(l, "%s/UploadATM", ctx->topic) + 2;
					assert( lm+1 < MAXLINE-10 );	/* Enough space for the response ? */
					sprintf( l+lm, "%d", u );
					mqttpublish( cfg.client, l, strlen(l+lm), l+lm, 0 );
					if(verbose)
						printf("Freebox : %s -> %s\n", l, l+lm);
				} else if(striKWcmp(l, "  Marge de bruit")){
					float u, d; 
					int lm;

					if(sscanf(l+25,"%f", &d) != 1) d = -1;
					if(sscanf(l+44,"%f", &u) != 1) u = -1;

					lm = sprintf(l, "%s/DownloadMarge", ctx->topic) + 2;
					assert( lm+1 < MAXLINE-10 );	/* Enough space for the response ? */
					sprintf( l+lm, "%.2f", d );
					mqttpublish( cfg.client, l, strlen(l+lm), l+lm, 0 );
					if(verbose)
						printf("Freebox : %s -> %s\n", l, l+lm);

					lm = sprintf(l, "%s/UploadMarge", ctx->topic) + 2;
					assert( lm+1 < MAXLINE-10 );	/* Enough space for the response ? */
					sprintf( l+lm, "%.2f", u );
					mqttpublish( cfg.client, l, strlen(l+lm), l+lm, 0 );
					if(verbose)
						printf("Freebox : %s -> %s\n", l, l+lm);
				} else if(striKWcmp(l, "  WAN")){
					int u, d, lm;

					if(sscanf(l+40,"%d", &d) != 1) d = -1;
					if(sscanf(l+55,"%d", &u) != 1) u = -1;

					lm = sprintf(l, "%s/DownloadWAN", ctx->topic) + 2;
					assert( lm+1 < MAXLINE-10 );	/* Enough space for the response ? */
					sprintf( l+lm, "%d", d );
					mqttpublish( cfg.client, l, strlen(l+lm), l+lm, 0 );
					if(verbose)
						printf("Freebox : %s -> %s\n", l, l+lm);

					lm = sprintf(l, "%s/UploadWAN", ctx->topic) + 2;
					assert( lm+1 < MAXLINE-10 );	/* Enough space for the response ? */
					sprintf( l+lm, "%d", u );
					mqttpublish( cfg.client, l, strlen(l+lm), l+lm, 0 );
					if(verbose)
						printf("Freebox : %s -> %s\n", l, l+lm);
				} else if(striKWcmp(l, "  Ethernet")){
					int u, d, lm;

					if(sscanf(l+40,"%d", &d) != 1) d = -1;
					if(sscanf(l+55,"%d", &u) != 1) u = -1;

					lm = sprintf(l, "%s/DownloadTV", ctx->topic) + 2;
					assert( lm+1 < MAXLINE-10 );	/* Enough space for the response ? */
					sprintf( l+lm, "%d", d );
					mqttpublish( cfg.client, l, strlen(l+lm), l+lm, 0 );
					if(verbose)
						printf("Freebox : %s -> %s\n", l, l+lm);

					lm = sprintf(l, "%s/UploadTV", ctx->topic) + 2;
					assert( lm+1 < MAXLINE-10 );	/* Enough space for the response ? */
					sprintf( l+lm, "%d", u );
					mqttpublish( cfg.client, l, strlen(l+lm), l+lm, 0 );
					if(verbose)
						printf("Freebox : %s -> %s\n", l, l+lm);
				} else if(striKWcmp(l, "  USB")){
					int u, d, lm;

					if(sscanf(l+40,"%d", &d) != 1) d = -1;
					if(sscanf(l+55,"%d", &u) != 1) u = -1;

					lm = sprintf(l, "%s/DownloadUSB", ctx->topic) + 2;
					assert( lm+1 < MAXLINE-10 );	/* Enough space for the response ? */
					sprintf( l+lm, "%d", d );
					mqttpublish( cfg.client, l, strlen(l+lm), l+lm, 0 );
					if(verbose)
						printf("Freebox : %s -> %s\n", l, l+lm);

					lm = sprintf(l, "%s/UploadUSB", ctx->topic) + 2;
					assert( lm+1 < MAXLINE-10 );	/* Enough space for the response ? */
					sprintf( l+lm, "%d", u );
					mqttpublish( cfg.client, l, strlen(l+lm), l+lm, 0 );
					if(verbose)
						printf("Freebox : %s -> %s\n", l, l+lm);
				} else if(striKWcmp(l, "  Switch")){
					int u, d, lm;

					if(sscanf(l+40,"%d", &d) != 1) d = -1;
					if(sscanf(l+55,"%d", &u) != 1) u = -1;

					lm = sprintf(l, "%s/DownloadLan", ctx->topic) + 2;
					assert( lm+1 < MAXLINE-10 );	/* Enough space for the response ? */
					sprintf( l+lm, "%d", d );
					mqttpublish( cfg.client, l, strlen(l+lm), l+lm, 0 );
					if(verbose)
						printf("Freebox : %s -> %s\n", l, l+lm);

					lm = sprintf(l, "%s/UploadLan", ctx->topic) + 2;
					assert( lm+1 < MAXLINE-10 );	/* Enough space for the response ? */
					sprintf( l+lm, "%d", u );
					mqttpublish( cfg.client, l, strlen(l+lm), l+lm, 0 );
					if(verbose)
						printf("Freebox : %s -> %s\n", l, l+lm);
				}
			}

			close(sockfd);
		}
		sleep( ctx->sample );
	}

	pthread_exit(0);
}

#endif
