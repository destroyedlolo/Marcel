/* module.h
 * 	module managements
 *
 * 	08/09/2022 - LF - First version
 */

#ifndef MODULE_H

#include "Marcel.h"

	/* Modules are stored in a fixed length table. Their indexes are used
	 * to reference them is sections (which module/method is handling this section)
	 *
	 * This value may have to be increased when new modules are added to Marcel ...
	 * up to 255. 
	 */
#define MAX_MODULES	16

struct Module {
	const char *name;					/* module's name */
	int module_index;

	bool (*readconf)( const char * );	/* is provided line apply to this module (true) */
};

extern unsigned int numbe_of_loaded_modules;
extern struct Module *modules[];

extern void register_module( struct Module * );

#endif
