/*+-------------------------------------------------------------------------
	zcommon.c -  ecurz/ecusz common code
	derived from public domain code by Chuck Forsberg
	ecu adaptation wht@wht.net

  Defined functions:
	ck_sigint()
	get_curr_dir(currdir, currdir_max)
	get_home_dir(home_dir)
	mode(new_mode)
	pputc()
	pputs()
	Rdchk(fd)
	sendbrk()
	strerror(err_no)
	zmputs(str)

--------------------------------------------------------------------------*/
/*+:EDITS:*/
/*:12-15-1997-19:29-wht@fep-move search for FIONREAD to ecu_config.h */
/*:01-24-1997-02:38-wht@yuriatin-SOURCE RELEASE 4.00 */
/*:09-19-1996-01:58-wht@yuriatin-add strerror */
/*:09-11-1996-20:01-wht@yuriatin-3.48-major telnet,curses,structural overhaul */
/*:01-26-1996-05:08-wht@kepler-always READCHECK */
/*:11-27-1995-12:12-wht@kepler-detection of telnet in mode() needed return */
/*:11-14-1995-10:23-wht@kepler-3.37.80-source control point: SOCKETS */
/*:10-19-1995-05:15-wht@kepler-dont endwin-LINUX burps */
/*:10-15-1995-15:28-wht@jonah-undef doubly defed sun defs */
/*:10-14-1995-23:52-wht@kepler-Bitrate now set by -@ cmd switch */
/*:10-14-1995-17:08-wht@kepler-'struct termio' to 'struct TERMIO' */
/*:03-12-1995-02:48-wht@kepler-remove superflous getpwuid decl */
/*:01-12-1995-15:20-wht@n4hgf-apply Andrew Chernov 8-bit clean+FreeBSD patch */
/*:05-04-1994-04:40-wht@n4hgf-ECU release 3.30 */
/*:11-23-1993-10:48-wht@n4hgf-hpux */
/*:02-16-1993-13:14-wht@n4hgf-make Bitrate long for 286 */
/*:09-10-1992-14:00-wht@n4hgf-ECU release 3.20 */
/*:08-22-1992-15:39-wht@n4hgf-ECU release 3.20 BETA */
/*:07-17-1992-18:28-wht@n4hgf-remove Nap() and use common ../nap.o */
/*:05-11-1992-16:43-wht@gyro-fix WORKING_SELECT nap once and for all */
/*:05-08-1992-02:42-wht@n4hgf-select-based Nap was buggy */
/*:08-30-1991-20:09-wht@n4hgf2-sun Nap was not returning a value */
/*:08-30-1991-02:34-jdeitch@jadpc.cts.com-fix no hzmsec */
/*:08-21-1991-06:23-wht@n4hgf-sun porting */
/*:07-25-1991-12:59-wht@n4hgf-ECU release 3.10 */
/*:08-14-1990-20:40-wht@n4hgf-ecu3.00-flush old edit history */

#include <stdio.h>
#include <errno.h>
#include <signal.h>
#include <setjmp.h>
#include <ctype.h>
#include "../ecu_config.h"
#include "../ecu_types.h"
#include <sys/param.h>
#include "../ecu_pwd.h"
#include "../ecu_time.h"
#include "zmodem.h"

#ifdef __FreeBSD__
#include <locale.h>
#endif

extern unsigned char vmin_count;
extern int Zmodem;
extern unsigned long Bitrate;
extern int Twostop;			 /* Use two stop bits */

#if defined(LLITOUT)
long Locmode;				 /* Saved "local mode" for 4.x BSD "new
							  * driver" */
long Locbit = LLITOUT;		 /* Bit SUPPOSED to disable output
							  * translations */
#endif

int sigint = 0;				 /* dummy for nap.c */
struct TERMIO oldtty, tty;
extern int iofd;			 /* File descriptor for ioctls & reads */

/* crctab calculated by Mark G. Mendel,Network Systems Corporation */
unsigned short crctab[256] =
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

#if defined(WANT_UPDCRC_HERE)/* wht -- moved to zmodem.h */
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

#define updcrc(cp,crc) ( crctab[((crc >> 8) & 255)] ^ (crc << 8) ^ cp)
#endif

/*
  First,the polynomial itself and its table of feedback terms.  The
  polynomial is:

  X^32+X^26+X^23+X^22+X^16+X^12+X^11+X^10+X^8+X^7+X^5+X^4+X^2+X^1+X^0

  Note that we take it "backwards" and put the highest-order term in the
  lowest-order bit.  The X^32 term is "implied"; the LSB is the X^31
  term,etc.  The X^0 term (usually shown as "+1") results in the MSB being
  1.  Note that the usual hardware shift register implementation,which is
  what we're using (we're merely optimizing it by doing eight-bit chunks at
  a time) shifts bits into the lowest-order term.  In our
  implementation,that means shifting towards the right.  Why do we do it
  this way?  Because the calculated CRC must be transmitted in order from
  highest-order term to lowest-order term.  UARTs transmit characters in
  order from LSB to MSB.  By storing the CRC this way, we hand it to the
  UART in the order low-byte to high-byte; the UART sends each low-bit to
  high-bit; and the result is transmission bit by bit from highest- to
  lowest-order term without requiring any bit shuffling on our part.
  Reception works similarly.

  The feedback terms table consists of 256 32-bit entries.  Notes:

     The macro for using the table is UPDC32 is located in zmodem.h

     It might not be obvious,but the feedback terms simply represent the
     results of eight shift/xor opera- tions for all combinations of data
     and CRC register values.

     The values must be right-shifted by eight bits by the "UPDC32" logic;
     the shift must be unsigned (bring in zeroes).  On some hardware you
     could probably optimize the shift in assembler by using byte-swap
     instructions.
*/

unsigned long cr3tab[] =	 /* CRC polynomial 0xedb88320 */
{
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

int
ck_sigint()
{
	return (0);
}

/*+-------------------------------------------------------------------------
	mode(new_mode)

  mode:
    0: restore original tty mode
    1: save old tty stat, set raw mode
    2: set XON/XOFF for sb/sz with ZMODEM or YMODEM-g
    3: save old tty stat, set raw mode with flow control

  if telnet, ignore the mode
--------------------------------------------------------------------------*/
int
mode(new_mode)
int new_mode;
{
	static did0 = FALSE;
	extern int telnet;

	if (telnet && !did0)
	{
		report_mode(4);
		report_comm_bitrate(Bitrate);
		did0 = 1;
		return (OK);
	}

	report_mode(new_mode);
	switch (new_mode)
	{
		case 2:			 /* Un-raw mode used by sz,sb when -g detected */
			if (!did0)
				(void)ecugetattr(iofd, &oldtty);
			tty = oldtty;
			tty.c_iflag &= ~(IXON | IXOFF | IXANY);

#if defined(RTSFLOW) && defined(CTSFLOW)
			if (tty.c_cflag & (RTSFLOW | CTSFLOW))
				tty.c_iflag = BRKINT;
			else
				tty.c_iflag = BRKINT | IXON;
#else
			tty.c_iflag = BRKINT | IXON;
#endif

			tty.c_oflag = 0; /* Transparent output */

			tty.c_cflag &= ~PARENB;	/* Disable parity */
			tty.c_cflag |= CS8;	/* Set character size = 8 */
			if (Twostop)
				tty.c_cflag |= CSTOPB;	/* Set two stop bits */

			tty.c_lflag = Zmodem ? 0 : ISIG;
			tty.c_cc[VINTR] = Zmodem ? -1 : 030;	/* Interrupt char */
			tty.c_cc[VQUIT] = -1;	/* Quit char */
#if defined(NFGVMIN)
			tty.c_cc[VMIN] = 1;
#else
			tty.c_cc[VMIN] = 3;	/* This many chars satisfies reads */
#endif
			tty.c_cc[VTIME] = 1;	/* or in this many tenths of seconds */

			(void)ecusetattr(iofd, TCSETAW, &tty);
			did0 = TRUE;
			return (OK);
		case 1:
		case 3:
			if (!did0)
				(void)ecugetattr(iofd, &oldtty);
			tty = oldtty;
			tty.c_iflag &= ~(IXON | IXOFF | IXANY);

#if defined(RTSFLOW) && defined(CTSFLOW)
			tty.c_iflag = new_mode == 3 ? (IGNBRK | RTSFLOW) : IGNBRK;
#else
			tty.c_iflag = new_mode == 3 ? (IGNBRK | IXOFF) : IGNBRK;
#endif

			/* No echo,crlf mapping,INTR,QUIT,delays,no erase/kill */
			tty.c_lflag &= ~(ECHO | ICANON | ISIG);

			tty.c_oflag = 0; /* Transparent output */

			tty.c_cflag &= ~PARENB;	/* Same bit rate,disable parity */
			tty.c_cflag |= CS8;	/* Set character size = 8 */
			if (Twostop)
				tty.c_cflag |= CSTOPB;	/* Set two stop bits */
#if defined(NFGVMIN)
			tty.c_cc[VMIN] = 1;	/* This many chars satisfies reads */
#else
			tty.c_cc[VMIN] = vmin_count;	/* This many chars satisfies
											 * reads */
#endif
			tty.c_cc[VTIME] = 1;	/* or in this many tenths of seconds */
			(void)ecusetattr(iofd, TCSETAW, &tty);
			did0 = TRUE;
			report_comm_bitrate(Bitrate);
			return (OK);
		case 0:
			if (!did0)
				return (ERROR);
			ecudrain(iofd);
			ecuflush(iofd, TCIFLUSH);
			ecusetattr(iofd, TCSETAW, &oldtty);
			ecuflow(iofd, TCOON);
			return (OK);
		default:
			return (ERROR);
	}
}							 /* end of mode */

/*+-------------------------------------------------------------------------
	sendbrk()
--------------------------------------------------------------------------*/
void
sendbrk()
{
	ecubreak(iofd);
}							 /* end of sendbrk */

/*+-------------------------------------------------------------------------
	get_curr_dir(currdir,currdir_max)
--------------------------------------------------------------------------*/
void
get_curr_dir(currdir, currdir_max)
char *currdir;
int currdir_max;
{
	getcwd(currdir, currdir_max);
}							 /* end of get_curr_dir */

/*+-------------------------------------------------------------------------
	zmputs(str) - send a string to the modem

processing for \336 (sleep 1 sec) and \335 (break signal)
--------------------------------------------------------------------------*/
void
zmputs(str)
register char *str;
{
	register char strch;

	while (strch = *str++)
	{
		switch (strch)
		{
			case '\336':
				sleep(1);
				continue;
			case '\335':
				sendbrk();
				continue;
			default:
				sendline(strch);
		}
	}
}							 /* end of zmputs */

/*+-----------------------------------------------------------------------
	get_home_dir(home_dir):  leave plenty of room for result!
------------------------------------------------------------------------*/
int
get_home_dir(home_dir)
char *home_dir;
{
	char *cptr;
	static char home_directory[256] = "";
	struct passwd *pwent;
	char *getenv();

	if (home_directory[0])
	{
		strcpy(home_dir, home_directory);
		return (0);
	}

	if (cptr = getenv("HOME"))	/* x286 seems to blow up ... */
	{						 /* ... in pwent code ... hmmm */
		strcpy(home_directory, cptr);
		strcpy(home_dir, cptr);
		return (0);
	}

	if (!(pwent = getpwuid(getuid())))
	{
		perror("cannot get pwent for you!!");
		exit(1);
	}
	strcpy(home_directory, pwent->pw_dir);
	strcpy(home_dir, pwent->pw_dir);
	endpwent();
	return (0);

}							 /* end of get_home_dir */

/*+-------------------------------------------------------------------------
	Rdchk(f)
--------------------------------------------------------------------------*/
#if defined(CFG_FionreadRdchk)
int
Rdchk(fd)
int fd;
{
	int waiting = 0;

	ioctl(fd, FIONREAD, &waiting);
	return (!!waiting);

}							 /* end of Rdchk */
#endif /* CFG_FionreadRdchk */

/*+-------------------------------------------------------------------------
	strerror(err_no)
--------------------------------------------------------------------------*/
#ifndef CFG_HasStrerror
char *
strerror(err_no)
int err_no;
{
	static char errant[32];
	extern char *sys_errlist[];
	extern int sys_nerr;

	if (err_no > sys_nerr)
	{
		sprintf(errant, "Error %d", err_no);
		return (errant);
	}
	return (sys_errlist[err_no]);

}							 /* end of strerror */
#endif /* CFG_HasStrerror */

/*+-------------------------------------------------------------------------
	pputc()/pputs() - never called, resolve hex_dump
--------------------------------------------------------------------------*/
void
pputc()
{
}							 /* end of pputc */

void
pputs()
{
}							 /* end of pputs */

/* end of zcommon.c */
/* vi: set tabstop=4 shiftwidth=4: */
