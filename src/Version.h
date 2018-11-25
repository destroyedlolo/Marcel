/* Version.h
 * 	Only define Marcel version
 *
 * This file is part of Marcel project and is following the same
 * license rules (see LICENSE file)
 *
 * 07/06/2016	- LF - externalize version and switch to VV.SSMM schem.
 * 10/06/2016	- LF - v5.01 - Handle 1w alarm
 * 24/07/2016	- LF - v5.02 - Handle offset for FFV
 * 13/08/2016	- LF - v5.03 - Add Out file
 * 19/08/2016	- LF - v6.00 - handle disable
 * 21/08/2016	- LF - v6.02 - starting this version all section MUST have an uniq ID
 * 					 - v6.03 - OnOff working
 * 22/08/2016	- LF - v6.04 - Add Lua function on OutFile
 * 31/08/2016	- LF - v6.05 - Add Keep
 * 15/10/2016	- LF - v6.06 - Add MARCEL_SCRIPT_DIR Lua variable
 * 17/11/2016	- LF - v6.07 - Add user function to FFV
 * 21/11/2016	- LF - v6.08 - Add MinVersion directive
 * 22/07/2017	- LF - v6.09 - Add LookForChange section
 * 23/08/2017	- LF - v6.10 - Add CRC, HEC and FEC for the Freebox
 * 28/02/2018	- LF - v6.11 - Replace $alert by $notification
 * 05/03/2018	- LF 		 - Replace SMSUrl by RESTUrl
 * 01/06/2018	- LF - v6.12 - REST user func may be called with NIL
 * 18/08/2018	- LF - v6.13 - Add absolute time to Every
 * 25/11/2018	- LF - Add safe85 to FFV
 */

#ifndef MARCEL_VERSION_H
#define MARCEL_VERSION_H


#define VERSION "6.1400"	/* Need to stay numerique as exposed to Lua 
							 * VV.SSMM :
							 * 	VV - Version
							 * 	SS - SubVersion
							 */
#endif
