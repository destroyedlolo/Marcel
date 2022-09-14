/* Module.c
 * 	module managements
 *
 * 	08/09/2022 - LF - First version
 */

#include "Module.h"

#include <stdio.h>
#include <stdlib.h>

unsigned int numbe_of_loaded_modules = 0;
struct Module *modules[MAX_MODULES];

/**
 * Register a module.
 * Add it in the liste of known modules
 */
void register_module( struct Module *mod ){
	if(numbe_of_loaded_modules >= MAX_MODULES){
		publishLog('F',"Too many registered modules. Increase MAX_MODULES\n", stderr);
		exit( EXIT_FAILURE );
	}

	mod->module_index = numbe_of_loaded_modules;
	modules[numbe_of_loaded_modules++] = mod;
}
