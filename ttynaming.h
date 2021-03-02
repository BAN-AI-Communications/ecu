/*+-------------------------------------------------------------------------
	ttynaming.h -- SCO tty naming decision
	wht@wht.net

  You might want to change this (if you are on SCO, but have
  non-SCO style ttys, but then you have to be careful about
  using upper- versus lower-case tty names in inittab/utmp,
  Devices, dialing directories and interactive usage; some or all
  XENIX implimentations have problems with CLOCAL swapping to
  simulate upper- vs.  lower-case name choices, but that is too
  long a story to go into here :-< ...  you may need to omit
  SCO_TTY_NAMING under XENIX and watch your cases.  There are
  several uses of SCO_TTY_NAMING throughout the code but one
  common use has been localized in the TTYNAME_STRCMP macro
--------------------------------------------------------------------------*/
/*+:EDITS:*/
/*:04-26-2000-11:16-wht@bob-RELEASE 4.42 */
/*:01-24-1997-02:38-wht@yuriatin-SOURCE RELEASE 4.00 */
/*:09-11-1996-20:01-wht@yuriatin-3.48-major telnet,curses,structural overhaul */
/*:11-23-1995-11:20-wht@kepler-source control 3.37 for tsx-11 */
/*:11-14-1995-10:23-wht@kepler-3.37.80-source control point: SOCKETS */
/*:05-04-1994-04:40-wht@n4hgf-ECU release 3.30 */
/*:09-10-1992-14:00-wht@n4hgf-ECU release 3.20 */
/*:08-22-1992-15:39-wht@n4hgf-ECU release 3.20 BETA */
/*:08-21-1992-13:39-wht@n4hgf-rewire direct/modem device use */
/*:08-07-1992-19:03-wht@n4hgf-creation */

#ifndef _ttynaming_h
#define _ttynaming_h

#if (defined(M_SYSV) || defined(SCO32v5)) && !defined(SCO_TTY_NAMING)
#define SCO_TTY_NAMING
#endif

#undef NEED_TTY_NAME_CONVERSION

#ifdef SCO_TTY_NAMING
#define TTYNAME_STRCMP(name1,name2) strcmpi(name1,name2)
#define NEED_TTY_NAME_CONVERSION
char *direct_tty();
char *modem_tty();

#else
#define TTYNAME_STRCMP(name1,name2) strcmp(name1,name2)
#define direct_tty(tty) (tty)
#define modem_tty(tty) (tty)
#endif

#endif /* _ttynaming_h */

/* vi: set tabstop=4 shiftwidth=4: */
/* end of ttynaming.h */
