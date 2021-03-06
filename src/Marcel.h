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
 * 01/05/2016	- LF - Rename all DPD* as Sub*
 * 14/05/2016	- LF - add REST section
 * 07/06/2016	- LF - externalize version
 * 21/08/2016	- LF - starting v6.02, each section MUST have an ID
 * 31/08/2016	- LF - Add "keep" option"
 * 01/09/2016	- LF - Add publishLog() function
 * 18/08/2018	- LF - Add absolute time to Every
 * 25/11/2018	- LF - Add safe85 to FFV
 * 09/04/2021	- LF - Add Retained
 */

#ifndef MARCEL_H
#define MARCEL_H

#define _POSIX_C_SOURCE 200112L	/* Otherwise some defines/types are not defined with -std=c99 */

#include <pthread.h>
#include <MQTTClient.h> /* PAHO library needed */ 
#include <stdint.h>		/* uint*_t */
#include <stdbool.h>

#define DEFAULT_CONFIGURATION_FILE "/usr/local/etc/Marcel.conf"
#define MAXLINE 1024	/* Maximum length of a line to be read */
#define BRK_KEEPALIVE 60	/* Keep alive signal to the broker */

	/* Configuration / context */
enum _tp_msec {
	MSEC_INVALID =0,	/* Ignored */
	MSEC_FFV,			/* File String Value */
	MSEC_OUTFILE,		/* Output file */
	MSEC_FREEBOX,		/* FreeBox */
	MSEC_UPS,			/* UPS */
	MSEC_DEADPUBLISHER,	/* alarm on missing MQTT messages */
	MSEC_EVERY,			/* Launch a function at given interval */
	MSEC_LOOK4CHANGES,	/* Notify when a directory content changed */
	MSEC_METEO3H,		/* 3H meteo */
	MSEC_METEOD,		/* Daily meteo */
	MSEC_RTSCMD,		/* Somfy RTS commands */
	MSEC_REST,			/* REST query */
	MSRC_SHT31			/* Sht31 humidity sensor */
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
		const char *uid;	/* Unique identifier */
		int h;				/* hash code for this id */
		const char *topic;
		bool disabled;		/* this section is currently disabled */
		bool keep;			/* Stay alive in cas of failure */
		bool retained;		/* send MQTT retained message */
		int sample;
		const char *failfunc;	/* Lua function to be called in case of failure */
		int failfuncid;
	} common;
	struct _FFV {
		union CSection *next;
		enum _tp_msec section_type;
		pthread_t thread;
		const char *uid;	/* Unique identifier */
		int h;				/* hash code for this id */
		const char *topic;	/* Topic to publish to */
		bool disabled;
		bool keep;
		bool retained;		/* send MQTT retained message */
		int sample;			/* delay b/w 2 samples */
		const char *failfunc;
		int failfuncid;
		const char *funcname;	/* User function to call on data arrival */
		int funcid;			/* Function id in Lua registry */
		const char *file;	/* File containing the data to read */
		const char *latch;	/* Related latch file (optional) */
		float offset;		/* Offset to apply to the raw value */
		bool safe85;		/* Ignores underpowered temperature probes */
	} FFV;
	struct _OutFile {
		union CSection *next;
		enum _tp_msec section_type;
		pthread_t thread;
		const char *uid;	/* Unique identifier */
		int h;				/* hash code for this id */
		const char *topic;	/* Topic to subscribe to */
		bool disabled;
		bool keep;
		bool padding_retaining;
		int padding_sample;	/* not used */
		const char *failfunc;
		int failfuncid;
		const char *funcname;	/* User function to call on data arrival */
		int funcid;			/* Function id in Lua registry */
		const char *file;	/* File to write to */
	} OutFile;
	struct _FreeBox {
		union CSection *next;
		enum _tp_msec section_type;
		pthread_t thread;	
		const char *uid;	/* Unique identifier */
		int h;				/* hash code for this id */
		const char *topic;	/* Root of the topics to publish to */
		bool disabled;
		bool keep;
		bool retained;		/* send MQTT retained message */
		int sample;			/* delay b/w 2 samples */
		const char *failfunc;
		int failfuncid;
	} FreeBox;
	struct _UPS {
		union CSection *next;
		enum _tp_msec section_type;
		pthread_t thread;
		const char *uid;	/* Unique identifier */
		int h;				/* hash code for this id */
		const char *topic;	/* Root of the topics to publish to */
		bool disabled;
		bool keep;
		bool retained;		/* send MQTT retained message */
		int sample;			/* delay b/w 2 samples */
		const char *failfunc;
		int failfuncid;
		const char *host;	/* NUT's server */
		int port;			/* NUT's port */
		struct var *var_list;	/* List of variables to read */
	} Ups;
	struct _DeadPublisher {
		union CSection *next;
		enum _tp_msec section_type;
		pthread_t thread;
		const char *uid;	/* Unique identifier */
		int h;				/* hash code for this id */
		const char *topic;	/* Topic to wait data from */
		bool disabled;
		bool keep;
		bool padding_retaining;
		int sample;			/* Timeout */
		const char *failfunc;
		int failfuncid;
		const char *funcname;	/* User function to call on data arrival */
		int funcid;			/* Function id in Lua registry */
		const char *errtopic;	/* Topic to publish error to */
		int rcv;			/* Event for data receiving */
		bool inerror;		/* true if this DPD is in error */
	} DeadPublisher;
	struct _Every {	/* Must be aligned with _REST */
		union CSection *next;
		enum _tp_msec section_type;
		pthread_t thread;
		const char *uid;	/* Unique identifier */
		int h;				/* hash code for this id */
		const char *padding;	/* not used */
		bool disabled;
		bool keep;
		bool padding_retaining;
		int sample;			/* Delay b/w launches */
		const char *failfunc;
		int failfuncid;
		const char *funcname;	/* Function to be called */
		int funcid;			/* Function id in Lua registry */
		bool immediate;		/* If the function has to run at startup */
		int at;				/* at which time, the query has to be launched */
		int min;			/* minutes when at decripted */
		bool runifover;		/* run immediately if the 'At' hour is already passed */
	} Every;
#ifdef INOTIFY
	struct _Look4Changes {
		union CSection *next;
		enum _tp_msec section_type;
		pthread_t thread;	/* Unused */
		const char *uid;	/* Unique identifier */
		int h;				/* hash code for this id */
		const char *topic;	/* Topic to wait data from */
		bool disabled;
		bool keep;			/* Unimplemented yet : should be used to re-watch if the dir belongs to an unmountable FS */
		bool retained;
		int padding_sample;	/* not used */
		const char *failfunc;
		int failfuncid;
		const char *funcname;	/* Function to be called */
		int funcid;			/* Function id in Lua registry */
		const char *dir;	/* In fact, can be used on file as well */
		uint32_t flags;		/* What to survey */
		int wd;				/* watch discriptor */
	} Look4Changes;
#endif
	struct _REST {	/* Must be aligned with _Every */
		union CSection *next;
		enum _tp_msec section_type;
		pthread_t thread;	/* Child to handle this section */
		const char *uid;	/* Unique identifier */
		int h;				/* hash code for this id */
		const char *url;	/* URL to query */
		bool disabled;
		bool keep;
		bool padding_retaining;
		int sample;			/* Delay b/w 2 queries */
		const char *failfunc;
		int failfuncid;
		const char *funcname;	/* Function to be called */
		int funcid;			/* Function id in Lua registry */
		bool immediate;		/* If the function has to run at startup */
		int at;				/* at which time, the query has to be launched */
		int min;			/* minutes when at decripted */
		bool runifover;		/* run immediately if the 'At' hour is already passed */
	} REST;
	struct _Meteo {
		union CSection *next;
		enum _tp_msec section_type;
		pthread_t thread;	/* Child to handle this section */
		const char *uid;	/* Unique identifier */
		int h;				/* hash code for this id */
		const char *topic;	/* Root of the topics to publish to */
		bool disabled;
		bool keep;
		bool padding_retaining;
		int sample;			/* Delay b/w 2 queries */
		const char *failfunc;
		int failfuncid;
		const char *City;	/* CityName,Country to query */
		const char *Units;	/* Result's units */
		const char *Lang;	/* Result's language */
	} Meteo;
	struct _RTSCmd {
		union CSection *next;
		enum _tp_msec section_type;
		pthread_t thread;	/* Child to handle this section */
		const char *uid;	/* Unique identifier */
		int h;				/* hash code for this id */
		const char *topic;	/* Topic to wait data from */
		bool disabled;
		bool keep;
		bool padding_retaining;
		int padding_sample;	/* not used */
		const char *failfunc;
		int failfuncid;
		uint32_t did;		/* ID corresponding to this device */
	} RTSCmd;
	struct _Sht31 {
		union CSection *next;
		enum _tp_msec section_type;
		pthread_t thread;	/* Child to handle this section */
		const char *uid;	/* Unique identifier */
		int h;				/* hash code for this id */
		const char *topic;	/* Root topic */
		bool disabled;		/* this section is currently disabled */
		bool keep;			/* Stay alive in cas of failure */
		bool retained;		/* send MQTT retained message */
		int sample;
		const char *failfunc;	/* Lua function to be called in case of failure */
		int failfuncid;
		const char *device;		/* I2C device */
		uint8_t i2c_addr;	/* I2C address (default : 0x44) */
		float offset;		/* Offset to apply to the raw value */
	} Sht;
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
	char *OnOffTopic;	/* topic to enable/disable sections */
	MQTTClient client;
	bool Randomize;			/* add randome delay before FFV */
	bool Sublast;			/* Dead Publisher Detect are grouped at the end of sections list */
	union CSection *first_Sub;	/* Pointer to the first subscription */
	bool L4Cgrouped;		/* Look4Changes are all grouped */
	struct _Look4Changes *first_L4C;	/* Pointer to the first subscription */
	bool ConLostFatal;		/* Die if broker connection is lost */
	bool publishlog;		/* Publish logs */
	const char *luascript;	/* file containing Lua functions */
	const char *OwAlarm;	/* Path to 1-wire alarm directory */
	int OwAlarmSample;		/* Delay b/w 2 sample on alarm directory */
	bool OwAlarmKeep;		/* Alarm thread doesn't die in case of error */
	pthread_t OwAlarmThread;	/* Thread to handle 1-wire alarming */
		/* Single alert / notification */
	const char *SMSurl;		/* Where to send SMS */
	const char *AlertCmd;	/* External command to send alerts */
		/* Alerts by id */
	struct notification *notiflist;
	const char *RFXdevice;
} cfg;

	/* Helper functions */
extern bool verbose;

extern char *removeLF(char *);
extern char *striKWcmp( char *, const char * );
extern char *mystrdup(const char *);
#define strdup(s) mystrdup(s)

extern size_t socketreadline( int, char *, size_t);

	/* Lookup values for variables substitution
	 *	Replace all occurrences of given variables with values
	 *	Variables' name have to contains enclosing %
	 *	(f.e "%ClientID%")
	 *
	 *	The list must NULL terminated
	 */
struct _VarSubstitution {
	const char *var;	/* Variable's name */
	const char *val;	/* Value */
	size_t lvar;		/* size of the variable name (initialize to 0)*/
	size_t lval;		/* size of the value (initialize to 0)*/
};

	/* Calculate lengths */
extern void init_VarSubstitution( struct _VarSubstitution * );

	/* replacement
	 * Lookup table must has been initialized before.
	 * Resulting string is dynamically allocated.
	 */
extern char *replaceVar( const char *, struct _VarSubstitution * );

	/* Logging */
extern void publishLog( char, const char *, ...);

	/* Lua related */
#ifdef LUA
#include <lua.h>		/* Lua's Basic */
extern lua_State *L;

extern void init_Lua( const char * );
extern int findUserFunc( const char * );
extern void execUserFuncDeadPublisher( struct _DeadPublisher *, const char *, const char *);
extern void execUserFuncEvery( struct _Every * );
extern void execUserFuncREST( struct _REST *, char *);
extern void execUserFuncOutFile( struct _OutFile *, const char * );
extern bool execUserFuncFFV( struct _FFV *, float, float);

extern void executeFailFunc( union CSection *, const char * );
#endif

#endif

