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
#include "../Marcel/DList.h"

#ifdef LUA
#	include "../mod_Lua/mod_Lua.h"
#endif

extern struct module_alert mod_alert;

struct actions {
	char *url;
	char *cmd;
};

struct section_unnamednotification {
	struct Section section;

	struct actions actions;
};

struct section_alert {
	struct Section section;

	struct actions actions;
};

struct section_raise {
	struct Section section;

	struct actions actions;
};

struct section_correct {
	struct Section section;

	struct actions actions;
};

struct namednotification {
	struct namednotification *next;

	struct actions actions;
	char name;
	bool disabled;
};

	/* Active alert list */
struct alert {
	struct DLNode node;
	const char *alert;
};

	/* Custom structure to store module's configuration */
struct module_alert {
	struct Module module;

	struct namednotification *nnotif, 	/* named notification list */
		*current;						/* current named */

	struct DList alerts;				/* Alerts' list */

	const char *countertopic;			/* Topic to send counter too */

#ifdef LUA
	struct module_Lua *mod_Lua;
#endif
};

extern struct module_alert mod_alert;

	/* **
	 * Unamed notification
	 * **/
extern void notif_postconfInit(struct Section *);
extern bool notif_unnamednotification_processMQTT(struct Section *, const char *, char *);

	/* **
	 * Alerts
	 * **/
extern void malert_postconfInit(struct Section *);
extern bool malert_alert_processMQTT(struct Section *, const char *, char *);

	/* **
	 * RaiseAlert
	 * **/
extern bool salrt_raisealert_processMQTT(struct Section *, const char *, char *);

bool RiseAlert(const char *id, const char *msg);
bool AlertIsOver(const char *id, const char *msg);

	/* **
	 * CorrectAlert
	 * **/
extern bool salrt_correctalert_processMQTT(struct Section *, const char *, char *);

	/* **
	 * Named notification
	 * **/
extern struct namednotification *findNamed(const char );

	/* **
	 * Actions
	 * **/
extern void execOSCmd(const char *, const char *, const char *);
extern void execRest(const char *, const char *, const char *);
#endif
