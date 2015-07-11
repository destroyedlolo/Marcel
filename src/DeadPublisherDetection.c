/* DeadPublisherDetection.c
 * 	DPD processing
 *
 * This file is part of Marcel project and is following the same
 * license rules (see LICENSE file)
 *
 * 09/07/2015	- LF - First version
 */

#include "DeadPublisherDetection.h"

#include <time.h>
#include <sys/timerfd.h>
#include <sys/eventfd.h>
#include <unistd.h>

extern void *process_DPD(void *actx){
	struct _DeadPublisher *ctx = actx;	/* Only to avoid zillions of cast */
	struct itimerspec itval;

	if(( ctx->rcv = eventfd( 0, 0 )) == -1 ){
		perror("eventfd()");
		pthread_exit(0);
	}

	itval.it_value.tv_sec = (time_t)ctx->sample;
	itval.it_value.tv_nsec = 0;
	itval.it_interval.tv_sec = 0;
	itval.it_interval.tv_nsec = 0;
	if(( ctx->timer = timerfd_create( CLOCK_REALTIME, 0 )) == -1){
		perror("timerfd()");
		close( ctx->rcv );
		ctx->rcv = -1;
		pthread_exit(0);
	}

	if( MQTTClient_subscribe( cfg.client, ctx->topic, 0 ) != MQTTCLIENT_SUCCESS ){
		close( ctx->rcv );
		close( ctx->timer );
		fprintf(stderr, "Can't subscribe to '%s'\n", ctx->topic );
		pthread_exit(0);
	}

	pthread_exit(0);
}

