/* RFXtrx_marcel.c
 * 	Code used to handle RFXtrx device
 *
 * This file is part of Marcel project and is following the same
 * license rules (see LICENSE file).
 * In addition the code present in this file is based on RFXtrx SDK which is
 * a property of RFXCOM. Consequently, communication protocol RFXtrx protocol
 * may only be used for RFXCOM equipment.
 *
 * 30/04/2016 - LF - First version
 * 20/08/2016	- LF - handles Disabled
 */

#ifdef RFXTRX

#include "RFXtrx_marcel.h"

#include <termios.h>
#include <fcntl.h>	/* open() */
#include <unistd.h>	/* read(), write() */
#include <string.h>	/* memset() */
#include <assert.h>
#include <signal.h>
#include <setjmp.h>
#include <errno.h>

typedef uint8_t BYTE;	/* Compatibility */
#include "RFXtrx.h"

RBUF buff;					/* Communication buffer */
pthread_mutex_t oneTRXcmd;	/* One command can be send at a time */

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

inline ssize_t writeRFX( int fd ){
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
 * Initialise RFXtrx
 *
 * Note : there is no need to lock the FD as it is only used before any other
 * threads are created.
 */
static jmp_buf env_alarm;
static void sig_alarm(int signo){
    longjmp(env_alarm, 1);
}

void init_RFX(){
	if(!cfg.RFXdevice)
		return;	/* No device defined */

	int fd;
	if((fd = open (cfg.RFXdevice, O_RDWR | O_NOCTTY | O_SYNC)) < 0 ){	/* Open the port */
		publishLog('E', "RFX open() : %s", strerror(errno));
		publishLog('F', "RFXtrx disabled");
		cfg.RFXdevice = NULL;
		return;
	}

		/* Set attributes */
	struct termios tty;
	memset (&tty, 0, sizeof tty);
	if(tcgetattr (fd, &tty)){
		publishLog('E', "RFX tcgetattr() : %s", strerror(errno));
		publishLog('F', "RFXtrx disabled");
		close(fd);
		cfg.RFXdevice = NULL;
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
		cfg.RFXdevice = NULL;
		return;
	}

	assert( signal(SIGALRM, sig_alarm) != SIG_ERR );
	if(setjmp(env_alarm) != 0){
		publishLog('E', "Timeout during RFXtrx initialisation");
		publishLog('F', "RFXtrx disabled");
		close(fd);
		cfg.RFXdevice = NULL;
		return;
	}

	alarm(10);	/* The initialisation has to be done within 10s */

	clearbuff( 0x0d );	/* Sending reset */
	dumpbuff();
	if(writeRFX(fd) == -1){
		publishLog('E', "RFX reset write() : %s", strerror(errno));
		publishLog('F', "RFXtrx disabled");
		close(fd);
		cfg.RFXdevice = NULL;
		return;
	}
	publishLog('I', "RFXtrx reset sent");
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
		cfg.RFXdevice = NULL;
		return;
	}
	publishLog('I', "RFXtrx GET STATUS sent");
	
	if(!readRFX(fd)){
		publishLog('E', "RFX Reading status : %s", strerror(errno));
		publishLog('F', "RFXtrx disabled");
		close(fd);
		cfg.RFXdevice = NULL;
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
		cfg.RFXdevice = NULL;
		return;
	}
	publishLog('I', "RFXtrx START COMMAND sent");

	if(!readRFX(fd)){
		publishLog('E', "RFX Reading status : %s", strerror(errno));
		publishLog('F', "RFXtrx disabled");
		close(fd);
		cfg.RFXdevice = NULL;
		return;
	} else {
		dumpbuff();
	}

	close(fd);
	alarm(0);	/* Initialisation is over */


	pthread_mutex_init( &oneTRXcmd, NULL );
}

void processRTSCmd( struct _RTSCmd *ctx, const char *msg ){
	if(ctx->disabled){
		publishLog('I', "Commanding RTSCmd '%s' is disabled", ctx->uid);
		return;
	}

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
		publishLog('E', "[%s] RTS unsupported command : '%s'", ctx->uid, msg);
		return;
	}
		
	if((fd = open (cfg.RFXdevice, O_RDWR | O_NOCTTY | O_SYNC)) < 0 ){
		publishLog('E', "RFX open() : %s", strerror(errno));
		return;
	}

	pthread_mutex_lock( &oneTRXcmd );
	clearbuff( 0x0c );
	buff.RFY.packettype = pTypeRFY;
	buff.RFY.subtype = sTypeRFY;
	buff.RFY.seqnbr = 0;
	buff.RFY.id1 = ctx->did >> 24;
	buff.RFY.id2 = (ctx->did >> 16) & 0xff;
	buff.RFY.id3 = (ctx->did >> 8) & 0xff;
	buff.RFY.unitcode = ctx->did & 0xff;
	buff.RFY.cmnd = cmd;
	dumpbuff();
	if(writeRFX(fd) == -1)
		publishLog('E', "RFX Cmd write() : %s", strerror(errno));
	else if(!readRFX(fd))
		publishLog('E', "RFX Reading status : %s", strerror(errno));

	pthread_mutex_unlock( &oneTRXcmd );
	close(fd);

	publishLog('I', "Sending '%s' (%d) command to '%s' (%04x)\n", msg, cmd, ctx->uid, ctx->did);
}
#endif
