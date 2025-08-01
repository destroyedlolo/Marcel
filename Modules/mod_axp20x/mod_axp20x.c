/* mod_axp20x
 *
 * Expose axp20x PMU figures
 *
 * This file is part of Marcel project and is following the same
 * license rules (see LICENSE file)
 *
 * 19/07/2025 - LF - First version
 */

#include "mod_axp20x.h"	/* module's own stuffs */
#ifdef LUA
#	include "../mod_Lua/mod_Lua.h"
#endif
#include "../Marcel/MQTT_tools.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <linux/i2c.h>

static struct module_axp20x mod_axp20x;

enum {
	ST_AXP20X= 0,
};

static enum RC_readconf readconf(uint8_t mid, const char *l, struct Section **section ){
	const char *arg;

	if((arg = striKWcmp(l,"*AXP20x="))){	/* Starting a section definition */
		if(findSectionByName(arg)){
			publishLog('F', "Section '%s' is already defined", arg);
			exit(EXIT_FAILURE);
		}

		struct section_axp20x *nsection = malloc(sizeof(struct section_axp20x));	/* Allocate a new section */
		initSection( (struct Section *)nsection, mid, ST_AXP20X, strdup(arg), "AXP20X");	/* Initialize shared fields */

#if 0	/* ToDo */
		nsection->section.publishCustomFigures = publishCustomFiguresAXP20x;
#endif
		nsection->device = NULL;
		nsection->i2c_addr = 0x34;
		nsection->ac = false;
		nsection->vbus = false;
		nsection->bat = false;
		nsection->ips = false;
		nsection->temperature = false;

		if(cfg.verbose)	/* Be verbose if requested */
			publishLog('C', "\tEntering section axp20x '%s' (%04x)", nsection->section.uid, nsection->section.id);

		*section = (struct Section *)nsection;	/* we're now in a section */
		return ACCEPTED;
	} else if(*section){
		if((arg = striKWcmp(l,"Device="))){
			acceptSectionDirective(*section, "Device=");
			assert(( (*(struct section_axp20x **)section)->device = strdup(arg) ));

			if(cfg.verbose)	/* Be verbose if requested */
				publishLog('C', "\t\tDevice : '%s'", (*(struct section_axp20x **)section)->device);
			return ACCEPTED;
		} else if((arg = striKWcmp(l,"Address="))){
			acceptSectionDirective(*section, "Address=");
			(*(struct section_axp20x **)section)->i2c_addr = strtoul(arg, NULL, 0);

			if(cfg.verbose)	/* Be verbose if requested */
				publishLog('C', "\t\tI2c address: 0x%02x", (*(struct section_axp20x **)section)->i2c_addr);
			return ACCEPTED;
		} else if((arg = striKWcmp(l,"Figures="))){
			char *tok = strtok((char *)arg, ","); /* the cast is safe as 'l' is not a constant */
			if(cfg.verbose)	/* Be verbose if requested */
				publishLog('C', "\t\tFigures :");
			while(tok){
				if(!strcmp(tok,"ac")){
					(*(struct section_axp20x **)section)->ac = true;
					if(cfg.verbose)	/* Be verbose if requested */
						publishLog('C', "\t\t\tAC");
				} else if(!strcmp(tok,"vbus")){
					(*(struct section_axp20x **)section)->vbus = true;
					if(cfg.verbose)	/* Be verbose if requested */
						publishLog('C', "\t\t\tVBUS");
				} else if(!strcmp(tok,"bat")){
					(*(struct section_axp20x **)section)->bat = true;
					if(cfg.verbose)	/* Be verbose if requested */
						publishLog('C', "\t\t\tBAT (not yet supported)");
				} else if(!strcmp(tok,"ips")){
					(*(struct section_axp20x **)section)->ips = true;
					if(cfg.verbose)	/* Be verbose if requested */
						publishLog('C', "\t\t\tIPS");
				} else if(!strcmp(tok,"temp")){
					(*(struct section_axp20x **)section)->temperature = true;
					if(cfg.verbose)	/* Be verbose if requested */
						publishLog('C', "\t\t\tTemperature");
				} else {
					publishLog('F', "Unknown figure '%s'", tok);
					exit(EXIT_FAILURE);
				}
				tok = strtok(NULL, ",");
			}
			return ACCEPTED;
		}
	}

	return REJECTED;
}

static bool mh_acceptSDirective( uint8_t sec_id, const char *directive ){
	if(sec_id == ST_AXP20X){
		if( !strcmp(directive, "Disabled") )
			return true;	/* Accepted */
		else if( !strcmp(directive, "DoNotSimulate") )
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
		else if( !strcmp(directive, "Figures=") )
			return true;	/* Accepted */
	}

	return false;
}

static uint16_t read_12bit(struct section_axp20x *s, int fd, uint8_t reg){
    struct i2c_rdwr_ioctl_data packets;
    struct i2c_msg messages[2];

    uint8_t outbuf = reg;
    uint8_t inbuf[2];

    messages[0].addr  = s->i2c_addr;
    messages[0].flags = 0;             // Writing the address
    messages[0].len   = 1;
    messages[0].buf   = &outbuf;

    messages[1].addr  = s->i2c_addr;
    messages[1].flags = I2C_M_RD;      // Reading the value
    messages[1].len   = 2;
    messages[1].buf   = inbuf;

    packets.msgs = messages;
    packets.nmsgs = 2;

    if(ioctl(fd, I2C_RDWR, &packets) < 0){
		publishLog('F', "I2C_RDWR (read_12bit) : %s", strerror(errno));
        return 0xFFFF;
    }

    return ((inbuf[1] & 0x0f) | (inbuf[0] << 4));  // 12 bits : bits [11:0]
}

static void *processAXP20x(void *actx){
	struct section_axp20x *s = (struct section_axp20x *)actx;

		/* Sanity checks */
	if(!s->section.topic){
		publishLog('F', "[%s] Topic must be set. Dying ...", s->section.uid);
		SectionError((struct Section *)s, true);
		pthread_exit(0);
	}

	if(!s->section.sample){
		publishLog('E', "[%s] Sample time can't be 0. Dying ...", s->section.uid);
		SectionError((struct Section *)s, true);
		pthread_exit(0);
	}

	if(!s->device){
		publishLog('E', "[%s] I2c device must be set. Dying ...", s->section.uid);
		SectionError((struct Section *)s, true);
		pthread_exit(0);
	}

		/* Handle Lua functions */
#ifdef LUA
	if(mod_Lua){
		if(s->section.funcname){	/* if an user function defined ? */
			if( (s->section.funcid = mod_Lua->findUserFunc(s->section.funcname)) == LUA_REFNIL ){
				publishLog('E', "[%s] configuration error : user function \"%s\" is not defined. This thread is dying.", s->section.uid, s->section.funcname);
				SectionError((struct Section *)s, true);
				pthread_exit(NULL);
			}
		}
	}
#endif

			/* For each "figures", following topics are published
			 * .../<name>/current
			 * .../<name>/voltage
			 */
					/* /vbus/voltage + 0 : 14 */
	char t[ strlen(s->section.topic) + 14 ];
	strcpy(t, s->section.topic);
	size_t sep = strlen(t);

	for(bool first=true;; first=false){	/* Infinite publishing loop */
		bool inerror = true;	/* By default, we're in trouble */
		if(isDisabled((struct Section *)s)){
			inerror = false;
#ifdef DEBUG
			if(cfg.debug)
				publishLog('d', "[%s] is disabled", s->section.uid);
#endif
		} else if( !first || s->section.immediate ){	/* processing */
			int fd = open(s->device, O_RDWR);	/* Opening I2C */
			if(fd<0)
				publishLog('F', "open(%s) : %s", s->device, strerror(errno));
			else {
				if(s->ac){
					float volt, amp;

					volt = read_12bit(s, fd, 0x56) * 0.0017f;
					amp = read_12bit(s, fd, 0x58) * 0.375f;

					if(cfg.verbose){
						publishLog('I', "AXP209's ACIn Voltage : %.02f V", volt);
						publishLog('I', "AXP209's ACIn Current : %.02f mA", amp);
						publishLog('I', "AXP209's ACIn Power   : %.02f W", volt * amp / 1000);
					}

					bool ret = true;
#ifdef LUA
					if(mod_Lua){
						if(s->section.funcid != LUA_REFNIL){	/* if an user function defined ? */
							mod_Lua->lockState();
							mod_Lua->pushFunctionId( s->section.funcid );
							mod_Lua->pushString( s->section.uid );
							mod_Lua->pushString( "AC" );
							mod_Lua->pushNumber( volt );
							mod_Lua->pushNumber( amp );
							if(mod_Lua->exec(4, 1)){
								publishLog('E', "[%s] AXP20x : %s", s->section.uid, mod_Lua->getStringFromStack(-1));
								mod_Lua->pop(1);	/* pop error message from the stack */
								mod_Lua->pop(1);	/* pop NIL from the stack */
							} else {
								ret = mod_Lua->getBooleanFromStack(-1);	/* Check the return code */
							}
							mod_Lua->unlockState();
						}
					}
#endif

					if(ret){
						char val[8];

						t[sep] = 0;
						strcat(t, "/ac/voltage");
						sprintf(val, "%.02f", volt);
						mqttpublish(cfg.client, t, strlen(val), val, 0);

						t[sep] = 0;
						strcat(t, "/ac/current");
						sprintf(val, "%.02f", amp);
						mqttpublish(cfg.client, t, strlen(val), val, 0);

						t[sep] = 0;
						strcat(t, "/ac/power");
						sprintf(val, "%.02f", volt * amp / 1000);
						mqttpublish(cfg.client, t, strlen(val), val, 0);
					} else if(cfg.verbose)
						publishLog('d', "[%s/AC] AXP20x : Callback rejected the data", s->section.uid);
				}

				if(s->vbus){
					float volt, amp;

					volt = read_12bit(s, fd, 0x5A) * 0.0017f;
					amp = read_12bit(s, fd, 0x5C) * 0.375f;

					if(cfg.verbose){
						publishLog('I', "AXP209's VBus Voltage : %.02f V", volt);
						publishLog('I', "AXP209's VBus Current : %.02f mA", amp);
						publishLog('I', "AXP209's VBus Power   : %.02f W", volt * amp / 1000);
					}

					bool ret = true;
#ifdef LUA
					if(mod_Lua){
						if(s->section.funcid != LUA_REFNIL){	/* if an user function defined ? */
							mod_Lua->lockState();
							mod_Lua->pushFunctionId( s->section.funcid );
							mod_Lua->pushString( s->section.uid );
							mod_Lua->pushString( "VBus" );
							mod_Lua->pushNumber( volt );
							mod_Lua->pushNumber( amp );
							if(mod_Lua->exec(4, 1)){
								publishLog('E', "[%s] AXP20x : %s", s->section.uid, mod_Lua->getStringFromStack(-1));
								mod_Lua->pop(1);	/* pop error message from the stack */
								mod_Lua->pop(1);	/* pop NIL from the stack */
							} else
								ret = mod_Lua->getBooleanFromStack(-1);	/* Check the return code */
							mod_Lua->unlockState();
						}
					}
#endif

					if(ret){
						char val[8];

						t[sep] = 0;
						strcat(t, "/vbus/voltage");
						sprintf(val, "%.02f", volt);
						mqttpublish(cfg.client, t, strlen(val), val, 0);

						t[sep] = 0;
						strcat(t, "/vbus/current");
						sprintf(val, "%.02f", amp);
						mqttpublish(cfg.client, t, strlen(val), val, 0);

						t[sep] = 0;
						strcat(t, "/vbus/power");
						sprintf(val, "%.02f", volt * amp / 1000);
						mqttpublish(cfg.client, t, strlen(val), val, 0);
					} else if(cfg.verbose)
						publishLog('d', "[%s/VBUS] AXP20x : Callback rejected the data", s->section.uid);
				}

				if(s->ips){
					float volt = read_12bit(s, fd, 0x7E) * 0.0014f;
					if(cfg.verbose)
						publishLog('I', "AXP209's IPS Voltage : %.02f V", volt);
				
					bool ret = true;
#ifdef LUA
					if(mod_Lua){
						if(s->section.funcid != LUA_REFNIL){	/* if an user function defined ? */
							mod_Lua->lockState();
							mod_Lua->pushFunctionId( s->section.funcid );
							mod_Lua->pushString( s->section.uid );
							mod_Lua->pushString( "IPS" );
							mod_Lua->pushNumber( volt );
							if(mod_Lua->exec(3, 1)){
								publishLog('E', "[%s] AXP20x : %s", s->section.uid, mod_Lua->getStringFromStack(-1));
								mod_Lua->pop(1);	/* pop error message from the stack */
								mod_Lua->pop(1);	/* pop NIL from the stack */
							} else
								ret = mod_Lua->getBooleanFromStack(-1);	/* Check the return code */
							mod_Lua->unlockState();
						}
					}
#endif

					if(ret){
						char val[8];

						t[sep] = 0;
						strcat(t, "/ips/voltage");
						sprintf(val, "%.02f", volt);
						mqttpublish(cfg.client, t, strlen(val), val, 0);
					} else if(cfg.verbose)
						publishLog('d', "[%s/IPS] AXP20x : Callback rejected the data", s->section.uid);
				}

				if(s->temperature){
					float temp = read_12bit(s, fd, 0x5E) * 0.1f - 144.7;
					if(cfg.verbose)
						publishLog('I', "AXP209's Temperature : %.02f °C", temp);
				
					bool ret = true;
#ifdef LUA
					if(mod_Lua){
						if(s->section.funcid != LUA_REFNIL){	/* if an user function defined ? */
							mod_Lua->lockState();
							mod_Lua->pushFunctionId( s->section.funcid );
							mod_Lua->pushString( s->section.uid );
							mod_Lua->pushString( "Temperature" );
							mod_Lua->pushNumber( temp );
							if(mod_Lua->exec(3, 1)){
								publishLog('E', "[%s] AXP20x : %s", s->section.uid, mod_Lua->getStringFromStack(-1));
								mod_Lua->pop(1);	/* pop error message from the stack */
								mod_Lua->pop(1);	/* pop NIL from the stack */
							} else
								ret = mod_Lua->getBooleanFromStack(-1);	/* Check the return code */
							mod_Lua->unlockState();
						}
					}
#endif

					if(ret){
						char val[8];

						t[sep] = 0;
						strcat(t, "/temperature");
						sprintf(val, "%.02f", temp);
						mqttpublish(cfg.client, t, strlen(val), val, 0);
					} else if(cfg.verbose)
						publishLog('d', "[%s/Temperature] AXP20x : Callback rejected the data", s->section.uid);
				}

			}

			close(fd);
		}

		SectionError((struct Section *)s, inerror);
		struct timespec ts;
		ts.tv_sec = (time_t)s->section.sample;
		ts.tv_nsec = (unsigned long int)((s->section.sample - (time_t)s->section.sample) * 1e9);

		nanosleep( &ts, NULL );
	}

	pthread_exit(0);
}

ThreadedFunctionPtr mh_getSlaveFunction(uint8_t sid){
	if(sid == ST_AXP20X)
		return processAXP20x;

	return NULL;
}

#ifdef LUA
static int so_inError(lua_State *L){
	struct section_axp20x **s = luaL_testudata(L, 1, "AXP20X");
	luaL_argcheck(L, s != NULL, 1, "'AXP20X' expected");

	lua_pushboolean(L, (*s)->section.inerror);
	return 1;
}

static const struct luaL_Reg soM[] = {
	{"inError", so_inError},
	{NULL, NULL}
};
#endif

void InitModule( void ){
	initModule((struct Module *)&mod_axp20x, "mod_axp20x");	/* Identify the module */

		/* Initialize callbacks
		 * It's MANDATORY that all callbacks are initialised
		 */
	mod_axp20x.module.readconf = readconf;
	mod_axp20x.module.acceptSDirective = mh_acceptSDirective;
	mod_axp20x.module.getSlaveFunction = mh_getSlaveFunction;

	registerModule( (struct Module *)&mod_axp20x );	/* Register the module */

#ifdef LUA	/* ToDo */
	if(mod_Lua){ /* Is mod_Lua loaded ? */

			/* Expose shared methods */
		mod_Lua->initSectionSharedMethods(mod_Lua->L, "AXP20X");

			/* Expose mod_owm's own function */
		mod_Lua->exposeObjMethods(mod_Lua->L, "AXP20X", soM);
	}
#endif
}
