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
 *
 *	Copyright 2015-2016 Laurent Faillie
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
 *	06/06/2016	- LF - switch to v5.0 - Prepare Alarm handling
 *							FFV, Every's sample can be -1. 
 *							In this case, launched only once.
 *	08/06/2016	- LF - 5.01 - 1-wire Alarm handled
 *	24/07/2016	- LF - 5.02 - Handle offset for FFV
 *	14/08/2016	- LF - 5.03 - Handle Outfile
 *				-------
 *	19/08/2016	- LF - switch to v6.0 - prepare controls
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

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#ifdef LUA
#	include <lauxlib.h>
#endif

bool verbose = false;
bool configtest = false;
struct Config cfg;

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
		fputs("*F* Empty uid provided.\n", stderr);
		exit(EXIT_FAILURE);
	}

	int h = chksum(uid);

	for(union CSection *s = cfg.sections; s; s = s->common.next){
		if( s->common.h == h && !strcmp(s->common.uid, uid) ){
			fprintf(stderr, "*F* '%s' uid is used more than once.\n", uid);
			exit(EXIT_FAILURE);
		}
	}

	assert( sec->common.uid = strdup( uid ) );
	sec->common.h = h;
}

static void read_configuration( const char *fch){
	FILE *f;
	char l[MAXLINE];
	char *arg;
	union CSection *last_section=NULL;

	cfg.sections = NULL;
	cfg.Broker = "tcp://localhost:1883";
	cfg.ClientID = "Marcel";
	cfg.client = NULL;
	cfg.Sublast = false;
	cfg.ConLostFatal = false;
	cfg.first_Sub = NULL;

	cfg.luascript = NULL;

	cfg.RFXdevice = NULL;

	cfg.SMSurl = NULL;
	cfg.AlertCmd= NULL;
	cfg.notiflist=NULL;

	cfg.OwAlarm=NULL;
	cfg.OwAlarmSample=0;

	if(verbose)
		printf("\nReading configuration file '%s'\n---------------------------\n", fch);

	if(!(f=fopen(fch, "r"))){
		perror(fch);
		exit(EXIT_FAILURE);
	}

	while(fgets(l, MAXLINE, f)){
		if(*l == '#' || *l == '\n')
			continue;

		if((arg = striKWcmp(l,"Broker="))){
			assert( cfg.Broker = strdup( removeLF(arg) ) );
			if(verbose)
				printf("Broker : '%s'\n", cfg.Broker);
		} else if((arg = striKWcmp(l,"ClientID="))){
			assert( cfg.ClientID = strdup( removeLF(arg) ) );
			if(verbose)
				printf("MQTT Client ID : '%s'\n", cfg.ClientID);
		} else if((arg = striKWcmp(l,"SMSUrl="))){
			if(cfg.notiflist){
				assert( cfg.notiflist->url = strdup( removeLF(arg) ) );
				if(verbose)
					printf("\tSMS Url : '%s'\n", cfg.notiflist->url);
			} else {
				assert( cfg.SMSurl = strdup( removeLF(arg) ) );
				if(verbose)
					printf("SMS Url : '%s'\n", cfg.SMSurl);
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
			fputs("*E* RFXtrx_Port defined without RFXtrx support enabled\n", stderr);
#endif
		} else if((arg = striKWcmp(l,"1wire-Alarm="))){
			assert( cfg.OwAlarm = strdup( removeLF(arg) ) );
			if(verbose)
				printf("1 wire Alarm directory : '%s'\n", cfg.OwAlarm);
		} else if((arg = striKWcmp(l,"1wire-Alarm-sample="))){
			cfg.OwAlarmSample = atoi( arg );
			if(verbose)
				printf("1 wire Alarm sample delay : '%d'\n", cfg.OwAlarmSample);
		} else if((arg = striKWcmp(l,"UserFuncScript="))){
#ifdef LUA
			assert( cfg.luascript = strdup( removeLF(arg) ) );
			if(verbose)
				printf("User functions definition script : '%s'\n", cfg.luascript);
#else
			fputs("*E* UserFuncScript defined without Lua support enabled\n", stderr);
#endif
		} else if((arg = striKWcmp(l,"$alert="))){
			struct notification *n = malloc( sizeof(struct notification) );
			assert(n);
			memset(n, 0, sizeof(struct notification));

			if(!*arg || *arg == '\n'){
				fputs("*F* Unamed $alert section, giving up !\n", stderr);
				exit(EXIT_FAILURE);
			}

			n->id = *arg;
			n->next = cfg.notiflist;
			cfg.notiflist = n;

			if(verbose)
				printf("Entering alert definition '%c'\n", n->id);
		} else if((arg = striKWcmp(l,"*FFV="))){
			union CSection *n = malloc( sizeof(struct _FFV) );
			assert(n);
			memset(n, 0, sizeof(struct _FFV));
			n->common.section_type = MSEC_FFV;
			setUID( n, removeLF(arg) );

			if(last_section)
				last_section->common.next = n;
			else	/* First section */
				cfg.sections = n;
			last_section = n;
			if(verbose)
				printf("Entering FFV section '%s'\n", n->common.uid);
		} else if((arg = striKWcmp(l,"*OutFile="))){
			union CSection *n = malloc( sizeof(struct _OutFile) );
			assert(n);
			memset(n, 0, sizeof(struct _OutFile));
			n->common.section_type = MSEC_OUTFILE;
			setUID( n, removeLF(arg) );

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
			union CSection *n = malloc( sizeof(struct _FreeBox) );
			assert(n);
			memset(n, 0, sizeof(struct _FreeBox));
			n->common.section_type = MSEC_FREEBOX;
			setUID( n, "Freebox" );

			if(last_section)
				last_section->common.next = n;
			else	/* First section */
				cfg.sections = n;
			last_section = n;
			if(verbose)
				puts("Entering section 'Freebox'");
		} else if((arg = striKWcmp(l,"*UPS="))){
			union CSection *n = malloc( sizeof(struct _UPS) );
			assert(n);
			memset(n, 0, sizeof(struct _UPS));
			n->common.section_type = MSEC_UPS;

			assert( n->Ups.section_name = strdup( removeLF(arg) ) );

			if(last_section)
				last_section->common.next = n;
			else	/* First section */
				cfg.sections = n;
			last_section = n;
			if(verbose)
				printf("Entering section 'UPS/%s'\n", n->Ups.section_name);
		} else if((arg = striKWcmp(l,"*Every="))){
			union CSection *n = malloc( sizeof(struct _Every) );
			assert(n);
			memset(n, 0, sizeof(struct _Every));
			n->common.section_type = MSEC_EVERY;

			assert( n->Every.name = strdup( removeLF(arg) ));

#ifndef LUA
			fputs("*F* Every section is only available when compiled with Lua support\n", stderr);
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
				printf("Entering section 'Every/%s'\n", n->Every.name );
		} else if((arg = striKWcmp(l,"*Meteo3H"))){
			union CSection *n = malloc( sizeof(struct _Meteo) );
			assert(n);
			memset(n, 0, sizeof(struct _Meteo));
			n->common.section_type = MSEC_METEO3H;

#ifndef METEO
			fputs("*F* Meteo3H section is only available when compiled with METEO support\n", stderr);
			exit(EXIT_FAILURE);
#endif

			if(last_section)
				last_section->common.next = n;
			else	/* First section */
				cfg.sections = n;
			last_section = n;
			if(verbose)
				puts("Entering section 'Meteo 3H");
		} else if((arg = striKWcmp(l,"*MeteoDaily"))){
			union CSection *n = malloc( sizeof(struct _Meteo) );
			assert(n);
			memset(n, 0, sizeof(struct _Meteo));
			n->common.section_type = MSEC_METEOD;

#ifndef METEO
			fputs("*F* MeteoDaily section is only available when compiled with METEO support\n", stderr);
			exit(EXIT_FAILURE);
#endif

			if(last_section)
				last_section->common.next = n;
			else	/* First section */
				cfg.sections = n;
			last_section = n;
			if(verbose)
				puts("Entering section 'Meteo Daily'");
		} else if((arg = striKWcmp(l,"*DPD="))){
			union CSection *n = malloc( sizeof(struct _DeadPublisher) );
			assert(n);
			memset(n, 0, sizeof(struct _DeadPublisher));
			n->common.section_type = MSEC_DEADPUBLISHER;
			setUID( n, removeLF(arg) );

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
			union CSection *n = malloc( sizeof(struct _RTSCmd) );
			assert(n);
			memset(n, 0, sizeof(struct _RTSCmd));
			n->common.section_type = MSEC_RTSCMD;
			setUID( n, removeLF(arg) );

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
			union CSection *n = malloc( sizeof(struct _REST) );
			assert(n);
			memset(n, 0, sizeof(struct _REST));
			n->common.section_type = MSEC_REST;
			n->REST.at = -1;

			assert( n->REST.url = strdup( removeLF(arg) ));

#ifndef LUA
			fputs("*F* REST section is only available when compiled with Lua support\n", stderr);
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
				printf("Entering section REST '%s'\n", n->REST.url );
		} else if(!strcmp(l,"DPDLast\n") || !strcmp(l,"SubLast\n")){	/* Subscriptions are grouped at the end of the configuration file */
			cfg.Sublast = true;
			if(verbose)
				puts("Subscriptions (DPD, RTSCmd) sections are grouped at the end of the configuration");
		} else if(!strcmp(l,"ConnectionLostIsFatal\n")){	/* Crash if the broker connection is lost */
			cfg.ConLostFatal = true;
			if(verbose)
				puts("Crash if the broker connection is lost");
		} else if((arg = striKWcmp(l,"File="))){
			if(!last_section || (
				last_section->common.section_type != MSEC_FFV &&
				last_section->common.section_type != MSEC_OUTFILE
			) ){
				fputs("*F* Configuration issue : File directive outside FFV or OutFile section\n", stderr);
					exit(EXIT_FAILURE);
			}
			assert( last_section->FFV.file = strdup( removeLF(arg) ));
			if(verbose)
				printf("\tFile : '%s'\n", last_section->FFV.file);
		} else if((arg = striKWcmp(l,"Latch="))){
			if(!last_section || last_section->common.section_type != MSEC_FFV){
				fputs("*F* Configuration issue : Latch directive outside a FFV section\n", stderr);
					exit(EXIT_FAILURE);
			}
			assert( last_section->FFV.latch = strdup( removeLF(arg) ));
			if(verbose)
				printf("\tLatch file : '%s'\n", last_section->FFV.latch);
		} else if((arg = striKWcmp(l,"Offset="))){
			if(!last_section || last_section->common.section_type != MSEC_FFV){
				fputs("*F* Configuration issue : Offset directive outside a FFV section\n", stderr);
					exit(EXIT_FAILURE);
			}
			last_section->FFV.offset = atof(arg);
			if(verbose){
				printf("\tOffset : %f\n", last_section->FFV.offset);
				if(!last_section->FFV.offset)
					puts("*W*\tIs it normal it's a NULL offset ?");
			}
		} else if((arg = striKWcmp(l,"Host="))){
			if(!last_section || last_section->common.section_type != MSEC_UPS){
				fputs("*F* Configuration issue : Host directive outside a UPS section\n", stderr);
					exit(EXIT_FAILURE);
			}
			assert( last_section->Ups.host = strdup( removeLF(arg) ));
			if(verbose)
				printf("\tHost : '%s'\n", last_section->Ups.host);
		} else if((arg = striKWcmp(l,"Port="))){
			if(!last_section || last_section->common.section_type != MSEC_UPS){
				fputs("*F* Configuration issue : Port directive outside a UPS section\n", stderr);
				exit(EXIT_FAILURE);
			}
			if(!(last_section->Ups.port = atoi(arg))){
				fputs("*F* Configuration issue : Port is null (or is not a number)\n", stderr);
				exit(EXIT_FAILURE);
			}
			if(verbose)
				printf("\tPort : %d\n", last_section->Ups.port);
		} else if((arg = striKWcmp(l,"ID="))){
			if(!last_section || last_section->common.section_type != MSEC_RTSCMD){
				fputs("*F* Configuration issue : ID directive outside a RTSCmd section\n", stderr);
				exit(EXIT_FAILURE);
			}
			last_section->RTSCmd.did = strtol(arg, NULL, 0);
			if(verbose)
				printf("\tID : %04x\n", last_section->RTSCmd.did);
		} else if((arg = striKWcmp(l,"Var="))){
			if(!last_section || last_section->common.section_type != MSEC_UPS){
				fputs("*F* Configuration issue : Var directive outside a UPS section\n", stderr);
				exit(EXIT_FAILURE);
			}
			struct var *v = malloc(sizeof(struct var));
			assert(v);
			assert( v->name = strdup( removeLF(arg) ));
			v->next = last_section->Ups.var_list;
			last_section->Ups.var_list = v;
			if(verbose)
				printf("\tVar : '%s'\n", v->name);
		} else if((arg = striKWcmp(l,"Func="))){
			if(!last_section || (
				last_section->common.section_type != MSEC_DEADPUBLISHER &&
				last_section->common.section_type != MSEC_EVERY &&
				last_section->common.section_type != MSEC_REST
			)){
				fputs("*F* Configuration issue : Func directive outside a DPD, Every or REST section\n", stderr);
				exit(EXIT_FAILURE);
			}
#ifndef LUA
			fputs("*F* User functions can only be used when compiled with Lua support\n", stderr);
			exit(EXIT_FAILURE);
#endif
			assert( last_section->DeadPublisher.funcname = strdup( removeLF(arg) ));
			if(verbose)
				printf("\tFunction : '%s'\n", last_section->DeadPublisher.funcname);
		} else if(!strcmp(l,"RunIfOver\n")){	/* Crash if the broker connection is lost */
			if(!last_section || (
				last_section->common.section_type != MSEC_REST
			)){
				fputs("*F* Configuration issue : RunIfOver directive outside a REST section\n", stderr);
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
				fputs("*F* Configuration issue : Immediate directive outside a REST section\n", stderr);
				exit(EXIT_FAILURE);
			}
			last_section->REST.immediate = true;
			if(verbose)
				printf("\tImmediate\n");
		} else if((arg = striKWcmp(l,"At="))){
			if(!last_section || (
				last_section->common.section_type != MSEC_REST
			)){
				fputs("*F* Configuration issue : At directive outside a REST section\n", stderr);
				exit(EXIT_FAILURE);
			}

			last_section->REST.at = atoi( arg );
			if(verbose)
				printf("\tAt : '%d'\n", last_section->REST.at);
		} else if((arg = striKWcmp(l,"Sample="))){
			if(!last_section){
				fputs("*F* Configuration issue : Sample directive outside a section\n", stderr);
				exit(EXIT_FAILURE);
			}
			if( last_section->common.section_type == MSEC_RTSCMD ||
				last_section->common.section_type == MSEC_OUTFILE){
				fputs("*F* Configuration issue : Sample directive isn't compatible with RTSCmd or Outfile sections\n", stderr);
				exit(EXIT_FAILURE);
			}
			last_section->common.sample = atoi( arg );
			if(verbose)
				printf("\tDelay between samples : %ds\n", last_section->common.sample);
		} else if(!strcmp(l,"Disabled\n")){		/* This section is currently disabled */
			if(!last_section){
				fputs("*F* Configuration issue : Disabled directive outside a section\n", stderr);
				exit(EXIT_FAILURE);
			}
			last_section->common.disabled = true;
			if(verbose)
				puts("\tDisabled");
		} else if((arg = striKWcmp(l,"Topic="))){
			if(!last_section){
				fputs("*F* Configuration issue : Topic directive outside a section\n", stderr);
				exit(EXIT_FAILURE);
			}
			assert( last_section->common.topic = strdup( removeLF(arg) ));
			if(verbose)
				printf("\tTopic : '%s'\n", last_section->common.topic);
		} else if((arg = striKWcmp(l,"ErrorTopic="))){
			if(!last_section ||
				last_section->common.section_type != MSEC_DEADPUBLISHER
			){
				fputs("*F* Configuration issue : ErrorTopic directive outside DPD section\n", stderr);
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
				fputs("*F* Configuration issue : City directive outside a Meteo* section\n", stderr);
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
				fputs("*F* Configuration issue : Units directive outside a Meteo* section\n", stderr);
				exit(EXIT_FAILURE);
			}
			assert( last_section->Meteo.Units = strdup( removeLF(arg) ));
			if( strcmp( last_section->Meteo.Units, "metric" ) &&
				strcmp( last_section->Meteo.Units, "imperial" ) &&
				strcmp( last_section->Meteo.Units, "Standard" ) ){
				fputs("*F* Configuration issue : Units can only be only \"metric\" or \"imperial\" or \"Standard\"\n", stderr);
				exit(EXIT_FAILURE);
			}
			if(verbose)
				printf("\tUnits : '%s'\n", last_section->Meteo.Units);
		} else if((arg = striKWcmp(l,"Lang="))){
			if(!last_section || (
				last_section->common.section_type != MSEC_METEO3H &&
				last_section->common.section_type != MSEC_METEOD
			)){
				fputs("*F* Configuration issue : Lang directive outside a Meteo* section\n", stderr);
				exit(EXIT_FAILURE);
			}
			assert( last_section->Meteo.Lang = strdup( removeLF(arg) ));
			if(verbose)
				printf("\tLang : '%s'\n", last_section->Meteo.Lang);
		}
	}

	fclose(f);
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

	if(verbose)
		printf("*I* message arrival (topic : '%s', msg : '%s')\n", topic, payload);

	if((aid = striKWcmp(topic,"Alert/")))
		rcv_alert( aid, payload );
	else if((aid = striKWcmp(topic,"Notification/")))
		rcv_notification( aid, payload );
	else if((aid = striKWcmp(topic,"nNotification/")))
		rcv_nnotification( aid, payload );
	else for(; Sec; Sec = Sec->common.next){
		if(Sec->common.section_type == MSEC_DEADPUBLISHER){
			if(!mqtttokcmp(Sec->DeadPublisher.topic, topic)){	/* Topic found */
				uint64_t v = 1;
				if(write( Sec->DeadPublisher.rcv, &v, sizeof(v) ) == -1)	/* Signal it */
					perror("eventfd to signal message reception");

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
	printf("*W* Broker connection lost due to %s\n", cause);
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
			basename(av[0]), VERSION, DEFAULT_CONFIGURATION_FILE
		);
		exit(EXIT_FAILURE);
		break;
	case 't':
		configtest = true;
	case 'v':
		if(!verbose){
			puts("Marcel (c) L.Faillie 2015-2016");
			printf("%s v%s starting ...\n", basename(av[0]), VERSION);
		}
		verbose = true;
		break;
	case 'f':
		conf_file = optarg;
		break;
	default:
		fprintf(stderr, "Unknown option '%c'\n%s -h\n\tfor some help\n", c, av[0]);
		exit(EXIT_FAILURE);
	}
	read_configuration( conf_file );

	if(configtest){
		fputs("*W* Testing only the configuration ... leaving.\n", stderr);
		exit(EXIT_FAILURE);
	}

		/* Connecting to the broker */
	conn_opts.reliable = 0;
	MQTTClient_create( &cfg.client, cfg.Broker, cfg.ClientID, MQTTCLIENT_PERSISTENCE_NONE, NULL);
	MQTTClient_setCallbacks( cfg.client, NULL, connlost, msgarrived, NULL);

	switch( MQTTClient_connect( cfg.client, &conn_opts) ){
	case MQTTCLIENT_SUCCESS : 
		break;
	case 1 : fputs("Unable to connect : Unacceptable protocol version\n", stderr);
		exit(EXIT_FAILURE);
	case 2 : fputs("Unable to connect : Identifier rejected\n", stderr);
		exit(EXIT_FAILURE);
	case 3 : fputs("Unable to connect : Server unavailable\n", stderr);
		exit(EXIT_FAILURE);
	case 4 : fputs("Unable to connect : Bad user name or password\n", stderr);
		exit(EXIT_FAILURE);
	case 5 : fputs("Unable to connect : Not authorized\n", stderr);
		exit(EXIT_FAILURE);
	default :
		fputs("Unable to connect\n", stderr);
		exit(EXIT_FAILURE);
	}
	atexit(brkcleaning);

		/* Curl related */
	curl_global_init(CURL_GLOBAL_ALL);
	atexit( curl_global_cleanup );

		/* Sections related */
	init_alerting();
#ifdef LUA
	init_Lua( conf_file );
#endif
#ifdef RFXTRX
	init_RFX();
#endif

		/* Creating childs */
	if(verbose)
		puts("\nCreating childs processes\n"
			   "---------------------------");

	assert(!pthread_attr_init (&thread_attr));
	assert(!pthread_attr_setdetachstate (&thread_attr, PTHREAD_CREATE_DETACHED));

	if( cfg.OwAlarm ){
		if( !cfg.OwAlarmSample )
			fputs("*E* Can't launch 1-wire Alarm monitoring : 0 waiting delay", stderr);
		else {
			if(pthread_create( &cfg.OwAlarmThread, &thread_attr, process_1wAlrm, NULL) < 0){
				fputs("*F* Can't create a processing thread\n", stderr);
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
				if(verbose)
					printf("*W* [%s] no topic specified, using the uid.\n", s->common.uid);
			}
			if(s->common.sample){
				if(pthread_create( &(s->common.thread), &thread_attr, process_FFV, s) < 0){
					fprintf(stderr, "*F* [%s] Can't create a processing thread\n", s->common.uid);
					exit(EXIT_FAILURE);
				}
				firstFFV=false;
			} else
				if(firstFFV)
					fprintf(stderr, "*E* Can't launch FFV for '%s' : 0 waiting delay\n", s->FFV.topic);
			break;
#ifdef FREEBOX
		case MSEC_FREEBOX:
			if(!s->common.sample){
				fputs("*E* Freebox section without sample time : ignoring ...\n", stderr);
			} else {
				if( !s->common.topic ){
					s->common.topic = s->common.uid;
					if(verbose)
						printf("*W* [%s] no topic specified, using the uid.\n", s->common.uid);
				}
				if(pthread_create( &(s->common.thread), &thread_attr, process_Freebox, s) < 0){
					fputs("*F* [Freebox] Can't create a processing thread\n", stderr);
					exit(EXIT_FAILURE);
				}
			}
			firstFFV=true;
			break;
#endif
#ifdef UPS
		case MSEC_UPS:
			if(!s->common.sample){ /* we won't group UPS to prevent too many DNS lookup */
				fputs("*E* UPS section without sample time : ignoring ...\n", stderr);
			} else if(pthread_create( &(s->common.thread), &thread_attr, process_UPS, s) < 0){
				fputs("*F* Can't create a processing thread\n", stderr);
				exit(EXIT_FAILURE);
			}			
			firstFFV=true;
			break;
#endif
#ifdef LUA
		case MSEC_EVERY:
			if(!s->Every.funcname)
				fputs("*E* EVERY without function defined : ignoring ...\n", stderr);
			else if(pthread_create( &(s->common.thread), &thread_attr, process_Every, s) < 0){
				fputs("*F* Can't create a processing thread\n", stderr);
				exit(EXIT_FAILURE);
			}
			firstFFV=true;
			break;
#endif
		case MSEC_DEADPUBLISHER:
			if(!s->common.topic){
				fprintf(stderr, "*E* configuration error : no topic specified, ignoring DPD '%s' section\n", s->common.uid );
			} else if(!s->common.sample && !s->DeadPublisher.funcname){
				fputs("*E* DeadPublisher section without sample time or user function defined : ignoring ...\n", stderr);
			} else {
				if(MQTTClient_subscribe( cfg.client, s->common.topic, 0 ) != MQTTCLIENT_SUCCESS ){
					fprintf(stderr, "Can't subscribe to '%s'\n", s->common.topic );
				} else if(pthread_create( &(s->common.thread), &thread_attr, process_DPD, s) < 0){
					fputs("*F* Can't create a processing thread\n", stderr);
					exit(EXIT_FAILURE);
				}
			}
			firstFFV=true;
			break;
		case MSEC_OUTFILE:
			if(!s->OutFile.file){
				fprintf(stderr, "*E* configuration error : no file specified, ignoring OutFile '%s' section\n", s->common.uid);
			} else {
				if( !s->common.topic ){
					s->common.topic = s->common.uid;
					if(verbose)
						printf("*W* [%s] no topic specified, using the uid.\n", s->common.uid);
				}
				if(MQTTClient_subscribe( cfg.client, s->common.topic, 0 ) != MQTTCLIENT_SUCCESS )
					fprintf(stderr, "Can't subscribe to '%s'\n", s->common.topic );
			}
			firstFFV=true;
			break;
#ifdef METEO
		case MSEC_METEO3H:
			if(!s->common.sample){ /* we won't METEO */
				fputs("*E* Meteo3H section without sample time : ignoring ...\n", stderr);
			} else if(pthread_create( &(s->common.thread), &thread_attr, process_Meteo3H, s) < 0){
				fputs("*F* Can't create a processing thread\n", stderr);
				exit(EXIT_FAILURE);
			}			
			firstFFV=true;
			break;
		case MSEC_METEOD:
			if(!s->common.sample){ /* we won't METEO */
				fputs("*E* Meteo Daily section without sample time : ignoring ...\n", stderr);
			} else if(pthread_create( &(s->common.thread), &thread_attr, process_MeteoD, s) < 0){
				fputs("*F* Can't create a processing thread\n", stderr);
				exit(EXIT_FAILURE);
			}			
			firstFFV=true;
			break;
#endif
#ifdef RFXTRX
		case MSEC_RTSCMD:
			if(!s->common.topic || !s->RTSCmd.did ){
				fputs("*E* configuration error : no topic or device ID specified, ignoring this RTSCmd section\n", stderr);
			} else if(cfg.RFXdevice){
				if(MQTTClient_subscribe( cfg.client, s->common.topic, 0 ) != MQTTCLIENT_SUCCESS ){
					fprintf(stderr, "Can't subscribe to '%s'\n", s->common.topic );
				}
			}
			firstFFV=true;
			break;
#endif
		case MSEC_REST:
			if(!s->REST.sample && s->REST.at == -1)
				fputs("*E* REST without sample or \"At\" time is useless : ignoring ...\n", stderr);
			else if(!s->REST.funcname)
				fputs("*E* REST without function defined : ignoring ...\n", stderr);
			else if(pthread_create( &(s->common.thread), &thread_attr, process_REST, s) < 0){
				fputs("*F* Can't create a processing thread\n", stderr);
				exit(EXIT_FAILURE);
			}
			firstFFV=true;
			break;
		default :	/* Ignore unsupported type */
			break;
		}
	}

	signal(SIGINT, handleInt);
	pause();

	exit(EXIT_SUCCESS);
}

