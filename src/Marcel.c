/*
 * Marcel
 *	A daemon to publish smart home data to MQTT broker and rise alert
 *	if needed.
 *
 * Additional options :
 *	-DFREEBOX : enable Freebox (v4 / v5) statistics
 *	-DUPS : enable UPS statistics (NUT needed)
 *	-DLUA : enable Lua user functions
 *	-DMETEO : enable meteo forcast publishing
 *	-DINOTIFY : add inotify support (needed by *LookForChanges)
 *
 *	Copyright 2015-2018 Laurent Faillie
 *
 *		Marcel is covered by
 *		Creative Commons Attribution-NonCommercial 3.0 License
 *      (http://creativecommons.org/licenses/by-nc/3.0/) 
 *      Consequently, you're free to use if for personal or non-profit usage,
 *      professional or commercial usage REQUIRES a commercial licence.
 *  
 *      Marcel is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 *	18/05/2015	- LF start of development (inspired from TeleInfod)
 *	20/05/2015	- LF - v1.0 - "file float value" working
 *	25/05/2015	- LF - v1.1 - Adding "Freebox"
 *	28/05/2015	- LF - v1.2 - Adding UPS
 *				-------
 *	08/07/2015	- LF - start v2.0 - make source modular
 *	21/07/2015	- LF - v2.1 - secure non-NULL MQTT payload
 *	26/07/2015	- LF - v2.2 - Add ConnectionLostIsFatal
 *	27/07/2015	- LF - v2.3 - Add ClientID to avoid connection loss during my tests
 *				-------
 *	07/07/2015	- LF - switch v3.0 - Add Lua user function in DPD
 *	09/08/2015	- LF - 3.1 - Add mutex to avoid parallel subscription which seems
 *					trashing broker connection
 *	09/08/2015	- LF - 3.2 - all subscriptions are done in the main thread as it seems 
 *					paho is not thread safe.
 *	07/09/2015	- LF - 3.3 - Adding Every tasks.
 *				-------
 *	06/10/2015	- LF - switch to v4.0 - curl can be used in several "section"
 *	29/10/2015	- LF - 4.1 - Add meteo forcast
 *	29/11/2015	- LF - 4.2 - Correct meteo forcast icon
 *	31/01/2016	- LF - 4.3 - Add AlertCommand
 *	04/02/2016	- LF - 4.4 - Alert can send only a mail
 *	24/02/2016	- LF - 4.5 - Add Notifications
 *	20/03/2016	- LF - 4.6 - Add named notifications
 *							- Can work without sections (Marcel acts as alerting relay)
 *	29/04/2016	- LF - 4.7 - Add RFXtrx support
 *	01/05/2016	- LF - 		DPD* replaced by Sub*
 *	14/05/2016	- LF - 4.10 - Add REST section
 *				-------
 *	06/09/2022	- LF - switch to v8
 */
#include "Marcel.h"
#include "Version.h"

int main(int ac, char **av){
}

