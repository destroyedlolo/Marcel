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

	bool inerror;			/* This section is in error */

		/* options */
	bool disabled;			/* this section is currently disabled */
	bool dontSimulate;		/* disabled if we're in simulation mode */

		/* MQTT */
	const char *topic;
	bool retained;			/* send MQTT retained message */
	bool (*processMsg)(struct Section *, const char *, char *);

		/* options that may or may not used in this kind of section */
	bool keep;				/* Stay alive in cas of failure */
	double sample;			/* sample rate or delay */
	bool immediate;			/* run it immediately */
	bool quiet;				/* this section will not produce log */

		/* D2 documentation related */
	const char *desc;	/* Long "tool tips" comment */
	const char *ecom;	/* Embedded comment */
	const char *group;	/* Group objects */

	const char *fqid;	/* Fully qualified identifier */

		/* Lua user function
		 * (only applicable to some sections)
		 */
	const char *funcname;	/* User function to call on data arrival */
	int funcid;				/* Function id in Lua registry */
	const char *arg;		/* Arguments to be passed to the function */

		/* Callback */
	void (*postconfInit)(struct Section *);	/* Initialisation to be done after configuration phase */
	int (*publishCustomFigures)(struct Section *);	/* Publish figures specific to this section kind */
	void (*gend2)(struct Section *);	/* Generate D2 documentation */
};

extern struct Section *sections,	/* First section */
					*last_section;	/* the last one */

extern struct Section *findSectionByName(const char *name);
extern void initSection(struct Section *sec, int8_t module_id, uint8_t section_id, const char *name, const char *kind);
extern void SectionOnOff(struct Section *, bool);
extern void SectionError(struct Section *, bool);
extern void publishSectionStatus(struct Section *);
extern bool isDisabled(struct Section *);

extern void genGeneralD2(struct Section *);
extern const char *getFqID(struct Section *);

#endif
