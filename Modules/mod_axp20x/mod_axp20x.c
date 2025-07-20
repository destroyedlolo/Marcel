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
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>

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
						publishLog('C', "\t\t\tBAT");
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

void InitModule( void ){
	initModule((struct Module *)&mod_axp20x, "mod_axp20x");	/* Identify the module */

		/* Initialize callbacks
		 * It's MANDATORY that all callbacks are initialised
		 */
	mod_axp20x.module.readconf = readconf;
	mod_axp20x.module.acceptSDirective = mh_acceptSDirective;
#if 0	/* ToDo */
	mod_axp20x.module.getSlaveFunction = mh_getSlaveFunction;
#endif

	registerModule( (struct Module *)&mod_axp20x );	/* Register the module */

#ifdef LUAx
	if(mod_Lua){ /* Is mod_Lua loaded ? */

			/* Expose shared methods */
		mod_Lua->initSectionSharedMethods(mod_Lua->L, "AXP20X");

			/* Expose mod_owm's own function */
		mod_Lua->exposeObjMethods(mod_Lua->L, "AXP20X", soM);
	}
#endif
}
