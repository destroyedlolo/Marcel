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
#ifdef MCHECK
#include <malloc.h>
#endif

static bool internal_callAPI(struct Section *s, struct section_TaHoma *gw, const char *api, const char *post, struct MemoryStruct *buff){
	assert(!buff->memory);	/* Otherwise, it's meaning it hasn't been initialized */

#ifdef MCHECK
	/* Check if there is any memory leak */
	struct mallinfo2 beg, end;
	beg = mallinfo2();
#endif

	CURL *curl = curl_easy_init();
	struct curl_slist *headers = NULL;

	if(!curl){
		publishLog('E', "[%s] curl_easy_init() failed", s->uid);
		SectionError(s, true);
		return false;
	}

	char host_header[6 + strlen(gw->hostname) + 5 + 2]; /* Host: host:port + null */
	sprintf(host_header, "Host: %s:%u", gw->hostname, gw->port);
	headers = curl_slist_append(headers, host_header);

	headers = curl_slist_append(headers, "Content-Type: application/json");
	headers = curl_slist_append(headers, "accept: application/json");

	char auth_header[22 + strlen(gw->token) + 1];
	strcpy(auth_header, "Authorization: Bearer ");
	strcat(auth_header, gw->token);
	headers = curl_slist_append(headers, auth_header);

	int res = curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
	if(res != CURLE_OK){
		publishLog('E', "[%s] curl_easy_setopt() : %s", s->uid, curl_easy_strerror(res));
		SectionError(s, true);
		curl_easy_cleanup(curl);
		curl_slist_free_all(headers);
		return false;
	}

	curl_easy_setopt(curl, CURLOPT_URL, api);
	if(cfg.debug){
		publishLog('d', "[%s] Calling \"%s\"", s->uid, api);
		curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
	}

	curl_easy_setopt(curl, CURLOPT_USERAGENT, "Marcel/" MARCEL_VERSION);

	if(gw->unsafe){
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
		publishLog('E', "[%s] curl_easy_perform() : %s", s->uid, curl_easy_strerror(res));
	else {
		long http_code = 0;
		curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);

		if(http_code != 200){
			publishLog('E', "[%s] HTTP return code : %ld", s->uid, http_code);
			res += 1;	/* Just not to keep CURLE_OK */
		}
	}

		/* Cleanup */
	curl_easy_cleanup(curl);
	curl_slist_free_all(headers);

#ifdef MCHECK
	/* Check if there is any memory leak */
	end = mallinfo2();

	if(end.uordblks != beg.uordblks)
		publishLog('W', "[%s] (callAPI) %zu octets lost", s->uid, end.uordblks - beg.uordblks);
	else
		publishLog('W', "[%s] (callAPI) No octets lost", s->uid);
#endif

	return(res == CURLE_OK);
}

/* Query the TaHoma from a device
 * -> s : Corresponding device
 * -> api : API to query (if NULL, use s->target_url)
 * -> post : payload to provide (if NULL, use GET method)
 * -> buff : buffer to feed
 */
bool callAPIDev(struct section_Device *s, const char *api, const char *post, struct MemoryStruct *buff){
	if(!api)
		return internal_callAPI(&s->section, s->gateway, s->target_url, post, buff);

	char url[s->gateway->url_len + strlen(api) + 1];
	sprintf(url, "%s%s", s->gateway->baseurl, api);

	return internal_callAPI(&s->section, s->gateway, s->url, post, buff);
}

bool callAPIGW(struct section_TaHoma *gw, const char *api, const char *post, struct MemoryStruct *buff){
	if(!api)
		return internal_callAPI(&gw->section, gw, gw->baseurl, post, buff);

	char url[gw->url_len + strlen(api) + 1];
	sprintf(url, "%s%s", gw->baseurl, api);

	return internal_callAPI(&gw->section, gw, url, post, buff);
}
