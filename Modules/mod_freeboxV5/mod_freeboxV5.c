/* mod_freebox
 *
 *  Publish Freebox v4/v5 figures (French Internet Service Provider)
 *
 * This file is part of Marcel project and is following the same
 * license rules (see LICENSE file)
 *
 * THIS MODULE IS DEPRECATING : Free is phasing out these old devices
 * and I haven't one anymore.
 *
 * 01/10/2022 - LF - First version
 * 20/09/2023 - LF - Rename to V5
 */

#include "mod_freeboxV5.h"
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

#define FBX_HOST	"mafreebox.freebox.fr"
#define FBX_URI "/pub/fbx_info.txt"
#define FBX_PORT	80

#define FBX_REQ "GET "FBX_URI" HTTP/1.0\n\n"

static struct module_freeboxV5 mod_freeboxV5;

enum {
	SFB_FREEBOXV5= 0
};

static int publishCustomFiguresFbV5(struct Section *asection){
#ifdef LUA
	if(mod_Lua){
		struct section_freeboxV5 *s = (struct section_freeboxV5 *)asection;

		lua_newtable(mod_Lua->L);

		lua_pushstring(mod_Lua->L, "Error state");			/* Push the index */
		lua_pushboolean(mod_Lua->L, s->section.inerror);	/* the value */
		lua_rawset(mod_Lua->L, -3);	/* Add it in the table */

		return 1;
	} else
#endif
	return 0;
}

static void *process_freeboxV5(void *actx){
	struct section_freeboxV5 *ctx = (struct section_freeboxV5 *)actx;
	char l[MAXLINE];
	struct hostent *server;
	struct sockaddr_in serv_addr;

	if(!ctx->section.topic){
		publishLog('E', "[FreeboxV5] configuration error : no topic specified, ignoring this section");
		SectionError((struct Section *)ctx, true);
		pthread_exit(0);
	}

	if(!(server = gethostbyname( FBX_HOST ))){
		publishLog('E', "[FreeboxV5] %s : %s", FBX_HOST, strerror( errno ));
		publishLog('F', "[FreeboxV5] Dying");
		SectionError((struct Section *)ctx, true);
		pthread_exit(0);
	}

	memset( &serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons( FBX_PORT );
	memcpy(&serv_addr.sin_addr.s_addr,*server->h_addr_list,server->h_length);

	if(cfg.verbose)
		publishLog('I', "Launching a processing flow for FreeboxV5");

	for(bool first=true;; first=false){
		if(ctx->section.disabled){
			SectionError((struct Section *)ctx, false);
#ifdef DEBUG
			if(cfg.debug)
				publishLog('d', "[%s] is disabled", ctx->section.uid);
#endif
		} else if( !first || ctx->section.immediate ){
			bool inerror = true;

			int sockfd = socket(AF_INET, SOCK_STREAM, 0);
			if(sockfd < 0){
				publishLog('E', "[%s] Can't create socket : %s", ctx->section.uid, strerror( errno ));
				if(!ctx->section.keep){
					publishLog('F', "[%s] Dying", ctx->section.uid);
					SectionError((struct Section *)ctx, true);
					pthread_exit(0);
				}
			} else {
				if(connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0)
					publishLog('E', "[%s] Connecting : %s", ctx->section.uid, strerror( errno ));
				else if( send(sockfd, FBX_REQ, strlen(FBX_REQ), 0) == -1 )
					publishLog('E', "[%s] Sending : %s", ctx->section.uid, strerror( errno ));
				else while( socketreadline(sockfd, l, sizeof(l)) != -1 ){
					inerror = false;

					if(strstr(l, "ATM")){
						int u, d, lm;
						if(sscanf(l+25,"%d", &d) != 1) d=-1;
						if(sscanf(l+44,"%d", &u) != 1) u=-1;

						lm = sprintf(l, "%s/DownloadATM", ctx->section.topic) + 2;
						assert( lm+1 < MAXLINE-10 );	/* Enough space for the response ? */
						sprintf( l+lm, "%d", d );
						mqttpublish( cfg.client, l, strlen(l+lm), l+lm, ctx->section.retained );
						publishLog('T', "FreeboxV5 : %s -> %s", l, l+lm);

						lm = sprintf(l, "%s/UploadATM", ctx->section.topic) + 2;
						assert( lm+1 < MAXLINE-10 );	/* Enough space for the response ? */
						sprintf( l+lm, "%d", u );
						mqttpublish( cfg.client, l, strlen(l+lm), l+lm, ctx->section.retained );
						publishLog('T', "FreeboxV5 : %s -> %s", l, l+lm);
					} else if(striKWcmp(l, "  Marge de bruit")){
						float u, d; 
						int lm;

						if(sscanf(l+25,"%f", &d) != 1) d = -1;
						if(sscanf(l+44,"%f", &u) != 1) u = -1;

						lm = sprintf(l, "%s/DownloadMarge", ctx->section.topic) + 2;
						assert( lm+1 < MAXLINE-10 );	/* Enough space for the response ? */
						sprintf( l+lm, "%.2f", d );
						mqttpublish( cfg.client, l, strlen(l+lm), l+lm, ctx->section.retained );
						publishLog('T', "FreeboxV5 : %s -> %s", l, l+lm);

						lm = sprintf(l, "%s/UploadMarge", ctx->section.topic) + 2;
						assert( lm+1 < MAXLINE-10 );	/* Enough space for the response ? */
						sprintf( l+lm, "%.2f", u );
						mqttpublish( cfg.client, l, strlen(l+lm), l+lm, ctx->section.retained );
						publishLog('T', "FreeboxV5 : %s -> %s", l, l+lm);
					} else if(striKWcmp(l, "  FEC")){
						unsigned long u, d;
						int lm;

						if(sscanf(l+25,"%lu", &d) != 1) d = -1;
						if(sscanf(l+44,"%lu", &u) != 1) u = -1;

						lm = sprintf(l, "%s/DownloadFEC", ctx->section.topic) + 2;
						assert( lm+1 < MAXLINE-10 );	/* Enough space for the response ? */
						sprintf( l+lm, "%lu", d );
						mqttpublish( cfg.client, l, strlen(l+lm), l+lm, ctx->section.retained );
						publishLog('T', "FreeboxV5 : %s -> %s", l, l+lm);

						lm = sprintf(l, "%s/UploadFEC", ctx->section.topic) + 2;
						assert( lm+1 < MAXLINE-10 );	/* Enough space for the response ? */
						sprintf( l+lm, "%lu", u );
						mqttpublish( cfg.client, l, strlen(l+lm), l+lm, ctx->section.retained );
						publishLog('T', "FreeboxV5 : %s -> %s", l, l+lm);
					} else if(striKWcmp(l, "  CRC")){
						unsigned long u, d;
						int lm;

						if(sscanf(l+25,"%lu", &d) != 1) d = -1;
						if(sscanf(l+44,"%lu", &u) != 1) u = -1;

						lm = sprintf(l, "%s/DownloadCRC", ctx->section.topic) + 2;
						assert( lm+1 < MAXLINE-10 );	/* Enough space for the response ? */
						sprintf( l+lm, "%lu", d );
						mqttpublish( cfg.client, l, strlen(l+lm), l+lm, ctx->section.retained );
						publishLog('T', "FreeboxV5 : %s -> %s", l, l+lm);

						lm = sprintf(l, "%s/UploadCRC", ctx->section.topic) + 2;
						assert( lm+1 < MAXLINE-10 );	/* Enough space for the response ? */
						sprintf( l+lm, "%lu", u );
						mqttpublish( cfg.client, l, strlen(l+lm), l+lm, ctx->section.retained );
						publishLog('T', "FreeboxV5 : %s -> %s", l, l+lm);
					} else if(striKWcmp(l, "  HEC")){
						unsigned long u, d;
						int lm;

						if(sscanf(l+25,"%lu", &d) != 1) d = -1;
						if(sscanf(l+44,"%lu", &u) != 1) u = -1;

						lm = sprintf(l, "%s/DownloadHEC", ctx->section.topic) + 2;
						assert( lm+1 < MAXLINE-10 );	/* Enough space for the response ? */
						sprintf( l+lm, "%lu", d );
						mqttpublish( cfg.client, l, strlen(l+lm), l+lm, ctx->section.retained );
						publishLog('T', "FreeboxV5 : %s -> %s", l, l+lm);

						lm = sprintf(l, "%s/UploadHEC", ctx->section.topic) + 2;
						assert( lm+1 < MAXLINE-10 );	/* Enough space for the response ? */
						sprintf( l+lm, "%lu", u );
						mqttpublish( cfg.client, l, strlen(l+lm), l+lm, ctx->section.retained );
						publishLog('T', "FreeboxV5 : %s -> %s", l, l+lm);
					} else if(striKWcmp(l, "  WAN")){
						int u, d, lm;

						if(sscanf(l+40,"%d", &d) != 1) d = -1;
						if(sscanf(l+55,"%d", &u) != 1) u = -1;

						lm = sprintf(l, "%s/DownloadWAN", ctx->section.topic) + 2;
						assert( lm+1 < MAXLINE-10 );	/* Enough space for the response ? */
						sprintf( l+lm, "%d", d );
						mqttpublish( cfg.client, l, strlen(l+lm), l+lm, ctx->section.retained );
						publishLog('T', "FreeboxV5 : %s -> %s", l, l+lm);

						lm = sprintf(l, "%s/UploadWAN", ctx->section.topic) + 2;
						assert( lm+1 < MAXLINE-10 );	/* Enough space for the response ? */
						sprintf( l+lm, "%d", u );
						mqttpublish( cfg.client, l, strlen(l+lm), l+lm, ctx->section.retained );
						publishLog('T', "FreeboxV5 : %s -> %s", l, l+lm);
					} else if(striKWcmp(l, "  Ethernet")){
						int u, d, lm;

						if(sscanf(l+40,"%d", &d) != 1) d = -1;
						if(sscanf(l+55,"%d", &u) != 1) u = -1;

						lm = sprintf(l, "%s/DownloadTV", ctx->section.topic) + 2;
						assert( lm+1 < MAXLINE-10 );	/* Enough space for the response ? */
						sprintf( l+lm, "%d", d );
						mqttpublish( cfg.client, l, strlen(l+lm), l+lm, ctx->section.retained );
						publishLog('T', "FreeboxV5 : %s -> %s", l, l+lm);

						lm = sprintf(l, "%s/UploadTV", ctx->section.topic) + 2;
						assert( lm+1 < MAXLINE-10 );	/* Enough space for the response ? */
						sprintf( l+lm, "%d", u );
						mqttpublish( cfg.client, l, strlen(l+lm), l+lm, ctx->section.retained );
						publishLog('T', "FreeboxV5 : %s -> %s", l, l+lm);
					} else if(striKWcmp(l, "  USB")){
						int u, d, lm;

						if(sscanf(l+40,"%d", &d) != 1) d = -1;
						if(sscanf(l+55,"%d", &u) != 1) u = -1;

						lm = sprintf(l, "%s/DownloadUSB", ctx->section.topic) + 2;
						assert( lm+1 < MAXLINE-10 );	/* Enough space for the response ? */
						sprintf( l+lm, "%d", d );
						mqttpublish( cfg.client, l, strlen(l+lm), l+lm, ctx->section.retained );
						publishLog('T', "FreeboxV5 : %s -> %s", l, l+lm);

						lm = sprintf(l, "%s/UploadUSB", ctx->section.topic) + 2;
						assert( lm+1 < MAXLINE-10 );	/* Enough space for the response ? */
						sprintf( l+lm, "%d", u );
						mqttpublish( cfg.client, l, strlen(l+lm), l+lm, ctx->section.retained );
						publishLog('T', "FreeboxV5 : %s -> %s", l, l+lm);
					} else if(striKWcmp(l, "  Switch")){
						int u, d, lm;

						if(sscanf(l+40,"%d", &d) != 1) d = -1;
						if(sscanf(l+55,"%d", &u) != 1) u = -1;

						lm = sprintf(l, "%s/DownloadLan", ctx->section.topic) + 2;
						assert( lm+1 < MAXLINE-10 );	/* Enough space for the response ? */
						sprintf( l+lm, "%d", d );
						mqttpublish( cfg.client, l, strlen(l+lm), l+lm, ctx->section.retained );
						publishLog('T', "FreeboxV5 : %s -> %s", l, l+lm);

						lm = sprintf(l, "%s/UploadLan", ctx->section.topic) + 2;
						assert( lm+1 < MAXLINE-10 );	/* Enough space for the response ? */
						sprintf( l+lm, "%d", u );
						mqttpublish( cfg.client, l, strlen(l+lm), l+lm, ctx->section.retained );
						publishLog('T', "FreeboxV5 : %s -> %s", l, l+lm);
					}
				}

				close(sockfd);
			}

			SectionError((struct Section *)ctx, inerror);
		}

		struct timespec ts;
		ts.tv_sec = (time_t)ctx->section.sample;
		ts.tv_nsec = (unsigned long int)((ctx->section.sample - (time_t)ctx->section.sample) * 1e9);

		nanosleep( &ts, NULL );
	}
}

static enum RC_readconf readconf(uint8_t mid, const char *l, struct Section **asection ){
	struct section_freeboxV5 **section = (struct section_freeboxV5 **)asection;
	const char *arg;

	if(!strcmp(l,"*FreeboxV5")){	/* Starting a section definition */
		arg = "FreeboxV5";

		if(findSectionByName(arg)){
			publishLog('F', "Section '%s' is already defined", arg);
			exit(EXIT_FAILURE);
		}

		struct section_freeboxV5 *nsection = malloc(sizeof(struct section_freeboxV5));
		initSection( (struct Section *)nsection, mid, SFB_FREEBOXV5, strdup(arg), "FreeboxV5");

		nsection->section.publishCustomFigures = publishCustomFiguresFbV5;

		if(cfg.verbose)	/* Be verbose if requested */
			publishLog('C', "\tEntering FreeboxV5 section '%s' (%04x)", nsection->section.uid, nsection->section.id);

		*section = nsection;	/* we're now in a section */
		return ACCEPTED;
	}

	return REJECTED;
}

static bool mfb_acceptSDirective( uint8_t sec_id, const char *directive ){
	if(sec_id == SFB_FREEBOXV5){
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

static ThreadedFunctionPtr mfb_getSlaveFunction(uint8_t sid){
	if(sid == SFB_FREEBOXV5)
		return process_freeboxV5;

	return NULL;
}

#ifdef LUA
static int s_inError(lua_State *L){
	struct section_freeboxV5 **s = luaL_testudata(L, 1, "FreeboxV5");
	luaL_argcheck(L, s != NULL, 1, "'FreeboxV5' expected");

	lua_pushboolean(L, (*s)->section.inerror);
	return 1;
}

static const struct luaL_Reg sM[] = {
	{"inError", s_inError},
	{NULL, NULL}
};
#endif


void InitModule( void ){
	initModule((struct Module *)&mod_freeboxV5, "mod_freeboxV5");

	mod_freeboxV5.module.readconf = readconf;
	mod_freeboxV5.module.acceptSDirective = mfb_acceptSDirective;
	mod_freeboxV5.module.getSlaveFunction = mfb_getSlaveFunction;

	registerModule( (struct Module *)&mod_freeboxV5 );

#ifdef LUA
	if(mod_Lua){ /* Is mod_Lua loaded ? */

			/* Expose shared methods */
		mod_Lua->initSectionSharedMethods(mod_Lua->L, "FreeboxV5");

			/* Expose module's own function */
		mod_Lua->exposeObjMethods(mod_Lua->L, "FreeboxV5", sM);
	}
#endif
}
