/* module.h
 * 	module managements
 *
 * This file is part of Marcel project and is following the same
 * license rules (see LICENSE file)
 *
 * 	08/09/2022 - LF - First version
 */

#ifndef MODULE_H
#define MODULE_H

#include "Marcel.h"
#include "Section.h"

	/* Modules are stored in a fixed length table. Their indexes are used
	 * to reference them is sections (which module/method is handling this section)
	 *
	 * This value may have to be increased when new modules are added to Marcel ...
	 * up to 255. 
	 */
#define MAX_MODULES	16

	/* Return code of readconf() callback */
enum RC_readconf {
	REJECTED = 0,	/* The module doesn't recognize the directive */
	ACCEPTED,		/* The module proceed the directive */
	SKIP_FILE		/* This configuration file has to be skipped */
};

typedef void *(*ThreadedFunctionPtr) (void *);	/* Function to call in thread */

struct Module {
	const char *name;		/* module's name */
	int h;					/* hash code for the name */
	uint8_t module_index;

	enum RC_readconf (*readconf)( uint8_t mod_id, const char *, struct Section ** );	/* is provided line apply to this module (true) */
	bool (*acceptSDirective)( uint8_t sec_id, const char * );	/* process a directive */
	ThreadedFunctionPtr(*getSlaveFunction)(uint8_t sid);		/* function to call to process a section */
	void (*postconfInit)(void);										/* Initialisation to be done after configuration phase */
};

extern uint8_t number_of_loaded_modules;
extern struct Module *modules[];

extern void acceptSectionDirective( struct Section *section, const char *directive );
extern uint8_t findModuleByName(const char *name);
extern void register_module( struct Module * );
#endif
