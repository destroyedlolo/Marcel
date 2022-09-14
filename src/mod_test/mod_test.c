/* mod_test
 *
 * This "fake" module only shows how to create a module for Marcel
 *
 * 13/09/2022 - LF - First version
 */

/* ***
 * Marcel's own include 
 * ***/

#include "mod_test.h"	/* module's own stuffs */

/* ***
 * System's include
 * ***/

#include <stdlib.h>
#include <string.h>

/* ***
 * instantiate module's structure.
 * ***/

struct module_test mod_test;

/* ***
 * Callback called for each and every line of configuration files
 *
 * Return "true" if the module accept/recognize line content. "false" otherwise
 * and in this case, the line is passed to next module.
 */

static bool readconf(const char *l){
	const char *arg;	/* Argument of the configuration directive */

		/* Parse the command line.
		 * striKWcmp() checks if the file start with the given token.
		 * Returns a pointer to directive's argument or NULL if the token
		 * is not recognized.
		 */

	if((arg = striKWcmp(l,"TestValue="))){	/* Directive with an argument */
			/* Processing ... */
		mod_test.test = atoi(arg);

		if(cfg.verbose)	/* Be verbose if requested */
			publishLog('C', "\tMod_test's \"test\" variable set to %d", mod_test.test);

		return true;
	} else if(!strcmp(l, "TestFlag")){	/* Directive without argument */
		/* processing */
		mod_test.flag = true;

		if(cfg.verbose)	/* Be verbose if requested */
			publishLog('C', "\tMod_test's \"flag\" set to 'TRUE'");

		return true;
	}

	return false;
}

/* ***
 * InitModule() - Module's initialisation function
 *
 * This function MUST exist and is called when the module is loaded.
 * Its goal is to initialise module's configuration and register the module.
 * If needed, it can also do some internal initialisation work for the module.
 * ***/

void InitModule( void ){
		/*
		 * Initialize module declarations
		 */
	mod_test.module.name = "mod_test";	/* Identify the module */
	mod_test.module.readconf = readconf; /* Initialize callbacks */

	register_module( (struct Module *)&mod_test );	/* Register the module */

		/*
		 * Do internal initialization
		 */
	mod_test.test = 0;
	mod_test.flag = false;
}
