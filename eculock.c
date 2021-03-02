/*+-----------------------------------------------------------------------
	eculock.c -- ECU lock file management (creation)
	wht@wht.net

  Defined functions:
	create_lock_file(name)
	lock_tty(line)
	unlock_tty(line)

  This module provides the functions for creating lock files.
  A companion module, ecuLCK.c, has functions for testing lock files.
  This module is included by ECU alone. It uses ecuLCK.c functions.
------------------------------------------------------------------------*/
/*+:EDITS:*/
/*:04-26-2000-11:15-wht@bob-RELEASE 4.42 */
/*:02-09-1997-20:29-wht@yuriatin-harden */
/*:01-24-1997-02:37-wht@yuriatin-SOURCE RELEASE 4.00 */
/*:09-11-1996-20:00-wht@yuriatin-3.48-major telnet,curses,structural overhaul */
/*:08-11-1996-02:10-wht@kepler-rename ecu_log_event to logevent */
/*:12-03-1995-19:57-wht@gyro-use Setuid */
/*:11-23-1995-11:20-wht@kepler-source control 3.37 for tsx-11 */
/*:11-14-1995-10:23-wht@kepler-3.37.80-source control point: SOCKETS */
/*:06-12-1995-15:03-wht@n4hgf-if ecu has uucp euid, make use of it */
/*:05-04-1994-04:38-wht@n4hgf-ECU release 3.30 */
/*:01-25-1994-17:36-wht@n4hgf-rename USE_DECIMAL_PIDS->CFG_BinaryUucpPids */
/*:12-18-1993-18:12-wht@n4hgf-use CFG_BinaryUucpPids in place of HONEYDANBER */
/*:11-14-1993-12:33-wht@n4hgf-HP-UX port by Carl Wuebker at HP */
/*:05-29-1993-21:17-wht@n4hgf-better debug */
/*:09-10-1992-13:58-wht@n4hgf-ECU release 3.20 */
/*:08-22-1992-15:38-wht@n4hgf-ECU release 3.20 BETA */
/*:07-19-1992-22:09-wht@n4hgf-rename check_utmp to reserve_line and move it */
/*:07-19-1992-21:54-wht@n4hgf-lock_tty does not effect ungetty_get anymore */
/*:08-25-1991-14:39-wht@n4hgf-SVR4 port thanks to aega84!lh */
/*:08-10-1991-17:39-wht@n4hgf-US_WEGOTIT handling */
/*:08-09-1991-11:07-wht@n4hgf-configurable lock directory */
/*:07-25-1991-12:56-wht@n4hgf-ECU release 3.10 */
/*:10-16-1990-20:43-wht@n4hgf-add SHARE_DEBUG */
/*:09-19-1990-19:36-wht@n4hgf-logevent now gets pid for log from caller */
/*:08-14-1990-20:40-wht@n4hgf-ecu3.00-flush old edit history */

#include "ecu.h"

extern int errno;
extern char ungetty_ttyname[];

/*+-------------------------------------------------------------------------
	create_lock_file(name)
--------------------------------------------------------------------------*/
int
create_lock_file(name)
char *name;
{
	int fd;
	int rtn_val = -2;		 /* flag if not explicitly set below */
	int pid = getpid();
	char LTMP_fname[64];

#if !defined(CFG_BinaryUucpPids)
	char s32[32];

#endif

	if (setuid_uucp)
		Setuid(uid_uucp);

	errno = 0;
	sprintf(LTMP_fname, "%s/LTMP.%05d", lock_dir_name, pid);
	if ((fd = creat(LTMP_fname, 0444)) < 0)
	{
		if (errno == EACCES)
		{
			strcpy(lopen_err_str, "lock error - try chmod 0777 ");
			strcat(lopen_err_str, lock_dir_name);
		}
		unlink(LTMP_fname);
		rtn_val = -1;
		goto RETURN_STATUS;
	}

#if defined(CFG_BinaryUucpPids)
	write(fd, (char *)&pid, sizeof(int));

#else
	sprintf(s32, "%10d\n", getpid());
	write(fd, s32, 11);
#endif

	chmod(LTMP_fname, 0444); /* some programs seem to think writable lock
							  * file is game for killing */
	close(fd);

	/*
	 * this is the real lock ... if this link does not succeed, then the
	 * file already exists; if the link suceeds, we have locked the device
	 * with no chance of a race
	 */
	errno = 0;
	rtn_val = link(LTMP_fname, name);
	unlink(LTMP_fname);
	chmod(name, 0444);

  RETURN_STATUS:

#if defined(LOCK_DEBUG)
	{
		char s512[512];
		char *cp;

		if (cp = strrchr(name, '/'))
			cp++;
		else
			cp = name;
		sprintf(s512, "CREATE %s %s (%s)", cp,
			(rtn_val) ? "FAIL" : "SUCCEED",
			(rtn_val) ? strerror(errno) : "No error");
		logevent(getpid(), s512);
	}
#endif

	if (setuid_uucp)
		Setuid(uid);

	return (rtn_val);

}							 /* end of create_lock_file */

/*+-------------------------------------------------------------------------
	lock_tty(line) - create lock files for tty line in 'line'

return LINST_OK (0) if locked else LINST_... error
--------------------------------------------------------------------------*/
enum linst
lock_tty(line)
char *line;
{
	enum linst linst = LINST_OK;
	char name[ECU_MAXPN];

	name[0] = 0;
	errno = 0;

	if (linst = make_lock_name(line, name))
		goto FUNC_RETURN;

#if defined(LOCK_DEBUG)
	{
		char s512[512];
		char *cp = base_name(name);

		sprintf(s512, "LOCK TTY REQUEST %s %s %s %s", line, name,
			(linst) ? LINST_text(linst) : "",
			(linst) ? strerror(errno) : "");
		logevent(getpid(), s512);
	}
#endif

	if (create_lock_file(name))
	{
		if (linst = is_active_lock(name))
		{
			if (linst == LINST_WEGOTIT)
			{
				linst = LINST_OK;
				goto FUNC_RETURN;
			}
			ungetty_return_line(line, "lock_tty 1");
			errno = EACCES;	 /* for termecu() */
			goto FUNC_RETURN;
		}
		if (create_lock_file(name))
		{
			ungetty_return_line(line, "lock_tty 2");
			errno = EACCES;	 /* for termecu() */
			{
				linst = LINST_LCKERR;
				goto FUNC_RETURN;
			}
		}
	}

  FUNC_RETURN:

#if defined(LOCK_DEBUG)
	{
		char s512[512];
		char *cp = base_name(name);

		sprintf(s512, "LOCK TTY RESULT %s %s (%s)", cp,
			(linst) ? LINST_text(linst) : "SUCCESS",
			(linst) ? strerror(errno) : "No error");
		logevent(getpid(), s512);
	}
#endif

	return (linst);

}							 /* end of lock_tty */

/*+-----------------------------------------------------------------------
	void unlock_tty(line)
------------------------------------------------------------------------*/
void
unlock_tty(line)
char *line;
{
	char lockname[512];

	if (make_lock_name(line, lockname))
	{
		ff(se, "unlock_tty cannot build lock file name for %s\r\n", line);
		termecu(TERMECU_LOGIC_ERROR);
	}

	ungetty_return_line(line, "unlock_tty");

	if (unlink(lockname) && setuid_uucp)
	{
		Setuid(uid_uucp);
		unlink(lockname);
		Setuid(uid);
	}

}							 /* end of unlock_tty */

/* end of eculock.c */
/* vi: set tabstop=4 shiftwidth=4: */
