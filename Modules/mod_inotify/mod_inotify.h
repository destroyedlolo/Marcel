/* mod_inotify
 *
 * Notification for filesystem changes
 *
 * This file is part of Marcel project and is following the same
 * license rules (see LICENSE file)
 *
 * 29/10/2022 - LF - Emancipate from Marcel.c as a standalone module
 */

#ifndef MOD_INOTIFY_H
#define MOD_INOTIFY_H

#include "../Marcel/Module.h"
#include "../Marcel/Section.h"

struct section_Look4Change;

struct module_inotify {
	struct Module module;

	bool grouped;	/* All LookForChanges are grouped */

	pthread_t thread;		/* Slave thread to handle the notification */
	int infd;

	struct section_Look4Change *first_section;	/* Pointer to the first section handled by mod_inotify */
};

	/* Section identifiers */
enum {
	SI_L4C= 0,
};

struct section_Look4Change {
	struct Section section;

	const char *dir;	/* Directory (or file) to monitor */
	uint32_t flags;		/* What to survey */
	int wd;
};

#endif
