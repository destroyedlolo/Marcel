/* Section.h
 * 	Common section definition
 * 	
 * This file is part of Marcel project and is following the same
 * license rules (see LICENSE file)
 *
 * 14/09/2022 - LF - First version
 */

#ifndef SECTION_H
#define SECTION_H

#include "Marcel.h"

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <pthread.h>

struct Section {
		/* Section technicals */
	struct Section *next;	/* next section */
	uint16_t id;			/* section identifier */
	const char *kind;	/* Section kind in clear text */
	const char *uid;		/* unique identifier (name) */
	int h;					/* hash code for this id */
	pthread_t thread;		/* Child to handle this section */

		/* options */
	bool disabled;			/* this section is currently disabled */

		/* MQTT */
	const char *topic;
	bool retained;			/* send MQTT retained message */
	bool (*processMsg)(struct Section *, const char *, char *);

		/* options that may or may not used in this kind of section */
	bool keep;				/* Stay alive in cas of failure */
	double sample;			/* sample rate or delay */
	bool immediate;			/* run it immediately */
	bool quiet;				/* this section will not produce log */

		/* Lua user function
		 * (only applicable to some sections)
		 */
	const char *funcname;	/* User function to call on data arrival */
	int funcid;				/* Function id in Lua registry */

		/* Callback */
	void (*postconfInit)(struct Section *);	/* Initialisation to be done after configuration phase */
	int (*publishCustomFigures)(struct Section *);	/* Publish figures specific to this section kind */
};

extern struct Section *sections;

extern struct Section *findSectionByName(const char *name);
extern void initSection( struct Section *sec, int8_t module_id, uint8_t section_id, const char *name, const char *kind);
#endif
