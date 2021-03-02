char *revision = "@(#)ecuungetty 3.21";

#if defined(SHARE_DEBUG)
#ifndef ECUUNGETTY_DEBUG
#define ECUUNGETTY_DEBUG
#endif
#endif

/*+-------------------------------------------------------------------------
	ecuungetty.c - ecu "ungetty" program
	wht@wht.net

Get a line:
ecuungetty /dev/ttyxx <bamboozle-str>
ecuungetty -g /dev/ttyxx <bamboozle-str>

Test a line's atatus:
ecuungetty -t /dev/ttyxx <bamboozle-str>

Return a line:
ecuungetty -r /dev/ttyxx <bamboozle-str>

  Defined functions:
	assign_tty(tty, uid, gid, mode)
	errno_text(errnum)
	eug_exit(code)
	logevent(pid, event_note)
	main(argc, argv, envp)
	termecu()

No effort is made to close the passwd file with endpwent().
I use setpwent() instead.  It is so contrary for me to leave
a file open that I just had to put a reminder to myself here.
If the program lived for more than 1/2 second, I'd probably
keep to my usual practice.
--------------------------------------------------------------------------*/
/*+:EDITS:*/
/*:01-24-1997-02:37-wht@yuriatin-SOURCE RELEASE 4.00 */
/*:09-11-1996-20:00-wht@yuriatin-3.48-major telnet,curses,structural overhaul */
/*:08-20-1996-12:39-wht@kepler-locale/ctype fixes from ache@nagual.ru */
/*:08-11-1996-02:10-wht@kepler-rename ecu_log_event to logevent */
/*:11-23-1995-11:20-wht@kepler-source control 3.37 for tsx-11 */
/*:11-14-1995-10:23-wht@kepler-3.37.80-source control point: SOCKETS */
/*:05-04-1994-04:39-wht@n4hgf-ECU release 3.30 */
/*:07-14-1993-17:53-wht@n4hgf-dummy last_ugstat for ugtext.o */
/*:09-10-1992-13:59-wht@n4hgf-ECU release 3.20 */
/*:09-02-1992-07:10-wht@n4hgf-DEBUG now gets actual user log file name */
/*:09-02-1992-06:48-wht@n4hgf-UG_RESTART exit any time we chown tty to user */
/*:08-22-1992-15:38-wht@n4hgf-ECU release 3.20 BETA */
/*:08-16-1992-01:59-wht@n4hgf-absolutely ensure no chown/chmod of /dev/tty */
/*:08-07-1992-18:50-wht@n4hgf-chown/chmod both tty names on SCO */
/*:07-19-1992-09:07-wht@n4hgf-"rudimentary" security/validity checks on tty */
/*:06-19-1992-20:27-root@n4hgf-needed CFG_UngettyChown in another place */
/*:06-04-1992-12:21-wht@n4hgf-chown/chmod with debugging */
/*:04-27-1992-19:30-wht@n4hgf-add optional chown/chmod */
/*:04-24-1992-20:12-wht@n4hgf-bel@nosc.mil found long time bug - bad kill */
/*:08-10-1991-17:39-wht@n4hgf-US_WEGOTIT handling */
/*:08-07-1991-14:15-wht@n4hgf-add debug log event code */
/*:07-25-1991-12:57-wht@n4hgf-ECU release 3.10 */
/*:09-19-1990-19:36-wht@n4hgf-logevent now gets pid for log from caller */
/*:08-14-1990-20:40-wht@n4hgf-ecu3.00-flush old edit history */

#include <stdio.h>
#include <ctype.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include "../ecu_types.h"
#include "../ecu_stat.h"
#include <utmp.h>
#include <sys/locking.h>
#include "../ecuungetty.h"
#include "../utmpstatus.h"
#include "../ttynaming.h"

#ifdef CFG_UngettyChown
#include <pwd.h>
struct passwd *uupw;
struct passwd uucp_pwd;

#endif

void eug_exit();

extern struct utmp last_utmp;

int real_uid;
int xmtr_pid;
struct stat tstat;

char **gargv;
int gargc;

#if defined(ECUUNGETTY_DEBUG)
char s256[256];

#endif /* ECUUNGETTY_DEBUG */

/* not used but must be defined for shared LINST_text code */
char lopen_err_str[1];

/* not used but must be defined for shared UG_text code */
unsigned char last_ugstat;

/*+-------------------------------------------------------------------------
	logevent(pid,event_note)
--------------------------------------------------------------------------*/
void
logevent(pid, event_note)
int pid;
char *event_note;
{
#if defined(ECUUNGETTY_DEBUG)
	FILE *ecu_log_fp;
	static char logname[256] = "";

	if (!logname[0])
	{
		struct passwd *uidpw;

		setpwent();
		if (!(uidpw = getpwuid(real_uid)))
			eug_exit(UGE_LOGIC);
		strcpy(logname, uidpw->pw_dir);
		strcat(logname, "/.ecu/log");
	}

	if ((ecu_log_fp = fopen(logname, "a")) != NULL)
	{
		locking(fileno(ecu_log_fp), LK_LOCK, 0L);
		fputs("ECUUNGET", ecu_log_fp);
		fprintf(ecu_log_fp, "-%05d-(%05d) ", getppid(), pid);
		fputs(event_note, ecu_log_fp);
		fputs("\n", ecu_log_fp);
		fflush(ecu_log_fp);
		locking(fileno(ecu_log_fp), LK_UNLCK, 0L);
		fclose(ecu_log_fp);
	}
#endif
}							 /* end of logevent */

/*+-------------------------------------------------------------------------
	assign_tty(tty,uid,gid,mode) - set a tty owner, group, mode

returns 0 on success, -1 on error
--------------------------------------------------------------------------*/
int
assign_tty(tty, uid, gid, mode)
char *tty;
int uid;
int gid;
int mode;
{
#ifndef CFG_UngettyChown
	return (0);
#else
#if defined(ECUUNGETTY_DEBUG)
	struct stat tstat2;

#endif /* ECUUNGETTY_DEBUG */
#ifdef SCO_TTY_NAMING
	char other_tty[128];
	int itmp;
	char *cptr;

#endif /* SCO_TTY_NAMING */
#if defined(ECUUNGETTY_DEBUG) && defined(SCO_TTY_NAMING)
	struct stat otstat;

#endif /* ECUUNGETTY_DEBUG && SCO_TTY_NAMING */

	if (!strcmp(tty, "/dev/tty"))	/* somebody reported this is still
									 * happening */
		eug_exit(UGE_BADARGV);

#ifdef SCO_TTY_NAMING
	itmp = strlen(tty);
	if (itmp > 1)
	{
		strcpy(other_tty, tty);
		cptr = other_tty + itmp - 1;
		if (isalpha((unsigned char)*cptr))
		{
			*cptr = (isupper((unsigned char)*cptr))
				? tolower((unsigned char)*cptr)
				: toupper((unsigned char)*cptr);
		}
#if defined(ECUUNGETTY_DEBUG)
		stat(other_tty, &otstat);
#endif /* ECUUNGETTY_DEBUG */
		chown(other_tty, uid, gid);
		chmod(other_tty, (unsigned short)mode);
	}
#endif /* SCO_TTY_NAMING */

	chown(tty, uid, gid);
	chmod(tty, (unsigned short)mode);

#if defined(ECUUNGETTY_DEBUG)
	stat(tty, &tstat2);
	sprintf(s256, "TTY '%s' o=(%d,%d) m=0%o (was %d,%d,0%o)",
		tty,
		tstat2.st_uid, tstat2.st_gid, tstat2.st_mode & 0777,
		tstat.st_uid, tstat.st_gid, tstat.st_mode & 0777);
	logevent(getpid(), s256);
#ifdef SCO_TTY_NAMING
	stat(other_tty, &tstat2);
	sprintf(s256, "TTY '%s' o=(%d,%d) m=0%o (was %d,%d,0%o)",
		other_tty,
		tstat2.st_uid, tstat2.st_gid, tstat2.st_mode & 0777,
		otstat.st_uid, otstat.st_gid, otstat.st_mode & 0777);
	logevent(getpid(), s256);
#endif /* SCO_TTY_NAMING */
#endif /* ECUUNGETTY_DEBUG */

#endif /* CFG_UngettyChown */
}							 /* end of assign_tty */

/*+-------------------------------------------------------------------------
	eug_exit(code) - exit() with debug logging
--------------------------------------------------------------------------*/
void
eug_exit(code)
int code;
{
#if defined(ECUUNGETTY_DEBUG)
	int iargv;
	char s1024[1024];

	s1024[0] = 0;
	for (iargv = 1; iargv < gargc; iargv++)
	{
		strcat(s1024, gargv[iargv]);
		strcat(s1024, " ");
	}
	sprintf(s1024 + strlen(s1024), "exit code %d", code);
	logevent(getpid(), s1024);
#endif
	exit(code);
}							 /* end of eug_exit */

/*+-------------------------------------------------------------------------
	termecu() - "dummy" for utmpstat.c

This particular incantation will only be called if utmp is non-existent
or not readable.......
--------------------------------------------------------------------------*/
termecu()
{
	eug_exit(UGE_LOGIC);
}							 /* end of hangup */

/*+-------------------------------------------------------------------------
	errno_text(errnum)
--------------------------------------------------------------------------*/
char *
errno_text(errnum)
int errnum;
{
	static char errstr[16];

	sprintf(errstr, "%d", errnum);
	return (errstr);
}							 /* end of errno_text */

/*+-------------------------------------------------------------------------
	main(argc,argv,envp)
--------------------------------------------------------------------------*/
main(argc, argv, envp)
int argc;
char **argv;
char **envp;
{
	int op = 'g';			 /* assume "get" operation */
	int status;
	int itmp = 0;
	char *cptr;
	char *tty = argv[1];
	char *bamboozlement = argv[2];
	char *bamboozle();

	gargv = argv;
	gargc = argc;

	real_uid = getuid();
	if (geteuid())
		eug_exit(UGE_NOTROOT);

#ifdef CFG_UngettyChown
	if (!(uupw = getpwnam("uucp")))
		eug_exit(UGE_NOUUCP);
	uucp_pwd = *uupw;
	uupw = &uucp_pwd;
#endif

	if (*argv[1] == '-')
	{
		switch (op = *(argv[1] + 1))
		{
			case 'r':
			case 't':
				break;
			default:
				eug_exit(UGE_BADSWITCH);
		}
		if (argc < 3)
			eug_exit(UGE_BADARGC);
		tty = argv[2];
		bamboozlement = argv[3];
	}
	else if (argc <= 2)
		eug_exit(UGE_BADARGC);

	if (real_uid)			 /* if caller not actually root */
	{
		if (strcmp(bamboozlement, bamboozle(getppid())))
			eug_exit(UGE_CALLER);
	}

	/*
	 * rudimentary checks must be in /dev, no ".." in path,not
	 * /dev/tty,must be char special
	 */
	if (strncmp(tty, "/dev/", 5))
		eug_exit(UGE_BADARGV);
	if ((cptr = strchr(tty, '.')) && (*(cptr + 1) == '.'))
		eug_exit(UGE_BADARGV);
	if (!strcmp(tty, "/dev/tty"))
		eug_exit(UGE_BADARGV);
	if (stat(tty, &tstat))
	{
#if defined(ECUUNGETTY_DEBUG)
		sprintf(s256, "TTY '%s' stat error %d", tty, errno);
		logevent(getpid(), s256);
#endif /* ECUUNGETTY_DEBUG */
		eug_exit(UGE_BADARGV);
	}
	if ((tstat.st_mode & S_IFMT) != S_IFCHR)
		eug_exit(UGE_BADARGV);

	xmtr_pid = getppid();
	status = utmp_status(tty);
#if defined(ECUUNGETTY_DEBUG)
	sprintf(s256, "-%c utmp status=%d", op, status);
	logevent(getpid(), s256);
#endif

	switch (op)
	{
		case 'g':
			switch (status)
			{
				case US_NOTFOUND:	/* not in utmp, or getty dead */
					itmp = assign_tty(tty, real_uid, getgid(), 0622);
					eug_exit((itmp) ? UG_NOTENAB : UG_RESTART);
				case US_LOGIN:	/* enabled for login, idle */
					kill(last_utmp.ut_pid, SIGUSR1);
					nap(200L);
					itmp = assign_tty(tty, real_uid, getgid(), 0622);
					eug_exit((itmp) ? UG_NOTENAB : UG_RESTART);
				case US_DIALOUT:	/* enabled for login, currently
									 * dialout */
				case US_LOGGEDIN:	/* enabled for login, in use */
					eug_exit(UG_FAIL);
				case US_WEGOTIT:	/* we on it */
					itmp = assign_tty(tty, real_uid, getgid(), 0622);
#if 1
					eug_exit((itmp) ? UG_NOTENAB : UG_RESTART);
#else
					if (!kill(last_utmp.ut_pid, 0))	/* is there a getty? */
						eug_exit(UG_RESTART);	/* yes */
					else
						eug_exit(UG_NOTENAB);
#endif
			}
			break;

		case 't':			 /* no longer called by ecu as of BETA 3.20.02 */
			switch (status)
			{
				case US_NOTFOUND:	/* not in utmp, or getty dead */
#ifdef CFG_UngettyChown
					assign_tty(tty, uupw->pw_uid, uupw->pw_gid, 0640);
#endif
					eug_exit(UGE_T_NOTFOUND);
				case US_LOGIN:	/* enabled for login, idle */
					eug_exit(UGE_T_LOGIN);
				case US_LOGGEDIN:	/* enabled for login, in use */
					eug_exit(UGE_T_LOGGEDIN);
				case US_WEGOTIT:	/* we have the line */
#ifdef CFG_UngettyChown
					assign_tty(tty, uupw->pw_uid, uupw->pw_gid, 0640);
#endif
					eug_exit(UG_RESTART);
				case US_DIALOUT:	/* enabled for login, currently
									 * dialout */
					eug_exit(UG_RESTART);
			}
			break;

		case 'r':
			switch (status)
			{
				case US_NOTFOUND:	/* not in utmp, or getty dead */
				case US_LOGIN:	/* enabled for login, idle */
#ifdef CFG_UngettyChown
					assign_tty(tty, uupw->pw_uid, uupw->pw_gid, 0640);
#endif
					eug_exit(0);
				case US_LOGGEDIN:	/* enabled for login, in use */
					eug_exit(0);
				case US_WEGOTIT:	/* we own it */
				case US_DIALOUT:	/* enabled for login, currently
									 * dialout */
#ifdef CFG_UngettyChown
					assign_tty(tty, uupw->pw_uid, uupw->pw_gid, 0640);
#endif
					itmp = 5;
					while (itmp--)
					{
						if (kill(last_utmp.ut_pid, SIGUSR2))
							break;
						nap(100L);
					}
					eug_exit(0);
			}
			break;
	}
	eug_exit(UGE_LOGIC);
}							 /* end of main */

/* vi: set tabstop=4 shiftwidth=4: */
/* end of ecuungetty.c */
