/* Section.c
 * 	section managements
 *
 * This file is part of Marcel project and is following the same
 * license rules (see LICENSE file)
 *
 * 	15/09/2022 - LF - First version
 */

#include "Section.h"

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
 * @brief Initialize mandatory (only) field of a structure
 *
 * @param section Structure to initialize
 * @param module_id Module unique identifier
 * @param section_id Section's unique (to this module) identifier
 * @param name Section's name (its pointer is only copied)
 */
void initSection( struct Section *section, int8_t module_id, uint8_t section_id, const char *name){
	assert( name );	/* as most of the time allocated by a strdup() */

	section->next = sections;	/* Replace the head */
	sections = section;
	section->id = section_id << 8 | module_id;
	section->uid = name;
	section->h = chksum(name);

	section->thread = 0;
	section->disabled = false;
	section->immediate = false;

	section->topic = NULL;
	section->retained = false;
	section->processMsg = NULL;

	section->keep = false;
	section->sample = 0;

	section->funcname = NULL;
	section->funcid = LUA_REFNIL;
}
