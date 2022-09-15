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
 * Unique identifier of section known by this module
 * Must start by 0 and < 255 
 */

enum {
	ST_TEST = 0
};

/* ***
 * Callback called for each and every line of configuration files
 *
 * Return "true" if the module accept/recognize line content. "false" otherwise
 * and in this case, the line is passed to next module.
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
		mod_test.test = atoi(arg);

		if(cfg.verbose)	/* Be verbose if requested */
			publishLog('C', "\tMod_test's \"test\" variable set to %d", mod_test.test);

		return ACCEPTED;
	} else if(!strcmp(l, "TestFlag")){	/* Directive without argument */
		if(*section){
			publishLog('F', "TestFlag can't be part of a section");
			exit(EXIT_FAILURE);
		}

		/* processing */
		mod_test.flag = true;

		if(cfg.verbose)	/* Be verbose if requested */
			publishLog('C', "\tMod_test's \"flag\" set to 'TRUE'");

		return ACCEPTED;
	} else if((arg = striKWcmp(l,"*TEST="))){	/* Starting a section definition */
		/* By convention, section directives starts by a start '*'.
		 * Its argument is an UNIQUE name : this name is used "OnOff" topic to
		 * identify which section it has to deal with.
		 */

		if(findSectionByName(arg)){
			publishLog('F', "Section '%s' is already defined", arg);
			exit(EXIT_FAILURE);
		}

		struct section_test *section = malloc(sizeof(struct section_test));
		initSection( (struct Section *)section, mid, ST_TEST, "TEST");
	
		return ACCEPTED;
	}

	return REJECTED;
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

	if(findModuleByName(mod_test.module.name) != (uint8_t)-1){
		publishLog('F', "Module '%s' is already loaded", mod_test.module.name);
		exit(EXIT_FAILURE);
	}

	register_module( (struct Module *)&mod_test );	/* Register the module */

		/*
		 * Do internal initialization
		 */
	mod_test.test = 0;
	mod_test.flag = false;
}
