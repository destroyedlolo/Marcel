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
 * 31/08/2016	- LF - Add "keep" option
 * 01/09/2016	- LF - Add publishLog() function
 * 18/08/2018	- LF - Add absolute time to Every
 * 25/11/2018	- LF - Add safe85 to FFV
 * 09/04/2021	- LF - Add Retained
 * 	---
 * 06/09/2022	- LF - switch to v8
 */

#ifndef MARCEL_H
#define MARCEL_H

#include <stdbool.h>
#include <MQTTClient.h> /* PAHO library needed */

/* **
 * Configuration
 *
 * A structure is not really needed but it makes the code cleaner
 * **/

#define MAXLINE 1024

extern struct Config {
	bool verbose;
	bool debug;

	bool sublast;	/* Subscribing sections are at the end */

		/* MQTT related */
	const char *Broker;		/* Broker's URL */
	const char *ClientID;	/* Marcel client id : must be unique among a broker clients */
	MQTTClient client;
} cfg;

extern struct _VarSubstitution {
	const char *var;	/* Variable's name */
	const char *val;	/* Value */
	int h;				/* hash of the variable's name */
	size_t lvar;		/* size of the variable name (initialize to 0)*/
	size_t lval;		/* size of the value (initialize to 0)*/
} vslookup[];	/* used by both mod_core (initializing) and Marcel (command line reading) */


extern void init_VarSubstitution( struct _VarSubstitution *tbl );
extern char *replaceVar( const char *, struct _VarSubstitution *);
extern bool setSubstitutionVar(struct _VarSubstitution *vars, const char *name, const char *val, bool freeval);
extern const char *getSubstitutionVar( struct _VarSubstitution *lookup, const char *name );

	/* **
	 * Utilities
	 * **/

	/* As Lua is used in EVERY module, this pointer is defined globally */
#ifdef LUA
extern struct module_Lua *mod_Lua;
#else
extern void *mod_Lua;
#endif

extern void publishLog( char l, const char *msg, ...);
extern const char *striKWcmp( const char *s, const char *kw );
extern char *removeLF(char *s);
extern int chksum(const char *s);
extern char *stradd(char *p, const char *s, bool addspace);
extern size_t socketreadline( int fd, char *l, size_t sz);

#endif

