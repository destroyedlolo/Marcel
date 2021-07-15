/* Sht31.h
 * 	Definitions related to sht31 task
 *
 * This file is part of Marcel project and is following the same
 * license rules (see LICENSE file)
 *
 * 10/07/2021	- LF - First version
 */

#ifdef SHT31

#include "Marcel.h"
#include "Sht31.h"
#include "MQTT_tools.h"

#include <fcntl.h>	/* open() */
#include <unistd.h>	/* read(), write() */
#include <errno.h>
#include <string.h>	/* strerror() */
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>

void *process_Sht31(void *actx){
	struct _Sht31 *ctx = actx;	/* Only to avoid zillions of cast */

	if(!ctx->device){
		publishLog('F', "[%s] configuration error : no device specified, ignoring this section", ctx->uid);
		pthread_exit(0);
	}

	publishLog('I', "Launching a processing flow for SHT31 '%s'", ctx->uid);

		/* Build temperature topic */
	struct _VarSubstitution tempvslookup[] = {
		{ "%FIGURE%", "Temperature" },	/* MUST BE THE 1ST VARIABLE */
		{ NULL }
	};
	init_VarSubstitution( tempvslookup );
	const char *temptopic = replaceVar( ctx->topic, tempvslookup );

		/* Build Humidity topic */
	struct _VarSubstitution humvslookup[] = {
		{ "%FIGURE%", "Humidity" },	/* MUST BE THE 1ST VARIABLE */
		{ NULL }
	};
	init_VarSubstitution( humvslookup );
	const char *humtopic = replaceVar( ctx->topic, humvslookup );

	publishLog('I', "[%s] Temperature : '%s'", ctx->uid, temptopic);
	publishLog('I', "[%s] Humidity : '%s'", ctx->uid, humtopic);

	for(;;){	/* Infinite loop to publish data */
		if(ctx->disabled){
			publishLog('T', "SHT31 '%s' is disabled", ctx->uid);
			sleep(ctx->sample);
			continue;
		}

		int fd;
		if((fd = open(ctx->device, O_RDWR)) < 0){
			publishLog('E', "[%s] i2c-bus open(%s) : %s", ctx->uid, ctx->device, strerror(errno));
			break;
		}
		ioctl(fd, I2C_SLAVE, ctx->i2c_addr);	/* Which slave to talk to */

		char data[6];

		data[0] = 0x2c;	/* high repeatability measurement command */
		data[1] = 0x06;
		write(fd, data, 2);
		sleep(1);

		if(read(fd, data, 6) != 6){	/* Read the result */
			publishLog('E', "[%s] I/O error", ctx->uid);
		} else {
			double val = (((data[0] * 256) + data[1]) * 175.0) / 65535.0  - 45.0;
			printf("Temp : %.2f\n", val);

			val = (((data[3] * 256) + data[4])) * 100.0 / 65535.0;
			printf("Hum : %.2f\n", val);
		}
		close(fd);

		sleep(ctx->sample);
	}

	pthread_exit(0);
}

#endif
