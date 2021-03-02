#define MULTISCREEN_DUMP_BUG
/*+-------------------------------------------------------------------------
	ecuscrdump.c - screen dump
	wht@wht.net

  Defined functions:
	screen_dump(scrfile)

--------------------------------------------------------------------------*/
/*+:EDITS:*/
/*:04-26-2000-11:15-wht@bob-RELEASE 4.42 */
/*:01-24-1997-02:37-wht@yuriatin-SOURCE RELEASE 4.00 */
/*:09-11-1996-20:00-wht@yuriatin-3.48-major telnet,curses,structural overhaul */
/*:08-22-1996-14:36-wht@fep-turn off SCO raw screen dump */
/*:11-23-1995-11:20-wht@kepler-source control 3.37 for tsx-11 */
/*:11-14-1995-10:23-wht@kepler-3.37.80-source control point: SOCKETS */
/*:10-14-1995-17:08-wht@kepler-'struct termio' to 'struct TERMIO' */
/*:09-17-1995-16:28-wht@kepler-remove obsolete #if 0 code */
/*:03-12-1995-03:27-wht@kepler-use ECU_MAXPN */
/*:01-12-1995-15:19-wht@n4hgf-apply Andrew Chernov 8-bit clean+FreeBSD patch */
/*:05-04-1994-04:39-wht@n4hgf-ECU release 3.30 */
/*:11-12-1993-11:00-wht@n4hgf-Linux changes by bob@vancouver.zadall.com */
/*:09-10-1992-13:58-wht@n4hgf-ECU release 3.20 */
/*:08-22-1992-15:38-wht@n4hgf-ECU release 3.20 BETA */
/*:05-29-1992-13:28-wht@n4hgf-no banner - phone numbers are security risk */
/*:07-25-1991-12:56-wht@n4hgf-ECU release 3.10 */
/*:07-17-1991-07:04-wht@n4hgf-avoid SCO UNIX nap bug */
/*:12-21-1990-17:27-wht@n4hgf-non-ansi considerations */
/*:08-14-1990-20:40-wht@n4hgf-ecu3.00-flush old edit history */

#include "ecucurses.h"
#include "ecu.h"
#include "ecukey.h"
#include "pc_scr.h"

extern char curr_dir[ECU_MAXPN];	/* current working directory */
extern UINT tcap_LINES;
extern UINT tcap_COLS;
extern struct TERMIO tty_termio_at_entry;
extern int tty_not_char_special;
extern int tty_is_multiscreen;

char screen_dump_file_name[256];

/*+-------------------------------------------------------------------------
	screen_dump(scrfile) - dump physical display contents
unless stdin is non-multiscreen and/or /dev/null, in which case,
dump rcvr virtual screen
if scrfile == NULL, default to ~/.ecu/screen.dump
--------------------------------------------------------------------------*/
void
screen_dump(scrfile)
char *scrfile;
{
	uchar schar;
	uchar s256[256];
	uchar *cp = s256;
	uchar *sptr = (uchar *) shm->screen;
	UINT srow = 0;
	UINT scol = 0;
	FILE *fp;
	int restart_rcvr = need_rcvr_restart();
	UINT lines_left = tcap_LINES;

#if 0 && defined(M_SYSV)  || defined(SCO32v5)
	int use_ansi_MC = !(!tty_is_multiscreen || tty_not_char_special);
	struct TERMIO dump_tty_termio_at_entry;
	struct TERMIO dump_tty_termio_current;

#endif /* M_SYSV */

	kill_rcvr_process(SIGUSR1);

#if 0 && defined(M_SYSV)  || defined(SCO32v5)
	if (use_ansi_MC)
	{
		/* save keyboard termio at entry */
		ecugetattr(TTYIN, &dump_tty_termio_at_entry);

		/*
		 * set keyboard to termio status at staart of execution of program
		 * plus a few mods
		 */

		dump_tty_termio_current = tty_termio_at_entry;
		dump_tty_termio_current.c_cflag &= ~(PARENB | PARODD);
		dump_tty_termio_current.c_cflag |= CS8;
		dump_tty_termio_current.c_iflag &= ~(ISTRIP);
		dump_tty_termio_current.c_lflag &= ~(ICANON | ISIG | ECHO);
		ecusetattr(TTYIN, TCSETAW, &dump_tty_termio_current);
		ttyflush(2);
	}
#endif /* M_SYSV */

	if (scrfile)
		fp = fopen(scrfile, "a");
	else
	{
		get_home_dir(s256);
		strcat((char *)s256, "/.ecu/screen.dump");
		fp = fopen((char *)s256, "a");
	}
	if (!fp)
	{
#if defined(MORSE)
		xbell(XBELL_DONE, 1);
#else
		ring_bell();
		Nap(50L);
		ring_bell();
#endif
		return;
	}

#if 0 && defined(M_SYSV)  || defined(SCO32v5)
	if (use_ansi_MC)
		write(1, "\033[2i", 4);	/* spill your guts, screen */
#endif /* M_SYSV */

	while (1)
	{
#if 0 && defined(M_SYSV)  || defined(SCO32v5)
		if (use_ansi_MC)
		{
			if (!ttyrdchk())
			{
				Nap(hzmsec * 3);
				if (!ttyrdchk())
					break;
			}
			read(0, (char *)&schar, 1);
			if (!lines_left)
				continue;
		}
		else
#endif /* M_SYSV */
		{
			if (srow == tcap_LINES)
				break;
			if (scol == tcap_COLS)
			{
				scol = 0;
				srow++;
				schar = NL;
			}
			else
			{
				schar = *sptr++;
				scol++;
			}
		}

		if ((schar > 0x7E) || (schar < 0x20))
		{
			switch (schar)
			{
				case NL:
					while ((cp > s256) && (*(cp - 1) == ' '))
						cp--;
					*cp++ = 0x0A;
					*cp = 0;
					fputs((char *)s256, fp);
					cp = s256;
					*cp = 0;
					--lines_left;
					continue;

				case at_TL:
					schar = vanilla_TL;
					break;
				case at_TR:
					schar = vanilla_TR;
					break;
				case at_BL:
					schar = vanilla_BL;
					break;
				case at_BR:
					schar = vanilla_BR;
					break;
				case at_LT: /* left hand T */
					schar = vanilla_LT;
					break;
				case at_RT: /* right hand T */
					schar = vanilla_RT;
					break;
				case at_VR: /* vertical rule */
					schar = vanilla_VR;
					break;
				case at_HR: /* horizontal rule */
					schar = vanilla_HR;
					break;
				default:
					schar = ' ';
			}
		}
		*cp++ = schar;
	}

#if 0 && defined(M_SYSV)  || defined(SCO32v5)
	if (use_ansi_MC)
	{
		/* restore keyboard termio at entry */
		ecusetattr(TTYIN, TCSETAW, &dump_tty_termio_at_entry);
		ttyflush(2);
#if defined(MULTISCREEN_DUMP_BUG)

		/*
		 * bug in XENIX 2.3.1 sco video driver leaves "ESC[2" active; use
		 * "l" (unlock tty) a noop
		 */
		write(TTYOUT, "l", 1);
#endif /* MULTISCREEN_DUMP_BUG */
	}
#endif /* M_SYSV */

	fclose(fp);

#if defined(MORSE)
	xbell(XBELL_DONE, 1);
#else
	ring_bell();
#endif

	if (restart_rcvr)
		start_rcvr_process(0);

}							 /* end of screen_dump */
