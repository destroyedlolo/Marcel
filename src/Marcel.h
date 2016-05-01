/* Marcel.h
 * 	Shared definition
 *
 * This file is part of Marcel project and is following the same
 * license rules (see LICENSE file)
 *
 * 08/07/2015	- LF - start v2.0 - make source modular
 * 20/03/2016	- LF - add named notification handling
 * 21/04/2016	- LF - add errortopic for DPD
 * 29/04/2016	- LF - add support for RFXtrx
 */

#ifndef MARCEL_H
#define MARCEL_H

#define _POSIX_C_SOURCE 200112L	/* Otherwise some defines/types are not defined with -std=c99 */

#include <pthread.h>
#include <MQTTClient.h> /* PAHO library needed */ 
#include <stdint.h>		/* uint*_t */

#define VERSION "4.8b"	/* Need to stay numerique as exposed to Lua */

#define DEFAULT_CONFIGURATION_FILE "/usr/local/etc/Marcel.conf"
#define MAXLINE 1024	/* Maximum length of a line to be read */
#define BRK_KEEPALIVE 60	/* Keep alive signal to the broker */

	/* Configuration / context */
enum _tp_msec {
	MSEC_INVALID =0,	/* Ignored */
	MSEC_FFV,			/* File String Value */
	MSEC_FREEBOX,		/* FreeBox */
	MSEC_UPS,			/* UPS */
	MSEC_DEADPUBLISHER,	/* alarm on missing MQTT messages */
	MSEC_EVERY,			/* Launch a function at given interval */
	MSEC_METEO3H,		/* 3H meteo */
	MSEC_METEOD,		/* Daily meteo */
	MSEC_RTSCMD			/* Somfy RTS commands */
};

struct var {	/* Storage for var list */
	struct var *next;
	const char *name;
};

union CSection {
	struct {	/* Fields common to all sections */
		union CSection *next;
		enum _tp_msec section_type;
		pthread_t thread;	/* Child to handle this section */
		const char *topic;
		int sample;
	} common;
	struct _FFV {
		union CSection *next;
		enum _tp_msec section_type;
		pthread_t thread;
		const char *topic;	/* Topic to publish to */
		int sample;			/* delay b/w 2 samples */
		const char *file;	/* File containing the data to read */
	} FFV;
	struct _FreeBox {
		union CSection *next;
		enum _tp_msec section_type;
		pthread_t thread;	
		const char *topic;	/* Root of the topics to publish to */
		int sample;			/* delay b/w 2 samples */
	} FreeBox;
	struct _UPS {
		union CSection *next;
		enum _tp_msec section_type;
		pthread_t thread;
		const char *topic;	/* Root of the topics to publish to */
		int sample;			/* delay b/w 2 samples */
		const char *section_name;	/* Name of the UPS as defined in NUT */
		const char *host;	/* NUT's server */
		int port;			/* NUT's port */
		struct var *var_list;	/* List of variables to read */
	} Ups;
	struct _DeadPublisher {
		union CSection *next;
		enum _tp_msec section_type;
		pthread_t thread;
		const char *topic;	/* Topic to wait data from */
		int sample;			/* Timeout */
		const char *errtopic;	/* Topic to publish error to */
		const char *funcname;	/* User function to call on data arrival */
		int funcid;			/* Function id in Lua registry */
		const char *errorid;	/* Error's name */
		int rcv;			/* Event for data receiving */
		int inerror;		/* true if this DPD is in error */
	} DeadPublisher;
	struct _Every {
		union CSection *next;
		enum _tp_msec section_type;
		pthread_t thread;
		const char *name;	/* Name of the section, passed to Lua function */
		int sample;			/* Delay b/w launches */
		const char *funcname;	/* Function to be called */
		int funcid;			/* Function id in Lua registry */
	} Every;
	struct _Meteo {
		union CSection *next;
		enum _tp_msec section_type;
		pthread_t thread;	/* Child to handle this section */
		const char *topic;	/* Root of the topics to publish to */
		int sample;			/* Delay b/w 2 queries */
		const char *City;	/* CityName,Country to query */
		const char *Units;	/* Result's units */
		const char *Lang;	/* Result's language */
	} Meteo;
	struct _RTSCmd {
		union CSection *next;
		enum _tp_msec section_type;
		pthread_t thread;	/* Child to handle this section */
		const char *topic;	/* Topic to wait data from */
		uint32_t id;		/* ID corresponding to this device */
	} RTSCmd;
};

struct notification {	/* Storage for named notification */
	struct notification *next;
	char id;
	char *url;
	char *cmd;
};

extern struct Config {
	union CSection *sections;	/* Sections' list */
	const char *Broker;		/* Broker's URL */
	const char *ClientID;	/* Marcel client id : must be unique among a broker clients */
	MQTTClient client;
	int DPDlast;			/* Dead Publisher Detect are grouped at the end of sections list */
	int ConLostFatal;		/* Die if broker connection is lost */
	union CSection *first_DPD;	/* Pointer to the first DPD */
	const char *luascript;	/* file containing Lua functions */
		/* Single alert / notification */
	const char *SMSurl;		/* Where to send SMS */
	const char *AlertCmd;	/* External command to send alerts */
		/* Alerts by id */
	struct notification *notiflist;
	const char *RFXdevice;
} cfg;

	/* Helper functions */
extern int verbose;

extern char *removeLF(char *);
extern char *striKWcmp( char *, const char * );
extern char *mystrdup(const char *);
#define strdup(s) mystrdup(s)

extern size_t socketreadline( int, char *, size_t);

	/* Broker related */
extern int papub( const char *, int, void *, int);

	/* Lua related */
#ifdef LUA
#include <lua.h>		/* Lua's Basic */
extern lua_State *L;

extern void init_Lua( const char * );
extern int findUserFunc( const char * );
extern void execUserFuncDeadPublisher( struct _DeadPublisher *, const char *, const char *);
extern void execUserFuncEvery( struct _Every * );
#endif
#endif

