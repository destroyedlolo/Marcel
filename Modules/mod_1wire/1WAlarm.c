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

void start1WAlarm( uint8_t mid ){
	uint16_t sid = (S1_ALRM << 8) | mid;	/* Alarm sectionID */

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
			
			if(s->initfunc){	/* Initialise all 1wAlarm */
#ifdef LUA
				uint8_t mod_Lua_id = findModuleByName("mod_Lua");	/* Is mod_Lua loaded ? */
				if(mod_Lua_id != (uint8_t)-1){
					struct module_Lua *mod_Lua = (struct module_Lua *)modules[mod_Lua_id];
					if(s->initfunc){	/* if an user function defined ? */
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
				} else {
					publishLog('E', "[%s] Init function defined but mod_Lua is not loaded. Dying.", s->common.section.uid);
					exit(EXIT_FAILURE);
				}
			}
#endif

			/* First reading if Immediate */
/*AF */
		}
	}

	/* Launch reading thread */
/*AF */
}
