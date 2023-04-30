/* mod_alert/action
 *
 * Actions related to notifications
 *
 * This file is part of Marcel project and is following the same
 * license rules (see LICENSE file)
 *
 * 23/11//2022 - LF - First version
 */

#include "mod_alert.h"	/* module's own stuffs */

#include <stdlib.h>
#include <curl/curl.h>
#include <assert.h>

void execOSCmd(const char *cmd, const char *id, const char *msg){
	const char *p = cmd;
	size_t nbre=0;	/* # of %t% in the string */

	if(!p)
		return;

	while((p = strstr(p, "%t%"))){
		nbre++;
		p += 3;	/* add '%t%' length */
	}

	char tcmd[ strlen(cmd) + nbre*strlen(id) + 1 ];
	char *d = tcmd;
	p = cmd;

	while(*p){
		if(*p =='%' && !strncmp(p, "%t%",3)){
			for(const char *s = id; *s; s++)
				if(*s =='"')
					*d++ = '\'';
				else
					*d++ = *s;
			p += 3;
		} else
			*d++ = *p++;
	}
	*d = 0;

	FILE *f = popen( tcmd, "w");
	if(!f){
		perror("popen()");
		return;
	}
	fputs(msg, f);
	fclose(f);
}

void execRest(const char *url, const char *title, const char *msg){
	if(!url)
		return;

	CURL *curl;

	if((curl = curl_easy_init())){
			/* Calculate the number of replacement to do */
		size_t nbrem = 0, nbret = 0;
		const char *p = url;
		while((p = strstr(p, "%m%"))){	/* Messages */
			nbrem++;
			p += 3;	/* add '%m%' length */
		}

		p = url;
		while((p = strstr(p, "%t%"))){	/* Title */
			nbret++;
			p += 3;	/* add '%t%' length */
		}

		CURLcode res;
		char *etitle;
		char *emsg = curl_easy_escape(curl,msg,0);

		char aurl[ strlen(url) + 
			nbrem * (emsg ? strlen(emsg) : strlen("(empty)")) +
			nbret * strlen( etitle=curl_easy_escape(curl,title,0) ) + 1
		];

		char *d = aurl;
		p = url;

		while(*p){
			const char *what = NULL;

			if(*p =='%'){	/* Is it a token to replace ? */
				if(!strncmp(p, "%t%",3))
					what = etitle;
				else if(!strncmp(p, "%m%",3))
					what = emsg ? emsg : "(empty)";
			}

			if(what){	/* we got a token */
				for(const char *s = what; *s; s++)
					if(*s =='"')
						*d++ = '\'';
					else
						*d++ = *s;
				p += 3;
			} else	/* not a token ... copying */
				*d++ = *p++;
		}
		*d = 0;

		curl_free(emsg);
		curl_free(etitle);

		FILE *fnull = fopen("/dev/null", "wb");
		assert(fnull);

		curl_easy_setopt(curl, CURLOPT_WRITEDATA, fnull);	/* avoid curl response to be outputted on STDOUT */
		curl_easy_setopt(curl, CURLOPT_URL, aurl);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);

		if((res = curl_easy_perform(curl)) != CURLE_OK)
			publishLog('E', "Sending SMS : %s", curl_easy_strerror(res));

		curl_easy_cleanup(curl);
		fclose(fnull);
	}
}

