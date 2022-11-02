/* 1WAlarm.c
 *
 * Handle alarm driven probe
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

#include <stdlib.h>
#include <errno.h>
#include <assert.h>
#include <dirent.h>

	/* ***
	 * Process one probe
	 * ***/
static void processProbe( struct section_1wAlarm *s
#ifdef LUA
	, struct module_Lua *mod_Lua 
#endif
){
	if(s->common.section.disabled){
#ifdef DEBUG
			if(cfg.debug)
				publishLog('d', "[%s] is disabled", s->common.section.uid);
#endif
		return;
	}

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
				publishLog('E', "[%s] 1WAlarm failfunction : %s", s->common.section.uid, mod_Lua->getStringFromStack(-1));
				mod_Lua->pop(1);	/* pop error message from the stack */
				mod_Lua->pop(1);
			}
			mod_Lua->unlockState();
		}
#endif
	} else {
		if(!fgets(l, MAXLINE, f))
			publishLog('E', "[%s] : %s -> Unable to read a float value.", s->common.section.uid, s->common.file);
		else {
			bool publish = true;

#ifdef LUA
			if(s->common.section.funcid != LUA_REFNIL){
				mod_Lua->lockState();
				mod_Lua->pushFunctionId( s->common.section.funcid );
				mod_Lua->pushString( s->common.section.uid );
				mod_Lua->pushString( s->common.section.topic );
				mod_Lua->pushString( l );
				if(mod_Lua->exec(3, 1)){
					publishLog('E', "[%s] 1WAlarm : %s", s->common.section.uid, mod_Lua->getStringFromStack(-1));
					mod_Lua->pop(1);	/* pop error message from the stack */
					mod_Lua->pop(1);	/* pop NIL from the stack */
				} else
					publish = mod_Lua->getBooleanFromStack(-1);	/* Check the return code */
				mod_Lua->unlockState();
			}
#endif

			if(publish){
				publishLog('T', "[%s] -> %s", s->common.section.uid, l);
				mqttpublish(cfg.client, s->common.section.topic, strlen(l), l, s->common.section.retained );
			} else
				publishLog('T', "[%s] UserFunction requested not to publish", s->common.section.uid);
		}

		fclose(f);
	}
}

	/* ***
	 * Scan Alert directory
	 * ***/
void *scanAlertDir(void *amod){
	struct module_1wire *mod_1wire = (struct module_1wire *)amod;
	uint16_t sid = S1_ALRM << 8 | mod_1wire->module.module_index;

		/* mod_lua */
#ifdef LUA
	uint8_t mod_Lua_id = findModuleByName("mod_Lua");	/* Is mod_Lua loaded ? */
	struct module_Lua *mod_Lua = NULL;

	if(mod_Lua_id != (uint8_t)-1)
		 mod_Lua = (struct module_Lua *)modules[mod_Lua_id];
#endif

	for(;;){
		struct dirent *de;
		DIR *d = opendir( mod_1wire->OwAlarm );
		if( !d ){
			publishLog(mod_1wire->OwAlarmKeep ? 'E' : 'F', "[1-wire Alarm] : %s", strerror(errno));
			if(!mod_1wire->OwAlarmKeep)
				pthread_exit(0);
		} else {
			while(( de = readdir(d) )){
				if( de->d_type == 4 && *de->d_name != '.' ){	/* 4 : directory */
					publishLog('T', "%s : in alert", de->d_name);

					for(struct section_1wAlarm *s = (struct section_1wAlarm *)sections; s; s = (struct section_1wAlarm *)s->common.section.next){
						if( s->common.section.id == sid && strstr(s->common.file, de->d_name)){
							processProbe(s
#ifdef LUA
								, mod_Lua
#endif
							);
						}
					}
				}
			}
			closedir(d);
		}

		struct timespec ts;
		ts.tv_sec = (time_t)mod_1wire->OwAlarmSample;
		ts.tv_nsec = (unsigned long int)((mod_1wire->OwAlarmSample - (time_t)mod_1wire->OwAlarmSample) * 1e9);

		nanosleep( &ts, NULL );
	}
}

	/* ***
	 * Startup of 1-wire alarm driven probe
	 * ***/
void start1WAlarm( uint8_t mid ){
	struct module_1wire *mod_1wire = (struct module_1wire *)modules[mid];
	uint16_t sid = (S1_ALRM << 8) | mid;	/* Alarm sectionID */

	if(!mod_1wire->alarm_in_use){
		publishLog('W', "[mod_1wire] No alarm defined");
		return;
	}
		
		/* Sanity checks */
	if(!mod_1wire->OwAlarm){
		publishLog('F', "[mod_1wire] 1wire-Alarm-directory must be set. Dying ...");
		exit(EXIT_FAILURE);
	}

	if(mod_1wire->OwAlarmSample <= 0){
		publishLog('F', "[mod_1wire] 1wire-Alarm-sample can't be <= 0. Dying ...");
		exit(EXIT_FAILURE);
	}

		/* mod_lua */
#ifdef LUA
	uint8_t mod_Lua_id = findModuleByName("mod_Lua");	/* Is mod_Lua loaded ? */
	struct module_Lua *mod_Lua = NULL;

	if(mod_Lua_id != (uint8_t)-1)
		 mod_Lua = (struct module_Lua *)modules[mod_Lua_id];
#endif

		/* As section identifier is part of the common Section structure, 
		 * it's harmless to directly use section_1wAlarm pointer
		 */
	for(struct section_1wAlarm *s = (struct section_1wAlarm *)sections; s; s = (struct section_1wAlarm *)s->common.section.next){
		if(s->common.section.id == sid){
				/* Sanity checks */
			if(!s->common.section.topic){
				publishLog('F', "[%s] Topic must be set. Dying ...", s->common.section.uid);
				exit(EXIT_FAILURE);
			}

			if(!s->common.file){
				publishLog('E', "[%s] File must be set. Dying ...", s->common.section.uid);
				exit(EXIT_FAILURE);
			}

			if(s->common.section.disabled){
#ifdef DEBUG
				if(cfg.debug)
					publishLog('d', "[%s] is disabled", s->common.section.uid);
#endif
			}

#ifdef LUA
			if(s->common.section.funcname){
				assert(mod_Lua_id != (uint8_t)-1);
				
				if( (s->common.section.funcid = mod_Lua->findUserFunc(s->common.section.funcname)) == LUA_REFNIL ){
					publishLog('F', "[%s] configuration error : user function \"%s\" is not defined", s->common.section.uid, s->common.section.funcname);
					exit(EXIT_FAILURE);
				}
			}

			if(s->common.failfunc){
				assert(mod_Lua_id != (uint8_t)-1);
				
				if( (s->common.failfuncid = mod_Lua->findUserFunc(s->common.failfunc)) == LUA_REFNIL ){
					publishLog('F', "[%s] configuration error : Fail function \"%s\" is not defined", s->common.section.uid, s->common.failfunc);
					exit(EXIT_FAILURE);
				}
			}

			if(s->initfunc){	/* Initialise all 1wAlarm */
				assert(mod_Lua_id != (uint8_t)-1);

				int funcid;
				if( (funcid = mod_Lua->findUserFunc(s->initfunc)) == LUA_REFNIL ){
					publishLog('E', "[%s] configuration error : Init function \"%s\" is not defined. Dying.", s->common.section.uid, s->initfunc);
					exit(EXIT_FAILURE);
				}

				mod_Lua->lockState();
				mod_Lua->pushFunctionId( funcid );
				mod_Lua->pushString( s->common.section.uid );
				mod_Lua->pushString( s->common.file );
				if(mod_Lua->exec(2, 0)){
					publishLog('E', "[%s] Init function : %s", s->common.section.uid, mod_Lua->getStringFromStack(-1));
					mod_Lua->pop(1);	/* pop error message from the stack */
					mod_Lua->pop(1);
				}
				mod_Lua->unlockState();
			}
#endif

			/* First reading if Immediate */
			if(s->common.section.immediate)
				processProbe(s
#ifdef LUA
				, mod_Lua
#endif
				);
		}
	}

	/* Launch reading thread */
	pthread_attr_t thread_attr;
	assert(!pthread_attr_init (&thread_attr));
	assert(!pthread_attr_setdetachstate (&thread_attr, PTHREAD_CREATE_DETACHED));

	if(pthread_create( &(mod_1wire->thread), &thread_attr, scanAlertDir, mod_1wire) < 0){
		publishLog('F', "Can't create 1-wire alert reader thread");
		exit(EXIT_FAILURE);
	}
}
