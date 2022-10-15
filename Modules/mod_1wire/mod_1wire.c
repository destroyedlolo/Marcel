/* mod_1wire
 *
 * Handle 1-wire probes.
 * (also suitable for other exposed as a file values)
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
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>

static struct module_1wire mod_1wire;

enum {
	ST_FFV= 0,
};

static void *processFFV(void *actx){
	struct section_FFV *s = (struct section_FFV *)actx;

		/* Sanity checks */
	if(!s->section.topic){
		publishLog('F', "[%s] Topic must be set. Dying ...", s->section.uid);
		pthread_exit(0);
	}

	if(!s->section.sample){
		publishLog('E', "[%s] Sample time can't be 0. Dying ...", s->section.uid);
		pthread_exit(0);
	}

	if(!s->file){
		publishLog('E', "[%s] File must be set. Dying ...", s->section.uid);
		pthread_exit(0);
	}

			/* Handle Lua functions */
	struct module_Lua *mod_Lua = NULL;
	uint8_t mod_Lua_id = findModuleByName("mod_Lua");	/* Is mod_Lua loaded ? */
	if(mod_Lua_id != (uint8_t)-1){
#ifdef LUA
		mod_Lua = (struct module_Lua *)modules[mod_Lua_id];
		if(s->section.funcname){	/* if an user function defined ? */
			if( (s->section.funcid = mod_Lua->findUserFunc(s->section.funcname)) == LUA_REFNIL ){
				publishLog('E', "[%s] configuration error : user function \"%s\" is not defined. This thread is dying.", s->section.uid, s->section.funcname);
				pthread_exit(NULL);
			}
		}

		if(s->failfunc){	/* if an user function defined ? */
			if( (s->failfuncid = mod_Lua->findUserFunc(s->failfunc)) == LUA_REFNIL ){
				publishLog('E', "[%s] configuration error : fail function \"%s\" is not defined. This thread is dying.", s->section.uid, s->failfunc);
				pthread_exit(NULL);
			}
		}
#endif
	}

	for(bool first=true;; first=false){	/* Infinite publishing loop */
		if(s->section.disabled){
#ifdef DEBUG
			if(cfg.debug)
				publishLog('d', "[%s] is disabled", s->section.uid);
#endif
		} else if( !first || s->section.immediate || s->section.sample == -1 ){	/* processing */
			FILE *f;
			char l[MAXLINE];

			if(!(f = fopen( s->file, "r" ))){
				char *emsg = strerror(errno);
				publishLog('E', "[%s] %s : %s", s->section.uid, s->file, emsg);

				if(strlen(s->section.topic) + 7 < MAXLINE){  /* "/Alarm" +1 */
					int msg;
					strcpy(l, "Alarm/");
					strcat(l, s->section.topic);
					msg = strlen(l) + 2;

					if(strlen(s->file) + strlen(emsg) + 5 < MAXLINE - msg){ /* S + " : " + 0 */
						*(l + msg) = 'S';
						strcpy(l + msg + 1, s->file);
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
				if(s->failfuncid != LUA_REFNIL){
					mod_Lua->lockState();
					mod_Lua->pushFunctionId( s->failfuncid );
					mod_Lua->pushString( s->section.uid );
					mod_Lua->pushString( emsg );
					if(mod_Lua->exec(2, 0)){
						publishLog('E', "[%s] FFV failfunction : %s", s->section.uid, mod_Lua->getStringFromStack(-1));
						mod_Lua->pop(1);	/* pop error message from the stack */
						mod_Lua->pop(1);
					}
				}
#endif
			} else {
				float val;
				if(!fscanf(f, "%f", &val))
					publishLog('E', "[%s] : %s -> Unable to read a float value.", s->section.uid, s->file);
				else {	/* Only to normalize the response */
					bool publish = true;
					float compensated = val + s->offset;

					if(s->safe85 && val == 85.0)
						publishLog('E', "[%s] The probe replied 85° implying powering issue.", s->section.uid);
					else {
#ifdef LUA
						if(s->section.funcid != LUA_REFNIL){
							mod_Lua->lockState();
							mod_Lua->pushFunctionId( s->section.funcid );
							mod_Lua->pushString( s->section.uid );
							mod_Lua->pushNumber( val );
							mod_Lua->pushNumber( compensated );
							if(mod_Lua->exec(3, 1)){
								publishLog('E', "[%s] FFV : %s", s->section.uid, mod_Lua->getStringFromStack(-1));
								mod_Lua->pop(1);	/* pop error message from the stack */
								mod_Lua->pop(1);	/* pop NIL from the stack */
							} else
								publish = mod_Lua->getBooleanFromStack(-1);	/* Check the return code */
							mod_Lua->unlockState();
						}
#endif

						if(publish){
							publishLog('T', "[%s] -> %f", s->section.uid, compensated);
							sprintf(l,"%.1f", compensated);
							mqttpublish(cfg.client, s->section.topic, strlen(l), l, s->section.retained );
						} else
							publishLog('T', "[%s] UserFunction requested not to publish", s->section.uid);
					}
				}

				fclose(f);
			}
		}

		if(s->section.sample == -1)	/* Run once */
			pthread_exit(0);
		else {
			if(s->section.sample < 0)
				break;

			struct timespec ts;
			ts.tv_sec = (time_t)s->section.sample;
			ts.tv_nsec = (unsigned long int)((s->section.sample - (time_t)s->section.sample) * 1e9);

			nanosleep( &ts, NULL );
		}
	}

	pthread_exit(0);
}

static enum RC_readconf readconf(uint8_t mid, const char *l, struct Section **section ){
	const char *arg;

	if(!strcmp(l, "RandomizeProbes")){
		if(*section){
			publishLog('F', "TestFlag can't be part of a section");
			exit(EXIT_FAILURE);
		}

		mod_1wire.randomize = true;

		if(cfg.verbose)
			publishLog('C', "\tProbes are randomized");

		return ACCEPTED;
	} else if((arg = striKWcmp(l,"DefaultSampleDelay="))){
		/* No need to check if we are on not inside a section :
		 * despite this directive is a top level one, it can be placed
		 * anywhere : we don't know when a section definition
		 * is over
		 */

		mod_1wire.defaultsampletime = strtof(arg, NULL);

		if(cfg.verbose)
			publishLog('C', "\tDefault sample time : %f", mod_1wire.defaultsampletime);

		return ACCEPTED;	
	} else if((arg = striKWcmp(l,"*FFV="))){
		if(findSectionByName(arg)){
			publishLog('F', "Section '%s' is already defined", arg);
			exit(EXIT_FAILURE);
		}

		struct section_FFV *nsection = malloc(sizeof(struct section_FFV));	/* Allocate a new section */
		initSection( (struct Section *)nsection, mid, ST_FFV, strdup(arg));	/* Initialize shared fields */

		nsection->section.sample = mod_1wire.defaultsampletime;
		nsection->file = NULL;
		nsection->latch = NULL;
		nsection->offset = 0.0;
		nsection->safe85 = false;
		nsection->failfunc = NULL;
		nsection->failfuncid = LUA_REFNIL;

		if(cfg.verbose)	/* Be verbose if requested */
			publishLog('C', "\tEntering FFV section '%s' (%04x)", nsection->section.uid, nsection->section.id);

		*section = (struct Section *)nsection;	/* we're now in a section */
		return ACCEPTED;
	} else if(*section){
		if((arg = striKWcmp(l,"File="))){
			assert(( (*(struct section_FFV **)section)->file = strdup(arg) ));

			if(cfg.verbose)	/* Be verbose if requested */
				publishLog('C', "\t\tFile : '%s'", (*(struct section_FFV **)section)->file);
			return ACCEPTED;
		} else if((arg = striKWcmp(l,"Offset="))){
			(*(struct section_FFV **)section)->offset = strtof(arg, NULL);

			if(cfg.verbose)	/* Be verbose if requested */
				publishLog('C', "\t\tOffset: %f", (*(struct section_FFV **)section)->offset);
			return ACCEPTED;
#ifdef LUA
		} else if((arg = striKWcmp(l,"FailFunc="))){
			assert(( (*(struct section_FFV **)section)->failfunc = strdup(arg) ));

			if(cfg.verbose)	/* Be verbose if requested */
				publishLog('C', "\t\tFailFunc: '%s'", (*(struct section_FFV **)section)->failfunc);
			return ACCEPTED;
#endif
		} else if(!strcmp(l, "Safe85")){
			(*(struct section_FFV **)section)->safe85 = true;

			if(cfg.verbose)
				publishLog('C', "\t\tIgnore 85°C probes");
			return ACCEPTED;
		}
	}

	return REJECTED;
}

static bool m1_acceptSDirective( uint8_t sec_id, const char *directive ){
	if(sec_id == ST_FFV){
		if( !strcmp(directive, "Disabled") )
			return true;	/* Accepted */
		else if( !strcmp(directive, "Immediate") )
			return true;	/* Accepted */
		else if( !strcmp(directive, "Retained") )
			return true;	/* Accepted */
		else if( !strcmp(directive, "Sample=") )
			return true;	/* Accepted */
		else if( !strcmp(directive, "Topic=") )
			return true;	/* Accepted */
		else if( !strcmp(directive, "Func=") )
			return true;	/* Accepted */
		else if( !strcmp(directive, "FailFunc=") )
			return true;	/* Accepted */
		else if( !strcmp(directive, "File=") )
			return true;	/* Accepted */
		else if( !strcmp(directive, "Offset=") )
			return true;	/* Accepted */
		else if( !strcmp(directive, "Safe85") )
			return true;	/* Accepted */
	}

	return false;
}

ThreadedFunctionPtr m1_getSlaveFunction(uint8_t sid){
	if(sid == ST_FFV)
		return processFFV;

	return NULL;
}

void InitModule( void ){
	mod_1wire.module.name = "mod_1wire";	/* Identify the module */

		/* Initialize callbacks
		 * It's MANDATORY that all callbacks are initialised (even by a NULL value)
		 */
	mod_1wire.module.readconf = readconf;
	mod_1wire.module.acceptSDirective = m1_acceptSDirective;
	mod_1wire.module.getSlaveFunction = m1_getSlaveFunction;
	mod_1wire.module.postconfInit = NULL;

	mod_1wire.randomize = false;
	mod_1wire.defaultsampletime = 0.0;

	register_module( (struct Module *)&mod_1wire );	/* Register the module */
}
