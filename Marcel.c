/*
 * Marcel
 *	A daemon to publish some smart home data to MQTT broker
 *
 *	Compilation
gcc -std=c99 -lpthread -lpaho-mqtt3c -Wall Marcel.c -o Marcel
 *
 * Additional options :
 *	-DFREEBOX : enable Freebox statistics
 *	-DUPS : enable UPS statistics (NUT needed)
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
 *	28/05/2015	- LF - v1.2 - Adding UPS
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

#define VERSION "1.2"
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

size_t socketreadline( int fd, char *l, size_t sz){
/* read a line :
 * -> 	fd : file descriptor to read
 *		l : buffer to store the result
 *		sz : max size of the result
 * <- size read, -1 if error or EoF
 */
	int s=0;
	char *p = l, c;

	for(;;){
		int r = read( fd, &c, 1);

		if(r == -1)
			return -1;
		else if(!r){	/* EOF */
			return -1;
		} else if(c == '\n')
			break;
		else {
			*p++ = c;
			s++;
		}
	}
	*p = '\0';

	return s;
}

	/*
	 * Configuration
	 */
enum _tp_msec {
	MSEC_INVALID =0,	/* Ignored */
	MSEC_FFV,			/* File String Value */
	MSEC_FREEBOX,		/* FreeBox */
	MSEC_UPS			/* UPS */
};

struct var {	/* Storage for var list */
	struct var *next;
	const char *name;
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
	struct _UPS {
		union CSection *next;
		enum _tp_msec section_type;
		int sample;
		pthread_t thread;
		const char *topic;
		const char *section_name;
		const char *host;
		int port;
		struct var *var_list;
	} Ups;
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
				puts("Entering section 'Freebox'");
		} else if((arg = striKWcmp(l,"*UPS="))){
			union CSection *n = malloc( sizeof(struct _UPS) );
			assert(n);
			memset(n, 0, sizeof(struct _UPS));
			n->common.section_type = MSEC_UPS;

			assert( n->Ups.section_name = strdup( removeLF(arg) ) );

			if(last_section)
				last_section->common.next = n;
			else	/* First section */
				cfg.sections = n;
			last_section = n;
			if(debug)
				printf("Entering section 'UPS/%s'\n", n->Ups.section_name);
		} else if((arg = striKWcmp(l,"File="))){
			if(!last_section || last_section->common.section_type != MSEC_FFV){
				fputs("*F* Configuration issue : File directive outside a FFV section\n", stderr);
					exit(EXIT_FAILURE);
			}
			assert( last_section->FFV.file = strdup( removeLF(arg) ));
			if(debug)
				printf("\tFile : '%s'\n", last_section->FFV.file);
		} else if((arg = striKWcmp(l,"Host="))){
			if(!last_section || last_section->common.section_type != MSEC_UPS){
				fputs("*F* Configuration issue : Host directive outside a UPS section\n", stderr);
					exit(EXIT_FAILURE);
			}
			assert( last_section->Ups.host = strdup( removeLF(arg) ));
			if(debug)
				printf("\tHost : '%s'\n", last_section->Ups.host);
		} else if((arg = striKWcmp(l,"Port="))){
			if(!last_section || last_section->common.section_type != MSEC_UPS){
				fputs("*F* Configuration issue : Port directive outside a UPS section\n", stderr);
				exit(EXIT_FAILURE);
			}
			if(!(last_section->Ups.port = atoi(arg))){
				fputs("*F* Configuration issue : Port is null (or is not a number)\n", stderr);
				exit(EXIT_FAILURE);
			}
			if(debug)
				printf("\tPort : %d\n", last_section->Ups.port);
		} else if((arg = striKWcmp(l,"Var="))){
			if(!last_section || last_section->common.section_type != MSEC_UPS){
				fputs("*F* Configuration issue : Var directive outside a UPS section\n", stderr);
				exit(EXIT_FAILURE);
			}
			struct var *v = malloc(sizeof(struct var));
			assert(v);
			assert( v->name = strdup( removeLF(arg) ));
			v->next = last_section->Ups.var_list;
			last_section->Ups.var_list = v;
			if(debug)
				printf("\tVar : '%s'\n", v->name);
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

#define FBX_REQ "GET "FBX_URI" HTTP/1.0\n\n"

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

	memset( &serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons( FBX_PORT );
	memcpy(&serv_addr.sin_addr.s_addr,*server->h_addr_list,server->h_length);

	if(debug)
		printf("Launching a processing flow for Freebox\n");

	for(;;){	/* Infinite loop to process data */
		int sockfd = socket(AF_INET, SOCK_STREAM, 0);

		if(sockfd < 0){
			fprintf(stderr, "*E* Can't create socket : %s\n", strerror( errno ));
			fputs("*E* Stopping this thread\n", stderr);
			pthread_exit(0);
		}
		if(connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0){
/*AF : Send error topic */
			perror("*E* Connecting");
		} else if( send(sockfd, FBX_REQ, strlen(FBX_REQ), 0) == -1 ){
/*AF : Send error topic */
			perror("*E* Sending");
		} else while( socketreadline(sockfd, l, sizeof(l)) != -1 ){
			if(strstr(l, "ATM")){
				int u, d, lm;
				if(sscanf(l+25,"%d", &d) != 1) d=-1;
				if(sscanf(l+44,"%d", &u) != 1) u=-1;

				lm = sprintf(l, "%s/DownloadATM", ctx->topic) + 2;
				assert( lm+1 < MAXLINE-10 );	/* Enough space for the response ? */
				sprintf( l+lm, "%d", d );
				papub( l, strlen(l+lm), l+lm, 0 );
				if(debug)
					printf("Freebox : %s -> %s\n", l, l+lm);

				lm = sprintf(l, "%s/UploadATM", ctx->topic) + 2;
				assert( lm+1 < MAXLINE-10 );	/* Enough space for the response ? */
				sprintf( l+lm, "%d", u );
				papub( l, strlen(l+lm), l+lm, 0 );
				if(debug)
					printf("Freebox : %s -> %s\n", l, l+lm);
			} else if(striKWcmp(l, "  Marge de bruit")){
				float u, d; 
				int lm;

				if(sscanf(l+25,"%f", &d) != 1) d = -1;
				if(sscanf(l+44,"%f", &u) != 1) u = -1;

				lm = sprintf(l, "%s/DownloadMarge", ctx->topic) + 2;
				assert( lm+1 < MAXLINE-10 );	/* Enough space for the response ? */
				sprintf( l+lm, "%.2f", d );
				papub( l, strlen(l+lm), l+lm, 0 );
				if(debug)
					printf("Freebox : %s -> %s\n", l, l+lm);

				lm = sprintf(l, "%s/UploadMarge", ctx->topic) + 2;
				assert( lm+1 < MAXLINE-10 );	/* Enough space for the response ? */
				sprintf( l+lm, "%.2f", u );
				papub( l, strlen(l+lm), l+lm, 0 );
				if(debug)
					printf("Freebox : %s -> %s\n", l, l+lm);
			} else if(striKWcmp(l, "  WAN")){
				int u, d, lm;

				if(sscanf(l+40,"%d", &d) != 1) d = -1;
				if(sscanf(l+55,"%d", &u) != 1) u = -1;

				lm = sprintf(l, "%s/DownloadWIFI", ctx->topic) + 2;
				assert( lm+1 < MAXLINE-10 );	/* Enough space for the response ? */
				sprintf( l+lm, "%d", d );
				papub( l, strlen(l+lm), l+lm, 0 );
				if(debug)
					printf("Freebox : %s -> %s\n", l, l+lm);

				lm = sprintf(l, "%s/UploadWIFI", ctx->topic) + 2;
				assert( lm+1 < MAXLINE-10 );	/* Enough space for the response ? */
				sprintf( l+lm, "%d", u );
				papub( l, strlen(l+lm), l+lm, 0 );
				if(debug)
					printf("Freebox : %s -> %s\n", l, l+lm);
			} else if(striKWcmp(l, "  Ethernet")){
				int u, d, lm;

				if(sscanf(l+40,"%d", &d) != 1) d = -1;
				if(sscanf(l+55,"%d", &u) != 1) u = -1;

				lm = sprintf(l, "%s/DownloadTV", ctx->topic) + 2;
				assert( lm+1 < MAXLINE-10 );	/* Enough space for the response ? */
				sprintf( l+lm, "%d", d );
				papub( l, strlen(l+lm), l+lm, 0 );
				if(debug)
					printf("Freebox : %s -> %s\n", l, l+lm);

				lm = sprintf(l, "%s/UploadTV", ctx->topic) + 2;
				assert( lm+1 < MAXLINE-10 );	/* Enough space for the response ? */
				sprintf( l+lm, "%d", u );
				papub( l, strlen(l+lm), l+lm, 0 );
				if(debug)
					printf("Freebox : %s -> %s\n", l, l+lm);
			} else if(striKWcmp(l, "  USB")){
				int u, d, lm;

				if(sscanf(l+40,"%d", &d) != 1) d = -1;
				if(sscanf(l+55,"%d", &u) != 1) u = -1;

				lm = sprintf(l, "%s/DownloadUSB", ctx->topic) + 2;
				assert( lm+1 < MAXLINE-10 );	/* Enough space for the response ? */
				sprintf( l+lm, "%d", d );
				papub( l, strlen(l+lm), l+lm, 0 );
				if(debug)
					printf("Freebox : %s -> %s\n", l, l+lm);

				lm = sprintf(l, "%s/UploadUSB", ctx->topic) + 2;
				assert( lm+1 < MAXLINE-10 );	/* Enough space for the response ? */
				sprintf( l+lm, "%d", u );
				papub( l, strlen(l+lm), l+lm, 0 );
				if(debug)
					printf("Freebox : %s -> %s\n", l, l+lm);
			} else if(striKWcmp(l, "  Switch")){
				int u, d, lm;

				if(sscanf(l+40,"%d", &d) != 1) d = -1;
				if(sscanf(l+55,"%d", &u) != 1) u = -1;

				lm = sprintf(l, "%s/DownloadLan", ctx->topic) + 2;
				assert( lm+1 < MAXLINE-10 );	/* Enough space for the response ? */
				sprintf( l+lm, "%d", d );
				papub( l, strlen(l+lm), l+lm, 0 );
				if(debug)
					printf("Freebox : %s -> %s\n", l, l+lm);

				lm = sprintf(l, "%s/UploadLan", ctx->topic) + 2;
				assert( lm+1 < MAXLINE-10 );	/* Enough space for the response ? */
				sprintf( l+lm, "%d", u );
				papub( l, strlen(l+lm), l+lm, 0 );
				if(debug)
					printf("Freebox : %s -> %s\n", l, l+lm);
			}
		}

		close(sockfd);
		sleep( ctx->sample );
	}

	pthread_exit(0);
}
#endif

#ifdef UPS
void *process_UPS(void *actx){
	struct _UPS *ctx = actx;	/* Only to avoid zillions of cast */
	char l[MAXLINE];
	struct hostent *server;
	struct sockaddr_in serv_addr;

		/* Sanity checks */
	if(!ctx->topic){
		fputs("*E* configuration error : no topic specified, ignoring this section\n", stderr);
		pthread_exit(0);
	}
	if(!ctx->host || !ctx->port){
		fputs("*E* configuration error : Don't know how to reach NUT\n", stderr);
		pthread_exit(0);
	}

	if(!(server = gethostbyname( ctx->host ))){
		perror( ctx->host );
		fputs("*E* Stopping this thread\n", stderr);
		pthread_exit(0);
	}

	memset( &serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons( ctx->port );
	memcpy(&serv_addr.sin_addr.s_addr,*server->h_addr_list,server->h_length);

	if(debug)
		printf("Launching a processing flow for UPS/%s\n", ctx->section_name);

	for(;;){	/* Infinite loop to process data */
		int sockfd = socket(AF_INET, SOCK_STREAM, 0);

		if(sockfd < 0){
			fprintf(stderr, "*E* Can't create socket : %s\n", strerror( errno ));
			fputs("*E* Stopping this thread\n", stderr);
			pthread_exit(0);
		}
		if(connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0){
/*AF : Send error topic */
			perror("*E* Connecting");
		} else {
/*AF : Test only
 * add list of variables into configuration and loop on them
 */
			const char *cmd = "GET VAR onduleur ups.load\n";
			if( send(sockfd, cmd , strlen(cmd), 0) == -1 ){
/*AF : Send error topic */
				perror("*E* Sending");
			} else {
printf("=> %d\n", socketreadline(sockfd, l, sizeof(l)) );
puts(l);
			}

			close(sockfd);
			sleep( ctx->sample );
		}
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
				printf("%s v%s starting ...\n", basename(av[0]), VERSION);
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
#ifdef UPS
		case MSEC_UPS:
			if(!s->common.sample){ /* we won't group UPS to prevent too many DNS lookup */
				fputs("*E* UPS section without sample time : ignoring ...\n", stderr);
			} else if(pthread_create( &(s->common.thread), &thread_attr, process_UPS, s) < 0){
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

