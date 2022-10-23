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

void *process_UPS(void *actx){
	struct section_ups *ctx = (struct section_ups *)actx;
	char l[MAXLINE];
	struct hostent *server;
	struct sockaddr_in serv_addr;

		/* Sanity checks */
	if(!ctx->section.topic){
		publishLog('F', "[%s] Topic must be set. Dying ...", ctx->section.uid);
		pthread_exit(0);
	}

	if( !ctx->host || !ctx->port ){
		publishLog('E', "[%s] NUT server missing. Dying ...", ctx->section.uid);
		pthread_exit(0);
	}

	if( !ctx->section.sample ){
		publishLog('E', "[%s] No sample time. Dying ...", ctx->section.uid);
		pthread_exit(0);
	}

	if(!(server = gethostbyname( ctx->host ))){
		publishLog('E', "[%s] %s : Don't know how to reach NUT. Dying", ctx->section.uid, strerror(errno));
		pthread_exit(0);
	}

	memset( &serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons( ctx->port );
	memcpy(&serv_addr.sin_addr.s_addr,*server->h_addr_list,server->h_length);

	if(cfg.verbose)
		publishLog('I', "Launching a processing flow for UPS/%s", ctx->section.uid);

	for(;;){	/* Infinite loop to process data */
		if(ctx->section.disabled){
			publishLog('T', "Reading UPS/%s is disabled", ctx->section.uid);
		} else {
			int sockfd = socket(AF_INET, SOCK_STREAM, 0);
			if(sockfd < 0){
				publishLog('E', "[%s] Can't create socket : %s", ctx->section.uid, strerror( errno ));
				if(!ctx->section.keep){
					publishLog('F', "[%s] Dying", ctx->section.uid);
					pthread_exit(0);
				}
			} else {
				if(connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0){
					publishLog('E', "[%s] Connecting : %s", ctx->section.uid, strerror( errno ));
					if(!ctx->section.keep){
						publishLog('F', "[%s] Dying", ctx->section.uid);
						pthread_exit(0);
					}
				} else {
					for(struct var *v = ctx->var_list; v; v = v->next){
						sprintf(l, "GET VAR %s %s\n", ctx->section.uid, v->name);
						if( send(sockfd, l , strlen(l), 0) == -1 ){
							publishLog('E', "[%s] Sending : %s", ctx->section.uid, strerror( errno ));

							if(!ctx->section.keep){
								publishLog('F', "[%s] Dying", ctx->section.uid);
								pthread_exit(0);
							}
						} else {
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
		initSection( (struct Section *)nsection, mid, ST_UPS, strdup(arg));

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

void InitModule( void ){
	mod_ups.module.name = "mod_ups";

	mod_ups.module.readconf = readconf;
	mod_ups.module.acceptSDirective = mu_acceptSDirective;
	mod_ups.module.getSlaveFunction = mu_getSlaveFunction;
	mod_ups.module.postconfInit = NULL;

	register_module( (struct Module *)&mod_ups );
}
