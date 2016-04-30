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
 */

#ifdef RFXTRX

#include "RFXtrx_marcel.h"

#include <termios.h>
#include <stdint.h>	/* uint8_t */
#include <fcntl.h>	/* open() */
#include <unistd.h>	/* read(), write() */
#include <string.h>	/* memset() */
#include <assert.h>

typedef uint8_t BYTE;	/* Compatibility */
#include "RFXtrx.h"

RBUF buff;					/* Communication buffer */
pthread_mutex_t oneTRXcmd;	/* One command can be send at a time */

/*
 * Utilities functions
 */

#ifdef DEBUG
status void dumpbuff(){
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

int readRFX( int fd ){
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
 * Note : there is no need to lock the FD as it only used before any other
 * threads are created.
 */

void init_RFX(){
	if(!cfg.RFXdevice)
		return;	/* No device defined */

	int fd;
	if((fd = open (cfg.RFXdevice, O_RDWR | O_NOCTTY | O_SYNC)) < 0 ){	/* Open the port */
		perror("RFX open()");
		cfg.RFXdevice = NULL;
		fputs("*E* RFXtrx disabled.\n", stderr);
		return;
	}

		/* Set attributes */
	struct termios tty;
	memset (&tty, 0, sizeof tty);
	if(tcgetattr (fd, &tty)){
		perror("RFX tcgetattr()");
		close(fd);
		cfg.RFXdevice = NULL;
		fputs("*E* RFXtrx disabled.\n", stderr);
		return;
	}
	cfsetospeed(&tty, B38400);
	cfsetispeed(&tty, B38400);
	tty.c_cflag = (tty.c_cflag & ~(CSIZE | PARENB | CSTOPB /*| CRTSCTS*/)) | CS8 | CLOCAL | CREAD;
	tty.c_iflag &= ~(IGNBRK | IXON | IXOFF | IXANY);
	tty.c_lflag = 0;
	tty.c_oflag = 0;
	if(tcsetattr(fd, TCSANOW, &tty)){
		perror("RFX tcsetattr()");
		close(fd);
		cfg.RFXdevice = NULL;
		fputs("*E* RFXtrx disabled.\n", stderr);
		return;
	}

	clearbuff( 0x0d );
	dumpbuff();
	if(writeRFX(fd) == -1){
		perror("RFX write()");
		close(fd);
		cfg.RFXdevice = NULL;
		fputs("*E* RFXtrx disabled.\n", stderr);
		return;
	}
	if(verbose)
		puts("*I* RFXtrx reset sent");
	sleep(1);
	tcflush( fd, TCIFLUSH );	/* Clear input buffer */

	pthread_mutex_init( &oneTRXcmd, NULL );
}

#endif
