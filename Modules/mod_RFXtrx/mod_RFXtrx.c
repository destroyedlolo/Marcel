/* mod_RFXtrx
 *
 * Handle RFXtrx devices (like the RFXCom)
 *
 * This file is part of Marcel project and is following the same
 * license rules (see LICENSE file)
 *
 */

#include "mod_RFXtrx.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>

static struct module_RFXtrx mod_RFXtrx;

/* Section identifiers */
enum {
	ST_CMD = 0,	/* RFXtrx commands */
};

static enum RC_readconf readconf(uint8_t mid, const char *l, struct Section **section ){
	const char *arg; /* Argument of the configuration directive */

	if((arg = striKWcmp(l,"RFXtrx_Port="))){ /* "serial" port where RFXcom is plugged */
		if(*section){
			publishLog('F', "RFXtrx_Port= can't be part of a section");
			exit(EXIT_FAILURE);
		}
		if(mod_RFXtrx.RFXdevice){
			publishLog('F', "RFXtrx_Port= can be only defined once");
			exit(EXIT_FAILURE);
		}
		assert( (mod_RFXtrx.RFXdevice = strdup(arg)) );

		if(cfg.verbose)
			publishLog('C', "\tRFXtrx's device : '%s'", mod_RFXtrx.RFXdevice);

		return ACCEPTED;
	} else if((arg = striKWcmp(l,"*RTSCmd="))){	/* Starting a section definition */
		if(findSectionByName(arg)){
			publishLog('F', "Section '%s' is already defined", arg);
			exit(EXIT_FAILURE);
		}

		struct section_RFXCom *nsection = malloc(sizeof(struct section_RFXCom));
		initSection( (struct Section *)nsection, mid, ST_CMD, strdup(arg));

		nsection->did = 0;

		if(cfg.verbose)
			publishLog('C', "\tEntering RTSCmd section '%s' (%04x)", nsection->section.uid, nsection->section.id);

		*section = (struct Section *)nsection;	/* we're now in a section */
		return ACCEPTED;
	} else if(section){
		if((arg = striKWcmp(l,"ID="))){
			acceptSectionDirective(*section, "ID=");
			((struct section_RFXCom *)section)->did = strtol(arg, NULL, 0);

			if(cfg.verbose)
				publishLog('C', "\t\tID : %04x", ((struct section_RFXCom *)section)->did);

			return ACCEPTED;
		}
	}

	return REJECTED;
}

static bool mr_acceptSDirective( uint8_t sec_id, const char *directive ){
	if(sec_id == ST_CMD){
		if( !strcmp(directive, "Disabled") )
			return true;
		else if( !strcmp(directive, "ID=") )
			return true;
		else if( !strcmp(directive, "Topic=") )
			return true;
	}
	return false;
}

void InitModule( void ){
	initModule((struct Module *)&mod_RFXtrx, "mod_RFXtrx"); /* Identify the module */

		/* Callbacks */
	mod_RFXtrx.module.readconf = readconf;
	mod_RFXtrx.module.acceptSDirective = mr_acceptSDirective;

		/* Register the module */
	registerModule( (struct Module *)&mod_RFXtrx );

		/*
		 * Do internal initialization
		 */
	mod_RFXtrx.RFXdevice = NULL;
}
