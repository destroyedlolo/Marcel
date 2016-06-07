/* FFV.h
 * 	Definitions related to FFV task
 *
 * This file is part of Marcel project and is following the same
 * license rules (see LICENSE file)
 *
 * 07/06/2016	- LF - extracted from Marcel.c
 */
#include "Marcel.h"
#include "FFV.h"
#include "MQTT_tools.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

void *process_FFV(void *actx){
	struct _FFV *ctx = actx;	/* Only to avoid zillions of cast */
	FILE *f;
	char l[MAXLINE];

		/* Sanity checks */
	if(!ctx->topic){
		fputs("*E* configuration error : no topic specified, ignoring this section\n", stderr);
		pthread_exit(0);
	}
	if(!ctx->file){
		fprintf(stderr, "*E* configuration error : no file specified for '%s', ignoring this section\n", ctx->topic);
		pthread_exit(0);
	}

	if(verbose)
		printf("Launching a processing flow for FFV '%s'\n", ctx->topic);

	for(;;){	/* Infinite loop to process messages */
		ctx = actx;	/* Back to the 1st one */
		for(;;){
			if(!(f = fopen( ctx->file, "r" ))){
				if(verbose)
					perror( ctx->file );
				if(strlen(ctx->topic) + 7 < MAXLINE){  /* "/Alarm" +1 */
					int msg;
					char *emsg;
					strcpy(l, "Alarm/");
					strcat(l, ctx->topic);
					msg = strlen(l) + 2;

					if(strlen(ctx->file) + strlen(emsg = strerror(errno)) + 5 < MAXLINE - msg){ /* S + " : " + 0 */
						*(l + msg) = 'S';
						strcpy(l + msg + 1, ctx->file);
						strcat(l + msg, " : ");
						strcat(l + msg, emsg);

						mqttpublish(cfg.client, l, strlen(l + msg), l + msg, 0);
					} else if( strlen(emsg) + 2 < MAXLINE - msg ){	/* S + error message */
						*(l + msg) = 'S';
						strcpy(l + msg + 1, emsg);

						mqttpublish(cfg.client, l, strlen(l + msg), l + msg, 0);
					} else {
						char *msg = "Can't open file (and not enough space for the error)";
						mqttpublish(cfg.client, l, strlen(msg), msg, 0);
					}
				}
			} else {
				float val;
				if(!fscanf(f, "%f", &val)){
					if(verbose)
						printf("FFV : %s -> Unable to read a float value.\n", ctx->topic);
				} else {	/* Only to normalize the response */
					sprintf(l,"%.1f", val);

					mqttpublish(cfg.client, ctx->topic, strlen(l), l, 0 );
					if(verbose)
						printf("FFV : %s -> %f\n", ctx->topic, val);
				}
				fclose(f);

				if(ctx->latch){
					if(!(f = fopen( ctx->latch, "w" ))){
						if(verbose)
							perror( ctx->latch );
						if(strlen(ctx->topic) + 7 < MAXLINE){  /* "/Alarm" +1 */
							int msg;
							char *emsg;
							strcpy(l, "Alarm/");
							strcat(l, ctx->topic);
							msg = strlen(l) + 2;

							if(strlen(ctx->file) + strlen(emsg = strerror(errno)) + 5 < MAXLINE - msg){ /* S + " : " + 0 */
								*(l + msg) = 'S';
								strcpy(l + msg + 1, ctx->file);
								strcat(l + msg, " : ");
								strcat(l + msg, emsg);

								mqttpublish(cfg.client, l, strlen(l + msg), l + msg, 0);
							} else if( strlen(emsg) + 2 < MAXLINE - msg ){	/* S + error message */
								*(l + msg) = 'S';
								strcpy(l + msg + 1, emsg);

								mqttpublish(cfg.client, l, strlen(l + msg), l + msg, 0);
							} else {
								char *msg = "Can't open file (and not enough space for the error)";
								mqttpublish(cfg.client, l, strlen(msg), msg, 0);
							}
						}
					} else {	/* Reset the latch */
						fprintf(f, "1\n");
						fclose(f);
					}
				}
			}

			if(!(ctx = (struct _FFV *)ctx->next))	/* It was the last entry */
				break;
			if(ctx->section_type != MSEC_FFV || ctx->sample)	/* Not the same kind or new thread requested */
				break;
		}

		if(((struct _FFV *)actx)->sample == -1){
			if(verbose)
				printf("*I* FFV '%s' has -1 sample delay : dying ...\n", ((struct _FFV *)actx)->topic);
			break;
		} else 
			sleep( ((struct _FFV *)actx)->sample );
	}

	pthread_exit(0);
}


