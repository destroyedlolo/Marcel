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

#define VERSION "2.0"
#define DEFAULT_CONFIGURATION_FILE "/usr/local/etc/Marcel.conf"
#define MAXLINE 1024	/* Maximum length of a line to be read */
#define BRK_KEEPALIVE 60	/* Keep alive signal to the broker */

	/* Configuration / context */
enum _tp_msec {
	MSEC_INVALID =0,	/* Ignored */
	MSEC_FFV,			/* File String Value */
	MSEC_FREEBOX,		/* FreeBox */
	MSEC_UPS,			/* UPS */
	MSEC_DEADPUBLISHER	/* alarm on missing MQTT messages */
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
		pthread_t thread;
		const char *topic;
	} common;
	struct _FFV {
		union CSection *next;
		enum _tp_msec section_type;
		int sample;
		pthread_t thread;
		const char *topic;
		const char *file;
	} FFV;
	struct _FreeBox {
		union CSection *next;
		enum _tp_msec section_type;
		int sample;
		pthread_t thread;
		const char *topic;
	} FreeBox;
	struct _UPS {
		union CSection *next;
		enum _tp_msec section_type;
		int sample;
		pthread_t thread;
		const char *topic;
		const char *section_name;
		const char *host;
		int port;
		struct var *var_list;
	} Ups;
	struct _DeadPublisher {
		union CSection *next;
		enum _tp_msec section_type;
		int sample;			/* Timeout */
		pthread_t thread;
		const char *topic;	/* Topic to wait data from */
		const char *errorid;
		int rcv;			/* Event for data receiving */
	} DeadPublisher;
};

struct Config {
	union CSection *sections;
	const char *Broker;
	MQTTClient client;
	int DPDlast;
	union CSection *first_DPD;
} cfg;

	/* Helper functions */
extern int debug;

extern char *removeLF(char *);
extern char *striKWcmp( char *, const char * );
extern char *mystrdup(const char *);
#define strdup(s) mystrdup(s)

extern size_t socketreadline( int, char *, size_t);

	/* Broker related */
int papub( const char *, int, void *, int);
#endif
