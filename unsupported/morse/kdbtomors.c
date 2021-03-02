/* CHK=0xBF59 */
/*+-------------------------------------------------------------------------
	user-morse.c
--------------------------------------------------------------------------*/
/*+:EDITS:*/
/*:01-24-1997-02:38-wht@yuriatin-SOURCE RELEASE 4.00 */
/*:11-08-1990-13:52-wht@n4hgf-convert hack to decent test */
/*:11-06-1988-21:17-wht-creation */

#include <stdio.h>

#include <signal.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <fcntl.h>
#include <sys/ioctl.h>
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
	ttygetc(xkey_ok) -- get a key from the keyboard
if XENIX, map extended keys to sign-bit-set special value
if xkey_ok is 0, disallow extended keys
--------------------------------------------------------------------------*/
UINT
ttygetc(xkey_ok)
int xkey_ok;
{
	uchar ctmp;

	if (read(0, &ctmp, 1) <= 0)
		return (kbdeof);
	ctmp &= 0x7F;
	return ((int)ctmp);
}							 /* end if ttygetc */

/*+-------------------------------------------------------------------------
	main(argc,argv,envp)
--------------------------------------------------------------------------*/
main(argc, argv, envp)
int argc;
char **argv;
char **envp;
{
	int fdm = open("/dev/morse", O_WRONLY, 0);
	int ticks = 3;
	char ch;

	if (fdm < 0)
	{
		perror("/dev/morse open error");
		exit(1);
	}

	setbuf(stdout, NULL);
	setbuf(stderr, NULL);

	for (ch = 1; ch < NSIG; ch++)
		signal((int)ch, SIG_IGN);

	ttyinit();
	ttymode(1);
	while ((ch = ttygetc()) != kbdeof)
	{
		if (ch == '+')
		{
			ticks++;
			printf("\nticks = %d ioctl status = %d\n",
				ticks, ioctl(fdm, MORSE_SET_SPEED, &ticks));
		}
		else if (ch == '-')
		{
			if (ticks > 0)
				ticks--;
			printf("\nticks = %d ioctl status = %d\n",
				ticks, ioctl(fdm, MORSE_SET_SPEED, &ticks));
		}
		write(1, &ch, 1);
		if (ch == 0x0D)
		{
			char tmp = 0x0A;

			write(1, &tmp, 1);
		}
		write(fdm, &ch, 1);
	}
	ttymode(0);
	exit(0);
}							 /* end of main */

/* end of user-morse.c */
/* vi: set tabstop=4 shiftwidth=4: */
