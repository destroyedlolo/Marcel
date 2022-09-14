/* Section.h
 * 	Common section definition
 * 	
 * 14/09/2022 - LF - First version
 */

#ifndef SECTION_H
#define SECTION_H

#include <stdint.h>
#include <pthread.h>

struct Section {
		/* Section technicals */
	struct Section *next;	/* next section */
	uint16_t id;			/* section identifier */
	const char *uid;		/* unique identifier (name) */
	int h;					/* hash code for this id */
	void (*func)(void *);	/* function to handle this section */
	pthread_t thread;		/* Child to handle this section */

		/* MQTT */
	const char *topic;
	bool retained;			/* send MQTT retained message */

		/* options */
	bool disabled;			/* this section is currently disabled */

		/* options that may or may not used in this kind of section */
	bool keep;				/* Stay alive in cas of failure */
	int sample;				/* sample rate */

		/* Lua callbacks called in case of failure */
	const char *failfunc;
	int failfuncid;
};

#endif
