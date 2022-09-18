/* Section.h
 * 	Common section definition
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
	const char *uid;		/* unique identifier (name) */
	int h;					/* hash code for this id */
	pthread_t thread;		/* Child to handle this section */

		/* options */
	bool disabled;			/* this section is currently disabled */

		/* MQTT */
	const char *topic;
	bool retained;			/* send MQTT retained message */

		/* options that may or may not used in this kind of section */
	bool keep;				/* Stay alive in cas of failure */
	double sample;			/* sample rate or delay */

		/* Lua callbacks called in case of failure */
	const char *failfunc;
	int failfuncid;
};

extern struct Section *sections;

extern struct Section *findSectionByName(const char *name);
extern void initSection( struct Section *sec, int8_t module_id, uint8_t section_id, const char *name);
#endif
