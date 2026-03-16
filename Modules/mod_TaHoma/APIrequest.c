/* Process API calls
 *
 * This file is part of Marcel project and is following the same
 * license rules (see LICENSE file)
 *
 * 18/03/2026 - LF - Creation
 */

#include "mod_TaHoma.h"
#include "../Marcel/Version.h"

#include <curl/curl.h>
#include <assert.h>
#include <stdlib.h>

/* Query the TaHoma
 * -> gateway : the TaHoma to query
 * -> api : API to query
 * -> post : payload to provide (if NULL, use GET method)
 * -> buff : buffer to feed
 */
bool callAPI(struct section_Device *s, const char *post, struct MemoryStruct *buff){
	assert(!buff->memory);	/* Otherwise, it's meaning it hasn't been initialized */

	CURL *curl = curl_easy_init();
	if(!curl){
		publishLog('E', "[%s] curl_easy_init() failed", s->section.uid);
		SectionError((struct Section *)s, true);
		return false;
	}

	struct curl_slist *headers = NULL;

	char host_header[6 + strlen(s->gateway->hostname) + 5 + 2]; /* Host: host:port + null */
	sprintf(host_header, "Host: %s:%u", s->gateway->hostname, s->gateway->port);
	headers = curl_slist_append(headers, host_header);

	headers = curl_slist_append(headers, "Content-Type: application/json");
	headers = curl_slist_append(headers, "accept: application/json");

	char auth_header[22 + strlen(s->gateway->token) + 1];
	strcpy(auth_header, "Authorization: Bearer ");
	strcat(auth_header, s->gateway->token);
	headers = curl_slist_append(headers, auth_header);

	int res = curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
	if(res != CURLE_OK){
		publishLog('E', "[%s] curl_easy_setopt() : %s", s->section.uid, curl_easy_strerror(res));
		SectionError((struct Section *)s, true);
		curl_slist_free_all(headers);
		return false;
	}

	curl_easy_setopt(curl, CURLOPT_URL, s->target_url);
	if(cfg.debug){
		publishLog('d', "[%s] Calling \"%s\"", s->section.uid, s->target_url);
		curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
	}

	curl_easy_setopt(curl, CURLOPT_USERAGENT, "Marcel/" MARCEL_VERSION);

	if(s->gateway->unsafe){
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);	/* Don't verify SSL */
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
	}

	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)buff);
	curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);

	if(post){
		curl_easy_setopt(curl, CURLOPT_POST, 1L);
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post);
		curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE_LARGE, (curl_off_t)-1);
	} else {
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, NULL);
		curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, 0L);
		curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
	}

	if((res = curl_easy_perform(curl)) != CURLE_OK)
		publishLog('E', "[%s] curl_easy_perform() : %s", s->section.uid, curl_easy_strerror(res));
	else {
		long http_code = 0;
		curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);

		if(http_code != 200){
			publishLog('E', "[%s] HTTP return code : %ld", s->section.uid, http_code);
			res += 1;	/* Just not to keep CURLE_OK */
		}
	}

		/* Cleanup */
	curl_easy_cleanup(curl);
	curl_slist_free_all(headers);

	return(res == CURLE_OK);
}
