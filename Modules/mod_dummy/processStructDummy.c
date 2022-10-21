/* in order to speedup compilation it's better to split large source files in
 * multiple chunks.
 * Here, as example, dummy section processing is externalized
 *
 * This file is part of Marcel project and is following the same
 * license rules (see LICENSE file)
 */


/* ***
 * Marcel's own include 
 * ***/

#include "mod_dummy.h"	/* module's own stuffs */
#ifdef LUA
#	include "../mod_Lua/mod_Lua.h"
#endif

/* ***
 * System's include
 * ***/

/**
 * @brief Task to be launched in slave thread to process Test section
 *
 * @param actx section_dummy stucture corresponding to the section
 *
 * Notez-bien : as value is displayed at 'I'nformation level, it will
 * be shown only if Marcel is launched in verbose (or debug) mode.
 */
void *processDummy(void *actx){
	struct section_dummy *s = (struct section_dummy *)actx;	/* Only to avoid multiple cast */
	uint8_t mid = s->section.id & 0xff;	/* Module identifier */

	if(!s->section.sample){
		publishLog('E', "[%s] Sample time can't be 0. Dying ...", s->section.uid);
		pthread_exit(0);
	}

		/* Handle Lua functions */
#ifdef LUA
	struct module_Lua *mod_Lua = NULL;
	uint8_t mod_Lua_id = findModuleByName("mod_Lua");	/* Is mod_Lua loaded ? */
	if(mod_Lua_id != (uint8_t)-1){
		if(s->section.funcname){	/* if an user function defined ? */
			mod_Lua = (struct module_Lua *)modules[mod_Lua_id];
			if( (s->section.funcid = mod_Lua->findUserFunc(s->section.funcname)) == LUA_REFNIL ){
				publishLog('E', "[%s] configuration error : user function \"%s\" is not defined. This thread is dying.", s->section.uid, s->section.funcname);
				pthread_exit(NULL);
			}
		}
	}
#endif

		/* If not event driven, most of the time, the section code
		 * is an endless loop.
		 */
	for(;;){
			/* 1st of all, checking if the section is active */
		if(s->section.disabled){
#ifdef DEBUG
			if(cfg.debug)
				publishLog('d', "[%s] is disabled", s->section.uid);
#endif
		} else {	/* Processing */

			bool ret = true;	/* display value in the module */
#ifdef LUA
			/* Call user function if defined.
			 * Notez-bien : the implementation is totally module/section
			 * dependant.
			 * Here, we decided to pass one argument ("dummy" value)
			 * and got a return code : true if the remaining of the code
			 * has to be executed, false otherwise.
			 */

			if(s->section.funcid != LUA_REFNIL){ /* A function is defined */
					/* As the state is shared among all threads, it's MANDATORY
					 * to lock it before any action (like pushing arguments)
					 * to avoid race condition.
					 * These stopped periods MUST be as short as possible
					 */
				mod_Lua->lockState();

				mod_Lua->pushFunctionId( s->section.funcid );	/* Push the function to be called */
				mod_Lua->pushNumber( (double)s->dummy );	/* Push the argument */
				if(mod_Lua->exec(1, 1)){
					publishLog('E', "[%s] Dummy : %s", s->section.uid, mod_Lua->getStringFromStack(-1));
					mod_Lua->pop(1);	/* pop error message from the stack */
					mod_Lua->pop(1);	/* pop NIL from the stack */
				} else
					ret = mod_Lua->getBooleanFromStack(-1);	/* Check the return code */
				mod_Lua->unlockState();
			}
#endif

			/* section's fields are accessible
			 * Only one task is accessing to section fields.
			 */
			if(ret)
				publishLog('I', "Dummy : %d", s->dummy);

			/* and module's ones as well.
			 * CAUTION : if one section is modifying module's fields, arbitration
			 * is required (i.e : semaphore)
			 */
			s->dummy++;
			s->dummy %= ((struct module_dummy *)modules[mid])->test;
		}

		if(s->section.sample < 0){
			/* Usually, a sampletime < 0 means this process will run only once.
			 * NOTEZ-BIEN : it's section dependant, other ones may react differently
			 */
#ifdef DEBUG
			if(cfg.debug)
				publishLog('d', "[%s] runs only once. Dying ...", s->section.uid);
#endif
			pthread_exit(0);

		} else {
			/* Wait for the specified sample time */
			struct timespec ts;
			ts.tv_sec = (time_t)s->section.sample;
			ts.tv_nsec = (unsigned long int)((s->section.sample - (time_t)s->section.sample) * 1e9);

			nanosleep( &ts, NULL );
		}
	}
}


