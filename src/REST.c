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

void *process_REST(void *actx){
	struct _REST *ctx = actx;	/* Only to avoid zillions of cast */

	if(ctx->funcname){
		if( (ctx->funcid = findUserFunc( ctx->funcname )) == LUA_REFNIL ){
			fprintf(stderr, "*F* configuration error : user function \"%s\" is not defined\n", ctx->funcname);
			exit(EXIT_FAILURE);
		}
	}

	if(verbose)
		printf("Launching a processing flow for '%s' REST task\n", ctx->funcname);

	pthread_exit(0);
}

#endif /* LUA */
