/* mod_ups.h
 *
 * Retrieves UPS figures through its NUT server
 *
 * This file is part of Marcel project and is following the same
 * license rules (see LICENSE file)
 *
 * 29/09/2022 - LF - First version
 */

#include "mod_ups.h"	/* module's own stuffs */
#include "../Marcel/MQTT_tools.h"

#ifdef LUA
#	include "../mod_Lua/mod_Lua.h"
#endif

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

static struct module_ups mod_ups;

enum {
	ST_UPS= 0
};

static int publishCustomFiguresUPS(struct Section *asection){
#ifdef LUA
	if(mod_Lua){
		struct section_ups *s = (struct section_ups *)asection;

		lua_newtable(mod_Lua->L);

		lua_pushstring(mod_Lua->L, "host");			/* Push the index */
		lua_pushstring(mod_Lua->L, s->host);	/* the value */
		lua_rawset(mod_Lua->L, -3);	/* Add it in the table */

		lua_pushstring(mod_Lua->L, "port");			/* Push the index */
		lua_pushnumber(mod_Lua->L, s->port);	/* the value */
		lua_rawset(mod_Lua->L, -3);	/* Add it in the table */

		lua_pushstring(mod_Lua->L, "Sample");			/* Push the index */
		lua_pushnumber(mod_Lua->L, s->section.sample);	/* the value */
		lua_rawset(mod_Lua->L, -3);	/* Add it in the table */

		lua_pushstring(mod_Lua->L, "Topic");			/* Push the index */
		lua_pushstring(mod_Lua->L, s->section.topic);	/* the value */
		lua_rawset(mod_Lua->L, -3);	/* Add it in the table */

		lua_pushstring(mod_Lua->L, "Keep");			/* Push the index */
		lua_pushboolean(mod_Lua->L, s->section.keep);	/* the value */
		lua_rawset(mod_Lua->L, -3);	/* Add it in the table */

		lua_pushstring(mod_Lua->L, "Error state");			/* Push the index */
		lua_pushboolean(mod_Lua->L, s->section.inerror);	/* the value */
		lua_rawset(mod_Lua->L, -3);	/* Add it in the table */

		return 1;
	} else
#endif
	return 0;
}

void *process_UPS(void *actx){
	struct section_ups *ctx = (struct section_ups *)actx;
	char l[MAXLINE];
	struct hostent *server;
	struct sockaddr_in serv_addr;

		/* Sanity checks */
	if(!ctx->section.topic){
		publishLog('F', "[%s] Topic must be set. Dying ...", ctx->section.uid);
		SectionError((struct Section *)ctx, true);
		pthread_exit(0);
	}

	if( !ctx->host || !ctx->port ){
		publishLog('E', "[%s] NUT server missing. Dying ...", ctx->section.uid);
		SectionError((struct Section *)ctx, true);
		pthread_exit(0);
	}

	if( !ctx->section.sample ){
		publishLog('E', "[%s] No sample time. Dying ...", ctx->section.uid);
		SectionError((struct Section *)ctx, true);
		pthread_exit(0);
	}

	if(!(server = gethostbyname( ctx->host ))){
		publishLog('E', "[%s] %s : Don't know how to reach NUT. Dying", ctx->section.uid, strerror(errno));
		SectionError((struct Section *)ctx, true);
		pthread_exit(0);
	}

	memset( &serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons( ctx->port );
	memcpy(&serv_addr.sin_addr.s_addr,*server->h_addr_list,server->h_length);

	if(cfg.verbose)
		publishLog('I', "Launching a processing flow for UPS/%s", ctx->section.uid);

	for(;;){	/* Infinite loop to process data */
		bool inerror = true;

		if(ctx->section.disabled){
			inerror = false;
			publishLog('T', "Reading UPS/%s is disabled", ctx->section.uid);
		} else {
			int sockfd = socket(AF_INET, SOCK_STREAM, 0);
			if(sockfd < 0){
				publishLog('E', "[%s] Can't create socket : %s", ctx->section.uid, strerror( errno ));
				if(!ctx->section.keep){
					publishLog('E', "[%s] Dying", ctx->section.uid);
				}
			} else {
				if(connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0){
					publishLog('E', "[%s] Connecting : %s", ctx->section.uid, strerror( errno ));
					if(!ctx->section.keep){
						publishLog('E', "[%s] Dying", ctx->section.uid);
					}
				} else {
					for(struct var *v = ctx->var_list; v; v = v->next){
						sprintf(l, "GET VAR %s %s\n", ctx->section.uid, v->name);
						if( send(sockfd, l , strlen(l), 0) == -1 ){
							publishLog('E', "[%s] Sending : %s", ctx->section.uid, strerror( errno ));

							if(!ctx->section.keep){
								publishLog('F', "[%s] Dying", ctx->section.uid);
								SectionError((struct Section *)ctx, true);
								pthread_exit(0);
							}
						} else {
							inerror = false;

							char *ps, *pe;
							socketreadline(sockfd, l, sizeof(l));
							if(!( ps = strchr(l, '"')) || !( pe = strchr(ps+1, '"') ))
								publishLog('W', "[%s] %s : unexpected result '%s'", ctx->section.uid, v->name, l);
							else {
								ps++; *pe++ = 0;	/* Extract only the result */
								assert(pe - l + strlen(ctx->section.topic) + strlen(v->name) + 2 < MAXLINE ); /* ensure there is enough place for the topic name */
								sprintf( pe, "%s/%s", ctx->section.topic, v->name );
								mqttpublish( cfg.client, pe, strlen(ps), ps, ctx->section.retained );
								publishLog('T', "[%s] UPS : %s -> '%s'", ctx->section.uid, pe, ps);
							}
						}
					}
				}
				close(sockfd);
			}
		}

		SectionError((struct Section *)ctx, inerror);

			/* Wait for next sample time */
		struct timespec ts;
		ts.tv_sec = (time_t)ctx->section.sample;
		ts.tv_nsec = (unsigned long int)((ctx->section.sample - (time_t)ctx->section.sample) * 1e9);

		nanosleep( &ts, NULL );
	}
	pthread_exit(0);

}

static enum RC_readconf readconf(uint8_t mid, const char *l, struct Section **asection ){
	struct section_ups **section = (struct section_ups **)asection;
	const char *arg;

	if((arg = striKWcmp(l,"*UPS="))){	/* Starting a section definition */
		if(findSectionByName(arg)){
			publishLog('F', "Section '%s' is already defined", arg);
			exit(EXIT_FAILURE);
		}

		struct section_ups *nsection = malloc(sizeof(struct section_ups));
		initSection( (struct Section *)nsection, mid, ST_UPS, strdup(arg), "UPS");

		nsection->section.publishCustomFigures = publishCustomFiguresUPS;
		nsection->host = NULL;
		nsection->port = 0;
		nsection->var_list = NULL;

		if(cfg.verbose)	/* Be verbose if requested */
			publishLog('C', "\tEntering UPS section '%s' (%04x)", nsection->section.uid, nsection->section.id);

		*section = nsection;	/* we're now in a section */
		return ACCEPTED;
	} else if(*section){
		if((arg = striKWcmp(l,"Host="))){
			acceptSectionDirective(*asection, "Host=");
			(*section)->host = strdup(arg);
			assert( (*section)->host );

			if(cfg.verbose)
				publishLog('C', "\t\tNUT's host : '%s'", (*section)->host);

			return ACCEPTED;
		} else if((arg = striKWcmp(l,"Port="))){
			acceptSectionDirective(*asection, "Port=");
			(*section)->port = atoi(arg);

			if(cfg.verbose)
				publishLog('C', "\t\tNUT's port : %u", (*section)->port);

			return ACCEPTED;
		} else if((arg = striKWcmp(l,"Var="))){
			acceptSectionDirective(*asection, "Var=");
			struct var *v = malloc(sizeof(struct var));	/* New variable */
			assert(v);
			assert( (v->name = strdup( arg )) );

			v->next = (*section)->var_list;	/* add it in the list */
			(*section)->var_list = v;

			if(cfg.verbose)
				publishLog('C', "\t\tVar : '%s'", v->name);

			return ACCEPTED;
		}
	}

	return REJECTED;
}

static bool mu_acceptSDirective( uint8_t sec_id, const char *directive ){
	if(sec_id == ST_UPS){
		if( !strcmp(directive, "Disabled") )
			return true;
		else if( !strcmp(directive, "Sample=") )
			return true;
		else if( !strcmp(directive, "Topic=") )
			return true;
		else if( !strcmp(directive, "Host=") )
			return true;
		else if( !strcmp(directive, "Port=") )
			return true;
		else if( !strcmp(directive, "Var=") )
			return true;
		else if( !strcmp(directive, "Keep") )
			return true;
	}

	return false;
}

ThreadedFunctionPtr mu_getSlaveFunction(uint8_t sid){
	if(sid == ST_UPS)
		return process_UPS;

	return NULL;
}

#ifdef LUA
static int so_inError(lua_State *L){
	struct section_ups **s = luaL_testudata(L, 1, "UPS");
	luaL_argcheck(L, s != NULL, 1, "'UPS' expected");

	lua_pushboolean(L, (*s)->section.inerror);
	return 1;
}

static const struct luaL_Reg soM[] = {
	{"inError", so_inError},
	{NULL, NULL}
};
#endif

void InitModule( void ){
	initModule((struct Module *)&mod_ups, "mod_ups");

	mod_ups.module.readconf = readconf;
	mod_ups.module.acceptSDirective = mu_acceptSDirective;
	mod_ups.module.getSlaveFunction = mu_getSlaveFunction;

	registerModule( (struct Module *)&mod_ups );

#ifdef LUA
	if(mod_Lua){ /* Is mod_Lua loaded ? */

			/* Expose shared methods */
		mod_Lua->initSectionSharedMethods(mod_Lua->L, "UPS");

			/* Expose mod_owm's own function */
		mod_Lua->exposeObjMethods(mod_Lua->L, "UPS", soM);
	}
#endif
}
