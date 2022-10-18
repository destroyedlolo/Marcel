/* 1WAlarm.c
 *
 * Handle alarm driven probe
 *
 * This file is part of Marcel project and is following the same
 * license rules (see LICENSE file)
 *
 * 11/10/2022 - LF - First version
 */

#include "mod_1wire.h"	/* module's own stuffs */
#ifdef LUA
#	include "../mod_Lua/mod_Lua.h"
#endif
#include "../Marcel/MQTT_tools.h"

void start1WAlarm( uint8_t mid ){
	/* Initialise all 1wAlarm */

	/* First reading if Immediate */

	/* Launch reading thread */
}
