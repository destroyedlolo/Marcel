/* Every.c
 * 	Repeating task
 *
 * This file is part of Marcel project and is following the same
 * license rules (see LICENSE file)
 *
 * 07/09/2015	- LF - First version
 * 17/05/2016	- LF - Add 'Immediate'
 * 06/06/2016	- LF - If sample is null, stop 
 * 20/08/2016	- LF - Prevent a nasty bug making system to crash if 
 * 		user function lookup is failling
 */
#ifdef LUA	/* Only useful with Lua support */

#include "Every.h"
#include "Marcel.h"

#include <stdlib.h>
#include <unistd.h>
#include <lauxlib.h>

void *process_Every(void *actx){
	struct _Every *ctx = actx;	/* Only to avoid zillions of cast */

	if(ctx->funcname && ctx->funcid == LUA_REFNIL){
		if( (ctx->funcid = findUserFunc( ctx->funcname )) == LUA_REFNIL ){
			fprintf(stderr, "*E* [%s] configuration error : user function \"%s\" is not defined\n*E*This thread is dying.\n", ctx->uid, ctx->funcname);
			pthread_exit(NULL);
		}
	}

	if(verbose)
		printf("Launching a processing flow for '%s' Every task\n", ctx->uid);

	if(ctx->immediate && !ctx->disabled)
		execUserFuncEvery( ctx );

	for(;;){
		if(!ctx->sample){
			if(verbose)
				printf("*I* Every '%s' has 0 sample delay : dying ...\n", ctx->uid);
			break;
		} else
			sleep( ctx->sample );
		if( ctx->disabled ){
			if(verbose)
				printf("*I* Every '%s' is disabled.\n", ctx->uid);
		} else 
			execUserFuncEvery( ctx );
	}

	pthread_exit(0);
}

#endif /* LUA */
