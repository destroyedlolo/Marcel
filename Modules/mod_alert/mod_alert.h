/* mod_alert
 *
 * Handle notification and alerting
 *
 * This file is part of Marcel project and is following the same
 * license rules (see LICENSE file)
 *
 * 05/11//2022 - LF - First version
 */

#ifndef MOD_ALERT_H
#define MOD_ALERT_H

#include "../Marcel/Module.h"
#include "../Marcel/Section.h"

extern struct module_alert mod_alert;

struct actions {
	char *url;
	char *cmd;
};

struct section_unnamednotification {
	struct Section section;

	struct actions actions;
};

struct namednotification {
	struct namednotification *next;

	struct actions actions;
	char name;
	bool disabled;
};

/* Custom structure to store module's configuration */
struct module_alert {
	struct Module module;

	struct namednotification *nnotif, 	/* named notification list */
		*current;						/* current named */
};

extern void notif_postconfInit(struct Section *);
extern bool notif_unnamednotification_processMQTT(struct Section *, const char *, char *);

extern struct namednotification *findNamed(const char );

extern void execOSCmd(const char *, const char *, const char *);
extern void execRest(const char *, const char *, const char *);
#endif
