/* mod_freebox
 *
 *  Publish FreeboxOS figures
 *
 * This file is part of Marcel project and is following the same
 * license rules (see LICENSE file)
 *
 * 24/09/2023 - LF - First version
 */

#include "mod_freebox.h"
#include "../Marcel/MQTT_tools.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#if 0
#include <unistd.h>
#include <errno.h>
#endif

static struct module_freebox mod_freebox;

enum {
	SFB_FREEBOX = 0
};

static enum RC_readconf readconf(uint8_t mid, const char *l, struct Section **asection ){
	struct section_freebox **section = (struct section_freebox **)asection;
	const char *arg;

	if((arg = striKWcmp(l,"*Freebox="))){	/* Starting a section definition */
		if(findSectionByName(arg)){
			publishLog('F', "Section '%s' is already defined", arg);
			exit(EXIT_FAILURE);
		}

		struct section_freebox *nsection = malloc(sizeof(struct section_freebox));
		initSection( (struct Section *)nsection, mid, SFB_FREEBOX, strdup(arg));

		nsection->url = NULL;
		nsection->app_token = NULL;

		if(cfg.verbose)	/* Be verbose if requested */
			publishLog('C', "\tEntering Freebox section '%s' (%04x)", nsection->section.uid, nsection->section.id);

		*section = nsection;	/* we're now in a section */
		return ACCEPTED;
	} else if(*section){
		if((arg = striKWcmp(l,"URL="))){
/* Not needed as only one section exists
			acceptSectionDirective(*section, "URL=");
*/
			if((*(struct section_freebox **)section)->url){
				publishLog('F', "['%s'] Url defined twice", (*section)->section.uid);
				exit(EXIT_FAILURE);
			}
			assert(( (*section)->url = strdup(arg) ));

			if(cfg.verbose)	/* Be verbose if requested */
				publishLog('C', "\t\tURL : '%s'", (*section)->url);
	
			return ACCEPTED;
		}
	}

	return REJECTED;
}

static bool mfb_acceptSDirective( uint8_t sec_id, const char *directive ){
	if(sec_id == SFB_FREEBOX){
		if( !strcmp(directive, "Disabled") )
			return true;
		else if( !strcmp(directive, "URL=") )
			return true;
		else if( !strcmp(directive, "Sample=") )
			return true;
		else if( !strcmp(directive, "Topic=") )
			return true;
		else if( !strcmp(directive, "Immediate") )
			return true;
		else if( !strcmp(directive, "Keep") )
			return true;
	}

	return false;
}

void InitModule( void ){
	initModule((struct Module *)&mod_freebox, "mod_freebox");

	mod_freebox.module.readconf = readconf;
	mod_freebox.module.acceptSDirective = mfb_acceptSDirective;
#if 0
	mod_freebox.module.getSlaveFunction = mfb_getSlaveFunction;
#endif

	registerModule( (struct Module *)&mod_freebox );
}
