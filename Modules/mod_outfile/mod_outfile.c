/* mod_outfile
 *
 * Write value to file
 *
 * This file is part of Marcel project and is following the same
 * license rules (see LICENSE file)
 *
 */

#include "mod_outfile.h"
#include "../Marcel/MQTT_tools.h"
#ifdef LUA
#	include "../mod_Lua/mod_Lua.h"
#endif

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>

static struct module_outfile mod_outfile;

enum {
	SOF_OUTFILE = 0
};

static int publishCustomFiguresOF(struct Section *asection){
#ifdef LUA
	if(mod_Lua){
		struct section_outfile *s = (struct section_outfile *)asection;

		lua_newtable(mod_Lua->L);

		lua_pushstring(mod_Lua->L, "File");			/* Push the index */
		lua_pushstring(mod_Lua->L, s->file);	/* the value */
		lua_rawset(mod_Lua->L, -3);	/* Add it in the table */

		lua_pushstring(mod_Lua->L, "Error state");			/* Push the index */
		lua_pushboolean(mod_Lua->L, s->inerror);	/* the value */
		lua_rawset(mod_Lua->L, -3);	/* Add it in the table */

		return 1;
	} else
#endif
	return 0;
}

static void so_postconfInit(struct Section *asec){
	struct section_outfile *s = (struct section_outfile *)asec;	/* avoid lot of casting */

		/* Sanity checks */
	if(!s->section.topic){
		publishLog('F', "[%s] Topic must be set. Dying ...", s->section.uid);
		pthread_exit(0);
	}

	if(!s->file){
		publishLog('E', "[%s] File must be set. Dying ...", s->section.uid);
		pthread_exit(0);
	}

#ifdef LUA
	if(mod_Lua){
		if(s->section.funcname){	/* if an user function defined ? */
			if( (s->section.funcid = mod_Lua->findUserFunc(s->section.funcname)) == LUA_REFNIL ){
					publishLog('F', "[%s] configuration error : user function \"%s\" is not defined. This thread is dying.", s->section.uid, s->section.funcname);
					pthread_exit(NULL);
				}
			}
		}
#endif

		/* Subscribing */
	if(MQTTClient_subscribe( cfg.client, s->section.topic, 0 ) != MQTTCLIENT_SUCCESS){
		publishLog('E', "Can't subscribe to '%s'", s->section.topic );
		exit( EXIT_FAILURE );
	}
}

static bool so_processMQTT(struct Section *asec, const char *topic, char *payload ){
	struct section_outfile *s = (struct section_outfile *)asec;	/* avoid lot of casting */

	if(!mqtttokcmp(s->section.topic, topic, NULL)){
		if(s->section.disabled){
#ifdef DEBUG
			if(cfg.debug)
				publishLog('d', "[%s] is disabled", s->section.uid);
#endif
			return true;
		}

		bool ret = true;
#ifdef LUA
		if(mod_Lua){
			if(s->section.funcid != LUA_REFNIL){	/* if an user function defined ? */
				mod_Lua->lockState();
				mod_Lua->pushFunctionId( s->section.funcid );
				mod_Lua->pushString( s->section.uid );
				mod_Lua->pushString( payload );
				if(mod_Lua->exec(2, 1)){
					publishLog('E', "[%s] Outfile : %s", s->section.uid, mod_Lua->getStringFromStack(-1));
					mod_Lua->pop(1);	/* pop error message from the stack */
					mod_Lua->pop(1);	/* pop NIL from the stack */
				} else
					ret = mod_Lua->getBooleanFromStack(-1);	/* Check the return code */
				mod_Lua->unlockState();
			}
		}
#endif

		if(ret){
			s->inerror = true;	/* By default, we're falling */

			FILE *f=fopen( s->file, "w" );
			if(!f){
				publishLog('E', "[%s] '%s' : %s", s->section.uid, s->file, strerror(errno));
				return true;
			}
			fputs(payload, f);
			if(ferror(f)){
				publishLog('E', "[%s] '%s' : %s", s->section.uid, s->file, strerror(errno));
				fclose(f);
				return true;
			}
			s->inerror = false;
			publishLog('T', "[%s] '%s' written in '%s'", s->section.uid, payload, s->file);
			fclose(f);
		} else 
			publishLog('T', "[%s] Not write due to Lua function", s->section.uid);

		return true;	/* we processed the message */
	}

	return false;	/* Let's try with other sections */
}

static enum RC_readconf readconf(uint8_t mid, const char *l, struct Section **section ){
	const char *arg;

	if((arg = striKWcmp(l,"*OutFile="))){
		if(findSectionByName(arg)){
			publishLog('F', "Section '%s' is already defined", arg);
			exit(EXIT_FAILURE);
		}

		struct section_outfile *nsection = malloc(sizeof(struct section_outfile));
		initSection( (struct Section *)nsection, mid, SOF_OUTFILE, strdup(arg), "OutFile");

		nsection->section.publishCustomFigures = publishCustomFiguresOF;
		nsection->file = NULL;
		nsection->section.postconfInit = so_postconfInit;
		nsection->section.processMsg = so_processMQTT;

		if(cfg.verbose)	/* Be verbose if requested */
			publishLog('C', "\tEntering OutFile section '%s' (%04x)", nsection->section.uid, nsection->section.id);

		*section = (struct Section *)nsection;	/* we're now in a section */
		return ACCEPTED;
	} else if(*section){
		if((arg = striKWcmp(l,"File="))){
			acceptSectionDirective(*section, "File=");
			assert(( (*(struct section_outfile **)section)->file = strdup(arg) ));

			if(cfg.verbose)	/* Be verbose if requested */
				publishLog('C', "\t\tFile : '%s'", (*(struct section_outfile **)section)->file);

			return ACCEPTED;
		}
	}

	return REJECTED;
}

static bool mo_acceptSDirective( uint8_t sec_id, const char *directive ){
	if(sec_id == SOF_OUTFILE){
		if( !strcmp(directive, "Disabled") )
			return true;	/* Accepted */
		else if( !strcmp(directive, "Topic=") )
			return true;	/* Accepted */
		else if( !strcmp(directive, "Func=") )
			return true;	/* Accepted */
		else if( !strcmp(directive, "File=") )
			return true;	/* Accepted */
	}

	return false;
}

#ifdef LUA
static int so_inError(lua_State *L){
	struct section_outfile **s = luaL_testudata(L, 1, "OutFile");
	luaL_argcheck(L, s != NULL, 1, "'OutFile' expected");

	lua_pushboolean(L, (*s)->inerror);
	return 1;
}

static const struct luaL_Reg soM[] = {
	{"inError", so_inError},
	{NULL, NULL}
};
#endif

void InitModule( void ){
	initModule((struct Module *)&mod_outfile, "mod_outfile");

	mod_outfile.module.readconf = readconf;
	mod_outfile.module.acceptSDirective = mo_acceptSDirective;

	registerModule( (struct Module *)&mod_outfile );

#ifdef LUA
	if(mod_Lua){ /* Is mod_Lua loaded ? */

			/* Expose shared methods */
		mod_Lua->initSectionSharedMethods(mod_Lua->L, "OutFile");

			/* Expose module's own function */
		mod_Lua->exposeObjMethods(mod_Lua->L, "OutFile", soM);
	}
#endif
}
