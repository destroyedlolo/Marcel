/* OutFile.c
 *	Write value to file
 *
 * This file is part of Marcel project and is following the same
 * license rules (see LICENSE file)
 *
 * 14/08/2016	- LF - creation
 * 20/08/2016	- LF - handles Disabled
 */

#include "OutFile.h"

#include <stdio.h>
#include <errno.h>
#include <string.h>

#ifdef LUA
#include <lauxlib.h>
#endif

void processOutFile( struct _OutFile *ctx, const char *msg){
	if(ctx->disabled){
		if(verbose)
			printf("*I* Writing to OutFile '%s' is disabled\n", ctx->uid);
		return;
	}

#ifdef LUA
	if(ctx->funcname && ctx->funcid == LUA_REFNIL){
		if( (ctx->funcid = findUserFunc( ctx->funcname )) == LUA_REFNIL ){
			fprintf(stderr, "*E* [%s] configuration error : user function \"%s\" is not defined\n*E*This thread is dying.\n", ctx->uid, ctx->funcname);
			pthread_exit(NULL);
		}
	}
#endif

	FILE *f=fopen( ctx->file, "w" );
	if(!f){
		fprintf(stderr, "*E* '%s' : %s\n", ctx->file, strerror(errno));
		return;
	}
	fputs(msg, f);
	if(ferror(f)){
		fprintf(stderr, "*E* '%s' : %s\n", ctx->file, strerror(errno));
		fclose(f);
		return;
	} else if(verbose)
		printf("*I* [%s] '%s' written in '%s'\n", ctx->uid, msg, ctx->file);
	fclose(f);

#ifdef LUA
	execUserFuncOutFile( ctx, msg );
#endif

	return;
}
