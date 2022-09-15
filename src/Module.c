/* Module.c
 * 	module managements
 *
 * 	08/09/2022 - LF - First version
 */

#include "Module.h"

#include <stdio.h>
#include <stdlib.h>

uint8_t number_of_loaded_modules = 0;
struct Module *modules[MAX_MODULES];

/**
 * @brief Search for a section
 *
 * @param name Name of the section we are looking for
 * @return uid of this module (-1 if not found)
 */
uint8_t findModuleByName(const char *name){
	int h = chksum(name);

	for(int i = 0; i < number_of_loaded_modules; i++){
		if(modules[i]->h == h && !strcmp(name, modules[i]->name))
			return i;
	}

	return (uint8_t)-1;	/* Not found */
}

/**
 * Register a module.
 * Add it in the liste of known modules
 */
void register_module( struct Module *mod ){
	if(number_of_loaded_modules >= MAX_MODULES){
		publishLog('F',"Too many registered modules. Increase MAX_MODULES\n", stderr);
		exit( EXIT_FAILURE );
	}

	mod->module_index = number_of_loaded_modules;
	mod->h = chksum(mod->name);
	modules[number_of_loaded_modules++] = mod;
}
