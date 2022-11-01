/*
 * Marcel
 *	A daemon to publish smart home data to MQTT broker and rise alert
 *	if needed.
 *
 *	Copyright 2015-2022 Laurent Faillie
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

#include "Module.h"
#include "Section.h"
#include "mod_core.h"

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <libgen.h>
#include <dirent.h>
#include <sys/stat.h>
#include <signal.h>
#include <ctype.h>

struct Config cfg;
bool configtest = false;


	/* ***
	 * Read configuration directory
	 * ***/

static void process_conffile(const char *fch){
	FILE *f;
	char l[MAXLINE];
	struct Section *sec = NULL;	/* Section's definition can't be spread among files */

	if(cfg.verbose)
		publishLog('C', "Reading configuration file : '%s'", fch);

	if(!(f=fopen(fch, "r"))){
		publishLog('F', "%s : %s", fch, strerror( errno ));
		exit(EXIT_FAILURE);
	}

	while(fgets(l, MAXLINE, f)){
		char *line = l;

		while(isspace(*line))
			line++;

		if(*line == '#' || *line == '\n' || !*line)
			continue;

		line = replaceVar(removeLF(line), vslookup);

			/* Ask each module if it knows this configuration */
		enum RC_readconf rc = REJECTED;
		for(unsigned int i=0; i<number_of_loaded_modules; i++){
			if(!modules[i]->readconf)
				continue;

			rc = modules[i]->readconf(i, line, &sec);
			if(rc == ACCEPTED || rc == SKIP_FILE)
				break;
		}

		if(rc == REJECTED){
			publishLog('F', "'%s' is not recognized by any loaded module or outside section", line);
			exit( EXIT_FAILURE );
		}

		free(line);

		if(rc == SKIP_FILE)	/* remaining of the file is ignored */
			break;
	}

	fclose(f);
}

static int acceptfile(const struct dirent *entry){
#ifdef DT_REG
	if(entry->d_type != DT_REG){
#	ifdef DEBUG
		if(cfg.debug && *entry->d_name != '.')
			publishLog('I', "'%s' is not a regular and so, is ignored", entry->d_name);
#	endif
		return false;
	}
#else
#	warning("scandir() doesn't identify file type, directory are not ignored")
#endif
	return(*entry->d_name != '.');	/* ignore dot files */
}

static void read_configuration( const char *dir ){
	int n;	/* read and sort files */
	struct dirent **namelist;

	if((n = scandir(".", &namelist, acceptfile, alphasort)) < 0){
		perror(dir);
		exit( EXIT_FAILURE );
	}

	for(int i=0; i<n; i++)
		process_conffile(namelist[i]->d_name);

		/* Cleanup */
	while(n--)
		free(namelist[n]);

	free(namelist);

}

	/*
	 * Broker related functions
	 */

	/* @brief receive a message
	 *
	 * Notez-bien : only straight strings are supported for topic and payload
	 */
static int msgarrived(void *actx, char *topic, int tlen, MQTTClient_message *msg){
		/* format payload */
	char payload[msg->payloadlen + 1];
	memcpy(payload, msg->payload, msg->payloadlen);
	payload[msg->payloadlen] = 0;

#ifdef DEBUG
	if(cfg.debug)
		publishLog('T', "message arrival (topic : '%s', msg : '%s')", topic, payload);
#endif

		/* looks for a sections that is accepting MQTT's messages */
	for(struct Section *s = sections; s; s = s->next){
		if(s->processMsg){
			if(s->processMsg(s, topic, payload))	/* true if message processed */
				break;
		} else if(cfg.sublast)
			break;
	}
	
		/* Clean messages */
	MQTTClient_freeMessage(&msg);
	MQTTClient_free(topic);
	return 1;
}

static void connlost(void *ctx, char *cause){
	publishLog('E', "Broker connection lost due to %s", cause);
	exit(EXIT_FAILURE);
}

static void brkcleaning(void){	/* Clean broker stuffs */
	MQTTClient_disconnect(cfg.client, 10000);	/* 10s for the grace period */
	MQTTClient_destroy(&cfg.client);
}

static void handleInt(int na){
	exit(EXIT_SUCCESS);
}

int main(int ac, char **av){
	const char *conf_file = DEFAULT_CONFIGURATION_FILE;
	int c;

		/* Default values */
	cfg.debug = false;
	cfg.verbose = false;
	cfg.sublast = false;

	while((c = getopt(ac, av, "hvtdf:")) != EOF) switch(c){
#ifdef DEBUG
	case 'd':
		cfg.debug = true;
		puts(MARCEL_COPYRIGHT);
		cfg.verbose = true;
		break;
#endif
	case 't':
		configtest = true;
	case 'v':
		puts(MARCEL_COPYRIGHT);
		cfg.verbose = true;
		break;
	case 'f':
		conf_file = optarg;
		break;
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

	srand(time(NULL));	/* Initialize random generator */
	init_VarSubstitution( vslookup );
	init_module_core();

	/* ***
	 * Read the configuration
	 * ***
	 * to avoid temporary storage for each file to read,
	 * we chdir() to the configuration directory.
	 * Consequently, files are accessible from the current
	 * directory.
	 *
	 * It's done here to allow postconfInit() to run from
	 * the configuration directory (so Lua's scripts can be load
	 * relatively to this directory)
	 */

		/* keep the cwd */
	char *cwd = realpath(".", NULL);
	if(!cwd){
		perror("current directory");
		exit( EXIT_FAILURE );
	}

#if DEBUG
	if(cfg.debug){
		printf("*d* current directory : %s\n", cwd);
		printf("*d* reading config from : %s\n", conf_file);
	}
#endif

	if(chdir(conf_file)){	/* go to configuration directory */
		perror(conf_file);
		exit( EXIT_FAILURE );
	}


	read_configuration( conf_file );

	if(configtest){
		publishLog('W', "Testing only the configuration ... leaving.");

		if(chdir(cwd)){
			perror(cwd);
			exit( EXIT_FAILURE );
		}
		free(cwd);

		exit(EXIT_SUCCESS);
	}

	puts("");


		/* Init MQTT stuffs */
	MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
	conn_opts.reliable = 0;
	MQTTClient_create( &cfg.client, cfg.Broker, cfg.ClientID, MQTTCLIENT_PERSISTENCE_NONE, NULL);
	MQTTClient_setCallbacks( cfg.client, NULL, connlost, msgarrived, NULL);

	int err;
	switch( err=MQTTClient_connect( cfg.client, &conn_opts) ){
	case MQTTCLIENT_SUCCESS : 
		break;
	case 1 : publishLog('F', "[%s] Unable to connect : Unacceptable protocol version", cfg.Broker);
		if(chdir(cwd)){
			perror(cwd);
			exit( EXIT_FAILURE );
		}
		free(cwd);
		exit(EXIT_FAILURE);
	case 2 : publishLog('F', "[%s] Unable to connect : Identifier rejected", cfg.Broker);
		if(chdir(cwd)){
			perror(cwd);
			exit( EXIT_FAILURE );
		}
		free(cwd);
		exit(EXIT_FAILURE);
	case 3 : publishLog('F', "[%s] Unable to connect : Server unavailable", cfg.Broker);
		if(chdir(cwd)){
			perror(cwd);
			exit( EXIT_FAILURE );
		}
		free(cwd);
		exit(EXIT_FAILURE);
	case 4 : publishLog('F', "[%s] Unable to connect : Bad user name or password", cfg.Broker);
		if(chdir(cwd)){
			perror(cwd);
			exit( EXIT_FAILURE );
		}
		free(cwd);
		exit(EXIT_FAILURE);
	case 5 : publishLog('F', "[%s] Unable to connect : Not authorized", cfg.Broker);
		if(chdir(cwd)){
			perror(cwd);
			exit( EXIT_FAILURE );
		}
		free(cwd);
		exit(EXIT_FAILURE);
	default :
		if(chdir(cwd)){
			perror(cwd);
			exit( EXIT_FAILURE );
		}
		free(cwd);
		publishLog('F', "[%s] Unable to connect (%d)", cfg.Broker, err);
		exit(EXIT_FAILURE);
	}
	atexit(brkcleaning);

	publishLog('I', MARCEL_COPYRIGHT);

		/* Post conf initialisation to be done after configuration reading */
	if(cfg.verbose)
		publishLog('C', "Initialising modules");

	for(unsigned int i=0; i<number_of_loaded_modules; i++){
		if(modules[i]->postconfInit)
			modules[i]->postconfInit(i);
	}

	for(struct Section *s = sections; s; s = s->next){
		if(s->postconfInit)
			s->postconfInit(s);
	}

	if(chdir(cwd)){
		perror(cwd);
		exit( EXIT_FAILURE );
	}
	free(cwd);

		/* Display / publish copyright */
	publishLog('W', "%s v%s starting ...", basename(av[0]), MARCEL_VERSION);

	pthread_attr_t thread_attr;
	assert(!pthread_attr_init (&thread_attr));
	assert(!pthread_attr_setdetachstate (&thread_attr, PTHREAD_CREATE_DETACHED));

	for(struct Section *s = sections; s; s = s->next){
		uint8_t mid = s->id & 0xff;
		uint8_t sid = (s->id >> 8) & 0xff;

		if(mid >= number_of_loaded_modules){
			publishLog('F',"Internal error for [%s] : module ID larger than # loaded modules", s->uid);
			exit( EXIT_FAILURE );
		}

		if(modules[mid]->getSlaveFunction){
			ThreadedFunctionPtr slave = modules[mid]->getSlaveFunction(sid);

			if(slave){
				if(pthread_create( &(s->thread), &thread_attr, slave, s) < 0){
					publishLog('F', "[%s] Can't create a processing thread", s->uid);
					exit(EXIT_FAILURE);
				}
			}
#ifdef DEBUG
			else if(cfg.debug)
				publishLog('d', "No threaded slave for [%s]", s->uid);
#endif
		}
	}

	signal(SIGINT, handleInt);
	pause();

	exit(EXIT_SUCCESS);
}

