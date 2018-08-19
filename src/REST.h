/* REST.h
 * 	Definitions related to REST querying
 *
 * This file is part of Marcel project and is following the same
 * license rules (see LICENSE file)
 *
 * 14/05/2016	- LF - First verison
 */

#ifndef REST_H
#define REST_H
#include "Marcel.h"	/* for struct _REST */

extern void *process_REST(void *);
extern void waitNextQuery(struct _REST *ctx);

#endif
