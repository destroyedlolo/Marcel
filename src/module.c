/* module.c
 * 	module managements
 *
 * 	08/09/2022 - LF - First version
 */

#include "module.h"

#include <stdio.h>
#include <stdlib.h>

int numbe_of_loaded_modules = 0;
struct module *modules[MAX_MODULES];

/**
 * Register a module.
 * Add it in the liste of known modules
 */
void register_module( struct module *mod ){
	if(numbe_of_loaded_modules >= MAX_MODULES){
		fputs("*F* too many registered modules. Increase MAX_MODULES\n", stderr);
		exit( EXIT_FAILURE );
	}

	mod->module_index = numbe_of_loaded_modules;
	modules[numbe_of_loaded_modules++] = mod;
}