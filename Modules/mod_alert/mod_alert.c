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
		initSection( (struct Section *)nsection, mid, SA_NOTIF, "$unnamedNotification", "unnamedNotification");

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

		if(*arg == '/'){
			publishLog('F', "[%s] \"$namedNotification=\"'s name can't be a slash", arg);
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
		initSection( (struct Section *)nsection, mid, SA_ALERT, "$alert", "alert");

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
		initSection( (struct Section *)nsection, mid, SA_RAISE, strdup(arg), "RaiseAlert");		

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
		initSection( (struct Section *)nsection, mid, SA_CORRECT, strdup(arg), "CorrectAlert");		

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
		else if( !strcmp(directive, "Quiet") )
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
		else 
			pnNotify(arg, ++title, payload);

		return true;	/* Even if notification are unknown, we processed the message */
	}

	return false;
}

#ifdef LUA
static int amRiseAlert(lua_State *L){
	struct section_alert *s;
	const char *sname;
	uint8_t type;

	switch(lua_gettop(L)){
	case 2:
		sname = "$alert";
		type = SA_ALERT;
		break;
	case 3:
		sname = luaL_checkstring(L, 3);
		type = SA_RAISE;
		break;
	default:
		publishLog('E', "In your Lua code, RiseAlert() requires 2 or 3 arguments : title, message [,name]");
		return 0;
	}

	if(!(s = (struct section_alert *)findSectionByName(sname))){
		publishLog('E', "[%s] not defined", sname);
		return 0;
	} else if(s->section.id != (type << 8 | mod_alert.module.module_index)){
		publishLog('E', "RaiseAlert() : A section is named '%s' but it's not an %s definition", sname, type == SA_ALERT ? "alert" : "RaiseAlert");
		return 0;
	} else if(!s->section.disabled){
		const char *id = luaL_checkstring(L, 1);
		const char *msg = luaL_checkstring(L, 2);
		if(RiseAlert(id, msg, s->section.quiet)){
			execOSCmd(s->actions.cmd, id, msg);
			if(type == SA_RAISE)
				execRest(s->actions.url, id, msg);
		}
	} else if(cfg.debug)
		publishLog('T', "Alert ignored : [%s] is disabled", sname);
	
	return 0;
}

static int amRiseAlertREST(lua_State *L){
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
		if(RiseAlert(id, msg, s->section.quiet)){
			execOSCmd(s->actions.cmd, id, msg);
			execRest(s->actions.url, id, msg);
		}
	} else if(cfg.debug)
		publishLog('T', "Alert ignored : $alert is disabled");
	
	return 0;
}

static int amClearAlert(lua_State *L){
	struct section_alert *s;
	const char *sname;
	uint8_t type;

	switch(lua_gettop(L)){
	case 1:
	case 2:
		sname = "$alert";
		type = SA_ALERT;
		break;
	case 3:
		sname = luaL_checkstring(L, 3);
		type = SA_CORRECT;
		break;
	default:
		publishLog('E', "In your Lua code, ClearAlert() requires 1 to 3 arguments : title [, message [,name]]");
		return 0;
	}

	if(!(s = (struct section_alert *)findSectionByName(sname))){
		publishLog('E', "[%s] not defined", sname);
		return 0;
	} else if(s->section.id != (type << 8 | mod_alert.module.module_index)){
		publishLog('E', "ClearAlert() : A section is named '%s' but it's not an %s definition", sname, type == SA_ALERT ? "alert" : "CorrectAlert");
		return 0;
	} else if(!s->section.disabled){
		const char *id = luaL_checkstring(L, 1);
		const char *msg = lua_tostring(L, 2);
		if(AlertIsOver(id, msg, s->section.quiet)){
			execOSCmd(s->actions.cmd, id, msg);
			execRest(s->actions.url, id, msg);
		}
	} else if(cfg.debug)
		publishLog('T', "Alert ignored : $alert is disabled");

	return 0;
}

static int amSendAlertsCounter(lua_State *L){
	sentAlertsCounter();

	return 0;
}

static int amSendNotification(lua_State *L){
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

static int amSendNotificationREST(lua_State *L){
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

static int amSendNamedNotification(lua_State *L){
	if(lua_gettop(L) != 3){
		publishLog('E', "In your Lua code, SendNamedNotification() requires 3 arguments : Alerts' names, title and message");
		return 0;
	}

	const char *names = luaL_checkstring(L, 1);
	const char *topic = luaL_checkstring(L, 2);
	const char *msg = luaL_checkstring(L, 3);
	pnNotify( names, topic, msg );
	
	return 0;

}

static int amListAlert(lua_State *L){
	int n=0;

	for(struct alert *an = (struct alert *)mod_alert.alerts.first; an; an = (struct alert *)an->node.next){
		lua_pushstring(L, an->alert);
		n++;
	}

	return n;
}

static void pushNamedNObject(lua_State *L, struct namednotification *obj){
	struct namednotification **s = (struct namednotification **)lua_newuserdata(L, sizeof(struct namednotification *));
	if(!s)
		luaL_error(L, "No memory");

	*s = obj;

	luaL_getmetatable(L, "NamedNotification");
	lua_setmetatable(L, -2);
}

static int am_ainter(lua_State *L){
	if(mod_alert.pnotif){
		pushNamedNObject(L, mod_alert.pnotif);
		mod_alert.pnotif = mod_alert.pnotif->next;

		return 1;
	} else
		return 0;
}

static int amNNotifs(lua_State *L){
	mod_alert.pnotif = mod_alert.nnotif;

	lua_pushcclosure(L, am_ainter, 1);

	return 1;
}

static int amFNNotifs(lua_State *L){
	const char *nname = luaL_checkstring(L, 1);

	struct namednotification *s = findNamed(*nname);
	if(!s)
		return 0;

	pushNamedNObject(L, s);
	return 1;
}


static const struct luaL_Reg ModAlertLib [] = {
	{"RiseAlert", amRiseAlert},
	{"RiseAlertSMS", amRiseAlertREST},	/* compatibility only */
	{"RiseAlertREST", amRiseAlertREST},
	{"ClearAlert", amClearAlert},
	{"SendAlertsCounter", amSendAlertsCounter},
	{"SendMessage", amSendNotification},	/* compatibility only */
	{"SendNotification", amSendNotification},
	{"SendMessageSMS", amSendNotificationREST},	/* compatibility only */
	{"SendNotificationREST", amSendNotificationREST},
	{"SendNamedMessage", amSendNamedNotification},	/* compatibility only */
	{"SendNamedNotification", amSendNamedNotification},
	{"ListAlert", amListAlert},

	{"NamedNotifications", amNNotifs},
	{"FindNamedNotifications", amFNNotifs},

	{NULL, NULL}
};

static int amn_getName(lua_State *L){
	struct namednotification **s = luaL_testudata(L, 1, "NamedNotification");
	luaL_argcheck(L, s != NULL, 1, "'NamedNotification' expected");

	char name[] = { (*s)->name, 0 };
	
	lua_pushstring(L, name);

	return 1;
}

static int amn_isEnabled(lua_State *L){
	struct namednotification **s = luaL_testudata(L, 1, "NamedNotification");
	luaL_argcheck(L, s != NULL, 1, "'NamedNotification' expected");

	lua_pushboolean(L, !(*s)->disabled);

	return 1;
}

static const struct luaL_Reg alNamedM[] = {
	{"getName", amn_getName},
	{"isEnabled", amn_isEnabled},
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

	mod_alert.findNamedNotificationByName = findNamed;
	mod_alert.namedNNDisable = namedNNDisable;

	registerModule( (struct Module *)&mod_alert );	/* Register the module */

#ifdef LUA
	if(mod_Lua){ /* Is mod_Lua loaded ? */

			/* Expose shared methods */
		mod_Lua->initSectionSharedMethods(mod_Lua->L, "unnamedNotification");
		mod_Lua->initSectionSharedMethods(mod_Lua->L, "alert");
		mod_Lua->initSectionSharedMethods(mod_Lua->L, "RaiseAlert");
		mod_Lua->initSectionSharedMethods(mod_Lua->L, "CorrectAlert");

			/* Expose mod_alert's own function */
		mod_Lua->exposeFunctions("mod_alert", ModAlertLib);

			/* Expose NamedNotification's own function */
		mod_Lua->exposeObjMethods(mod_Lua->L, "NamedNotification", alNamedM);
	}
#endif
}
