/* Weather forecast using OpenWeatherMap
 * Process daily request
 *  
 * This file is part of Marcel project and is following the same
 * license rules (see LICENSE file)
 *
 * 03/11/2022 - LF - Move to V8
 */

#include "mod_owm.h"
#include "../Marcel/MQTT_tools.h"
#include "../Marcel/Version.h"
#include "../Marcel/CURL_helpers.h"

#include <curl/curl.h>
#include <json-c/json.h>

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>

static short int doy=-1;	/* Day of the year ... do we switched to another day ? */

static const char URLMETEOD[] = "http://api.openweathermap.org/data/2.5/forecast/daily?q=%s&mode=json&units=%s&lang=%s&appid=%s";
static const char URLMETEOCUR[] = "http://api.openweathermap.org/data/2.5/weather?q=%s&mode=json&units=%s&lang=%s&appid=%s";

		/* Note : URLMETEOD _has_ to be longer than URLMETEOCUR */

static void MeteoD(struct section_OWMQuery *ctx){
	CURL *curl;
	enum json_tokener_error jerr = json_tokener_success;

	publishLog('T', "Querying Meteo Daily '%s'", ctx->section.uid);

	if((curl = curl_easy_init())){
		char url[strlen(URLMETEOD) + strlen(ctx->city) + strlen(ctx->units) + strlen(ctx->lang) + strlen(mod_owm.apikey)];	/* Thanks to %s, Some room left for \0 */

		CURLcode res;
		struct MemoryStruct chunk;

		chunk.memory = malloc(1);
		chunk.size = 0;

		sprintf(url, URLMETEOD, ctx->city, ctx->units, ctx->lang, mod_owm.apikey);
puts(url);
		curl_easy_setopt(curl, CURLOPT_URL, url);
		curl_easy_setopt(curl, CURLOPT_USERAGENT, "Marcel/" MARCEL_VERSION);

		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);

		if((res = curl_easy_perform(curl)) == CURLE_OK){	/* Processing data */
			json_object * jobj = json_tokener_parse_verbose(chunk.memory, &jerr);
			if(jerr != json_tokener_success)
				publishLog('E', "[%s] Querying meteo daily: %s", ctx->section.uid, json_tokener_error_desc(jerr));
			else {
				struct json_object *wlist;
				if( json_object_object_get_ex( jobj, "list", &wlist) ){
					int nbre = json_object_array_length(wlist);
					char l[MAXLINE];
					for(int i=0; i<nbre; i++){
						struct json_object *wo = json_object_array_get_idx( wlist, i );	/* Weather's object */
						struct json_object *wod;
						if( json_object_object_get_ex( wo, "dt", &wod) ){	/* Weather's data */
							int lm = sprintf(l, "%s/%d/time", ctx->section.topic, i) + 2;
							assert( lm+1 < MAXLINE-10 );
							sprintf( l+lm, "%u", json_object_get_int(wod));
							mqttpublish( cfg.client, l, strlen(l+lm), l+lm, 1);
						} else
							publishLog('E', "[%s] Querying meteo : no dt", ctx->section.uid);

						if( json_object_object_get_ex( wo, "temp", &wod ) ){
							struct json_object *swod;
							if( json_object_object_get_ex( wod, "day", &swod) ){
								int lm = sprintf(l, "%s/%d/temperature/day", ctx->section.topic, i) + 2;
								assert( lm+1 < MAXLINE-10 );
								sprintf( l+lm, "%.2lf", json_object_get_double(swod));
								mqttpublish( cfg.client, l, strlen(l+lm), l+lm, 1);
							} else
								publishLog('E', "[%s] Querying meteo : no temp/day", ctx->section.uid);

							if( json_object_object_get_ex( wod, "night", &swod) ){
								int lm = sprintf(l, "%s/%d/temperature/night", ctx->section.topic, i) + 2;
								assert( lm+1 < MAXLINE-10 );
								sprintf( l+lm, "%.2lf", json_object_get_double(swod));
								mqttpublish( cfg.client, l, strlen(l+lm), l+lm, 1);
							} else
								publishLog('E', "[%s] Querying meteo : no temp/night", ctx->section.uid);

							if( json_object_object_get_ex( wod, "eve", &swod) ){
								int lm = sprintf(l, "%s/%d/temperature/evening", ctx->section.topic, i) + 2;
								assert( lm+1 < MAXLINE-10 );
								sprintf( l+lm, "%.2lf", json_object_get_double(swod));
								mqttpublish( cfg.client, l, strlen(l+lm), l+lm, 1);
							} else
								publishLog('E', "[%s] Querying meteo : no temp/eve", ctx->section.uid);

							if( json_object_object_get_ex( wod, "morn", &swod) ){
								int lm = sprintf(l, "%s/%d/temperature/morning", ctx->section.topic, i) + 2;
								assert( lm+1 < MAXLINE-10 );
								sprintf( l+lm, "%.2lf", json_object_get_double(swod));
								mqttpublish( cfg.client, l, strlen(l+lm), l+lm, 1);
							} else
								publishLog('E', "[%s] Querying meteo : no temp/morn", ctx->section.uid);
						} else
							publishLog('E', "[%s] Querying meteo : no temp", ctx->section.uid);

						if( json_object_object_get_ex( wo, "weather", &wod) ){
							wod = json_object_array_get_idx( wod, 0 );
							struct json_object *swod;
							if( json_object_object_get_ex( wod, "description", &swod) ){
								int lm = sprintf(l, "%s/%d/weather/description", ctx->section.topic, i) + 2;
								const char *ts = json_object_get_string(swod);
								assert( lm+1 < MAXLINE-strlen(ts) );
								sprintf( l+lm, "%s", ts);
								mqttpublish( cfg.client, l, strlen(l+lm), l+lm, 1);
							} else
								publishLog('E', "[%s] Querying meteo : no weather/description", ctx->section.uid);

							if( json_object_object_get_ex( wod, "icon", &swod) ){
								int lm = sprintf(l, "%s/%d/weather/code", ctx->section.topic, i) + 2;
								const char *ts = json_object_get_string(swod);
								assert( lm+1 < MAXLINE-strlen(ts) );
								sprintf( l+lm, "%s", ts);
								mqttpublish( cfg.client, l, strlen(l+lm), l+lm, 1);
								int dayornight = (ts[2] == 'd');

									/* Accurate weathear icon */
								if( json_object_object_get_ex( wod, "id", &swod) ){
									int lm = sprintf(l, "%s/%d/weather/acode", ctx->section.topic, i) + 2;
									assert( lm+1 < MAXLINE-10 );
									sprintf( l+lm, "%d", convWCode(json_object_get_int(swod), dayornight) );
									mqttpublish( cfg.client, l, strlen(l+lm), l+lm, 1);
								} else
									publishLog('E', "[%s] Querying meteo : no weather/id", ctx->section.uid);
							} else
								publishLog('E', "[%s] Querying meteo : no weather/icon", ctx->section.uid);
						} else
							publishLog('E', "[%s] Querying meteo : no weather", ctx->section.uid);


						if( json_object_object_get_ex( wo, "pressure", &wod) ){
							int lm = sprintf(l, "%s/%d/pressure", ctx->section.topic, i) + 2;
							assert( lm+1 < MAXLINE-10 );
							sprintf( l+lm, "%.2lf", json_object_get_double(wod));
							mqttpublish( cfg.client, l, strlen(l+lm), l+lm, 1);
						} else
							publishLog('E', "[%s] Querying meteo : no pressure", ctx->section.uid);

						if( json_object_object_get_ex( wo, "clouds", &wod) ){
							int lm = sprintf(l, "%s/%d/clouds", ctx->section.topic, i) + 2;
							assert( lm+1 < MAXLINE-10 );
							sprintf( l+lm, "%d", json_object_get_int(wod));
							mqttpublish( cfg.client, l, strlen(l+lm), l+lm, 1);
						} else
							publishLog('E', "[%s] Querying meteo : no clouds", ctx->section.uid);

						if( json_object_object_get_ex( wo, "snow", &wod) ){
							int lm = sprintf(l, "%s/%d/snow", ctx->section.topic, i) + 2;
							assert( lm+1 < MAXLINE-10 );
							sprintf( l+lm, "%d", json_object_get_int(wod));
							mqttpublish( cfg.client, l, strlen(l+lm), l+lm, 1);
						} else {
							int lm = sprintf(l, "%s/%d/snow", ctx->section.topic, i) + 2;
							assert( lm+1 < MAXLINE-10 );
							*(l+lm++) = '0';
							*(l+lm) = 0;
							mqttpublish( cfg.client, l, strlen(l+lm), l+lm, 1);
						}

						if( json_object_object_get_ex( wo, "speed", &wod) ){
							int lm = sprintf(l, "%s/%d/wind/speed", ctx->section.topic, i) + 2;
							assert( lm+1 < MAXLINE-10 );
							sprintf( l+lm, "%.2lf", json_object_get_double(wod));
							mqttpublish( cfg.client, l, strlen(l+lm), l+lm, 1);
						} else
								publishLog('E', "[%s] Querying meteo : no speed", ctx->section.uid);

						if( json_object_object_get_ex( wo, "deg", &wod) ){
							int lm = sprintf(l, "%s/%d/wind/direction", ctx->section.topic, i) + 2;
							assert( lm+1 < MAXLINE-10 );
							sprintf( l+lm, "%d", json_object_get_int(wod));
							mqttpublish( cfg.client, l, strlen(l+lm), l+lm, 1);
						} else
								publishLog('E', "[%s] Querying meteo : no deg", ctx->section.uid);
					}
				} else
					publishLog('E', "[%s] Querying meteo : no list", ctx->section.uid);
			}
			json_object_put(jobj);
		} else
			publishLog('E', "[%s] Querying meteo : %s", ctx->section.uid, curl_easy_strerror(res));

			/* Check if we switched to another day */
		time_t now;
		struct tm tmt;
		time(&now);
		localtime_r(&now, &tmt);
		if(doy != tmt.tm_yday){
			doy = tmt.tm_yday;

			publishLog('T', "[%s] Querying current meteo", ctx->section.uid);

			free(chunk.memory);
			chunk.memory = malloc(1);
			chunk.size = 0;
			
			sprintf(url, URLMETEOCUR, ctx->city, ctx->units, ctx->lang, mod_owm.apikey);
			curl_easy_setopt(curl, CURLOPT_URL, url);

			if((res = curl_easy_perform(curl)) == CURLE_OK){	/* Processing data */
				json_object * jobj = json_tokener_parse_verbose(chunk.memory, &jerr);
				if(jerr != json_tokener_success)
					publishLog('E', "[%s] Querying current meteo : %s", ctx->section.uid, json_tokener_error_desc(jerr));
				else {
					struct json_object *wsys;
					if( json_object_object_get_ex(jobj, "sys", &wsys) ){
						char l[MAXLINE];

						struct json_object *wdt;
						if( json_object_object_get_ex(wsys, "sunrise", &wdt ) ){
							now = json_object_get_int(wdt);

							int lm = sprintf(l, "%s/sunrise", ctx->section.topic) + 2;
							assert( lm+1 < MAXLINE-10 );
							localtime_r(&now, &tmt);
							sprintf( l+lm, "%02u:%02u", tmt.tm_hour, tmt.tm_min);
							mqttpublish( cfg.client, l, strlen(l+lm), l+lm, 1);
						
							lm = sprintf(l, "%s/sunrise/GMT", ctx->section.topic) + 2;
							assert( lm+1 < MAXLINE-10 );
							gmtime_r(&now, &tmt);
							sprintf( l+lm, "%02u:%02u", tmt.tm_hour, tmt.tm_min);
							mqttpublish( cfg.client, l, strlen(l+lm), l+lm, 1);
							publishLog('T', "[%s] sunrise published", ctx->section.uid);
						} else
							publishLog('E', "[%s] Querying meteo : no sunrise", ctx->section.uid);
						
						if( json_object_object_get_ex(wsys, "sunset", &wdt ) ){
							now = json_object_get_int(wdt);

							int lm = sprintf(l, "%s/sunset", ctx->section.topic) + 2;
							assert( lm+1 < MAXLINE-10 );
							localtime_r(&now, &tmt);
							sprintf( l+lm, "%02u:%02u", tmt.tm_hour, tmt.tm_min);
							mqttpublish( cfg.client, l, strlen(l+lm), l+lm, 1);
						
							lm = sprintf(l, "%s/sunset/GMT", ctx->section.topic) + 2;
							assert( lm+1 < MAXLINE-10 );
							gmtime_r(&now, &tmt);
							sprintf( l+lm, "%02u:%02u", tmt.tm_hour, tmt.tm_min);
							mqttpublish( cfg.client, l, strlen(l+lm), l+lm, 1);
							publishLog('T', "[%s] sunset published", ctx->section.uid);
						} else
							publishLog('E', "[%s] Querying meteo : no sunset", ctx->section.uid);
					} else
						publishLog('E', "[%s] Querying meteo : Bad response from server (no sys object)", ctx->section.uid);
				}
				publishLog('T', "[%s] current meteo published", ctx->section.uid);
				json_object_put(jobj);
			} else
				publishLog('E', "[%s] Querying meteo : %s", ctx->section.uid, curl_easy_strerror(res));
		}

			/* Cleanup */
		curl_easy_cleanup(curl);
		free(chunk.memory);
	}
}

void *processWFDaily(void *actx){
	struct section_OWMQuery *s = (struct section_OWMQuery *)actx;

		/* Sanity checks */
	if(!s->section.topic){
		publishLog('F', "[%s] Topic must be set. Dying ...", s->section.uid);
		pthread_exit(0);
	}

	if(s->section.sample < 600){
		publishLog('E', "[%s] Sample time can't be less than 600. Dying ...", s->section.uid);
		pthread_exit(0);
	}

	if(!s->city){
		publishLog('E', "[%s] City not specified. Dying ...", s->section.uid);
		pthread_exit(0);
	}

	publishLog('I', "Launching a processing flow for Meteo Daily '%s'", s->section.uid);

	for(bool first=true;; first=false){	/* Infinite publishing loop */
		if(s->section.disabled){
#ifdef DEBUG
			if(cfg.debug)
				publishLog('d', "[%s] is disabled", s->section.uid);
#endif
		} else if( !first || s->section.immediate )
			MeteoD(s);

		struct timespec ts;
		ts.tv_sec = (time_t)s->section.sample;
		ts.tv_nsec = (unsigned long int)((s->section.sample - (time_t)s->section.sample) * 1e9);

		nanosleep( &ts, NULL );
	}
}
