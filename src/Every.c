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
 * 		user function lookup is failing
 */
#ifdef LUA	/* Only useful with Lua support */

#include "Every.h"
#include "REST.h" /* for waitNextQuery */
#include "Marcel.h"

#include <stdlib.h>
#include <unistd.h>
#include <lauxlib.h>

void *process_Every(void *actx){
	struct _Every *ctx = actx;	/* Only to avoid zillions of cast */

	if(!cfg.luascript){
			publishLog('E', "[%s] configuration error : No Lua script defined. This thread is dying.", ctx->uid, ctx->funcname);
			pthread_exit(NULL);
	}

	if(ctx->funcname && ctx->funcid == LUA_REFNIL){
		if( (ctx->funcid = findUserFunc( ctx->funcname )) == LUA_REFNIL ){
			publishLog('E', "[%s] configuration error : user function \"%s\" is not defined. This thread is dying.", ctx->uid, ctx->funcname);
			pthread_exit(NULL);
		}
	}

	publishLog('I', "Launching a processing flow for '%s' Every task", ctx->uid);
	ctx->min = -1;	/* Indicate it's the 1st run */

	for(;;){
		waitNextQuery( (struct _REST *)ctx );
		if(ctx->disabled){
			publishLog('I', "Every '%s' is currently disabled.\n", ctx->uid);
		} else
			execUserFuncEvery( ctx );
	}	

	pthread_exit(0);
}

#endif /* LUA */
