/* CHK=0x9A80 */
/*+-------------------------------------------------------------------------
	mkdirs.c - make multiple directories
	wht@wht.net

  Defined functions:
	make_dirs(pathname)
	mkdir(dpath, dmode)

  XENIX lacks mkdir() so use elegant PD version by John Gilmore

--------------------------------------------------------------------------*/
/*+:EDITS:*/
/*:04-26-2000-11:16-wht@bob-RELEASE 4.42 */
/*:01-24-1997-02:37-wht@yuriatin-SOURCE RELEASE 4.00 */
/*:09-11-1996-20:00-wht@yuriatin-3.48-major telnet,curses,structural overhaul */
/*:11-23-1995-11:20-wht@kepler-source control 3.37 for tsx-11 */
/*:11-14-1995-10:23-wht@kepler-3.37.80-source control point: SOCKETS */
/*:05-04-1994-04:39-wht@n4hgf-ECU release 3.30 */
/*:08-06-1993-21:25-wht@n4hgf-add mkdir_auto */
/*:09-10-1992-13:59-wht@n4hgf-ECU release 3.20 */
/*:08-22-1992-15:39-wht@n4hgf-ECU release 3.20 BETA */
/*:08-25-1991-14:21-wht@n4hgf-XENIX code hinges on M_XENIX not !sun&&!isc etc */
/*:08-09-1991-00:30-wht@n4hgf-no need for sys/wait.h + XENIX doesn't have it */
/*:08-06-1991-02:37-root@n4hgf-how did compile succeed without signal.h? */
/*:07-25-1991-12:58-wht@n4hgf-ECU release 3.10 */
/*:07-15-1991-14:20-wht@n4hgf-creation */

#include <string.h>
#include <errno.h>
#include "ecu_types.h"
#include "ecu_stat.h"

extern int errno;

#ifdef M_UNIX
#undef M_XENIX
#endif

#if defined(M_XENIX)
#include <signal.h>
#endif

/*+-------------------------------------------------------------------------
	mkdir(dpath,dmode)

 Directory-creating routines from Public Domain TAR by John Gilmore
 Make a directory.  Compatible with the mkdir() system call on 4.2BSD.
--------------------------------------------------------------------------*/
#if defined(M_XENIX)
#define	TERM_SIGNAL(status)		((status) & 0x7F)
#define TERM_COREDUMP(status)	(((status) & 0x80) != 0)
#define TERM_VALUE(status)		((status) >> 8)
mkdir(dpath, dmode)
char *dpath;
int dmode;
{
	int cpid, status;
	struct stat statbuf;

	x(*original_sighdlr) ();

	if (stat(dpath, &statbuf) == 0)
	{
		errno = EEXIST;		 /* Stat worked,so it already exists */
		return (-1);
	}

	/* If stat fails for a reason other than non-existence,return error */
	if (errno != ENOENT)
		return (-1);

	original_sighdlr = signal(SIGCLD, SIG_DFL);
	switch (cpid = smart_fork())
	{

		case -1:			 /* Error in fork() */
			return (-1);	 /* Errno is set already */

		case 0:			 /* Child process */

			/*
			 * Cheap hack to set mode of new directory.  Since this child
			 * process is going away anyway,we zap its umask. FIXME,this
			 * won't suffice to set SUID,SGID,etc. on this directory. Does
			 * anybody care?
			 */
			status = umask(0);	/* Get current umask */
			status = umask(status | (0777 & ~dmode));	/* Set for mkdir */
			execl("/bin/mkdir", "mkdir", dpath, (char *)0);
			_exit(-1);		 /* Can't exec /bin/mkdir */

		default:			 /* Parent process */
			while ((cpid != wait(&status)) && (cpid != -1))
				;			 /* Wait for kid to finish */
	}

	signal(SIGCLD, original_sighdlr);

	if (TERM_SIGNAL(status) != 0 || TERM_VALUE(status) != 0)
	{
		errno = EIO;		 /* We don't know why,but */
		return (-1);		 /* /bin/mkdir failed */
	}

	return (0);
}							 /* end of mkdir */
#endif

/*+-------------------------------------------------------------------------
	make_dirs(pathname)

  Directory-creating routines from Public Domain TAR by John Gilmore
  After a file/link/symlink/dir creation has failed, see if it's because
  some required directory was not present, and if so, create all
  required dirs.

  returns 0 if no directory made, else # levels required to get target
--------------------------------------------------------------------------*/
int
make_dirs(pathname)
char *pathname;
{
	char *p;				 /* Points into path */
	int madeone = 0;		 /* Did we do anything yet? */
	int save_errno = errno;	 /* Remember caller's errno */
	struct stat fst;

	if (errno != ENOENT)
		return (0);			 /* Not our problem */

	for (p = strchr(pathname, '/'); p; p = strchr(p + 1, '/'))
	{
		/* Avoid mkdir of empty string,if leading or double '/' */
		if (p == pathname || p[-1] == '/')
			continue;
		/* Avoid mkdir where last part of path is '.' */
		if (p[-1] == '.' && (p == pathname + 1 || p[-2] == '/'))
			continue;
		*p = 0;				 /* Truncate the path there */
		if (!stat(pathname, &fst))
		{
			if ((fst.st_mode & S_IFMT) == S_IFDIR)
			{
				*p = '/';
				continue;
			}
#ifdef S_IFLNK
			if (((fst.st_mode & S_IFMT) == S_IFLNK) && !lstat(pathname, &fst))
			{
				if ((fst.st_mode & S_IFMT) == S_IFDIR)
				{
					*p = '/';
					continue;
				}
			}
#endif /* S_IFLNK */
			errno = ENOTDIR;
			return (0);
		}

		if (!mkdir(pathname, 0777))
		{					 /* Try to create it as a dir */
			madeone++;		 /* Remember if we made one */
			*p = '/';
			continue;
		}
		*p = '/';
		if (errno == EEXIST) /* Directory already exists */
			continue;

		/*
		 * Some other error in the mkdir.  We return to the caller.
		 */
		break;
	}
	errno = save_errno;		 /* Restore caller's errno */
	return (madeone);		 /* Tell them to retry if we made one */

}							 /* end of make_dirs */

/* vi: set tabstop=4 shiftwidth=4: */
/* end of mkdirs.c */
