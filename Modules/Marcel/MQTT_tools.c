/* MQTT_tools.c
 *
 * Tools useful for MQTT processing
 * (this file is shared by several projects)
 *
 * This file is part of Marcel project and is following the same
 * license rules (see LICENSE file)
 *
 * 13/07/2015 LF : First version
 */

#include "MQTT_tools.h"
#include "Marcel.h"

int mqtttokcmp(const char *s, const char *t, const char **rem){
	char last = 0;
	if(!s || !t)
		return -1;

	for(; ; s++, t++){
		if(!*s){ /* End of string */
			return(*s - *t);
		} else if(*s == '#'){ /* ignore remaining of the string */
			int ret = (!*++s && (!last || last=='/')) ? 0 : -1;
			if(rem)
				*rem = t;
			return ret;
		} else if(*s == '+'){	/* ignore current level of the hierarchy */
			s++;
			if((last !='/' && last ) || (*s != '/' && *s )) /* has to be enclosed by '/' */
				return -1;
			while(*t != '/'){	/* looking for the closing '/' */
				if(!*t)
					return 0;
				t++;
			}
			if(*t != *s)	/* ensure the s isn't finished */
				return -1;
		} else if(*s != *t)
			break;
		last = *s;
	}
	return(*s - *t);
}

int mqttpublish(MQTTClient client, const char *topic, int length, void *payload, int retained ){ /* Custom wrapper to publish */
	MQTTClient_message pubmsg = MQTTClient_message_initializer;
	pubmsg.retained = retained;
	pubmsg.payloadlen = length;
	pubmsg.payload = payload;

	int err = MQTTClient_publishMessage(client, topic, &pubmsg, NULL);

	if(cfg.verbose && err != MQTTCLIENT_SUCCESS)
		publishLog('E', "mqttpublish() : %s", MQTTClient_strerror(err) );

	return err;
}

