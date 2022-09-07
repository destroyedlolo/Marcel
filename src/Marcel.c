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

#include <unistd.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include <libgen.h>
#include <dirent.h>

bool verbose = false;
bool configtest = false;

#ifdef DEBUG
bool debug = false;
#endif

	/* buffer to read files */
#define MAXL 2048
static char l[MAXL];

	/* ***
	 * Read configuration directory
	 * ***/

void read_configuration( const char *dir ){
	DIR *dp;
	struct dirent *entry;
	size_t dirl = strlen(dir);

	assert(dirl < MAXL -2);
	strcpy(l, dir);
	char *file = l + dirl++;
	*(file++) = '/';
	*file = 0;

#if DEBUG
	if(debug)
		printf("*d* reading config from : %s\n", l);
#endif

	if(!(dp = opendir(dir))){
		fprintf(stderr,"cannot open directory: %s\n", dir);
		exit( EXIT_FAILURE );
	}

	while((entry = readdir(dp))){
		if(*entry->d_name == '.')	/* Ignore dot file */
			continue;

		size_t filel = strlen(entry->d_name);
		assert(dirl + filel < MAXL -2);
		strcpy(file, entry->d_name);
puts(l);
	}

	closedir(dp);
}


int main(int ac, char **av){
	const char *conf_file = DEFAULT_CONFIGURATION_FILE;
	int c;

	while((c = getopt(ac, av, "hvtdf:")) != EOF) switch(c){
	case 't':
		configtest = true;
	case 'v':
		puts(MARCEL_COPYRIGHT);
		verbose = true;
		break;
	case 'f':
		conf_file = optarg;
		break;
#ifdef DEBUG
	case 'd':
		debug = true;
		break;
#endif
	case 'h':
	default:
		if( c != '?' && c != 'h'){
			fprintf( stderr, "*F* invalid option -- '%c'\n", c);
			c = '?';
		}

		fprintf( c=='?' ? stderr: stdout,
			MARCEL_COPYRIGHT
			"\nA lightweight MQTT publisher\n"
			"\n"
			"Known options are :\n"
			"\t-h : this online help\n"
			"\t-v : enable verbose messages\n"
#ifdef DEBUG
			"\t-d : enable debug messages\n"
#endif
			"\t-f<file> : read <file> for configuration\n"
			"\t\t(default is '%s')\n"
			"\t-t : test configuration file and exit\n",
			conf_file
		);
		exit( c=='?' ? EXIT_FAILURE : EXIT_SUCCESS );
	}

	read_configuration( conf_file );

	exit(EXIT_SUCCESS);
}

