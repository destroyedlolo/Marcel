/* mod_TaHoma
 *
 * Steer Overkiz' TaHoma using local API
 *
 * This file is part of Marcel project and is following the same
 * license rules (see LICENSE file)
 *
 * 24/02/2026 - LF - Creation
 * 25/03/2026 - LF - Redesign to optimize
 */

#ifndef MOD_TAHOMA_H
#define MOD_TAHOMA_H

#include "../Marcel/Module.h"
#include "../Marcel/Section.h"
#include "../Marcel/CURL_helpers.h"

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
	ST_PROBE,
	ST_ACUATOR,

		/* Not really sections but used to validate options */
	ST_STATE,
	ST_EXPECTATION
};

		/* Gateway's */
struct Expectation_definition;
struct section_TaHoma {
	struct Section section;

	const char *hostname;
	const char *ip;
	const char *token;
	uint16_t port;
	bool unsafe;	/* Don't verify SSL chain */

	char *baseurl;	/* How to reach the taHoma */
	size_t url_len;	/* To avoid to recompute the url length */

	struct Expectation_definition *expectations;
};

struct Expectation_definition {
	struct Expectation_definition *next;

	const char *uid;	/* Identifier */

	const char *event;	/* Event's name */
	const char *url;	/* Device's url */
	const char *state;	/* State's name */

		/* options */
	bool disabled;			/* this section is currently disabled */
	bool dontSimulate;		/* disabled if we're in simulation mode */

		/* MQTT */
	const char *topic;
	bool retained;			/* send MQTT retained message */

		/* Lua user function */
	const char *funcname;	/* User function to call on data arrival */
	int funcid;				/* Function id in Lua registry */
};

	/* Device definition */
struct section_Device {
	struct Section section;

	const char *TaHoma;		/* Gateway */
	struct section_TaHoma *gateway;		/* To whish TaHoma we are connected */
	const char *url;		/* Probe's location */
	const char *target_url;	/* URL formating is done only once at startup */
};

	/* Probe definition */
struct State_definition;
struct section_Probe {
	struct section_Device device;
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

		/* Lua user function */
	const char *funcname;	/* User function to call on data arrival */
	int funcid;				/* Function id in Lua registry */
};

extern void *processProbe(void *);
extern void *processEvent(void *);
extern bool callAPIDev(struct section_Device *, const char *, const char *, struct MemoryStruct *);
extern bool callAPIGW(struct section_TaHoma *, const char *, const char *, struct MemoryStruct *);

#endif
