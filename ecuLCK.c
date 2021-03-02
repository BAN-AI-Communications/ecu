#ifdef SVR4
#define USE_SVR4_MKDEV_H	 /* jeff@samantha.chi.il.us SVR4 lock file */
#endif

/*+-----------------------------------------------------------------------
	ecuLCK.c -- ECU lock file management (testing)
	wht@wht.net

  Defined functions:
	LINST_text(linst)
	is_active_lock(name)
	line_lock_status(ttyname)
	lopen_error_reset()
	make_lock_name(ttyname, lock_file_name)

  This module provides the functions for testing lock files.
  It is included by ECU and other utilities (such as ecuungetty).
  A companion module, eculock.c, has functions for creating lock files.

------------------------------------------------------------------------*/
/*+:EDITS:*/
/*:04-26-2000-11:15-wht@bob-RELEASE 4.42 */
/*:01-24-1997-02:37-wht@yuriatin-SOURCE RELEASE 4.00 */
/*:09-11-1996-20:00-wht@yuriatin-3.48-major telnet,curses,structural overhaul */
/*:09-09-1996-03:39-wht@yuriatin-add LINST_TELNETFAIL */
/*:08-11-1996-02:10-wht@kepler-rename ecu_log_event to logevent */
/*:12-06-1995-11:41-wht@n4hgf-revert ODT3 lock files to previous */
/*:12-03-1995-19:57-wht@gyro-use Setuid */
/*:11-23-1995-11:20-wht@kepler-source control 3.37 for tsx-11 */
/*:11-14-1995-10:23-wht@kepler-3.37.80-source control point: SOCKETS */
/*:09-04-1995-18:57-wht@n4hgf-fix broken 32v5 lock file change */
/*:09-01-1995-17:36-wht@n4hgf-32v5 cvts ALL ttynm chars to lc for lock file */
/*:06-12-1995-15:03-wht@n4hgf-if ecu has uucp euid, make use of it */
/*:05-04-1994-04:38-wht@n4hgf-ECU release 3.30 */
/*:01-25-1994-17:36-wht@n4hgf-rename USE_DECIMAL_PIDS->CFG_BinaryUucpPids */
/*:12-18-1993-18:12-wht@n4hgf-use CFG_BinaryUucpPids in place of HONEYDANBER */
/*:11-14-1993-12:33-wht@n4hgf-HP-UX port by Carl Wuebker at HP */
/*:08-18-1993-05:51-wht@n4hgf-cvting to enum linst uncovered gcc bug */
/*:06-12-1993-12:18-wht@n4hgf-put LINST_text here for ecuungetty debug reach */
/*:06-11-1993-17:14-wht@n4hgf-beef up ISLOCK logevent */
/*:10-07-1992-21:09-jeff@samantha.chi.il.us-SVR4 lock file correction */
/*:09-10-1992-13:58-wht@n4hgf-ECU release 3.20 */
/*:08-22-1992-15:38-wht@n4hgf-ECU release 3.20 BETA */
/*:08-21-1992-13:39-wht@n4hgf-rewire direct/modem device use */
/*:04-24-1992-21:59-wht@n4hgf-more SCO tty name normalizing */
/*:08-25-1991-14:39-wht@n4hgf-ISCSVR4 port thanks to aega84!lh */
/*:08-21-1991-03:37-wht@n4hgf-kill LINST_INVALID check */
/*:08-11-1991-18:06-wht@n4hgf-SCO_TTY_NAMING considerations */
/*:08-09-1991-11:07-wht@n4hgf-configurable lock directory */
/*:08-07-1991-14:41-wht@n4hgf-race with ecuungetty over lock resolved */
/*:07-25-1991-12:55-wht@n4hgf-ECU release 3.10 */
/*:11-19-1990-01:05-wht@n4hgf-remove lock in is_active_lock if we locked */
/*:10-16-1990-20:43-wht@n4hgf-add SHARE_DEBUG */
/*:09-19-1990-19:36-wht@n4hgf-logevent now gets pid for log from caller */
/*:08-14-1990-20:39-wht@n4hgf-ecu3.00-flush old edit history */

#include "ecu.h"
#include "utmpstatus.h"

#if defined(SVR4)
#ifdef USE_SVR4_MKDEV_H
#include <sys/mkdev.h>
#else
#include <sys/sysmacros.h>
#endif /* USE_SVR4_MKDEV_H */
#endif

extern int errno;
extern char ungetty_ttyname[];

char *lock_dir_name = CFG_LockDir;	/* location of LCK.. files */

int setuid_uucp;			 /* true if ecu is owned by uucp and chmod +s */
short uid;
short euid;
short uid_uucp;

/*+-------------------------------------------------------------------------
	lopen_error_reset() - clear lopen_err_str
--------------------------------------------------------------------------*/
void
lopen_error_reset()
{
	lopen_err_str[0] = 0;
}							 /* end of lopen_error_reset */

/*+-------------------------------------------------------------------------
	LINST_text(linst) - enum linst to text
--------------------------------------------------------------------------*/
char *
LINST_text(linst)
enum linst linst;
{
	static char linst_s80[80];
	extern uchar last_ugstat;
	char *UG_text();

	if (lopen_err_str[0])
		return (lopen_err_str);

/*
 * The first bug I have found in gcc 2.3.3 (and 2.4.5 is out):
 * Reading specs from /usr/local/lib/gcc-lib/i486-sco3.2v4/2.3.3/specs
 * gcc version 2.3.3
 *
 * if the (int) cast is removed from the following, a large positive
 * integer (a pid) in linst results in "OK" being returned.  Go figure.
 */

	switch ((int)linst)
	{
		case LINST_OK:
			return ("OK");
		case LINST_INVALID:
			return ("invalid line name");
		case LINST_UNKPID:
			return ("unknown pid is using line");
		case LINST_LCKERR:
			return ("error creating lock file");
		case LINST_NODEV:
			return ("line does not exist");
		case LINST_ALREADY:
			return ("line already open!?");
		case LINST_OPNFAIL:
			sprintf(linst_s80, "open error (%-.60s)", strerror(errno));
			return (linst_s80);
		case LINST_ENABLED:
			return ("line enabled for incoming login");
		case LINST_ENABLED_IN_USE:
			return ("line used by incoming login");
		case LINST_DIALOUT_IN_USE:
			return ("line used by another dial out");
		case LINST_NOPTY:
			return ("ptys not supported");
		case LINST_WEGOTIT:
			return ("line already locked by this process");
		case LINST_ECUUNGETTY:
			sprintf(linst_s80, "ecuungetty error (%-.45s)",
				UG_text(last_ugstat));
			return (linst_s80);
		case LINST_ECUUNGETTY2:
			return ("ecuungetty execution error");
		case LINST_NOTCHR:
			return ("not a character special device");
#ifdef CFG_TelnetOption
		case LINST_TELNETFAIL:
			return ("telnet open failed");
#endif
	}
	if (linst > 0)
		sprintf(linst_s80, "pid %d using line", linst);
	else
		sprintf(linst_s80, "unknown line error %d", linst);
	return (linst_s80);

}							 /* end of LINST_text */

/*+-------------------------------------------------------------------------
	make_lock_name(ttyname,lock_file_name)
--------------------------------------------------------------------------*/
enum linst
make_lock_name(ttyname, lock_file_name)
char *ttyname;
char *lock_file_name;
{

#if defined(SVR4)
	struct stat tbuf;

	if (stat(ttyname, &tbuf) < 0)
	{
		if (errno == ENOENT)
			return (LINST_NODEV);	/* device does not exist */
		else
			return (LINST_OPNFAIL);	/* could not access line */
	}
	sprintf(lock_file_name, "%s/LK.%03u.%03u.%03u",
		lock_dir_name, major(tbuf.st_dev),
#ifndef USE_SVR4_MKDEV_H
		tbuf.st_rdev >> 18,
#else
		major(tbuf.st_rdev),
#endif /* USE_SVR4_MKDEV_H */
		minor(tbuf.st_rdev));
#else

	/*
	 * SVR3 and SCO
	 */
	strcpy(lock_file_name, lock_dir_name);
	strcat(lock_file_name, "/LCK..");

#if defined(SCO32v5)

	/*
	 * starting with 32v5 and as early as ODT 3.0, ALL characters of the
	 * tty name are converted to lower case; this takes care of tty names
	 * like tty1A00
	 */
	lock_file_name += strlen(lock_file_name);
	strcpy(lock_file_name, ttyname + 5);
	while (*lock_file_name)
	{
		*lock_file_name = to_lower(*lock_file_name);
		lock_file_name++;
	}
#else
#ifdef SCO_TTY_NAMING

	/*
	 * the last character of the tty name is converted to lower case; that
	 * is, the direct ttyname is used for lock files
	 */
	strcat(lock_file_name, direct_tty(ttyname) + 5);
#else
	strcat(lock_file_name, ttyname + 5);
#endif /* SCO_TTY_NAMING */
#endif /* SCO32v5 */
#endif /* SVR4 */

	return (LINST_OK);
}							 /* end of make_lock_name */

/*+-------------------------------------------------------------------------
	is_active_lock(name) - check to see if lock still active

if not unlink any old lock name
--------------------------------------------------------------------------*/
enum linst
is_active_lock(name)
char *name;
{
	int itmp;
	CFG_PidType lockpid = 0;
	int fd;
	enum linst linst = LINST_OK;
	char pidstr[16];

	errno = 0;
	if ((fd = open(name, O_RDONLY, 0)) < 0)
	{
		if (errno != ENOENT)
			linst = LINST_LCKERR;
		goto RETURN_STATUS;
	}

	errno = 0;
#if defined(CFG_BinaryUucpPids)
	itmp = read(fd, (char *)&lockpid, sizeof(int));

	close(fd);
	if (itmp != sizeof(int))
		    goto UNLINK_OLD_LOCK;

#else
	itmp = read(fd, (char *)pidstr, 11);
	pidstr[11] = 0;
	close(fd);
	if (itmp != 11)
		goto UNLINK_OLD_LOCK;
	lockpid = atoi(pidstr);
#endif

	/* if we are the locker, return no error */
	errno = 0;
	if (lockpid == xmtr_pid)
	{
		linst = LINST_WEGOTIT;
		goto RETURN_STATUS;
	}

	if ((!(itmp = kill(lockpid, 0))) || (errno != ESRCH))
	{
		errno = EACCES;		 /* for termecu() */
		linst = (enum linst)lockpid;
		goto RETURN_STATUS;
	}

  UNLINK_OLD_LOCK:
	errno = 0;
	if (setuid_uucp)
		Setuid(uid_uucp);
	if (unlink(name))
		linst = LINST_LCKERR;
	if (setuid_uucp)
		Setuid(uid);

  RETURN_STATUS:

#if defined(LOCK_DEBUG)
	{
		char s512[512];
		char *cp;

		cp = strrchr(name, '/');
		if (cp)
			cp++;
		else
			cp = name;
		sprintf(s512, "LOCKED? %s: %s", cp,
			(linst != LINST_OK) ? LINST_text(linst) : "NO");
		if (linst)
			sprintf(s512 + strlen(s512), " (%s)", strerror(errno));
		logevent(getpid(), s512);
	}
#endif

	return (linst);
}							 /* end of is_active_lock */

/*+-----------------------------------------------------------------------
	line_lock_status(ttyname)

  ttyname must be of style "/dev/ttyxx"
  Returns locking pid if locked else LOPEN lock error code (< 0) else 0
------------------------------------------------------------------------*/
enum linst
line_lock_status(ttyname)
char *ttyname;
{
	enum linst linst = LINST_OK;
	char lock_file_name[128];

	if (linst = make_lock_name(ttyname, lock_file_name))
		return (linst);

	if (linst = is_active_lock(lock_file_name))
		return (linst);

	return (LINST_OK);

}							 /* end of line_lock_status */

/* end of ecuLCK.c */
/* vi: set tabstop=4 shiftwidth=4: */
