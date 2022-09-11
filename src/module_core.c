/* Marcel's core module
 *
 * 	08/09/2022 - LF - First version
 */

#include "module_core.h"

#include <stddef.h>
#include <sys/types.h>
#include <unistd.h>
#include <assert.h>

#include <stdio.h>
#include <string.h>

struct module_Core {
	struct module module;
} mod_Core;

static bool mc_readconf(const char *l){
	const char *arg;

	if((arg = striKWcmp(l,"ClientID="))){
		assert( (cfg.ClientID = strdup( arg )) );
		setSubstitutionVar(vslookup, "%ClientID%", cfg.ClientID, true);
		if(cfg.verbose)
			publishLog('C', "MQTT Client ID : '%s'", cfg.ClientID);
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
