/* CHK=0x9FD3 */
/*+-------------------------------------------------------------------------
	tsz.c

  This is a version of sz 1.44 for use with ECU over telnet.
  This sz has been modified from Chuck Forsberg's ZMODEM 1.44
  to properly escape 0377 (255 decimal, 0xFF) characters for
  shipping through a telnet session. 0377 is is a telnet IAC
  character and must be escaped.

  ECU code (ecusz) has been corrected and does not require a
  special rz program.

  Compile with:
  cc -o tsz tsz.c            on a SYSV-like system
  cc -o tsz -DV7 tsz.c       on a BSD-like system

  Prepare to get MANY warnings with gcc

  This code has been modified by wht@wht.net.
  Please do not bug Chuck with problems you find!!!

  Defined functions:
	UPDC32(b, c)
	alrm()
	bibi(n)
	bttyout(c)
	canit()
	chartest(m)
	chkinvok(s)
	countem(argc, argv)
	cucheck()
	filbuf(buf, count)
	flushmo(id)
	from_cu()
	getinsync(flag)
	getnak()
	getspeed(code)
	getzrxinit()
	main(argc, argv)
	mode(n)
	noxrd7()
	onintr()
	purgeline()
	rclhdr(hdr)
	readline(n)
	readock(timeout, count)
	saybibi()
	sendbrk()
	sendzsinit()
	stohdr(pos)
	substr(s, t)
	usage()
	vfile(f, a, b, c)
	wcputsec(buf, sectnum, cseclen)
	wcs(oname)
	wcsend(argc, argp)
	wctx(flen)
	wctxpn(name)
	xputs(str)
	xsendline(c)
	zdlread()
	zfilbuf(buf, count)
	zgeth1()
	zgethdr(hdr, eflag)
	zgethex()
	zperr(s, p, u)
	zputhex(c)
	zrbhdr(hdr)
	zrbhdr32(hdr)
	zrdat32(buf, length)
	zrdata(buf, length)
	zrhhdr(hdr)
	zsbh32(hdr, type)
	zsbhdr(type, hdr)
	zsda32(buf, length, frameend)
	zsdata(buf, length, frameend)
	zsendcmd(buf, blen)
	zsendfdata()
	zsendfile(buf, blen)
	zsendline(c)
	zshhdr(type, hdr)

--------------------------------------------------------------------------*/
/*+:EDITS:*/
/*:04-26-2000-11:16-wht@bob-RELEASE 4.42 */
/*:01-24-1997-02:38-wht@yuriatin-SOURCE RELEASE 4.00 */
/*:09-11-1996-20:01-wht@yuriatin-3.48-major telnet,curses,structural overhaul */
/*:12-28-1995-12:54-wht@kepler-Andrey Chernov FreeBSD fixes */
/*:11-29-1995-18:12-wht@kepler-combine files into one and name telnet_sz.c */
/*:11-29-1995-16:54-wht@kepler-add our own blocking/logging */
/*:07-25-1991-23:53-root@n4hgf-recover from shorter blklen */

#define VERSION "sz 1.44.1 mod for telnet by wht@n4hgf"
#define PUBDIR "/usr/spool/uucppublic"
#define NFGVMIN

#ifndef V7
#define SV
#endif

char *substr(), *getenv();

#define LOGFILE "/tmp/szlog"

#include <stdio.h>
#include <signal.h>
#include <setjmp.h>
#include <ctype.h>

#define PATHLEN 256
#define OK 0
#define FALSE 0
#define TRUE 1
#define ERROR (-1)

int Zmodem = 0;				 /* ZMODEM protocol requested by receiver */
unsigned Baudrate;
unsigned Txwindow;			 /* Control the size of the transmitted window */
unsigned Txwspac;			 /* Spacing between zcrcq requests */
unsigned Txwcnt;			 /* Counter used to space ack requests */
long Lrxpos;				 /* Receiver's last reported offset */
int errors;
char checked = 0;

#ifdef V7
#include <sys/types.h>
#include <sys/stat.h>
#include <sgtty.h>
#define OS "V7/BSD"
#ifdef LLITOUT
long Locmode;				 /* Saved "local mode" for 4.x BSD "new
							  * driver" */
long Locbit = LLITOUT;		 /* Bit SUPPOSED to disable output
							  * translations */
#include <strings.h>
#endif
#endif

#ifndef OS
#ifndef USG
#define USG
#endif
#endif

#ifdef USG
#include <sys/types.h>
#include <sys/stat.h>
#include <termio.h>
#include <sys/ioctl.h>
#define OS "SYS III/V"
#define MODE2OK
#include <string.h>
#endif

/*
 * return 1 iff stdout and stderr are different devices
 *  indicating this program operating with a modem on a
 *  different line
 */
int Fromcu;					 /* Were called from cu or yam */
from_cu()
{
	struct stat a, b;

	fstat(1, &a);
	fstat(2, &b);
	Fromcu = a.st_rdev != b.st_rdev;
	return;
}
cucheck()
{
	if (Fromcu)
		fprintf(stderr, "Please read the manual page BUGS chapter!\r\n");
}

struct
{
	unsigned baudr;
	int speedcode;
}
speeds[] =
{
	110, B110,
	300, B300,
	600, B600,
	1200, B1200,
	2400, B2400,
	4800, B4800,
	9600, B9600,
	19200, EXTA,
	38400, EXTB,
	0,
};

int Twostop;				 /* Use two stop bits */

static unsigned
getspeed(code)
{
	register n;

	for (n = 0; speeds[n].baudr; ++n)
		if (speeds[n].speedcode == code)
			return speeds[n].baudr;
	return 38400;			 /* Assume fifo if ioctl failed */
}

#ifdef ICANON
struct termio oldtty, tty;

#else
struct sgttyb oldtty, tty;
struct tchars oldtch, tch;

#endif

int iofd = 0;				 /* File descriptor for ioctls & reads */

/*
 * mode(n)
 *  3: save old tty stat, set raw mode with flow control
 *  2: set XON/XOFF for sb/sz with ZMODEM or YMODEM-g
 *  1: save old tty stat, set raw mode
 *  0: restore original tty mode
 */
mode(n)
{
	static did0 = FALSE;

	vfile("mode:%d", n);
	switch (n)
	{
#ifdef USG
		case 2:			 /* Un-raw mode used by sz, sb when -g
							  * detected */
			if (!did0)
				(void)ioctl(iofd, TCGETA, &oldtty);
			tty = oldtty;

			tty.c_iflag = BRKINT | IXON;

			tty.c_oflag = 0; /* Transparent output */

			tty.c_cflag &= ~PARENB;	/* Disable parity */
			tty.c_cflag |= CS8;	/* Set character size = 8 */
			if (Twostop)
				tty.c_cflag |= CSTOPB;	/* Set two stop bits */

			tty.c_lflag = ISIG;
			tty.c_cc[VINTR] = Zmodem ? 03 : 030;	/* Interrupt char */
			tty.c_cc[VQUIT] = -1;	/* Quit char */
			tty.c_cc[VMIN] = 1;
			tty.c_cc[VTIME] = 1;	/* or in this many tenths of seconds */

			(void)ioctl(iofd, TCSETAW, &tty);
			did0 = TRUE;
			return OK;
		case 1:
		case 3:
			if (!did0)
				(void)ioctl(iofd, TCGETA, &oldtty);
			tty = oldtty;

			tty.c_iflag = n == 3 ? (IGNBRK | IXOFF) : IGNBRK;

			/* No echo, crlf mapping, INTR, QUIT, delays, no erase/kill */
			tty.c_lflag &= ~(ECHO | ICANON | ISIG);

			tty.c_oflag = 0; /* Transparent output */

			tty.c_cflag &= ~PARENB;	/* Same baud rate, disable parity */
			tty.c_cflag |= CS8;	/* Set character size = 8 */
			if (Twostop)
				tty.c_cflag |= CSTOPB;	/* Set two stop bits */
			tty.c_cc[VMIN] = 1;	/* This many chars satisfies reads */
			tty.c_cc[VTIME] = 1;	/* or in this many tenths of seconds */
			(void)ioctl(iofd, TCSETAW, &tty);
			did0 = TRUE;
			Baudrate = getspeed(tty.c_cflag & CBAUD);
			return OK;
#endif
#ifdef V7

			/*
			 * NOTE: this should transmit all 8 bits and at the same time
			 * respond to XOFF/XON flow control.  If no FIONREAD or other
			 * READCHECK alternative, also must respond to INTRRUPT char
			 * This doesn't work with V7.  It should work with LLITOUT,
			 * but LLITOUT was broken on the machine I tried it on.
			 */
		case 2:			 /* Un-raw mode used by sz, sb when -g
							  * detected */
			if (!did0)
			{
				ioctl(iofd, TIOCEXCL, 0);
				ioctl(iofd, TIOCGETP, &oldtty);
				ioctl(iofd, TIOCGETC, &oldtch);
#ifdef LLITOUT
				ioctl(TIOCLGET, &Locmode);	/* Get "local mode" */
#endif
			}
			tty = oldtty;
			tch = oldtch;
			tch.t_intrc = Zmodem ? 03 : 030;	/* Interrupt char */
			tty.sg_flags |= (ODDP | EVENP | CBREAK);
			tty.sg_flags &= ~(ALLDELAY | CRMOD | ECHO | LCASE);
			ioctl(iofd, TIOCSETP, &tty);
			ioctl(iofd, TIOCSETC, &tch);
#ifdef LLITOUT
			ioctl(TIOCLBIS, &Locbit);
#endif
			bibi(99);		 /* un-raw doesn't work w/o lit out */
			did0 = TRUE;
			return OK;
		case 1:
		case 3:
			if (!did0)
			{
				ioctl(iofd, TIOCEXCL, 0);
				ioctl(iofd, TIOCGETP, &oldtty);
				ioctl(iofd, TIOCGETC, &oldtch);
#ifdef LLITOUT
				ioctl(TIOCLGET, &Locmode);	/* Get "local mode" */
#endif
			}
			tty = oldtty;
			tty.sg_flags |= RAW;
			tty.sg_flags &= ~ECHO;
			ioctl(iofd, TIOCSETP, &tty);
			did0 = TRUE;
			Baudrate = getspeed(tty.sg_ospeed);
			return OK;
#endif
		case 0:
			if (!did0)
				return ERROR;
#ifdef USG
			(void)ioctl(iofd, TCSBRK, 1);	/* Wait for output to drain */
			(void)ioctl(iofd, TCFLSH, 1);	/* Flush input queue */
			(void)ioctl(iofd, TCSETAW, &oldtty);	/* Restore modes */
			(void)ioctl(iofd, TCXONC, 1);	/* Restart output */
#endif
#ifdef V7
			ioctl(iofd, TIOCSETP, &oldtty);
			ioctl(iofd, TIOCSETC, &oldtch);
			ioctl(iofd, TIOCNXCL, 0);
#ifdef LLITOUT
			ioctl(TIOCLSET, &Locmode);	/* Restore "local mode" */
#endif
#endif

			return OK;
		default:
			return ERROR;
	}
}

sendbrk()
{
#ifdef V7
#ifdef TIOCSBRK
#define CANBREAK
	sleep(1);
	ioctl(iofd, TIOCSBRK, 0);
	sleep(1);
	ioctl(iofd, TIOCCBRK, 0);
#endif
#endif
#ifdef USG
#define CANBREAK
	ioctl(iofd, TCSBRK, 0);
#endif
}

/* crctab calculated by Mark G. Mendel, Network Systems Corporation */
static unsigned short crctab[256] =
{
	0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50a5, 0x60c6, 0x70e7,
	0x8108, 0x9129, 0xa14a, 0xb16b, 0xc18c, 0xd1ad, 0xe1ce, 0xf1ef,
	0x1231, 0x0210, 0x3273, 0x2252, 0x52b5, 0x4294, 0x72f7, 0x62d6,
	0x9339, 0x8318, 0xb37b, 0xa35a, 0xd3bd, 0xc39c, 0xf3ff, 0xe3de,
	0x2462, 0x3443, 0x0420, 0x1401, 0x64e6, 0x74c7, 0x44a4, 0x5485,
	0xa56a, 0xb54b, 0x8528, 0x9509, 0xe5ee, 0xf5cf, 0xc5ac, 0xd58d,
	0x3653, 0x2672, 0x1611, 0x0630, 0x76d7, 0x66f6, 0x5695, 0x46b4,
	0xb75b, 0xa77a, 0x9719, 0x8738, 0xf7df, 0xe7fe, 0xd79d, 0xc7bc,
	0x48c4, 0x58e5, 0x6886, 0x78a7, 0x0840, 0x1861, 0x2802, 0x3823,
	0xc9cc, 0xd9ed, 0xe98e, 0xf9af, 0x8948, 0x9969, 0xa90a, 0xb92b,
	0x5af5, 0x4ad4, 0x7ab7, 0x6a96, 0x1a71, 0x0a50, 0x3a33, 0x2a12,
	0xdbfd, 0xcbdc, 0xfbbf, 0xeb9e, 0x9b79, 0x8b58, 0xbb3b, 0xab1a,
	0x6ca6, 0x7c87, 0x4ce4, 0x5cc5, 0x2c22, 0x3c03, 0x0c60, 0x1c41,
	0xedae, 0xfd8f, 0xcdec, 0xddcd, 0xad2a, 0xbd0b, 0x8d68, 0x9d49,
	0x7e97, 0x6eb6, 0x5ed5, 0x4ef4, 0x3e13, 0x2e32, 0x1e51, 0x0e70,
	0xff9f, 0xefbe, 0xdfdd, 0xcffc, 0xbf1b, 0xaf3a, 0x9f59, 0x8f78,
	0x9188, 0x81a9, 0xb1ca, 0xa1eb, 0xd10c, 0xc12d, 0xf14e, 0xe16f,
	0x1080, 0x00a1, 0x30c2, 0x20e3, 0x5004, 0x4025, 0x7046, 0x6067,
	0x83b9, 0x9398, 0xa3fb, 0xb3da, 0xc33d, 0xd31c, 0xe37f, 0xf35e,
	0x02b1, 0x1290, 0x22f3, 0x32d2, 0x4235, 0x5214, 0x6277, 0x7256,
	0xb5ea, 0xa5cb, 0x95a8, 0x8589, 0xf56e, 0xe54f, 0xd52c, 0xc50d,
	0x34e2, 0x24c3, 0x14a0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405,
	0xa7db, 0xb7fa, 0x8799, 0x97b8, 0xe75f, 0xf77e, 0xc71d, 0xd73c,
	0x26d3, 0x36f2, 0x0691, 0x16b0, 0x6657, 0x7676, 0x4615, 0x5634,
	0xd94c, 0xc96d, 0xf90e, 0xe92f, 0x99c8, 0x89e9, 0xb98a, 0xa9ab,
	0x5844, 0x4865, 0x7806, 0x6827, 0x18c0, 0x08e1, 0x3882, 0x28a3,
	0xcb7d, 0xdb5c, 0xeb3f, 0xfb1e, 0x8bf9, 0x9bd8, 0xabbb, 0xbb9a,
	0x4a75, 0x5a54, 0x6a37, 0x7a16, 0x0af1, 0x1ad0, 0x2ab3, 0x3a92,
	0xfd2e, 0xed0f, 0xdd6c, 0xcd4d, 0xbdaa, 0xad8b, 0x9de8, 0x8dc9,
	0x7c26, 0x6c07, 0x5c64, 0x4c45, 0x3ca2, 0x2c83, 0x1ce0, 0x0cc1,
	0xef1f, 0xff3e, 0xcf5d, 0xdf7c, 0xaf9b, 0xbfba, 0x8fd9, 0x9ff8,
	0x6e17, 0x7e36, 0x4e55, 0x5e74, 0x2e93, 0x3eb2, 0x0ed1, 0x1ef0
};

/*
 * updcrc macro derived from article Copyright (C) 1986 Stephen Satchell.
 *  NOTE: First srgument must be in range 0 to 255.
 *        Second argument is referenced twice.
 *
 * Programmers may incorporate any or all code into their programs,
 * giving proper credit within the source. Publication of the
 * source routines is permitted so long as proper credit is given
 * to Stephen Satchell, Satchell Evaluations and Chuck Forsberg,
 * Omen Technology.
 */

#define updcrc(cp, crc) ( crctab[((crc >> 8) & 255)] ^ (crc << 8) ^ cp)

/*
 * Copyright (C) 1986 Gary S. Brown.  You may use this program, or
 * code or tables extracted from it, as desired without restriction.
 */

/* First, the polynomial itself and its table of feedback terms.  The  */
/* polynomial is                                                       */
/* X^32+X^26+X^23+X^22+X^16+X^12+X^11+X^10+X^8+X^7+X^5+X^4+X^2+X^1+X^0 */
/* Note that we take it "backwards" and put the highest-order term in  */
/* the lowest-order bit.  The X^32 term is "implied"; the LSB is the   */
/* X^31 term, etc.  The X^0 term (usually shown as "+1") results in    */
/* the MSB being 1.                                                    */

/* Note that the usual hardware shift register implementation, which   */
/* is what we're using (we're merely optimizing it by doing eight-bit  */
/* chunks at a time) shifts bits into the lowest-order term.  In our   */
/* implementation, that means shifting towards the right.  Why do we   */
/* do it this way?  Because the calculated CRC must be transmitted in  */
/* order from highest-order term to lowest-order term.  UARTs transmit */
/* characters in order from LSB to MSB.  By storing the CRC this way,  */
/* we hand it to the UART in the order low-byte to high-byte; the UART */
/* sends each low-bit to hight-bit; and the result is transmission bit */
/* by bit from highest- to lowest-order term without requiring any bit */
/* shuffling on our part.  Reception works similarly.                  */

/* The feedback terms table consists of 256, 32-bit entries.  Notes:   */
/*                                                                     */
/*     The table can be generated at runtime if desired; code to do so */
/*     is shown later.  It might not be obvious, but the feedback      */
/*     terms simply represent the results of eight shift/xor opera-    */
/*     tions for all combinations of data and CRC register values.     */
/*                                                                     */
/*     The values must be right-shifted by eight bits by the "updcrc"  */
/*     logic; the shift must be unsigned (bring in zeroes).  On some   */
/*     hardware you could probably optimize the shift in assembler by  */
/*     using byte-swap instructions.                                   */

static long cr3tab[] =
{							 /* CRC polynomial 0xedb88320 */
	0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419,
	0x706af48f, 0xe963a535, 0x9e6495a3, 0x0edb8832, 0x79dcb8a4,
	0xe0d5e91e, 0x97d2d988, 0x09b64c2b, 0x7eb17cbd, 0xe7b82d07,
	0x90bf1d91, 0x1db71064, 0x6ab020f2, 0xf3b97148, 0x84be41de,
	0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7, 0x136c9856,
	0x646ba8c0, 0xfd62f97a, 0x8a65c9ec, 0x14015c4f, 0x63066cd9,
	0xfa0f3d63, 0x8d080df5, 0x3b6e20c8, 0x4c69105e, 0xd56041e4,
	0xa2677172, 0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b,
	0x35b5a8fa, 0x42b2986c, 0xdbbbc9d6, 0xacbcf940, 0x32d86ce3,
	0x45df5c75, 0xdcd60dcf, 0xabd13d59, 0x26d930ac, 0x51de003a,
	0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423, 0xcfba9599,
	0xb8bda50f, 0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
	0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d, 0x76dc4190,
	0x01db7106, 0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f,
	0x9fbfe4a5, 0xe8b8d433, 0x7807c9a2, 0x0f00f934, 0x9609a88e,
	0xe10e9818, 0x7f6a0dbb, 0x086d3d2d, 0x91646c97, 0xe6635c01,
	0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e, 0x6c0695ed,
	0x1b01a57b, 0x8208f4c1, 0xf50fc457, 0x65b0d9c6, 0x12b7e950,
	0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3,
	0xfbd44c65, 0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2,
	0x4adfa541, 0x3dd895d7, 0xa4d1c46d, 0xd3d6f4fb, 0x4369e96a,
	0x346ed9fc, 0xad678846, 0xda60b8d0, 0x44042d73, 0x33031de5,
	0xaa0a4c5f, 0xdd0d7cc9, 0x5005713c, 0x270241aa, 0xbe0b1010,
	0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
	0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17,
	0x2eb40d81, 0xb7bd5c3b, 0xc0ba6cad, 0xedb88320, 0x9abfb3b6,
	0x03b6e20c, 0x74b1d29a, 0xead54739, 0x9dd277af, 0x04db2615,
	0x73dc1683, 0xe3630b12, 0x94643b84, 0x0d6d6a3e, 0x7a6a5aa8,
	0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1, 0xf00f9344,
	0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb,
	0x196c3671, 0x6e6b06e7, 0xfed41b76, 0x89d32be0, 0x10da7a5a,
	0x67dd4acc, 0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5,
	0xd6d6a3e8, 0xa1d1937e, 0x38d8c2c4, 0x4fdff252, 0xd1bb67f1,
	0xa6bc5767, 0x3fb506dd, 0x48b2364b, 0xd80d2bda, 0xaf0a1b4c,
	0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55, 0x316e8eef,
	0x4669be79, 0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
	0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f, 0xc5ba3bbe,
	0xb2bd0b28, 0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31,
	0x2cd99e8b, 0x5bdeae1d, 0x9b64c2b0, 0xec63f226, 0x756aa39c,
	0x026d930a, 0x9c0906a9, 0xeb0e363f, 0x72076785, 0x05005713,
	0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38, 0x92d28e9b,
	0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21, 0x86d3d2d4, 0xf1d4e242,
	0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1,
	0x18b74777, 0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c,
	0x8f659eff, 0xf862ae69, 0x616bffd3, 0x166ccf45, 0xa00ae278,
	0xd70dd2ee, 0x4e048354, 0x3903b3c2, 0xa7672661, 0xd06016f7,
	0x4969474d, 0x3e6e77db, 0xaed16a4a, 0xd9d65adc, 0x40df0b66,
	0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
	0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605,
	0xcdd70693, 0x54de5729, 0x23d967bf, 0xb3667a2e, 0xc4614ab8,
	0x5d681b02, 0x2a6f2b94, 0xb40bbe37, 0xc30c8ea1, 0x5a05df1b,
	0x2d02ef8d
};

#ifdef NFGM
long
UPDC32(b, c)
long c;
{
	return (cr3tab[((int)c ^ b) & 0xff] ^ ((c >> 8) & 0x00FFFFFF));
}

#else

#define UPDC32(b, c) (cr3tab[((int)c ^ b) & 0xff] ^ ((c >> 8) & 0x00FFFFFF))
#endif

/* End of rbsb.c */
/* vi: set tabstop=4 shiftwidth=4 */
int Filesleft;
long Totalleft;

/*
 * Attention string to be executed by receiver to interrupt streaming data
 *  when an error is detected.  A pause (0336) may be needed before the
 *  ^C (03) or after it.
 */
char Myattn[] =
{0};

FILE *in;

/* Ward Christensen / CP/M parameters - Don't change these! */
#define ENQ 005
#define CAN ('X'&037)
#define XOFF ('s'&037)
#define XON ('q'&037)
#define SOH 1
#define STX 2
#define EOT 4
#define ACK 6
#define NAK 025
#define CPMEOF 032
#define WANTCRC 0103		 /* send C not NAK to get crc not checksum */
#define WANTG 0107			 /* Send G not NAK to get nonstop batch xmsn */
#define TIMEOUT (-2)
#define RCDO (-3)
#define RETRYMAX 10

char Lastrx;
char Crcflg;
int Wcsmask = 0377;
int Verbose = 0;
int Modem2 = 0;				 /* XMODEM Protocol - don't send pathnames */
int Restricted = 0;			 /* restricted; no /.. or ../ in filenames */
int Quiet = 0;				 /* overrides logic that would otherwise set
							  * verbose */
int Ascii = 0;				 /* Add CR's for brain damaged programs */
int Fullname = 0;			 /* transmit full pathname */
int Unlinkafter = 0;		 /* Unlink file after it is sent */
int Dottoslash = 0;			 /* Change foo.bar.baz to foo/bar/baz */
int firstsec;
int errcnt = 0;				 /* number of files unreadable */
int blklen = 128;			 /* length of transmitted records */
int blklen_original;
int Optiong;				 /* Let it rip no wait for sector ACK's */
int Noeofseen;
int Totsecs;				 /* total number of sectors this file */
char txbuf[1024];
int Filcnt = 0;				 /* count of number of files opened */
int Lfseen = 0;
unsigned Rxbuflen = 16384;	 /* Receiver's max buffer length */
int Tframlen = 0;			 /* Override for tx frame length */
int blkopt = 0;				 /* Override value for zmodem blklen */
int Rxflags = 0;
long bytcnt;
int Wantfcs32 = TRUE;		 /* want to send 32 bit FCS */
char Lzconv;				 /* Local ZMODEM file conversion request */
char Lzmanag;				 /* Local ZMODEM file management request */
int Lskipnocor;
char Lztrans;
char zconv;					 /* ZMODEM file conversion request */
char zmanag;				 /* ZMODEM file management request */
char ztrans;				 /* ZMODEM file transport request */
int Command;				 /* Send a command, then exit. */
char *Cmdstr;				 /* Pointer to the command string */
int Cmdtries = 11;
int Cmdack1;				 /* Rx ACKs command, then do it */
int Exitcode;
int Test;					 /* 1= Force receiver to send Attn, etc with
							  * qbf. */
/* 2= Character transparency test */
char *qbf = "The quick brown fox jumped over the lazy dog's back 1234567890\r\n";
long Lastread;				 /* Beginning offset of last buffer read */
int Lastn;					 /* Count of last buffer read or -1 */
int Dontread;				 /* Don't read the buffer, it's still there */
long Lastsync;				 /* Last offset to which we got a ZRPOS */
int Beenhereb4;				 /* How many times we've been ZRPOS'd same
							  * place */

jmp_buf tohere;				 /* For the interrupt on RX timeout */
jmp_buf intrjmp;			 /* For the interrupt on RX CAN */

char s128[128];

#define ZPAD '*'			 /* 052 Padding character begins frames */
#define ZDLE 030			 /* Ctrl-X Zmodem escape - `ala BISYNC DLE */
#define ZDLEE (ZDLE^0100)	 /* Escaped ZDLE as transmitted */
#define ZBIN 'A'			 /* Binary frame indicator */
#define ZHEX 'B'			 /* HEX frame indicator */
#define ZBIN32 'C'			 /* Binary frame with 32 bit FCS */

/* Frame types (see array "frametypes" in zm.c) */
#define ZRQINIT	0			 /* Request receive init */
#define ZRINIT	1			 /* Receive init */
#define ZSINIT 2			 /* Send init sequence (optional) */
#define ZACK 3				 /* ACK to above */
#define ZFILE 4				 /* File name from sender */
#define ZSKIP 5				 /* To sender: skip this file */
#define ZNAK 6				 /* Last packet was garbled */
#define ZABORT 7			 /* Abort batch transfers */
#define ZFIN 8				 /* Finish session */
#define ZRPOS 9				 /* Resume data trans at this position */
#define ZDATA 10			 /* Data packet(s) follow */
#define ZEOF 11				 /* End of file */
#define ZFERR 12			 /* Fatal Read or Write error Detected */
#define ZCRC 13				 /* Request for file CRC and response */
#define ZCHALLENGE 14		 /* Receiver's Challenge */
#define ZCOMPL 15			 /* Request is complete */
#define ZCAN 16				 /* Other end canned session with CAN*5 */
#define ZFREECNT 17			 /* Request for free bytes on filesystem */
#define ZCOMMAND 18			 /* Command from sending program */
#define ZSTDERR 19			 /* Output to standard error, data follows */

/* ZDLE sequences */
#define ZCRCE 'h'			 /* CRC next, frame ends, header packet
							  * follows */
#define ZCRCG 'i'			 /* CRC next, frame continues nonstop */
#define ZCRCQ 'j'			 /* CRC next, frame continues, ZACK expected */
#define ZCRCW 'k'			 /* CRC next, ZACK expected, end of frame */
#define ZRUB0 'l'			 /* Translate to rubout 0177 */
#define ZRUB1 'm'			 /* Translate to rubout 0377 */

/* zdlread return values (internal) */
/* -1 is general error, -2 is timeout */
#define GOTOR 0400
#define GOTCRCE (ZCRCE|GOTOR)/* ZDLE-ZCRCE received */
#define GOTCRCG (ZCRCG|GOTOR)/* ZDLE-ZCRCG received */
#define GOTCRCQ (ZCRCQ|GOTOR)/* ZDLE-ZCRCQ received */
#define GOTCRCW (ZCRCW|GOTOR)/* ZDLE-ZCRCW received */
#define GOTCAN	(GOTOR|030)	 /* CAN*5 seen */

/* Byte positions within header array */
#define ZF0	3				 /* First flags byte */
#define ZF1	2
#define ZF2	1
#define ZF3	0
#define ZP0	0				 /* Low order 8 bits of position */
#define ZP1	1
#define ZP2	2
#define ZP3	3				 /* High order 8 bits of file position */

/* Bit Masks for ZRINIT flags byte ZF0 */
#define CANFDX	01			 /* Rx can send and receive true FDX */
#define CANOVIO	02			 /* Rx can receive data during disk I/O */
#define CANBRK	04			 /* Rx can send a break signal */
#define CANCRY	010			 /* Receiver can decrypt */
#define CANLZW	020			 /* Receiver can uncompress */
#define CANFC32	040			 /* Receiver can use 32 bit Frame Check */
#define ESCCTL 0100			 /* Receiver expects ctl chars to be escaped */
#define ESC8   0200			 /* Receiver expects 8th bit to be escaped */

/* Parameters for ZSINIT frame */
#define ZATTNLEN 32			 /* Max length of attention string */
/* Bit Masks for ZSINIT flags byte ZF0 */
#define TESCCTL 0100		 /* Transmitter expects ctl chars to be
							  * escaped */
#define TESC8   0200		 /* Transmitter expects 8th bit to be escaped */

/* Parameters for ZFILE frame */
/* Conversion options one of these in ZF0 */
#define ZCBIN	1			 /* Binary transfer - inhibit conversion */
#define ZCNL	2			 /* Convert NL to local end of line convention */
#define ZCRESUM	3			 /* Resume interrupted file transfer */
/* Management include options, one of these ored in ZF1 */
#define ZMSKNOLOC	0200	 /* Skip file if not present at rx */
/* Management options, one of these ored in ZF1 */
#define ZMMASK	037			 /* Mask for the choices below */
#define ZMNEWL	1			 /* Transfer if source newer or longer */
#define ZMCRC	2			 /* Transfer if different file CRC or length */
#define ZMAPND	3			 /* Append contents to existing file (if any) */
#define ZMCLOB	4			 /* Replace existing file */
#define ZMNEW	5			 /* Transfer if source newer */
 /* Number 5 is alive ... */
#define ZMDIFF	6			 /* Transfer if dates or lengths different */
#define ZMPROT	7			 /* Protect destination file */
/* Transport options, one of these in ZF2 */
#define ZTLZW	1			 /* Lempel-Ziv compression */
#define ZTCRYPT	2			 /* Encryption */
#define ZTRLE	3			 /* Run Length encoding */
/* Extended options for ZF3, bit encoded */
#define ZXSPARS	64			 /* Encoding for sparse file operations */

/* Parameters for ZCOMMAND frame ZF0 (otherwise 0) */
#define ZCACK1	1			 /* Acknowledge, then do command */

long rclhdr();

#define BFMX 16384
unsigned char buf[BFMX];
unsigned char *bufp = buf;
unsigned int bufcnt = 0;
void
flushmo(id)
int id;
{
	write(1, buf, bufcnt);
	bufp = buf;
	bufcnt = 0;
}
void
xsendline(c)
unsigned char c;
{
	if (c == 0377)
	{
		xsendline(ZDLE);
		xsendline(ZRUB1);
		return;
	}
	if (bufcnt == BFMX)
		flushmo(-1);
	*bufp++ = c;
	bufcnt++;
}
#define sendline(c) xsendline(c & Wcsmask)

/*+-------------------------------------------------------------------------
	xputs(str)
--------------------------------------------------------------------------*/
xputs(str)
char *str;
{
	while (*str)
		xsendline(*str++);
}							 /* end of xputs */

/* called by signal interrupt or terminate to clean things up */
bibi(n)
{
	canit();
	flushmo(100);
	mode(0);
	fprintf(stderr, "sz: caught signal %d; exiting\n", n);
	if (n == SIGQUIT)
		abort();
	if (n == 99)
		fprintf(stderr, "mode(2) in rbsb.c not implemented!!\n");
	cucheck();
	exit(128 + n);
}
/* Called when ZMODEM gets an interrupt (^X) */
onintr()
{
	signal(SIGINT, SIG_IGN);
	longjmp(intrjmp, -1);
}

int Zctlesc = 1;			 /* Encode control characters REQUIRED for
							  * telnet */
int Nozmodem = 0;			 /* If invoked as "sb" */
char *Progname = "sz";
int Zrwindow = 1400;		 /* RX window size (controls garbage count) */

int Rxtimeout = 100;		 /* Tenths of seconds to wait for something */

#ifndef UNSL
#define UNSL
#endif

/* Globals used by ZMODEM functions */
int Rxframeind;				 /* ZBIN ZBIN32, or ZHEX type of frame
							  * received */
int Rxtype;					 /* Type of header received */
int Rxcount;				 /* Count of data bytes received */
char Rxhdr[4];				 /* Received header */
char Txhdr[4];				 /* Transmitted header */
long Rxpos;					 /* Received file position */
long Txpos;					 /* Transmitted file position */
int Txfcs32;				 /* TURE means send binary frames with 32 bit
							  * FCS */
int Crc32t;					 /* Display flag indicating 32 bit CRC being
							  * sent */
int Crc32;					 /* Display flag indicating 32 bit CRC being
							  * received */
int Znulls;					 /* Number of nulls to send at beginning of
							  * ZDATA hdr */
char Attn[ZATTNLEN + 1];	 /* Attention string rx sends to tx on err */

static lastsent;			 /* Last char we sent */
static evenp;				 /* Even parity seen on header */

static char *frametypes[] =
{
	"Carrier Lost",			 /* -3 */
	"TIMEOUT",				 /* -2 */
	"ERROR",				 /* -1 */
#define FTOFFSET 3
	"ZRQINIT",
	"ZRINIT",
	"ZSINIT",
	"ZACK",
	"ZFILE",
	"ZSKIP",
	"ZNAK",
	"ZABORT",
	"ZFIN",
	"ZRPOS",
	"ZDATA",
	"ZEOF",
	"ZFERR",
	"ZCRC",
	"ZCHALLENGE",
	"ZCOMPL",
	"ZCAN",
	"ZFREECNT",
	"ZCOMMAND",
	"ZSTDERR",
	"xxxxx"
#define FRTYPES 22			 /* Total number of frame types in this array */
 /* not including psuedo negative entries */
};

static char masked[] = "8 bit transparent path required";
static char badcrc[] = "Bad CRC";

/* Send ZMODEM binary header hdr of type type */
zsbhdr(type, hdr)
register char *hdr;
{
	register int n;
	register unsigned short crc;

	vfile("zsbhdr: %s %lx", frametypes[type + FTOFFSET], rclhdr(hdr));
	if (type == ZDATA)
		for (n = Znulls; --n >= 0;)
			xsendline(0);

	xsendline(ZPAD);
	xsendline(ZDLE);

	if (Crc32t = Txfcs32)
		zsbh32(hdr, type);
	else
	{
		xsendline(ZBIN);
		zsendline(type);
		crc = updcrc(type, 0);

		for (n = 4; --n >= 0; ++hdr)
		{
			zsendline(*hdr);
			crc = updcrc((0377 & *hdr), crc);
		}
		crc = updcrc(0, updcrc(0, crc));
		zsendline(crc >> 8);
		zsendline(crc);
	}
	if (type != ZDATA)
		flushmo(3);
}

/* Send ZMODEM binary header hdr of type type */
zsbh32(hdr, type)
register char *hdr;
{
	register int n;
	register UNSL long crc;

	xsendline(ZBIN32);
	zsendline(type);
	crc = 0xFFFFFFFFL;
	crc = UPDC32(type, crc);

	for (n = 4; --n >= 0; ++hdr)
	{
		crc = UPDC32((0377 & *hdr), crc);
		zsendline(*hdr);
	}
	crc = ~crc;
	for (n = 4; --n >= 0;)
	{
		zsendline((int)crc);
		crc >>= 8;
	}
}

/* Send ZMODEM HEX header hdr of type type */
zshhdr(type, hdr)
register char *hdr;
{
	register int n;
	register unsigned short crc;

	vfile("zshhdr: %s %lx", frametypes[type + FTOFFSET], rclhdr(hdr));
	sendline(ZPAD);
	sendline(ZPAD);
	sendline(ZDLE);
	sendline(ZHEX);
	zputhex(type);
	Crc32t = 0;

	crc = updcrc(type, 0);
	for (n = 4; --n >= 0; ++hdr)
	{
		zputhex(*hdr);
		crc = updcrc((0377 & *hdr), crc);
	}
	crc = updcrc(0, updcrc(0, crc));
	zputhex(crc >> 8);
	zputhex(crc);

	/* Make it printable on remote machine */
	sendline(015);
	sendline(012);

	/*
	 * Uncork the remote in case a fake XOFF has stopped data flow
	 */
	if (type != ZFIN && type != ZACK)
		sendline(021);
	flushmo(1);
}

/*
 * Send binary array buf of length length, with ending ZDLE sequence frameend
 */
static char *Zendnames[] =
{"ZCRCE", "ZCRCG", "ZCRCQ", "ZCRCW"};

zsdata(buf, length, frameend)
register char *buf;
{
	register unsigned short crc;

	vfile("zsdata: %d %s", length, Zendnames[frameend - ZCRCE & 3]);
	if (Crc32t)
		zsda32(buf, length, frameend);
	else
	{
		crc = 0;
		for (; --length >= 0; ++buf)
		{
			zsendline(*buf);
			crc = updcrc((0377 & *buf), crc);
		}
		xsendline(ZDLE);
		xsendline(frameend);
		crc = updcrc(frameend, crc);

		crc = updcrc(0, updcrc(0, crc));
		zsendline(crc >> 8);
		zsendline(crc);
	}
	if (frameend == ZCRCW)
	{
		xsendline(XON);
		flushmo(2);
	}
}

zsda32(buf, length, frameend)
register char *buf;
{
	register int c;
	register UNSL long crc;

	crc = 0xFFFFFFFFL;
	for (; --length >= 0; ++buf)
	{
		c = *buf & 0377;
		zsendline(c);
		crc = UPDC32(c, crc);
	}
	xsendline(ZDLE);
	xsendline(frameend);
	crc = UPDC32(frameend, crc);

	crc = ~crc;
	for (length = 4; --length >= 0;)
	{
		zsendline((int)crc);
		crc >>= 8;
	}
}

/*
 * Receive array buf of max length with ending ZDLE sequence
 *  and CRC.  Returns the ending character or error code.
 *  NB: On errors may store length+1 bytes!
 */
zrdata(buf, length)
register char *buf;
{
	register int c;
	register unsigned short crc;
	register char *end;
	register int d;

	if (Rxframeind == ZBIN32)
		return zrdat32(buf, length);

	crc = Rxcount = 0;
	end = buf + length;
	while (buf <= end)
	{
		if ((c = zdlread()) & ~0377)
		{
		  crcfoo:
			switch (c)
			{
				case GOTCRCE:
				case GOTCRCG:
				case GOTCRCQ:
				case GOTCRCW:
					crc = updcrc((d = c) & 0377, crc);
					if ((c = zdlread()) & ~0377)
						goto crcfoo;
					crc = updcrc(c, crc);
					if ((c = zdlread()) & ~0377)
						goto crcfoo;
					crc = updcrc(c, crc);
					if (crc & 0xFFFF)
					{
						zperr(badcrc);
						return ERROR;
					}
					Rxcount = length - (end - buf);
					vfile("zrdata: %d  %s", Rxcount,
						Zendnames[d - GOTCRCE & 3]);
					return d;
				case GOTCAN:
					zperr("Sender Canceled");
					return ZCAN;
				case TIMEOUT:
					zperr("TIMEOUT");
					return c;
				default:
					zperr("Bad data subpacket");
					return c;
			}
		}
		*buf++ = c;
		crc = updcrc(c, crc);
	}
	zperr("Data subpacket too long");
	return ERROR;
}

zrdat32(buf, length)
register char *buf;
{
	register int c;
	register UNSL long crc;
	register char *end;
	register int d;

	crc = 0xFFFFFFFFL;
	Rxcount = 0;
	end = buf + length;
	while (buf <= end)
	{
		if ((c = zdlread()) & ~0377)
		{
		  crcfoo:
			switch (c)
			{
				case GOTCRCE:
				case GOTCRCG:
				case GOTCRCQ:
				case GOTCRCW:
					d = c;
					c &= 0377;
					crc = UPDC32(c, crc);
					if ((c = zdlread()) & ~0377)
						goto crcfoo;
					crc = UPDC32(c, crc);
					if ((c = zdlread()) & ~0377)
						goto crcfoo;
					crc = UPDC32(c, crc);
					if ((c = zdlread()) & ~0377)
						goto crcfoo;
					crc = UPDC32(c, crc);
					if ((c = zdlread()) & ~0377)
						goto crcfoo;
					crc = UPDC32(c, crc);
					if (crc != 0xDEBB20E3)
					{
						zperr(badcrc);
						return ERROR;
					}
					Rxcount = length - (end - buf);
					vfile("zrdat32: %d %s", Rxcount,
						Zendnames[d - GOTCRCE & 3]);
					return d;
				case GOTCAN:
					zperr("Sender Canceled");
					return ZCAN;
				case TIMEOUT:
					zperr("TIMEOUT");
					return c;
				default:
					zperr("Bad data subpacket");
					return c;
			}
		}
		*buf++ = c;
		crc = UPDC32(c, crc);
	}
	zperr("Data subpacket too long");
	return ERROR;
}

/*
 * Read a ZMODEM header to hdr, either binary or hex.
 *  eflag controls local display of non zmodem characters:
 *	0:  no display
 *	1:  display printing characters only
 *	2:  display all non ZMODEM characters
 *  On success, set Zmodem to 1, set Rxpos and return type of header.
 *   Otherwise return negative on error.
 *   Return ERROR instantly if ZCRCW sequence, for fast error recovery.
 */
zgethdr(hdr, eflag)
char *hdr;
{
	register int c, n, cancount;

	n = Zrwindow + Baudrate; /* Max bytes before start of frame */
	Rxframeind = Rxtype = 0;

  startover:
	cancount = 5;
  again:
	/* Return immediate ERROR if ZCRCW sequence seen */
	switch (c = readline(Rxtimeout))
	{
		case RCDO:
		case TIMEOUT:
			goto fifi;
		case CAN:
		  gotcan:
			if (--cancount <= 0)
			{
				c = ZCAN;
				goto fifi;
			}
			switch (c = readline(1))
			{
				case TIMEOUT:
					goto again;
				case ZCRCW:
					c = ERROR;
					/* **** FALL THRU TO **** */
				case RCDO:
					goto fifi;
				default:
					break;
				case CAN:
					if (--cancount <= 0)
					{
						c = ZCAN;
						goto fifi;
					}
					goto again;
			}
			/* **** FALL THRU TO **** */
		default:
		  agn2:
			if (--n == 0)
			{
				zperr("Garbage count exceeded");
				return (ERROR);
			}
			if (eflag && ((c &= 0177) & 0140))
				bttyout(c);
			else if (eflag > 1)
				bttyout(c);
#ifdef UNIX
			fflush(stderr);
#endif
			goto startover;
		case ZPAD | 0200:	 /* This is what we want. */
		case ZPAD:			 /* This is what we want. */
			evenp = c & 0200;
			break;
	}
	cancount = 5;
  splat:
	switch (c = noxrd7())
	{
		case ZPAD:
			goto splat;
		case RCDO:
		case TIMEOUT:
			goto fifi;
		default:
			goto agn2;
		case ZDLE:			 /* This is what we want. */
			break;
	}

	switch (c = noxrd7())
	{
		case RCDO:
		case TIMEOUT:
			goto fifi;
		case ZBIN:
			Rxframeind = ZBIN;
			Crc32 = FALSE;
			c = zrbhdr(hdr);
			break;
		case ZBIN32:
			Crc32 = Rxframeind = ZBIN32;
			c = zrbhdr32(hdr);
			break;
		case ZHEX:
			Rxframeind = ZHEX;
			Crc32 = FALSE;
			c = zrhhdr(hdr);
			break;
		case CAN:
			goto gotcan;
		default:
			goto agn2;
	}
	Rxpos = hdr[ZP3] & 0377;
	Rxpos = (Rxpos << 8) + (hdr[ZP2] & 0377);
	Rxpos = (Rxpos << 8) + (hdr[ZP1] & 0377);
	Rxpos = (Rxpos << 8) + (hdr[ZP0] & 0377);
  fifi:
	switch (c)
	{
		case GOTCAN:
			c = ZCAN;
			/* **** FALL THRU TO **** */
		case ZNAK:
		case ZCAN:
		case ERROR:
		case TIMEOUT:
		case RCDO:
			zperr("Got %s", frametypes[c + FTOFFSET]);
			/* **** FALL THRU TO **** */
		default:
			if (c >= -3 && c <= FRTYPES)
				vfile("zgethdr: %s %lx", frametypes[c + FTOFFSET], Rxpos);
			else
				vfile("zgethdr: %d %lx", c, Rxpos);
	}
	return c;
}

/* Receive a binary style header (type and position) */
zrbhdr(hdr)
register char *hdr;
{
	register int c, n;
	register unsigned short crc;

	if ((c = zdlread()) & ~0377)
		return c;
	Rxtype = c;
	crc = updcrc(c, 0);

	for (n = 4; --n >= 0; ++hdr)
	{
		if ((c = zdlread()) & ~0377)
			return c;
		crc = updcrc(c, crc);
		*hdr = c;
	}
	if ((c = zdlread()) & ~0377)
		return c;
	crc = updcrc(c, crc);
	if ((c = zdlread()) & ~0377)
		return c;
	crc = updcrc(c, crc);
	if (crc & 0xFFFF)
	{
		if (evenp)
			zperr(masked);
		zperr(badcrc);
		return ERROR;
	}
#ifdef ZMODEM
	Protocol = ZMODEM;
#endif
	Zmodem = 1;
	return Rxtype;
}

/* Receive a binary style header (type and position) with 32 bit FCS */
zrbhdr32(hdr)
register char *hdr;
{
	register int c, n;
	register UNSL long crc;

	if ((c = zdlread()) & ~0377)
		return c;
	Rxtype = c;
	crc = 0xFFFFFFFFL;
	crc = UPDC32(c, crc);
#ifdef DEBUGZ
	vfile("zrbhdr32 c=%X  crc=%lX", c, crc);
#endif

	for (n = 4; --n >= 0; ++hdr)
	{
		if ((c = zdlread()) & ~0377)
			return c;
		crc = UPDC32(c, crc);
		*hdr = c;
#ifdef DEBUGZ
		vfile("zrbhdr32 c=%X  crc=%lX", c, crc);
#endif
	}
	for (n = 4; --n >= 0;)
	{
		if ((c = zdlread()) & ~0377)
			return c;
		crc = UPDC32(c, crc);
#ifdef DEBUGZ
		vfile("zrbhdr32 c=%X  crc=%lX", c, crc);
#endif
	}
	if (crc != 0xDEBB20E3)
	{
		if (evenp)
			zperr(masked);
		zperr(badcrc);
		return ERROR;
	}
#ifdef ZMODEM
	Protocol = ZMODEM;
#endif
	Zmodem = 1;
	return Rxtype;
}

/* Receive a hex style header (type and position) */
zrhhdr(hdr)
char *hdr;
{
	register int c;
	register unsigned short crc;
	register int n;

	if ((c = zgethex()) < 0)
		return c;
	Rxtype = c;
	crc = updcrc(c, 0);

	for (n = 4; --n >= 0; ++hdr)
	{
		if ((c = zgethex()) < 0)
			return c;
		crc = updcrc(c, crc);
		*hdr = c;
	}
	if ((c = zgethex()) < 0)
		return c;
	crc = updcrc(c, crc);
	if ((c = zgethex()) < 0)
		return c;
	crc = updcrc(c, crc);
	if (crc & 0xFFFF)
	{
		zperr(badcrc);
		return ERROR;
	}
	if (readline(1) == '\r') /* Throw away possible cr/lf */
		readline(1);
#ifdef ZMODEM
	Protocol = ZMODEM;
#endif
	Zmodem = 1;
	return Rxtype;
}

/* Send a byte as two hex digits */
zputhex(c)
register int c;
{
	static char digits[] = "0123456789abcdef";

	if (Verbose > 8)
		vfile("zputhex: %02X", c);
	sendline(digits[(c & 0xF0) >> 4]);
	sendline(digits[(c) & 0xF]);
}

/*
 * Send character c with ZMODEM escape sequence encoding.
 *  Escape XON, XOFF. Escape CR following @ (Telenet net escape)
 */
zsendline(c)
{

	if (c == 0377)
	{
		xsendline(ZDLE);
		xsendline(lastsent = ZRUB1);
	}
	else if (c & 0140)		 /* Quick check for non control characters */
		xsendline(lastsent = c);
	else
	{
		switch (c &= 0377)
		{
			case ZDLE:
				xsendline(ZDLE);
				xsendline(lastsent = (c ^= 0100));
				break;
			case 015:
			case 0215:
				if (!Zctlesc && (lastsent & 0177) != '@')
					goto sendit;
				/* **** FALL THRU TO **** */
			case 020:
			case 021:
			case 023:
			case 0220:
			case 0221:
			case 0223:
				xsendline(ZDLE);
				c ^= 0100;
			  sendit:
				xsendline(lastsent = c);
				break;
			default:
				if (Zctlesc && !(c & 0140))
				{
					xsendline(ZDLE);
					c ^= 0100;
				}
				xsendline(lastsent = c);
		}
	}
}

/* Decode two lower case hex digits into an 8 bit byte value */
zgethex()
{
	register int c;

	c = zgeth1();
	if (Verbose > 8)
		vfile("zgethex: %02X", c);
	return c;
}
zgeth1()
{
	register int c, n;

	if ((c = noxrd7()) < 0)
		return c;
	n = c - '0';
	if (n > 9)
		n -= ('a' - ':');
	if (n & ~0xF)
		return ERROR;
	if ((c = noxrd7()) < 0)
		return c;
	c -= '0';
	if (c > 9)
		c -= ('a' - ':');
	if (c & ~0xF)
		return ERROR;
	c += (n << 4);
	return c;
}

/*
 * Read a byte, checking for ZMODEM escape encoding
 *  including CAN*5 which represents a quick abort
 */
zdlread()
{
	register int c;

  again:
	/* Quick check for non control characters */
	if ((c = readline(Rxtimeout)) & 0140)
		return c;
	switch (c)
	{
		case ZDLE:
			break;
		case 023:
		case 0223:
		case 021:
		case 0221:
			goto again;
		default:
			if (Zctlesc && !(c & 0140))
			{
				goto again;
			}
			return c;
	}
  again2:
	if ((c = readline(Rxtimeout)) < 0)
		return c;
	if (c == CAN && (c = readline(Rxtimeout)) < 0)
		return c;
	if (c == CAN && (c = readline(Rxtimeout)) < 0)
		return c;
	if (c == CAN && (c = readline(Rxtimeout)) < 0)
		return c;
	switch (c)
	{
		case CAN:
			return GOTCAN;
		case ZCRCE:
		case ZCRCG:
		case ZCRCQ:
		case ZCRCW:
			return (c | GOTOR);
		case ZRUB0:
			return 0177;
		case ZRUB1:
			return 0377;
		case 023:
		case 0223:
		case 021:
		case 0221:
			goto again2;
		default:
			if (Zctlesc && !(c & 0140))
			{
				goto again2;
			}
			if ((c & 0140) == 0100)
				return (c ^ 0100);
			break;
	}
	if (Verbose > 1)
		zperr("Bad escape sequence %x", c);
	return ERROR;
}

/*
 * Read a character from the modem line with timeout.
 *  Eat parity, XON and XOFF characters.
 */
noxrd7()
{
	register int c;

	for (;;)
	{
		if ((c = readline(Rxtimeout)) < 0)
			return c;
		switch (c &= 0177)
		{
			case XON:
			case XOFF:
				continue;
			default:
				if (Zctlesc && !(c & 0140))
					continue;
			case '\r':
			case '\n':
			case ZDLE:
				return c;
		}
	}
}

/* Store long integer pos in Txhdr */
stohdr(pos)
long pos;
{
	Txhdr[ZP0] = pos;
	Txhdr[ZP1] = pos >> 8;
	Txhdr[ZP2] = pos >> 16;
	Txhdr[ZP3] = pos >> 24;
}

/* Recover a long integer from a header */
long
rclhdr(hdr)
register char *hdr;
{
	register long l;

	l = (hdr[ZP3] & 0377);
	l = (l << 8) | (hdr[ZP2] & 0377);
	l = (l << 8) | (hdr[ZP1] & 0377);
	l = (l << 8) | (hdr[ZP0] & 0377);
	return l;
}

/* End of zm.c */
/* vi: set tabstop=4 shiftwidth=4 */
main(argc, argv)
char *argv[];
{
	register char *cp;
	register npats;
	int dm;
	char **patts;
	static char xXbuf[BUFSIZ];

	if ((cp = getenv("ZNULLS")) && *cp)
		Znulls = atoi(cp);
	if ((cp = getenv("SHELL")) && (substr(cp, "rsh") || substr(cp, "rksh")))
		Restricted = TRUE;
	from_cu();
	chkinvok(argv[0]);

	Rxtimeout = 600;
	npats = 0;
	if (argc < 2)
		usage();
	setbuf(stdout, xXbuf);
	while (--argc)
	{
		cp = *++argv;
		if (*cp++ == '-' && *cp)
		{
			while (*cp)
			{
				switch (*cp++)
				{
					case '+':
						Lzmanag = ZMAPND;
						break;
					case '1':
						iofd = 1;
						break;
#ifdef CSTOPB
					case '2':
						Twostop = TRUE;
						break;
#endif
					case '7':
						Wcsmask = 0177;
						break;
					case 'a':
						Lzconv = ZCNL;
						Ascii = TRUE;
						break;
					case 'b':
						Lzconv = ZCBIN;
						break;
					case 'C':
						if (--argc < 1)
						{
							usage();
						}
						Cmdtries = atoi(*++argv);
						break;
					case 'i':
						Cmdack1 = ZCACK1;
						/* **** FALL THROUGH TO **** */
					case 'c':
						if (--argc != 1)
						{
							usage();
						}
						Command = TRUE;
						Cmdstr = *++argv;
						break;
					case 'd':
						++Dottoslash;
						/* **** FALL THROUGH TO **** */
					case 'f':
						Fullname = TRUE;
						break;
					case 'e':
						Zctlesc = 1;
						break;
					case 'k':
						blklen = 1024;
						break;
					case 'L':
						if (--argc < 1)
						{
							usage();
						}
						blkopt = atoi(*++argv);
						if (blkopt < 24 || blkopt > 1024)
							usage();
						break;
					case 'l':
						if (--argc < 1)
						{
							usage();
						}
						Tframlen = atoi(*++argv);
						if (Tframlen < 32 || Tframlen > 1024)
							usage();
						break;
					case 'N':
						Lzmanag = ZMNEWL;
						break;
					case 'n':
						Lzmanag = ZMNEW;
						break;
					case 'o':
						Wantfcs32 = FALSE;
						break;
					case 'p':
						Lzmanag = ZMPROT;
						break;
					case 'r':
						Lzconv = ZCRESUM;
					case 'R':
						Restricted = 1;
						break;
					case 'q':
						Quiet = TRUE;
						Verbose = 0;
						break;
					case 't':
						if (--argc < 1)
						{
							usage();
						}
						Rxtimeout = atoi(*++argv);
						if (Rxtimeout < 10 || Rxtimeout > 1000)
							usage();
						break;
					case 'T':
						if (++Test > 1)
						{
							chartest(1);
							chartest(2);
							mode(0);
							exit(0);
						}
						break;
					case 'u':
						++Unlinkafter;
						break;
					case 'v':
						++Verbose;
						break;
					case 'w':
						if (--argc < 1)
						{
							usage();
						}
						Txwindow = atoi(*++argv);
						if (Txwindow < 256)
							Txwindow = 256;
						Txwindow = (Txwindow / 64) * 64;
						Txwspac = Txwindow / 4;
						if (blkopt > Txwspac
							|| (!blkopt && Txwspac < 1024))
							blkopt = Txwspac;
						break;
					case 'X':
						++Modem2;
						break;
					case 'Y':
						Lskipnocor = TRUE;
						/* **** FALLL THROUGH TO **** */
					case 'y':
						Lzmanag = ZMCLOB;
						break;
					default:
						usage();
				}
			}
		}
		else if (!npats && argc > 0)
		{
			if (argv[0][0])
			{
				npats = argc;
				patts = argv;
				if (!strcmp(*patts, "-"))
					iofd = 1;
			}
		}
	}
	if (npats < 1 && !Command && !Test)
		usage();
	if (Verbose)
	{
		if (freopen(LOGFILE, "a", stderr) == NULL)
		{
			sprintf(s128, "Can't open log file %s\n", LOGFILE);
			xputs(s128);
			exit(0200);
		}
		setbuf(stderr, NULL);
	}
	if (Fromcu && !Quiet)
	{
		if (Verbose == 0)
			Verbose = 2;
	}

	mode(1);

	if (signal(SIGINT, bibi) == SIG_IGN)
	{
		signal(SIGINT, SIG_IGN);
		signal(SIGKILL, SIG_IGN);
	}
	else
	{
		signal(SIGINT, bibi);
		signal(SIGKILL, bibi);
	}
	if (!Fromcu)
		signal(SIGQUIT, SIG_IGN);
	signal(SIGTERM, bibi);

	if (!Modem2)
	{
		if (!Nozmodem)
		{
			xputs("rz\r");
			flushmo(101);
		}
		countem(npats, patts);
		if (!Command && !Quiet && Verbose != 1)
		{
			dm = Filesleft + (Totalleft * 11L) / (Baudrate * 6L);
			fprintf(stderr,
				"%s: %d file%s %ld bytes %u.%u minutes\r\n",
				Progname, Filesleft, Filesleft > 1 ? "s" : "", Totalleft,
				dm / 10, dm % 10);
		}
		if (!Nozmodem)
		{
			stohdr(0L);
			if (Command)
				Txhdr[ZF0] = ZCOMMAND;
			zshhdr(ZRQINIT, Txhdr);
		}
	}
	flushmo(102);

	if (Command)
	{
		if (getzrxinit())
		{
			Exitcode = 0200;
			canit();
		}
		else if (zsendcmd(Cmdstr, 1 + strlen(Cmdstr)))
		{
			Exitcode = 0200;
			canit();
		}
	}
	else if (wcsend(npats, patts) == ERROR)
	{
		Exitcode = 0200;
		canit();
	}
	flushmo(103);
	mode(0);
	dm = ((errcnt != 0) | Exitcode);
	if (dm)
		cucheck();
	exit(dm);
	/* NOTREACHED */
}

wcsend(argc, argp)
char *argp[];
{
	register n;

	Crcflg = FALSE;
	firstsec = TRUE;
	bytcnt = -1;
	for (n = 0; n < argc; ++n)
	{
		Totsecs = 0;
		if (wcs(argp[n]) == ERROR)
			return ERROR;
	}
	Totsecs = 0;
	if (Filcnt == 0)
	{						 /* bitch if we couldn't open ANY files */
		if (!Modem2)
		{
#ifdef original_code
			Command = TRUE;
			Cmdstr = "echo \"sz: Can't open any requested files\"";
			if (getnak())
			{
				Exitcode = 0200;
				canit();
			}
			if (!Zmodem)
				canit();
			else if (zsendcmd(Cmdstr, 1 + strlen(Cmdstr)))
			{
				Exitcode = 0200;
				canit();
			}
#else
			canit();
#endif
			Exitcode = 1;
			return OK;
		}
		canit();
		fprintf(stderr, "\r\nCan't open any requested files.\r\n");
		return ERROR;
	}
	if (Zmodem)
		saybibi();
	else if (!Modem2)
		wctxpn("");
	return OK;
}

wcs(oname)
char *oname;
{
	register c;
	register char *p;
	struct stat f;
	char name[PATHLEN];

	strcpy(name, oname);

	if (Restricted)
	{
		/* restrict pathnames to current tree or uucppublic */
		if (substr(name, "../")
			|| (name[0] == '/' && strncmp(name, PUBDIR, strlen(PUBDIR))))
		{
			canit();
			fprintf(stderr, "\r\nsz:\tSecurity Violation\r\n");
			return ERROR;
		}
	}

	if (!strcmp(oname, "-"))
	{
		if ((p = getenv("ONAME")) && *p)
			strcpy(name, p);
		else
			sprintf(name, "s%d.sz", getpid());
		in = stdin;
	}
	else if ((in = fopen(oname, "r")) == NULL)
	{
		++errcnt;
		return OK;			 /* pass over it, there may be others */
	}
	++Noeofseen;
	Lastread = 0;
	Lastn = -1;
	Dontread = FALSE;
	/* Check for directory or block special files */
	fstat(fileno(in), &f);
	c = f.st_mode & S_IFMT;
	if (c == S_IFDIR || c == S_IFBLK)
	{
		fclose(in);
		return OK;
	}

	++Filcnt;
	switch (wctxpn(name))
	{
		case ERROR:
			return ERROR;
		case ZSKIP:
			return OK;
	}
	if (!Zmodem && wctx((long)f.st_size) == ERROR)
		return ERROR;
	if (Unlinkafter)
		unlink(oname);
	return 0;
}

/*
 * generate and transmit pathname block consisting of
 *  pathname (null terminated),
 *  file length, mode time and file mode in octal
 *  as provided by the Unix fstat call.
 *  N.B.: modifies the passed name, may extend it!
 */
wctxpn(name)
char *name;
{
	register char *p, *q;
	char name2[PATHLEN];
	struct stat f;

	if (Modem2)
	{
		if ((in != stdin) && *name && fstat(fileno(in), &f) != -1)
		{
			fprintf(stderr, "Sending %s, %ld blocks: ",
				name, (long)(f.st_size >> 7));
		}
		fprintf(stderr, "Give your local XMODEM receive command now.\r\n");
		return OK;
	}
	zperr("Awaiting pathname nak for %s", *name ? name : "<END>");
	if (!Zmodem)
		if (getnak())
			return ERROR;

	q = (char *)0;
	if (Dottoslash)
	{						 /* change . to . */
		for (p = name; *p; ++p)
		{
			if (*p == '/')
				q = p;
			else if (*p == '.')
				*(q = p) = '/';
		}
		if (q && strlen(++q) > 8)
		{					 /* If name>8 chars */
			q += 8;			 /* make it .ext */
			strcpy(name2, q);/* save excess of name */
			*q = '.';
			strcpy(++q, name2);	/* add it back */
		}
	}

	for (p = name, q = txbuf; *p;)
		if ((*q++ = *p++) == '/' && !Fullname)
			q = txbuf;
	*q++ = 0;
	p = q;
	while (q < (txbuf + 1024))
		*q++ = 0;
	if (!Ascii && (in != stdin) && *name && fstat(fileno(in), &f) != -1)
		sprintf(p, "%lu %lo %o 0 %d %ld", (long)f.st_size, f.st_mtime,
			f.st_mode, Filesleft, Totalleft);
	Totalleft -= f.st_size;
	if (--Filesleft <= 0)
		Totalleft = 0;
	if (Totalleft < 0)
		Totalleft = 0;

	/* force 1k blocks if name won't fit in 128 byte block */
	if (txbuf[125])
		blklen = 1024;
	else
	{						 /* A little goodie for IMP/KMD */
		txbuf[127] = (f.st_size + 127) >> 7;
		txbuf[126] = (f.st_size + 127) >> 15;
	}
	if (Zmodem)
		return zsendfile(txbuf, 1 + strlen(p) + (p - txbuf));
	if (wcputsec(txbuf, 0, 128) == ERROR)
		return ERROR;
	return OK;
}

getnak()
{
	register firstch;

	Lastrx = 0;
	for (;;)
	{
		switch (firstch = readock(800, 1))
		{
			case ZPAD:
				if (getzrxinit())
					return ERROR;
				Ascii = 0;	 /* Receiver does the conversion */
				return FALSE;
			case TIMEOUT:
				zperr("Timeout on pathname");
				return TRUE;
			case WANTG:
#ifdef MODE2OK
				mode(2);	 /* Set cbreak, XON/XOFF, etc. */
#endif
				Optiong = TRUE;
				blklen = 1024;
			case WANTCRC:
				Crcflg = TRUE;
			case NAK:
				return FALSE;
			case CAN:
				if ((firstch = readock(20, 1)) == CAN && Lastrx == CAN)
					return TRUE;
			default:
				break;
		}
		Lastrx = firstch;
	}
}

wctx(flen)
long flen;
{
	register int thisblklen;
	register int sectnum, attempts, firstch;
	long charssent;

	charssent = 0;
	firstsec = TRUE;
	thisblklen = blklen;
	vfile("wctx:file length=%ld", flen);

	while ((firstch = readock(Rxtimeout, 2)) != NAK && firstch != WANTCRC
		&& firstch != WANTG && firstch != TIMEOUT && firstch != CAN)
		;
	if (firstch == CAN)
	{
		zperr("Receiver CANcelled");
		return ERROR;
	}
	if (firstch == WANTCRC)
		Crcflg = TRUE;
	if (firstch == WANTG)
		Crcflg = TRUE;
	sectnum = 0;
	for (;;)
	{
		if (flen <= (charssent + 896L))
			thisblklen = 128;
		if (!filbuf(txbuf, thisblklen))
			break;
		if (wcputsec(txbuf, ++sectnum, thisblklen) == ERROR)
			return ERROR;
		charssent += thisblklen;
	}
	fclose(in);
	attempts = 0;
	do
	{
		purgeline();
		sendline(EOT);
		flushmo(105);
		++attempts;
	}
	while ((firstch = (readock(Rxtimeout, 1)) != ACK) && attempts < RETRYMAX);
	if (attempts == RETRYMAX)
	{
		zperr("No ACK on EOT");
		return ERROR;
	}
	else
		return OK;
}

wcputsec(buf, sectnum, cseclen)
char *buf;
int sectnum;
int cseclen;				 /* data length of this sector to send */
{
	register checksum, wcj;
	register char *cp;
	unsigned oldcrc;
	int firstch;
	int attempts;

	firstch = 0;			 /* part of logic to detect CAN CAN */

	if (Verbose > 2)
		fprintf(stderr, "Sector %3d %2dk\n", Totsecs, Totsecs / 8);
	else if (Verbose > 1)
		fprintf(stderr, "\rSector %3d %2dk ", Totsecs, Totsecs / 8);
	for (attempts = 0; attempts <= RETRYMAX; attempts++)
	{
		Lastrx = firstch;
		sendline(cseclen == 1024 ? STX : SOH);
		sendline(sectnum);
		sendline(-sectnum - 1);
		oldcrc = checksum = 0;
		for (wcj = cseclen, cp = buf; --wcj >= 0;)
		{
			sendline(*cp);
			oldcrc = updcrc((0377 & *cp), oldcrc);
			checksum += *cp++;
		}
		if (Crcflg)
		{
			oldcrc = updcrc(0, updcrc(0, oldcrc));
			sendline((int)oldcrc >> 8);
			sendline((int)oldcrc);
		}
		else
			sendline(checksum);

		if (Optiong)
		{
			firstsec = FALSE;
			return OK;
		}
		firstch = readock(Rxtimeout, (Noeofseen && sectnum) ? 2 : 1);
	  gotnak:
		switch (firstch)
		{
			case CAN:
				if (Lastrx == CAN)
				{
				  cancan:
					zperr("Cancelled");
					return ERROR;
				}
				break;
			case TIMEOUT:
				zperr("Timeout on sector ACK");
				continue;
			case WANTCRC:
				if (firstsec)
					Crcflg = TRUE;
			case NAK:
				zperr("NAK on sector");
				continue;
			case ACK:
				firstsec = FALSE;
				Totsecs += (cseclen >> 7);
				return OK;
			case ERROR:
				zperr("Got burst for sector ACK");
				break;
			default:
				zperr("Got %02x for sector ACK", firstch);
				break;
		}
		for (;;)
		{
			Lastrx = firstch;
			if ((firstch = readock(Rxtimeout, 2)) == TIMEOUT)
				break;
			if (firstch == NAK || firstch == WANTCRC)
				goto gotnak;
			if (firstch == CAN && Lastrx == CAN)
				goto cancan;
		}
	}
	zperr("Retry Count Exceeded");
	return ERROR;
}

/* fill buf with count chars padding with ^Z for CPM */
filbuf(buf, count)
register char *buf;
{
	register c, m;

	if (!Ascii)
	{
		m = read(fileno(in), buf, count);
		if (m <= 0)
			return 0;
		while (m < count)
			buf[m++] = 032;
		return count;
	}
	m = count;
	if (Lfseen)
	{
		*buf++ = 012;
		--m;
		Lfseen = 0;
	}
	while ((c = getc(in)) != EOF)
	{
		if (c == 012)
		{
			*buf++ = 015;
			if (--m == 0)
			{
				Lfseen = TRUE;
				break;
			}
		}
		*buf++ = c;
		if (--m == 0)
			break;
	}
	if (m == count)
		return 0;
	else
		while (--m >= 0)
			*buf++ = CPMEOF;
	return count;
}
/* fill buf with count chars */
zfilbuf(buf, count)
register char *buf;
{
	register c, m;

	m = count;
	while ((c = getc(in)) != EOF)
	{
		*buf++ = c;
		if (--m == 0)
			break;
	}
	return (count - m);
}

/* VARARGS1 */
vfile(f, a, b, c)
register char *f;
{
	if (Verbose > 2)
	{
		fprintf(stderr, f, a, b, c);
		fprintf(stderr, "\n");
	}
}

alrm()
{
	longjmp(tohere, -1);
}

/*
 * readock(timeout, count) reads character(s) from file descriptor 0
 *  (1 <= count <= 3)
 * it attempts to read count characters. If it gets more than one,
 * it is an error unless all are CAN
 * (otherwise, only normal response is ACK, CAN, or C)
 *  Only looks for one if Optiong, which signifies cbreak, not raw input
 *
 * timeout is in tenths of seconds
 */
readock(timeout, count)
{
	register int c;
	static char byt[5];

	if (Optiong)
		count = 1;			 /* Special hack for cbreak */

	flushmo(106);
	if (setjmp(tohere))
	{
		zperr("TIMEOUT");
		return TIMEOUT;
	}
	c = timeout / 10;
	if (c < 2)
		c = 2;
	if (Verbose > 5)
	{
		fprintf(stderr, "Timeout=%d Calling alarm(%d) ", timeout, c);
		byt[1] = 0;
	}
	signal(SIGALRM, alrm);
	alarm(c);
#ifdef ONEREAD
	c = read(iofd, byt, 1);	 /* regulus raw read is unique */
#else
	c = read(iofd, byt, count);
#endif
	alarm(0);
	if (Verbose > 5)
		fprintf(stderr, "ret cnt=%d %x %x\n", c, byt[0], byt[1]);
	if (c < 1)
		return TIMEOUT;
	if (c == 1)
		return (byt[0] & 0377);
	else
		while (c)
			if (byt[--c] != CAN)
				return ERROR;
	return CAN;
}
readline(n)
{
	return (readock(n, 1));
}

purgeline()
{
#ifdef USG
	ioctl(iofd, TCFLSH, 0);
#else
	lseek(iofd, 0L, 2);
#endif
}

/* send cancel string to get the other end to shut up */
canit()
{
	static char canistr[] =
	{
		24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 0
	};

	xputs(canistr);
	flushmo(107);
}

/*
 * Log an error
 */
/*VARARGS1*/
zperr(s, p, u)
char *s, *p, *u;
{
	if (Verbose <= 0)
		return;
	fprintf(stderr, "Retry %d: ", errors);
	fprintf(stderr, s, p, u);
	fprintf(stderr, "\n");
}

/*
 * substr(string, token) searches for token in string s
 * returns pointer to token within string if found, NULL otherwise
 */
char *
substr(s, t)
register char *s, *t;
{
	register char *ss, *tt;

	/* search for first char of token */
	for (ss = s; *s; s++)
		if (*s == *t)
			/* compare token with substring */
			for (ss = s, tt = t;;)
			{
				if (*tt == 0)
					return s;
				if (*ss++ != *tt++)
					break;
			}
	return NULL;
}

char *babble[] =
{
	"Send file(s) with ZMODEM/YMODEM/XMODEM Protocol",
	"	(Y) = Option applies to YMODEM only",
	"	(Z) = Option applies to ZMODEM only",
	"Usage:	sz [-12+abdefkLlNnquvwYy] [-] file ...",
	"	sz [-12Ceqv] -c COMMAND",
	"	sb [-12adfkquv] [-] file ...",
	"	sx [-12akquv] [-] file",
	"	1 Use stdout for modem input",
#ifdef CSTOPB
	"	2 Use 2 stop bits",
#endif
	"	+ Append to existing destination file (Z)",
	"	a (ASCII) change NL to CR/LF",
	"	b Binary file transfer override",
	"	c send COMMAND (Z)",
	"	d Change '.' to '/' in pathnames (Y/Z)",
	"	e Escape all control characters (Z)",
	"	f send Full pathname (Y/Z)",
	"	i send COMMAND, ack Immediately (Z)",
	"	k Send 1024 byte packets (Y)",
	"	L N Limit subpacket length to N bytes (Z)",
	"	l N Limit frame length to N bytes (l>=L) (Z)",
	"	n send file if source newer (Z)",
	"	N send file if source newer or longer (Z)",
	"	o Use 16 bit CRC instead of 32 bit CRC (Z)",
	"	p Protect existing destination file (Z)",
	"	r Resume/Recover interrupted file transfer (Z)",
	"	q Quiet (no progress reports)",
	"	u Unlink file after transmission",
	"	v Verbose - provide debugging information",
	"	w N Window is N bytes (Z)",
	"	Y Yes, overwrite existing file, skip if not present at rx (Z)",
	"	y Yes, overwrite existing file (Z)",
	"- as pathname sends standard input as sPID.sz or environment ONAME",
	""
};

usage()
{
	char **pp;

	for (pp = babble; **pp; ++pp)
		fprintf(stderr, "%s\n", *pp);
	fprintf(stderr, "%s for %s by Chuck Forsberg, Omen Technology INC\n",
		VERSION, OS);
	fprintf(stderr, "\t\t\042The High Reliability Software\042\n");
	cucheck();
	exit(1);
}

/*
 * Get the receiver's init parameters
 */
getzrxinit()
{
	register n;
	struct stat f;

	for (n = 10; --n >= 0;)
	{

		switch (zgethdr(Rxhdr, 1))
		{
			case ZCHALLENGE:/* Echo receiver's challenge numbr */
				stohdr(Rxpos);
				zshhdr(ZACK, Txhdr);
				continue;
			case ZCOMMAND:	 /* They didn't see out ZRQINIT */
				stohdr(0L);
				zshhdr(ZRQINIT, Txhdr);
				continue;
			case ZRINIT:
				Rxflags = 0377 & Rxhdr[ZF0];
				Txfcs32 = (Wantfcs32 && (Rxflags & CANFC32));
				Zctlesc |= Rxflags & TESCCTL;
				Rxbuflen = (0377 & Rxhdr[ZP0]) + ((0377 & Rxhdr[ZP1]) << 8);
				if (!(Rxflags & CANFDX))
					Txwindow = 0;
				vfile("Rxbuflen=%d Tframlen=%d", Rxbuflen, Tframlen);
				if (!Fromcu)
					signal(SIGINT, SIG_IGN);
#ifdef MODE2OK
				mode(2);	 /* Set cbreak, XON/XOFF, etc. */
#endif
#ifndef READCHECK
#ifndef USG
				/* Use 1024 byte frames if no sample/interrupt */
				if (Rxbuflen < 32 || Rxbuflen > 1024)
				{
					Rxbuflen = 1024;
					vfile("Rxbuflen=%d", Rxbuflen);
				}
#endif
#endif
				/* Override to force shorter frame length */
				if (Rxbuflen && (Rxbuflen > Tframlen) && (Tframlen >= 32))
					Rxbuflen = Tframlen;
				if (!Rxbuflen && (Tframlen >= 32) && (Tframlen <= 1024))
					Rxbuflen = Tframlen;
				vfile("Rxbuflen=%d", Rxbuflen);

				/* If using a pipe for testing set lower buf len */
				fstat(iofd, &f);
				if ((f.st_mode & S_IFMT) != S_IFCHR
					&& (Rxbuflen == 0 || Rxbuflen > 4096))
					Rxbuflen = 4096;

				/*
				 * If input is not a regular file, force ACK's each 1024
				 * (A smarter strategey could be used here ...)
				 */
				if (!Command)
				{
					fstat(fileno(in), &f);
					if (((f.st_mode & S_IFMT) != S_IFREG)
						&& (Rxbuflen == 0 || Rxbuflen > 1024))
						Rxbuflen = 1024;
				}

				if (Baudrate > 300)	/* Set initial subpacket len */
					blklen = 256;
				if (Baudrate > 1200)
					blklen = 512;
#ifdef original_code
				if (Baudrate > 2400)
#else
				if (Baudrate >= 2400)
#endif
					blklen = 1024;
				if (Rxbuflen && blklen > Rxbuflen)
					blklen = Rxbuflen;
				if (blkopt && blklen > blkopt)
					blklen = blkopt;
				vfile("Rxbuflen=%d blklen=%d", Rxbuflen, blklen);
				vfile("Txwindow = %u Txwspac = %d", Txwindow, Txwspac);
				blklen_original = blklen;
				return (sendzsinit());

			case ZCAN:
			case TIMEOUT:
				return ERROR;

			case ZRQINIT:
				if (Rxhdr[ZF0] == ZCOMMAND)
					continue;
			default:
				zshhdr(ZNAK, Txhdr);
				continue;
		}
	}
	return ERROR;
}

/* Send send-init information */
sendzsinit()
{
	register c;

	if (Myattn[0] == '\0' && (!Zctlesc || (Rxflags & TESCCTL)))
		return OK;
	errors = 0;
	for (;;)
	{
		stohdr(0L);
		if (Zctlesc)
		{
			Txhdr[ZF0] |= TESCCTL;
			zshhdr(ZSINIT, Txhdr);
		}
		else
			zsbhdr(ZSINIT, Txhdr);
		zsdata(Myattn, 1 + strlen(Myattn), ZCRCW);
		c = zgethdr(Rxhdr, 1);
		switch (c)
		{
			case ZCAN:
				return ERROR;
			case ZACK:
				return OK;
			default:
				if (++errors > 19)
					return ERROR;
				continue;
		}
	}
}

/* Send file name and related info */
zsendfile(buf, blen)
char *buf;
{
	register c;

	for (;;)
	{
		Txhdr[ZF0] = Lzconv; /* file conversion request */
		Txhdr[ZF1] = Lzmanag;/* file management request */
		if (Lskipnocor)
			Txhdr[ZF1] |= ZMSKNOLOC;
		Txhdr[ZF2] = Lztrans;/* file transport request */
		Txhdr[ZF3] = 0;
		blklen = blklen_original;
		zsbhdr(ZFILE, Txhdr);
		zsdata(buf, blen, ZCRCW);
	  again:
		c = zgethdr(Rxhdr, 1);
		switch (c)
		{
			case ZRINIT:
				while ((c = readline(50)) > 0)
					if (c == ZPAD)
					{
						goto again;
					}
				/* **** FALL THRU TO **** */
			default:
				continue;
			case ZCAN:
			case TIMEOUT:
			case ZABORT:
			case ZFIN:
				return ERROR;
			case ZSKIP:
				fclose(in);
				return c;
			case ZRPOS:

				/*
				 * Suppress zcrcw request otherwise triggered by
				 * lastyunc==bytcnt
				 */
				Lastsync = (bytcnt = Txpos = Rxpos) - 1;
				fseek(in, Rxpos, 0);
				Dontread = FALSE;
				return zsendfdata();
		}
	}
}

/* Send the data in the file */
zsendfdata()
{
	register c, e, n;
	register newcnt;
	register long tcount = 0;
	int junkcount;			 /* Counts garbage chars received by TX */
	static int tleft = 6;	 /* Counter for test mode */

	Lrxpos = 0;
	junkcount = 0;
	Beenhereb4 = FALSE;
  somemore:
	if (setjmp(intrjmp))
	{
	  waitack:
		junkcount = 0;
		c = getinsync(0);
	  gotack:
		switch (c)
		{
			default:
			case ZCAN:
				fclose(in);
				return ERROR;
			case ZSKIP:
				fclose(in);
				return c;
			case ZACK:
			case ZRPOS:
				break;
			case ZRINIT:
				return OK;
		}
#ifdef READCHECK

		/*
		 * If the reverse channel can be tested for data, this logic may
		 * be used to detect error packets sent by the receiver, in place
		 * of setjmp/longjmp Rdchk(fdes) returns non 0 if a character is
		 * available
		 */
		while (Rdchk(iofd))
		{
#ifdef SV
			switch (checked)
#else
			switch (readline(1))
#endif
			{
				case CAN:
				case ZPAD:
					c = getinsync(1);
					goto gotack;
				case XOFF:	 /* Wait a while for an XON */
				case XOFF | 0200:
					readline(100);
			}
		}
#endif
	}

	if (!Fromcu)
		signal(SIGINT, onintr);
	newcnt = Rxbuflen;
	Txwcnt = 0;
	stohdr(Txpos);
	zsbhdr(ZDATA, Txhdr);

	/*
	 * Special testing mode.  This should force receiver to Attn,ZRPOS
	 * many times.  Each time the signal should be caught, causing the
	 * file to be started over from the beginning.
	 */
	if (Test)
	{
		if (--tleft)
			while (tcount < 20000)
			{
				xputs(qbf);
				flushmo(108);
				tcount += strlen(qbf);
#ifdef READCHECK
				while (Rdchk(iofd))
				{
#ifdef SV
					switch (checked)
#else
					switch (readline(1))
#endif
					{
						case CAN:
						case ZPAD:
#ifdef TCFLSH
							ioctl(iofd, TCFLSH, 1);
#endif
							goto waitack;
						case XOFF:	/* Wait for XON */
						case XOFF | 0200:
							readline(100);
					}
				}
#endif
			}
		signal(SIGINT, SIG_IGN);
		canit();
		sleep(3);
		purgeline();
		mode(0);
		sprintf(s128, "\nsz: Tcount = %ld\n", tcount);
		xputs(s128);
		if (tleft)
		{
			printf("ERROR: Interrupts Not Caught\n");
			exit(1);
		}
		exit(0);
	}

	do
	{
		if (Dontread)
		{
			n = Lastn;
		}
		else
		{
			n = zfilbuf(txbuf, blklen);
			Lastread = Txpos;
			Lastn = n;
		}
		Dontread = FALSE;
		if (n < blklen)
			e = ZCRCE;
		else if (junkcount > 3)
		{
			if (Verbose > 3)
				fprintf(stderr, "\njunkcount=%d\n", junkcount);
			e = ZCRCW;
		}
		else if (bytcnt == Lastsync)
			e = ZCRCW;
		else if (Rxbuflen && (newcnt -= n) <= 0)
			e = ZCRCW;
		else if (Txwindow && (Txwcnt += n) >= Txwspac)
		{
			Txwcnt = 0;
			e = ZCRCQ;
		}
		else
			e = ZCRCG;
		if (Verbose > 1)
			fprintf(stderr, "\r%7ld ZMODEM%s    ",
				Txpos, Crc32t ? " CRC-32" : "");
		zsdata(txbuf, n, e);
		bytcnt = Txpos += n;
		if (e == ZCRCW)
			goto waitack;
#ifdef READCHECK

		/*
		 * If the reverse channel can be tested for data, this logic may
		 * be used to detect error packets sent by the receiver, in place
		 * of setjmp/longjmp Rdchk(fdes) returns non 0 if a character is
		 * available
		 */
		flushmo(109);
		while (Rdchk(iofd))
		{
#ifdef SV
			switch (checked)
#else
			switch (readline(1))
#endif
			{
				case CAN:
				case ZPAD:
					c = getinsync(1);
					if (c == ZACK)
						break;
#ifdef TCFLSH
					ioctl(iofd, TCFLSH, 1);
#endif
					/* zcrce - dinna wanna starta ping-pong game */
					zsdata(txbuf, 0, ZCRCE);
					goto gotack;
				case XOFF:	 /* Wait a while for an XON */
				case XOFF | 0200:
					readline(100);
					break;
				default:
					++junkcount;
			}
		}
#endif /* READCHECK */
		if (Txwindow)
		{
			while ((tcount = Txpos - Lrxpos) >= Txwindow)
			{
				vfile("%ld window >= %u", tcount, Txwindow);
				if (e != ZCRCQ)
					zsdata(txbuf, 0, e = ZCRCQ);
				c = getinsync(1);
				if (c != ZACK)
				{
#ifdef TCFLSH
					ioctl(iofd, TCFLSH, 1);
#endif
					zsdata(txbuf, 0, ZCRCE);
					goto gotack;
				}
			}
			vfile("window = %ld", tcount);
		}
	}
	while (n == blklen);
	if (!Fromcu)
		signal(SIGINT, SIG_IGN);

	for (;;)
	{
		stohdr(Txpos);
		zsbhdr(ZEOF, Txhdr);
		switch (getinsync(0))
		{
			case ZACK:
				continue;
			case ZRPOS:
				goto somemore;
			case ZRINIT:
				return OK;
			case ZSKIP:
				fclose(in);
				return c;
			default:
				fclose(in);
				return ERROR;
		}
	}
}

/*
 * Respond to receiver's complaint, get back in sync with receiver
 */
getinsync(flag)
{
	register c;

	for (;;)
	{
		if (Test)
		{
			printf("\r\n\n\n***** Signal Caught *****\r\n");
			Rxpos = 0;
			c = ZRPOS;
		}
		else
			c = zgethdr(Rxhdr, 0);
		switch (c)
		{
			case ZCAN:
			case ZABORT:
			case ZFIN:
			case TIMEOUT:
				return ERROR;
			case ZRPOS:
				/* ************************************* */
				/* If sending to a modem beuufer, you   */
				/* might send a break at this point to */
				/* dump the modem's buffer.		 */
				if (Lastn >= 0 && Lastread == Rxpos)
				{
					Dontread = TRUE;
				}
				else
				{
					clearerr(in);	/* In case file EOF seen */
					fseek(in, Rxpos, 0);
				}
				bytcnt = Lrxpos = Txpos = Rxpos;
				if (Lastsync == Rxpos)
				{
					if (++Beenhereb4 > 4)
						if (blklen > 256)
							blklen /= 2;
				}
				Lastsync = Rxpos;
				return c;
			case ZACK:
				Lrxpos = Rxpos;
				if (flag || Txpos == Rxpos)
				{
					if (blklen < blklen_original)
						blklen *= 2;
					return ZACK;
				}
				continue;
			case ZRINIT:
			case ZSKIP:
				fclose(in);
				return c;
			case ERROR:
			default:
				zsbhdr(ZNAK, Txhdr);
				continue;
		}
	}
}

/* Say "bibi" to the receiver, try to do it cleanly */
saybibi()
{
	for (;;)
	{
		stohdr(0L);			 /* CAF Was zsbhdr - minor change */
		zshhdr(ZFIN, Txhdr); /* to make debugging easier */
		switch (zgethdr(Rxhdr, 0))
		{
			case ZFIN:
				sendline('O');
				sendline('O');
				flushmo(110);
			case ZCAN:
			case TIMEOUT:
				return;
		}
	}
}

/* Local screen character display function */
bttyout(c)
{
	if (Verbose)
		putc(c, stderr);
}

/* Send command and related info */
zsendcmd(buf, blen)
char *buf;
{
	register c;
	long cmdnum;

	cmdnum = getpid();
	errors = 0;
	for (;;)
	{
		stohdr(cmdnum);
		Txhdr[ZF0] = Cmdack1;
		zsbhdr(ZCOMMAND, Txhdr);
		zsdata(buf, blen, ZCRCW);
	  listen:
		Rxtimeout = 100;	 /* Ten second wait for resp. */
		c = zgethdr(Rxhdr, 1);

		switch (c)
		{
			case ZRINIT:
				goto listen; /* CAF 8-21-87 */
			case ERROR:
			case TIMEOUT:
				if (++errors > Cmdtries)
					return ERROR;
				continue;
			case ZCAN:
			case ZABORT:
			case ZFIN:
			case ZSKIP:
			case ZRPOS:
				return ERROR;
			default:
				if (++errors > 20)
					return ERROR;
				continue;
			case ZCOMPL:
				Exitcode = Rxpos;
				saybibi();
				return OK;
			case ZRQINIT:
				vfile("******** RZ *******");
				system("rz");
				vfile("******** SZ *******");
				goto listen;
		}
	}
}

/*
 * If called as sb use YMODEM protocol
 */
chkinvok(s)
char *s;
{
	register char *p;

	p = s;
	while (*p == '-')
		s = ++p;
	while (*p)
		if (*p++ == '/')
			s = p;
	if (*s == 'v')
	{
		Verbose = 1;
		++s;
	}
	Progname = s;
	if (s[0] == 's' && s[1] == 'b')
	{
		Nozmodem = TRUE;
		blklen = 1024;
	}
	if (s[0] == 's' && s[1] == 'x')
	{
		Modem2 = TRUE;
	}
}
countem(argc, argv)
register char **argv;
{
	register c;
	struct stat f;

	for (Totalleft = 0, Filesleft = 0; --argc >= 0; ++argv)
	{
		f.st_size = -1;
		if (Verbose > 2)
		{
			fprintf(stderr, "\nCountem: %03d %s ", argc, *argv);
			fflush(stderr);
		}
		if (access(*argv, 04) >= 0 && stat(*argv, &f) >= 0)
		{
			c = f.st_mode & S_IFMT;
			if (c != S_IFDIR && c != S_IFBLK)
			{
				++Filesleft;
				Totalleft += f.st_size;
			}
		}
		if (Verbose > 2)
			fprintf(stderr, " %ld", (long)f.st_size);
	}
	if (Verbose > 2)
		fprintf(stderr, "\ncountem: Total %d %ld\n",
			Filesleft, Totalleft);
}

chartest(m)
{
	register n;

	mode(m);
	printf("\r\n\nCharacter Transparency Test Mode %d\r\n", m);
	printf("If Pro-YAM/ZCOMM is not displaying ^M hit ALT-V NOW.\r\n");
	printf("Hit Enter.\021");
	flushmo(111);
	readline(500);

	for (n = 0; n < 256; ++n)
	{
		if (!(n % 8))
			printf("\r\n");
		printf("%02x ", n);
		flushmo(112);
		sendline(n);
		flushmo(112);
		printf("  ");
		flushmo(112);
		if (n == 127)
		{
			printf("Hit Enter.\021");
			flushmo(112);
			readline(500);
			printf("\r\n");
			flushmo(112);
		}
	}
	printf("\021\r\nEnter Characters, echo is in hex.\r\n");
	printf("Hit SPACE or pause 40 seconds for exit.\r\n");

	while (n != TIMEOUT && n != ' ')
	{
		n = readline(400);
		printf("%02x\r\n", n);
		flushmo(112);
	}
	printf("\r\nMode %d character transparency test ends.\r\n", m);
	flushmo(112);
}

/* vi: set tabstop=4 shiftwidth=4 */
