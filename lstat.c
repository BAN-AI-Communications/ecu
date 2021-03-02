/*+-------------------------------------------------------------------------
	lstat.c - SCO 3.2v4OS with 3.2v2DS interim hack
	wht@wht.net

  Defined functions:
	lstat(path, statptr)

  Using 3.2v2 DS with 3.2v4 is wrought with troubles, but some
  ain't ready to go the trip yet, so here is a piece of
  projectile vomitus

--------------------------------------------------------------------------*/
/*+:EDITS:*/
/*:04-26-2000-11:16-wht@bob-RELEASE 4.42 */
/*:01-24-1997-02:37-wht@yuriatin-SOURCE RELEASE 4.00 */
/*:09-11-1996-20:00-wht@yuriatin-3.48-major telnet,curses,structural overhaul */
/*:11-23-1995-11:20-wht@kepler-source control 3.37 for tsx-11 */
/*:11-14-1995-10:23-wht@kepler-3.37.80-source control point: SOCKETS */
/*:06-05-1995-02:19-wht@n4hgf-decommit - problems with post 3.2v4 versions */
/*:05-04-1994-04:39-wht@n4hgf-ECU release 3.30 */
/*:04-16-1994-14:58-wht@n4hgf-get rid of ODT3 prototype warning */
/*:11-23-1993-11:15-sue@sbg486-some 3.2v4.1 DS do not have lstat */
/*:09-10-1992-13:59-wht@n4hgf-ECU release 3.20 */
/*:09-09-1992-06:05-wht@n4hgf-creation */

#if 000000000000000000

#include <sys/types.h>
#include <sys/stat.h>

#if defined(M_UNIX) && defined(S_IFLNK)

#include <errno.h>

#ifdef SCO32v4
#define _Const const
#else
#define _Const
#endif

/*+-------------------------------------------------------------------------
	lstat(path,statptr)
--------------------------------------------------------------------------*/
int
lstat(path, statptr)
_Const char *path;
struct stat *statptr;
{
	int err = EINVAL;

	/*
	 * try lstat system call first if it fails with EINVAL, we are not on
	 * 3.2v4
	 */
	if (err = syscall(0x5b, path, statptr) && (err == EINVAL))
		err = stat(path, statptr);
	return (err);

}							 /* end of lstat */

#endif /* if defined(M_UNIX) && defined(S_IFLNK) */

#endif /* 000000000000000000 */

/* vi: set tabstop=4 shiftwidth=4: */
/* end of lstat.c */
