/* Version.h
 * 	Only define Marcel version
 *
 * This file is part of Marcel project and is following the same
 * license rules (see LICENSE file)
 *
 *	06/06/2016	- LF - switch to v5.0 - Prepare Alarm handling
 *							FFV, Every's sample can be -1. 
 *							In this case, launched only once.
 * 07/06/2016	- LF - externalize version and switch to VV.SSMM schem.
 * 08/06/2016	- LF - v5.01 - 1-wire Alarm handled
 * 10/06/2016 				Handle 1w alarm
 * 24/07/2016	- LF - v5.02 - Handle offset for FFV
 * 13/08/2016	- LF - v5.03 - Add Out file
 * 			-----------
 * 19/08/2016	- LF - v6.00 - handle disable
 * 21/08/2016	- LF - v6.02 - starting this version all section MUST have an uniq ID
 * 					 - v6.03 - OnOff working
 * 22/08/2016	- LF - v6.04 - Add Lua function on OutFile
 * 31/08/2016	- LF - v6.05 - Add Keep
 * 01/09/2016				Add publishLog() function
 * 15/10/2016	- LF - v6.06 - Add MARCEL_SCRIPT_DIR Lua variable
 * 16/10/2016	- LF - 6.06.01 - Intitialise funcid for DPD to avoid a crash
 * 17/11/2016	- LF - v6.07 - Add user function to FFV
 * 21/11/2016	- LF - v6.08 - Add MinVersion directive
 * 22/07/2017	- LF - v6.09 - Add LookForChange section
 * 23/08/2017	- LF - v6.10 - Add CRC, HEC and FEC for the Freebox
 * 28/02/2018	- LF - v6.11 - Replace $alert by $notification
 * 05/03/2018	- LF 		 - Replace SMSUrl by RESTUrl
 * 01/06/2018	- LF - v6.12 - REST user func may be called with NIL
 * 18/08/2018	- LF - v6.13 - Add absolute time to Every
 * 25/11/2018	- LF - v6.14 - Add safe85 to FFV
 * 27/07/2020	- LF - v6.15 - Add Copyright to Lua
 * 			-----------
 * 09/07/2020	- LF - v7.00 - Create "Trace" log level
 * 01/08/2020	- LF - v7.01 - Add Log()
 * 02/08/2020	- LF - v7.02 - Directives can have variables (replaceVar())
 * 27/09/2020	- LF - v7.03 - Add Randomize
 * 09/04/2021	- LF - v7.04 - Add Retained directive
 * 25/06/2021	- LF - v7.05 - Introduce FailFunc
 * 10/07/2021	- LF - v7.06 - Add SHT31 support
 * 16/11/2021	- LF - v7.07 - Publish alerts
 * 			-----------
 * 06/09/2022	- LF - v8.00 - redesign for modularity
 * 							 - Sample can be a float
 * 14/10/2023	- LF - v8.01 - Improve Lua support
 */

#ifndef MARCEL_VERSION_H
#define MARCEL_VERSION_H

#define MARCEL_VERSION "8.0108"	/* Need to stay numerique as exposed to Lua 
							 * VV.SSMM :
							 * 	VV - Version
							 * 	SS - SubVersion
							 */

#define MARCEL_COPYRIGHT "Marcel v"MARCEL_VERSION" (c) L.Faillie 2015-2023"

#ifdef DEBUG
#	define DEFAULT_CONFIGURATION_FILE	"Config"
#else
#	define DEFAULT_CONFIGURATION_FILE	"/usr/local/etc/Marcel"
#endif


#endif
