/* mod_dpd.h
 *
 * 	Dead Publisher Detector
 * 	Publish a message if a figure doesn't come on time
 *
 * This file is part of Marcel project and is following the same
 * license rules (see LICENSE file)
 *
 * 06/10/2022 - LF - Migrated to v8
 */

#include "mod_dpd.h"
#include "../Marcel/MQTT_tools.h"

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <sys/select.h>
#include <sys/eventfd.h>
#include <errno.h>

static struct module_dpd mod_dpd;

enum {
	SD_DPD = 0
};

static int publishCustomFiguresDPD(struct Section *asection){
#ifdef LUA
	if(mod_dpd.mod_Lua){
		struct section_dpd *s = (struct section_dpd *)asection;

		lua_newtable(mod_dpd.mod_Lua->L);

		lua_pushstring(mod_dpd.mod_Lua->L, "Topic");			/* Push the index */
		lua_pushstring(mod_dpd.mod_Lua->L, s->section.topic);	/* the value */
		lua_rawset(mod_dpd.mod_Lua->L, -3);	/* Add it in the table */

		lua_pushstring(mod_dpd.mod_Lua->L, "Timeout");			/* Push the index */
		lua_pushnumber(mod_dpd.mod_Lua->L, s->section.sample);	/* the value */
		lua_rawset(mod_dpd.mod_Lua->L, -3);	/* Add it in the table */

		lua_pushstring(mod_dpd.mod_Lua->L, "NotificationTopic");			/* Push the index */
		lua_pushstring(mod_dpd.mod_Lua->L, s->notiftopic);	/* the value */
		lua_rawset(mod_dpd.mod_Lua->L, -3);	/* Add it in the table */

		lua_pushstring(mod_dpd.mod_Lua->L, "Error state");			/* Push the index */
		lua_pushboolean(mod_dpd.mod_Lua->L, s->inerror);	/* the value */
		lua_rawset(mod_dpd.mod_Lua->L, -3);	/* Add it in the table */

		return 1;
	} else
#endif
	return 0;
}

static bool sd_processMQTT(struct Section *asec, const char *topic, char *payload){
	struct section_dpd *s = (struct section_dpd *)asec;

	if(!mqtttokcmp(s->section.topic, topic, NULL)){
		if(s->section.disabled){
#ifdef DEBUG
			if(cfg.debug)
				publishLog('d', "[%s] is disabled", s->section.uid);
#endif
			return true;
	}

		bool ret = true;
#ifdef LUA
		if(mod_dpd.mod_Lua){
			if(s->section.funcid != LUA_REFNIL){	/* if an user function defined ? */
				mod_dpd.mod_Lua->lockState();
				mod_dpd.mod_Lua->pushFunctionId( s->section.funcid );
				mod_dpd.mod_Lua->pushString( s->section.uid );
				mod_dpd.mod_Lua->pushString( s->section.topic );
				mod_dpd.mod_Lua->pushString( payload );
				if(mod_dpd.mod_Lua->exec(3, 1)){
					publishLog('E', "[%s] DPD : %s", s->section.uid, mod_dpd.mod_Lua->getStringFromStack(-1));
					mod_dpd.mod_Lua->pop(1);	/* pop error message from the stack */
					mod_dpd.mod_Lua->pop(1);	/* pop NIL from the stack */
				} else
					ret = mod_dpd.mod_Lua->getBooleanFromStack(-1);	/* Check the return code */
				mod_dpd.mod_Lua->unlockState();
			}
		}
#endif

		if(ret){
			uint64_t v = 1;
			if(write( s->rcv, &v, sizeof(v) ) == -1)	/* Signaling */
				publishLog('E', "[%s] eventfd to signal message reception : %s", s->section.uid, strerror( errno ));
		}
		return true;
	}
	return false;	/* Let's try with other sections */
}

static void *processDPD(void *asec){
	struct section_dpd *s = (struct section_dpd *)asec;	/* avoid lot of casting */

		/* Sanity checks */
	if(!s->section.topic){
		publishLog('F', "[%s] Topic must be set. Dying ...", s->section.uid);
		pthread_exit(0);
	}

	if(!s->section.sample &&!s->section.funcname){
		publishLog('F', "[%s] sample time and function can't be both NULL", s->section.uid);
		pthread_exit(0);
	}

#ifdef LUA
		/* User function */
	if(mod_dpd.mod_Lua){
		if(s->section.funcname){	/* if an user function defined ? */
			if( (s->section.funcid = mod_dpd.mod_Lua->findUserFunc(s->section.funcname)) == LUA_REFNIL ){
				publishLog('E', "[%s] configuration error : user function \"%s\" is not defined. This thread is dying.", s->section.uid, s->section.funcname);
				pthread_exit(NULL);
			}
		}
	}
#endif

		/* event */
	if(( s->rcv = eventfd( 0, 0 )) == -1 ){
		publishLog('E', "[%s] eventfd() : %s", s->section.uid, strerror(errno));
		pthread_exit(0);
	}

		/* watchdog */
	struct timespec ts, *uts;
	if(s->section.sample){
		ts.tv_sec = (time_t)s->section.sample;
		ts.tv_nsec = 0;

		uts = &ts;
	} else
		uts = NULL;


		/* Subscribe */	
	if(MQTTClient_subscribe( cfg.client, s->section.topic, 0 ) != MQTTCLIENT_SUCCESS ){
		publishLog('E', "[%s] Can't subscribe to '%s'", s->section.uid, s->section.topic );
		pthread_exit(0);
	}

		/* processing loop */
	for(;;){
		fd_set rfds;
		FD_ZERO( &rfds );
		FD_SET( s->rcv, &rfds );

		switch( pselect( s->rcv+1, &rfds, NULL, NULL, uts, NULL ) ){
		case -1:	/* Error */
			close( s->rcv );
			s->rcv = 1;	/* stdout, only to avoid pselect() to crash if Keep */
			publishLog(s->section.keep ? 'E' : 'F', "[%s] pselect() : %s", s->section.uid, strerror(errno));
			if(s->section.keep){
				sleep(1);
				continue;
			} else
				pthread_exit(0);
		case 0:	/* Timeout */
			if(s->section.disabled){
#ifdef DEBUG
				if(cfg.debug)
					publishLog('d', "[%s] Alerting is disabled", s->section.uid);
#endif
			} else {
				publishLog('T', "timeout for DPD '%s'", s->section.uid);
				if(!s->inerror){	/* Entering in error condition */
					const char *msg_info = "%s data received after %d seconds";
					char msg[ strlen(msg_info) + 16 ];	/* Some room for 'No' and number of seconds */
					if(!s->notiftopic){		/* No notification topic defined : sending an alert */
						char topic[strlen(s->section.uid) + 7]; /* "Alert/" + 1 */
						strcpy( topic, "Alert/" );
						strcat( topic, s->section.uid );
						int msg_len = sprintf( msg, msg_info, "SNo", (int)s->section.sample );
						if( mqttpublish( cfg.client, topic, msg_len, msg, 0 ) == MQTTCLIENT_SUCCESS )
							s->inerror = true;	/* otherwise let a chance for next run */
					} else {	/* Sending to custom topic */
						char topic[ strlen(s->notiftopic) + strlen(s->section.uid) + 2];	/* + '/' + 0 */
						sprintf( topic, "%s/%s", s->notiftopic, s->section.uid );
						int msg_len = sprintf( msg, msg_info, "No", (int)s->section.sample );
						if( mqttpublish( cfg.client, topic, msg_len, msg, 0 ) == MQTTCLIENT_SUCCESS )
							s->inerror = true;	/* otherwise let a chance for next run */
					}

					publishLog('T', "Alert raises for DPD '%s'", s->section.uid);
				}
			}
			break;
		default: {	/* got some data */
				uint64_t v;

				if(read(s->rcv, &v, sizeof( uint64_t )) == -1)
					publishLog('E', "[%s] eventfd() : %s - reading notification", s->section.uid, strerror(errno));

				if(s->section.disabled){
#ifdef DEBUG
					if(cfg.debug)
						publishLog('d', "[%s] Disabled", s->section.uid);
#endif
				} else {
					if(s->inerror){	/* Exiting error condition */
						if(!s->notiftopic){		/* No notification topic defined : sending an alert */
							char topic[strlen(s->section.uid) + 7]; /* "Alert/" + 1 */
							strcpy( topic, "Alert/" );
							strcat( topic, s->section.uid );
							if( mqttpublish( cfg.client, topic, 1, "E", 0 ) == MQTTCLIENT_SUCCESS )
								s->inerror = false;
						} else {	/* Error topic defined */
							char topic[ strlen(s->notiftopic) + strlen(s->section.uid) + 2];	/* + '/' + 0 */
								/* I duno if it's really needed to have a writable payload,
								 * but anyway, it's safer as per API prototypes
								 */
							const char *msg_info = "Data received : issue corrected";
							size_t msglen = strlen(msg_info);
							char tmsg[ msglen+1 ];
							strcpy( tmsg, msg_info );

							sprintf( topic, "%s/%s", s->notiftopic, s->section.uid );
							if( mqttpublish( cfg.client, topic, msglen, tmsg, 0 ) == MQTTCLIENT_SUCCESS )
								s->inerror = false;

						}
						publishLog('T', "Alert corrected for DPD '%s'", s->section.uid);
					}
				}
			}
			break;
		}
	}

	close(s->rcv);
	s->rcv = 1;
	
	pthread_exit(0);
}

static enum RC_readconf readconf(uint8_t mid, const char *l, struct Section **section ){
	const char *arg;

	if((arg = striKWcmp(l,"*DPD="))){
		if(findSectionByName(arg)){
			publishLog('F', "Section '%s' is already defined", arg);
			exit(EXIT_FAILURE);
		}

		struct section_dpd *nsection = malloc(sizeof(struct section_dpd));
		initSection( (struct Section *)nsection, mid, SD_DPD, strdup(arg), "DPD");

		nsection->section.publishCustomFigures = publishCustomFiguresDPD;
		nsection->notiftopic = NULL;
		nsection->rcv = -1;
		nsection->inerror = false;

		nsection->section.processMsg = sd_processMQTT;	/* process incoming messages */

		if(cfg.verbose)	/* Be verbose if requested */
			publishLog('C', "\tEntering DPD section '%s' (%04x)", nsection->section.uid, nsection->section.id);

		*section = (struct Section *)nsection;
		return ACCEPTED;
	} else if(*section){
		if((arg = striKWcmp(l,"Timeout="))){
			acceptSectionDirective(*section, "Timeout=");
			(*section)->sample = atof(arg);

			if(cfg.verbose)
				publishLog('C', "\t\tTimeout : %lf", (*section)->sample);

			return ACCEPTED;			
		} else if((arg = striKWcmp(l,"NotificationTopic="))){
			acceptSectionDirective(*section, "NotificationTopic=");
			((struct section_dpd *)(*section))->notiftopic = strdup(arg);
			assert(((struct section_dpd *)(*section))->notiftopic);

			if(cfg.verbose)
				publishLog('C', "\t\tNotification Topic : '%s'", ((struct section_dpd *)(*section))->notiftopic);

			return ACCEPTED;			
		}
	}

	return REJECTED;
}

static bool md_acceptSDirective( uint8_t sec_id, const char *directive ){
	if(sec_id == SD_DPD){
		if( !strcmp(directive, "Disabled") )
			return true;	/* Accepted */
		else if( !strcmp(directive, "Keep") )
			return true;	/* Accepted */
		else if( !strcmp(directive, "Topic=") )
			return true;	/* Accepted */
		else if( !strcmp(directive, "Func=") )
			return true;	/* Accepted */
		else if( !strcmp(directive, "Sample=") )
			return true;	/* Accepted */
		else if( !strcmp(directive, "Timeout=") )
			return true;	/* Accepted */
		else if( !strcmp(directive, "NotificationTopic=") )
			return true;	/* Accepted */
	}

	return false;
}

static ThreadedFunctionPtr md_getSlaveFunction(uint8_t sid){
	if(sid == SD_DPD)
		return processDPD;

	/* No slave for Echo : it will process incoming messages */
	return NULL;
}

#ifdef LUA
static int md_inError(lua_State *L){
	struct section_dpd **s = luaL_testudata(L, 1, "DPD");
	luaL_argcheck(L, s != NULL, 1, "'DPD' expected");

	lua_pushboolean(mod_dpd.mod_Lua->L, (*s)->inerror);

	return 1;
}

static const struct luaL_Reg mdM[] = {
	{"inError", md_inError},
	{NULL, NULL}
};
#endif

void InitModule( void ){
	initModule((struct Module *)&mod_dpd, "mod_dpd");

	mod_dpd.module.readconf = readconf;
	mod_dpd.module.acceptSDirective = md_acceptSDirective;
	mod_dpd.module.getSlaveFunction = md_getSlaveFunction;

	registerModule( (struct Module *)&mod_dpd );	/* Register the module */

#ifdef LUA
	uint8_t mod_Lua_id = findModuleByName("mod_Lua");
	if(mod_Lua_id != (uint8_t)-1){ /* Is mod_Lua loaded ? */
		mod_dpd.mod_Lua = (struct module_Lua *)modules[mod_Lua_id];

			/* Expose shared methods */
		mod_dpd.mod_Lua->initSectionSharedMethods(mod_dpd.mod_Lua->L, "DPD");

			/* Expose mod_dpd's own function */
		mod_dpd.mod_Lua->exposeObjMethods(mod_dpd.mod_Lua->L, "DPD", mdM);
	} else
		mod_dpd.mod_Lua = NULL;
#endif
}
