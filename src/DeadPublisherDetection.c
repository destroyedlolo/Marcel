/* DeadPublisherDetection.c
 * 	DPD processing
 *
 * This file is part of Marcel project and is following the same
 * license rules (see LICENSE file)
 *
 * 09/07/2015	- LF - First version
 */

#include "DeadPublisherDetection.h"

extern void *process_DPD(void *actx){
	pthread_exit(0);
}

