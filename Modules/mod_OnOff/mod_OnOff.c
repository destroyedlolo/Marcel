/* mod_OnOff
 *
 * Enable or disable a section
 *
 * This file is part of Marcel project and is following the same
 * license rules (see LICENSE file)
 *
 * NOTEZ-BIEN : this module doesn't define any directive or section
 */

#include "mod_OnOff.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>

static struct module_OnOff mod_OnOff;

static void subTopic( uint8_t mid ){
	assert(( mod_OnOff.topic = malloc( strlen(cfg.ClientID) + 9) ));	/* "/OnOff/#" + \0 */
	sprintf( mod_OnOff.topic, "%s/OnOff/#", cfg.ClientID );

	if(cfg.verbose)
		publishLog('I', "OnOff topic : %s", mod_OnOff.topic);

	if(MQTTClient_subscribe( cfg.client, mod_OnOff.topic, 0 ) != MQTTCLIENT_SUCCESS){
		publishLog('E', "Can't subscribe to '%s'", mod_OnOff.topic );
		exit( EXIT_FAILURE );
	}

	mod_OnOff.topic[strlen(mod_OnOff.topic)-1] = 0;	/* Remove leading '#' */
}

static bool processMsg(const char *topic, char *payload){
	const char *sid;

	if((sid = striKWcmp(topic, mod_OnOff.topic))){
		int h = chksum(sid);
		struct Section *s;

		for(s = sections; s; s = s->next){
			if(s->h == h && !strcmp(s->uid, sid))	/* Section found */
				break;
		}
		if(s){
			bool disable = false;
			if( !strcmp(payload,"0") || !strcasecmp(payload,"off") || !strcasecmp(payload,"disable") )
				disable = true;
			s->disabled = disable;
			publishLog('T', "[OnOff] %s '%s'", disable ? "Disabling":"Enabling", sid);
		} else
			publishLog('T', "[OnOff] No section matching '%s'", sid);
		return true;
	}
	return false;
}

void InitModule( void ){
	initModule((struct Module *)&mod_OnOff, "mod_OnOff");

	mod_OnOff.module.postconfInit = subTopic;
	mod_OnOff.module.processMsg = processMsg;

	mod_OnOff.topic = NULL;

	registerModule( (struct Module *)&mod_OnOff );
}
