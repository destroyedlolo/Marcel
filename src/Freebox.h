/* Freebox.h
 * 	Definitions related to Freebox figures handling
 *
 * This file is part of Marcel project and is following the same
 * license rules (see LICENSE file)
 *
 * 08/07/2015	- LF - start v2.0 - make source modular
 */

#ifndef FREEBOX_H
#define FREEBOX_H

#ifdef FREEBOX

#include "Marcel.h"

#define FBX_HOST	"mafreebox.freebox.fr"
#define FBX_URI "/pub/fbx_info.txt"
#define FBX_PORT	80

#define FBX_REQ "GET "FBX_URI" HTTP/1.0\n\n"

extern void *process_Freebox(void *);

#endif
#endif
