/* mod_dummy
 *
 * This "fake" module only shows how to create a module for Marcel
 *
 * This file is part of Marcel project and is following the same
 * license rules (see LICENSE file)
 *
 * 13/09/2022 - LF - First version
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

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* ***
 * instantiate module's structure.
 * ***/

static struct module_dummy mod_dummy;

/* ***
 * Unique identifier of section known by this module
 * Must start by 0 and < 255 
 */

enum {
	ST_DUMMY = 0,
	ST_ECHO
};

/**
 * @brief get all customs figures for Dummy section
 *
 * @param section pointer to a section instance
 *
 * @return array of custom figures
 */
static int publishCustomFiguresDummy(struct Section *asection){
#ifdef LUA
	if(mod_Lua){
		struct section_dummy *s = (struct section_dummy *)asection;

		lua_newtable(mod_Lua->L);	/* Create a new table */

		lua_pushstring(mod_Lua->L, "Dummy variable");			/* Push the index */
		lua_pushnumber(mod_Lua->L, s->dummy);	/* the value */
		lua_rawset(mod_Lua->L, -3);	/* Add it in the table */

		return 1;
	} else
#endif
	return 0;
}

/**
 * @brief Callback called for each and every line of configuration files
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
		mod_dummy.test = atoi(arg);

		if(cfg.verbose)	/* Be verbose if requested */
			publishLog('C', "\tMod_dummy's \"test\" variable set to %d", mod_dummy.test);

		return ACCEPTED;
	} else if(!strcmp(l, "TestFlag")){	/* Directive without argument */
		if(*section){
			publishLog('F', "TestFlag can't be part of a section");
			exit(EXIT_FAILURE);
		}

		/* processing */
		mod_dummy.flag = true;

		if(cfg.verbose)	/* Be verbose if requested */
			publishLog('C', "\tMod_dummy's \"flag\" set to 'TRUE'");

		return ACCEPTED;
	} else if((arg = striKWcmp(l,"*Dummy="))){	/* Starting a section definition */
		/* By convention, section directives starts by a start '*'.
		 * Its argument is an UNIQUE name : this name is used "OnOff" topic to
		 * identify which section it has to deal with.
		 */

		if(findSectionByName(arg)){
			publishLog('F', "Section '%s' is already defined", arg);
			exit(EXIT_FAILURE);
		}

		struct section_dummy *nsection = malloc(sizeof(struct section_dummy));	/* Allocate a new section */
		initSection( (struct Section *)nsection, mid, ST_DUMMY, strdup(arg), "Dummy");	/* Initialize shared fields */

			/* Some section field may be initialised. Here, the callback
			 * to expose custom figure in Lua
			 */
		nsection->section.publishCustomFigures = publishCustomFiguresDummy;
	
			/* Custom fields may need to be initialized as well */
		nsection->dummy = 0;

		if(cfg.verbose)	/* Be verbose if requested */
			publishLog('C', "\tEntering Dummy section '%s' (%04x)", nsection->section.uid, nsection->section.id);

		*section = (struct Section *)nsection;	/* we're now in a section */
		return ACCEPTED;
	} else if((arg = striKWcmp(l,"*Echo="))){	/* Starting a section definition */
		if(findSectionByName(arg)){
			publishLog('F', "Section '%s' is already defined", arg);
			exit(EXIT_FAILURE);
		}

		struct section_echo *nsection = malloc(sizeof(struct section_echo));	/* Allocate a new section */
		initSection( (struct Section *)nsection, mid, ST_ECHO, strdup(arg), "Echo");	/* Initialize shared fields */

			/* This section is processing MQTT messages */
		nsection->section.postconfInit = st_echo_postconfInit;	/* Subscribe */
		nsection->section.processMsg = st_echo_processMQTT;		/* Processing */

		if(cfg.verbose)	/* Be verbose if requested */
			publishLog('C', "\tEntering Echo section '%s' (%04x)", nsection->section.uid, nsection->section.id);

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
		 */
	if(sec_id == ST_DUMMY){
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
	} else if(sec_id == ST_ECHO){
		if( !strcmp(directive, "Disabled") )
			return true;	/* Accepted */
		else if( !strcmp(directive, "Topic=") )
			return true;	/* Accepted */
	}

	return false;	/* Directive not handled by any of sections */
}

/**
 * @brief returns the function to process specified section as a slave thread
 *
 * @param sid section id
 * @return function to launch in slave thread or NULL if none
 */
ThreadedFunctionPtr mt_getSlaveFunction(uint8_t sid){
	if(sid == ST_DUMMY)
		return processDummy;

	/* No slave for Echo : it will process incoming messages */
	return NULL;
}

	/* ***
	 * Methods attached to Lua's objects
	 * Here, only to liked to "Dummy"
	 */

#ifdef LUA

/* getDummy() will return (push on the stack) the value of
 * "dummy" variable.
 *
 * At Lua side, can be called as following
 *	print(section:getDummy())
 */
static int md_getDummy(lua_State *L){
	struct section_dummy **s = luaL_testudata(L, 1, "Dummy");
	luaL_argcheck(L, s != NULL, 1, "'DPD' expected");

	lua_pushinteger(mod_Lua->L, (*s)->dummy);

	return 1;
}

static const struct luaL_Reg mdM[] = {
	{"getDummy", md_getDummy},
	{NULL, NULL}
};
#endif

/* ***
 * InitModule() - Module's initialisation function
 *
 * This function MUST exist and is called when the module is loaded.
 * Its goal is to initialize module's configuration and register the module.
 * If needed, it can also do some internal initialisation work for the module.
 * ***/
void InitModule( void ){
		/*
		 * Initialize module declarations
		 */
	initModule((struct Module *)&mod_dummy, "mod_dummy"); /* Identify the module */

		/* Initialize callbacks
		 * It's MANDATORY that all callbacks are initialised
		 */
	mod_dummy.module.readconf = readconf;
	mod_dummy.module.acceptSDirective = mt_acceptSDirective;
	mod_dummy.module.getSlaveFunction = mt_getSlaveFunction;

	registerModule( (struct Module *)&mod_dummy );	/* Register the module */

		/*
		 * Do internal initialization
		 */
	mod_dummy.test = 0;
	mod_dummy.flag = false;

#ifdef LUA
	if(mod_Lua){ /* Is mod_Lua loaded ? */

			/* Expose shared methods
			 * Here, we're exposing methods common to all sections
			 */
		mod_Lua->initSectionSharedMethods(mod_Lua->L, "Dummy");
		mod_Lua->initSectionSharedMethods(mod_Lua->L, "Echo");

			/* Expose mod_dummy's own function
			 * Here Dummy section only
			 */
		mod_Lua->exposeObjMethods(mod_Lua->L, "Dummy", mdM);
	}
#endif
}
