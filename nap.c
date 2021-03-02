/*+-------------------------------------------------------------------------
	nap.c - nap() support
	wht@wht.net

  Defined functions:
	Nap(msec)
	Ftime(tptr)
	init_Nap()

--------------------------------------------------------------------------*/
/*+:EDITS:*/
/*:04-26-2000-11:16-wht@bob-RELEASE 4.42 */
/*:02-26-1998-03:10-wht@kepler-linux2/redhat */
/*:12-12-1997-21:11-wht@kepler-complete isolation of substituted ftime */
/*:01-24-1997-02:37-wht@yuriatin-SOURCE RELEASE 4.00 */
/*:09-11-1996-20:00-wht@yuriatin-3.48-major telnet,curses,structural overhaul */
/*:11-23-1995-11:20-wht@kepler-source control 3.37 for tsx-11 */
/*:11-14-1995-10:23-wht@kepler-3.37.80-source control point: SOCKETS */
/*:10-18-1995-04:29-wht@kepler-always use select for nap */
/*:05-11-1995-15:55-wht@n4hgf-ck_sigint fools optimizing compilers */
/*:04-11-1995-15:48-wht@n4hgf-pretty work: memset 0 Ftime timeb before plug */
/*:05-04-1994-04:39-wht@n4hgf-ECU release 3.30 */
/*:01-12-1994-07:17-wht@fep-move Ftime() from ecutime.c */
/*:09-10-1992-14:00-wht@n4hgf-ECU release 3.20 */
/*:08-22-1992-15:39-wht@n4hgf-ECU release 3.20 BETA */
/*:08-10-1992-03:04-wht@n4hgf-add init_Nap */
/*:07-17-1992-18:19-wht@n4hgf-creation of common module for all binaries */

#include "ecu.h"

#undef NULL					 /* some stdio and param.h define these
							  * differently */
#include <sys/param.h>
#ifndef NULL				 /* fake usual sys/param.h value */
#define NULL 0
#endif

#if defined(BSD)
#if !defined(HZ)
#define	HZ	100				 /* I don't want to talk about this */
#endif
#endif

int hertz;						 /* HZ from environ or sys/param.h */
UINT32 hzmsec;				 /* clock period in msec rounded up */

/*+-------------------------------------------------------------------------
	Ftime(tptr) - SVR3 ftime() surrogate

  If ftime() not available, fake one with gettimeofday(). Ftime()
  will become less common as SVR3 becomes the less common port;
  This usage should keep the memory of it alive a bit longer <smile>.
  See ecu_time.h for a fake struct TIMEB definition.

--------------------------------------------------------------------------*/
#if defined(CFG_GettodFtime)
void
Ftime(tptr)
struct TIMEB *tptr;
{
	struct timeval tval;
	struct timezone tzval;

	memset((char *)&tval, 0, sizeof(tval));
	gettimeofday(&tval, &tzval);
	memset((char *)tptr, 0, sizeof(*tptr));
	tptr->time = tval.tv_sec;
	tptr->millitm = tval.tv_usec / 1000;

}							 /* end of Ftime */
#endif /* CFG_GettodFtime */

/*+-------------------------------------------------------------------------
	Nap(msec) - system-dependent accurate short sleeps
--------------------------------------------------------------------------*/
long
Nap(msec)
long msec;
{
/* precision guard */
#define SECDELTA 684300000L	 /* sometime in 9/91 */
/*
 * Compute  A -= B for timeb structs A, B (old method thanks to ping.c)
 */
#define tbsub(t, t0) \
    if(1) \
    { \
		int millitm_delta = (t)->millitm; \
		(t)->millitm = (millitm_delta -= (t0)->millitm); \
        (t)->time -= (t0)->time; \
        if(millitm_delta < 0) \
        { \
            (t)->time--; \
            (t)->millitm += 1000; \
        } \
    } else					 /* ; supplied by invoker */

	struct TIMEB timer;
	struct TIMEB start;
	struct TIMEB now;
	struct timeval tval;

	Ftime(&start);
	start.time -= SECDELTA;
	timer.time = msec / 1000;
	timer.millitm = msec % 1000;

	tval.tv_sec = timer.time;
	tval.tv_usec = timer.millitm * 1000;
	errno = 0;
	if (select(0, (SelBitmask *) 0, (SelBitmask *) 0,
			(SelBitmask *) 0, &tval) < 0)
	{
		if (ck_sigint())	 /* if SIGINT posted, exit now */
			return (-1);
	}

	Ftime(&now);
	now.time -= SECDELTA;
	tbsub(&now, &start);
	msec = (now.time * 1000) + now.millitm;
	return (msec);

}							 /* end of Nap */

/*+-------------------------------------------------------------------------
	init_Nap()
--------------------------------------------------------------------------*/
void
init_Nap()
{

	/*
	 * learn tick rate for various timers
	 */
	if (getenv("HZ"))
		hertz = (UINT32) atoi(getenv("HZ"));
	else
		hertz = HZ;
	hzmsec = (UINT32) (1000 / hertz) + 2;
}							 /* end of init_Nap */

/* vi: set tabstop=4 shiftwidth=4: */
/* end of nap.c */
