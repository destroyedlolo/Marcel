/* Marcel's core module
 *
 * 	08/09/2022 - LF - First version
 */

#include "module_core.h"

#include <stddef.h>
#include <sys/types.h>
#include <unistd.h>
#include <assert.h>
#include <dlfcn.h>
#include <stdlib.h>

#include <stdio.h>
#include <string.h>

	/* This module is the only one having configuration stored in
	 * "cfg" and not inside its own structure.
	 * Because it's dealing with configuration used globally in Marcel's
	 */
struct module_Core {
	struct module module;
} mod_Core;

static bool mc_readconf(const char *l){
	const char *arg;

	if((arg = striKWcmp(l,"ClientID="))){
		assert(( cfg.ClientID = strdup( arg ) ));
		setSubstitutionVar(vslookup, "%ClientID%", cfg.ClientID, true);
		if(cfg.verbose)
			publishLog('C', "MQTT Client ID : '%s'", cfg.ClientID);
		return true;
	} else if((arg = striKWcmp(l,"Broker="))){
		assert(( cfg.Broker = strdup( arg ) ));
		if(cfg.verbose)
			publishLog('C', "Broker : '%s'", cfg.Broker);
		return true;
	} else if((arg = striKWcmp(l,"LoadModule="))){
		void *pgh;
		char t[strlen(PLUGIN_DIR) + strlen(arg) + 2];
		void (*func)(void);

		sprintf(t, "%s/%s", PLUGIN_DIR, arg);

		if(cfg.verbose)
			publishLog('C', "Loading module '%s'", cfg.debug ? t : arg);

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

		return true;
	}

	return false;
}

void init_module_core(){
	mod_Core.module.name = "mod_core";
	mod_Core.module.readconf = mc_readconf;

	register_module( (struct module *)&mod_Core );


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
	cfg.ConLostFatal = false;

		/* init vslookup */
	setSubstitutionVar(vslookup, "%ClientID%", cfg.ClientID, false);

#ifdef DEBUG
	if(cfg.debug)
		printf("*d* default ClientID : '%s'\n", cfg.ClientID);
#endif
}
