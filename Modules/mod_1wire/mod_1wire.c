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

static struct module_1wire mod_1wire;

enum {
	ST_FFV= 0,
	ST_ALRM
};

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
	} else if((arg = striKWcmp(l,"1wire-Alarm-directory="))){
		if(*section){
			publishLog('F', "TestFlag can't be part of a section");
			exit(EXIT_FAILURE);
		}

		if(mod_1wire.OwAlarm)
			publishLog('E', "1wire-Alarm-directory= defined more than once. Let's continue ...");

		assert(( mod_1wire.OwAlarm = strdup(arg) ));

		if(cfg.verbose)
			publishLog('C', "\t1-wire Alarm directory : '%s'", mod_1wire.OwAlarm);

		return ACCEPTED;	
	} else if((arg = striKWcmp(l,"1wire-Alarm-sample="))){
		if(*section){
			publishLog('F', "1wire-Alarm-sample= can't be part of a section");
			exit(EXIT_FAILURE);
		}

		mod_1wire.OwAlarmSample = atof(arg);

		if(cfg.verbose)
			publishLog('C', "\t1-wire Alarm Sample time : %lf", mod_1wire.OwAlarmSample);

		return ACCEPTED;
	} else if(!strcmp(l, "1wire-Alarm-keep")){
		if(*section){
			publishLog('F', "1wire-Alarm-keep can't be part of a section");
			exit(EXIT_FAILURE);
		}

		mod_1wire.randomize = true;

		if(cfg.verbose)
			publishLog('C', "\t1-wire Technical errors are not fatal");

		return ACCEPTED;
	} else if((arg = striKWcmp(l,"*FFV="))){
		if(findSectionByName(arg)){
			publishLog('F', "Section '%s' is already defined", arg);
			exit(EXIT_FAILURE);
		}

		struct section_FFV *nsection = malloc(sizeof(struct section_FFV));	/* Allocate a new section */
		initSection( (struct Section *)nsection, mid, ST_FFV, strdup(arg));	/* Initialize shared fields */

		nsection->common.section.sample = mod_1wire.defaultsampletime;
		nsection->common.file = NULL;
		nsection->common.failfunc = NULL;
		nsection->common.failfuncid = LUA_REFNIL;
		nsection->offset = 0.0;
		nsection->safe85 = false;

		if(cfg.verbose)	/* Be verbose if requested */
			publishLog('C', "\tEntering FFV section '%s' (%04x)", nsection->common.section.uid, nsection->common.section.id);

		*section = (struct Section *)nsection;	/* we're now in a section */
		return ACCEPTED;
	} else if((arg = striKWcmp(l,"*1WAlert="))){
		if(findSectionByName(arg)){
			publishLog('F', "Section '%s' is already defined", arg);
			exit(EXIT_FAILURE);
		}

		struct section_1wAlerte *nsection = malloc(sizeof(struct section_1wAlerte));	/* Allocate a new section */
		initSection( (struct Section *)nsection, mid, ST_ALRM, strdup(arg));	/* Initialize shared fields */

		nsection->common.file = NULL;
		nsection->common.failfunc = NULL;
		nsection->common.failfuncid = LUA_REFNIL;
		nsection->initfunc = NULL;
		nsection->latch = NULL;

		if(cfg.verbose)	/* Be verbose if requested */
			publishLog('C', "\tEntering 1-wire Alarm section '%s' (%04x)", nsection->common.section.uid, nsection->common.section.id);

		*section = (struct Section *)nsection;	/* we're now in a section */
		return ACCEPTED;
	} else if(*section){
		if((arg = striKWcmp(l,"File="))){
			assert(( (*(struct section_FFV **)section)->common.file = strdup(arg) ));

			if(cfg.verbose)	/* Be verbose if requested */
				publishLog('C', "\t\tFile : '%s'", (*(struct section_FFV **)section)->common.file);
			return ACCEPTED;
		} else if((arg = striKWcmp(l,"Latch="))){
			assert(( (*(struct section_1wAlerte **)section)->latch = strdup(arg) ));

			if(cfg.verbose)	/* Be verbose if requested */
				publishLog('C', "\t\tLatch : '%s'", (*(struct section_1wAlerte **)section)->latch);
			return ACCEPTED;
		} else if((arg = striKWcmp(l,"Offset="))){
			(*(struct section_FFV **)section)->offset = strtof(arg, NULL);

			if(cfg.verbose)	/* Be verbose if requested */
				publishLog('C', "\t\tOffset: %f", (*(struct section_FFV **)section)->offset);
			return ACCEPTED;
#ifdef LUA
		} else if((arg = striKWcmp(l,"FailFunc="))){
			assert(( (*(struct section_FFV **)section)->common.failfunc = strdup(arg) ));

			if(cfg.verbose)	/* Be verbose if requested */
				publishLog('C', "\t\tFailFunc: '%s'", (*(struct section_FFV **)section)->common.failfunc);
			return ACCEPTED;
		} else if((arg = striKWcmp(l,"InitFunc="))){
			assert(( (*(struct section_1wAlerte **)section)->initfunc = strdup(arg) ));

			if(cfg.verbose)	/* Be verbose if requested */
				publishLog('C', "\t\tInitFunc: '%s'", (*(struct section_1wAlerte **)section)->initfunc);
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
	} else if(sec_id == ST_ALRM){
		if( !strcmp(directive, "Disabled") )
			return true;	/* Accepted */
		else if( !strcmp(directive, "Immediate") )
			return true;	/* Accepted */
		else if( !strcmp(directive, "Retained") )
			return true;	/* Accepted */
		else if( !strcmp(directive, "Topic=") )
			return true;	/* Accepted */
		else if( !strcmp(directive, "Func=") )
			return true;	/* Accepted */
		else if( !strcmp(directive, "FailFunc=") )
			return true;	/* Accepted */
		else if( !strcmp(directive, "File=") )
			return true;	/* Accepted */
		else if( !strcmp(directive, "InitFunc=") )
			return true;	/* Accepted */
		else if( !strcmp(directive, "Latch=") )
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

	mod_1wire.OwAlarm = NULL;
	mod_1wire.OwAlarmSample = 0.0;
	mod_1wire.OwAlarmKeep = false;

	register_module( (struct Module *)&mod_1wire );	/* Register the module */
}
