/* Meteo.c
 *	Publish meteo information.
 *
 *	Based on OpenWeatherMap.org information
 *
 * 29/11/2015 - Add /weather/acode as OWM's icon code is not accurate
 */

#ifndef METEO_H
#include "Meteo.h"
#include "MQTT_tools.h"
#include "CURL_helpers.h"

#include <curl/curl.h>
#include <stdlib.h>		/* memory */
#include <string.h>		/* memcpy() */
#include <assert.h>
#include <unistd.h>		/* sleep() */
#include <json-c/json.h>

static short int doy=-1;	/* Day of the year ... do we switched to another day ? */

	/* Convert weather condition to accurate code */
static int convWCode( int code, int dayornight ){
	if( code >=200 && code < 300 )
		return 0;
	else if( code >= 300 && code <= 312 )
		return 9;
	else if( code > 312 && code < 400 )
		return( dayornight ? 39:45 );
	else if( code == 500 || code == 501 )
		return 11;
	else if( code >= 502 && code <= 504 )
		return 12;
	else if( code == 511 )
		return 10;
	else if( code >= 520 && code <= 529 )
		return( dayornight ? 39:45 );
	else if( code == 600)
		return 13;
	else if( code > 600 && code < 610 )
		return 14;
	else if( code == 612 || (code >= 620 && code < 630) )
		return( dayornight ? 41:46 );
	else if( code >= 610 && code < 620 )
		return 5;
	else if( code >= 700 && code < 800 )
		return 21;
	else if( code == 800 )
		return( dayornight ? 32:31 );
	else if( code == 801 )
		return( dayornight ? 34:33 );
	else if( code == 802 )
		return( dayornight ? 30:29 );
	else if( code == 803 || code == 804 )
		return( dayornight ? 28:27 );

	return -1;
}

static void Meteo3H(struct _Meteo *ctx){
	CURL *curl;
	enum json_tokener_error jerr = json_tokener_success;

	if(verbose)
		puts("*D* Querying Meteo 3H");

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
		curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);

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

						wod = json_object_object_get( wo, "weather" );
						wod = json_object_array_get_idx( wod, 0 );
						swod = json_object_object_get( wod, "description");
						lm = sprintf(l, "%s/%d/weather/description", ctx->topic, i) + 2;
						const char *ts = json_object_get_string(swod);
						assert( lm+1 < MAXLINE-strlen(ts) );
						sprintf( l+lm, "%s", ts);
						mqttpublish( cfg.client, l, strlen(l+lm), l+lm, 1);

						swod = json_object_object_get( wod, "icon");
						lm = sprintf(l, "%s/%d/weather/code", ctx->topic, i) + 2;
						ts = json_object_get_string(swod);
						assert( lm+1 < MAXLINE-strlen(ts) );
						sprintf( l+lm, "%s", ts);
						mqttpublish( cfg.client, l, strlen(l+lm), l+lm, 1);
						int dayornight = (ts[2] == 'd');

							/* Accurate weathear icon */
						swod = json_object_object_get( wod, "id");
						lm = sprintf(l, "%s/%d/weather/acode", ctx->topic, i) + 2;
						assert( lm+1 < MAXLINE-10 );
						sprintf( l+lm, "%d", convWCode(json_object_get_int(swod), dayornight) );
						mqttpublish( cfg.client, l, strlen(l+lm), l+lm, 1);

						wod = json_object_object_get( wo, "clouds" );
						swod = json_object_object_get( wod, "all");
						lm = sprintf(l, "%s/%d/clouds", ctx->topic, i) + 2;
						assert( lm+1 < MAXLINE-10 );
						sprintf( l+lm, "%d", json_object_get_int(swod));
						mqttpublish( cfg.client, l, strlen(l+lm), l+lm, 1);

						wod = json_object_object_get( wo, "wind" );
						swod = json_object_object_get( wod, "speed");
						lm = sprintf(l, "%s/%d/wind/speed", ctx->topic, i) + 2;
						assert( lm+1 < MAXLINE-10 );
						sprintf( l+lm, "%.2lf", json_object_get_double(swod));
						mqttpublish( cfg.client, l, strlen(l+lm), l+lm, 1);

						swod = json_object_object_get( wod, "deg");
						lm = sprintf(l, "%s/%d/wind/direction", ctx->topic, i) + 2;
						assert( lm+1 < MAXLINE-10 );
						sprintf( l+lm, "%.2lf", json_object_get_double(swod));
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
	if(verbose)
		printf("Launching a processing flow for Meteo 3H\n");

	for(;;){
		Meteo3H(actx);
		sleep( ((struct _Meteo *)actx)->sample);
	}

	pthread_exit(0);
}

static void MeteoD(struct _Meteo *ctx){
	CURL *curl;
	enum json_tokener_error jerr = json_tokener_success;

	if(verbose)
		puts("*D* Querying Meteo daily");

	if((curl = curl_easy_init())){
		char url[strlen(URLMETEOD) + strlen(ctx->City) + strlen(ctx->Units) + strlen(ctx->Lang)];	/* Thanks to %s, Some room left for \0 */
		/* Note : URLMETEOD _has_ to be longer than URLMETEOCUR */

		CURLcode res;
		struct MemoryStruct chunk;

		chunk.memory = malloc(1);
		chunk.size = 0;

		sprintf(url, URLMETEOD, ctx->City, ctx->Units, ctx->Lang);
		curl_easy_setopt(curl, CURLOPT_URL, url);
		curl_easy_setopt(curl, CURLOPT_USERAGENT, "Marcel/" VERSION);

		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);

		if((res = curl_easy_perform(curl)) == CURLE_OK){	/* Processing data */
			json_object * jobj = json_tokener_parse_verbose(chunk.memory, &jerr);
			if(jerr != json_tokener_success)
				fprintf(stderr, "*E* Querying meteo daily : %s\n", json_tokener_error_desc(jerr));
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

						wod = json_object_object_get( wo, "temp" );
						struct json_object *swod = json_object_object_get( wod, "day");
						lm = sprintf(l, "%s/%d/temperature/day", ctx->topic, i) + 2;
						assert( lm+1 < MAXLINE-10 );
						sprintf( l+lm, "%.2lf", json_object_get_double(swod));
						mqttpublish( cfg.client, l, strlen(l+lm), l+lm, 1);

						swod = json_object_object_get( wod, "night");
						lm = sprintf(l, "%s/%d/temperature/night", ctx->topic, i) + 2;
						assert( lm+1 < MAXLINE-10 );
						sprintf( l+lm, "%.2lf", json_object_get_double(swod));
						mqttpublish( cfg.client, l, strlen(l+lm), l+lm, 1);

						swod = json_object_object_get( wod, "eve");
						lm = sprintf(l, "%s/%d/temperature/evening", ctx->topic, i) + 2;
						assert( lm+1 < MAXLINE-10 );
						sprintf( l+lm, "%.2lf", json_object_get_double(swod));
						mqttpublish( cfg.client, l, strlen(l+lm), l+lm, 1);

						swod = json_object_object_get( wod, "morn");
						lm = sprintf(l, "%s/%d/temperature/morning", ctx->topic, i) + 2;
						assert( lm+1 < MAXLINE-10 );
						sprintf( l+lm, "%.2lf", json_object_get_double(swod));
						mqttpublish( cfg.client, l, strlen(l+lm), l+lm, 1);

						wod = json_object_object_get( wo, "weather" );
						wod = json_object_array_get_idx( wod, 0 );
						swod = json_object_object_get( wod, "description");
						lm = sprintf(l, "%s/%d/weather/description", ctx->topic, i) + 2;
						const char *ts = json_object_get_string(swod);
						assert( lm+1 < MAXLINE-strlen(ts) );
						sprintf( l+lm, "%s", ts);
						mqttpublish( cfg.client, l, strlen(l+lm), l+lm, 1);

						swod = json_object_object_get( wod, "icon");
						lm = sprintf(l, "%s/%d/weather/code", ctx->topic, i) + 2;
						ts = json_object_get_string(swod);
						assert( lm+1 < MAXLINE-strlen(ts) );
						sprintf( l+lm, "%s", ts);
						mqttpublish( cfg.client, l, strlen(l+lm), l+lm, 1);
						int dayornight = (ts[2] == 'd');

							/* Accurate weathear icon */
						swod = json_object_object_get( wod, "id");
						lm = sprintf(l, "%s/%d/weather/acode", ctx->topic, i) + 2;
						assert( lm+1 < MAXLINE-10 );
						sprintf( l+lm, "%d", convWCode(json_object_get_int(swod), dayornight) );
						mqttpublish( cfg.client, l, strlen(l+lm), l+lm, 1);

						wod = json_object_object_get( wo, "pressure" );
						lm = sprintf(l, "%s/%d/pressure", ctx->topic, i) + 2;
						assert( lm+1 < MAXLINE-10 );
						sprintf( l+lm, "%.2lf", json_object_get_double(wod));
						mqttpublish( cfg.client, l, strlen(l+lm), l+lm, 1);

						wod = json_object_object_get( wo, "clouds" );
						lm = sprintf(l, "%s/%d/clouds", ctx->topic, i) + 2;
						assert( lm+1 < MAXLINE-10 );
						sprintf( l+lm, "%d", json_object_get_int(wod));
						mqttpublish( cfg.client, l, strlen(l+lm), l+lm, 1);

						wod = json_object_object_get( wo, "humidity" );
						lm = sprintf(l, "%s/%d/humidity", ctx->topic, i) + 2;
						assert( lm+1 < MAXLINE-10 );
						sprintf( l+lm, "%d", json_object_get_int(wod));
						mqttpublish( cfg.client, l, strlen(l+lm), l+lm, 1);

						wod = json_object_object_get( wo, "speed" );
						lm = sprintf(l, "%s/%d/wind/speed", ctx->topic, i) + 2;
						assert( lm+1 < MAXLINE-10 );
						sprintf( l+lm, "%.2lf", json_object_get_double(wod));
						mqttpublish( cfg.client, l, strlen(l+lm), l+lm, 1);

						wod = json_object_object_get( wo, "deg" );
						lm = sprintf(l, "%s/%d/wind/direction", ctx->topic, i) + 2;
						assert( lm+1 < MAXLINE-10 );
						sprintf( l+lm, "%d", json_object_get_int(wod));
						mqttpublish( cfg.client, l, strlen(l+lm), l+lm, 1);
					}
				}
			}
			json_object_put(jobj);
		} else
			fprintf(stderr, "*E* Querying meteo : %s\n", curl_easy_strerror(res));

			/* Check if we switched to another day */
		time_t now;
		struct tm tmt;
		time(&now);
		localtime_r(&now, &tmt);
		if(doy != tmt.tm_yday){
			doy = tmt.tm_yday;

			if(verbose)
				puts("*D* Querying current meteo");

			free(chunk.memory);
			chunk.memory = malloc(1);
			chunk.size = 0;
			
			sprintf(url, URLMETEOCUR, ctx->City, ctx->Units, ctx->Lang);
			curl_easy_setopt(curl, CURLOPT_URL, url);

			if((res = curl_easy_perform(curl)) == CURLE_OK){	/* Processing data */
				json_object * jobj = json_tokener_parse_verbose(chunk.memory, &jerr);
				if(jerr != json_tokener_success)
					fprintf(stderr, "*E* Querying current meteo : %s\n", json_tokener_error_desc(jerr));
				else {
					struct json_object *wsys = json_object_object_get( jobj, "sys");
					if(wsys){
						char l[MAXLINE];
						struct json_object *wdt = json_object_object_get(wsys, "sunrise");
						now = json_object_get_int(wdt);

						int lm = sprintf(l, "%s/sunrise", ctx->topic) + 2;
						assert( lm+1 < MAXLINE-10 );
						localtime_r(&now, &tmt);
						sprintf( l+lm, "%02u:%02u", tmt.tm_hour, tmt.tm_min);
						mqttpublish( cfg.client, l, strlen(l+lm), l+lm, 1);
						
						lm = sprintf(l, "%s/sunrise/GMT", ctx->topic) + 2;
						assert( lm+1 < MAXLINE-10 );
						gmtime_r(&now, &tmt);
						sprintf( l+lm, "%02u:%02u", tmt.tm_hour, tmt.tm_min);
						mqttpublish( cfg.client, l, strlen(l+lm), l+lm, 1);
						
						wdt = json_object_object_get(wsys, "sunset");
						now = json_object_get_int(wdt);

						lm = sprintf(l, "%s/sunset", ctx->topic) + 2;
						assert( lm+1 < MAXLINE-10 );
						localtime_r(&now, &tmt);
						sprintf( l+lm, "%02u:%02u", tmt.tm_hour, tmt.tm_min);
						mqttpublish( cfg.client, l, strlen(l+lm), l+lm, 1);
						
						lm = sprintf(l, "%s/sunset/GMT", ctx->topic) + 2;
						assert( lm+1 < MAXLINE-10 );
						gmtime_r(&now, &tmt);
						sprintf( l+lm, "%02u:%02u", tmt.tm_hour, tmt.tm_min);
						mqttpublish( cfg.client, l, strlen(l+lm), l+lm, 1);
					}
					json_object_put(jobj);
				}
			}
		}

			/* Cleanup */
		curl_easy_cleanup(curl);
		free(chunk.memory);
	}
}


void *process_MeteoD(void *actx){
	if(verbose)
		printf("Launching a processing flow for Meteo Daily\n");

	for(;;){
		MeteoD(actx);
		sleep( ((struct _Meteo *)actx)->sample);
	}

	pthread_exit(0);
}

#endif
