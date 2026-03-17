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

	/* Extract a JSon object per its path
	 */
struct json_object *getObj(struct json_object *parent, const char *path[]){
	struct json_object *obj = parent;

	for(int i=0; path[i]; ++i){
#if 0	/* Remove uneeded noise */
		if(debug)
			printf("*D* %d: '%s'\n", i, path[i]);
#endif

		if(!obj){
			if(cfg.debug)
				publishLog('E', "JSon : Broken path at %dth\n", i);
			return NULL;
		}
		obj = json_object_object_get(obj, path[i]);
	}

	return obj;
}

const char *getObjString(struct json_object *parent, const char *path[]){
	struct json_object *obj = getObj(parent, path);
	if(!obj)
		return NULL;

	if(json_object_is_type(obj, json_type_string))
		return json_object_get_string(obj);
	else if(cfg.debug)
		publishLog('E', "JSon : Expecting a string");

	return NULL;
}

int getObjInt(struct json_object *parent, const char *path[]){
	struct json_object *obj = getObj(parent, path);
	if(!obj)
		return 0;

	if(json_object_is_type(obj, json_type_int))
		return json_object_get_int(obj);
	else if(cfg.debug)
		publishLog('E', "JSon : Expecting an integer");

	return 0;
}

double getObjNumber(struct json_object *parent, const char *path[]){
	struct json_object *obj = getObj(parent, path);
	if(!obj)
		return 0;

	if(json_object_is_type(obj, json_type_double))
		return json_object_get_double(obj);
	else if(json_object_is_type(obj, json_type_int))	/* It seems some object are only integers */
		return json_object_get_int(obj);
	else if(cfg.debug)
		publishLog('E', "JSon : Expecting a double or integer");

	return 0;
}

bool getObjBool(struct json_object *parent, const char *path[]){
	struct json_object *obj = getObj(parent, path);
	if(!obj)
		return false;

	if(json_object_is_type(obj, json_type_boolean))
		return json_object_get_boolean(obj);
	else if(cfg.debug)
		publishLog('E', "JSon : Expecting a boolean\n");

	return false;
}
#endif /* USE_CURL */
