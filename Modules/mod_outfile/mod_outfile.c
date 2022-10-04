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

static struct module_outfile mod_outfile;

enum {
	SOF_OUTFILE = 0
};

static void so_postconfInit(struct Section *asec){
	struct section_outfile *s = (struct section_outfile *)asec;	/* avoid lot of casting */

		/* Sanity checks
		 * As they're highlighting configuration issue, let's
		 * consider error as fatal.
		 */
	if(!s->section.topic){
		publishLog('F', "[%s] Topic can't be NULL", s->section.uid);
		exit( EXIT_FAILURE );
	}
	if(!s->file){
		publishLog('F', "[%s] File can't be NULL", s->section.uid);
		exit( EXIT_FAILURE );
	}

		/* Subscribing */
	if(MQTTClient_subscribe( cfg.client, s->section.topic, 0 ) != MQTTCLIENT_SUCCESS ){
		publishLog('E', "Can't subscribe to '%s'", s->section.topic );
		exit( EXIT_FAILURE );
	}
}

static bool so_processMQTT(struct Section *asec, const char *topic, char *payload ){
	struct section_outfile *s = (struct section_outfile *)asec;	/* avoid lot of casting */

	if(!mqtttokcmp(s->section.topic, topic)){
		publishLog('I', "[%s] %s", s->section.uid, payload);
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
		initSection( (struct Section *)nsection, mid, SOF_OUTFILE, strdup(arg));

		nsection->file = NULL;
		nsection->section.postconfInit = so_postconfInit;
		nsection->section.processMsg = so_processMQTT;

		if(cfg.verbose)	/* Be verbose if requested */
			publishLog('C', "\tEntering section '%s' (%04x)", nsection->section.uid, nsection->section.id);

		*section = (struct Section *)nsection;	/* we're now in a section */
		return ACCEPTED;
	} else if(*section){
		if((arg = striKWcmp(l,"File="))){
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
		else if( !strcmp(directive, "File=") )
			return true;	/* Accepted */
	}

	return false;
}

void InitModule( void ){
	mod_outfile.module.name = "mod_outfile";

	mod_outfile.module.readconf = readconf;
	mod_outfile.module.acceptSDirective = mo_acceptSDirective;
	mod_outfile.module.getSlaveFunction = NULL;
	mod_outfile.module.postconfInit = NULL;

	register_module( (struct Module *)&mod_outfile );
}
