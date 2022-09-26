/* mod_every
 *
 * Repeating tasks
 *
 * This file is part of Marcel project and is following the same
 * license rules (see LICENSE file)
 *
 * 07/09/2015	- LF - First version
 * 17/05/2016	- LF - Add 'Immediate'
 * 06/06/2016	- LF - If sample is null, stop 
 * 20/08/2016	- LF - Prevent a nasty bug making system to crash if 
 * 		user function lookup is failing
 * 					----
 * 24/09/2022	- LF - Move to V8
 */

#include "mod_every.h"

#ifdef LUA
#	include "../mod_Lua/mod_Lua.h"
#else
#	error "mod_every is useless without Lua"
#endif

#include <stdlib.h>
#include <string.h>

static struct module_every mod_every;

	/* Section identifier */
enum {
	SE_EVERY = 0
};

/* ***
 * Processing
 * ***/
static void *processEvery(void *actx){
	struct section_every *s = (struct section_every *)actx;	/* Only to avoid multiple cast */

		/* Sanity checks */
	if(s->section.sample <= 0){
		publishLog('E', "[%s] Sample time can't be negative of null. Dying ...", s->section.uid);
		pthread_exit(0);
	}

	struct module_Lua *mod_Lua = NULL;
	uint8_t mod_Lua_id = findModuleByName("mod_Lua");
	if(mod_Lua_id == (uint8_t)-1){
		publishLog('E', "[%s] Every without Lua support is useless. This thread is dying.", s->section.uid);
		pthread_exit(NULL);
	}

#ifdef LUA
	if(s->section.funcname){	/* if an user function defined ? */
		mod_Lua = (struct module_Lua *)modules[mod_Lua_id];
		if( (s->section.funcid = mod_Lua->findUserFunc(s->section.funcname)) == LUA_REFNIL ){
			publishLog('E', "[%s] configuration error : user function \"%s\" is not defined. This thread is dying.", s->section.uid, s->section.funcname);
			pthread_exit(NULL);
		}
	}

	for(bool first=true;; first=false){
		if(s->section.disabled){
#ifdef DEBUG
			if(cfg.debug)
				publishLog('d', "[%s] is disabled", s->section.uid);
#endif
		} else if( !first || s->section.immediate ){
			mod_Lua->lockState();
			mod_Lua->pushFUnctionId( s->section.funcid );
			if(s->section.topic)
				mod_Lua->pushString( s->section.topic );
			else
				mod_Lua->pushString( s->section.uid );

			if(mod_Lua->exec(1, 0)){
				publishLog('E', "[%s] Dummy : %s", s->section.uid, mod_Lua->getStringFromStack(-1));
				mod_Lua->pop(1);	/* pop error message from the stack */
				mod_Lua->pop(1);	/* pop NIL from the stack */
			}

			mod_Lua->unlockState();
		}

		struct timespec ts;
		ts.tv_sec = (time_t)s->section.sample;
		ts.tv_nsec = (unsigned long int)((s->section.sample - (time_t)s->section.sample) * 1e9);

		nanosleep( &ts, NULL );
	}
#endif
}

/* ***
 * Module interface
 * ***/
static enum RC_readconf readconf(uint8_t mid, const char *l, struct Section **section ){
	const char *arg;	/* Argument of the configuration directive */

	if((arg = striKWcmp(l,"*Every="))){	/* Starting a section definition */
		if(findSectionByName(arg)){	/* Name must be unique */
			publishLog('F', "Section '%s' is already defined", arg);
			exit(EXIT_FAILURE);
		}

		struct section_every *nsection = malloc(sizeof(struct section_every));
		initSection( (struct Section *)nsection, mid, SE_EVERY, strdup(arg));

		if(cfg.verbose)	/* Be verbose if requested */
			publishLog('C', "\tEntering section '%s' (%04x)", nsection->section.uid, nsection->section.id);

		*section = (struct Section *)nsection;
		return ACCEPTED;
	}

	 return REJECTED;
}

static bool me_acceptSDirective( uint8_t sec_id, const char *directive ){
	if(sec_id == SE_EVERY){
		if( !strcmp(directive, "Disabled") )
			return true;	/* Accepted */
		else if( !strcmp(directive, "Sample=") )
			return true;	/* Accepted */
		else if( !strcmp(directive, "Func=") )
			return true;	/* Accepted */
		else if( !strcmp(directive, "Topic=") )
			return true;	/* Accepted */
		else if( !strcmp(directive, "Immediate") )
			return true;	/* Accepted */
		else {	
				/* Custom error message.
				 * Well it's only an example as it's the default message
				 * raised.
				 */
			publishLog('F', "'%s' not allowed here", directive);
			exit(EXIT_FAILURE);
		}
	}

	return false;	/* not accepted */
}

/**
 * @brief returns the function to process specified section as a slave thread
 *
 * @param sid section id
 * @return function to launch in slave thread or NULL if none
 */
ThreadedFunctionPtr me_getSlaveFunction(uint8_t sid){
	if(sid == SE_EVERY)
		return processEvery;

	return NULL;
}

void InitModule( void ){
	mod_every.module.name = "mod_every";

	mod_every.module.readconf = readconf;
	mod_every.module.acceptSDirective = me_acceptSDirective;
	mod_every.module.getSlaveFunction = me_getSlaveFunction;
	mod_every.module.postconfInit = NULL;

	register_module( (struct Module *)&mod_every );
}
