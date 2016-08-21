/* FFV.h
 * 	Definitions related to FFV task
 *
 * This file is part of Marcel project and is following the same
 * license rules (see LICENSE file)
 *
 * 07/06/2016	- LF - extracted from Marcel.c
 * 10/06/2016	- LF - Handle alarms
 */
#include "Marcel.h"
#include "FFV.h"
#include "MQTT_tools.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <dirent.h>

static void handle_FFV(struct _FFV *ctx){
	if(ctx->disabled){
		if(verbose)
			printf("*I* Reading FFV '%s' is disabled\n", ctx->topic);
		return;
	}

	FILE *f;
	char l[MAXLINE];

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
			val += ctx->offset;
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

					if(strlen(ctx->latch) + strlen(emsg = strerror(errno)) + 5 < MAXLINE - msg){ /* S + " : " + 0 */
						*(l + msg) = 'S';
						strcpy(l + msg + 1, ctx->latch);
						strcat(l + msg, " : ");
						strcat(l + msg, emsg);

						mqttpublish(cfg.client, l, strlen(l + msg), l + msg, 0);
					} else if( strlen(emsg) + 2 < MAXLINE - msg ){	/* S + error message */
						*(l + msg) = 'S';
						strcpy(l + msg + 1, emsg);

						mqttpublish(cfg.client, l, strlen(l + msg), l + msg, 0);
					} else {
						char *msg = "Can't open latch file (and not enough space for the error)";
						mqttpublish(cfg.client, l, strlen(msg), msg, 0);
					}
				}
			} else {	/* Reset the latch */
				fprintf(f, "1\n");
				fclose(f);
			}
		}
	}
}

void *process_FFV(void *actx){
	struct _FFV *ctx = actx;	/* Only to avoid zillions of cast */

		/* Sanity checks */
	if(!ctx->file){
		fprintf(stderr, "*E* [%s] configuration error : no file specified, ignoring this section\n", ctx->uid);
		pthread_exit(0);
	}

	if(verbose)
		printf("Launching a processing flow for FFV '%s'\n", ctx->uid);

	for(;;){	/* Infinite loop to process messages */
		ctx = actx;	/* Back to the 1st one */
		for(;;){
			handle_FFV(ctx);

			if(!(ctx = (struct _FFV *)ctx->next))	/* It was the last entry */
				break;
			if(ctx->section_type != MSEC_FFV || ctx->sample)	/* Not the same kind or new thread requested */
				break;
		}

		if(((struct _FFV *)actx)->sample == -1){
			if(verbose)
				printf("*I* FFV '%s' has -1 sample delay : dying ...\n", ((struct _FFV *)actx)->uid);
			break;
		} else 
			sleep( ((struct _FFV *)actx)->sample );
	}

	pthread_exit(0);
}


	/* Process 1w alarms */
void *process_1wAlrm(void *actx){
	for(;;){
		struct dirent *de;
		DIR *d = opendir( cfg.OwAlarm );
		if( !d ){
			perror( cfg.OwAlarm );
			pthread_exit(0);
		}

		while(( de = readdir(d) )){
			if( de->d_type == 4 && *de->d_name != '.' ){	/* 4 : directory */
				if( verbose )
					printf("*I* %s : in alert\n", de->d_name);

				for(union CSection *s = cfg.sections; s; s = s->common.next){
					if( s->common.section_type == MSEC_FFV && strstr(s->FFV.file, de->d_name)){
						if(s->FFV.sample == -1)
							handle_FFV( &s->FFV );
						else if( verbose )	/* Ignored as its sample is not -1 */
							fprintf(stderr, "*W* '%s' has alarm, is found but its sample time is not -1 : ignored\n", s->FFV.file);
					}
				}
			}
		}
		closedir(d);
		sleep( cfg.OwAlarmSample );
	}
	pthread_exit(0);
}
