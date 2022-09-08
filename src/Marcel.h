/* Marcel.h
 * 	Shared definition
 *
 * This file is part of Marcel project and is following the same
 * license rules (see LICENSE file)
 *
 * 08/07/2015	- LF - start v2.0 - make source modular
 * 20/03/2016	- LF - add named notification handling
 * 21/04/2016	- LF - add errortopic for DPD
 * 29/04/2016	- LF - add support for RFXtrx
 * 01/05/2016	- LF - Rename all DPD* as Sub*
 * 14/05/2016	- LF - add REST section
 * 07/06/2016	- LF - externalize version
 * 21/08/2016	- LF - starting v6.02, each section MUST have an ID
 * 31/08/2016	- LF - Add "keep" option"
 * 01/09/2016	- LF - Add publishLog() function
 * 18/08/2018	- LF - Add absolute time to Every
 * 25/11/2018	- LF - Add safe85 to FFV
 * 09/04/2021	- LF - Add Retained
 * 	---
 * 06/09/2022	- LF - switch to v8
 */

#ifndef MARCEL_H
#define MARCEL_H

#include <stdbool.h>

#define MAXLINE 1024

extern bool verbose;
#ifdef DEBUG
extern bool debug;
#endif

extern void publishLog( char l, const char *msg, ...);

#endif

