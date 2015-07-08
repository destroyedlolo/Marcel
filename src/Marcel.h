/* Marcel.h
 *
 * Shared definition
 *
 * 08/07/2015	- LF - start v2.0 - make source modular
 */

#ifndef MARCEL_H
#define MARCEL_H

#include <pthread.h>
#include <MQTTClient.h> /* PAHO library needed */ 

#define VERSION "2.0"
#define DEFAULT_CONFIGURATION_FILE "/usr/local/etc/Marcel.conf"
#define MAXLINE 1024	/* Maximum length of a line to be read */
#define BRK_KEEPALIVE 60	/* Keep alive signal to the broker */

	/* Helper functions */
extern int debug;

extern char *removeLF(char *);
extern char *striKWcmp( char *, const char * );
extern char *mystrdup(const char *);
#define strdup(s) mystrdup(s)

extern size_t socketreadline( int, char *, size_t);

	/* Configuration / context */
enum _tp_msec {
	MSEC_INVALID =0,	/* Ignored */
	MSEC_FFV,			/* File String Value */
	MSEC_FREEBOX,		/* FreeBox */
	MSEC_UPS			/* UPS */
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
};

struct Config {
	union CSection *sections;
	const char *Broker;
	MQTTClient client;
} cfg;

#endif
