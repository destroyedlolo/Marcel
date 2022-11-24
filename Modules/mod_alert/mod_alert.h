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

/* Custom structure to store module's configuration */
struct module_alert {
	struct Module module;
};

struct section_namednotification {
	struct Section section;

	char *url;
	char *cmd;
};

extern void notif_postconfInit(struct Section *);
extern bool notif_unnamednotification_processMQTT(struct Section *, const char *, char *);

extern void execOSCmd(const char *, const char *, const char *);
extern void execRest(const char *, const char *, const char *);
#endif
