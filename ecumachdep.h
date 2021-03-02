/*+-------------------------------------------------------------------------
	ecumachdep.h
--------------------------------------------------------------------------*/
/*+:EDITS:*/
/*:04-26-2000-11:15-wht@bob-RELEASE 4.42 */
/*:01-24-1997-02:37-wht@yuriatin-SOURCE RELEASE 4.00 */
/*:09-11-1996-20:00-wht@yuriatin-3.48-major telnet,curses,structural overhaul */
/*:11-23-1995-11:20-wht@kepler-source control 3.37 for tsx-11 */
/*:11-14-1995-10:23-wht@kepler-3.37.80-source control point: SOCKETS */
/*:05-09-1995-18:33-wht@kepler-include vt/kd headers for linux */
/*:01-12-1995-15:42-wht@n4hgf-apply Bob Bailin machdep.h vs console.h fix */
/*:01-12-1995-15:19-wht@n4hgf-apply Andrew Chernov 8-bit clean+FreeBSD patch */
/*:05-04-1994-04:38-wht@n4hgf-ECU release 3.30 */
/*:01-16-1994-15:47-wht@n4hgf-creation */

#ifndef _ecumachdep_h
#define _ecumachdep_h

#if defined(ISC) && !defined(USE_AT_ANSI)
#define USE_AT_ANSI
#endif

#if defined(ISCSVR4) && !defined(USE_AT_ANSI)
#define USE_AT_ANSI
#endif

#if defined(ESIXSVR4) && !defined(USE_AT_ANSI)
#define USE_AT_ANSI
#endif

#ifdef __FreeBSD__
#include <machine/console.h>
#endif

#ifdef linux
#include <linux/vt.h>
#include <linux/kd.h>
#endif

#if defined(M_SYSV) || defined(SCO32v5)
#if defined(M_UNIX) || defined(SCO32v5)

#include <sys/console.h>
#else
#include <sys/machdep.h>
#endif
/*
 * Thanks for the G2, er GIO_ATTR, to staceyc@sco.COM (Stacey Campbell)
 * GIO_ATTR was not defined in header files as of this writing
 */
#if !defined(GIO_ATTR)
#define GIO_ATTR  ('a' << 8) | 0	/* Ioctl call for current attribute */
#endif
#else
#if defined(USE_AT_ANSI)
#include <sys/at_ansi.h>
#include <sys/kd.h>
#endif /* USE_AT_ANSI */
#endif /* M_SYSV */

#endif /* _ecumachdep_h */

/* vi: set tabstop=4 shiftwidth=4: */
/* end of ecumachdep.h */
