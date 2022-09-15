/* Section.c
 * 	section managements
 *
 * 	15/09/2022 - LF - First version
 */

#include "Section.h"

struct Section *sections = NULL;

struct Section *findSectionByName(const char *name){
	int h = chksum(name);

	for(struct Section *s = sections; s; s = s->next){
		if(s->h == h && !strcmp(name, s->uid))
			return s;
	}

	return NULL;	/* Not found */
}
