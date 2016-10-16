/* DeadPublisherDetection.c
 * 	DPD processing
 *
 * This file is part of Marcel project and is following the same
 * license rules (see LICENSE file)
 *
 * 09/07/2015	- LF - First version
 * 28/07/2015	- LF - Add user function
 * 20/08/2016	- LF - Prevent a nasty bug making system to crash if 
 * 		user function lookup is failling
 * 21/08/2016	- LF - Replace errorid by uid
 */

#include "DeadPublisherDetection.h"
#include "MQTT_tools.h"

#include <sys/time.h>
#include <sys/select.h>

#include <sys/eventfd.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#ifdef LUA
#include <lauxlib.h>
#endif

void *process_DPD(void *actx){
	struct _DeadPublisher *ctx = actx;	/* Only to avoid zillions of cast */
	struct timespec ts, *uts;

		/* Sanity checks */
#ifdef LUA
	if(ctx->funcname && cfg.luascript){
		if( (ctx->funcid = findUserFunc( ctx->funcname )) == LUA_REFNIL ){
			publishLog('E', "[%s] configuration error : user function \"%s\" is not defined. This thread is dying.", ctx->uid, ctx->funcname);
			pthread_exit(NULL);
		}
	}
#endif
	publishLog('I', "Launching a processing flow for DeadPublisherDetect (DPD) '%s'", ctx->uid);

		/* Creating the fd for the notification */
	if(( ctx->rcv = eventfd( 0, 0 )) == -1 ){
		publishLog('E', "[%s] eventfd() : %s", ctx->uid, strerror(errno));
		pthread_exit(0);
	}
	if(ctx->sample){
		ts.tv_sec = (time_t)ctx->sample;
		ts.tv_nsec = 0;

		uts = &ts;
	} else
		uts = NULL;

		/* Subscribe */
		if(MQTTClient_subscribe( cfg.client, ctx->topic, 0 ) != MQTTCLIENT_SUCCESS ){
			publishLog('E', "[%s] Can't subscribe to '%s'", ctx->uid, ctx->topic );
			pthread_exit(NULL);
		}


	for(;;){
		fd_set rfds;
		FD_ZERO( &rfds );
		FD_SET( ctx->rcv, &rfds );

		switch( pselect( ctx->rcv+1, &rfds, NULL, NULL, uts, NULL ) ){
		case -1:	/* Error */
			close( ctx->rcv );
			ctx->rcv = 1;
			publishLog(ctx->keep ? 'E' : 'F', "[%s] pselect() : %s", ctx->uid, strerror(errno));
			if(ctx->keep){
				sleep(1);
				continue;
			} else
				pthread_exit(0);
		case 0:	/* timeout */
			if( ctx->disabled )
				publishLog('I', "Alerting for DPD '%s' is disabled", ctx->uid);
			else {
				publishLog('I', "timeout for DPD '%s'", ctx->uid);
				if( !ctx->inerror ){	/* Entering in error */
					if(!ctx->errtopic){		/* No error topic defined : sending an alert */
						char topic[strlen(ctx->uid) + 7]; /* "Alert/" + 1 */
						const char *msg_info = "SNo data received after %d seconds";
						char msg[ strlen(msg_info) + 15 ];	/* Some room for number of seconds */

						strcpy( topic, "Alert/" );
						strcat( topic, ctx->uid );
						int msg_len = sprintf( msg, msg_info, ctx->sample );
						if( mqttpublish( cfg.client, topic, msg_len, msg, 0 ) == MQTTCLIENT_SUCCESS )
							ctx->inerror = true;
					} else {	/* Error topic defined */
						char topic[ strlen(ctx->errtopic) + strlen(ctx->uid) + 2];	/* + '/' + 0 */
						const char *msg_info = "No data received after %d seconds";
						char msg[ strlen(msg_info) + 15 ];	/* Some room for number of seconds */

						sprintf( topic, "%s/%s", ctx->errtopic, ctx->uid );
						int msg_len = sprintf( msg, msg_info, ctx->sample );
						if( mqttpublish( cfg.client, topic, msg_len, msg, 0 ) == MQTTCLIENT_SUCCESS )
							ctx->inerror = true;
					}

					publishLog('I', "Alert raises for DPD '%s'", ctx->uid);
				}
			}
			break;
		default:{	/* Got some data
					 *
					 * Notez-Bien : Lua's user function is launched in 
					 * Marcel.c/msgarrived() directly
					 */
				uint64_t v;
				if(read(ctx->rcv, &v, sizeof( uint64_t )) == -1)
					publishLog('E', "[%s] eventfd() : %s - reading notification", ctx->uid, strerror(errno));
	
				if( ctx->disabled )
					publishLog('I', "Alerting for DPD '%s' is disabled", ctx->uid);
				else {
					if( ctx->inerror ){	/* Existing error condition */
						if(!ctx->errtopic){		/* No error topic defined : sending an alert */
							char topic[strlen(ctx->uid) + 7]; /* "Alert/" + 1 */
							strcpy( topic, "Alert/" );
							strcat( topic, ctx->uid );
							if( mqttpublish( cfg.client, topic, 1, "E", 0 ) == MQTTCLIENT_SUCCESS )
								ctx->inerror = false;
						} else {	/* Error topic defined */
							char topic[ strlen(ctx->errtopic) + strlen(ctx->uid) + 2];	/* + '/' + 0 */
								/* I duno if it's really needed to have a writable payload,
								 * but anyway, it's safer as per API provided
								 */
							const char *msg_info = "Data received : issue corrected";
							size_t msglen = strlen(msg_info);
							char tmsg[ msglen+1 ];
							strcpy( tmsg, msg_info );
						
							sprintf( topic, "%s/%s", ctx->errtopic, ctx->uid );
							if( mqttpublish( cfg.client, topic, msglen, tmsg, 0 ) == MQTTCLIENT_SUCCESS )
								ctx->inerror = false;
						}

						publishLog('I', "Alert corrected for DPD '%s'", ctx->uid);
					}
				}
			}
			break;
		}
	}

	close( ctx->rcv );
	ctx->rcv = 1;
	
	pthread_exit(0);
}

