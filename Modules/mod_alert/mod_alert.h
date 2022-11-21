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

struct section_namednotification;

/* Custom structure to store module's configuration */
struct module_alert {
	struct Module module;

	bool alert_used;
	bool unotif_used;
};

struct section_namednotification {
	struct Section section;

	char *url;
	char *cmd;
};
#endif
