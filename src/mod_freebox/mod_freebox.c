/* mod_freebox
 *
 *  Publish Freebox v4/v5 figures (French Internet Service Provider)
 *
 * This file is part of Marcel project and is following the same
 * license rules (see LICENSE file)
 *
 * 01/10/2022 - LF - First version
 */

#include "mod_freebox.h"
#include "../MQTT_tools.h"

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#define FBX_HOST	"mafreebox.freebox.fr"
#define FBX_URI "/pub/fbx_info.txt"
#define FBX_PORT	80

#define FBX_REQ "GET "FBX_URI" HTTP/1.0\n\n"

static struct module_freebox mod_freebox;

enum {
	SFB_FREEBOX= 0
};

void *process_freebox(void *actx){
	struct section_freebox *ctx = (struct section_freebox *)actx;
	char l[MAXLINE];
	struct hostent *server;
	struct sockaddr_in serv_addr;

	if(!ctx->section.topic){
		publishLog('E', "[Freebox] configuration error : no topic specified, ignoring this section");
		pthread_exit(0);
	}

	if(!(server = gethostbyname( FBX_HOST ))){
		publishLog('E', "[Freebox] %s : %s", FBX_HOST, strerror( errno ));
		publishLog('F', "[Freebox] Dying");
		pthread_exit(0);
	}

	memset( &serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons( FBX_PORT );
	memcpy(&serv_addr.sin_addr.s_addr,*server->h_addr_list,server->h_length);

	if(cfg.verbose)
		publishLog('I', "Launching a processing flow for Freebox");

	for(bool first=true;; first=false){
		if(ctx->section.disabled){
#ifdef DEBUG
			if(cfg.debug)
				publishLog('d', "[%s] is disabled", ctx->section.uid);
#endif
		} else if( !first || ctx->section.immediate ){
			puts("Freebox !!");
		}

		struct timespec ts;
		ts.tv_sec = (time_t)ctx->section.sample;
		ts.tv_nsec = (unsigned long int)((ctx->section.sample - (time_t)ctx->section.sample) * 1e9);

		nanosleep( &ts, NULL );
	}
}

static enum RC_readconf readconf(uint8_t mid, const char *l, struct Section **asection ){
	struct section_freebox **section = (struct section_freebox **)asection;
	const char *arg;

	if(!strcmp(l,"*Freebox")){	/* Starting a section definition */
		arg = "Freebox";

		if(findSectionByName(arg)){
			publishLog('F', "Section '%s' is already defined", arg);
			exit(EXIT_FAILURE);
		}

		struct section_freebox *nsection = malloc(sizeof(struct section_freebox));
		initSection( (struct Section *)nsection, mid, SFB_FREEBOX, strdup(arg));

		if(cfg.verbose)	/* Be verbose if requested */
			publishLog('C', "\tEntering section '%s' (%04x)", nsection->section.uid, nsection->section.id);

		*section = nsection;	/* we're now in a section */
		return ACCEPTED;
	}

	return REJECTED;
}

static bool mfb_acceptSDirective( uint8_t sec_id, const char *directive ){
	if(sec_id == SFB_FREEBOX){
		if( !strcmp(directive, "Disabled") )
			return true;
		else if( !strcmp(directive, "Sample=") )
			return true;
		else if( !strcmp(directive, "Topic=") )
			return true;
		else if( !strcmp(directive, "Immediate") )
			return true;
		else if( !strcmp(directive, "Keep") )
			return true;
	}

	return false;
}

ThreadedFunctionPtr mfb_getSlaveFunction(uint8_t sid){
	if(sid == SFB_FREEBOX)
		return process_freebox;

	return NULL;
}

void InitModule( void ){
	mod_freebox.module.name = "mod_freebox";

	mod_freebox.module.readconf = readconf;
	mod_freebox.module.acceptSDirective = mfb_acceptSDirective;
	mod_freebox.module.getSlaveFunction = mfb_getSlaveFunction;
	mod_freebox.module.postconfInit = NULL;

	register_module( (struct Module *)&mod_freebox );
}
