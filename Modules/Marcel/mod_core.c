/* Marcel's core module
 *
 * This file is part of Marcel project and is following the same
 * license rules (see LICENSE file)
 *
 * 	08/09/2022 - LF - First version
 */

#include "mod_core.h"

#include <stddef.h>
#include <sys/types.h>
#include <unistd.h>
#include <assert.h>
#include <dlfcn.h>
#include <stdlib.h>

#include <stdio.h>
#include <string.h>

	/* ***
	 * Lookup table to applicate on all configuration files
	 * ***/

struct _VarSubstitution vslookup[] = {
	{ "%ClientID%", NULL },
	{ "%Hostname%", NULL },
	{ NULL }
};


	/* This module is the only one having configuration stored in
	 * "cfg" and not inside its own structure.
	 * Because it's dealing with configuration used globally in Marcel's
	 */
struct module_Core {
	struct Module module;
} mod_Core;

static enum RC_readconf mc_readconf(uint8_t mid, const char *l, struct Section **section ){
	const char *arg;

	if((arg = striKWcmp(l,"ClientID="))){
		if(*section){
			publishLog('F', "ClientID= can't be part of a section");
			exit(EXIT_FAILURE);
		}

		assert(( cfg.ClientID = strdup( arg ) ));
		setSubstitutionVar(vslookup, "%ClientID%", cfg.ClientID, true);
		if(cfg.verbose)
			publishLog('C', "\tMQTT Client ID : '%s'", cfg.ClientID);
		return ACCEPTED;
	} else if((arg = striKWcmp(l,"Broker="))){
		if(*section){
			publishLog('F', "Broker= can't be part of a section");
			exit(EXIT_FAILURE);
		}

		assert(( cfg.Broker = strdup( arg ) ));
		if(cfg.verbose)
			publishLog('C', "\tBroker : '%s'", cfg.Broker);
		return ACCEPTED;
	} else if((arg = striKWcmp(l,"LoadModule="))){
		void *pgh;
		char t[strlen(PLUGIN_DIR) + strlen(arg) + 2];
		void (*func)(void);

		if(*section){
			publishLog('F', "LoadModule= can't be part of a section");
			exit(EXIT_FAILURE);
		}

		sprintf(t, "%s/%s", PLUGIN_DIR, arg);

		if(cfg.verbose)
			publishLog('C', "\tLoading module '%s'", cfg.debug ? t : arg);

		if(!(pgh = dlopen(t, RTLD_LAZY))){
			publishLog('F', "Can't load plug-in : %s\n", dlerror());
			exit(EXIT_FAILURE);
		}
		dlerror(); /* Clear any existing error */

		if(!(func = dlsym( pgh, "InitModule" ))){
			publishLog('F', "Can't find plug-in init function : %s\n", dlerror());
			exit(EXIT_FAILURE);
		}
		(*func)();

		return ACCEPTED;
	} else if((arg = striKWcmp(l,"Needs="))){
		if(*section){
			publishLog('F', "Needs= can't be part of a section");
			exit(EXIT_FAILURE);
		}

		if(findModuleByName(arg) == (uint8_t)-1){
#ifdef DEBUG
			if(cfg.debug)
				publishLog('C', "\tConfiguration file ignored : '%s' plug-in not loaded", arg);
#endif
			return SKIP_FILE;
		}
		return ACCEPTED;

	} else if(!strcmp(l, "SubLast")){
			if(cfg.verbose)
				publishLog('C', "\tSubscription topics are grouped at the end");
			
			cfg.sublast = true;
			return ACCEPTED;
			

		/* ***************
		 * 	Here starting section's shared directives
		 * ***************/

	} else if(*section){
		if(!strcmp(l, "Disabled")){
			acceptSectionDirective( *section, l);

			(*section)->disabled = true;

			if(cfg.verbose)
				publishLog('C', "\t\tStarting DISABLED");

			return ACCEPTED;
		} else if(!strcmp(l, "Quiet")){
			acceptSectionDirective( *section, l);

			(*section)->quiet = true;

			if(cfg.verbose)
				publishLog('C', "\t\tStarting QUIET");

			return ACCEPTED;
		} else if((arg = striKWcmp(l, "Sample="))){
			acceptSectionDirective( *section, "Sample=" );

			(*section)->sample = atof(arg);

			if(cfg.verbose)
				publishLog('C', "\t\tSample time : %lf", (*section)->sample);

			return ACCEPTED;
		} else if((arg = striKWcmp(l, "Topic="))){
			acceptSectionDirective( *section, "Topic=" );

			(*section)->topic = strdup(arg);
			assert((*section)->topic);

			if(cfg.verbose)
				publishLog('C', "\t\tTopic : '%s'", (*section)->topic);

			return ACCEPTED;
		} else if(!strcmp(l, "Retained")){
			acceptSectionDirective( *section, l );

			(*section)->retained = true;

			if(cfg.verbose)
				publishLog('C', "\t\tSent as retained");

			return ACCEPTED;
		} else if(!strcmp(l, "Immediate")){
			acceptSectionDirective( *section, l );

			(*section)->immediate = true;

			if(cfg.verbose)
				publishLog('C', "\t\tRun immediately");

			return ACCEPTED;
		} else if(!strcmp(l, "Keep")){
			acceptSectionDirective( *section, l );

			(*section)->keep = true;

			if(cfg.verbose)
				publishLog('C', "\t\tKeep");

			return ACCEPTED;
		}
	}

	return REJECTED;
}

void init_module_core(){
	initModule((struct Module *)&mod_Core, "mod_core");
	mod_Core.module.readconf = mc_readconf;
	registerModule( (struct Module *)&mod_Core );

		/* initialize substitution variables */
	
	char t[256];	/* Arbitrary size (larger is indecent) */
	gethostname(t, 256);
	t[255] = 0;		/* as gethostname() is not guaranteed to provide \0 string */

	char *hn = strdup(t);
	assert(hn);
	setSubstitutionVar(vslookup, "%Hostname%", hn, false);

	snprintf(t, 255, "Marcel.%x.%s", getpid(),hn);

		/* Default values */
	
	assert((cfg.ClientID = strdup(t)));
	cfg.Broker = "tcp://localhost:1883";
	cfg.client = NULL;

		/* init vslookup */
	setSubstitutionVar(vslookup, "%ClientID%", cfg.ClientID, false);

#ifdef DEBUG
	if(cfg.debug)
		printf("*d* default ClientID : '%s'\n", cfg.ClientID);
#endif
}
