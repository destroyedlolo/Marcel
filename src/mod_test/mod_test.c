/* mod_test
 *
 * This "fake" module only shows how to create a module for Marcel
 *
 * 13/09/2022 - LF - First version
 */

/* ***
 * Marcel's own include 
 * ***/

#include "mod_test.h"	/* module's own stuffs */
#ifdef LUA
#	include "../mod_Lua/mod_Lua.h"
#endif

/* ***
 * System's include
 * ***/

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* ***
 * instantiate module's structure.
 * ***/

static struct module_test mod_test;

/* ***
 * Unique identifier of section known by this module
 * Must start by 0 and < 255 
 */

enum {
	ST_TEST = 0
};

/**
 * @breif Callback called for each and every line of configuration files
 *
 * @param mid Module id
 * @param l line read
 * @param section Pointer to the section container (set when we're entering in a section)
 *
 * @return RC_readconf's value depending if the directive is ACCEPTED or if it has to be passed to next module.
 */

static enum RC_readconf readconf(uint8_t mid, const char *l, struct Section **section ){
	const char *arg;	/* Argument of the configuration directive */

		/* Parse the command line.
		 * striKWcmp() checks if the file start with the given token.
		 * Returns a pointer to directive's argument or NULL if the token
		 * is not recognized.
		 */

	if((arg = striKWcmp(l,"TestValue="))){	/* Directive with an argument */

		/* Some directives aren't not applicable to section ...
		 * We are in section if the 2nd argument does point to a non null value
		 */
		if(*section){
			publishLog('F', "TestValue can't be part of a section");
			exit(EXIT_FAILURE);
		}

		/* Processing ... */
		mod_test.test = atoi(arg);

		if(cfg.verbose)	/* Be verbose if requested */
			publishLog('C', "\tMod_test's \"test\" variable set to %d", mod_test.test);

		return ACCEPTED;
	} else if(!strcmp(l, "TestFlag")){	/* Directive without argument */
		if(*section){
			publishLog('F', "TestFlag can't be part of a section");
			exit(EXIT_FAILURE);
		}

		/* processing */
		mod_test.flag = true;

		if(cfg.verbose)	/* Be verbose if requested */
			publishLog('C', "\tMod_test's \"flag\" set to 'TRUE'");

		return ACCEPTED;
	} else if((arg = striKWcmp(l,"*TEST="))){	/* Starting a section definition */
		/* By convention, section directives starts by a start '*'.
		 * Its argument is an UNIQUE name : this name is used "OnOff" topic to
		 * identify which section it has to deal with.
		 */

		if(findSectionByName(arg)){
			publishLog('F', "Section '%s' is already defined", arg);
			exit(EXIT_FAILURE);
		}

		struct section_test *nsection = malloc(sizeof(struct section_test));	/* Allocate a new section */
		initSection( (struct Section *)nsection, mid, ST_TEST, "TEST");	/* Initialize shared fields */

			/* Custom fields may need to be initialized as well */
		nsection->dummy = 0;

		if(cfg.verbose)	/* Be verbose if requested */
			publishLog('C', "\tEntering section '%s' (%04x)", nsection->section.uid, nsection->section.id);

		*section = (struct Section *)nsection;	/* we're now in a section */
		return ACCEPTED;
	}

	return REJECTED;
}


/**
 * @brief Tell if a directive is acceptable inside a section
 *
 * @param sec_id section type of the current section
 * @param directive
 *
 * @return true we're accepting this directive.
 * NOTEZ-BIEN : in some cases, it's better this function crash by itself if we need
 * custom error message.
 */
static bool mt_acceptSDirective( uint8_t sec_id, const char *directive ){
		/* Check each section types known by this module
		 * (in this example, only one : ST_TEST)
		 */
	if(sec_id == ST_TEST){
		if( !strcmp(directive, "Disabled") )
			return true;	/* Accepted */
		else if( !strcmp(directive, "Sample=") )
			return true;	/* Accepted */
		else if( !strcmp(directive, "Func=") )
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

	return false;	/* Directive not handled by any of sections */
}

/**
 * @brief Task to be launched in slave thread to process Test section
 *
 * @param actx section_test stucture corresponding to the section
 *
 * Notez-bien : as test value is displayed at 'I'nformation level, it will
 * be shown only if Marcel is launched in verbose (or debug) mode.
 */
static void *processTest(void *actx){
	struct section_test *s = (struct section_test *)actx;	/* Only to avoid multiple cast */
	uint8_t mid = s->section.id & 0xff;	/* Module identifier */

	if(!s->section.sample){
		publishLog('E', "{%s] Sample time can't be 0. Dying ...", s->section.uid);
		pthread_exit(0);
	}

		/* Handle Lua functions */
	struct module_Lua *mod_Lua = NULL;
	uint8_t mod_Lua_id = findModuleByName("mod_Lua");	/* Is mod_Lua loaded ? */
	if(mod_Lua_id != (uint8_t)-1){
#ifdef LUA
		if(s->section.funcname){	/* if an user function defined ? */
			mod_Lua = (struct module_Lua *)modules[mod_Lua_id];
			if( (s->section.funcid = mod_Lua->findUserFunc(s->section.funcname)) == LUA_REFNIL ){
				publishLog('E', "[%s] configuration error : user function \"%s\" is not defined. This thread is dying.", s->section.uid, s->section.funcname);
				pthread_exit(NULL);
			}
		}
#endif
	}

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

#ifdef LUA
			/* Call user function if defined.
			 * Notez-bien : the implementation is totally module/section
			 * dependant.
			 * Here, we decided to pass one argument (dummy value)
			 * and got a return code.
			 */
			if(s->section.funcid != LUA_REFNIL){ /* A function is defined */
					/* As the state is shared among all threads, it's MANDATORY
					 * to lock it before any action (like pushing arguments)
					 * to avoid race condition.
					 * These stopped periods MUST be as short as possible
					 */
				mod_Lua->lockState();

				mod_Lua->pushFUnctionId( s->section.funcid );	/* Push the function to be called */
				mod_Lua->pushNumber( (double)s->dummy );	/* Push the argument */
				if(mod_Lua->exec(1, 0)){
					publishLog('E', "[%s] Test : %s", s->section.uid, mod_Lua->getStringFromStack(-1));
					mod_Lua->pop(1);	/* pop error message from the stack */
					mod_Lua->pop(1);	/* pop NIL from the stack */
				}

				mod_Lua->unlockState();
			}
#endif

			/* section's fields are accessible
			 * Only one task is accessing to section fields.
			 */
			publishLog('I', "Test's dummy : %d", s->dummy++);	

			/* and module's ones as well.
			 * CAUTION : if one section is modifying module's fields, arbitration
			 * is required (i.e : semaphore)
			 */
			s->dummy %= ((struct module_test *)modules[mid])->test;
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

/**
 * @brief returns the function to process specified section as a slave thread
 *
 * @param sid section id
 * @return function to launch in slave thread or NULL if none
 */
ThreadedFunctionPtr mt_getSlaveFunction(uint8_t sid){
	if(sid == ST_TEST)
		return processTest;

	return NULL;
}

/* ***
 * InitModule() - Module's initialisation function
 *
 * This function MUST exist and is called when the module is loaded.
 * Its goal is to initialise module's configuration and register the module.
 * If needed, it can also do some internal initialisation work for the module.
 * ***/
void InitModule( void ){
		/*
		 * Initialize module declarations
		 */
	mod_test.module.name = "mod_test";							/* Identify the module */

		/* Initialize callbacks
		 * It's MANDATORY that all callbacks are initialised (even by a NULL value)
		 */
	mod_test.module.readconf = readconf;
	mod_test.module.acceptSDirective = mt_acceptSDirective;
	mod_test.module.getSlaveFunction = mt_getSlaveFunction;
	mod_test.module.postconfInit = NULL;

	if(findModuleByName(mod_test.module.name) != (uint8_t)-1){
		publishLog('F', "Module '%s' is already loaded", mod_test.module.name);
		exit(EXIT_FAILURE);
	}

	register_module( (struct Module *)&mod_test );	/* Register the module */

		/*
		 * Do internal initialization
		 */
	mod_test.test = 0;
	mod_test.flag = false;
}
