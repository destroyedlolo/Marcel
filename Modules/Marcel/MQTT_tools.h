/* MQTT_tools.h
 *
 * Definition for tools useful for MQTT processing
 *
 * This file is part of Marcel project and is following the same
 * license rules (see LICENSE file)
 *
 * 13/07/2015 LF : First version
 */

#ifndef MQTT_TOOL_H
#define MQTT_TOOL_H

#include <MQTTClient.h>


/**
 * @brief Compare 2 strings like strcmp() but s can contain MQTT wildcards
 * '#' : replace remaining of the line
 * '+' : a sub topic and must be enclosed by '/'
 *
 *  
 * Wildcards are checked as per mosquitto's source code rules
 * (comment in http://git.eclipse.org/c/mosquitto/org.eclipse.mosquitto.git/tree/src/subs.c)
 *
 * @param s string to be compared
 * @param t topic to be compared to
 * @param remain pointer to the remaining of the topic.
 *  If the topic ends by a '#', if set, points to the what the '"' wildcard replaces.
 * @return 0 : strings match, -1 : wildcard error, others : strings are different
 */
extern int mqtttokcmp(register const char *s, register const char *t, const char **remain);

/* Publish an MQTT message */
extern int mqttpublish(MQTTClient client, const char *topic, int length, void *payload, int retained );
#endif
