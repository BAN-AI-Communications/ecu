/* CHK=0xC88F */
char *rev = "1.21";
/*+-------------------------------------------------------------------------
	usemorse.c

 2^1/12 = 1.059463
--------------------------------------------------------------------------*/
/*+:EDITS:*/
/*:01-24-1997-02:38-wht@yuriatin-SOURCE RELEASE 4.00 */
/*:09-13-1996-20:10-wht@n4hgf-wait 2 min for busy device */
/*:05-12-1996-22:58-wht@n4hgf-add freq and tick mod during execution */
/*:07-05-1995-19:04-wht@n4hgf-make sure /dev/morse is a char special */
/*:05-31-1993-13:51-wht@n4hgf-case 1 now 1-5-1 descending */
/*:05-31-1993-13:24-wht@n4hgf-add banner case 0: no banner */
/*:04-13-1992-20:18-wht@n4hgf-add non-conversational features */
/*:11-08-1990-16:17-wht@n4hgf-clean up */
/*:11-06-1988-21:17-wht-creation */

#include <stdio.h>
#include <ctype.h>
#include <math.h>

#include <signal.h>
#include <sys/types.h>
#include <sys/param.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <termio.h>

#include "morse_dvr.h"

#if !defined(UINT)
#define uchar	unsigned char
#define UINT	unsigned int
#endif

int current_ttymode = 0;

struct termio ttyt0;
struct termio ttyt;

char kbdeof;				 /* current input EOF */
char kbdeol2;				 /* current secondary input EOL */
char kbdeol;				 /* current input EOL */
char kbderase;				 /* current input ERASE */
char kbdintr;				 /* current input INTR */
char kbdkill;				 /* current input KILL */
char kbdquit;				 /* current input QUIT */
int echo_erase_char;		 /* save users ECHOE bit */
int echo_kill_char;			 /* save users ECHOK bit */

#define TWELVETH_ROOT 1.059463

/*+-------------------------------------------------------------------------
	ttyinit()
--------------------------------------------------------------------------*/
void
ttyinit()
{
	ioctl(1, (int)TCGETA, (char *)&ttyt0);	/* save initial tty state */

	kbdintr = (ttyt0.c_cc[VINTR]) ? (ttyt0.c_cc[VINTR] & 0x7F) : '\377';
	kbdquit = (ttyt0.c_cc[VQUIT]) ? (ttyt0.c_cc[VQUIT] & 0x7F) : '\377';
	kbderase = (ttyt0.c_cc[VERASE]) ? (ttyt0.c_cc[VERASE] & 0x7F) : '\377';
	kbdkill = (ttyt0.c_cc[VKILL]) ? (ttyt0.c_cc[VKILL] & 0x7F) : '\377';
	kbdeof = (ttyt0.c_cc[VEOF]) ? (ttyt0.c_cc[VEOF] & 0x7F) : '\04';
	kbdeol2 = (ttyt0.c_cc[VEOL]) ? (ttyt0.c_cc[VEOL] & 0x7F) : '\377';
	kbdeol = (ttyt0.c_iflag & ICRNL) ? '\r' : '\n';

	echo_erase_char = ttyt0.c_lflag & ECHOE;
	echo_kill_char = ttyt0.c_lflag & ECHOK;

}							 /* end of ttyinit */

/*+-----------------------------------------------------------------------
	ttymode(arg) -- control user console (kbd/screen)

  Where arg ==
	0 restore attributes saved at start of execution
	1 raw mode (send xon/xoff, but do not respond to it, no ISIG/SIGINT)
	2 raw mode (same as 1 but allow keyboard interrupts)

------------------------------------------------------------------------*/
void
ttymode(arg)
{
	register char *mode_type;

	if (arg == 0)
	{
		ioctl(1, (int)TCSETAW, (char *)&ttyt0);
		current_ttymode = 0;
	}
	else
	{
		ioctl(1, (int)TCGETA, (char *)&ttyt);
		/* don't want to honor tty xon/xoff, but pass to other end */
		ttyt.c_iflag &= ~(INLCR | ICRNL | IGNCR | IXON | IUCLC);
		ttyt.c_iflag |= ISTRIP;
		ttyt.c_iflag |= IXOFF;	/* this end will xon/xoff if necessary */

		ttyt.c_lflag &= ~(ICANON | ISIG | ECHO);

		ttyt.c_cc[VMIN] = 1;
		ttyt.c_cc[VTIME] = 0;

		ioctl(1, (int)TCSETAW, (char *)&ttyt);
		current_ttymode = arg;
	}
}							 /* end of ttymode */

/*+-------------------------------------------------------------------------
	ttygetc(fd) -- get a key from the keyboard
--------------------------------------------------------------------------*/
UINT
ttygetc(fd)
int fd;
{
	uchar ctmp;

	if (read(fd, &ctmp, 1) <= 0)
		return (kbdeof);
	ctmp &= 0x7F;
	return ((int)ctmp);
}							 /* end if ttygetc */

/*+-----------------------------------------------------------------------
	ttygets(str,maxsize)
------------------------------------------------------------------------*/
void
ttygets(str, maxsize)
char *str;
int maxsize;
{
	int inch;
	char ch;
	int strcount = 0;
	char *bs_str = "\010 \010";

	--maxsize;				 /* decrement for safety */
	*str = 0;

	while (1)
	{
		inch = ttygetc(1);
		if (inch == 0x1B)
		{
			write(1, "<ESC>\n", 6);
			str[0] = 0;
			return;
		}
		else if (inch == kbdkill)
		{
			while (strcount)
			{
				write(1, bs_str, strlen(bs_str));
				strcount--;
			}
			*str = 0;
			continue;
		}
		else if (inch == kbderase)
		{
			if (strcount)
			{
				write(1, bs_str, strlen(bs_str));
				strcount--;
			}
			str[strcount] = 0;
			continue;
		}

		switch (inch)
		{
			case 0x0D:
				str[strcount] = 0;
				write(1, "\n", 1);
				return;

			default:
				if (inch > 0xFF || !isprint(inch))
				{
					write(1, "\007", 1);
					break;
				}
				if (strcount == maxsize)
				{
					write(1, "\007", 1);
					continue;
				}
				str[strcount++] = inch;
				ch = (char)inch;
				write(1, &ch, 1);
				str[strcount] = 0;
		}
	}

}							 /* end of ttygets() */

/*+-------------------------------------------------------------------------
	main(argc,argv,envp)
--------------------------------------------------------------------------*/
main(argc, argv, envp)
int argc;
char **argv;
char **envp;
{
	int fdm = -1;
	char ch;
	char nl = 0x0A;
	int i;
	int itmp;
	int errflg = 0;
	int freq = 600;
	int f2, f3, t2;
	int ticks;
	int hertz;
	int banner = 0;
	FILE *fp = 0;
	char *cptr;
	char s32[32];
	char *getenv();
	char *devnm = "/dev/morse";
	struct stat st;
	extern char *optarg;
	extern int optind;

	if (!(cptr = getenv("HZ")))
		hertz = HZ;
	else
		hertz = atoi(cptr);

	ticks = hertz / 13;

	while ((itmp = getopt(argc, argv, "b:f:t:")) != -1)
	{
		switch (itmp)
		{
			case 'b':
				banner = atoi(optarg);
				break;
			case 'f':
				if (((freq = atoi(optarg)) <= 0) || (freq > 10000))
					freq = 800;
				break;
			case 't':
				if (((ticks = atoi(optarg)) <= 0) || (ticks > 15))
					ticks = 4;
				break;
			case '?':
				errflg++;
		}
	}
	if (errflg)
	{
		(void)fprintf(stderr,
			"usage: usemorse [-b banner#] [-f freq] [-t ticks] [text]\n");
		exit(1);
	}

	if (stat(devnm, &st))
	{
		perror(devnm);
		exit(255);
	}
	if ((st.st_mode & S_IFMT) != S_IFCHR)
	{
		fprintf(stderr, "%s is not a special character driver\n", devnm);
		exit(255);
	}
	itmp = 120;
	while (itmp--)
	{
		if ((fdm = open(devnm, O_WRONLY, 0)) >= 0)
			break;
		if (errno != EACCES) /* oif error other than "in use" */
		{
			perror("usemorse: morse driver said");
			exit(1);
		}
		nap(1000);
	}

	if (fdm < 0)
	{
		fprintf(stderr, "usemorse: after 5 retries, morse driver still busy\n");
		exit(2);
	}

	if (ioctl(fdm, MORSE_ENABLE, &itmp))
	{
		fprintf(stderr,
			"I think you have an old version of the driver installed.\n");
	}

	/*
	 * banner/warning
	 */
	switch (banner)
	{
		case 0:			 /* no banner */
			break;

		case 1:
			t2 = 1;
			ioctl(fdm, MORSE_SET_SPEED, &t2);

			f2 = 1600;
			ioctl(fdm, MORSE_SET_FREQUENCY, &f2);
			write(fdm, "xx ", 3);

			f3 = (int)((float)f2 / 1.33484);	/* a fifth below */
			ioctl(fdm, MORSE_SET_FREQUENCY, &f3);
			write(fdm, "xx  ", 4);

			f3 = (int)((float)f2 / 2.00000);	/* an octave below */
			ioctl(fdm, MORSE_SET_FREQUENCY, &f3);
			write(fdm, "xx  ", 4);
			break;

		case 2:
			t2 = 4;
			ioctl(fdm, MORSE_SET_SPEED, &t2);
			f2 = freq;
			ioctl(fdm, MORSE_SET_FREQUENCY, &f2);
			write(fdm, "uuu", 3);
			break;

		case 3:
			for (itmp = 0; itmp < 3; itmp++)
			{
				f2 = freq;
				ioctl(fdm, MORSE_SET_FREQUENCY, &f2);
				write(fdm, "5", 1);

				f2 = (int)((double)freq * pow(TWELVETH_ROOT, 7));	/* up 1/5 */
				ioctl(fdm, MORSE_SET_FREQUENCY, &f2);
				write(fdm, "0", 1);

				f2 = freq;
				ioctl(fdm, MORSE_SET_FREQUENCY, &f2);
				write(fdm, "5", 1);

				f2 = (int)((double)freq * pow(TWELVETH_ROOT, 5));	/* up 1/4 */
				ioctl(fdm, MORSE_SET_FREQUENCY, &f2);
				write(fdm, "0", 1);
			}
			break;

		default:
			t2 = 8;
			ioctl(fdm, MORSE_SET_SPEED, &t2);
			f2 = freq;
			ioctl(fdm, MORSE_SET_FREQUENCY, &f2);
			write(fdm, "tt", 2);
			break;
	}

	if (banner)
		nap(250L);

	ioctl(fdm, MORSE_SET_SPEED, &ticks);
	ioctl(fdm, MORSE_SET_FREQUENCY, &freq);

	if (optind != argc)
	{
		for (itmp = optind; itmp < argc; itmp++)
		{
			write(fdm, argv[itmp], strlen(argv[itmp]));
			write(fdm, " ", 1);
		}
		exit(0);
	}

	setbuf(stdout, NULL);
	for (ch = 1; ch < NSIG; ch++)
		signal((int)ch, SIG_IGN);

	ttyinit();
	ttymode(1);
	printf("[ usemorse rev %s ]\n",rev);
	printf("type characters to send (^D to exit, ^F freq chg, ^T tick chg)\n");
	while ((ch = ttygetc(0)) != kbdeof)
	{
		if (!isatty(0) && Rdchk(1))
		{
			write(1,"\n",1);
			ttygetc(1);
			ttymode(0);
			exit(0);
		}

		if (((ch >= 0x20) && (ch < '~')) || (ch == 0x0D))
			write(1, &ch, 1);
		switch (ch)
		{
			case 0x0D:
				write(1, &nl, 1);
				break;

			case 'T' & 0x1F:
				fprintf(stderr, "\nticks(%d)=", ticks);
				ttygets(s32, 32);
				if ((itmp = atoi(s32)) > 0)
				{
					i = itmp;
					itmp = ioctl(fdm, MORSE_SET_SPEED, &i);
					fprintf(stderr, "setting ticks to %d %s\n", i,
						(itmp) ? "FAILED" : "succeeded");
					if (!itmp)
						ticks = i;
				}
				continue;

			case 'F' & 0x1F:
				fprintf(stderr, "\nfreq(%d)=", freq);
				ttygets(s32, 32);
				if ((itmp = atoi(s32)) > 0)
				{
					i = itmp;
					itmp = ioctl(fdm, MORSE_SET_FREQUENCY, &i);
					fprintf(stderr, "setting freq to %d %s\n", i,
						(itmp) ? "FAILED" : "succeeded");
					if (!itmp)
						freq = i;
				}
				continue;
		}

		write(fdm, &ch, 1);
	}
	write(1,"\n",1);
	ttymode(0);
	exit(0);
}							 /* end of main */

/* end of usemorse.c */
/* vi: set tabstop=4 shiftwidth=4: */
