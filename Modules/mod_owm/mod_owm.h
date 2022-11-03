/* mod_owm
 *
 * Weather forecast using OpenWeatherMap
 *  
 * This file is part of Marcel project and is following the same
 * license rules (see LICENSE file)
 *
 * 03/11/2022 - LF - Move to V8
 */

#ifndef MOD_OWM_H
#define MOD_OWM_H

#include "../Marcel/Module.h"
#include "../Marcel/Section.h"

/* Custom structure to store module's configuration */
struct module_owm {
	struct Module module;

	const char *apikey;
};

/* Section identifiers */
enum {
	SM_DAILY= 0,
	SM_3H
};

struct section_OWMQuery {
	struct Section section;

	const char *city;	/* CityName,Country (mandatory) */
	const char *units;	/* metric (default), imperial or Standard */
	const char *lang;	/* Language (almost) */
};

#define DEFAULT_WEATHER_SAMPLE 600

#endif
