/* mod_sht31
 *
 * Exposes SHT31 (Temperature/humidity probe) figures.
 *
 * This file is part of Marcel project and is following the same
 * license rules (see LICENSE file)
 *
 * 09/10/2022 - LF - First version
 */

#include "mod_sht31.h"	/* module's own stuffs */
#ifdef LUA
#	include "../mod_Lua/mod_Lua.h"
#endif
#include "../Marcel/MQTT_tools.h"

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>

static struct module_sht31 mod_sht31;

enum {
	ST_SHT31= 0,
};

static void *processSHT31(void *actx){
	struct section_sht31 *s = (struct section_sht31 *)actx;

		/* Sanity checks */
	if(!s->section.topic){
		publishLog('F', "[%s] Topic must be set. Dying ...", s->section.uid);
		pthread_exit(0);
	}

	if(!s->section.sample){
		publishLog('E', "[%s] Sample time can't be 0. Dying ...", s->section.uid);
		pthread_exit(0);
	}

	if(!s->device){
		publishLog('E', "[%s] I2c device must be set. Dying ...", s->section.uid);
		pthread_exit(0);
	}

		/* Handle Lua functions */
#ifdef LUA
	struct module_Lua *mod_Lua = NULL;
	uint8_t mod_Lua_id = findModuleByName("mod_Lua");	/* Is mod_Lua loaded ? */
	if(mod_Lua_id != (uint8_t)-1){
		if(s->section.funcname){	/* if an user function defined ? */
			mod_Lua = (struct module_Lua *)modules[mod_Lua_id];
			if( (s->section.funcid = mod_Lua->findUserFunc(s->section.funcname)) == LUA_REFNIL ){
				publishLog('E', "[%s] configuration error : user function \"%s\" is not defined. This thread is dying.", s->section.uid, s->section.funcname);
				pthread_exit(NULL);
			}
		}
	}
#endif

		/* Build temperature topic */
	struct _VarSubstitution tempvslookup[] = {
		{ "%FIGURE%", "Temperature" },	/* MUST BE THE 1ST VARIABLE */
		{ NULL }
	};
	init_VarSubstitution( tempvslookup );
	const char *temptopic = replaceVar( s->section.topic, tempvslookup );

		/* Build Humidity topic */
	struct _VarSubstitution humvslookup[] = {
		{ "%FIGURE%", "Humidity" },	/* MUST BE THE 1ST VARIABLE */
		{ NULL }
	};
	init_VarSubstitution( humvslookup );
	const char *humtopic = replaceVar( s->section.topic, humvslookup );

	publishLog('I', "[%s] Temperature : '%s'", s->section.uid, temptopic);
	publishLog('I', "[%s] Humidity : '%s'", s->section.uid, humtopic);

	for(bool first=true;; first=false){	/* Infinite publishing loop */
		if(s->section.disabled){
#ifdef DEBUG
			if(cfg.debug)
				publishLog('d', "[%s] is disabled", s->section.uid);
#endif
		} else if( !first || s->section.immediate ){	/* processing */
			int fd;
			if((fd = open(s->device, O_RDWR)) < 0){
				publishLog('E', "[%s] i2c-bus open(%s) : %s", s->section.uid, s->device, strerror(errno));
				if(s->section.keep)
					continue;
				else
					break;
			}
			ioctl(fd, I2C_SLAVE, s->i2c_addr);	/* Which slave to talk to */

			char data[6];

			data[0] = 0x2c;	/* clock stretching */
			data[1] = 0x06; /* high repeatability measurement command */
			if(write(fd, data, 2) != 2){
				close(fd);
				publishLog('E', "[%s] write I/O error", s->section.uid);
			} else {

/* Thanks to clock stretching, it's not needed as the reading will be possible
 * only after acquisition.
 * sleep(1); 
 */

				if(read(fd, data, 6) != 6){	/* Read the result */
					close(fd);
					publishLog('E', "[%s] I/O error", s->section.uid);
				} else {	/* Conversion formulas took from SHT31 datasheet */
					close(fd);	/* Release the bus as soon as possible */

					double valt, valh;
					char t[8];

					valt = (((data[0] * 256) + data[1]) * 175.0) / 65535.0  - 45.0;
					valt += s->offset;
					valh = (((data[3] * 256) + data[4])) * 100.0 / 65535.0;
					valh += s->offsetH;

					bool ret = true;
#ifdef LUA
					if(mod_Lua_id != (uint8_t)-1){
						if(s->section.funcid != LUA_REFNIL){	/* if an user function defined ? */
							mod_Lua->lockState();
							mod_Lua->pushFunctionId( s->section.funcid );
							mod_Lua->pushString( s->section.uid );
							mod_Lua->pushNumber( valt );
							mod_Lua->pushNumber( valh );
							if(mod_Lua->exec(3, 1)){
								publishLog('E', "[%s] SHT31 : %s", s->section.uid, mod_Lua->getStringFromStack(-1));
								mod_Lua->pop(1);	/* pop error message from the stack */
								mod_Lua->pop(1);	/* pop NIL from the stack */
							} else
								ret = mod_Lua->getBooleanFromStack(-1);	/* Check the return code */
							mod_Lua->unlockState();
						}
					}
#endif

					if(ret){
						sprintf(t, "%.2f", valt);
						mqttpublish(cfg.client, temptopic, strlen(t), t, 0);

						sprintf(t, "%.2f", valh);
						mqttpublish(cfg.client, humtopic, strlen(t), t, 0);
					}
				}
			}
		}

		struct timespec ts;
		ts.tv_sec = (time_t)s->section.sample;
		ts.tv_nsec = (unsigned long int)((s->section.sample - (time_t)s->section.sample) * 1e9);

		nanosleep( &ts, NULL );
	}

	pthread_exit(0);
}

static enum RC_readconf readconf(uint8_t mid, const char *l, struct Section **section ){
	const char *arg;

	if((arg = striKWcmp(l,"*SHT31="))){	/* Starting a section definition */
		if(findSectionByName(arg)){
			publishLog('F', "Section '%s' is already defined", arg);
			exit(EXIT_FAILURE);
		}

		struct section_sht31 *nsection = malloc(sizeof(struct section_sht31));	/* Allocate a new section */
		initSection( (struct Section *)nsection, mid, ST_SHT31, strdup(arg), "SHT31");	/* Initialize shared fields */

		nsection->device = NULL;
		nsection->i2c_addr = 0x44;
		nsection->offset = 0.0;
		nsection->offsetH = 0.0;

		if(cfg.verbose)	/* Be verbose if requested */
			publishLog('C', "\tEntering section sht31 '%s' (%04x)", nsection->section.uid, nsection->section.id);

		*section = (struct Section *)nsection;	/* we're now in a section */
		return ACCEPTED;
	} else if(*section){
		if((arg = striKWcmp(l,"Device="))){
			acceptSectionDirective(*section, "Device=");
			assert(( (*(struct section_sht31 **)section)->device = strdup(arg) ));

			if(cfg.verbose)	/* Be verbose if requested */
				publishLog('C', "\t\tDevice : '%s'", (*(struct section_sht31 **)section)->device);
			return ACCEPTED;
		} else if((arg = striKWcmp(l,"Address="))){
			acceptSectionDirective(*section, "Address=");
			(*(struct section_sht31 **)section)->i2c_addr = strtoul(arg, NULL, 0);

			if(cfg.verbose)	/* Be verbose if requested */
				publishLog('C', "\t\tI2c address: 0x%02x", (*(struct section_sht31 **)section)->i2c_addr);
			return ACCEPTED;
		} else if((arg = striKWcmp(l,"Offset="))){
			acceptSectionDirective(*section, "Offset=");
			(*(struct section_sht31 **)section)->offset = strtof(arg, NULL);

			if(cfg.verbose)	/* Be verbose if requested */
				publishLog('C', "\t\tOffset: %f", (*(struct section_sht31 **)section)->offset);
			return ACCEPTED;
		} else if((arg = striKWcmp(l,"OffsetH="))){
			acceptSectionDirective(*section, "OffsetH=");
			(*(struct section_sht31 **)section)->offsetH = strtof(arg, NULL);

			if(cfg.verbose)	/* Be verbose if requested */
				publishLog('C', "\t\tOffsetH: %f", (*(struct section_sht31 **)section)->offsetH);
			return ACCEPTED;
		}
	}

	return REJECTED;
}

static bool mh_acceptSDirective( uint8_t sec_id, const char *directive ){
	if(sec_id == ST_SHT31){
		if( !strcmp(directive, "Disabled") )
			return true;	/* Accepted */
		else if( !strcmp(directive, "Immediate") )
			return true;	/* Accepted */
		else if( !strcmp(directive, "Keep") )
			return true;	/* Accepted */
		else if( !strcmp(directive, "Sample=") )
			return true;	/* Accepted */
		else if( !strcmp(directive, "Topic=") )
			return true;	/* Accepted */
		else if( !strcmp(directive, "Func=") )
			return true;	/* Accepted */
		else if( !strcmp(directive, "Device=") )
			return true;	/* Accepted */
		else if( !strcmp(directive, "Address=") )
			return true;	/* Accepted */
		else if( !strcmp(directive, "Offset=") )
			return true;	/* Accepted */
		else if( !strcmp(directive, "OffsetH=") )
			return true;	/* Accepted */
	}

	return false;
}

ThreadedFunctionPtr mh_getSlaveFunction(uint8_t sid){
	if(sid == ST_SHT31)
		return processSHT31;

	/* No slave for Echo : it will process incoming messages */
	return NULL;
}

void InitModule( void ){
	initModule((struct Module *)&mod_sht31, "mod_sht31");	/* Identify the module */

		/* Initialize callbacks
		 * It's MANDATORY that all callbacks are initialised
		 */
	mod_sht31.module.readconf = readconf;
	mod_sht31.module.acceptSDirective = mh_acceptSDirective;
	mod_sht31.module.getSlaveFunction = mh_getSlaveFunction;

	registerModule( (struct Module *)&mod_sht31 );	/* Register the module */
}
