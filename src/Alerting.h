/* Alerting.h
 * 	Definitions related to Alerting
 *
 * This file is part of Marcel project and is following the same
 * license rules (see LICENSE file)
 *
 * 16/07/2015	- LF - First version
 */

#ifndef ALERTING_H
#define ALERTING_H

#include "DList.h"

	/* Active alert list */
struct alert {
	struct DLNode node;
	const char *alert;
};

extern struct DList alerts;

extern void init_alerting(void);
extern void RiseAlert(const char *id, const char *msg);
extern void AlertIsOver(const char *id);

extern void rcv_alert(const char *id, const char *msg);

#endif
