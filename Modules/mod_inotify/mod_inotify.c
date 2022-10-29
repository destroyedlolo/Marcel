/* mod_inotify
 *
 * Notification for file system changes
 *
 * This file is part of Marcel project and is following the same
 * license rules (see LICENSE file)
 *
 * 29/10/2022 - LF - Emancipate from Marcel.c as a standalone module
 */

#include "mod_inotify.h"	/* module's own stuffs */
#include "../Marcel/MQTT_tools.h"

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>

#include <sys/inotify.h>
#include <dirent.h>

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

static void *handleNotification(void *amod){
	struct module_inotify *mod_inotify = (struct module_inotify *)amod;

	char buf[ INOTIFY_BUF_LEN ];

	for(;;){
		ssize_t len = read(mod_inotify->infd, buf, INOTIFY_BUF_LEN);
		const struct inotify_event *event;

		if(len == -1){
			if(errno == EAGAIN)	/* Nothing to read */
				continue;
			publishLog('F', "[mod_inotify] read() : %s", strerror(errno));
			exit(EXIT_FAILURE);
		}

		for(char *ptr = buf; ptr < buf + len; ptr += sizeof(struct inotify_event) + event->len){
			event = (const struct inotify_event *) ptr;

			for(struct section_Look4Change *s = mod_inotify->first_section; s; s = (struct section_Look4Change *)s->section.next){
				if(s->section.id != mod_inotify->first_section->section.id){	/* Not a L4C section anymore */
					if(mod_inotify->grouped)
						break;		/* List is over */
					else
						continue;	/* skip to next section */
				} else {
					if( event->wd == s->wd ){	/* Event's matching */
						char *amsg=NULL;
						if(event->mask & IN_ACCESS)
							amsg = stradd( amsg, ",ACCESS", true);
						if(event->mask & IN_ATTRIB)
							amsg = stradd( amsg, ",ATTRIB", true);
						if(event->mask & IN_CLOSE_NOWRITE)
							amsg = stradd( amsg, ",CLOSE_NOWRITE", true);
						if(event->mask & IN_CLOSE_WRITE)
							amsg = stradd( amsg, ",CLOSE_WRITE", true);
						if(event->mask & IN_CREATE)
							amsg = stradd( amsg, ",CREATE", true);
						if(event->mask & IN_DELETE)
							amsg = stradd( amsg, ",DELETE", true);
						if(event->mask & IN_DELETE_SELF)
							amsg = stradd( amsg, ",DELETE_SELF", true);
						if(event->mask & IN_IGNORED)
							amsg = stradd( amsg, ",IGNORED", true);
						if(event->mask & IN_ISDIR)
							amsg = stradd( amsg, ",ISDIR", true);
						if(event->mask & IN_MODIFY)
							amsg = stradd( amsg, ",MODIFY", true);
						if(event->mask & IN_MOVE_SELF)
							amsg = stradd( amsg, ",MOVE_SELF", true);
						if(event->mask & IN_MOVED_FROM)
							amsg = stradd( amsg, ",MOVED_FROM", true);
						if(event->mask & IN_MOVED_TO)
							amsg = stradd( amsg, ",MOVED_TO", true);
						if(event->mask & IN_OPEN)
							amsg = stradd( amsg, ",OPEN", true);
						if(event->mask & IN_Q_OVERFLOW)
							amsg = stradd( amsg, ",Q_OVERFLOW", true);
						if(event->mask & IN_UNMOUNT)
							amsg = stradd( amsg, ",UNMOUNT", true);

						size_t sz = event->len + strlen(amsg) + 2;
						char msg[sz+1];
						sprintf(msg, "%s:%s", event->len ? event->name : "", amsg);
						free(amsg);
						mqttpublish(cfg.client, s->section.topic, sz, msg, s->section.retained);
					}
				}
			}
		}
	}
}

static void startNotif( uint8_t mid ){
	if(mod_inotify.first_section){
		mod_inotify.infd = inotify_init();
		if( mod_inotify.infd < 0 ){
			perror("inotify_init()");
			exit(EXIT_FAILURE);
		}

		/* Sanity checks */
		for(struct section_Look4Change *s = mod_inotify.first_section; s; s = (struct section_Look4Change *)s->section.next){
			if(s->section.id != mod_inotify.first_section->section.id){	/* Not a L4C section anymore */
				if(mod_inotify.grouped)
					break;		/* List is over */
				else
					continue;	/* skip to next section */
			}
			
			if(!s->dir){
				publishLog('E', "[%s] No directory defined (On= missing ?)", s->section.uid);
				exit(EXIT_FAILURE);
			}
			if(!s->flags){
				publishLog('E', "[%s] Nothing to look for (For= missing ?)", s->section.uid);
				exit(EXIT_FAILURE);
			}

			if(!s->section.topic){
				publishLog('E', "[%s] No Topic defined", s->section.uid);
				exit(EXIT_FAILURE);
			}

			if((s->wd = inotify_add_watch(mod_inotify.infd, s->dir, s->flags)) < 0){
				const char *emsg = strerror(errno);
				publishLog('E', "[%s] %s : %s", s->section.uid, s->dir, emsg);
			}
		}

			/* Launch notification thread */
		pthread_attr_t thread_attr;
		assert(!pthread_attr_init (&thread_attr));
		assert(!pthread_attr_setdetachstate (&thread_attr, PTHREAD_CREATE_DETACHED));

		if(pthread_create( &(mod_inotify.thread), &thread_attr, handleNotification, &mod_inotify) < 0){
			publishLog('F', "Can't create 1-wire alert reader thread");
			exit(EXIT_FAILURE);
		}
	} else if(cfg.verbose)
		publishLog('I', "No file system notification configured");
}

void InitModule( void ){
	mod_inotify.module.name = "mod_inotify";	/* Identify the module */

	mod_inotify.module.readconf = readconf;
	mod_inotify.module.acceptSDirective = acceptSDirective;
	mod_inotify.module.getSlaveFunction = NULL;
	mod_inotify.module.postconfInit = startNotif;

	mod_inotify.grouped = false;
	mod_inotify.first_section = NULL;

	register_module( (struct Module *)&mod_inotify );	/* Register the module */
}
