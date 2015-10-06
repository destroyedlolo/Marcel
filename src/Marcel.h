/* Marcel.h
 * 	Shared definition
 *
 * This file is part of Marcel project and is following the same
 * license rules (see LICENSE file)
 *
 * 08/07/2015	- LF - start v2.0 - make source modular
 */

#ifndef MARCEL_H
#define MARCEL_H

#define _POSIX_C_SOURCE 200112L	/* Otherwise some defines/types are not defined with -std=c99 */

#include <pthread.h>
#include <MQTTClient.h> /* PAHO library needed */ 

#define VERSION "4.0"	/* Need to stay numerique as exposed to Lua */

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
	MSEC_METEOST		/* Short Term meteo */
};

struct var {	/* Storage for var list */
	struct var *next;
	const char *name;
};

union CSection {
	struct {	/* Fields common to all sections */
		union CSection *next;
		enum _tp_msec section_type;
		int sample;
		pthread_t thread;	/* Child to handle this section */
		const char *topic;
	} common;
	struct _FFV {
		union CSection *next;
		enum _tp_msec section_type;
		int sample;			/* delay b/w 2 samples */
		pthread_t thread;
		const char *topic;	/* Topic to publish to */
		const char *file;	/* File containing the data to read */
	} FFV;
	struct _FreeBox {
		union CSection *next;
		enum _tp_msec section_type;
		int sample;			/* delay b/w 2 samples */
		pthread_t thread;	
		const char *topic;	/* Root of the topics to publish to */
	} FreeBox;
	struct _UPS {
		union CSection *next;
		enum _tp_msec section_type;
		int sample;			/* delay b/w 2 samples */
		pthread_t thread;
		const char *topic;	/* Root of the topics to publish to */
		const char *section_name;	/* Name of the UPS as defined in NUT */
		const char *host;	/* NUT's server */
		int port;			/* NUT's port */
		struct var *var_list;	/* List of variables to read */
	} Ups;
	struct _DeadPublisher {
		union CSection *next;
		enum _tp_msec section_type;
		int sample;			/* Timeout */
		pthread_t thread;
		const char *topic;	/* Topic to wait data from */
		const char *funcname;	/* User function to call on data arrival */
		int funcid;			/* Function id in Lua registry */
		const char *errorid;	/* Error's name */
		int rcv;			/* Event for data receiving */
		int inerror;		/* true if this DPD is in error */
	} DeadPublisher;
	struct _Every {
		union CSection *next;
		enum _tp_msec section_type;
		int sample;			/* Delay b/w launches */
		pthread_t thread;
		const char *name;	/* Name of the section, passed to Lua function */
		const char *funcname;	/* Function to be called */
		int funcid;			/* Function id in Lua registry */
	} Every;
	struct _MeteoST {
		union CSection *next;
		enum _tp_msec section_type;
		int sample;			/* Delay b/w 2 queries */
		pthread_t thread;	/* Child to handle this section */
		const char *topic;	/* Root of the topics to publish to */
		const char *City;	/* CityName,Country to query */
		const char *Units;	/* Result's units */
		const char *Lang;	/* Result's language */
	} MeteoST;
};

struct Config {
	union CSection *sections;	/* Sections' list */
	const char *Broker;		/* Broker's URL */
	const char *ClientID;	/* Marcel client id : must be unique among a broker clients */
	MQTTClient client;
	int DPDlast;			/* Dead Publisher Detect are grouped at the end of sections list */
	int ConLostFatal;		/* Die if broker connection is lost */
	union CSection *first_DPD;	/* Pointer to the first DPD */
	const char *SMSurl;		/* Where to send SMS */
	const char *luascript;	/* file containing Lua functions */
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

