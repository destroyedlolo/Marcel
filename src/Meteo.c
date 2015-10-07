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

	/* structure to decode json information */

struct jsonstatus {
	unsigned short int blocklevel;
	unsigned short int tablelevel;
	char instring;
};

char *readlabel(char **p){
	if(**p == '"'){
		char *lbl = ++(*p);
		while( **p != '"' && **p)
			++(*p);
		if(**p){
			**p = 0;
			(*p)++;
			return lbl;
		} else	/* End of data */
			return NULL;
	} else {
		if(verbose)
			fprintf(stderr, "*E* readlabel(), expecting '\"' but got '%s'\n", *p);
		return NULL;
	}
}

int isddigit( char c ){
	return( isdigit(c) || c == '.' );
}

char *skipdata(char *p){
	if(!*p)
		return NULL;
	else if(*p == '"'){	/* reading a string */
		while(*++p != '"'){
			if(!*p) return NULL;
		}
		return ++p;
	} else if(isddigit(*p)){	/* Reading a number */
		while(isddigit(*++p)){
			if(!*p) return NULL;
		}
		return p;
	} else {
		fprintf(stderr, "*E* skipdata() : Unknown character '%s'\n", p);
		return NULL;
	}
}

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
			char *p = chunk.memory;

			while(*p != '{'){	/* search block beginning */
				if(!*p)	/* end of data */
					goto allread;
				p++;
			}
			p++;

			for(;;){
				char *lbl = readlabel(&p);
				if(!lbl)
					goto allread;
printf("lbl: '%s'\n", lbl);
				if(*p != ':'){
					fprintf(stderr, "*E* Label must be followed by a ':', got '%s'\n", p);
					goto allread;
				}
				p++;	/* skip ':' */
				if(!strcmp(lbl, "list"))	/* Label found */
					break;
				if(!(p = skipdata(p)))
					goto allread;
printf("skip : '%s'\n", p);
			}
puts(p);
allread:
			;
		} else
			fprintf(stderr, "*E* Querying meteo : %s\n", curl_easy_strerror(res));

			/* Cleanup */
		curl_easy_cleanup(curl);
		free(chunk.memory);
	}

	pthread_exit(0);
}

#endif
