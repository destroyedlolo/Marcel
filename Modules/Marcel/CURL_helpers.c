/* CURL_helpers.c
 *
 * Helpers for Curl processing
 *
 * 16/05/2016 LF : First version
 */

#include "CURL_helpers.h"
#include "Marcel.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

	/* Curl's
	 * Storing downloaded information in memory
	 * From http://curl.haxx.se/libcurl/c/getinmemory.html example
	 */

size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp){
	size_t realsize = size * nmemb;
	struct MemoryStruct *mem = (struct MemoryStruct *)userp;

	mem->memory = realloc(mem->memory, mem->size + realsize + 1);
	if(mem->memory == NULL){ /* out of memory! */ 
		publishLog('E', "not enough memory (realloc returned NULL)");
		return 0;
	}

	memcpy(&(mem->memory[mem->size]), contents, realsize);
	mem->size += realsize;
	mem->memory[mem->size] = 0;
 
	return realsize;
}

	/*
	 * Global initialisation
	 */

#ifdef USE_CURL

#include <pthread.h>
#include <curl/curl.h>

static pthread_once_t curl_init_once = PTHREAD_ONCE_INIT;

	/* Global cleanup
	 *	I decided to let the OS to clean everything.
	 *	A cleaner code would call this function. Unfortunately
	 *	it's meaning to track all slave threads : it would be
	 *	a bit difficult to manage as Marcel is massively
	 *	multithreaded and subject to cleanup delay which is not
	 *	suitable for a daemon.
	 */
#if 0
void cleanup_curl_global(void) {
    curl_global_cleanup();
}
#endif

static void init_curl_global(void) {
    curl_global_init(CURL_GLOBAL_ALL);
}

	/* Usage
	 *
	 *	Curl may or may not be used. To avoid overwhelm or failure risk
	 * when curl is not used, pthread_once below has to be used in
	 * every module's initialization where curl is needed. 

void function_needing_curl() {
    pthread_once(&curl_init_once, init_curl_global);
    CURL *curl = curl_easy_init();
    // ... using curl ...
    curl_easy_cleanup(curl);
}

	* init_Curl() below abstract this call.
	*/
void init_Curl(void){
	pthread_once(&curl_init_once, init_curl_global);
}
#endif /* USE_CURL */
