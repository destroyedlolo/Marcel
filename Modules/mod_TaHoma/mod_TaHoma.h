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

	char *baseurl;	/* How to reach the taHoma */
	size_t url_len;	/* To avoid to recompute the url length */
};

	/* Device definition */
struct section_Device {
	struct Section section;

	const char *TaHoma;		/* Gateway */
	struct section_TaHoma *gateway;		/* To whish TaHoma we are connected */
	const char *url;		/* Probe's location */
	const char *target_url;	/* URL formating is done only once at startup */
};


#endif
