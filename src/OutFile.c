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
		publishLog('I', "Writing to OutFile '%s' is disabled\n", ctx->uid);
		return;
	}

#ifdef LUA
	if(ctx->funcname && ctx->funcid == LUA_REFNIL){
		if(!cfg.luascript){
			publishLog('E', "[%s] configuration error : No Lua script defined whereas a function is used. This thread is dying.", ctx->uid, ctx->funcname);
			pthread_exit(NULL);
		}
		
		if( (ctx->funcid = findUserFunc( ctx->funcname )) == LUA_REFNIL ){
			publishLog('E', "[%s] configuration error : user function \"%s\" is not defined", ctx->uid, ctx->funcname);
			publishLog('F', "[%s] Dying", ctx->uid);
			pthread_exit(NULL);
		}
	}
#endif

	FILE *f=fopen( ctx->file, "w" );
	if(!f){
		publishLog('E', "[%s] '%s' : %s", ctx->uid, ctx->file, strerror(errno));
		return;
	}
	fputs(msg, f);
	if(ferror(f)){
		publishLog('E', "[%s] '%s' : %s", ctx->uid, ctx->file, strerror(errno));
		fclose(f);
		return;
	} 
	publishLog('I', "[%s] '%s' written in '%s'\n", ctx->uid, msg, ctx->file);
	fclose(f);

#ifdef LUA
	execUserFuncOutFile( ctx, msg );
#endif

	return;
}
