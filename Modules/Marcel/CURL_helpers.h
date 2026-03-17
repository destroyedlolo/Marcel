/* CURL_helpers.h
 *
 * Helpers for Curl and JSon processing
 *
 * 16/05/2016 LF : First version
 * 17/03/2026 LF : add JSon support
 */

#ifndef CURL_HLP_H
#define CURL_HLP_H

#include <json-c/json.h>
#include <sys/types.h>

struct MemoryStruct {
	char *memory;
	size_t size;
};

#define EMPTY_MEMCHUNK { NULL, 0 }

extern size_t WriteMemoryCallback(void *, size_t, size_t, void *);
extern void init_Curl(void);

	/* Json's path */
#define OBJPATH(...) (const char*[]){ __VA_ARGS__ }

extern struct json_object *getObj(struct json_object *parent, const char *path[]);
extern const char *getObjString(struct json_object *parent, const char *path[]);
extern int getObjInt(struct json_object *parent, const char *path[]);
extern double getObjNumber(struct json_object *parent, const char *path[]);
extern bool getObjBool(struct json_object *parent, const char *path[]);
#endif
