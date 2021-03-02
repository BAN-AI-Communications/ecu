/*+-------------------------------------------------------------------------
	utmpstat.c - utmp status for XENIX/UNIX line
	wht@wht.net

  Defined functions:
	US_text(us_val)
	strcmpi(s1, s2)
	to_lower(ch)
	to_upper(ch)
	ut_type_text(ut_type)
	utmp_status(line)

                   system boot         0 Fri Apr 24 07:18:52 1992
                   run-level 2         0 Fri Apr 24 07:18:52 1992
asktimerck ck                         15 Fri Apr 24 07:19:38 1992
cat        copy                       17 Fri Apr 24 07:19:38 1992
brc        brc                        18 Fri Apr 24 07:19:39 1992
brc        mt                         22 Fri Apr 24 07:19:39 1992
authckrcac ack                        26 Fri Apr 24 07:19:39 1992
rc2        r2                         27 Fri Apr 24 07:20:05 1992
LOGIN      co      tty01             170 Fri Apr 24 07:20:09 1992
LOGIN      c02     tty02             171 Fri Apr 24 07:20:09 1992
uugetty    u2B     tty2B            3837 Fri Apr 24 21:24:38 1992
uugetty    u2h                       190 Fri Apr 24 07:20:08 1992
uugetty    u1A                      3830 Fri Apr 24 21:24:10 1992
wht        p0      ttyp0             206 Fri Apr 24 07:20:43 1992
wht        p1      ttyp1            1515 Fri Apr 24 20:55:53 1992
wht        p2      ttyp2            2929 Fri Apr 24 20:55:45 1992

--------------------------------------------------------------------------*/
/*+:EDITS:*/
/*:04-26-2000-11:16-wht@bob-RELEASE 4.42 */
/*:01-24-1997-02:38-wht@yuriatin-SOURCE RELEASE 4.00 */
/*:09-11-1996-20:01-wht@yuriatin-3.48-major telnet,curses,structural overhaul */
/*:08-20-1996-12:39-wht@kepler-locale/ctype fixes from ache@nagual.ru */
/*:08-11-1996-02:10-wht@kepler-rename ecu_log_event to logevent */
/*:11-23-1995-11:20-wht@kepler-source control 3.37 for tsx-11 */
/*:11-14-1995-10:23-wht@kepler-3.37.80-source control point: SOCKETS */
/*:11-03-1995-18:20-wht@wwtp1-internationalize to_lower/to_upper */
/*:01-12-1995-15:20-wht@n4hgf-apply Andrew Chernov 8-bit clean+FreeBSD patch */
/*:05-04-1994-04:40-wht@n4hgf-ECU release 3.30 */
/*:10-28-1993-12:07-wht@n4hgf-Gert Doering fixes -- he be kool */
/*:07-10-1993-18:24-wht@n4hgf-add ut_type tests */
/*:06-26-1993-16:59-wht@n4hgf-enhance US_text */
/*:05-29-1993-20:23-wht@n4hgf-handle /dev/ in ut_line and flex 'getty' test */
/*:05-29-1993-19:55-wht@n4hgf-add US_text */
/*:09-10-1992-14:00-wht@n4hgf-ECU release 3.20 */
/*:09-02-1992-14:18-wht@n4hgf-some mark dead utmp entries instead of rming */
/*:08-22-1992-15:39-wht@n4hgf-ECU release 3.20 BETA */
/*:06-30-1992-14:46-wht@n4hgf-honor DIALOUT set by 3.2v4 getty when we lock */
/*:04-28-1992-03:58-wht@n4hgf-check SCO utmp entry against ut_id */
/*:04-24-1992-21:59-wht@n4hgf-more SCO tty name normalizing */
/*:11-08-1991-21:09-root@n4hgf-bug in strcmpi made for erratic return value */
/*:08-25-1991-14:39-wht@n4hgf-SVR4 port thanks to aega84!lh */
/*:08-21-1991-02:23-wht@n4hgf-sun port */
/*:08-10-1991-17:39-wht@n4hgf-US_WEGOTIT handling */
/*:07-25-1991-12:59-wht@n4hgf-ECU release 3.10 */
/*:02-13-1991-02:00-ache@hq.demos.su-swap patch 5 US_ return values */
/*:02-07-1991-00:28-wht@n4hgf-utmp_status() was really messed up */
/*:02-03-1991-17:52-ache@hq.demos.su-fix for XENIX utmp handling bug */
/*:10-16-1990-20:43-wht@n4hgf-add SHARE_DEBUG */
/*:09-19-1990-19:36-wht@n4hgf-logevent now gets pid for log from caller */
/*:08-14-1990-20:40-wht@n4hgf-ecu3.00-flush old edit history */

#include "ecu.h"
#include "termecu.h"
#include "utmpstatus.h"
#include "ecuungetty.h"
#include "dialprog.h"
#include <errno.h>
#include <utmp.h>
#ifdef __FreeBSD__ /* superfluous? */
#include <ctype.h>
#endif

#if defined(sun)
#define ut_id ut_host		 /* fake debug info */
#else
#if !defined(ut_name)		 /* nobody can keep their mind made up; ... */
#define ut_name ut_user		 /* ... this is getting verry difficult, very
							  * old */
#endif
#endif /* sun */

#if defined(SVR4)
char *utmp_file = "/var/adm/utmp";

#else
#ifdef _PATH_UTMP
char *utmp_file = _PATH_UTMP;

#else
char *utmp_file = "/etc/utmp";

#endif
#endif

struct utmp last_utmp;

/*+-------------------------------------------------------------------------
    to_upper() / to_lower()

One would think that these were relatively standard types of
thing, but System V specifies toupper() to convert to upper case
if not already and BSD says to adjust without testing, so, two
stupid little routines here.  ASCII only -- no EBCDIC gradoo here please.
--------------------------------------------------------------------------*/
UINT
to_upper(ch)
unsigned char ch;
{
	return islower(ch) ? toupper(ch) : ch;
}							 /* end of to_upper() */

UINT
to_lower(ch)
unsigned char ch;
{
	return isupper(ch) ? tolower(ch) : ch;
}							 /* end of to_lower() */

/*+-------------------------------------------------------------------------
	strcmpi(s1,s2) - case-insensitive strcmp

This version of strcmp() is case-insensitive and works like a sane one
should, per strcmp(3), not per  the K&R1 example or POSIX/ANSI.

In here rather than ecuutil.c since other executables besides ecu
uses this module and strcmpi needed there too
--------------------------------------------------------------------------*/
int
strcmpi(s1, s2)
char *s1;
char *s2;
{

	while (*s1)
	{
		if (to_upper(*s1++) != to_upper(*s2++))
		{
			s1--;
			s2--;
			break;
		}
	}
	return (to_upper(*s1) - to_upper(*s2));

}							 /* end of strcmpi */

/*+-------------------------------------------------------------------------
	US_text(us_val) - text for US_xxx utmp status codes
--------------------------------------------------------------------------*/
char *
US_text(us_val)
enum utmp_status us_val;
{
	static char errant[32];

	switch (us_val)
	{
		case US_UNDEF:
			return ("UNDEF");
		case US_NOTFOUND:
			return ("NOTFOUND");
		case US_LOGIN:
			return ("LOGIN");
		case US_DIALOUT:
			return ("DIALOUT");
		case US_LOGGEDIN:
			return ("LOGGEDIN");
		case US_WEGOTIT:
			return ("WEGOTIT");
		default:
			sprintf(errant, "<US=%u?>", us_val);
			return (errant);
	}

}							 /* end of US_text */

/*+-------------------------------------------------------------------------
	ut_type_text(ut_type) - text for ut_type_xxx utmp status codes
--------------------------------------------------------------------------*/
#ifdef INIT_PROCESS
char *
ut_type_text(ut_type)
int ut_type;
{
	static char errant[32];

	switch (ut_type)
	{
#ifdef EMPTY
		case EMPTY:
			return ("EMPTY");
#endif
#ifdef RUN_LVL
		case RUN_LVL:
			return ("RUN_LVL");
#endif
#ifdef BOOT_TIME
		case BOOT_TIME:
			return ("BOOT_TIME");
#endif
#ifdef OLD_TIME
		case OLD_TIME:
			return ("OLD_TIME");
#endif
#ifdef NEW_TIME
		case NEW_TIME:
			return ("NEW_TIME");
#endif
#ifdef INIT_PROCESS
		case INIT_PROCESS:
			return ("INIT_PROCESS");
#endif
#ifdef LOGIN_PROCESS
		case LOGIN_PROCESS:
			return ("LOGIN_PROCESS");
#endif
#ifdef USER_PROCESS
		case USER_PROCESS:
			return ("USER_PROCESS");
#endif
#ifdef DEAD_PROCESS
		case DEAD_PROCESS:
			return ("DEAD_PROCESS");
#endif
#ifdef ACCOUNTING
		case ACCOUNTING:
			return ("ACCOUNTING");
#endif
		default:
			sprintf(errant, "<ut_type=%u?>", ut_type);
			return (errant);
	}

}							 /* end of ut_type_text */
#endif /* INIT_PROCESS */

/*+-------------------------------------------------------------------------
	utmp_status(line) - check line status in utmp
'line' is "/dev/ttyxx"-style
returns US_ value and global utmp struct last_utmp;
--------------------------------------------------------------------------*/
enum utmp_status
utmp_status(line)
char *line;
{
#if defined sun || defined(BSD) || defined(__FreeBSD__)
	return (US_NOTFOUND);
#else
	int itmp;
	int status = US_NOTFOUND;
	int ufd;

#if defined(UTMP_DEBUG)
	char s512[512];

#endif

/*
 * crock/bozo alert:
 * ut_name ain't but EIGHT characters long, but
 * EIGHT characters are often stored, so ya don't get no null
 * ut_id ain't but FOUR characters long, but
 * FOUR characters are routinely stored, so ya don't get no null
 */
	char namecopy[sizeof(last_utmp.ut_name) + 1];
	char idcopy[sizeof(last_utmp.ut_id) + 1];
	char linecopy[sizeof(last_utmp.ut_line) + 1];

	if ((ufd = open(utmp_file, O_RDONLY, 755)) < 0)
	{
		perror(utmp_file);
		termecu(TERMECU_LINE_OPEN_ERROR);
	}

	while ((status == US_NOTFOUND) &&
		(read(ufd, (char *)&last_utmp, sizeof(last_utmp)) == sizeof(last_utmp)))
	{

		/*
		 * make copies of each utmp entry we think might not be zero
		 * padded
		 * 
		 * be polite and skip over any non-standard "/dev/" we might find in
		 * a ut_line entry
		 */
		strncpy(namecopy, last_utmp.ut_name, sizeof(last_utmp.ut_name));
		namecopy[sizeof(last_utmp.ut_name)] = 0;

		strncpy(idcopy, last_utmp.ut_id, sizeof(last_utmp.ut_id));
		idcopy[sizeof(last_utmp.ut_id)] = 0;

		itmp = (!strncmp(last_utmp.ut_line, "/dev/", 5)) ? 5 : 0;
		strncpy(linecopy, last_utmp.ut_line + itmp,
			sizeof(last_utmp.ut_line) - itmp);
		linecopy[sizeof(last_utmp.ut_line) - itmp] = 0;

		/*
		 * yetch! SCO uugetty doesn't always plug ut_line!!!
		 * 
		 * However, most folks seem to follow the convention of making the
		 * last two characters of the inittab id field (hence the ut_id
		 * field) match the last two characters of the tty name
		 * 
		 * The following code runs on SCO to check the last two characters of
		 * the ut_id field against the last two characters of the line
		 * being tested
		 * 
		 * This only works for ttys with "standard" names. If you don't find
		 * standard names, you probably won't find the convention followed
		 * anyway.
		 */

#if defined(M_SYSV) || defined(SCO32v5)	/* SCO */
		if (!linecopy[0])	 /* if ut_line entry is null */
		{
			int itmp2;

			if (!linecopy[0] &&
				((itmp = strlen(line)) > 2) &&
				((itmp2 = strlen(idcopy)) > 2) &&
				!strcmpi(line + itmp - 2, idcopy + itmp2 - 2))
			{
				if (itmp = line_lock_status(line))
				{
					if (itmp == LINST_WEGOTIT)
						status = US_WEGOTIT;
					else
						status = US_DIALOUT;
				}
				else
					status = US_LOGIN;
				break;
			}
			continue;
		}
#endif

		/*
		 * if the name does not match, skip this entry
		 */
		if (TTYNAME_STRCMP(linecopy, line + 5))
			continue;

		/*
		 * THE LINE MATCHES -- now determine its status
		 */
#ifdef INIT_PROCESS
		if (last_utmp.ut_type == INIT_PROCESS)
			goto GETTY_RUNNING;
#endif /* INIT_PROCESS */

#ifdef LOGIN_PROCESS
		if (last_utmp.ut_type == LOGIN_PROCESS)
		{
			status = US_LOGIN;
			break;
		}
#endif

		if (!strncmp(namecopy, "LOGIN", 5))
			status = US_LOGIN;
		else if (ulindex(namecopy, "getty") >= 0)
		{
		  GETTY_RUNNING:

			/*
			 * some getty is running (note we match "getty", "uugetty" or
			 * "XYZGETTY")
			 */
			if (itmp = line_lock_status(line))
			{
				if (itmp == LINST_WEGOTIT)
					status = US_WEGOTIT;	/* hmmm.. 'getty' still in
											 * utmp? */
				else
					status = US_DIALOUT;	/* SCO locks tty during DCE
											 * init */
			}
			else
				status = US_LOGIN;
		}
		else if (!strcmp(namecopy, "DIALOUT"))
		{

			/*
			 * getty has seen a lock on a line and marked the utmp entry
			 * "DIALOUT".  This status is not reflected until
			 * 
			 * <1> getty has been SIGUSR1'd by ecuungetty on systems that
			 * support it, or
			 * 
			 * <2> getty sees carrier (when ECU connects outward and getty's
			 * waited open succeeds)
			 */
			status = US_DIALOUT;
			if (last_utmp.ut_pid == xmtr_pid)
				status = US_WEGOTIT;
			else if (line_lock_status(line) == LINST_WEGOTIT)
				status = US_WEGOTIT;
		}
		else if (!kill((CFG_PidType) last_utmp.ut_pid, 0) || (errno != ESRCH))
		{

			/*
			 * The process that "owns" the utmp entry (hence the line) is
			 * not getty mor login and is alive and presumably well; if it
			 * is WE who own it (for historical reasons, this was
			 * possible), then say so, else we cannot have the line.
			 * 
			 * We assume it is a dial in call since no DIALOUT status was
			 * posted; even if the assumption is wrong and the user is
			 * confused, the line remains protected
			 */
			status = (last_utmp.ut_pid == xmtr_pid) ? US_WEGOTIT : US_LOGGEDIN;
		}
	}

#if defined(UTMP_DEBUG)
	if (status == US_NOTFOUND)
		sprintf(s512, "UTMP %s: no entry in utmp", line);
	else
	{
		sprintf(s512, "UTMP %s:%s:%s:%d:%s",
			namecopy, idcopy, linecopy, last_utmp.ut_pid, US_text(status));
	}
#ifdef INIT_PROCESS
	sprintf(s512 + strlen(s512), ":%s", ut_type_text(last_utmp.ut_type));
#endif /* INIT_PROCESS */

	logevent(getpid(), s512);
#endif /* UTMP_DEBUG */

	close(ufd);
	return (status);
#endif /* sun */

}							 /* end of utmp_status */

/* vi: set tabstop=4 shiftwidth=4: */
/* end of utmpstat.c */
