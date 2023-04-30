/* mod_alert
 *
 * Handle notification and alerting
 *
 * This file is part of Marcel project and is following the same
 * license rules (see LICENSE file)
 *
 * 05/11//2022 - LF - First version
 */

#include "mod_alert.h"	/* module's own stuffs */

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>

struct module_alert mod_alert;

enum {
	SA_ALERT=0,
	SA_NOTIF,
	SA_RAISE,
	SA_CORRECT
};

static enum RC_readconf readconf(uint8_t mid, const char *l, struct Section **section ){
	const char *arg;

	if((arg = striKWcmp(l,"AlertsCounterTopic="))){
		if(*section){
			publishLog('F', "AlertCounterTopic can't be part of a section");
			exit(EXIT_FAILURE);
		}

		assert( (mod_alert.countertopic = strdup(arg)) );

		if(cfg.verbose)	/* Be verbose if requested */
			publishLog('C', "\tAlerts Counter's Topic set to \"%s\"", mod_alert.countertopic);

		return ACCEPTED;
	} else if((!strcmp(l,"$unnamedNotification"))){
		if(findSectionByName("$unnamedNotification")){
			publishLog('F', "'$unnamedNotification' section is already defined");
			exit(EXIT_FAILURE);
		}

		struct section_unnamednotification *nsection = malloc(sizeof(struct section_unnamednotification));
		initSection( (struct Section *)nsection, mid, SA_NOTIF, "$unnamedNotification");

		nsection->section.topic = "Notification/#";
		nsection->actions.url = NULL;
		nsection->actions.cmd = NULL;
		nsection->section.postconfInit = notif_postconfInit;
		nsection->section.processMsg = notif_unnamednotification_processMQTT;

		if(cfg.verbose)	/* Be verbose if requested */
			publishLog('C', "\tEntering $UnnamedNotification section (%04x)", nsection->section.id);

		mod_alert.current = NULL;	/* not anymore in a named */
		*section = (struct Section *)nsection;
		return ACCEPTED;
	} else if((arg = striKWcmp(l,"$namedNotification="))){
		if(arg[1]){
			publishLog('F', "[%s] \"$namedNotification=\"'s name can be only 1 character long", arg);
			exit(EXIT_FAILURE);
		}

		if(findNamed(*arg)){
			publishLog('F', "\"$namedNotification=%c\" is already defined", *arg);
			exit(EXIT_FAILURE);
		}

		struct namednotification *nnamed = malloc(sizeof(struct namednotification));
		assert(nnamed);
		nnamed->next = mod_alert.nnotif;
		nnamed->actions.url = NULL;
		nnamed->actions.cmd = NULL;
		nnamed->name = *arg;
		nnamed->disabled = false;

		if(cfg.verbose)	/* Be verbose if requested */
			publishLog('C', "\tEntering $namedNotification '%c'", *arg);

		mod_alert.nnotif = nnamed;
		mod_alert.current = nnamed;
		*section = NULL;	/* not anymore in a section */
		return ACCEPTED;
	} if((!strcmp(l,"$alert"))){
		if(findSectionByName("$alert")){
			publishLog('F', "'$alert' section is already defined");
			exit(EXIT_FAILURE);
		}

		struct section_alert *nsection = malloc(sizeof(struct section_alert));
		initSection( (struct Section *)nsection, mid, SA_ALERT, "$alert");

			/* This section is processing MQTT messages */
		nsection->section.topic = "Alert/#";
		nsection->section.postconfInit = malert_postconfInit;
		nsection->section.processMsg = malert_alert_processMQTT;

			/* Module's own fields */
		nsection->actions.url = NULL;
		nsection->actions.cmd = NULL;

		if(cfg.verbose)	/* Be verbose if requested */
			publishLog('C', "\tEntering $alert section (%04x)", nsection->section.id);

		mod_alert.current = NULL;	/* not anymore in a named */
		*section = (struct Section *)nsection;
		return ACCEPTED;
	} else if((arg = striKWcmp(l,"*RaiseAlert="))){
		if(findSectionByName(arg)){
			publishLog('F', "Section '%s' is already defined", arg);
			exit(EXIT_FAILURE);
		}

		struct section_raise *nsection = malloc(sizeof(struct section_raise));
		initSection( (struct Section *)nsection, mid, SA_RAISE, strdup(arg));		

			/* This section is processing MQTT messages */
		nsection->section.postconfInit = malert_postconfInit;	/* Subscribe */
		nsection->section.processMsg = salrt_raisealert_processMQTT;	/* Processing */

			/* Module's own fields */
		nsection->actions.url = NULL;
		nsection->actions.cmd = NULL;

		if(cfg.verbose)	/* Be verbose if requested */
			publishLog('C', "\tEntering RaiseAlert '%s' section (%04x)", nsection->section.uid ,nsection->section.id);

		mod_alert.current = NULL;	/* not anymore in a named */
		*section = (struct Section *)nsection;
		return ACCEPTED;
	} else if((arg = striKWcmp(l,"*CorrectAlert="))){
		if(findSectionByName(arg)){
			publishLog('F', "Section '%s' is already defined", arg);
			exit(EXIT_FAILURE);
		}

		struct section_raise *nsection = malloc(sizeof(struct section_correct));
		initSection( (struct Section *)nsection, mid, SA_CORRECT, strdup(arg));		

			/* This section is processing MQTT messages */
		nsection->section.postconfInit = malert_postconfInit;	/* Subscribe */
		nsection->section.processMsg = salrt_correctalert_processMQTT;	/* Processing */

			/* Module's own fields */
		nsection->actions.url = NULL;
		nsection->actions.cmd = NULL;

		if(cfg.verbose)	/* Be verbose if requested */
			publishLog('C', "\tEntering CorrectAlert '%s' section (%04x)", nsection->section.uid ,nsection->section.id);

		mod_alert.current = NULL;	/* not anymore in a named */
		*section = (struct Section *)nsection;
		return ACCEPTED;
	} else if(*section || mod_alert.current){
		if((arg = striKWcmp(l,"RESTUrl="))){
			if(mod_alert.current){	/* Named notification */
				assert(( mod_alert.current->actions.url = strdup(arg) ));

				if(cfg.verbose)
					publishLog('C', "\t\tRESTUrl : '%s'", mod_alert.current->actions.url);

				return ACCEPTED;
			} else {	/* Unamed notification or alert */
				acceptSectionDirective(*section, "RESTUrl=");
				((struct section_unnamednotification *)(*section))->actions.url = strdup(arg);
				assert(((struct section_unnamednotification *)(*section))->actions.url);

				if(cfg.verbose)
					publishLog('C', "\t\tRESTUrl : '%s'", ((struct section_unnamednotification *)(*section))->actions.url);

				return ACCEPTED;
			}
		} else	if((arg = striKWcmp(l,"OSCmd="))){
			if(mod_alert.current){	/* Named notification */
				assert(( mod_alert.current->actions.cmd = strdup(arg) ));

				if(cfg.verbose)
					publishLog('C', "\t\tOSCmd : '%s'", mod_alert.current->actions.cmd);

				return ACCEPTED;
			} else {	/* Unamed notification or alert */
				acceptSectionDirective(*section, "OSCmd=");
				((struct section_unnamednotification *)(*section))->actions.cmd = strdup(arg);
				assert(((struct section_unnamednotification *)(*section))->actions.cmd);

				if(cfg.verbose)
					publishLog('C', "\t\tOSCmd : '%s'", ((struct section_unnamednotification *)(*section))->actions.cmd);

				return ACCEPTED;
			}
		} else if(!strcmp(l,"Disabled") && mod_alert.current){
				/* Needed as alerts/notifications are stored alone and not
				 * considered as sections.
				 */
			mod_alert.current->disabled = true;

			if(cfg.verbose)
				publishLog('C', "\t\tDisabled");

			return ACCEPTED;
		}
	}
	return REJECTED;
}

static bool acceptSDirective( uint8_t sec_id, const char *directive ){
	if(sec_id == SA_NOTIF || sec_id == SA_ALERT){
		if( !strcmp(directive, "RESTUrl=") )
			return true;	/* Accepted */
		else if( !strcmp(directive, "OSCmd=") )
			return true;	/* Accepted */
		else if( !strcmp(directive, "Disabled") )
			return true;	/* Accepted */
	} else if(sec_id == SA_RAISE || sec_id == SA_CORRECT){
		if( !strcmp(directive, "RESTUrl=") )
			return true;	/* Accepted */
		else if( !strcmp(directive, "OSCmd=") )
			return true;	/* Accepted */
		else if( !strcmp(directive, "Disabled") )
			return true;	/* Accepted */
		else if( !strcmp(directive, "Topic=") )
			return true;	/* Accepted */
	}

	return false;
}

/* Subscribe ONLY if named notification are used */
static void subTopic( uint8_t mid ){
	if(mod_alert.nnotif){
		if( MQTTClient_subscribe( cfg.client, "nNotification/#", 0) != MQTTCLIENT_SUCCESS ){
			publishLog('F', "Can't subscribe to 'nNotification/#'");
			exit(EXIT_FAILURE);
		}
	} else if(cfg.verbose)
		publishLog('I', "Named notification disabled");
}

static bool processMsg(const char *topic, char *payload){
	const char *arg;

	/* processing named sessions */
	if((arg = striKWcmp(topic, "nNotification/"))){	/* Mustn't include wildcard otherwise, use mqtttokcmp() */
		char *title = strchr(arg, '/');

		if(!title)
			publishLog('E', "Received named notification \"%s\" without title : ignoring", arg);
		else {
			while(arg < title){
				char sec = *arg++;
				struct namednotification *n = findNamed(sec);
				publishLog('T', "Received named notification \"%c\" (%s) title \"%s\"", sec, n ? "found":"unknown", title+1);

				if(n){
					if(n->disabled){
#ifdef DEBUG
						if(cfg.debug)
							publishLog('d', "Named notification \"%c\" is disabled", n->name);
#endif
						continue;
					}

					if(n->actions.cmd)
						execOSCmd(n->actions.cmd, title+1, payload);
					if(n->actions.url)
						execRest(n->actions.url, title+1, payload);
				}
			}
		}
		return true;	/* Even if notification are unknown, we processed the message */
	}

	return false;
}

#ifdef LUA
static int lmRiseAlert(lua_State *L){
	if(lua_gettop(L) != 2){
		publishLog('E', "In your Lua code, RiseAlert() requires 2 arguments : title, message");
		return 0;
	}

	struct section_alert *s = (struct section_alert *)findSectionByName("$alert");
	if(!s)
		publishLog('E', "No $alert defined");
	else if(!s->section.disabled){
		const char *id = luaL_checkstring(L, 1);
		const char *msg = luaL_checkstring(L, 2);
		if(RiseAlert(id, msg))
			execOSCmd(s->actions.cmd, id, msg);
	} else if(cfg.debug)
		publishLog('T', "Alert ignored : $alert is disabled");
	
	return 0;
}

static int lmRiseAlertREST(lua_State *L){
	if(lua_gettop(L) != 2){
		publishLog('E', "In your Lua code, RiseAlertREST() requires 2 arguments : title, message");
		return 0;
	}

	struct section_alert *s = (struct section_alert *)findSectionByName("$alert");
	if(!s)
		publishLog('E', "No $alert defined");
	else if(!s->section.disabled){
		const char *id = luaL_checkstring(L, 1);
		const char *msg = luaL_checkstring(L, 2);
		if(RiseAlert(id, msg)){
			execOSCmd(s->actions.cmd, id, msg);
			execRest(s->actions.url, id, msg);
		}
	} else if(cfg.debug)
		publishLog('T', "Alert ignored : $alert is disabled");
	
	return 0;
}

static int lmClearAlert(lua_State *L){
	if(lua_gettop(L) != 1 && lua_gettop(L) != 2){
		publishLog('E', "In your Lua code, ClearAlert() requires at least 1 argument : title + optionally message");
		return 0;
	}

	struct section_alert *s = (struct section_alert *)findSectionByName("$alert");
	if(!s)
		publishLog('E', "No $alert defined");
	else if(!s->section.disabled){
		const char *id = luaL_checkstring(L, 1);
		const char *msg = luaL_checkstring(L, 2);
		
		if(!msg)
			msg = "";

		if(AlertIsOver(id, msg)){
			execOSCmd(s->actions.cmd, id, msg);
			execRest(s->actions.url, id, msg);
		}
	} else if(cfg.debug)
		publishLog('T', "Alert ignored : $alert is disabled");
	
	return 0;
}

static int lmSendAlertsCounter(lua_State *L){
	sentAlertsCounter();

	return 0;
}

static int lmSendNotification(lua_State *L){
	if(lua_gettop(L) != 2){
		publishLog('E', "In your Lua code, SendNotification() requires 2 arguments : title and message");
		return 0;
	}

	struct section_alert *s = (struct section_alert *)findSectionByName("$unnamedNotification");
	if(!s)
		publishLog('E', "No $unnamedNotification defined");
	else if(!s->section.disabled){
		const char *id = luaL_checkstring(L, 1);
		const char *msg = luaL_checkstring(L, 2);
		execOSCmd(s->actions.cmd, id, msg);
	} else if(cfg.debug)
		publishLog('T', "Notification not sent : $unnamedNotification is disabled");
	
	return 0;
}

static int lmSendNotificationREST(lua_State *L){
	if(lua_gettop(L) != 2){
		publishLog('E', "In your Lua code, SendNotification() requires 2 arguments : title and message");
		return 0;
	}

	struct section_alert *s = (struct section_alert *)findSectionByName("$unnamedNotification");
	if(!s)
		publishLog('E', "No $unnamedNotification defined");
	else if(!s->section.disabled){
		const char *id = luaL_checkstring(L, 1);
		const char *msg = luaL_checkstring(L, 2);
		execOSCmd(s->actions.cmd, id, msg);
		execRest(s->actions.url, id, msg);
	} else if(cfg.debug)
		publishLog('T', "Notification not sent : $unnamedNotification is disabled");
	
	return 0;
}

static const struct luaL_Reg ModAlertLib [] = {
	{"RiseAlert", lmRiseAlert},
	{"RiseAlertSMS", lmRiseAlertREST},	/* compatibility only */
	{"RiseAlertREST", lmRiseAlertREST},
	{"ClearAlert", lmClearAlert},
	{"SendAlertsCounter", lmSendAlertsCounter},
	{"SendMessage", lmSendNotification},	/* compatibility only */
	{"SendNotification", lmSendNotification},
	{"SendMessageSMS", lmSendNotificationREST},	/* compatibility only */
	{"SendNotificationREST", lmSendNotificationREST},
	{NULL, NULL}
};
#endif

void InitModule( void ){
	initModule((struct Module *)&mod_alert, "mod_alert");

	mod_alert.module.readconf = readconf;
	mod_alert.module.acceptSDirective = acceptSDirective;
	mod_alert.module.postconfInit = subTopic;	/* Named subscription if needed */
	mod_alert.module.processMsg = processMsg;	/* Named subscription */

	mod_alert.nnotif = NULL;
	mod_alert.current = NULL;

	DLListInit(&mod_alert.alerts);

	mod_alert.countertopic = NULL;

	registerModule( (struct Module *)&mod_alert );	/* Register the module */

#ifdef LUA
	uint8_t mod_Lua_id = findModuleByName("mod_Lua");
	if(mod_Lua_id != (uint8_t)-1){ /* Is mod_Lua loaded ? */
		mod_alert.mod_Lua = (struct module_Lua *)modules[mod_Lua_id];

			/* Expose mod_alert's own function */
		mod_alert.mod_Lua->exposeFunctions("Marcel", ModAlertLib);
	}
#endif
}
