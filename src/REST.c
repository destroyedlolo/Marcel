/* REST.c
 * 	Definitions related to REST querying
 *
 * This file is part of Marcel project and is following the same
 * license rules (see LICENSE file)
 *
 * 14/05/2016	- LF - First version
 * 20/08/2016	- LF - Prevent a nasty bug making system to crash if 
 * 		user function lookup is failling
 */

#ifdef LUA	/* Only useful with Lua support */

#include "Every.h"
#include "Version.h"
#include "Marcel.h"
#include "CURL_helpers.h"

#include <stdlib.h>
#include <unistd.h>
#include <lauxlib.h>

#include <curl/curl.h>

static void doRESTquery( struct _REST *ctx ){
	CURL *curl;

	if((curl = curl_easy_init())){
		CURLcode res;
		struct MemoryStruct chunk;

		chunk.memory = malloc(1);
		chunk.size = 0;

#if 0
		curl_easy_setopt(curl, CURLOPT_URL, "file:////home/laurent/Projets/Marcel/jour_tst.json");
#else
		curl_easy_setopt(curl, CURLOPT_URL, ctx->url);
#endif
		curl_easy_setopt(curl, CURLOPT_USERAGENT, "Marcel/" VERSION);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
		curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);

		if((res = curl_easy_perform(curl)) == CURLE_OK){	/* Reading data */
			execUserFuncREST( ctx, chunk.memory );
		} else
			publishLog('E', "[%s] Querying REST : %s\n", ctx->uid, curl_easy_strerror(res));

			/* Cleanup */
		curl_easy_cleanup(curl);
		free(chunk.memory);
	}
}

static void waitNextQuery(struct _REST *ctx){
	if(ctx->at == -1){
		if(ctx->min == -1){	/* It's the 1st run */
			ctx->min = 0;
			if(ctx->immediate)
				return;
		}
		sleep( ctx->sample );
	} else {
		time_t now;
		struct tm tmt;

		time(&now);
		localtime_r(&now, &tmt);

		if(ctx->min == -1){	/* It's the 1st run */
			ctx->min = ctx->at % 100;
			ctx->at /= 100;

			if(ctx->immediate)
				return;
			else if((ctx->at*60 + ctx->min < tmt.tm_hour * 60 + tmt.tm_min) && ctx->runifover)	/* Already over */
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
			publishLog('E', "[%s] configuration error : user function \"%s\" is not defined", ctx->uid, ctx->funcname);
			publishLog('F', "[%s] Dying", ctx->uid);
			pthread_exit(NULL);
		}
	}

	publishLog('I', "Launching a processing flow for '%s' REST task\n", ctx->uid);

	ctx->min = -1;	/* Indicate it's the 1st run */
	for(;;){
		waitNextQuery( ctx );
		if(ctx->disabled){
			publishLog('I', "REST '%s' is currently disabled.\n", ctx->uid);
		} else
			doRESTquery( ctx );
	}
}

#endif /* LUA */
