/* REST.c
 * 	Definitions related to REST querying
 *
 * This file is part of Marcel project and is following the same
 * license rules (see LICENSE file)
 *
 * 14/05/2016	- LF - First verison
 */

#ifdef LUA	/* Only useful with Lua support */

#include "Every.h"
#include "Marcel.h"

#include <stdlib.h>
#include <unistd.h>
#include <lauxlib.h>

static void doRESTquery( struct _REST *ctx ){
// curl ...
// pass the result
		execUserFuncREST( ctx );
}

static void waitNextQuery(struct _REST *ctx){
	if(ctx->at == -1){
		if(ctx->min == -1){	/* It's the 1st run */
			ctx->min = 0;
			return;
		} else
			sleep( ctx->sample );
	} else {
		time_t now;
		struct tm tmt;

		time(&now);
		localtime_r(&now, &tmt);

		if(ctx->min == -1){	/* It's the 1st run */
			ctx->min = ctx->at % 100;
			ctx->at /= 100;
			if(ctx->at*60 + ctx->min < tmt.tm_hour * 60 + tmt.tm_min)	/* Already over */
				return;
		} 

		if(ctx->at*60 + ctx->min <= tmt.tm_hour * 60 + tmt.tm_min)	/* We have to wait until the next day */
			tmt.tm_hour = ctx->at + 24;
		else
			tmt.tm_hour = ctx->at;
		tmt.tm_min = ctx->min;
		tmt.tm_sec = 0;
		sleep( (unsigned int)difftime(mktime( &tmt ), now) );
	}
}

void *process_REST(void *actx){
	struct _REST *ctx = actx;	/* Only to avoid zillions of cast */

	if(ctx->funcname && ctx->funcid == LUA_REFNIL){
		if( (ctx->funcid = findUserFunc( ctx->funcname )) == LUA_REFNIL ){
			fprintf(stderr, "*F* configuration error : user function \"%s\" is not defined\n", ctx->funcname);
			exit(EXIT_FAILURE);
		}
	}

	if(verbose)
		printf("Launching a processing flow for '%s' REST task\n", ctx->funcname);

	ctx->min = -1;	/* Indicate it's the 1st run */
	for(;;){
		waitNextQuery( ctx );
		doRESTquery( ctx );
	}
}

#endif /* LUA */
