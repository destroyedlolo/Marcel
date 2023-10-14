/* mod_dpd.h
 *
 * 	Dead Publisher Detector
 * 	Publish a message if a figure doesn't come on time
 *
 * This file is part of Marcel project and is following the same
 * license rules (see LICENSE file)
 *
 * 06/10/2022 - LF - Migrated to v8
 */

#ifndef MOD_DPD_H
#define MOD_DPD_H

#include "../Marcel/Module.h"
#include "../Marcel/Section.h"

#ifdef LUA
#	include "../mod_Lua/mod_Lua.h"
#endif

struct module_dpd {
	struct Module module;

#ifdef LUA
	struct module_Lua *mod_Lua;
#endif
};

struct section_dpd {
	struct Section section;

	const char *notiftopic;	/* Topic to publish error to */
	int rcv;				/* Event for data receiving */
	bool inerror;			/* true if this DPD is in error */
};
#endif
