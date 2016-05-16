/* CURL_helpers.h
 *
 * Helpers for Curl processing
 *
 * 16/05/2016 LF : First version
 */

#ifndef CURL_HLP_H
#define CURL_HLP_H

#include <sys/types.h>

struct MemoryStruct {
	char *memory;
	size_t size;
};

extern size_t WriteMemoryCallback(void *, size_t, size_t, void *);

#endif
