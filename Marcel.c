/*
 * Marcel
 *	A daemon to publish some smart home data to MQTT broker
 *
 *	Compilation
gcc -std=c99 -lpthread -lpaho-mqtt3c -Wall Marcel.c -o Marcel
 *
 * Additional options :
 *	-DFREEBOX : enable Freebox statistics
 *
 *	Copyright 2015 Laurent Faillie
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
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <errno.h>
#include <libgen.h>
#include <assert.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#ifdef FREEBOX
#	include <sys/types.h>
#	include	<sys/socket.h>
#	include	<netinet/in.h>
#	include	<netdb.h>
#endif

	/* PAHO library needed */ 
#include <MQTTClient.h>

#define VERSION "1.0"
#define DEFAULT_CONFIGURATION_FILE "/usr/local/etc/Marcel.conf"
#define MAXLINE 1024	/* Maximum length of a line to be read */
#define BRK_KEEPALIVE 60	/* Keep alive signal to the broker */

int debug = 0;

	/*
	 * Helpers
	 */
char *removeLF(char *s){
	size_t l=strlen(s);
	if(l && s[--l] == '\n')
		s[l] = 0;
	return s;
}

char *striKWcmp( char *s, const char *kw ){
/* compare string s against kw
 * Return :
 * 	- remaining string if the keyword matches
 * 	- NULL if the keyword is not found
 */
	size_t klen = strlen(kw);
	if( strncasecmp(s,kw,klen) )
		return NULL;
	else
		return s+klen;
}

char *mystrdup(const char *as){
	/* as strdup() is missing within C99, grrr ! */
	char *s;
	assert(as);
	assert(s = malloc(strlen(as)+1));
	strcpy(s, as);
	return s;
}
#define strdup(s) mystrdup(s)

char *extr_arg(char *s, int l){ 
/* Extract an argument from TéléInfo trame 
 *	s : argument string just after the token
 *	l : length of the argument
 */
	s++;	/* Skip the leading space */
	s[l]=0;
	return s;
}

	/*
	 * Configuration
	 */
enum _tp_msec {
	MSEC_INVALID =0,	/* Ignored */
	MSEC_FFV,			/* File String Value */
	MSEC_FREEBOX		/* FreeBox */
};

union CSection {
	struct {	/* Fields common to all sections */
		union CSection *next;
		enum _tp_msec section_type;
		int sample;
		pthread_t thread;
		const char *topic;
	} common;
	struct _FFV {
		union CSection *next;
		enum _tp_msec section_type;
		int sample;
		pthread_t thread;
		const char *topic;
		const char *file;
	} FFV;
	struct _FreeBox {
		union CSection *next;
		enum _tp_msec section_type;
		int sample;
		pthread_t thread;
		const char *topic;
	} FreeBox;
};

struct Config {
	union CSection *sections;
	const char *Broker;
	MQTTClient client;
} cfg;

void read_configuration( const char *fch){
	FILE *f;
	char l[MAXLINE];
	char *arg;
	union CSection *last_section=NULL;

	cfg.sections = NULL;
	cfg.Broker = "tcp://localhost:1883";
	cfg.client = NULL;

	if(debug)
		printf("Reading configuration file '%s'\n", fch);

	if(!(f=fopen(fch, "r"))){
		perror(fch);
		exit(EXIT_FAILURE);
	}

	while(fgets(l, MAXLINE, f)){
		if(*l == '#' || *l == '\n')
			continue;

		if((arg = striKWcmp(l,"Broker="))){
			assert( cfg.Broker = strdup( removeLF(arg) ) );
			if(debug)
				printf("Broker : '%s'\n", cfg.Broker);
		} else if((arg = striKWcmp(l,"*FFV="))){
			union CSection *n = malloc( sizeof(struct _FFV) );
			assert(n);
			memset(n, 0, sizeof(struct _FFV));

			n->common.section_type = MSEC_FFV;
			if(last_section)
				last_section->common.next = n;
			else	/* First section */
				cfg.sections = n;
			last_section = n;
			if(debug)
				printf("Entering section '%s'\n", removeLF(arg));
		} else if((arg = striKWcmp(l,"*Freebox"))){
			union CSection *n = malloc( sizeof(struct _FreeBox) );
			assert(n);
			memset(n, 0, sizeof(struct _FreeBox));

			n->common.section_type = MSEC_FREEBOX;
			if(last_section)
				last_section->common.next = n;
			else	/* First section */
				cfg.sections = n;
			last_section = n;
			if(debug)
				puts("Entering section 'Freebox");
		} else if((arg = striKWcmp(l,"File="))){
			if(!last_section || last_section->common.section_type != MSEC_FFV){
				fputs("*F* Configuration issue : File directive outside a FFV section\n", stderr);
					exit(EXIT_FAILURE);
			}
			assert( last_section->FFV.file = strdup( removeLF(arg) ));
			if(debug)
				printf("\tFile : '%s'\n", last_section->FFV.file);
		} else if((arg = striKWcmp(l,"Sample="))){
			if(!last_section){
				fputs("*F* Configuration issue : Sample directive outside a section\n", stderr);
				exit(EXIT_FAILURE);
			}
			last_section->common.sample = atoi( arg );
			if(debug)
				printf("\tDelay between samples : %ds\n", last_section->common.sample);
		} else if((arg = striKWcmp(l,"Topic="))){
			if(!last_section){
				fputs("*F* Configuration issue : Topic directive outside a section\n", stderr);
				exit(EXIT_FAILURE);
			}
			assert( last_section->common.topic = strdup( removeLF(arg) ));
			if(debug)
				printf("\tTopic : '%s'\n", last_section->common.topic);
		}
	}

	fclose(f);
}

	/*
	 * Broker related functions
	 */
int msgarrived(void *ctx, char *topic, int tlen, MQTTClient_message *msg){
	if(debug)
		printf("*I* Unexpected message arrival (topic : '%s')\n", topic);

	MQTTClient_freeMessage(&msg);
	MQTTClient_free(topic);
	return 1;
}

void connlost(void *ctx, char *cause){
	printf("*W* Broker connection lost due to %s\n", cause);
}

int papub( const char *topic, int length, void *payload, int retained ){ /* Custom wrapper to publish */
	MQTTClient_message pubmsg = MQTTClient_message_initializer;
	pubmsg.retained = retained;
	pubmsg.payloadlen = length;
	pubmsg.payload = payload;

	return MQTTClient_publishMessage( cfg.client, topic, &pubmsg, NULL);
}

void brkcleaning(void){	/* Clean broker stuffs */
	MQTTClient_disconnect(cfg.client, 10000);	/* 10s for the grace period */
	MQTTClient_destroy(&cfg.client);
}

	/*
	 * Processing
	 */
void *process_FFV(void *actx){
	struct _FFV *ctx = actx;	/* Only to avoid zillions of cast */
	FILE *f;
	char l[MAXLINE];

		/* Sanity checks */
	if(!ctx->topic){
		fputs("*E* configuration error : no topic specified, ignoring this section\n", stderr);
		pthread_exit(0);
	}
	if(!ctx->file){
		fprintf(stderr, "*E* configuration error : no file specified for '%s', ignoring this section\n", ctx->topic);
		pthread_exit(0);
	}

	if(debug)
		printf("Launching a processing flow for FFV '%s'\n", ctx->topic);

	for(;;){	/* Infinite loop to process messages */
		ctx = actx;	/* Back to the 1st one */
		for(;;){
			if(!(f = fopen( ctx->file, "r" ))){
				if(debug)
					perror( ctx->file );
				if(strlen(ctx->topic) + 7 < MAXLINE){  /* "/Alarm" +1 */
					int msg;
					char *emsg;
					strcpy(l, ctx->topic);
					strcat(l, "/Alarm");
					msg = strlen(l) + 2;

					if(strlen(ctx->file) + strlen(emsg = strerror(errno)) + 4 < MAXLINE - msg){
						strcpy(l + msg, ctx->file);
						strcat(l + msg, " : ");
						strcat(l + msg, emsg);

						papub(l, strlen(l + msg), l + msg, 0);
					} else
						papub(l, strlen(l + msg), emsg, 0);
				}
			} else {
				float val;
				fscanf(f, "%f", &val);	/* Only to normalize the response */
				sprintf(l,"%.1f", val);

				papub( ctx->topic, strlen(l), l, 0 );
				if(debug)
					printf("FFV : %s -> %f\n", ctx->topic, val);
				fclose(f);
			}

			if(!(ctx = (struct _FFV *)ctx->next))	/* It was the last entry */
				break;
			if(ctx->section_type != MSEC_FFV || ctx->sample)	/* Not the same kind or new thread requested */
				break;
		}

		sleep( ((struct _FFV *)actx)->sample );
	}

	pthread_exit(0);
}

#ifdef FREEBOX
#define FBX_HOST	"mafreebox.freebox.fr"
#define FBX_URI "/pub/fbx_info.txt"
#define FBX_PORT	80
void *process_Freebox(void *actx){
	struct _FreeBox *ctx = actx;	/* Only to avoid zillions of cast */
	char l[MAXLINE];
	struct hostent *server;
	struct sockaddr_in serv_addr;

		/* Sanity checks */
	if(!ctx->topic){
		fputs("*E* configuration error : no topic specified, ignoring this section\n", stderr);
		pthread_exit(0);
	}
	if(!(server = gethostbyname( FBX_HOST ))){
		perror( FBX_HOST );
		fputs("*E* Stopping this thread\n", stderr);
		pthread_exit(0);
	}

	memset( &serv_addr, '0', sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	memcpy( &serv_addr.sin_addr.s_addr, server->h_addr_list, server->h_length );
	serv_addr.sin_port = htons( FBX_PORT );

	if(debug)
		printf("Launching a processing flow for Freebox\n");

	for(;;){	/* Infinite loop to process data */
		int sockfd = socket(AF_INET, SOCK_STREAM, 0);

puts("0");
		if(sockfd < 0){
			fprintf(stderr, "*E* Can't create socket : %s\n", strerror( errno ));
			fputs("*E* Stopping this thread\n", stderr);
			pthread_exit(0);
		}
puts("1");
		connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr));
puts("2");
#if 0
		if(connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0){
/*AF : Send error topic */
puts("2");
			perror("*E* Connecting");
		}
#endif
puts("bip");
		close(sockfd);
		sleep( ctx->sample );
	}

	pthread_exit(0);
}
#endif

void handleInt(int na){
	exit(EXIT_SUCCESS);
}

int main(int ac, char **av){
	const char *conf_file = DEFAULT_CONFIGURATION_FILE;
	pthread_attr_t thread_attr;
	MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;

	if(ac > 0){
		int i;
		for(i=1; i<ac; i++){
			if(!strcmp(av[i], "-h")){
				fprintf(stderr, "%s (%s)\n"
					"Publish Smart Home figures to an MQTT broker\n"
					"Known options are :\n"
					"\t-h : this online help\n"
					"\t-d : enable debug messages\n"
					"\t-f<file> : read <file> for configuration\n"
					"\t\t(default is '%s')\n",
					basename(av[0]), VERSION, DEFAULT_CONFIGURATION_FILE
				);
				exit(EXIT_FAILURE);
			} else if(!strcmp(av[i], "-d")){
				debug = 1;
				puts("Marcel (c) L.Faillie 2015");
				printf("%s (%s) starting ...\n", basename(av[0]), VERSION);
			} else if(!strncmp(av[i], "-f", 2))
				conf_file = av[i] + 2;
			else {
				fprintf(stderr, "Unknown option '%s'\n%s -h\n\tfor some help\n", av[i], av[0]);
				exit(EXIT_FAILURE);
			}
		}
	}
	read_configuration( conf_file );

	if(!cfg.sections){
		fputs("*F* No section defined : giving up ...\n", stderr);
		exit(EXIT_FAILURE);
	}

		/* Connecting to the broker */
	conn_opts.reliable = 0;
	MQTTClient_create( &cfg.client, cfg.Broker, "Marcel", MQTTCLIENT_PERSISTENCE_NONE, NULL);
	MQTTClient_setCallbacks( cfg.client, NULL, connlost, msgarrived, NULL);

	switch( MQTTClient_connect( cfg.client, &conn_opts) ){
	case MQTTCLIENT_SUCCESS : 
		break;
	case 1 : fputs("Unable to connect : Unacceptable protocol version\n", stderr);
		exit(EXIT_FAILURE);
	case 2 : fputs("Unable to connect : Identifier rejected\n", stderr);
		exit(EXIT_FAILURE);
	case 3 : fputs("Unable to connect : Server unavailable\n", stderr);
		exit(EXIT_FAILURE);
	case 4 : fputs("Unable to connect : Bad user name or password\n", stderr);
		exit(EXIT_FAILURE);
	case 5 : fputs("Unable to connect : Not authorized\n", stderr);
		exit(EXIT_FAILURE);
	default :
		fputs("Unable to connect : Unknown version\n", stderr);
		exit(EXIT_FAILURE);
	}
	atexit(brkcleaning);

		/* Creating childs */
	assert(!pthread_attr_init (&thread_attr));
	assert(!pthread_attr_setdetachstate (&thread_attr, PTHREAD_CREATE_DETACHED));
	for(union CSection *s = cfg.sections; s; s = s->common.next){
		switch(s->common.section_type){
		case MSEC_FFV:
/* ATTENTION, lorsque plusieurs types seront implémenté, on peut grouper les sections
 * uniquement s'ils sont du même type
 */
			if(s->common.sample){	/* Creates only if sample is set */
				if(pthread_create( &(s->common.thread), &thread_attr, process_FFV, s) < 0){
					fputs("*F* Can't create a processing thread\n", stderr);
					exit(EXIT_FAILURE);
				}
			}
			break;
#ifdef FREEBOX
		case MSEC_FREEBOX:
			if(!s->common.sample){
				fputs("*E* Freebox section without sample time : ignoring ...\n", stderr);
			} else if(pthread_create( &(s->common.thread), &thread_attr, process_Freebox, s) < 0){
				fputs("*F* Can't create a processing thread\n", stderr);
				exit(EXIT_FAILURE);
			}
			break;
#endif
		default :	/* Ignore unsupported type */
			break;
		}
	}

	signal(SIGINT, handleInt);
	pause();

	exit(EXIT_SUCCESS);
}

