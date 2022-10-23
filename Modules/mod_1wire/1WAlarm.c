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
				publishLog('E', "[%s] FFV failfunction : %s", s->common.section.uid, mod_Lua->getStringFromStack(-1));
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
				mod_Lua->pushString( l );
				if(mod_Lua->exec(2, 1)){
					publishLog('E', "[%s] 1WAlert : %s", s->common.section.uid, mod_Lua->getStringFromStack(-1));
					mod_Lua->pop(1);	/* pop error message from the stack */
					mod_Lua->pop(1);	/* pop NIL from the stack */
				} else
					publish = mod_Lua->getBooleanFromStack(-1);	/* Check the return code */
				mod_Lua->unlockState();
			}
#endif

			if(publish){
				publishLog('T', "[%s] -> %f", s->common.section.uid, l);
				mqttpublish(cfg.client, s->common.section.topic, strlen(l), l, s->common.section.retained );
			} else
				publishLog('T', "[%s] UserFunction requested not to publish", s->common.section.uid);
		}

		fclose(f);
	}
}

	/* ***
	 * Startup of 1-wire alarm driven probe
	 * ***/
void start1WAlarm( uint8_t mid ){
	uint16_t sid = (S1_ALRM << 8) | mid;	/* Alarm sectionID */

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
			if(s->initfunc){	/* Initialise all 1wAlarm */
				if(mod_Lua_id != (uint8_t)-1){
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
				} else {
					publishLog('E', "[%s] Init function defined but mod_Lua is not loaded. Dying.", s->common.section.uid);
					exit(EXIT_FAILURE);
				}
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
/*AF */
}
