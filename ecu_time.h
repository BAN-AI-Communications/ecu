/*+-------------------------------------------------------------------------
	ecu_time.h
	wht@wht.net

  allow gettimeofday to stand in for ftime
--------------------------------------------------------------------------*/
/*+:EDITS:*/
/*:04-26-2000-11:15-wht@bob-RELEASE 4.42 */
/*:12-12-1997-21:11-wht@kepler-complete isolation of substituted ftime */
/*:01-24-1997-02:37-wht@yuriatin-SOURCE RELEASE 4.00 */
/*:09-11-1996-20:00-wht@yuriatin-3.48-major telnet,curses,structural overhaul */
/*:11-23-1995-11:20-wht@kepler-source control 3.37 for tsx-11 */
/*:11-14-1995-10:23-wht@kepler-3.37.80-source control point: SOCKETS */
/*:10-18-1995-04:29-wht@kepler-always use select for nap */
/*:05-04-1994-04:38-wht@n4hgf-ECU release 3.30 */
/*:12-12-1993-13:07-wht@n4hgf-creation */

#ifndef _ecu_time_h
#define _ecu_time_h

/*
 * our own re-inclusion prevention
 */

#if !defined(ECU_INCLUDED_SYS_TIME_H)
#include <sys/time.h>
#define ECU_INCLUDED_SYS_TIME_H
#endif /* ECU_INCLUDED_SYS_TIME_H */

/*
 * if ftime() not available, fake one with gettimeofday()
 * ftime() will become less common as SVR3 becomes
 * the less common port; this usage should keep the
 * memory of it alive a bit longer <smile>
 */
#if defined(CFG_GettodFtime)
#undef TIMEB
struct TIMEB
{
	long time;
	unsigned short millitm;
	short timezone;			 /* not supported by ECU Ftime clone */
	short dstflag;			 /* not supported by ECU Ftime clone */
};

#else /* !CFG_GettodFtime */
#include <sys/timeb.h>		 /* we do have Ftime() */
#undef TIMEB
#define TIMEB timeb
#define Ftime(tb) ftime(tb)
#endif /* CFG_GettodFtime */

/*
 * find a struct timeval for select()
 */
#ifdef CFG_IncSelectH
#include <sys/select.h>
#else /******* per 72027.3605compuserve.com */
#if !defined(ECU_INCLUDED_SYS_TIME_H)
#include <sys/time.h>
#define ECU_INCLUDED_SYS_TIME_H
#endif /* ECU_INCLUDED_SYS_TIME_H */
#endif /* 0 */

/*
 * <time.h> must be included after <sys/time.h> or later
 * SCO 32v4 DS will find the prototype definition for Ftime()
 * before it finds the struc declaration, thus barfing
 * with bad argument complaints when Ftime() is later used
 */
#include <time.h>

#endif /* _ecu_time_h */

/* vi: set tabstop=4 shiftwidth=4: */
/* end of ecu_time.h */
