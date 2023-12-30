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
#ifdef LUA
#	include "../mod_Lua/mod_Lua.h"
#endif

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>

#include <sys/inotify.h>
#include <dirent.h>

#define INOTIFY_BUF_LEN     (10 * (sizeof(struct inotify_event) + NAME_MAX + 1))

static struct module_inotify mod_inotify;

static int publishCustomFiguresL4C(struct Section *asection){
#ifdef LUA
	if(mod_Lua){
		struct section_Look4Change *s = (struct section_Look4Change *)asection;

		lua_newtable(mod_Lua->L);

		lua_pushstring(mod_Lua->L, "Directory");	/* Push the index */
		lua_pushstring(mod_Lua->L, s->dir);	/* the value */
		lua_rawset(mod_Lua->L, -3);	/* Add it in the table */

		lua_pushstring(mod_Lua->L, "FlagsV");	/* Push the index */
		lua_pushnumber(mod_Lua->L, s->flags);	/* the value */
		lua_rawset(mod_Lua->L, -3);	/* Add it in the table */
	
		lua_pushstring(mod_Lua->L, "Flags");
		lua_newtable(mod_Lua->L);
		int i = 1;
		if(s->flags & IN_CREATE){
			lua_pushstring(mod_Lua->L, "create");
			lua_rawseti(mod_Lua->L, -2, i++);
		}
		if(s->flags & IN_DELETE){
			lua_pushstring(mod_Lua->L, "remove");
			lua_rawseti(mod_Lua->L, -2, i++);
		}
		if(s->flags & IN_ATTRIB){
			lua_pushstring(mod_Lua->L, "modify");
			lua_rawseti(mod_Lua->L, -2, i++);
		}
		lua_rawset(mod_Lua->L, -3);	/* Add the sub table in the main table */

#if 0	/* Section can't be in error on themselves */
		lua_pushstring(mod_Lua->L, "Error state");			/* Push the index */
		lua_pushboolean(mod_Lua->L, s->section.inerror);	/* the value */
		lua_rawset(mod_Lua->L, -3);	/* Add it in the table */
#endif

		return 1;
	} else
#endif
	return 0;
}

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
		initSection( (struct Section *)nsection, mid, SI_L4C, strdup(arg), "LookForChanges");	/* Initialize shared fields */

		nsection->section.publishCustomFigures = publishCustomFiguresL4C;
		nsection->dir = NULL;
		nsection->flags = 0;
	
		if(cfg.verbose)	/* Be verbose if requested */
			publishLog('C', "\tEntering LookForChanges section '%s' (%04x)", nsection->section.uid, nsection->section.id);

		*section = (struct Section *)nsection;	/* we're now in a section */
		mod_inotify.first_section = nsection;	/* Add it in the module list as well */
		return ACCEPTED;
	} else if(*section){
		if((arg = striKWcmp(l,"On=")) || (arg = striKWcmp(l,"Dir="))){
			acceptSectionDirective(*section, "On=");
			assert(( (*(struct section_Look4Change **)section)->dir = strdup(arg) ));

			if(cfg.verbose)	/* Be verbose if requested */
				publishLog('C', "\t\tOn : '%s'", (*(struct section_Look4Change **)section)->dir);
			return ACCEPTED;
		} else if((arg = striKWcmp(l,"For="))){
			acceptSectionDirective(*section, "For=");

			char *amsg = NULL;
			for( char *t = strtok((char *)arg, " \t" ); t; t = strtok( NULL, " \t" ) ){
				if(!strcasecmp( t, "create" )){
					(*(struct section_Look4Change **)section)->flags |=  IN_CREATE | IN_MOVED_TO;
					if(cfg.verbose)
						amsg = stradd( amsg, ",create", true);
				} else if(!strcasecmp( t, "remove" )){
					(*(struct section_Look4Change **)section)->flags |=  IN_DELETE | IN_MOVED_FROM;
					if(cfg.verbose)
						amsg = stradd( amsg, ",remove", true);
				} else if(!strcasecmp( t, "modify" )){
					(*(struct section_Look4Change **)section)->flags |=  IN_ATTRIB | IN_CLOSE_WRITE;
					if(cfg.verbose)
						amsg = stradd( amsg, ",modify", true);
				}
			}
			if(cfg.verbose){
				char msg[strlen(amsg) + 8];
				sprintf(msg, "\t\tFor : %s", amsg);
				publishLog('C', msg);
			}
			free(amsg);
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
			mod_inotify->inerror = true;
			pthread_exit(0);
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
					if( event->wd == s->wd && !s->section.disabled ){	/* Event's matching */
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

						bool publish = true;

#ifdef LUA
						if(s->section.funcid != LUA_REFNIL){
							mod_Lua->lockState();
							mod_Lua->pushFunctionId(s->section.funcid);
							mod_Lua->pushString(s->section.uid);
							mod_Lua->pushString(event->len ? event->name : "");
							mod_Lua->pushString(amsg);

							if(mod_Lua->exec(3, 1)){
								publishLog('E', "[%s] LookForChanges : %s", s->section.uid, mod_Lua->getStringFromStack(-1));
								mod_Lua->pop(1);	/* pop error message from the stack */
								mod_Lua->pop(1);	/* pop NIL from the stack */
							} else
								publish = mod_Lua->getBooleanFromStack(-1);	/* Check the return code */
							mod_Lua->unlockState();
						}
#endif

						if(publish){
							size_t sz = event->len + strlen(amsg) + 2;
							char msg[sz+1];
							sprintf(msg, "%s:%s", event->len ? event->name : "", amsg);
							free(amsg);

							publishLog('T', "[%s] -> %s", s->section.uid, msg);
							mqttpublish(cfg.client, s->section.topic, sz, msg, s->section.retained);
						} else
							publishLog('T', "[%s] UserFunction requested not to publish", s->section.uid);
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

			/* Look for Lua functions */		
#ifdef LUA
			if(mod_Lua){
				if(s->section.funcname){
					if( (s->section.funcid = mod_Lua->findUserFunc(s->section.funcname)) == LUA_REFNIL ){
						publishLog('F', "[%s] configuration error : user function \"%s\" is not defined", s->section.uid, s->section.funcname);
						exit(EXIT_FAILURE);
					}
				}
			}
#endif
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

#ifdef LUA
static int m_inError(lua_State *L){
	struct section_dpd **s = luaL_testudata(L, 1, "LookForChanges");
	luaL_argcheck(L, s != NULL, 1, "'LookForChanges' expected");

	lua_pushboolean(L, mod_inotify.inerror);
	return 1;
}

static const struct luaL_Reg mM[] = {
	{"inError", m_inError},
	{NULL, NULL}
};
#endif

void InitModule( void ){
	initModule((struct Module *)&mod_inotify, "mod_inotify");	/* Identify the module */

	mod_inotify.module.readconf = readconf;
	mod_inotify.module.acceptSDirective = acceptSDirective;
	mod_inotify.module.postconfInit = startNotif;

	mod_inotify.grouped = false;
	mod_inotify.first_section = NULL;
	mod_inotify.inerror = false;

	registerModule( (struct Module *)&mod_inotify );	/* Register the module */

#ifdef LUA
	if(mod_Lua){ /* Is mod_Lua loaded ? */

			/* Expose shared methods */
		mod_Lua->initSectionSharedMethods(mod_Lua->L, "LookForChanges");

			/* Expose mod_1wire's own function */
		mod_Lua->exposeFunctions("mod_inotify", mM);
	}
#endif
}
