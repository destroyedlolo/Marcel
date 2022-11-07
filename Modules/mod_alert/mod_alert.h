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

/* Storage for named notification */
struct notification {
	struct notification *next;
	char id;
	char *url;
	char *cmd;
};

/* Custom structure to store module's configuration */
struct module_alert {
	struct Module module;

	char alert_name;	/* "Alert/" corresponding named notification */
	char notif_name;	/* "Notification/" corresponding notification */

	struct notification *notiflist;

	bool alertgrouped;	/* Alert are grouped (Optimisation) */
};

#endif
