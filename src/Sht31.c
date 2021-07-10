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

void *process_Sht31(void *actx){
	struct _Sht31 *ctx = actx;	/* Only to avoid zillions of cast */

	pthread_exit(0);
}

#endif
