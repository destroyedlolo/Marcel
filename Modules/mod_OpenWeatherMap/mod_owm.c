/* mod_owm
 *
 * Weather forecast using OpenWeatherMap
 *  
 * This file is part of Marcel project and is following the same
 * license rules (see LICENSE file)
 *
 * 03/11/2022 - LF - Move to V8
 */

#include "mod_owm.h"
#ifdef LUA
#	include "../mod_Lua/mod_Lua.h"
#endif

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>

struct module_owm mod_owm;

static int publishCustomFiguresOWM(struct Section *asection){
#ifdef LUA
	if(mod_Lua){
		struct section_OWMQuery *s = (struct section_OWMQuery *)asection;

		lua_newtable(mod_Lua->L);

		lua_pushstring(mod_Lua->L, "Topic");			/* Push the index */
		lua_pushstring(mod_Lua->L, s->section.topic);	/* the value */
		lua_rawset(mod_Lua->L, -3);	/* Add it in the table */

		lua_pushstring(mod_Lua->L, "Sample");			/* Push the index */
		lua_pushnumber(mod_Lua->L, s->section.sample);	/* the value */
		lua_rawset(mod_Lua->L, -3);	/* Add it in the table */
	
		lua_pushstring(mod_Lua->L, "City");			/* Push the index */
		lua_pushstring(mod_Lua->L, s->city);	/* the value */
		lua_rawset(mod_Lua->L, -3);	/* Add it in the table */
	
		lua_pushstring(mod_Lua->L, "Units");			/* Push the index */
		lua_pushstring(mod_Lua->L, s->units);	/* the value */
		lua_rawset(mod_Lua->L, -3);	/* Add it in the table */
	
		lua_pushstring(mod_Lua->L, "lang");			/* Push the index */
		lua_pushstring(mod_Lua->L, s->lang);	/* the value */
		lua_rawset(mod_Lua->L, -3);	/* Add it in the table */

		lua_pushstring(mod_Lua->L, "Immediate");			/* Push the index */
		lua_pushboolean(mod_Lua->L, s->section.immediate);	/* the value */
		lua_rawset(mod_Lua->L, -3);	/* Add it in the table */

		lua_pushstring(mod_Lua->L, "Error state");			/* Push the index */
		lua_pushboolean(mod_Lua->L, s->inerror);	/* the value */
		lua_rawset(mod_Lua->L, -3);	/* Add it in the table */

		return 1;
	} else
#endif
	return 0;
}

	/* Convert weather condition to accurate code */
int convWCode( int code, int dayornight ){
	if( code >=200 && code < 300 )
		return 0;
	else if( code >= 300 && code <= 312 )
		return 9;
	else if( code > 312 && code < 400 )
		return( dayornight ? 39:45 );
	else if( code == 500 || code == 501 )
		return 11;
	else if( code >= 502 && code <= 504 )
		return 12;
	else if( code == 511 )
		return 10;
	else if( code >= 520 && code <= 529 )
		return( dayornight ? 39:45 );
	else if( code == 600)
		return 13;
	else if( code > 600 && code < 610 )
		return 14;
	else if( code == 612 || (code >= 620 && code < 630) )
		return( dayornight ? 41:46 );
	else if( code >= 610 && code < 620 )
		return 5;
	else if( code >= 700 && code < 800 )
		return 21;
	else if( code == 800 )
		return( dayornight ? 32:31 );
	else if( code == 801 )
		return( dayornight ? 34:33 );
	else if( code == 802 )
		return( dayornight ? 30:29 );
	else if( code == 803 || code == 804 )
		return( dayornight ? 28:27 );

	return -1;
}

static enum RC_readconf readconf(uint8_t mid, const char *l, struct Section **section ){
	const char *arg;

	if((arg = striKWcmp(l,"APIkey="))){
		if(*section){
			publishLog('F', "RandomizeProbes can't be part of a section");
			exit(EXIT_FAILURE);
		}

		assert(( mod_owm.apikey = strdup(arg) ));

		if(cfg.verbose)
			publishLog('C', "\tAPIkey : %s", mod_owm.apikey);

		return ACCEPTED;
	} else if((arg = striKWcmp(l,"*MeteoDaily="))){
		if(findSectionByName(arg)){
			publishLog('F', "Section '%s' is already defined", arg);
			exit(EXIT_FAILURE);
		}

		struct section_OWMQuery *nsection = malloc(sizeof(struct section_OWMQuery));	/* Allocate a new section */
		initSection( (struct Section *)nsection, mid, SM_DAILY, strdup(arg), "MeteoDaily");	/* Initialize shared fields */

		nsection->section.sample = DEFAULT_WEATHER_SAMPLE;
		nsection->section.publishCustomFigures = publishCustomFiguresOWM;
		nsection->city = NULL;
		nsection->units = "metric";
		nsection->lang = "en";

		if(cfg.verbose)	/* Be verbose if requested */
			publishLog('C', "\tEntering Daily Weather section '%s' (%04x)", nsection->section.uid, nsection->section.id);

		*section = (struct Section *)nsection;	/* we're now in a section */
		return ACCEPTED;
	} else if((arg = striKWcmp(l,"*Meteo3H="))){
		if(findSectionByName(arg)){
			publishLog('F', "Section '%s' is already defined", arg);
			exit(EXIT_FAILURE);
		}

		struct section_OWMQuery *nsection = malloc(sizeof(struct section_OWMQuery));	/* Allocate a new section */
		initSection( (struct Section *)nsection, mid, SM_3H, strdup(arg), "Meteo3H");	/* Initialize shared fields */

		nsection->section.sample = DEFAULT_WEATHER_SAMPLE;
		nsection->section.publishCustomFigures = publishCustomFiguresOWM;
		nsection->city = NULL;
		nsection->units = "metric";
		nsection->lang = "en";

		if(cfg.verbose)	/* Be verbose if requested */
			publishLog('C', "\tEntering 3H Weather section '%s' (%04x)", nsection->section.uid, nsection->section.id);

		*section = (struct Section *)nsection;	/* we're now in a section */
		return ACCEPTED;
	} else if(*section){
		if((arg = striKWcmp(l,"City="))){
			acceptSectionDirective(*section, "City=");
			assert(( (*(struct section_OWMQuery **)section)->city = strdup(arg) ));

			if(cfg.verbose)	/* Be verbose if requested */
				publishLog('C', "\t\tCity : '%s'", (*(struct section_OWMQuery **)section)->city);
			return ACCEPTED;
		} else if((arg = striKWcmp(l,"Units="))){
			acceptSectionDirective(*section, "Units=");
			assert(( (*(struct section_OWMQuery **)section)->units = strdup(arg) ));

			if(cfg.verbose)	/* Be verbose if requested */
				publishLog('C', "\t\tUnits : '%s'", (*(struct section_OWMQuery **)section)->units);
			return ACCEPTED;
		} else if((arg = striKWcmp(l,"Lang="))){
			acceptSectionDirective(*section, "Lang=");
			assert(( (*(struct section_OWMQuery **)section)->lang = strdup(arg) ));

			if(cfg.verbose)	/* Be verbose if requested */
				publishLog('C', "\t\tLanguage : '%s'", (*(struct section_OWMQuery **)section)->lang);
			return ACCEPTED;
		}
	}

	return REJECTED;
}

static bool acceptSDirective( uint8_t sec_id, const char *directive ){
	if(sec_id == SM_DAILY || sec_id == SM_3H){
		if( !strcmp(directive, "Disabled") )
			return true;	/* Accepted */
		else if( !strcmp(directive, "Immediate") )
			return true;	/* Accepted */
		else if( !strcmp(directive, "Retained") )
			return true;	/* Accepted */
		else if( !strcmp(directive, "Topic=") )
			return true;	/* Accepted */
		else if( !strcmp(directive, "Sample=") )
			return true;	/* Accepted */
		else if( !strcmp(directive, "City=") )
			return true;	/* Accepted */
		else if( !strcmp(directive, "Units=") )
			return true;	/* Accepted */
		else if( !strcmp(directive, "Lang=") )
			return true;	/* Accepted */
	}

	return false;
}

static ThreadedFunctionPtr getSlaveFunction(uint8_t sid){
	if(sid == SM_DAILY)
		return processWFDaily;
	else if(sid == SM_3H)
		return processWF3H;
	return NULL;
}

#ifdef LUA
static int so_inError(lua_State *L){
	struct section_OWMQuery **s = luaL_testudata(L, 1, "MeteoDaily");
	if(!s)
		s = luaL_testudata(L, 1, "Meteo3H");
	luaL_argcheck(L, s != NULL, 1, "'Meteo' expected");

	lua_pushboolean(L, (*s)->inerror);
	return 1;
}

static const struct luaL_Reg soM[] = {
	{"inError", so_inError},
	{NULL, NULL}
};
#endif

void InitModule( void ){
	initModule((struct Module *)&mod_owm, "mod_owm");	/* Identify the module */

		/* Initialize callbacks
		 * It's MANDATORY that all callbacks are initialised
		 */
	mod_owm.module.readconf = readconf;
	mod_owm.module.acceptSDirective = acceptSDirective;
	mod_owm.module.getSlaveFunction = getSlaveFunction;

	mod_owm.apikey = NULL;

	registerModule( (struct Module *)&mod_owm );	/* Register the module */

#ifdef LUA
	if(mod_Lua){ /* Is mod_Lua loaded ? */

			/* Expose shared methods */
		mod_Lua->initSectionSharedMethods(mod_Lua->L, "MeteoDaily");
		mod_Lua->initSectionSharedMethods(mod_Lua->L, "Meteo3H");

			/* Expose mod_owm's own function */
		mod_Lua->exposeObjMethods(mod_Lua->L, "MeteoDaily", soM);
		mod_Lua->exposeObjMethods(mod_Lua->L, "Meteo3H", soM);
	}
#endif
}
