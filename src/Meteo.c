/* Meteo.c
 *	Publish meteo information.
 *
 *	Based on OpenWeatherMap.org information
 */

#ifndef METEO_H
#include "Meteo.h"
#include "MQTT_tools.h"

#include <curl/curl.h>
#include <stdlib.h>		/* memory */
#include <string.h>		/* memcpy() */
#include <assert.h>
#include <json-c/json.h>

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

static void Meteo3H(struct _Meteo *ctx){
	CURL *curl;
	enum json_tokener_error jerr = json_tokener_success;

	if(verbose)
		printf("Launching a processing flow for Meteo\n");

	if((curl = curl_easy_init())){
		char url[strlen(URLMETEO3H) + strlen(ctx->City) + strlen(ctx->Units) + strlen(ctx->Lang)];	/* Thanks to %s, Some room left for \0 */
		CURLcode res;
		struct MemoryStruct chunk;

		chunk.memory = malloc(1);
		chunk.size = 0;

		sprintf(url, URLMETEO3H, ctx->City, ctx->Units, ctx->Lang);
#if 0
		curl_easy_setopt(curl, CURLOPT_URL, "file:////home/laurent/Projets/Marcel/meteo_tst.json");
#else
		curl_easy_setopt(curl, CURLOPT_URL, url);
#endif
		curl_easy_setopt(curl, CURLOPT_USERAGENT, "Marcel/" VERSION);

		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);

		if((res = curl_easy_perform(curl)) == CURLE_OK){	/* Processing data */
			json_object * jobj = json_tokener_parse_verbose(chunk.memory, &jerr);
			if(jerr != json_tokener_success)
				fprintf(stderr, "*E* Querying meteo : %s\n", json_tokener_error_desc(jerr));
			else {
				struct json_object *wlist = json_object_object_get( jobj, "list");
				if(wlist){
					int nbre = json_object_array_length(wlist);
					char l[MAXLINE];
					for(int i=0; i<nbre; i++){
						struct json_object *wo = json_object_array_get_idx( wlist, i );	/* Weather's object */
						struct json_object *wod = json_object_object_get( wo, "dt" );	/* Weather's data */

						int lm = sprintf(l, "%s/%d/time", ctx->topic, i) + 2;
						assert( lm+1 < MAXLINE-10 );
						sprintf( l+lm, "%u", json_object_get_int(wod));
						mqttpublish( cfg.client, l, strlen(l+lm), l+lm, 1);

						wod = json_object_object_get( wo, "main" );
						struct json_object *swod = json_object_object_get( wod, "temp");
						lm = sprintf(l, "%s/%d/temperature", ctx->topic, i) + 2;
						assert( lm+1 < MAXLINE-10 );
						sprintf( l+lm, "%.2lf", json_object_get_double(swod));
						mqttpublish( cfg.client, l, strlen(l+lm), l+lm, 1);

						swod = json_object_object_get( wod, "pressure");
						lm = sprintf(l, "%s/%d/pressure", ctx->topic, i) + 2;
						assert( lm+1 < MAXLINE-10 );
						sprintf( l+lm, "%.2lf", json_object_get_double(swod));
						mqttpublish( cfg.client, l, strlen(l+lm), l+lm, 1);

						swod = json_object_object_get( wod, "humidity");
						lm = sprintf(l, "%s/%d/humidity", ctx->topic, i) + 2;
						assert( lm+1 < MAXLINE-10 );
						sprintf( l+lm, "%d", json_object_get_int(swod));
						mqttpublish( cfg.client, l, strlen(l+lm), l+lm, 1);

					}
				}
			}
			json_object_put(jobj);
		} else
			fprintf(stderr, "*E* Querying meteo : %s\n", curl_easy_strerror(res));

			/* Cleanup */
		curl_easy_cleanup(curl);
		free(chunk.memory);
	}
}

void *process_Meteo3H(void *actx){
	Meteo3H(actx);
	pthread_exit(0);
}

#endif
