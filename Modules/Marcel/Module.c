/* Module.c
 * 	module managements
 *
 * 	This file is part of Marcel project and is following the same
 * license rules (see LICENSE file)
 *
 * 	08/09/2022 - LF - First version
 */

#include "Module.h"

#include <stdio.h>
#include <stdlib.h>

uint8_t number_of_loaded_modules = 0;
struct Module *modules[MAX_MODULES];

/**
 * @brief check if a directive is applicable to the given section of mid module
 *
 * @param section section on which to apply the directive (MUST be non-NULL)
 * @param directive
 *
 * Raise a fatal error if the option is not application. If this function returns, it's
 * because the directive is accepted.
 */
void acceptSectionDirective( struct Section *section, const char *directive ){
	uint8_t mid = section->id & 0xff;
	uint8_t sid = (section->id >> 8) & 0xff;

	if(mid >= number_of_loaded_modules){
		publishLog('F',"Internal error : module ID larger than # loaded modules");
		exit( EXIT_FAILURE );
	}

	if( !(modules[mid]->acceptSDirective && modules[mid]->acceptSDirective(sid,directive)) ){
		publishLog('F', "'%s' not allowed here", directive);
		exit(EXIT_FAILURE);
	}
}


/**
 * @brief Search for a module
 *
 * @param name Name of the module we are looking for
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
 * @brief Initialise module's field
 * @param module to initialize
 * @param name Name of the module to initialize
 */
void initModule( struct Module *mod, const char *name){
	mod->name = name;
	mod->readconf = NULL;
	mod->acceptSDirective = NULL;
	mod->getSlaveFunction = NULL;
	mod->postconfInit = NULL;
	mod->processMsg = NULL;
}

/**
 * @brief Register a module.
 * Add it in the list of known modules
 */
void registerModule( struct Module *mod ){
	if(findModuleByName(mod->name) != (uint8_t)-1){
		publishLog('F', "Module '%s' is already loaded", mod->name);
		exit(EXIT_FAILURE);
	}

	if(number_of_loaded_modules >= MAX_MODULES){
		publishLog('F',"Too many registered modules. Increase MAX_MODULES");
		exit( EXIT_FAILURE );
	}

	mod->module_index = number_of_loaded_modules;
	mod->h = chksum(mod->name);
	modules[number_of_loaded_modules++] = mod;
}
