/* module.h
 * 	module managements
 *
 * 	08/09/2022 - LF - First version
 */

#ifndef MODULE_H

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

struct Module {
	const char *name;		/* module's name */
	int h;					/* hash code for the name */
	uint8_t module_index;

	enum RC_readconf (*readconf)( uint8_t mod_id, const char *, struct Section ** );	/* is provided line apply to this module (true) */
};

extern uint8_t number_of_loaded_modules;
extern struct Module *modules[];

extern uint8_t findModuleByName(const char *name);
extern void register_module( struct Module * );
#endif
