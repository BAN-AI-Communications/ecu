/*+-------------------------------------------------------------------------
	ecucurses.h - curses? curses? WHICH curses?
	wht@wht.net
--------------------------------------------------------------------------*/
/*+:EDITS:*/
/*:04-26-2000-11:15-wht@bob-RELEASE 4.42 */
/*:03-16-1997-02:36-rll@felton.felton.ca.us-make boxes for SCO Products */
/*:01-24-1997-02:37-wht@yuriatin-SOURCE RELEASE 4.00 */
/*:09-11-1996-20:00-wht@yuriatin-3.48-major telnet,curses,structural overhaul */
/*:09-06-1996-01:55-wht@kepler-process of elimination: CURSES_HEADER_INCLUDED */
/*:09-05-1996-17:44-wht@kepler-DCFG_UseNcursesH optimization */
/*:11-23-1995-11:20-wht@kepler-source control 3.37 for tsx-11 */
/*:11-14-1995-10:23-wht@kepler-3.37.80-source control point: SOCKETS */
/*:01-12-1995-15:19-wht@n4hgf-apply Andrew Chernov 8-bit clean+FreeBSD patch */
/*:05-04-1994-04:38-wht@n4hgf-ECU release 3.30 */
/*:11-12-1993-11:00-wht@n4hgf-Linux changes by bob@vancouver.zadall.com */
/*:09-10-1992-13:58-wht@n4hgf-ECU release 3.20 */
/*:08-22-1992-15:38-wht@n4hgf-ECU release 3.20 BETA */
/*:03-27-1992-16:21-wht@n4hgf-re-include protection for all .h files */
/*:07-25-1991-12:55-wht@n4hgf-ECU release 3.10 */
/*:05-02-1991-02:35-wht@n4hgf-creation */

#ifndef _ecucurses_h
#define _ecucurses_h

#undef CURSES_HEADER_INCLUDED

/*
 * remove any pre-conceived notion of TERMINFO vs. TERMCAP curses (SCO)
 */
#if defined(M_TERMINFO)
#undef M_TERMINFO
#endif /* M_TERMINFO */

#if defined(M_TERMCAP)
#undef M_TERMCAP
#endif /* M_TERMCAP */

#if defined(CFG_UseNcursesH)
#include <ncurses.h>
#define CURSES_HEADER_INCLUDED
#endif

#if defined(CFG_UseNcursesNcursesH)
#include <ncurses/ncurses.h>
#define CURSES_HEADER_INCLUDED
#endif

#if !defined(CURSES_HEADER_INCLUDED)
#include <curses.h>
#define CURSES_HEADER_INCLUDED
#endif

#endif /* _ecucurses_h */

/* vi: set tabstop=4 shiftwidth=4: */
/* end of ecucurses.h */
