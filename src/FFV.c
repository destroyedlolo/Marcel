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

#ifdef LUA
#include <lauxlib.h>
#endif

static void handle_FFV(struct _FFV *ctx){
	if(ctx->disabled){
		publishLog('T', "[%s] Reading FFV '%s' is disabled\n", ctx->uid, ctx->topic);
		return;
	}

#ifdef LUA
	if(ctx->failfunc && ctx->failfuncid == LUA_REFNIL){
		if(!cfg.luascript){
			publishLog('E', "[%s] configuration error : No Lua script defined whereas a function is used. This thread is dying.", ctx->uid, ctx->funcname);
			pthread_exit(NULL);
		}
		
		if( (ctx->failfuncid = findUserFunc( ctx->failfunc )) == LUA_REFNIL ){
			publishLog('E', "[%s] configuration error : user function \"%s\" is not defined", ctx->uid, ctx->failfunc);
			publishLog('F', "[%s] Dying", ctx->uid);
			pthread_exit(NULL);
		}
	}

	if(ctx->funcname && ctx->funcid == LUA_REFNIL){
		if(!cfg.luascript){
			publishLog('E', "[%s] configuration error : No Lua script defined whereas a function is used. This thread is dying.", ctx->uid, ctx->funcname);
			pthread_exit(NULL);
		}
		
		if( (ctx->funcid = findUserFunc( ctx->funcname )) == LUA_REFNIL ){
			publishLog('E', "[%s] configuration error : user function \"%s\" is not defined", ctx->uid, ctx->funcname);
			publishLog('F', "[%s] Dying", ctx->uid);
			pthread_exit(NULL);
		}
	}
#endif

	FILE *f;
	char l[MAXLINE];

	if(!(f = fopen( ctx->file, "r" ))){
		char *emsg = strerror(errno);
		publishLog('E', "[%s] %s : %s", ctx->uid, ctx->file, emsg);
		if(strlen(ctx->topic) + 7 < MAXLINE){  /* "/Alarm" +1 */
			int msg;
			strcpy(l, "Alarm/");
			strcat(l, ctx->topic);
			msg = strlen(l) + 2;

			if(strlen(ctx->file) + strlen(emsg) + 5 < MAXLINE - msg){ /* S + " : " + 0 */
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

#ifdef LUA
		if( ctx->failfuncid != LUA_REFNIL )
			executeFailFunc((union CSection *)ctx, emsg);
#endif

	} else {
		float val;
		if(!fscanf(f, "%f", &val))
			publishLog('E', "[%s] : %s -> Unable to read a float value.", ctx->uid, ctx->topic);
		else {	/* Only to normalize the response */
			bool publish = true;
			float compensated = val + ctx->offset;

			if( ctx->safe85 && val == 85.0 )
				publishLog('E', "[%s] : %s -> The probe replied 85° implying powering issue.", ctx->uid, ctx->topic);
			else {
#ifdef LUA
				if( ctx->funcid != LUA_REFNIL )
					publish = execUserFuncFFV(ctx, val, compensated);
#endif
				if(publish){
					publishLog('T', "[%s] : %s -> %f", ctx->uid, ctx->topic, compensated);
					sprintf(l,"%.1f", compensated);
					mqttpublish(cfg.client, ctx->topic, strlen(l), l, ctx->retained );
				} else
					publishLog('T', "[%s] UserFunction requested not to publish", ctx->uid);
			}
		}
		fclose(f);

		if(ctx->latch){
			if(!(f = fopen( ctx->latch, "w" ))){
				char *emsg = strerror(errno);
				publishLog('E', "[%s] %s : %s", ctx->uid, ctx->latch, emsg);
				if(strlen(ctx->topic) + 7 < MAXLINE){  /* "/Alarm" +1 */
					int msg;
					strcpy(l, "Alarm/");
					strcat(l, ctx->topic);
					msg = strlen(l) + 2;

					if(strlen(ctx->latch) + strlen(emsg) + 5 < MAXLINE - msg){ /* S + " : " + 0 */
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
		publishLog('F', "[%s] configuration error : no file specified, ignoring this section", ctx->uid);
		pthread_exit(0);
	}

	if(cfg.Randomize){
		uint16_t t = (uint16_t)pthread_self();
		publishLog('I', "Dailying FFV '%s' by %d", ctx->uid, t % ((struct _FFV *)actx)->sample);
		sleep( t % ((struct _FFV *)actx)->sample );
	}
		
	publishLog('I', "Launching a processing flow for FFV '%s'", ctx->uid);

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
			publishLog('T', "FFV '%s' has -1 sample delay : dying ...", ((struct _FFV *)actx)->uid);
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
			publishLog(cfg.OwAlarmKeep ? 'E' : 'F', "[1-wire Alarm] : %s", strerror(errno));
			if(!cfg.OwAlarmKeep)
				pthread_exit(0);
		} else {
			while(( de = readdir(d) )){
				if( de->d_type == 4 && *de->d_name != '.' ){	/* 4 : directory */
					publishLog('T', "%s : in alert", de->d_name);

					for(union CSection *s = cfg.sections; s; s = s->common.next){
						if( s->common.section_type == MSEC_FFV && strstr(s->FFV.file, de->d_name)){
							if(s->FFV.sample == -1)
								handle_FFV( &s->FFV );
							else /* Ignored as its sample is not -1 */
								publishLog('W', "'%s' has alarm, is found but its sample time is not -1 : ignored", s->FFV.file);
						}
					}
				}
			}
			closedir(d);
		}
		sleep( cfg.OwAlarmSample );
	}
	pthread_exit(0);
}
