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

#include "mod_core.h"

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdarg.h>
#include <errno.h>
#include <libgen.h>
#include <dirent.h>
#include <sys/stat.h>

struct Config cfg;
bool configtest = false;

struct _VarSubstitution vslookup[] = {
	{ "%ClientID%", NULL },
	{ "%Hostname%", NULL },
	{ NULL }
};

	/* ***
	 * Helpers
	 * ***/
/**
 * @brief compares a string against a keyword
 *
 * @param s string to compare
 * @param kw keyword
 * @return remaining string if the keyword matches or NULL if not
 */
const char *striKWcmp( const char *s, const char *kw ){
	size_t klen = strlen(kw);
	if( strncasecmp(s,kw,klen) )
		return NULL;
	else
		return s+klen;
}

/**
 * @brief removes a potential LF at the end of the string
 */
char *removeLF(char *s){
	size_t l=strlen(s);
	if(l && s[--l] == '\n')
		s[l] = 0;
	return s;
}

/**
 * @brief returns the checksum of a string
 */
int chksum(const char *s){
	int h = 0;
	while(*s)
		h += *s++;
	return h;
}

	/* ***
	 * Logging 
	 * ***/

void publishLog( char l, const char *msg, ...){
	va_list args;
	va_start(args, msg);

	if(cfg.verbose || l=='E' || l=='F'){
		char t[ strlen(msg) + 7 ];
		sprintf(t, "*%c* %s\n", l, msg);
		vfprintf((l=='E' || l=='F')? stderr : stdout, t, args);
	}

 #if 0
	if(cfg.client){
		char *sub;
		switch(l){
		case 'F':
			sub = "/Log/Fatal";
			break;
		case 'E':
			sub = "/Log/Error";
			break;
		case 'W':
			sub = "/Log/Warning";
			break;
		case 'I':
			sub = "/Log/Information";
			break;
		case 'C':
			sub = "/Log/Corrected";
			break;
		default :	/* Trace */
			sub = "/Log";
		}

		char tmsg[1024];	/* No simple way here to know the message size */
		char ttopic[ strlen(cfg.ClientID) + strlen(sub) + 1 ];
		sprintf(ttopic, "%s%s", cfg.ClientID, sub);
		vsnprintf(tmsg, sizeof(tmsg), msg, args);

		mqttpublish( cfg.client, ttopic, strlen(tmsg), tmsg, 0);
	}
	va_end(args);
#endif
}


	/* ***
	 * Variable substitution
	 * ***/

static void init_VarSubstitution( struct _VarSubstitution *tbl ){
	while(tbl->var){
		tbl->lvar = strlen(tbl->var);
		tbl->h = chksum(tbl->var);
		tbl++;
	}
}

bool setSubstitutionVar(struct _VarSubstitution *vars, const char *name, const char *val, bool freeval){
	int h = chksum(name);

	for(struct _VarSubstitution *tbl = vars; tbl->var; tbl++){
		if( h == tbl->h && !strcmp(name, tbl->var) ){
			if(freeval && tbl->val)
				free((void *)tbl->val);
			tbl->val = val;
			tbl->lval = strlen(val);
			return true;
		}
	}

#ifdef DEBUG
	if(cfg.debug)
		publishLog('E', "Variable '%s' not found", name);
#endif

	return false;
}

const char *getSubstitutionVar( struct _VarSubstitution *lookup, const char *name ){
	int  h = chksum(name);

	for(struct _VarSubstitution *tbl = lookup; tbl->var; tbl++){
		if( h == tbl->h && !strcmp(name, tbl->var) )
			return tbl->val;
	}

	return NULL;
}



/**
 * @brief Replace variables found in the string by their value
 *
 * @param arg original string
 * @param lookup variables lookup tables (name, value)
 * @return malloced resulting string
 */
static char *replaceVar( const char *arg, struct _VarSubstitution *lookup ){
	size_t idx, idxd, 				/* source and destination indexes */
		sz, max=strlen(arg);		/* size of allocated area and max index */

	char *s = malloc( sz=max+1 );	/* resulting string */
	assert(s);

	for(idx = idxd = 0; idx<max; idx++){
		if(arg[idx] == '%'){
			bool found=false;
			struct _VarSubstitution *t;

			for(t=lookup; t->var; t++){
				if(!strncmp(arg+idx, t->var, t->lvar)){
					sz += t->lval - t->lvar;
					assert((s = realloc(s, sz)));
					strcpy(s+idxd, t->val);		/* Insert the value */
					idxd += t->lval;			/* Skip variable's content */
					idx += t->lvar-1;			/* Skip variable's name */

					found=true;
					break;
				}
			}

			if(!found)
				s[idxd++] = arg[idx];
		} else
			s[idxd++] = arg[idx];
	}

	s[idxd] = 0;
	return s;
}

	/* ***
	 * Read configuration directory
	 * ***/

static void process_conffile(const char *fch){
	FILE *f;
	char l[MAXLINE];

		if(cfg.verbose)
		printf("\n*C* Reading configuration file : '%s'\n--------------------------------\n", fch);

	if(!(f=fopen(fch, "r"))){
		publishLog('F', "%s : %s", fch, strerror( errno ));
		exit(EXIT_FAILURE);
	}

	while(fgets(l, MAXLINE, f)){
		if(*l == '#' || *l == '\n')
			continue;

		char *line = replaceVar(removeLF(l), vslookup);

			/* Ask each module if it knows this configuration */
		bool accepted = false;
		for(unsigned int i=0; i<numbe_of_loaded_modules; i++){
			if(modules[i]->readconf(line)){
				accepted = true;
				break;
			}
		}

		if(!accepted){
			publishLog('F', "'%s' is not reconized by any loaded module", line);
			exit( EXIT_FAILURE );
		}

		free(line);
	}

	fclose(f);
}

static int acceptfile(const struct dirent *entry){
	return(*entry->d_name != '.');	/* ignore dot files */
}

static void read_configuration( const char *dir ){
	/* to avoid temporary storage for each file to read,
	 * we chdir() to the configuration directory.
	 * Consequently, files are accessible from the current
	 * directory.
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
		printf("*d* reading config from : %s\n", dir);
	}
#endif

	if(chdir(dir)){	/* go to configuration directory */
		perror(dir);
		exit( EXIT_FAILURE );
	}

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

	if(chdir(cwd)){
		perror(cwd);
		exit( EXIT_FAILURE );
	}
	free(cwd);
}


int main(int ac, char **av){
	const char *conf_file = DEFAULT_CONFIGURATION_FILE;
	int c;
	cfg.debug = false;

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

	init_VarSubstitution( vslookup );
	init_module_core();
	read_configuration( conf_file );

	exit(EXIT_SUCCESS);
}

