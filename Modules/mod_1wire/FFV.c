/* FFV.c
 *
 * Handle flat file value
 *
 * This file is part of Marcel project and is following the same
 * license rules (see LICENSE file)
 *
 * 11/10/2022 - LF - First version
 */

#include "mod_1wire.h"	/* module's own stuffs */
#ifdef LUA
#	include "../mod_Lua/mod_Lua.h"
#endif
#include "../Marcel/MQTT_tools.h"

#include <errno.h>
#include <stdlib.h>

void *processFFV(void *actx){
	struct section_FFV *s = (struct section_FFV *)actx;

		/* Sanity checks */
	if(!s->common.section.topic){
		publishLog('F', "[%s] Topic must be set. Dying ...", s->common.section.uid);
		pthread_exit(0);
	}

	if(!s->common.section.sample){
		publishLog('E', "[%s] Sample time can't be 0. Dying ...", s->common.section.uid);
		pthread_exit(0);
	}

	if(!s->common.file){
		publishLog('E', "[%s] File must be set. Dying ...", s->common.section.uid);
		pthread_exit(0);
	}

#ifdef LUA
			/* Handle Lua functions */
	struct module_Lua *mod_Lua = NULL;
	uint8_t mod_Lua_id = findModuleByName("mod_Lua");	/* Is mod_Lua loaded ? */
	if(mod_Lua_id != (uint8_t)-1){
		mod_Lua = (struct module_Lua *)modules[mod_Lua_id];
		if(s->common.section.funcname){	/* if an user function defined ? */
			if( (s->common.section.funcid = mod_Lua->findUserFunc(s->common.section.funcname)) == LUA_REFNIL ){
				publishLog('E', "[%s] configuration error : user function \"%s\" is not defined. This thread is dying.", s->common.section.uid, s->common.section.funcname);
				pthread_exit(NULL);
			}
		}

		if(s->common.failfunc){	/* if an user function defined ? */
			if( (s->common.failfuncid = mod_Lua->findUserFunc(s->common.failfunc)) == LUA_REFNIL ){
				publishLog('E', "[%s] configuration error : fail function \"%s\" is not defined. This thread is dying.", s->common.section.uid, s->common.failfunc);
				pthread_exit(NULL);
			}
		}
	}
#endif

	for(bool first=true;; first=false){	/* Infinite publishing loop */
		if(s->common.section.disabled){
#ifdef DEBUG
			if(cfg.debug)
				publishLog('d', "[%s] is disabled", s->common.section.uid);
#endif
		} else if( !first || s->common.section.immediate || s->common.section.sample == -1 ){	/* processing */
			FILE *f;
			char l[MAXLINE];

			if(!(f = fopen( s->common.file, "r" ))){	/* probe is not reachable */
				char *emsg = strerror(errno);
				publishLog('E', "[%s] %s : %s", s->common.section.uid, s->common.file, emsg);

				if(strlen(s->common.section.topic) + 7 < MAXLINE){  /* "/Alarm" +1 */
					int msg;
					strcpy(l, "Alarm/");
					strcat(l, s->common.section.topic);
					msg = strlen(l) + 2;

					if(strlen(s->common.file) + strlen(emsg) + 5 < MAXLINE - msg){ /* S + " : " + 0 */
						*(l + msg) = 'S';
						strcpy(l + msg + 1, s->common.file);
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
				} else {
					char *msg = "Can't open file (and not enough space for the error)";
					mqttpublish(cfg.client, l, strlen(msg), msg, 0);
				}

#ifdef LUA
				if(s->common.failfuncid != LUA_REFNIL){
					mod_Lua->lockState();
					mod_Lua->pushFunctionId( s->common.failfuncid );
					mod_Lua->pushString( s->common.section.uid );
					mod_Lua->pushString( emsg );
					if(mod_Lua->exec(2, 0)){
						publishLog('E', "[%s] FFV failfunction : %s", s->common.section.uid, mod_Lua->getStringFromStack(-1));
						mod_Lua->pop(1);	/* pop error message from the stack */
						mod_Lua->pop(1);
					}
					mod_Lua->unlockState();
				}
#endif
			} else {
				float val;
				if(!fscanf(f, "%f", &val))
					publishLog('E', "[%s] : %s -> Unable to read a float value.", s->common.section.uid, s->common.file);
				else {	/* Only to normalize the response */
					bool publish = true;
					float compensated = val + s->offset;

					if(s->safe85 && val == 85.0)
						publishLog('E', "[%s] The probe replied 85° implying powering issue.", s->common.section.uid);
					else {
#ifdef LUA
						if(s->common.section.funcid != LUA_REFNIL){
							mod_Lua->lockState();
							mod_Lua->pushFunctionId( s->common.section.funcid );
							mod_Lua->pushString( s->common.section.uid );
							mod_Lua->pushString( s->common.section.topic );
							mod_Lua->pushNumber( val );
							mod_Lua->pushNumber( compensated );
							if(mod_Lua->exec(4, 1)){
								publishLog('E', "[%s] FFV : %s", s->common.section.uid, mod_Lua->getStringFromStack(-1));
								mod_Lua->pop(1);	/* pop error message from the stack */
								mod_Lua->pop(1);	/* pop NIL from the stack */
							} else
								publish = mod_Lua->getBooleanFromStack(-1);	/* Check the return code */
							mod_Lua->unlockState();
						}
#endif

						if(publish){
							publishLog('T', "[%s] -> %f", s->common.section.uid, compensated);
							sprintf(l,"%.1f", compensated);
							mqttpublish(cfg.client, s->common.section.topic, strlen(l), l, s->common.section.retained );
						} else
							publishLog('T', "[%s] UserFunction requested not to publish", s->common.section.uid);
					}
				}

				fclose(f);
			}
		}

		if(s->common.section.sample == -1)	/* Run once */
			pthread_exit(0);
		else {
			if(s->common.section.sample < 0)
				break;

			struct timespec ts;
			if(first && mod_1wire.randomize){
				ts.tv_sec = (time_t)(rand() % ((int)s->common.section.sample/2));
				ts.tv_nsec = 0;

				if(cfg.debug)
					publishLog('I', "[%s] Delayed by %d seconds", s->common.section.uid, ts.tv_sec);

				nanosleep( &ts, NULL );
			}
	
			ts.tv_sec = (time_t)s->common.section.sample;
			ts.tv_nsec = (unsigned long int)((s->common.section.sample - (time_t)s->common.section.sample) * 1e9);

			nanosleep( &ts, NULL );
		}
	}

	pthread_exit(0);
}

