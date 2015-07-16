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

extern void init_alerting(void);
extern void rcv_alert(const char *id, const char *msg);

#endif
