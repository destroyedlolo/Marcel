/* mod_1wire
 *
 * Handle 1-wire probes.
 * (also suitable for other exposed as a file values)
 *
 * This file is part of Marcel project and is following the same
 * license rules (see LICENSE file)
 *
 * 11/10/2022 - LF - First version
 */

#ifndef MOD_1WIRE_H
#define MOD_1WIRE_H

#include "../Marcel/Module.h"
#include "../Marcel/Section.h"

/* Custom structure to store module's configuration */
struct module_1wire {
	struct Module module;

	bool randomize;		/* Randomize probes to avoid they are all launched at the same time */
	float defaultsampletime;

		/* Alarm handling */
	const char *OwAlarm;	/* 1w alarm directory */
	float OwAlarmSample;	/* Delay b/w 2 sample on alarm directory */
	bool OwAlarmKeep;		/* Alarm thread doesn't die in case of error */
};


/* Fields common to 1-wire structures */
struct OwCommon {
	struct Section section;

	const char *file;		/* File containing the data to read */
	const char *failfunc;	/* User function to call on data arrival */
	int failfuncid;			/* Function id in Lua registry */
};

/* Float value exposed as a file
 * 
 * Mostly for 1-wire exposed probes but can be used (without 1-wire only options)
 * with any value exposed as a file. The function callback can be
 * used to parse complex files
 */
struct section_FFV {
	struct OwCommon common;

	float offset;			/* Offset to apply to the raw value */
	bool safe85;			/* Ignores underpowered temperature probes */
};

extern void *processFFV(void *);

/* Alert driven probe
 *
 * This section kind has been mostly made for PIOs related alert.
 * Consequently, temperature related directive are not applicable.
 * It's up to the Lua user function to implement calibration and
 * under power situation if used for temperature alerting.
 */

struct section_1wAlarm {
	struct OwCommon common;

	const char *initfunc;	/* Initialisation function */
	const char *latch;		/* Related latch file (optional) */
};

extern void start1WAlarm( void );
#endif
