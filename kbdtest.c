/*+-----------------------------------------------------------------------
	kbdtest.c -- hack to test keyboard function key sequences
    wht@wht.net

  compile with     cc -o kbdtest kbdtest.c
  or just          cc kbdtest.c;a.out

------------------------------------------------------------------------*/
/*+:EDITS:*/
/*:04-26-2000-11:16-wht@bob-RELEASE 4.42 */
/*:01-24-1997-02:37-wht@yuriatin-SOURCE RELEASE 4.00 */
/*:09-11-1996-20:00-wht@yuriatin-3.48-major telnet,curses,structural overhaul */
/*:11-23-1995-11:20-wht@kepler-source control 3.37 for tsx-11 */
/*:11-14-1995-10:23-wht@kepler-3.37.80-source control point: SOCKETS */
/*:10-21-1995-12:54-wht@wwtp1-wild edit broke this HOW long ago? */
/*:05-04-1994-04:39-wht@n4hgf-ECU release 3.30 */
/*:09-10-1992-13:59-wht@n4hgf-ECU release 3.20 */
/*:08-22-1992-15:39-wht@n4hgf-ECU release 3.20 BETA */
/*:07-25-1991-12:58-wht@n4hgf-ECU release 3.10 */
/*:12-21-1990-23:47-wht@n4hgf-liven up for release with ECU 3 */
/*:04-07-1990-01:36-wht@tridom-bring out of the daaaaark ages a bit */
/*:04-18-1988-13:44-wht-first edits -- oollldd program */

#include <stdio.h>
#include <signal.h>
#include <ctype.h>
#include <fcntl.h>
#include <termio.h>
#include "ecu_types.h"
#include <sys/errno.h>
#include "ecu_stat.h"
#include <string.h>

#define TTYIN   0
#define TTYOUT  1
#define TTYERR  2

struct termio tv0;			 /* for saving, changing TTY atributes */
struct termio tv;			 /* for saving, changing TTY atributes */

/*+-----------------------------------------------------------------------
	ttymode(arg) -- control user console (kbd/screen)

  Where arg ==
	0 restore attributes saved at start of execution
	1 raw mode

------------------------------------------------------------------------*/
void
ttymode(arg)
int arg;
{
	char *mode_type;

	switch (arg)
	{
		case 0:
			mode_type = "console to cooked mode\r\n";
			break;
		default:
			mode_type = "console to raw mode\r\n";
			break;
	}
	(void)fprintf(stderr, mode_type);

	if (arg)
	{
		(void)ioctl(TTYIN, TCGETA, &tv);
		tv.c_cflag &= ~(CS8 | PARENB | PARODD);
		tv.c_cflag |= CS8;
		tv.c_iflag &= ~(INLCR | ICRNL | IGNCR | IXOFF | IUCLC | ISTRIP);
		tv.c_oflag |= OPOST;
		tv.c_oflag &= ~(OLCUC | ONLCR | OCRNL | ONOCR | ONLRET);
		tv.c_lflag &= ~(ICANON | ISIG | ECHO);
		tv.c_cc[VEOF] = '\01';
		tv.c_cc[VEOL] = '\0';
		tv.c_cc[VMIN] = 1;
		tv.c_cc[VTIME] = 1;
		(void)ioctl(TTYIN, TCSETAW, &tv);
	}
	else
		(void)ioctl(TTYIN, TCSETAW, &tv0);
}

/*+-----------------------------------------------------------------------
	main()
------------------------------------------------------------------------*/
main(argc, argv)
int argc;
char **argv;
{
	unsigned char inchar;

	setbuf(stdout, NULL);
	setbuf(stderr, NULL);

	ioctl(TTYIN, TCGETA, &tv0);	/* get original status */
	ttymode(2);

	fprintf(stderr, "press ^D (0x04) to terminate program\r\n");

	while (read(TTYIN, &inchar, 1) == 1)
	{
		printf("%02x %c\r\n", inchar,
			((inchar >= 0x20) && (inchar < 0x7F)) ? inchar : '.');
		if ((inchar & 0x7F) == 4)
		{
			ttymode(0);
			exit(0);
		}
	}
	ttymode(0);
	exit(1);

}

/* vi: set tabstop=4 shiftwidth=4: */
/* end of kbdtest.c */
