/* Weather forecast using OpenWeatherMap
 * Process 3 hours forcast
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

static const char URLMETEO3H[]="http://api.openweathermap.org/data/2.5/forecast?q=%s&mode=json&units=%s&lang=%s&appid=%s";

static void Meteo3H(struct section_OWMQuery *ctx){
	CURL *curl;
	enum json_tokener_error jerr = json_tokener_success;

	publishLog('T', "Querying Meteo 3H '%s'", ctx->section.uid);
	ctx->inerror = true;	/* By default, something failed */

	if((curl = curl_easy_init())){
		char url[strlen(URLMETEO3H) + strlen(ctx->city) + strlen(ctx->units) + strlen(ctx->lang) + strlen(mod_owm.apikey)];	/* Thanks to %s, Some room left for \0 */
		CURLcode res;
		struct MemoryStruct chunk;

		chunk.memory = malloc(1);
		chunk.size = 0;

		sprintf(url, URLMETEO3H, ctx->city, ctx->units, ctx->lang, mod_owm.apikey);
#if 0
		curl_easy_setopt(curl, CURLOPT_URL, "file:////home/laurent/Projets/Marcel/meteo_tst.json");
#else
		curl_easy_setopt(curl, CURLOPT_URL, url);
#endif
		curl_easy_setopt(curl, CURLOPT_USERAGENT, "Marcel/" MARCEL_VERSION);

		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
		curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);

		if((res = curl_easy_perform(curl)) == CURLE_OK){	/* Processing data */
			json_object * jobj = json_tokener_parse_verbose(chunk.memory, &jerr);
			if(jerr != json_tokener_success)
				publishLog('E', "[%s] Querying meteo : %s", ctx->section.uid, json_tokener_error_desc(jerr));
			else {
				struct json_object *wlist;
				ctx->inerror = false;	/* ok, seems we succeeded */

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
						} else {
							publishLog('E', "[%s] Querying meteo : no dt", ctx->section.uid);
							ctx->inerror = true;
						}

						if( json_object_object_get_ex( wo, "main", &wod ) ){
							struct json_object *swod;
							if( json_object_object_get_ex( wod, "temp", &swod) ){
								int lm = sprintf(l, "%s/%d/temperature", ctx->section.topic, i) + 2;
								assert( lm+1 < MAXLINE-10 );
								sprintf( l+lm, "%.2lf", json_object_get_double(swod));
								mqttpublish( cfg.client, l, strlen(l+lm), l+lm, 1);
							} else {
								publishLog('E', "[%s] Querying meteo : no main/temp", ctx->section.uid);
								ctx->inerror = true;
							}

							if( json_object_object_get_ex( wod, "pressure", &swod) ){
								int lm = sprintf(l, "%s/%d/pressure", ctx->section.topic, i) + 2;
								assert( lm+1 < MAXLINE-10 );
								sprintf( l+lm, "%.2lf", json_object_get_double(swod));
								mqttpublish( cfg.client, l, strlen(l+lm), l+lm, 1);
							} else {
								publishLog('E', "[%s] Querying meteo : no main/pressure", ctx->section.uid);
								ctx->inerror = true;
							}

							if( json_object_object_get_ex( wod, "humidity", &swod) ){
								int lm = sprintf(l, "%s/%d/humidity", ctx->section.topic, i) + 2;
								assert( lm+1 < MAXLINE-10 );
								sprintf( l+lm, "%d", json_object_get_int(swod));
								mqttpublish( cfg.client, l, strlen(l+lm), l+lm, 1);
							} else {
								publishLog('E', "[%s] Querying meteo : no main/humidity", ctx->section.uid);
								ctx->inerror = true;
							}
						} else {
							publishLog('E', "[%s] Querying meteo : no main", ctx->section.uid);
							ctx->inerror = true;
						}

						if( json_object_object_get_ex( wo, "weather", &wod ) ){
							wod = json_object_array_get_idx( wod, 0 );
							struct json_object *swod;
							if( json_object_object_get_ex( wod, "description", &swod) ){
								int lm = sprintf(l, "%s/%d/weather/description", ctx->section.topic, i) + 2;
								const char *ts = json_object_get_string(swod);
								assert( lm+1 < MAXLINE-strlen(ts) );
								sprintf( l+lm, "%s", ts);
								mqttpublish( cfg.client, l, strlen(l+lm), l+lm, 1);
							} else {
								publishLog('E', "[%s] Querying meteo : no weather/description", ctx->section.uid);
								ctx->inerror = true;
							}

							if( json_object_object_get_ex( wod, "icon", &swod) ){
								int lm = sprintf(l, "%s/%d/weather/code", ctx->section.topic, i) + 2;
								const char *ts = json_object_get_string(swod);
								assert( lm+1 < MAXLINE-strlen(ts) );
								sprintf( l+lm, "%s", ts);
								mqttpublish( cfg.client, l, strlen(l+lm), l+lm, 1);
								int dayornight = (ts[2] == 'd');

									/* Accurate weathear icon */
								if( json_object_object_get_ex( wod, "id", &swod) ){
									lm = sprintf(l, "%s/%d/weather/acode", ctx->section.topic, i) + 2;
									assert( lm+1 < MAXLINE-10 );
									sprintf( l+lm, "%d", convWCode(json_object_get_int(swod), dayornight) );
									mqttpublish( cfg.client, l, strlen(l+lm), l+lm, 1);
								} else {
									publishLog('E', "[%s] Querying meteo : no weather/id", ctx->section.uid);
									ctx->inerror = true;
								}
							} else {
								publishLog('E', "[%s] Querying meteo : no weather/icon", ctx->section.uid);
								ctx->inerror = true;
							}
						} else {
							publishLog('E', "[%s] Querying meteo : no weather", ctx->section.uid);
							ctx->inerror = true;
						}

						if( json_object_object_get_ex( wo, "clouds", &wod ) ){
							struct json_object *swod;
							if( json_object_object_get_ex( wod, "all", &swod) ){
								int lm = sprintf(l, "%s/%d/clouds", ctx->section.topic, i) + 2;
								assert( lm+1 < MAXLINE-10 );
								sprintf( l+lm, "%d", json_object_get_int(swod));
								mqttpublish( cfg.client, l, strlen(l+lm), l+lm, 1);
							} else {
								publishLog('E', "[%s] Querying meteo : no clouds/all", ctx->section.uid);
								ctx->inerror = true;
							}
						} else {
							publishLog('E', "[%s] Querying meteo : no clouds", ctx->section.uid);
							ctx->inerror = true;
						}


						if( json_object_object_get_ex( wo, "wind", &wod ) ){
							struct json_object *swod;
							if( json_object_object_get_ex( wod, "speed", &swod) ){
								int lm = sprintf(l, "%s/%d/wind/speed", ctx->section.topic, i) + 2;
								assert( lm+1 < MAXLINE-10 );
								sprintf( l+lm, "%.2lf", json_object_get_double(swod));
								mqttpublish( cfg.client, l, strlen(l+lm), l+lm, 1);
							} else {
								publishLog('E', "[%s] Querying meteo : no wind/speed", ctx->section.uid);
								ctx->inerror = true;
							}


							if( json_object_object_get_ex( wod, "deg", &swod) ){
								int lm = sprintf(l, "%s/%d/wind/direction", ctx->section.topic, i) + 2;
								assert( lm+1 < MAXLINE-10 );
								sprintf( l+lm, "%.2lf", json_object_get_double(swod));
								mqttpublish( cfg.client, l, strlen(l+lm), l+lm, 1);
							} else {
								publishLog('E', "[%s] Querying meteo : no wind/direction", ctx->section.uid);
								ctx->inerror = true;
							}
						} else {
							publishLog('E', "[%s] Querying meteo : no wind", ctx->section.uid);
							ctx->inerror = true;
						}
					}
				} else {
					publishLog('E', "[%s] Querying meteo : Bad response from server (no list object)", ctx->section.uid);
					ctx->inerror = true;
				}
			}
			publishLog('T', "[%s] meteo published", ctx->section.uid);
			json_object_put(jobj);
		} else {
			publishLog('E', "[%s] Querying meteo : %s", ctx->section.uid, curl_easy_strerror(res));
			ctx->inerror = true;
		}

			/* Cleanup */
		curl_easy_cleanup(curl);
		free(chunk.memory);
	}
}

void *processWF3H(void *actx){
	struct section_OWMQuery *s = (struct section_OWMQuery *)actx;
	s->inerror = true;

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
			s->inerror = false;
#ifdef DEBUG
			if(cfg.debug)
				publishLog('d', "[%s] is disabled", s->section.uid);
#endif
		} else if( !first || s->section.immediate )
			Meteo3H(s);

		struct timespec ts;
		ts.tv_sec = (time_t)s->section.sample;
		ts.tv_nsec = (unsigned long int)((s->section.sample - (time_t)s->section.sample) * 1e9);

		nanosleep( &ts, NULL );
	}
}
