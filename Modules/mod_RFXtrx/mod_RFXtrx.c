/* mod_RFXtrx
 *
 * Handle RFXtrx devices (like the RFXCom)
 *
 * This file is part of Marcel project and is following the same
 * license rules (see LICENSE file)
 *
 */

#include "mod_RFXtrx.h"
#ifdef LUA
#	include "../mod_Lua/mod_Lua.h"
#endif
#include "../Marcel/MQTT_tools.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <fcntl.h>	/* open() */
#include <unistd.h>	/* read(), write() */
#include <termios.h>
#include <signal.h>
#include <setjmp.h>
#include <errno.h>

static struct module_RFXtrx mod_RFXtrx;

static int publishCustomFiguresRFXCmd(struct Section *asection){
#ifdef LUA
	if(mod_Lua){
		struct section_RFXCom *s = (struct section_RFXCom *)asection;

		lua_newtable(mod_Lua->L);

		lua_pushstring(mod_Lua->L, "DeviceID");			/* Push the index */
		lua_pushnumber(mod_Lua->L, s->did);	/* the value */
		lua_rawset(mod_Lua->L, -3);	/* Add it in the table */

		char t[9];
		sprintf(t, "%04x", s->did);

		lua_pushstring(mod_Lua->L, "DeviceID Hexa");
		lua_pushstring(mod_Lua->L, t);
		lua_rawset(mod_Lua->L, -3);

		lua_pushstring(mod_Lua->L, "Topic");
		lua_pushstring(mod_Lua->L, s->section.topic);
		lua_rawset(mod_Lua->L, -3);

		lua_pushstring(mod_Lua->L, "Error state");			/* Push the index */
		lua_pushboolean(mod_Lua->L, s->inerror);	/* the value */
		lua_rawset(mod_Lua->L, -3);	/* Add it in the table */

		return 1;
	} else
#endif
	return 0;
}

static void sr_postconfInit(struct Section *);
static bool sr_processMQTT(struct Section *, const char *, char *);

/* Section identifiers */
enum {
	ST_CMD = 0,	/* RFXtrx commands */
};

static enum RC_readconf readconf(uint8_t mid, const char *l, struct Section **section ){
	const char *arg; /* Argument of the configuration directive */

	if((arg = striKWcmp(l,"RFXtrx_Port="))){ /* "serial" port where RFXcom is plugged */
		if(*section){
			publishLog('F', "RFXtrx_Port= can't be part of a section");
			exit(EXIT_FAILURE);
		}
		if(mod_RFXtrx.RFXdevice){
			publishLog('F', "RFXtrx_Port= can be only defined once");
			exit(EXIT_FAILURE);
		}
		assert( (mod_RFXtrx.RFXdevice = strdup(arg)) );

		if(cfg.verbose)
			publishLog('C', "\tRFXtrx's device : '%s'", mod_RFXtrx.RFXdevice);

		return ACCEPTED;
	} else if((arg = striKWcmp(l,"*RTSCmd="))){	/* Starting a section definition */
		if(findSectionByName(arg)){
			publishLog('F', "Section '%s' is already defined", arg);
			exit(EXIT_FAILURE);
		}

		struct section_RFXCom *nsection = malloc(sizeof(struct section_RFXCom));
		initSection( (struct Section *)nsection, mid, ST_CMD, strdup(arg), "RTSCmd");

		nsection->section.publishCustomFigures = publishCustomFiguresRFXCmd;

			/* This section is processing MQTT messages */
		nsection->section.postconfInit = sr_postconfInit;	/* Subscribe */
		nsection->section.processMsg = sr_processMQTT;		/* Processing */

		nsection->did = 0;

		if(cfg.verbose)
			publishLog('C', "\tEntering RTSCmd section '%s' (%04x)", nsection->section.uid, nsection->section.id);

		*section = (struct Section *)nsection;	/* we're now in a section */
		return ACCEPTED;
	} else if(section){
		if((arg = striKWcmp(l,"ID="))){
			acceptSectionDirective(*section, "ID=");
			((struct section_RFXCom *)*section)->did = strtol(arg, NULL, 0);

			if(cfg.verbose)
				publishLog('C', "\t\tID : %04x", ((struct section_RFXCom *)*section)->did);

			return ACCEPTED;
		}
	}

	return REJECTED;
}

static bool mr_acceptSDirective( uint8_t sec_id, const char *directive ){
	if(sec_id == ST_CMD){
		if( !strcmp(directive, "Disabled") )
			return true;
		else if( !strcmp(directive, "ID=") )
			return true;
		else if( !strcmp(directive, "Topic=") )
			return true;
	}
	return false;
}

	/* ***
	 * RFX's own
	 * ***/
typedef uint8_t BYTE;	/* Compatibility */

#include "RFXtrx.h"

static RBUF buff;					/* Communication buffer */
static pthread_mutex_t oneTRXcmd;	/* One command can be send at a time */

/*
 * Utilities functions
 */

#ifdef DEBUG
static void dumpbuff(){
	int i;
	printf("buff.ICMND.packetlength = %02x", buff.ICMND.packetlength);
	printf("\nbuff.ICMND.packettype = %02x\n", buff.ICMND.packettype);
	printf("buff.ICMND.subtype = %02x\n", buff.ICMND.subtype);
	printf("buff.ICMND.seqnbr = %02x\n", buff.ICMND.seqnbr);
	printf("buff.ICMND.cmnd = %02x\n", buff.ICMND.cmnd);
	for(i=0; i<16; i++)
		printf(" %02x", ((char *)&buff.IRESPONSE.msg1)[i]);
	puts("\n");
}
#else
#	define dumpbuff()
#endif

static void clearbuff( int len ){
	memset( &buff, 0, len+1);
	buff.ICMND.packetlength = len;
}

static ssize_t writeRFX( int fd ){
	return write( fd, &buff, buff.ICMND.packetlength +1 );
}

static int readRFX( int fd ){
	BYTE *p = &buff.ICMND.packetlength;
	ssize_t n2read;	/* Number of bytes to read */

	assert(read( fd, &buff.ICMND.packetlength, 1 ));	/* Reading msg size */
	n2read = buff.ICMND.packetlength;
	p++;

	assert(buff.ICMND.packetlength < sizeof(buff) / sizeof(BYTE) -1);

	while( n2read ){
		ssize_t n;
		
		if(!(n = read( fd, p, n2read )))
			return 0;	/* Error ! */
		n2read -= n;
		p += n;
	}

	return buff.ICMND.packetlength;
}

/*
 * MQTT
 */
static void sr_postconfInit(struct Section *asec){
	struct section_RFXCom *s = (struct section_RFXCom *)asec;	/* avoid lot of casting */

		/* Sanity checks
		 * As they're highlighting configuration issue, let's
		 * consider error as fatal.
		 */
	if(!s->section.topic){
		publishLog('F', "[%s] Topic must be set. Dying ...", s->section.uid);
		exit(EXIT_FAILURE);
	}

		/* Subscribing */
	if(MQTTClient_subscribe( cfg.client, s->section.topic, 0 ) != MQTTCLIENT_SUCCESS ){
		publishLog('F', "[%s]Can't subscribe to '%s'", s->section.uid, s->section.topic);
		exit(EXIT_FAILURE);
	}
}

static bool sr_processMQTT(struct Section *asec, const char *topic, char *msg){
	struct section_RFXCom *s = (struct section_RFXCom *)asec;	/* avoid lot of casting */
	if(!mqtttokcmp(s->section.topic, topic, NULL)){
		if(s->section.disabled || !mod_RFXtrx.RFXdevice){
			publishLog('I', "[%s] or RFX is disabled", s->section.uid);
			return true;	/* We understood the command but nothing is done */
		}

		s->inerror = true;	/* by default, we're in trouble */

		BYTE cmd;
		int fd;

		if(!strcmp(msg,"Stop") || !strcmp(msg,"My"))
			cmd = rfy_sStop;
		else if(!strcmp(msg,"Up"))
			cmd = rfy_sUp;
		else if(!strcmp(msg,"Down"))
			cmd = rfy_sDown;
		else if(!strcmp(msg,"Program"))
			cmd = rfy_sProgram;
		else {
			publishLog('E', "[%s] RTS unsupported command : '%s'", s->section.uid, msg);
			return true;
		}
		
		if((fd = open (mod_RFXtrx.RFXdevice, O_RDWR | O_NOCTTY | O_SYNC)) < 0 ){
			publishLog('E', "[%s] RFX open() : %s", s->section.uid, strerror(errno));
			return true;
		}

		pthread_mutex_lock( &oneTRXcmd );
		clearbuff( 0x0c );
		buff.RFY.packettype = pTypeRFY;
		buff.RFY.subtype = sTypeRFY;
		buff.RFY.seqnbr = 0;
		buff.RFY.id1 = s->did >> 24;
		buff.RFY.id2 = (s->did >> 16) & 0xff;
		buff.RFY.id3 = (s->did >> 8) & 0xff;
		buff.RFY.unitcode = s->did & 0xff;
		buff.RFY.cmnd = cmd;
		dumpbuff();
		if(writeRFX(fd) == -1)
			publishLog('E', "[%s] RFX Cmd write() : %s", s->section.uid, strerror(errno));
		else if(!readRFX(fd))
			publishLog('E', "[%s] RFX Reading status : %s", s->section.uid, strerror(errno));

		pthread_mutex_unlock( &oneTRXcmd );
		close(fd);

		s->inerror = false;

		publishLog('T', "[%s] Sending '%s' (%d) command to %04x", s->section.uid, msg, cmd, s->did);

		return true;	/* we processed the message */
	}

	return false;	/* Let's try with other sections */
}

/*
 * Initialise RFXtrx
 *
 * Note : there is no need to lock the FD as it is only used before any other
 * threads are created.
 */
static jmp_buf env_alarm;
static void sig_alarm(int signo){
    longjmp(env_alarm, 1);
}

static void init_RFX( uint8_t mid ){
	if(!mod_RFXtrx.RFXdevice)
		return;	/* No device defined */

	int fd;
	if((fd = open (mod_RFXtrx.RFXdevice, O_RDWR | O_NOCTTY | O_SYNC)) < 0 ){	/* Open the port */
		publishLog('E', "RFX open() : %s", strerror(errno));
		publishLog('F', "RFXtrx disabled");
		mod_RFXtrx.RFXdevice = NULL;
		return;
	}

		/* Set attributes */
	struct termios tty;
	memset (&tty, 0, sizeof tty);
	if(tcgetattr (fd, &tty)){
		publishLog('E', "RFX tcgetattr() : %s", strerror(errno));
		publishLog('F', "RFXtrx disabled");
		close(fd);
		mod_RFXtrx.RFXdevice = NULL;
		return;
	}
	cfsetospeed(&tty, B38400);
	cfsetispeed(&tty, B38400);
	tty.c_cflag = (tty.c_cflag & ~(CSIZE | PARENB | CSTOPB /*| CRTSCTS*/)) | CS8 | CLOCAL | CREAD;
	tty.c_iflag &= ~(IGNBRK | IXON | IXOFF | IXANY);
	tty.c_lflag = 0;
	tty.c_oflag = 0;
	if(tcsetattr(fd, TCSANOW, &tty)){
		publishLog('E', "RFX tcsetattr() : %s", strerror(errno));
		publishLog('F', "RFXtrx disabled");
		close(fd);
		mod_RFXtrx.RFXdevice = NULL;
		return;
	}

	assert( signal(SIGALRM, sig_alarm) != SIG_ERR );
	if(setjmp(env_alarm) != 0){
		publishLog('E', "Timeout during RFXtrx initialisation");
		publishLog('F', "RFXtrx disabled");
		close(fd);
		mod_RFXtrx.RFXdevice = NULL;
		return;
	}

	alarm(10);	/* The initialisation has to be done within 10s */

	clearbuff( 0x0d );	/* Sending reset */
	dumpbuff();
	if(writeRFX(fd) == -1){
		publishLog('E', "RFX reset write() : %s", strerror(errno));
		publishLog('F', "RFXtrx disabled");
		close(fd);
		mod_RFXtrx.RFXdevice = NULL;
		return;
	}
	publishLog('T', "RFXtrx reset sent");
	sleep(1);
	tcflush( fd, TCIFLUSH );	/* Clear input buffer */

	clearbuff( 0x0d );	/* Get Status command */
	buff.ICMND.packettype = pTypeInterfaceControl;
	buff.ICMND.subtype = sTypeInterfaceCommand;
	buff.ICMND.seqnbr = 1;
	buff.ICMND.cmnd = cmdSTATUS;
	dumpbuff();
	if(writeRFX(fd) == -1){
		publishLog('E', "RFX GET STATUS write() : %s", strerror(errno));
		publishLog('F', "RFXtrx disabled");
		close(fd);
		mod_RFXtrx.RFXdevice = NULL;
		return;
	}
	publishLog('T', "RFXtrx GET STATUS sent");
	
	if(!readRFX(fd)){
		publishLog('E', "RFX Reading status : %s", strerror(errno));
		publishLog('F', "RFXtrx disabled");
		close(fd);
		mod_RFXtrx.RFXdevice = NULL;
		return;
	} else {
		dumpbuff();
	}

	clearbuff( 0x0d );	/* Start command */
	buff.ICMND.packettype = pTypeInterfaceControl;
	buff.ICMND.subtype = sTypeInterfaceCommand;
	buff.ICMND.seqnbr = 2;
	buff.ICMND.cmnd = sTypeRecStarted;
	dumpbuff();
	if(writeRFX(fd) == -1){
		publishLog('E', "RFX START write() : %s", strerror(errno));
		publishLog('F', "RFXtrx disabled");
		close(fd);
		mod_RFXtrx.RFXdevice = NULL;
		return;
	}
	publishLog('T', "RFXtrx START COMMAND sent");

	if(!readRFX(fd)){
		publishLog('E', "RFX Reading status : %s", strerror(errno));
		publishLog('F', "RFXtrx disabled");
		close(fd);
		mod_RFXtrx.RFXdevice = NULL;
		return;
	} else {
		dumpbuff();
	}

	close(fd);
	alarm(0);	/* Initialisation is over */


	pthread_mutex_init( &oneTRXcmd, NULL );
}

#ifdef LUA
static int so_inError(lua_State *L){
	struct section_RFXCom **s = luaL_testudata(L, 1, "RTSCmd");
	luaL_argcheck(L, s != NULL, 1, "'RTSCmd' expected");

	lua_pushboolean(L, (*s)->inerror);
	return 1;
}

static const struct luaL_Reg soM[] = {
	{"inError", so_inError},
	{NULL, NULL}
};
#endif


void InitModule( void ){
	initModule((struct Module *)&mod_RFXtrx, "mod_RFXtrx"); /* Identify the module */

		/* Callbacks */
	mod_RFXtrx.module.readconf = readconf;
	mod_RFXtrx.module.acceptSDirective = mr_acceptSDirective;
	mod_RFXtrx.module.postconfInit = init_RFX;

		/* Register the module */
	registerModule( (struct Module *)&mod_RFXtrx );

		/*
		 * Do internal initialization
		 */
	mod_RFXtrx.RFXdevice = NULL;

#ifdef LUA
	if(mod_Lua){ /* Is mod_Lua loaded ? */

			/* Expose shared methods */
		mod_Lua->initSectionSharedMethods(mod_Lua->L, "RTSCmd");

			/* Expose mod_owm's own function */
		mod_Lua->exposeObjMethods(mod_Lua->L, "RTSCmd", soM);
	}
#endif
}
