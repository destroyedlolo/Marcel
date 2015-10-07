/* Meteo.c
 *	Publish meteo information.
 *
 *	Based on OpenWeatherMap.org information
 */

#ifndef METEO_H
#include <curl/curl.h>
#include <stdlib.h>		/* memory */
#include <string.h>		/* memcpy() */
#include <ctype.h>		/* isdigit() */

#include "Meteo.h"

	/* Curl's
	 * Storing downloaded information in memory
	 * From http://curl.haxx.se/libcurl/c/getinmemory.html example
	 */

struct MemoryStruct {
	char *memory;
	size_t size;
};

static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp){
	size_t realsize = size * nmemb;
	struct MemoryStruct *mem = (struct MemoryStruct *)userp;

	mem->memory = realloc(mem->memory, mem->size + realsize + 1);
	if(mem->memory == NULL){ /* out of memory! */ 
		fputs("not enough memory (realloc returned NULL)\n", stderr);
		return 0;
	}

	memcpy(&(mem->memory[mem->size]), contents, realsize);
	mem->size += realsize;
	mem->memory[mem->size] = 0;
 
	return realsize;
}

void *process_MeteoST(void *actx){
	CURL *curl;

	if(verbose)
		printf("Launching a processing flow for MeteoST\n");

	if((curl = curl_easy_init())){
		CURLcode res;
		struct MemoryStruct chunk;

		chunk.memory = malloc(1);
		chunk.size = 0;

		curl_easy_setopt(curl, CURLOPT_URL, "file:////home/laurent/Projets/Marcel/tst.json");
		curl_easy_setopt(curl, CURLOPT_USERAGENT, "Marcel/" VERSION);

		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);

		if((res = curl_easy_perform(curl)) == CURLE_OK){	/* Processing data */
		} else
			fprintf(stderr, "*E* Querying meteo : %s\n", curl_easy_strerror(res));

			/* Cleanup */
		curl_easy_cleanup(curl);
		free(chunk.memory);
	}

	pthread_exit(0);
}

#endif
