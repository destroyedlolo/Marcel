/* mod_TaHoma
 *
 * Steer Overkiz' TaHoma using local API
 *
 * This file is part of Marcel project and is following the same
 * license rules (see LICENSE file)
 *
 * 24/02/2025 - LF - Creation
 */

#ifndef MOD_TAHOMA_H
#define MOD_TAHOMA_H

#include "../Marcel/Module.h"
#include "../Marcel/Section.h"

/* Custom structure to store module's configuration */
struct module_TaHoma {
	struct Module module;

	bool randomize;		/* Randomize probes to avoid they are all launched at the same time */
	float defaultsampletime;
};

extern struct module_TaHoma mod_TaHoma;

/* Section identifiers */
enum {
	ST_TAHOMA = 0,
	ST_DEVICE,
	ST_STATE	/* Not really a section but used to validate options */
};

		/* Gateway's */
struct section_TaHoma {
	struct Section section;

	const char *hostname;
	const char *ip;
	const char *token;
	uint16_t port;

	bool unsafe;	/* Don't verify SSL chain */
};

	/* Device defintion */
struct State_definition;
struct section_Device {
	struct Section section;

	const char *TaHoma;	/* Gateway */
	const char *url;	/* Probe's location */

	struct State_definition *States;	/* States to extract */
};

	/* Query a state */
struct State_definition {
	struct State_definition *next;

	const char *state;	/* State's name */

		/* options */
	bool disabled;			/* this section is currently disabled */
	bool dontSimulate;		/* disabled if we're in simulation mode */

		/* MQTT */
	const char *topic;
	bool retained;			/* send MQTT retained message */

		/* Lua user function
		 */
	const char *funcname;	/* User function to call on data arrival */
	int funcid;				/* Function id in Lua registry */
};

#endif
