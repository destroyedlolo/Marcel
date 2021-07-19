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

		data[0] = 0x2c;	/* clock stretching */
		data[1] = 0x06; /* high repeatability measurement command */
		if(write(fd, data, 2) != 2){
			close(fd);
			publishLog('E', "[%s] write I/O error", ctx->uid);
		} else {

/* Thanks to clock stretching, it's not needed as the reading will be possible
 * only after acquisition.
 * sleep(1); 
 */

			if(read(fd, data, 6) != 6){	/* Read the result */
				close(fd);
				publishLog('E', "[%s] I/O error", ctx->uid);
			} else {	/* Conversion formulas took from SHT31 datasheet */
				double val;
				char t[8];

				close(fd);	/* Release the bus as soon as possible */

				val = (((data[0] * 256) + data[1]) * 175.0) / 65535.0  - 45.0;
				sprintf(t, "%.2f", val);
				mqttpublish(cfg.client, temptopic, strlen(t), t, 0);

				val = (((data[3] * 256) + data[4])) * 100.0 / 65535.0;
				sprintf(t, "%.2f", val);
				mqttpublish(cfg.client, humtopic, strlen(t), t, 0);
			}
		}

		sleep(ctx->sample);
	}

	pthread_exit(0);
}

#endif
