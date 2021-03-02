/*+-----------------------------------------------------------------
	ecugrabbag.c -- very machine/OS dependent functions
	wht@wht.net

  Defined functions:
	bell_alarm(xbell_type)
	bell_notify(xbell_type)
	bell_notify_text(bn)
	kbd_escape(xkey)
	morse_bell(xbell_type, count)
	parse_bell_notify_argument(strarg)
	rename(from, to)
	send_bell_fd(fd, count, nap_msec)
	send_get_response(narg, arg)
	set_bell_fd(fd, pitch, duration)
	set_default_escape_prompt()
	show_escape_prompt()
	signal_name_text(sig)
	xbell(type, count)
	xbell_fd(fd, type, count)
	xterm_title(text, code)

  This module is a grab bag for historical reasons.  Needs reorg.

  Like so many Americans, she was trying to construct a life that
  made sense from things she found in gift shops.  -- Kurt
  Vonnegut, Jr.

------------------------------------------------------------------*/
/*+:EDITS:*/
/*:04-26-2000-11:15-wht@bob-RELEASE 4.42 */
/*:01-24-1997-02:37-wht@yuriatin-SOURCE RELEASE 4.00 */
/*:09-11-1996-20:00-wht@yuriatin-3.48-major telnet,curses,structural overhaul */
/*:08-20-1996-12:39-wht@kepler-locale/ctype fixes from ache@nagual.ru */
/*:08-11-1996-02:10-wht@kepler-rename ecu_log_event to logevent */
/*:11-23-1995-11:20-wht@kepler-source control 3.37 for tsx-11 */
/*:11-14-1995-10:23-wht@kepler-3.37.80-source control point: SOCKETS */
/*:05-11-1995-15:55-wht@n4hgf-ck_sigint fools optimizing compilers */
/*:04-02-1995-04:06-wht@n4hgf-sgr command bug on console display */
/*:01-12-1995-15:19-wht@n4hgf-apply Andrew Chernov 8-bit clean+FreeBSD patch */
/*:12-27-1994-15:12-wht@n4hgf-turn on rename if CFG_FakeRename */
/*:05-04-1994-04:38-wht@n4hgf-ECU release 3.30 */
/*:11-12-1993-11:00-wht@n4hgf-Linux changes by bob@vancouver.zadall.com */
/*:08-07-1993-21:16-wht@n4hgf-entering history mgr from col > 1 caused glitch */
/*:01-01-1993-12:52-wht@n4hgf-add procedure binding for function keys */
/*:12-20-1992-12:37-wht@n4hgf-WHT experiment with attributes */
/*:09-10-1992-13:58-wht@n4hgf-ECU release 3.20 */
/*:08-22-1992-15:38-wht@n4hgf-ECU release 3.20 BETA */
/*:04-09-1992-05:47-wht@n4hgf-cleanup new "bn" argument parsing */
/*:02-16-1992-01:41-wht@n4hgf-turn off xterm_title */
/*:12-13-1991-17:14-wht@n4hgf-add bell_notify_text */
/*:12-13-1991-17:14-wht@n4hgf-add parse_bell_notify_argument */
/*:09-03-1991-18:23-wht@n4hgf-sigint rearrangement in send_get_response */
/*:08-28-1991-14:07-wht@n4hgf2-SVR4 cleanup by aega84!lh */
/*:08-13-1991-15:28-wht@n4hgf-more problems with history manager */
/*:07-25-1991-12:57-wht@n4hgf-ECU release 3.10 */
/*:07-17-1991-07:04-wht@n4hgf-avoid SCO UNIX nap bug */
/*:07-14-1991-18:18-wht@n4hgf-new ttygets functions */
/*:06-29-1991-15:42-wht@n4hgf-if WHT and xterm, play with title bar */
/*:06-04-1991-13:19-wht@n4hgf-WHT version always gets morse for bells */
/*:05-07-1991-06:10-wht@n4hgf-subtle changes in send_and_get_response */
/*:03-18-1991-22:31-wht@n4hgf-ISC 2.2 has rename() */
/*:01-16-1991-23:54-wht@n4hgf-if WHT, bell_notify always available */
/*:01-09-1991-22:31-wht@n4hgf-ISC port */
/*:12-04-1990-03:55-wht@n4hgf-bell_notify only if on multiscreen */
/*:09-19-1990-19:36-wht@n4hgf-logevent now gets pid for log from caller */
/*:08-14-1990-20:40-wht@n4hgf-ecu3.00-flush old edit history */

#include "ecu.h"
#include "ecukey.h"
#include "ecufkey.h"
#include "ecuxkey.h"
#include "ecutty.h"
#include "ecufork.h"
#include "esd.h"

#if defined(MORSE)
/* the space between # and include prevents make depend from seeing this */
#include <local/morse_dvr.h>
#endif

char *get_ttyname();

extern char curr_dir[];		 /* current working key defns */
extern KDE keyset_table[];
extern char keyset_name[];

ESD *icmd_prompt;			 /* interactive command prompt */
int icmd_prompt_len;

/*+-------------------------------------------------------------------------
	show_escape_prompt()
returns number of character positions written to screen
--------------------------------------------------------------------------*/
int
show_escape_prompt()
{
	char prompt_last;

	prompt_last = 'd';		 /* dummy */
	if (icmd_prompt->cb)
		prompt_last = *(icmd_prompt->pb + icmd_prompt->cb - 1);

	icmd_prompt_len = 0;
	shmx_make_rcvr_sleep(1);
	if (!(colors_current & 0xFFFF0000L) || !(colors_current & 0xFFFFL))
		setcolor(colors_normal);
	else
		setcolor(colors_current);
#ifdef WHT
	tcap_bold_on();
	tcap_underscore_on();
#else
	tcap_stand_out();
#endif
	if (icmd_prompt->cb)
	{
		ff(se, " %s", icmd_prompt->pb);
		icmd_prompt_len += icmd_prompt->cb + 1;
	}
	if (isalnum((uchar) prompt_last))
	{
		fputs(" %", se);
		icmd_prompt_len += 2;
	}
#ifdef WHT
	tcap_bold_off();
	tcap_underscore_off();
#else
	tcap_stand_end();
#endif
	fputc(' ', se);
	icmd_prompt_len++;
	return (icmd_prompt_len);

}							 /* end of show_escape_prompt */

/*+-------------------------------------------------------------------------
	set_default_escape_prompt()
--------------------------------------------------------------------------*/
void
set_default_escape_prompt()
{
	char *cp;
	char *getenv();

	if ((cp = getenv("ECUPROMPT")) != (char *)0)
	{
		strncpy(icmd_prompt->pb, cp, icmd_prompt->maxcb);
		*(icmd_prompt->pb + icmd_prompt->maxcb - 1) = 0;
		icmd_prompt->cb = strlen(icmd_prompt->pb);
		esd_null_terminate(icmd_prompt);
	}

}							 /* end of set_default_escape_prompt */

/*+-------------------------------------------------------------------------
	kbd_escape() -- xmtr got extended key -- process it
returns(1)
--------------------------------------------------------------------------*/
kbd_escape(xkey)
UINT xkey;
{
	int itmp;
	int ttygets_flags = TG_XDELIM;
	UINT delim;
	int strpos;
	int old_ttymode = get_ttymode();
	int backspace_count;
	KDE *tkde;
	uchar icmd_buf[128];

	if ((xkey >= XF_ALTA) && (xkey <= XF_ALTZ))
	{
		char alt_key_proc_name[8];
		char *arg = alt_key_proc_name;
		int narg = 1;
		int restart_rcvr = need_rcvr_restart();

		kill_rcvr_process(SIGUSR1);
		sprintf(alt_key_proc_name, "alt_%c", xkey - XF_ALTA + 'a');
		ttymode(2);
		ttyflush(0);
		if (do_proc(narg, &arg))
			ring_bell();
		sigint = 0;
		proc_file_reset();
		ttymode(old_ttymode);
		if (restart_rcvr)
			start_rcvr_process(0);
		return (1);
	}

	switch (xkey)
	{
		case XFhome:		 /* home key pressed ? */
			icmd_buf[0] = 0;
			backspace_count = show_escape_prompt();
			strpos = 0;
			icmd_buf[0] = 0;
		  GET_ICMD:
			ttygets(icmd_buf, sizeof(icmd_buf), ttygets_flags, &delim, &strpos);
			switch (delim)
			{
				case XFhome:
				case XFpgup:
				case XFcurup:
					itmp = strlen((char *)icmd_buf);
					while (itmp--)
						ff(se, "\b \b");
					if (icmd_history_manager(delim, icmd_buf, sizeof(icmd_buf)))
					{
						ring_bell();
						ttygets_flags |= 4;
						goto GET_ICMD;
					}
					break;
				case ESC:
				case NL:
					break;
				default:
					ring_bell();
					itmp = strlen((char *)icmd_buf);
					while (itmp--)
						ff(se, "\b \b");
					ttygets_flags |= 4;
					goto GET_ICMD;
			}
			if ((delim == ESC) || !icmd_buf[0])
			{
				while (backspace_count--)
					ff(se, "\b \b");
				break;
			}
			if (icmd(icmd_buf))
				termecu(TERMECU_OK);
			break;

		default:
			if ((itmp = xf_to_ikde(xkey)) < 0)
			{
				ring_bell();
				break;
			}
			tkde = &keyset_table[itmp];
			if ((itmp = tkde->count) > 0)
			{
				char *cp = tkde->str;

				while (itmp--)
					lputc(*cp++);
			}
			else if (tkde->count < 0)
			{
				switch (tkde->count)
				{
					case KACT_LOCAL_SHELL:
						fputs("\r\n", se);
						tcap_stand_out();
						ff(se, " local shell in %s ", curr_dir);
						tcap_stand_end();
						ff(se, "\r\n");
						shell("!");
						break;
					case KACT_REDISPLAY:
						redisplay_rcvr_screen();
						break;
					case KACT_PROC:
						{
#define ARG_MAX_QUAN 32
							char *arg[ARG_MAX_QUAN];
							int narg;

							build_arg_array(tkde->str, arg, ARG_MAX_QUAN, &narg);
							kill_rcvr_process(SIGUSR1);
							ttymode(2);
							do_proc(narg, arg);
							proc_file_reset();
							ttymode(1);
							sigint = 0;
						}
						break;
				}
			}
			else
				ring_bell();
			break;
	}
	return (1);
}							 /* end of kbd_escape */

/*+-------------------------------------------------------------------------
	set_bell_fd(fd,pitch,duration)
Example: 1B 5B 3D 34 30 30 30 3B 31 42 | .[=4000;1B
--------------------------------------------------------------------------*/
void
set_bell_fd(fd, pitch, duration)
int fd;
int pitch;
int duration;
{
#if defined(M_SYSV) || defined(SCO32v5)
	char bell_cmd[32];

	if (!tty_is_multiscreen)
		return;

	sprintf(bell_cmd, "\033[=%d;%dB", pitch, duration);
	write(fd, bell_cmd, strlen(bell_cmd));
#else

	/*
	 * shut up gcc warnings
	 */
	fd = pitch = duration = 0;
#endif
}							 /* end of set_bell_fd */

/*+-------------------------------------------------------------------------
	send_bell_fd(fd,count,nap_msec)
--------------------------------------------------------------------------*/
void
send_bell_fd(fd, count, nap_msec)
int fd;
int count;
int nap_msec;
{
	static char bellch = 0x07;

	if (count)
	{
		while (count--)
		{
			write(fd, &bellch, 1);
			Nap((long)nap_msec);
		}
	}
}							 /* end of send_bell_fd */

/*+-------------------------------------------------------------------------
	xbell_fd(fd,type,count)
--------------------------------------------------------------------------*/
void
xbell_fd(fd, type, count)
int fd;
int type;
int count;
{
#if defined(M_SYSV) || defined(SCO32v5)
	int itmp;

	if (!tty_is_multiscreen)
	{
		ring_bell();
		return;
	}

	if (count)
	{
		while (count--)
		{
			switch (type)
			{
				case XBELL_DONE:	/* octaves */
					set_bell_fd(fd, 1000, 1);
					send_bell_fd(fd, 1, 100);
					set_bell_fd(fd, 2000, 1);
					send_bell_fd(fd, 1, 100);
					set_bell_fd(fd, 4000, 1);
					send_bell_fd(fd, 1, 100);
					break;
				case XBELL_ATTENTION:	/* morse .-.-.- ATTENTION */
					for (itmp = 0; itmp < 3; itmp++)
					{
						set_bell_fd(fd, 2000, 1);
						send_bell_fd(fd, 1, 140);
						set_bell_fd(fd, 2000, 3);
						send_bell_fd(fd, 1, 340);
					}
					break;
				case XBELL_C:	/* morse -.-. C */
					for (itmp = 0; itmp < 2; itmp++)
					{
						set_bell_fd(fd, 2000, 3);
						send_bell_fd(fd, 1, 320);
						set_bell_fd(fd, 2000, 1);
						send_bell_fd(fd, 1, 120);
					}
					break;
				case XBELL_3T:	/* 3 morse T's */
					set_bell_fd(fd, 2000, 3);
					send_bell_fd(fd, 3, 460);
					break;
				default:
					set_bell_fd(fd, 2000, 4);
					send_bell_fd(fd, 1, 100);
					break;
			}
		}
	}

	set_bell_fd(fd, 4000, 1);
#else

	/*
	 * shut up gcc warnings
	 */
	fd = type = count = 0;
#endif
}							 /* end of xbell_fd */

/*+-------------------------------------------------------------------------
	morse_bell(xbell_type,count)
--------------------------------------------------------------------------*/
#if defined(MORSE)
morse_bell(xbell_type, count)
int xbell_type;
int count;
{
	int morse_fd;
	int morse_frequency = 800;
	char morse_char;
	static int morse_ticks = 0;

#if !defined(WHT)
	if (!tty_is_multiscreen)
	{
		ring_bell();
		return (0);
	}
#endif

	if (!morse_ticks)
		morse_ticks = hertz / 25;

	if ((morse_fd = open("/dev/morse", O_WRONLY, 0)) < 0)
		return (-1);

	ioctl(morse_fd, MORSE_SET_SPEED, &morse_ticks);
	ioctl(morse_fd, MORSE_SET_FREQUENCY, &morse_frequency);
	switch (xbell_type)
	{
		case XBELL_DONE:
			morse_char = 'd';
/*
			morse_frequency = 400;
			ioctl(morse_fd,MORSE_SET_FREQUENCY,&morse_frequency);
*/
			break;
		case XBELL_ATTENTION:
			morse_char = '.';
			break;
		case XBELL_C:
			morse_char = 'c';
			break;
		case XBELL_3T:
			morse_char = 'o';
			break;
		default:
			morse_char = BT;
			break;
	}
	while (count--)
		write(morse_fd, &morse_char, 1);
	close(morse_fd);
	return (0);
}							 /* end of morse_bell */
#endif

/*+-------------------------------------------------------------------------
	xbell(type,count)
--------------------------------------------------------------------------*/
void
xbell(type, count)
int type;
int count;
{
#if defined(WHT) && defined(MORSE)
	if (morse_bell(type, count))
		ring_bell();
#else
#if defined(WHT) && defined(AUDIO)
	void audio_notify();

	audio_notify(type);
#else
	if (!tty_is_multiscreen)
	{
		ring_bell();
		return;
	}

#if defined(MORSE)
	if (morse_bell(type, count))
#endif
		xbell_fd(1, type, count);
#endif /* WHT && AUDIO */
#endif /* WHT && MORSE */

}							 /* end of xbell */

/*+-------------------------------------------------------------------------
	bell_alarm(xbell_type)
  ring bell on multiscreens; if morse driver included, use it instead
--------------------------------------------------------------------------*/
int
bell_alarm(xbell_type)
int xbell_type;
{
#if defined(M_SYSV) || defined(SCO32v5)
	int notify_fd;
	int fork_pid;
	static long notify_time = 0L;
	char *get_ttyname();
	char devname[64];
	int devnum;
	int ttnum;

#if !(defined(WHT) && defined(MORSE))
	if (!tty_is_multiscreen)
	{
		ring_bell(xbell_type);
		return (1);
	}
#endif

	ttnum = atoi(get_ttyname() + 8);

/* if happened less than 15 secs ago, forget it */
	if ((time((long *)0) - notify_time) < 15L)
		return (0);

	notify_time = time((long *)0);

#if defined(MORSE)
	if (morse_bell(xbell_type, 1))
	{
#endif
		if ((fork_pid = smart_fork()) != 0)
		{
#if defined(FORK_DEBUG)
			sprintf(devname, "DEBUG bell notify pid %d", fork_pid);
			logevent(getpid(), devname);	/* bell notify */
#endif
			return (fork_pid > 0);
		}

		for (devnum = 1; devnum < 13; devnum++)
		{
			if (devnum == ttnum)	/* don't bell notify ecu's tty */
				continue;
			sprintf(devname, "/dev/tty%02d", devnum);
			if ((notify_fd = open(devname, O_WRONLY, 0)) >= 0)
			{
				xbell_fd(notify_fd, xbell_type, 1);
				close(notify_fd);
			}
		}

		_exit(0);			 /* end of child tine (of the fork, naturally) */
#if defined(MORSE)
	}
	/* NOTREACHED */
#endif
#else /* not SCO (M_SYSV) */
	ring_bell(xbell_type);
#endif
	return (1);
}							 /* end of bell_alarm */

/*+-------------------------------------------------------------------------
	bell_notify(xbell_type)
--------------------------------------------------------------------------*/
void
bell_notify(xbell_type)
int xbell_type;
{
#if defined(M_SYSV) || defined(SCO32v5)
	if (
#if !defined(WHT) && !defined(PTY_BELL_NOTIFY)
		tty_is_multiscreen &&
#endif
		shm->bell_notify_state)
	{
		bell_alarm(xbell_type);
	}
#else
	ring_bell(xbell_type);
#endif
}							 /* end of bell_notify */

/*+-------------------------------------------------------------------------
	parse_bell_notify_argument(strarg) - parse "bell notify" argument

  Returns 0,1,2 according to parsing rules
          -1 on error
--------------------------------------------------------------------------*/
int
parse_bell_notify_argument(strarg)
char *strarg;
{
	static STR_CLASSIFY sc[] =
	{
		{"off", 3, 0},
		{"on", 2, 1},
		{"alert", 1, 2},
		{(char *)0, 0, -1},
	};
	int itmp;

	if (isdigit((uchar) * strarg))
	{
		if (((itmp = atoi(strarg)) >= 0) && (itmp < 3))
			return (itmp);
		return (-1);
	}

	return (str_classify(sc, strarg));

}							 /* end of parse_bell_notify_argument */

/*+-------------------------------------------------------------------------
	bell_notify_text(bn) - text for bell notify status
--------------------------------------------------------------------------*/
char *
bell_notify_text(bn)
int bn;
{
	char *cp = "?";

	switch (bn)
	{
		case 0:
			cp = "OFF";
			break;
		case 1:
			cp = "ON";
			break;
		case 2:
			cp = "ON+ALERT";
			break;
		default:
			cp = "???";
			break;
	}
	return (cp);
}							 /* end of bell_notify_text */

/*+-------------------------------------------------------------------------
	signal_name_text(sig)
--------------------------------------------------------------------------*/
char *
signal_name_text(sig)
int sig;
{
	char *cp;
	static char sigunknown[20];

	sig &= 0x7F;
	switch (sig)
	{
		case SIGHUP:
			cp = "SIGHUP";
			break;
		case SIGINT:
			cp = "SIGINT";
			break;
		case SIGQUIT:
			cp = "SIGQUIT";
			break;
		case SIGILL:
			cp = "SIGILL";
			break;
		case SIGTRAP:
			cp = "SIGTRAP";
			break;
#ifdef SIGIOT
		case SIGIOT:
			cp = "SIGIOT";
			break;
#endif
#ifdef SIGEMT
		case SIGEMT:
			cp = "SIGEMT";
			break;
#endif
		case SIGFPE:
			cp = "SIGFPE";
			break;
		case SIGKILL:
			cp = "SIGKILL";
			break;
#if	defined(SIGBUS)
		case SIGBUS:
			cp = "SIGBUS";
			break;
#endif
		case SIGSEGV:
			cp = "SIGSEGV";
			break;
#if	defined(SIGSYS)
		case SIGSYS:
			cp = "SIGSYS";
			break;
#endif
		case SIGPIPE:
			cp = "SIGPIPE";
			break;
		case SIGALRM:
			cp = "SIGALRM";
			break;
		case SIGTERM:
			cp = "SIGTERM";
			break;
		case SIGUSR1:
			cp = "SIGUSR1";
			break;
		case SIGUSR2:
			cp = "SIGUSR2";
			break;
		case SIGCLD:
			cp = "SIGCLD";
			break;
#if	defined(SIGPWR)
		case SIGPWR:
			cp = "SIGPWR";
			break;
#endif
#if	defined(SIGSTOP)
		case SIGSTOP:
			cp = "SIGSTOP";
			break;
#endif
#if	defined(SIGTSOP)
		case SIGTSTP:
			cp = "SIGTSTP";
			break;
#endif
#if	defined(SIGCONT)
		case SIGCONT:
			cp = "SIGCONT";
			break;
#endif
#if	defined(SIGTTIN)
		case SIGTTIN:
			cp = "SIGTTIN";
			break;
#endif
#if	defined(SIGTTOU)
		case SIGTTOU:
			cp = "SIGTTOU";
			break;
#endif
		default:
			sprintf(sigunknown, "SIGNAL %u", sig);
			return (sigunknown);
	}
	return (cp);

}							 /* end of signal_name_text */

/*+-------------------------------------------------------------------------
	rename(from,to)
--------------------------------------------------------------------------*/
#if defined(M_XENIX) || defined(CFG_FakeRename)
int
rename(from, to)
char *from;
char *to;
{
	struct stat ss;
	int save_errno;
	extern int errno;

	if (!stat(to, &ss))		 /* if to exists, flunk */
	{
		errno = EEXIST;		 /* fake "file exists" error */
		return (-1);
	}

	if (link(from, to))		 /* if cannot link, flunk */
		return (-1);

	if (unlink(from))		 /* if cannot unlink, flunk */
	{
		save_errno = errno;
		unlink(to);
		errno = save_errno;
		return (-1);
	}

	return (0);

}							 /* end of rename */
#endif

/*+-------------------------------------------------------------------------
	send_get_response(narg,arg) - send a string, get and log response
--------------------------------------------------------------------------*/
/*ARGSUSED*/
void
send_get_response(narg, arg)
int narg;
char **arg;
{
	int itmp;
	int cmd;
	int mode;
	LRWT lr;
	char buf[1024];
	FILE *fplog;
	char *fname = "./ecu.sgr.log";
	static long sgrto1 = 10 * 1000L;
	static long sgrto2 = 2 * 1000L;
	long to;
	int restart_rcvr;

#define SGR_EXEC    0
#define SGR_SET_TO1 1
#define SGR_SET_TO2 2

	/*
	 * combination usage/parse hack detect unsupported commands and give
	 * usage
	 */
	if (!strcmp(arg[0], "sgr"))
		cmd = SGR_EXEC;
	else if (!strcmp(arg[0], "sgrto1"))
		cmd = SGR_SET_TO1;
	else if (!strcmp(arg[0], "sgrto2"))
		cmd = SGR_SET_TO2;
	else
	{
		ff(se, "   sgr logic error\r\n");
		return;
	}

	if ((cmd == SGR_EXEC) && (narg < 3))
	{
		ff(se, "\r\nusage: sgr <mode> <cmd>\r\n");
		ff(se, "    mode = 0 for sanitized response, 1 == raw\r\n");
		ff(se, "    cmd = 'Send' string (see 'expresp' proc cmd in manual)\r\n");
		ff(se, "    current sgrto1=%ld, sgrto2=%ld msec\r\n", sgrto1, sgrto2);
		return;
	}
	else if ((cmd != SGR_EXEC) && (narg != 2))
	{
		ff(se, "\r\nusage: sgrto1 <msec> or sgrto2 <msec>\r\n");
		ff(se, "    sets `sgr' to1=time for 1st character, to2 for later\r\n");
		ff(se, "    current sgrto1=%ld, sgrto2=%ld msec\r\n", sgrto1, sgrto2);
		return;
	}

	switch (cmd)
	{
		case SGR_EXEC:
			break;
		case SGR_SET_TO1:
		case SGR_SET_TO2:
			to = atol(arg[1]);
			if (to <= 0)
			{
				ff(se, "  cannot set timeout <= 0\r\n");
				return;
			}
			if (cmd == SGR_SET_TO1)
				sgrto1 = to;
			else
				sgrto2 = to;
			ff(se, "    sgrto1=%ld, sgrto2=%ld msec\r\n", sgrto1, sgrto2);
			return;
	}

	/*
	 * we are going to execute an `sgr'; don't want receiver stealing our
	 * response
	 */
	if (restart_rcvr = need_rcvr_restart())
		kill_rcvr_process(SIGUSR1);

	/*
	 * set up for capturing response
	 */
	mode = atoi(arg[1]) & 0x0F;;
	lr.to1 = sgrto1;
	lr.to2 = sgrto2;
	lr.raw_flag = 0x80 + mode;	/* allow interrupts */
	lr.buffer = buf;
	lr.bufsize = sizeof(buf);
	lr.delim = (char *)0;
	lr.echo_flag = 0;

	/*
	 * stimulus/response
	 */
	ff(se, "\r\nsend ... ");
	respond(arg[2]);
	ff(se, "wait (to1=%ld,to2=%ld) ... ", sgrto1, sgrto2);
	if (!ck_sigint())
	{
		lgets_timeout(&lr);
		if (ck_sigint())
			goto INTERRUPTED;
		ff(se, "done\r\n");

		/*
		 * log response if possible and display on screen
		 */
		if (fplog = fopen(fname, "a"))
		{
			itmp = strlen(arg[2]);
			hex_dump_fp(fplog, arg[2], itmp, "Stimulus", (itmp <= 16));
			hex_dump_fp(fplog, buf, lr.count, "Response", (lr.count <= 16));
			fputs("\n", fplog);
			fclose(fplog);
			ff(se, "\r\nDialog dump appended to '%s'\r\n", fname);
		}
		itmp = strlen(arg[2]);
		hex_dump(arg[2], itmp, "Stimulus", (itmp <= 16));
		hex_dump(buf, lr.count, "Response", (lr.count <= 16));
		if (!lr.count)
			ff(se, "\r\n");
	}
	else
	{
	  INTERRUPTED:
		sigint = 0;
		ff(se, "interrupted\r\n");
	}

	if (restart_rcvr)
		start_rcvr_process(0);
}							 /* end of send_get_response */

/*+-------------------------------------------------------------------------
	xterm_title(text,code) - experimental - watch this space
--------------------------------------------------------------------------*/
#if defined(WHT2) || defined(XTERM_FRIEND)
void
xterm_title(text, code)
char *text;
int code;
{
	static char *term = (char *)0;
	static char usrname[L_cuserid] = "";
	char *getenv();
	char *cuserid();
	static char xtstr1[82] = "";

	if (!term)
		term = getenv("TERM");
	if (!term)
	{
		term = "";
		return;
	}
	if (ulcmpb(term, "xterm") >= 0)
		return;

	if (!usrname[0])
		(void)cuserid(usrname);

	switch (code)
	{
		case 0:
		case 1:
			xtstr1[0] = 0;
			if (code == 1)
				strcpy(xtstr1, "ECU: ");
			strcat(xtstr1, text);
			if (usrname[0])
				sprintf(xtstr1 + strlen(xtstr1), " (%s)", usrname);
		case 2:
			fputs("\033]0;", se);
			fputs(xtstr1, se);
			fputc(7, se);
			break;
	}
}							 /* end of xterm_title */
#endif

/* end of ecugrabbag.c */
/* vi: set tabstop=4 shiftwidth=4: */
