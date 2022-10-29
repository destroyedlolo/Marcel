/* mod_inotify
 *
 * Notification for filesystem changes
 *
 * This file is part of Marcel project and is following the same
 * license rules (see LICENSE file)
 *
 * 29/10/2022 - LF - Emancipate from Marcel.c as a standalone module
 */

#include "mod_inotify.h"	/* module's own stuffs */

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>

#include <poll.h>	/* Only used for Look4Changes up to now */
#include <sys/inotify.h>
#define INOTIFY_BUF_LEN     (10 * (sizeof(struct inotify_event) + NAME_MAX + 1))

static struct module_inotify mod_inotify;

static enum RC_readconf readconf(uint8_t mid, const char *l, struct Section **section ){
	const char *arg;

	if(!strcmp(l, "LookForChangesGrouped")){
		if(*section){
			publishLog('F', "LookForChangesGrouped can't be part of a section");
			exit(EXIT_FAILURE);
		}

		mod_inotify.grouped = true;

		if(cfg.verbose)
			publishLog('C', "\tLookForChange section are grouped");

		return ACCEPTED;
	} else if((arg = striKWcmp(l,"*LookForChanges="))){
		if(findSectionByName(arg)){
			publishLog('F', "Section '%s' is already defined", arg);
			exit(EXIT_FAILURE);
		}

		struct section_Look4Change *nsection = malloc(sizeof(struct section_Look4Change));	/* Allocate a new section */
		initSection( (struct Section *)nsection, mid, SI_L4C, strdup(arg));	/* Initialize shared fields */

		nsection->dir = NULL;
		nsection->flags = 0;
	
		if(cfg.verbose)	/* Be verbose if requested */
			publishLog('C', "\tEntering LookForChanges section '%s' (%04x)", nsection->section.uid, nsection->section.id);

		*section = (struct Section *)nsection;	/* we're now in a section */
		mod_inotify.first_section = nsection;	/* Add it in the module list as well */
		return ACCEPTED;
	} else if(*section){
		if((arg = striKWcmp(l,"On="))){
			acceptSectionDirective(*section, "On=");
			assert(( (*(struct section_Look4Change **)section)->dir = strdup(arg) ));

			if(cfg.verbose)	/* Be verbose if requested */
				publishLog('C', "\t\tOn : '%s'", (*(struct section_Look4Change **)section)->dir);
			return ACCEPTED;
		} else if((arg = striKWcmp(l,"For="))){
			acceptSectionDirective(*section, "For=");

			if(cfg.verbose)
				publishLog('C', "\t\tFor :");
	
			for( char *t = strtok((char *)arg, " \t" ); t; t = strtok( NULL, " \t" ) ){
				if(!strcasecmp( t, "create" )){
					(*(struct section_Look4Change **)section)->flags |=  IN_CREATE | IN_MOVED_TO;
					if(cfg.verbose)
						publishLog('C', "\t\t\tcreate");
				} else if(!strcasecmp( t, "remove" )){
					(*(struct section_Look4Change **)section)->flags |=  IN_DELETE | IN_MOVED_FROM;
					if(cfg.verbose)
						publishLog('C', "\t\t\tremove");
				} else if(!strcasecmp( t, "modify" )){
					(*(struct section_Look4Change **)section)->flags |=  IN_ATTRIB | IN_CLOSE_WRITE;
					if(cfg.verbose)
						publishLog('C', "\t\t\tmodify");
				}
			}
			return ACCEPTED;
		}
	}

	return REJECTED;
}

static bool acceptSDirective( uint8_t sec_id, const char *directive ){
	if(sec_id == SI_L4C){
		if( !strcmp(directive, "Disabled") )
			return true;	/* Accepted */
		else if( !strcmp(directive, "Retained") )
			return true;	/* Accepted */
		else if( !strcmp(directive, "Topic=") )
			return true;	/* Accepted */
		else if( !strcmp(directive, "Func=") )
			return true;	/* Accepted */
		else if( !strcmp(directive, "Dir=") )
			return true;	/* Accepted */
		else if( !strcmp(directive, "On=") )
			return true;	/* Accepted */
		else if( !strcmp(directive, "For=") )
			return true;	/* Accepted */
	}

	return false;
}

void InitModule( void ){
	mod_inotify.module.name = "mod_inotify";	/* Identify the module */

	mod_inotify.module.readconf = readconf;
	mod_inotify.module.acceptSDirective = acceptSDirective;
	mod_inotify.module.getSlaveFunction = NULL;
	mod_inotify.module.postconfInit = NULL;

	mod_inotify.grouped = false;
	mod_inotify.first_section = NULL;

	register_module( (struct Module *)&mod_inotify );	/* Register the module */
}
