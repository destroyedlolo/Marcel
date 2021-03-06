/*
 * Marcel
 *	A daemon to publish smart home data to MQTT broker and rise alert
 *	if needed.
 *
 * Additional options :
 *	-DFREEBOX : enable Freebox (v4 / v5) statistics
 *	-DUPS : enable UPS statistics (NUT needed)
 *	-DLUA : enable Lua user functions
 *	-DMETEO : enable meteo forcast publishing
 *	-DINOTIFY : add inotify support (needed by *LookForChanges)
 *
 *	Copyright 2015-2018 Laurent Faillie
 *
 *		Marcel is covered by
 *		Creative Commons Attribution-NonCommercial 3.0 License
 *      (http://creativecommons.org/licenses/by-nc/3.0/) 
 *      Consequently, you're free to use if for personal or non-profit usage,
 *      professional or commercial usage REQUIRES a commercial licence.
 *  
 *      Marcel is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 *	18/05/2015	- LF start of development (inspired from TeleInfod)
 *	20/05/2015	- LF - v1.0 - "file float value" working
 *	25/05/2015	- LF - v1.1 - Adding "Freebox"
 *	28/05/2015	- LF - v1.2 - Adding UPS
 *				-------
 *	08/07/2015	- LF - start v2.0 - make source modular
 *	21/07/2015	- LF - v2.1 - secure non-NULL MQTT payload
 *	26/07/2015	- LF - v2.2 - Add ConnectionLostIsFatal
 *	27/07/2015	- LF - v2.3 - Add ClientID to avoid connection loss during my tests
 *				-------
 *	07/07/2015	- LF - switch v3.0 - Add Lua user function in DPD
 *	09/08/2015	- LF - 3.1 - Add mutex to avoid parallel subscription which seems
 *					trashing broker connection
 *	09/08/2015	- LF - 3.2 - all subscriptions are done in the main thread as it seems 
 *					paho is not thread safe.
 *	07/09/2015	- LF - 3.3 - Adding Every tasks.
 *				-------
 *	06/10/2015	- LF - switch to v4.0 - curl can be used in several "section"
 *	29/10/2015	- LF - 4.1 - Add meteo forcast
 *	29/11/2015	- LF - 4.2 - Correct meteo forcast icon
 *	31/01/2016	- LF - 4.3 - Add AlertCommand
 *	04/02/2016	- LF - 4.4 - Alert can send only a mail
 *	24/02/2016	- LF - 4.5 - Add Notifications
 *	20/03/2016	- LF - 4.6 - Add named notifications
 *							- Can work without sections (Marcel acts as alerting relay)
 *	29/04/2016	- LF - 4.7 - Add RFXtrx support
 *	01/05/2016	- LF - 		DPD* replaced by Sub*
 *	14/05/2016	- LF - 4.10 - Add REST section
 *				-------
 */
#include "Marcel.h"
#include "Version.h"
#include "FFV.h"
#include "Freebox.h"
#include "UPS.h"
#include "DeadPublisherDetection.h"
#include "MQTT_tools.h"
#include "Alerting.h"
#include "Every.h"
#include "Meteo.h"
#include "RFXtrx_marcel.h"
#include "REST.h"
#include "OutFile.h"
#include "Sht31.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <errno.h>
#include <libgen.h>
#include <assert.h>
#include <unistd.h>
#include <signal.h>
#include <sys/utsname.h>	/* uname */
#include <curl/curl.h>
#include <stdbool.h>
#include <stdarg.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#ifdef LUA
#	include <lauxlib.h>
#endif

#ifdef INOTIFY
#	include <poll.h>	/* Only used for Look4Changes up to now */
#	include <sys/inotify.h>
#	define INOTIFY_BUF_LEN     (10 * (sizeof(struct inotify_event) + NAME_MAX + 1))
#endif

bool verbose = false;
bool configtest = false;
struct Config cfg;

	/* Logging */
void publishLog( char l, const char *msg, ...){
	va_list args;
	va_start(args, msg);

	if(verbose || l=='E' || l=='F'){
		char t[ strlen(msg) + 7 ];
		sprintf(t, "*%c* %s\n", l, msg);
		vfprintf((l=='E' || l=='F')? stderr : stdout, t, args);
	}

	if(cfg.client){
		char *sub;
		switch(l){
		case 'F':
			sub = "/Log/Fatal";
			break;
		case 'E':
			sub = "/Log/Error";
			break;
		case 'W':
			sub = "/Log/Warning";
			break;
		case 'I':
			sub = "/Log/Information";
			break;
		default :	/* Trace */
			sub = "/Log";
		}

		char tmsg[1024];	/* No simple way here to know the message size */
		char ttopic[ strlen(cfg.ClientID) + strlen(sub) + 1 ];
		sprintf(ttopic, "%s%s", cfg.ClientID, sub);
		vsnprintf(tmsg, sizeof(tmsg), msg, args);

		mqttpublish( cfg.client, ttopic, strlen(tmsg), tmsg, 0);
	}
	va_end(args);
}

	/*
	 * Helpers
	 */
char *removeLF(char *s){
	size_t l=strlen(s);
	if(l && s[--l] == '\n')
		s[l] = 0;
	return s;
}

char *striKWcmp( char *s, const char *kw ){
/* compare string s against kw
 * Return :
 * 	- remaining string if the keyword matches
 * 	- NULL if the keyword is not found
 */
	size_t klen = strlen(kw);
	if( strncasecmp(s,kw,klen) )
		return NULL;
	else
		return s+klen;
}

char *mystrdup(const char *as){
	/* as strdup() is missing within C99, grrr ! */
	char *s;
	assert(as);
	assert(s = malloc(strlen(as)+1));
	strcpy(s, as);
	return s;
}

char *stradd(char *p, const char *s, bool addspace){
	/* Enlarge string pointed by p to add s
	 * if !p, start a new string
	 *
	 * if addspace == true, the 1st char is
	 * skipped if !p
	 */
	if(!p)
		return(mystrdup(s + (addspace ? 1:0)));

	size_t asz = strlen(p);
	char *np = realloc(p, strlen(p) + strlen(s) +1);
	assert(np);
	strcpy( np+asz, s );

	return(np);
}

void init_VarSubstitution( struct _VarSubstitution *tbl ){
	while(tbl->var){
		tbl->lvar = strlen(tbl->var);
		tbl->lval = strlen(tbl->val);
		tbl++;
	}
}

char *replaceVar( const char *arg, struct _VarSubstitution *lookup ){
	size_t idx, idxd, 				/* source and destination indexes */
		sz, max=strlen(arg);		/* size of allocated area and max index */

	char *s = malloc( sz=max+1 );	/* resulting string */
	assert(s);

	for(idx = idxd = 0; idx<max; idx++){
		if(arg[idx] == '%'){
			bool found=false;
			struct _VarSubstitution *t;

			for(t=lookup; t->var; t++){
				if(!strncmp(arg+idx, t->var, t->lvar)){
					sz += t->lval - t->lvar;
					assert(s = realloc(s, sz));
					strcpy(s+idxd, t->val);		/* Insert the value */
					idxd += t->lval;			/* Skip variable's content */
					idx += t->lvar-1;				/* Skip variable's name */

					found=true;
					break;
				}
			}

			if(!found)
				s[idxd++] = arg[idx];
		} else
			s[idxd++] = arg[idx];
	}

	s[idxd] = 0;
	return s;
}

size_t socketreadline( int fd, char *l, size_t sz){
/* read a line :
 * -> 	fd : file descriptor to read
 *		l : buffer to store the result
 *		sz : max size of the result
 * <- size read, -1 if error or EoF
 */
	int s=0;
	char *p = l, c;

	for( ; p-l< sz; ){
		int r = read( fd, &c, 1);

		if(r == -1)
			return -1;
		else if(!r){	/* EOF */
			return -1;
		} else if(c == '\n')
			break;
		else {
			*p++ = c;
			s++;
		}
	}
	*p = '\0';

	return s;
}

	/*
	 * Configuration
	 */
static int chksum(const char *s){
	int h = 0;
	while(*s)
		h += *s++;
	return h;
}

static void setUID( union CSection *sec, const char *uid ){
	if(!*uid){
		publishLog('F', "Empty uid provided.");
		exit(EXIT_FAILURE);
	}

	int h = chksum(uid);

	for(union CSection *s = cfg.sections; s; s = s->common.next){
		if( s->common.h == h && !strcmp(s->common.uid, uid) ){
			publishLog('F', "'%s' uid is used more than once.", uid);
			exit(EXIT_FAILURE);
		}
	}

	assert( sec->common.uid = strdup( uid ) );
	sec->common.h = h;
}

	/* Initialize an empty CSection */
static union CSection *createCSection( size_t s, char *id ){
	union CSection *n = malloc( s );
	assert(n);
	memset(n, 0, s);

	setUID( n, removeLF(id) );

	n->common.failfuncid = LUA_REFNIL;

	return(n);
}

static void read_configuration( const char *fch){
	FILE *f;
	char l[MAXLINE];
	char *arg;
	union CSection *last_section=NULL;

		/* Default config value */
	cfg.sections = NULL;
	cfg.Broker = "tcp://localhost:1883";
	cfg.ClientID = "Marcel";
	cfg.client = NULL;
	cfg.ConLostFatal = false;

	cfg.Randomize = false;

	cfg.Sublast = false;
	cfg.first_Sub = NULL;

	cfg.L4Cgrouped = false;
	cfg.first_L4C = NULL;

	cfg.luascript = NULL;

	cfg.RFXdevice = NULL;

	cfg.SMSurl = NULL;
	cfg.AlertCmd= NULL;
	cfg.notiflist=NULL;

	cfg.OwAlarm=NULL;
	cfg.OwAlarmSample=0;
	cfg.OwAlarmKeep=false;

	if(verbose)
		printf("\nReading configuration file '%s'\n---------------------------\n", fch);

	if(!(f=fopen(fch, "r"))){
		publishLog('F', "%s : %s", fch, strerror( errno ));
		exit(EXIT_FAILURE);
	}

		/* Build ClientID lookup */
	struct _VarSubstitution vslookup[] = {
		{ "%ClientID%", cfg.ClientID },	/* MUST BE THE 1ST VARIABLE */
		{ NULL }
	};
	init_VarSubstitution( vslookup );

	while(fgets(l, MAXLINE, f)){
		if(*l == '#' || *l == '\n')
			continue;

		if((arg = striKWcmp(l,"ClientID="))){
			assert( cfg.ClientID = strdup( removeLF(arg) ) );
			vslookup[0].val = cfg.ClientID;
			vslookup[0].lval = strlen(cfg.ClientID);
			if(verbose)
				printf("MQTT Client ID : '%s'\n", cfg.ClientID);
		} else if((arg = striKWcmp(l,"Broker="))){
			assert( cfg.Broker = strdup( removeLF(arg) ) );
			if(verbose)
				printf("Broker : '%s'\n", cfg.Broker);
		} else if((arg = striKWcmp(l,"MinVersion="))){
			float v = atof(arg);
			if( v > (float)atof(MARCEL_VERSION)){
				publishLog('F', "Expected Marcel version : %.04f, got %.04f", v, atof(MARCEL_VERSION));
				exit(EXIT_FAILURE);
			} else if(verbose)
				printf("Minimal Marcel version : %.04f\n", v);
		} else if((arg = striKWcmp(l,"SMSUrl=")) || (arg = striKWcmp(l,"RESTUrl="))){
			if(cfg.notiflist){
				assert( cfg.notiflist->url = strdup( removeLF(arg) ) );
				if(verbose)
					printf("\tREST Url : '%s'\n", cfg.notiflist->url);
			} else {
				assert( cfg.SMSurl = strdup( removeLF(arg) ) );
				if(verbose)
					printf("REST Url : '%s'\n", cfg.SMSurl);
			}
		} else if((arg = striKWcmp(l,"AlertCommand="))){
			if(cfg.notiflist){
				assert( cfg.notiflist->cmd = strdup( removeLF(arg) ) );
				if(verbose)
					printf("\tAlert Command : '%s'\n", cfg.notiflist->cmd);
			} else {
				assert( cfg.AlertCmd = strdup( removeLF(arg) ) );
				if(verbose)
					printf("Alert Command : '%s'\n", cfg.AlertCmd);
			}
		} else if((arg = striKWcmp(l,"RFXtrx_Port="))){
#ifdef RFXTRX
			assert( cfg.RFXdevice = strdup( removeLF(arg) ) );
			if(verbose)
				printf("RFXtrx's device : '%s'\n", cfg.RFXdevice);
#else
			publishLog('E', "RFXtrx_Port defined without RFXtrx support enabled");
#endif
		} else if((arg = striKWcmp(l,"1wire-Alarm="))){
			assert( cfg.OwAlarm = strdup( removeLF(arg) ) );
			if(verbose)
				printf("1 wire Alarm directory : '%s'\n", cfg.OwAlarm);
		} else if((arg = striKWcmp(l,"1wire-Alarm-sample="))){
			cfg.OwAlarmSample = atoi( arg );
			if(verbose)
				printf("1 wire Alarm sample delay : '%d'\n", cfg.OwAlarmSample);
		} else if(!strcmp(l,"1wire-Alarm-keep\n")){	/* Crash if the broker connection is lost */
			cfg.OwAlarmKeep = true;
			if(verbose)
				puts("Keep 1-wire alarm detection");
		} else if((arg = striKWcmp(l,"UserFuncScript="))){
#ifdef LUA
			assert( cfg.luascript = strdup( removeLF(arg) ) );
			if(verbose)
				printf("User functions definition script : '%s'\n", cfg.luascript);
#else
			publishLog('E', "UserFuncScript defined without Lua support enabled");
#endif
		} else if((arg = striKWcmp(l,"$alert=")) || (arg = striKWcmp(l,"$notification="))){
			struct notification *n = malloc( sizeof(struct notification) );
			assert(n);
			memset(n, 0, sizeof(struct notification));

			if(!*arg || *arg == '\n'){
				publishLog('F', "Unamed $notification section, giving up !");
				exit(EXIT_FAILURE);
			}

			n->id = *arg;
			n->next = cfg.notiflist;
			cfg.notiflist = n;

			if(verbose)
				printf("Entering notification definition '%c'\n", n->id);
		} else if((arg = striKWcmp(l,"*FFV="))){
			union CSection *n = createCSection( sizeof(struct _FFV), arg );
			n->common.section_type = MSEC_FFV;
			n->FFV.funcid = LUA_REFNIL;
			n->FFV.safe85 = false;

			if(last_section)
				last_section->common.next = n;
			else	/* First section */
				cfg.sections = n;
			last_section = n;
			if(verbose)
				printf("Entering FFV section '%s'\n", n->common.uid);
		} else if((arg = striKWcmp(l,"*OutFile="))){
			union CSection *n = createCSection( sizeof(struct _OutFile), arg );
			n->common.section_type = MSEC_OUTFILE;
			n->OutFile.funcid = LUA_REFNIL;

			if(last_section)
				last_section->common.next = n;
			else	/* First section */
				cfg.sections = n;
			last_section = n;
			if(!cfg.first_Sub)
				cfg.first_Sub = n;
	
			if(verbose)
				printf("Entering OutFile section '%s'\n", n->common.uid);
		} else if((arg = striKWcmp(l,"*Freebox"))){
			union CSection *n = createCSection( sizeof(struct _FreeBox), "Freebox" );
			n->common.section_type = MSEC_FREEBOX;

			if(last_section)
				last_section->common.next = n;
			else	/* First section */
				cfg.sections = n;
			last_section = n;
			if(verbose)
				puts("Entering section 'Freebox'");
		} else if((arg = striKWcmp(l,"*UPS="))){
			union CSection *n = createCSection( sizeof(struct _UPS), arg );
			n->common.section_type = MSEC_UPS;

			if(last_section)
				last_section->common.next = n;
			else	/* First section */
				cfg.sections = n;
			last_section = n;
			if(verbose)
				printf("Entering section 'UPS/%s'\n", n->Ups.uid);
		} else if((arg = striKWcmp(l,"*Every="))){
			union CSection *n = createCSection( sizeof(struct _Every), arg );
			n->common.section_type = MSEC_EVERY;
			n->Every.at = -1;

#ifndef LUA
			publishLog('F', "Every section is only available when compiled with Lua support");
			exit(EXIT_FAILURE);
#else
			n->Every.funcid = LUA_REFNIL;
#endif

			if(last_section)
				last_section->common.next = n;
			else	/* First section */
				cfg.sections = n;
			last_section = n;
			if(verbose)
				printf("Entering section 'Every/%s'\n", n->common.uid );
		} else if((arg = striKWcmp(l,"*LookForChanges="))){
#ifndef INOTIFY
			publishLog('F', "LookForChanges section is only available when compiled with inotify support");
			exit(EXIT_FAILURE);
#else
			union CSection *n = createCSection( sizeof(struct _Look4Changes), arg );
			n->common.section_type = MSEC_LOOK4CHANGES;

			n->Look4Changes.funcid = LUA_REFNIL;

			if(last_section)
				last_section->common.next = n;
			else	/* First section */
				cfg.sections = n;
			last_section = n;

			if(!cfg.first_L4C)
				cfg.first_L4C = (struct _Look4Changes *)n;

			if(verbose)
				printf("Entering section 'LookForChanges/%s'\n", n->common.uid );
#endif
		} else if((arg = striKWcmp(l,"*Meteo3H="))){
			union CSection *n = createCSection( sizeof(struct _Meteo), arg );
			n->common.section_type = MSEC_METEO3H;
#ifndef METEO
			publishLog('F', "Meteo3H section is only available when compiled with METEO support");
			exit(EXIT_FAILURE);
#endif

			if(last_section)
				last_section->common.next = n;
			else	/* First section */
				cfg.sections = n;
			last_section = n;
			if(verbose)
				printf("Entering section 'Meteo 3H/%s'\n", n->common.uid);
		} else if((arg = striKWcmp(l,"*MeteoDaily="))){
			union CSection *n = createCSection( sizeof(struct _Meteo), arg );
			n->common.section_type = MSEC_METEOD;

#ifndef METEO
			publishLog('F', "MeteoDaily section is only available when compiled with METEO support");
			exit(EXIT_FAILURE);
#endif

			if(last_section)
				last_section->common.next = n;
			else	/* First section */
				cfg.sections = n;
			last_section = n;
			if(verbose)
				printf("Entering section 'Meteo Daily/%s'\n",n->common.uid );
		} else if((arg = striKWcmp(l,"*DPD="))){
			union CSection *n = createCSection( sizeof(struct _DeadPublisher), arg );
			n->common.section_type = MSEC_DEADPUBLISHER;

			n->DeadPublisher.funcid = LUA_REFNIL;

			if(last_section)
				last_section->common.next = n;
			else	/* First section */
				cfg.sections = n;
			last_section = n;
			if(!cfg.first_Sub)
				cfg.first_Sub = n;

			if(verbose)
				printf("Entering section 'DeadPublisher/%s'\n", n->common.uid);
		} else if((arg = striKWcmp(l,"*RTSCmd="))){
			union CSection *n = createCSection( sizeof(struct _RTSCmd), arg );
			n->common.section_type = MSEC_RTSCMD;

			if(last_section)
				last_section->common.next = n;
			else	/* First section */
				cfg.sections = n;
			last_section = n;
			if(!cfg.first_Sub)
				cfg.first_Sub = n;

			if(verbose)
				printf("Entering section 'RTS Command' for '%s'\n", n->RTSCmd.uid);
		} else if((arg = striKWcmp(l,"*REST="))){
			union CSection *n = createCSection( sizeof(struct _REST), arg );
			n->common.section_type = MSEC_REST;

			n->REST.at = -1;
//			assert( n->REST.url = strdup( removeLF(arg) ));

#ifndef LUA
			publishLog('F', "REST section is only available when compiled with Lua support");
			exit(EXIT_FAILURE);
#else
			n->REST.funcid = LUA_REFNIL;
#endif

			if(last_section)
				last_section->common.next = n;
			else	/* First section */
				cfg.sections = n;
			last_section = n;
			if(verbose)
				printf("Entering section REST '%s'\n", n->REST.uid );
		} else if((arg = striKWcmp(l,"*SHT31="))){
			union CSection *n = createCSection( sizeof(struct _Sht31), arg );
			n->common.section_type = MSRC_SHT31;
			n->Sht.i2c_addr = 0x44;

			if(last_section)
				last_section->common.next = n;
			else	/* First section */
				cfg.sections = n;
			last_section = n;
			if(verbose)
				printf("Entering SHT31 section '%s'\n", n->common.uid);
		} else if(!strcmp(l,"Randomize\n")){	/* Randomize FFV starting */
			cfg.Randomize = true;
			if(verbose)
				puts("Randomize FFV starting");
		} else if(!strcmp(l,"DPDLast\n") || !strcmp(l,"SubLast\n")){	/* Subscriptions are grouped at the end of the configuration file */
			cfg.Sublast = true;
			if(verbose)
				puts("Subscriptions (DPD, RTSCmd) sections are grouped at the end of the configuration");
		} else if(!strcmp(l,"LookForChangesGrouped\n")){	/* LookForChanges are grouped */
			cfg.L4Cgrouped = true;
			if(verbose)
				puts("LookForChanges are grouped");
		} else if(!strcmp(l,"ConnectionLostIsFatal\n")){	/* Crash if the broker connection is lost */
			cfg.ConLostFatal = true;
			if(verbose)
				puts("Crash if the broker connection is lost");
		} else if((arg = striKWcmp(l,"File="))){
			if(!last_section){
				publishLog('F', "Configuration issue : File directive outside section");
				exit(EXIT_FAILURE);
			}
			switch( last_section->common.section_type ){
			case MSEC_FFV :
				assert( last_section->FFV.file = strdup( removeLF(arg) ));
				if(verbose)
					printf("\tFile : '%s'\n", last_section->FFV.file);
				break;
			case MSEC_OUTFILE :
				assert( last_section->OutFile.file = strdup( removeLF(arg) ));
				if(verbose)
					printf("\tFile : '%s'\n", last_section->OutFile.file);
				break;
			default :
				publishLog('F', "[%s] Configuration issue : File directive outside FFV or OutFile section", last_section->common.uid);
					exit(EXIT_FAILURE);
			}
		} else if((arg = striKWcmp(l,"Latch="))){
			if(!last_section || last_section->common.section_type != MSEC_FFV){
				publishLog('F', "Configuration issue : Latch directive outside a FFV section");
					exit(EXIT_FAILURE);
			}
			assert( last_section->FFV.latch = strdup( removeLF(arg) ));
			if(verbose)
				printf("\tLatch file : '%s'\n", last_section->FFV.latch);
		} else if(!strcmp(l,"safe85\n")){		/* This section is currently disabled */
			if(!last_section || last_section->common.section_type != MSEC_FFV){
				publishLog('F', "Configuration issue : safe85 directive outside a FFV section");
					exit(EXIT_FAILURE);
			}
			last_section->FFV.safe85 = true;
			if(verbose)
				puts("\tsafe85");
		} else if((arg = striKWcmp(l,"Offset="))){
			float offset;
			switch(last_section ? last_section->common.section_type : MSEC_INVALID){
			case MSEC_FFV:
				offset = last_section->FFV.offset = atof(arg);
				break;
			case MSRC_SHT31:
				offset = last_section->Sht.offset = atof(arg);
				break;
			default:
				publishLog('F', "Configuration issue : Offset directive outside a FFV or SHT31 section");
				exit(EXIT_FAILURE);
			}

			if(verbose){
				printf("\tOffset : %f\n", offset);
				if(!offset)
					puts("*W*\tIs it normal it's a NULL offset ?");
			}
		} else if((arg = striKWcmp(l,"Device="))){
			if(!last_section || last_section->common.section_type != MSRC_SHT31){
				publishLog('F', "Configuration issue : Device directive outside a SHT31 section");
				exit(EXIT_FAILURE);
			}
			assert( last_section->Sht.device = strdup( removeLF(arg) ));
			if(verbose)
				printf("\tDevice : '%s'\n", last_section->Sht.device);
		} else if((arg = striKWcmp(l,"Address="))){
			char *dummy;
			if(!last_section || last_section->common.section_type != MSRC_SHT31){
				publishLog('F', "Configuration issue : Address directive outside a SHT31 section");
				exit(EXIT_FAILURE);
			}
			last_section->Sht.i2c_addr = strtol(arg, &dummy, 16);
			if(verbose)
				printf("\tAddress : '0x%02x'\n", last_section->Sht.i2c_addr);
		} else if((arg = striKWcmp(l,"Host="))){
			if(!last_section || last_section->common.section_type != MSEC_UPS){
				publishLog('F', "Configuration issue : Host directive outside a UPS section");
				exit(EXIT_FAILURE);
			}
			assert( last_section->Ups.host = strdup( removeLF(arg) ));
			if(verbose)
				printf("\tHost : '%s'\n", last_section->Ups.host);
		} else if((arg = striKWcmp(l,"Port="))){
			if(!last_section || last_section->common.section_type != MSEC_UPS){
				publishLog('F', "Configuration issue : Port directive outside a UPS section");
				exit(EXIT_FAILURE);
			}
			if(!(last_section->Ups.port = atoi(arg))){
				publishLog('F', "Configuration issue : Port is null (or is not a number)");
				exit(EXIT_FAILURE);
			}
			if(verbose)
				printf("\tPort : %d\n", last_section->Ups.port);
		} else if((arg = striKWcmp(l,"Url="))){
			if(!last_section || last_section->common.section_type != MSEC_REST){
				publishLog('F', "Configuration issue : Url directive outside a REST section");
				exit(EXIT_FAILURE);
			}

			assert( last_section->REST.url = strdup( removeLF(arg) ));
			if(verbose)
				printf("\tUrl : %s\n", last_section->REST.url);
		} else if((arg = striKWcmp(l,"ID="))){
			if(!last_section || last_section->common.section_type != MSEC_RTSCMD){
				publishLog('F', "Configuration issue : ID directive outside a RTSCmd section");
				exit(EXIT_FAILURE);
			}
			last_section->RTSCmd.did = strtol(arg, NULL, 0);
			if(verbose)
				printf("\tID : %04x\n", last_section->RTSCmd.did);
		} else if((arg = striKWcmp(l,"Var="))){
			if(!last_section || last_section->common.section_type != MSEC_UPS){
				publishLog('F', "Configuration issue : Var directive outside a UPS section");
				exit(EXIT_FAILURE);
			}
			struct var *v = malloc(sizeof(struct var));
			assert(v);
			assert( v->name = strdup( removeLF(arg) ));
			v->next = last_section->Ups.var_list;
			last_section->Ups.var_list = v;
			if(verbose)
				printf("\tVar : '%s'\n", v->name);
#ifdef INOTIFY
		} else if((arg = striKWcmp(l,"On="))){
			if(!last_section || last_section->common.section_type != MSEC_LOOK4CHANGES){
				publishLog('F', "Configuration issue : 'On=' directive outside a LookForChanges section");
				exit(EXIT_FAILURE);
			}

			assert( last_section->Look4Changes.dir = strdup( removeLF(arg) ));
			if(verbose)
				printf("\tOn : '%s'\n", last_section->Look4Changes.dir);

		} else if((arg = striKWcmp(l,"For="))){
			if(!last_section || last_section->common.section_type != MSEC_LOOK4CHANGES){
				publishLog('F', "Configuration issue : 'For=' directive outside a LookForChanges section");
				exit(EXIT_FAILURE);
			}

			if(verbose)
				printf("\tFor :");

			for( char *t = strtok( removeLF(arg), " \t" ); t; t = strtok( NULL, " \t" ) ){
				if(!strcasecmp( t, "create" )){
					last_section->Look4Changes.flags |=  IN_CREATE | IN_MOVED_TO;
					if(verbose)
						printf(" create");
				} else if(!strcasecmp( t, "remove" )){
					last_section->Look4Changes.flags |=  IN_DELETE | IN_MOVED_FROM;
					if(verbose)
						printf(" remove");
				} else if(!strcasecmp( t, "modify" )){
					last_section->Look4Changes.flags |=  IN_ATTRIB | IN_CLOSE_WRITE;
					if(verbose)
						printf(" modify");
				}
			}
			if(verbose)
				puts("");
#endif
		} else if((arg = striKWcmp(l,"FailFunc="))){
			if(!last_section || (
				last_section->common.section_type != MSEC_FFV
			)){
				publishLog('F', "Configuration issue : FailFunc directive outside an FFV section");
				exit(EXIT_FAILURE);
			}
#ifndef LUA
			publishLog('F', "Fail functions can only be used when compiled with Lua support");
			exit(EXIT_FAILURE);
#endif
			assert( last_section->common.failfunc = strdup( removeLF(arg) ));
			if(verbose)
				printf("\tFail Function : '%s'\n", last_section->common.failfunc);
		} else if((arg = striKWcmp(l,"Func="))){
			if(!last_section || (
				last_section->common.section_type != MSEC_FFV &&
				last_section->common.section_type != MSEC_DEADPUBLISHER &&
				last_section->common.section_type != MSEC_EVERY &&
				last_section->common.section_type != MSEC_REST &&
				last_section->common.section_type != MSEC_OUTFILE
			)){
				publishLog('F', "Configuration issue : Func directive outside a DPD, Every, REST or OutFile section");
				exit(EXIT_FAILURE);
			}
#ifndef LUA
			publishLog('F', "User functions can only be used when compiled with Lua support");
			exit(EXIT_FAILURE);
#endif
			assert( last_section->DeadPublisher.funcname = strdup( removeLF(arg) ));
			if(verbose)
				printf("\tFunction : '%s'\n", last_section->DeadPublisher.funcname);
		} else if(!strcmp(l,"RunIfOver\n")){	/* Crash if the broker connection is lost */
			if(!last_section || (
				last_section->common.section_type != MSEC_REST &&
				last_section->common.section_type != MSEC_EVERY
			)){
				publishLog('F', "Configuration issue : RunIfOver directive outside a REST or Every section");
				exit(EXIT_FAILURE);
			}
			last_section->REST.runifover = true;
			if(verbose)
				printf("\tRunIfOver\n");
		} else if(!strcmp(l,"Immediate\n")){	/* Crash if the broker connection is lost */
			if(!last_section || (
				last_section->common.section_type != MSEC_EVERY &&
				last_section->common.section_type != MSEC_REST
			)){
				publishLog('F', "Configuration issue : Immediate directive outside a REST or Every section");
				exit(EXIT_FAILURE);
			}
			last_section->REST.immediate = true;
			if(verbose)
				printf("\tImmediate\n");
		} else if((arg = striKWcmp(l,"At="))){
			if(!last_section || (
				last_section->common.section_type != MSEC_REST &&
				last_section->common.section_type != MSEC_EVERY
			)){
				publishLog('F', "Configuration issue : At directive outside a REST or Every section");
				exit(EXIT_FAILURE);
			}

			last_section->REST.at = atoi( arg );
			if(verbose)
				printf("\tAt : '%d'\n", last_section->REST.at);
		} else if((arg = striKWcmp(l,"Sample="))){
			if(!last_section){
				publishLog('F', "Configuration issue : Sample directive outside a section");
				exit(EXIT_FAILURE);
			}
			if( last_section->common.section_type == MSEC_RTSCMD ||
				last_section->common.section_type == MSEC_OUTFILE){
				publishLog('F', "Configuration issue : Sample directive isn't compatible with RTSCmd or Outfile sections");
				exit(EXIT_FAILURE);
			}
			last_section->common.sample = atoi( arg );
			if(verbose)
				printf("\tDelay between samples : %ds\n", last_section->common.sample);
		} else if(!strcmp(l,"Disabled\n")){		/* This section is currently disabled */
			if(!last_section){
				publishLog('F', "Configuration issue : Disabled directive outside a section");
				exit(EXIT_FAILURE);
			}
			last_section->common.disabled = true;
			if(verbose)
				puts("\tDisabled");
		} else if(!strcmp(l,"Keep\n")){		/* Staying alive */
			if(!last_section){
				publishLog('F', "Configuration issue : Keep directive outside a section");
				exit(EXIT_FAILURE);
			}
			last_section->common.keep = true;
			if(verbose)
				puts("\tKeep");
		} else if(!strcmp(l,"Retained\n")){		/* Staying alive */
			if(!last_section){
				publishLog('F', "Configuration issue : Retained directive outside a section");
				exit(EXIT_FAILURE);
			}
			last_section->common.retained = true;
			if(verbose)
				puts("\tRetained");
		} else if((arg = striKWcmp(l,"Topic="))){
			if(!last_section){
				publishLog('F', "Configuration issue : Topic directive outside a section");
				exit(EXIT_FAILURE);
			}
			last_section->common.topic = removeLF(replaceVar(arg, vslookup));
			if(verbose)
				printf("\tTopic : '%s'\n", last_section->common.topic);
		} else if((arg = striKWcmp(l,"ErrorTopic="))){
			if(!last_section ||
				last_section->common.section_type != MSEC_DEADPUBLISHER
			){
				publishLog('F', "Configuration issue : ErrorTopic directive outside DPD section");
				exit(EXIT_FAILURE);
			}
			assert( last_section->DeadPublisher.errtopic = strdup( removeLF(arg) ));
			if(verbose)
				printf("\tError Topic : '%s'\n", last_section->DeadPublisher.errtopic);
		} else if((arg = striKWcmp(l,"City="))){
			if(!last_section || (
				last_section->common.section_type != MSEC_METEO3H &&
				last_section->common.section_type != MSEC_METEOD
			)){
				publishLog('F', "Configuration issue : City directive outside a Meteo* section");
				exit(EXIT_FAILURE);
			}
			assert( last_section->Meteo.City = strdup( removeLF(arg) ));
			if(verbose)
				printf("\tCity : '%s'\n", last_section->Meteo.City);
		} else if((arg = striKWcmp(l,"Units="))){
			if(!last_section || (
				last_section->common.section_type != MSEC_METEO3H &&
				last_section->common.section_type != MSEC_METEOD
			)){
				publishLog('F', "Configuration issue : Units directive outside a Meteo* section");
				exit(EXIT_FAILURE);
			}
			assert( last_section->Meteo.Units = strdup( removeLF(arg) ));
			if( strcmp( last_section->Meteo.Units, "metric" ) &&
				strcmp( last_section->Meteo.Units, "imperial" ) &&
				strcmp( last_section->Meteo.Units, "Standard" ) ){
				publishLog('F', "Configuration issue : Units can only be only \"metric\" or \"imperial\" or \"Standard\"");
				exit(EXIT_FAILURE);
			}
			if(verbose)
				printf("\tUnits : '%s'\n", last_section->Meteo.Units);
		} else if((arg = striKWcmp(l,"Lang="))){
			if(!last_section || (
				last_section->common.section_type != MSEC_METEO3H &&
				last_section->common.section_type != MSEC_METEOD
			)){
				publishLog('F', "Configuration issue : Lang directive outside a Meteo* section");
				exit(EXIT_FAILURE);
			}
			assert( last_section->Meteo.Lang = strdup( removeLF(arg) ));
			if(verbose)
				printf("\tLang : '%s'\n", last_section->Meteo.Lang);
		}
	}

	fclose(f);

	assert( cfg.OnOffTopic = malloc( strlen(cfg.ClientID) + 9) );	/* "/OnOff/#" + \0 */
	sprintf( cfg.OnOffTopic, "%s/OnOff/", cfg.ClientID );
	if(verbose)
		printf("OnOff topic : %s\n", cfg.OnOffTopic);
}

	/*
	 * Broker related functions
	 */
static int msgarrived(void *actx, char *topic, int tlen, MQTTClient_message *msg){
	union CSection *Sec = cfg.Sublast ? cfg.first_Sub : cfg.sections;
	const char *aid;
	char payload[msg->payloadlen + 1];

	memcpy(payload, msg->payload, msg->payloadlen);
	payload[msg->payloadlen] = 0;

	publishLog('T', "message arrival (topic : '%s', msg : '%s')", topic, payload);

	if((aid = striKWcmp(topic,"Alert/")))
		rcv_alert( aid, payload );
	else if((aid = striKWcmp(topic,"Notification/")))
		rcv_notification( aid, payload );
	else if((aid = striKWcmp(topic,"nNotification/")))
		rcv_nnotification( aid, payload );
	else if((aid = striKWcmp(topic, cfg.OnOffTopic))){
		int h = chksum( aid );
		for(Sec = cfg.sections; Sec; Sec = Sec->common.next){
			if( Sec->common.h == h && !strcmp( Sec->common.uid, aid ) )	/* Target found */
				break;
		}
		if( Sec ){
			bool disable = false;
			if( !strcmp(payload,"0") || !strcasecmp(payload,"off") || !strcasecmp(payload,"disable") )
				disable = true;
			Sec->common.disabled = disable;
			publishLog('T', "%s '%s'", disable ? "Disabling":"Enabling", aid);
		} else
			publishLog('T', "No section matching '%s'", aid);
	} else for(; Sec; Sec = Sec->common.next){
		if(Sec->common.section_type == MSEC_DEADPUBLISHER){
			if(!mqtttokcmp(Sec->DeadPublisher.topic, topic)){	/* Topic found */
				uint64_t v = 1;
				if(write( Sec->DeadPublisher.rcv, &v, sizeof(v) ) == -1)	/* Signal it */
					publishLog('E', "eventfd to signal message reception : %s", strerror( errno ));

#ifdef LUA
				if( !Sec->DeadPublisher.disabled )
					execUserFuncDeadPublisher( &(Sec->DeadPublisher), topic, payload );
#endif
			}
		} else if(Sec->common.section_type == MSEC_RTSCMD){
			if(!mqtttokcmp(Sec->RTSCmd.topic, topic))	/* Topic found */
				processRTSCmd( &(Sec->RTSCmd), payload );
		} else if(Sec->common.section_type == MSEC_OUTFILE){
			if(!mqtttokcmp(Sec->OutFile.topic, topic))	/* Topic found */
				processOutFile( &(Sec->OutFile), payload );
		}
	}

	MQTTClient_freeMessage(&msg);
	MQTTClient_free(topic);
	return 1;
}

static void connlost(void *ctx, char *cause){
	publishLog('W', "Broker connection lost due to %s", cause);
	if(cfg.ConLostFatal)
		exit(EXIT_FAILURE);
}

static void brkcleaning(void){	/* Clean broker stuffs */
	MQTTClient_disconnect(cfg.client, 10000);	/* 10s for the grace period */
	MQTTClient_destroy(&cfg.client);
}

static void handleInt(int na){
	exit(EXIT_SUCCESS);
}

int main(int ac, char **av){
	const char *conf_file = DEFAULT_CONFIGURATION_FILE;
	pthread_attr_t thread_attr;
	MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
	int c;

	while((c = getopt(ac, av, "vhf:t")) != EOF) switch(c){
	case 'h':
		fprintf(stderr, "%s (%s)\n"
			"Publish Smart Home figures to an MQTT broker\n"
			"Known options are :\n"
			"\t-h : this online help\n"
			"\t-v : enable verbose messages\n"
			"\t-f<file> : read <file> for configuration\n"
			"\t\t(default is '%s')\n"
			"\t-t : test configuration file and exit\n",
			basename(av[0]), MARCEL_VERSION, DEFAULT_CONFIGURATION_FILE
		);
		exit(EXIT_FAILURE);
		break;
	case 't':
		configtest = true;
		printf("%s v%s\n", basename(av[0]), MARCEL_VERSION);
	case 'v':
		verbose = true;
		break;
	case 'f':
		conf_file = optarg;
		break;
	default:
		publishLog('F', "Unknown option '%c'\n%s -h\n\tfor some help\n", c, av[0]);
		exit(EXIT_FAILURE);
	}
	read_configuration( conf_file );

	if(configtest){
		publishLog('W', "Testing only the configuration ... leaving.");
		exit(EXIT_FAILURE);
	}

		/* Connecting to the broker */
	conn_opts.reliable = 0;
	MQTTClient_create( &cfg.client, cfg.Broker, cfg.ClientID, MQTTCLIENT_PERSISTENCE_NONE, NULL);
	MQTTClient_setCallbacks( cfg.client, NULL, connlost, msgarrived, NULL);

	switch( MQTTClient_connect( cfg.client, &conn_opts) ){
	case MQTTCLIENT_SUCCESS : 
		break;
	case 1 : publishLog('F', "Unable to connect : Unacceptable protocol version");
		exit(EXIT_FAILURE);
	case 2 : publishLog('F', "Unable to connect : Identifier rejected");
		exit(EXIT_FAILURE);
	case 3 : publishLog('F', "Unable to connect : Server unavailable");
		exit(EXIT_FAILURE);
	case 4 : publishLog('F', "Unable to connect : Bad user name or password");
		exit(EXIT_FAILURE);
	case 5 : publishLog('F', "Unable to connect : Not authorized");
		exit(EXIT_FAILURE);
	default :
		publishLog('F', "Unable to connect");
		exit(EXIT_FAILURE);
	}
	atexit(brkcleaning);

		/* Curl related */
	curl_global_init(CURL_GLOBAL_ALL);
	atexit( curl_global_cleanup );

		/* Display / publish copyright */
	publishLog('W', MARCEL_COPYRIGHT);
	publishLog('W', "%s v%s starting ...", basename(av[0]), MARCEL_VERSION);

		/* Sections related */
	init_alerting();
#ifdef LUA
	init_Lua( conf_file );
#endif
#ifdef RFXTRX
	init_RFX();
#endif

#ifdef INOTIFY
	int infd = inotify_init();
	if( infd < 0 ){
		perror("inotify_init()");
		exit(EXIT_FAILURE);
	}
#endif

		/* Creating childs */
	if(verbose)
		puts("\nCreating childs processes\n"
			   "---------------------------");

	assert(!pthread_attr_init (&thread_attr));
	assert(!pthread_attr_setdetachstate (&thread_attr, PTHREAD_CREATE_DETACHED));

	if( cfg.OwAlarm ){
		if( !cfg.OwAlarmSample )
			publishLog('E', "Can't launch 1-wire Alarm monitoring : 0 waiting delay");
		else {
			if(pthread_create( &cfg.OwAlarmThread, &thread_attr, process_1wAlrm, NULL) < 0){
				publishLog('F', "Can't create a processing thread");
				exit(EXIT_FAILURE);
			}
		}
	}

	bool firstFFV=true;

	for(union CSection *s = cfg.sections; s; s = s->common.next){
		switch(s->common.section_type){
		case MSEC_FFV:
			if( !s->common.topic ){
				s->common.topic = s->common.uid;
				publishLog('W', "[%s] no topic specified, using the uid.", s->common.uid);
			}
			if(s->common.sample){
				if(pthread_create( &(s->common.thread), &thread_attr, process_FFV, s) < 0){
					publishLog('F', "[%s] Can't create a processing thread", s->common.uid);
					exit(EXIT_FAILURE);
				}
				firstFFV=false;
			} else
				if(firstFFV)
					publishLog('E', "Can't launch FFV for '%s' : 0 waiting delay", s->FFV.topic);
			break;
#ifdef FREEBOX
		case MSEC_FREEBOX:
			if(!s->common.sample){
				publishLog('E', "Freebox section without sample time : ignoring ...");
			} else {
				if( !s->common.topic ){
					s->common.topic = s->common.uid;
					publishLog('W', "[%s] no topic specified, using the uid.", s->common.uid);
				}
				if(pthread_create( &(s->common.thread), &thread_attr, process_Freebox, s) < 0){
					publishLog('F', "[Freebox] Can't create a processing thread");
					exit(EXIT_FAILURE);
				}
			}
			firstFFV=true;
			break;
#endif
#ifdef UPS
		case MSEC_UPS:
			if(!s->common.sample){ /* we won't group UPS to prevent too many DNS lookup */
				publishLog('E', "[%s] UPS section without sample time : ignoring ...", s->common.uid);
			} else {
				if( !s->common.topic ){
					s->common.topic = s->common.uid;
					publishLog('W', "[%s] no topic specified, using the uid.", s->common.uid);
				}
				if(pthread_create( &(s->common.thread), &thread_attr, process_UPS, s) < 0){
					publishLog('F', "Can't create a processing thread");
					exit(EXIT_FAILURE);
				}
			}
			firstFFV=true;
			break;
#endif
#ifdef LUA
		case MSEC_EVERY:
			if(!s->Every.funcname)
				publishLog('E', "EVERY without function defined : ignoring ...");
			else if(pthread_create( &(s->common.thread), &thread_attr, process_Every, s) < 0){
				publishLog('F', "Can't create a processing thread");
				exit(EXIT_FAILURE);
			}
			firstFFV=true;
			break;
#endif
		case MSEC_DEADPUBLISHER:
			if(!s->common.topic){
				publishLog('E', "configuration error : no topic specified, ignoring DPD '%s' section", s->common.uid );
			} else if(!s->common.sample && !s->DeadPublisher.funcname){
				publishLog('E', "DeadPublisher section without sample time or user function defined : ignoring ...");
			} else {
				if(pthread_create( &(s->common.thread), &thread_attr, process_DPD, s) < 0){
					publishLog('F', "Can't create a processing thread");
					exit(EXIT_FAILURE);
				}
			}
			firstFFV=true;
			break;
		case MSEC_OUTFILE:
			if(!s->OutFile.file){
				publishLog('E', "configuration error : no file specified, ignoring OutFile '%s' section", s->common.uid);
			} else {
				if( !s->common.topic ){
					s->common.topic = s->common.uid;
					publishLog('W', "[%s] no topic specified, using the uid.", s->common.uid);
				}
				if(MQTTClient_subscribe( cfg.client, s->common.topic, 0 ) != MQTTCLIENT_SUCCESS )
					publishLog('E', "Can't subscribe to '%s'", s->common.topic );
			}
			firstFFV=true;
			break;
#ifdef METEO
		case MSEC_METEO3H:
			if(!s->common.sample){
				publishLog('E', "Meteo3H '%s' section without sample time : ignoring ...", s->common.uid);
			} else if(pthread_create( &(s->common.thread), &thread_attr, process_Meteo3H, s) < 0){
				publishLog('F', "Can't create a processing thread");
				exit(EXIT_FAILURE);
			}			
			firstFFV=true;
			break;
		case MSEC_METEOD:
			if(!s->common.sample){
				publishLog('E', "Meteo Daily '%s' section without sample time : ignoring ...", s->common.uid);
			} else if(pthread_create( &(s->common.thread), &thread_attr, process_MeteoD, s) < 0){
				publishLog('F', "Can't create a processing thread");
				exit(EXIT_FAILURE);
			}			
			firstFFV=true;
			break;
#endif
#ifdef RFXTRX
		case MSEC_RTSCMD:
			if(!s->common.topic || !s->RTSCmd.did ){
				publishLog('E', "configuration error : no topic or device ID specified, ignoring this RTSCmd section");
			} else if(cfg.RFXdevice){
				if(MQTTClient_subscribe( cfg.client, s->common.topic, 0 ) != MQTTCLIENT_SUCCESS ){
					publishLog('E', "Can't subscribe to '%s'", s->common.topic );
				}
			}
			firstFFV=true;
			break;
#endif
		case MSEC_REST:
			if(!s->REST.sample && s->REST.at == -1)
				publishLog('E', "REST without sample or \"At\" time is useless : ignoring ...");
			else if(!s->REST.funcname)
				publishLog('E', "REST without function defined : ignoring ...");
			else if(pthread_create( &(s->common.thread), &thread_attr, process_REST, s) < 0){
				publishLog('F', "Can't create a processing thread");
				exit(EXIT_FAILURE);
			}
			firstFFV=true;
			break;
#ifdef INOTIFY
		case MSEC_LOOK4CHANGES:
			if(!s->Look4Changes.dir || !s->Look4Changes.flags)
				publishLog('E', "LookForChanges without anything to look for is useless : ignoring ...");
			else if(!s->Look4Changes.topic)
				publishLog('E', "LookForChanges without a topic is useless : ignoring ...");
			else {
				if((s->Look4Changes.wd = inotify_add_watch(infd, s->Look4Changes.dir, s->Look4Changes.flags)) < 0){
					const char *emsg = strerror(errno);
					publishLog('E', "[%s] %s : %s", s->Look4Changes.uid, s->Look4Changes.dir, emsg);
				}
			}
			break;
#endif
#ifdef SHT31
		case MSRC_SHT31:
			if( !s->common.topic ){
				s->common.topic = s->common.uid;
				publishLog('W', "[%s] no topic specified, using the uid.", s->common.uid);
			}
			if(s->common.sample){
				if(pthread_create( &(s->common.thread), &thread_attr, process_Sht31, s) < 0){
					publishLog('F', "[%s] Can't create a processing thread", s->common.uid);
					exit(EXIT_FAILURE);
				}
			} else
				publishLog('E', "Can't launch SHT31 for '%s' : 0 waiting delay", s->FFV.topic);
			firstFFV=true;
			break;
#endif
		default :	/* Ignore unsupported type */
			break;
		}
	}


		/* Marcel's own topic */
	int i = strlen(cfg.OnOffTopic);
	assert( i > 1 );
	cfg.OnOffTopic[i] = '#';
	cfg.OnOffTopic[i+1] = 0;
	if(MQTTClient_subscribe( cfg.client, cfg.OnOffTopic, 0 ) != MQTTCLIENT_SUCCESS ){
		publishLog('E', "Can't subscribe to '%s'", cfg.OnOffTopic );
	}
	cfg.OnOffTopic[i] = 0;

	signal(SIGINT, handleInt);

#ifdef INOTIFY
	/* We may use a simple read() here, but poll() solution opens for other
	 * even handling ... */
	struct pollfd fds[1];
	fds[0].fd = infd;
	fds[0].events = POLLIN;
	
	publishLog('I', "Waiting for notification to come");
	
	for(;;){
		int r=poll(fds, sizeof(fds)/sizeof(fds[0]), -1);

		if(r < 0){
			if( errno == EINTR)	/* Signal received */
				continue;
			publishLog('E', "poll() : %s", strerror(errno));
		}
			/* Enumerate here all FDs (only one up to now) */
		if( fds[0].revents & POLLIN){
			char buf[ INOTIFY_BUF_LEN ];

			for(;;){	/* Read events */
				ssize_t len = read(infd, buf, INOTIFY_BUF_LEN);
				const struct inotify_event *event;

				if(len == -1){
					if(errno == EAGAIN)	/* Nothing to read */
						break;
					publishLog('F', "read() : %s", strerror(errno));
					exit(EXIT_FAILURE);
				}

				for(char *ptr = buf; ptr < buf + len; ptr += sizeof(struct inotify_event) + event->len){
					event = (const struct inotify_event *) ptr;
					for( struct _Look4Changes *s = cfg.first_L4C; s; s = (struct _Look4Changes *)s->next ){
						if( s->section_type != MSEC_LOOK4CHANGES ){
							if( cfg.L4Cgrouped )
								break;	/* L4C list over */
							else
								continue;	/* skip this one */
						}
						if( event->wd == s->wd ){
							char *amsg=NULL;
							if(event->mask & IN_ACCESS)
								amsg = stradd( amsg, ",ACCESS", true);
							if(event->mask & IN_ATTRIB)
								amsg = stradd( amsg, ",ATTRIB", true);
							if(event->mask & IN_CLOSE_NOWRITE)
								amsg = stradd( amsg, ",CLOSE_NOWRITE", true);
							if(event->mask & IN_CLOSE_WRITE)
								amsg = stradd( amsg, ",CLOSE_WRITE", true);
							if(event->mask & IN_CREATE)
								amsg = stradd( amsg, ",CREATE", true);
							if(event->mask & IN_DELETE)
								amsg = stradd( amsg, ",DELETE", true);
							if(event->mask & IN_DELETE_SELF)
								amsg = stradd( amsg, ",DELETE_SELF", true);
							if(event->mask & IN_IGNORED)
								amsg = stradd( amsg, ",IGNORED", true);
							if(event->mask & IN_ISDIR)
								amsg = stradd( amsg, ",ISDIR", true);
							if(event->mask & IN_MODIFY)
								amsg = stradd( amsg, ",MODIFY", true);
							if(event->mask & IN_MOVE_SELF)
								amsg = stradd( amsg, ",MOVE_SELF", true);
							if(event->mask & IN_MOVED_FROM)
								amsg = stradd( amsg, ",MOVED_FROM", true);
							if(event->mask & IN_MOVED_TO)
								amsg = stradd( amsg, ",MOVED_TO", true);
							if(event->mask & IN_OPEN)
								amsg = stradd( amsg, ",OPEN", true);
							if(event->mask & IN_Q_OVERFLOW)
								amsg = stradd( amsg, ",Q_OVERFLOW", true);
							if(event->mask & IN_UNMOUNT)
								amsg = stradd( amsg, ",UNMOUNT", true);
							
							size_t sz = event->len + strlen(amsg) + 2;
							char msg[sz+1];
							sprintf(msg, "%s:%s", event->len ? event->name : "", amsg);
							free(amsg);
							mqttpublish(cfg.client, s->topic, sz, msg, s->retained);
						}
					}
				}
			}
		}
	}
#else
	pause();
#endif

	exit(EXIT_SUCCESS);
}
