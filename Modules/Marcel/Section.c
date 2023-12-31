/* Section.c
 * 	section managements
 *
 * This file is part of Marcel project and is following the same
 * license rules (see LICENSE file)
 *
 * 	15/09/2022 - LF - First version
 */

#include "Section.h"
#include "Module.h"
#include "../mod_alert/mod_alert.h"
#include "MQTT_tools.h"

#include <assert.h>

#ifdef LUA
#	include <lualib.h>
#else
#	define LUA_REFNIL	(-1)
#endif

struct Section *sections = NULL;


/**
 * @brief Search for a section
 *
 * @param name Name of the section we are looking for
 * @return Pointer to the section's structure or NULL if not found
 */
struct Section *findSectionByName(const char *name){
	int h = chksum(name);

	for(struct Section *s = sections; s; s = s->next){
		if(s->h == h && !strcmp(name, s->uid))
			return s;
	}

	return NULL;	/* Not found */
}


/**
 * @brief Enable or disable a section
 *
 * @param section section to manage
 * @param status new status of this section (true = enabled)
 */
void SectionOnOff(struct Section *s, bool v){
	s->disabled = !v;

	publishSectionStatus(s);
}

/**
 * @brief set error status of a section
 *
 * @param section section to manage
 * @param status new error status
 */
void SectionError(struct Section *s, bool v){
	bool previous = s->inerror;
	s->inerror = v;

	if(s->inerror != previous){
		publishSectionStatus(s);

		uint8_t mod_alertID = findModuleByName("mod_alert");
		if(mod_alertID != (uint8_t)-1)
			((struct module_alert *)modules[mod_alertID])->sentAlertsCounter();
	}
}

/**
 * @brief Publish the status of a section
 *
 * @param section section to publish
 */
void publishSectionStatus(struct Section *s){
	char ttopic[ strlen(cfg.ClientID) + 9 + strlen(s->uid) ];	/* + "/Change/" */
	sprintf(ttopic, "%s/Change/%s", cfg.ClientID, s->uid);

	char t[4];
	sprintf(t, "%c,%c", s->disabled ? '0':'1', s->inerror ? '1':'0');

	mqttpublish(cfg.client, ttopic, 3, t, false);

#ifdef DEBUG
	publishLog('d', "\"%s\" status is \"%s\"", s->uid, t);
#endif
}

/**
 * @brief Initialize mandatory (only) field of a structure
 *
 * @param section Structure to initialize
 * @param module_id Module unique identifier
 * @param section_id Section's unique (to this module) identifier
 * @param name Section's name (its pointer is only copied)
 * @param kind clear text for section kind
 */
void initSection( struct Section *section, int8_t module_id, uint8_t section_id, const char *name, const char *akind){
	assert( section );
	assert( name );	/* as most of the time allocated by a strdup() */

	section->next = sections;	/* Replace the head */
	sections = section;
	section->id = section_id << 8 | module_id;
	section->kind = akind;
	section->uid = name;
	section->h = chksum(name);

	section->thread = 0;
	section->inerror = false;
	section->disabled = false;
	section->immediate = false;
	section->quiet = false;

	section->topic = NULL;
	section->retained = false;
	section->processMsg = NULL;

	section->keep = false;
	section->sample = 0;

	section->funcname = NULL;
	section->funcid = LUA_REFNIL;

	section->postconfInit = NULL;
	section->publishCustomFigures = NULL;

#ifdef LUA
	struct module_Lua *mod_Lua;
	uint8_t mod_Lua_id = findModuleByName("mod_Lua");
	if(mod_Lua_id != (uint8_t)-1){ /* Is mod_Lua loaded ? */
		mod_Lua = (struct module_Lua *)modules[mod_Lua_id];

		mod_Lua->initSectionSharedMethods(akind);	/* Declare Lua object and attach default methods */
	}
#endif

}
