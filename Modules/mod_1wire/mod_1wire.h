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
};

/* Float value exposed as a file
 * 
 * Mostly for 1-wire exposed probes but can be used (without 1-wire only options)
 * with any value exposed as a file. The function callback can be
 * used to parse complex files
 */
struct section_FFV {
	struct Section section;

	const char *file;	/* File containing the data to read */
	const char *latch;	/* Related latch file (optional) */
	float offset;		/* Offset to apply to the raw value */
	bool safe85;		/* Ignores underpowered temperature probes */
};

#endif
