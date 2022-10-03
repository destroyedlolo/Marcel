/* in order to speedup compilation it's better to split large source files in
 * multiple chunks.
 * Here, as example, dummy section processing is externalized
 *
 * This file is part of Marcel project and is following the same
 * license rules (see LICENSE file)
 */


/* ***
 * Marcel's own include 
 * ***/

#include "mod_dummy.h"	/* module's own stuffs */

/* ***
 * System's include
 * ***/

bool st_echo_processMQTT( const char *topic, char *payload ){
	return true;
}
