/*+-------------------------------------------------------------------------
	utmpstatus.h
	wht@wht.net
--------------------------------------------------------------------------*/
/*+:EDITS:*/
/*:04-26-2000-11:16-wht@bob-RELEASE 4.42 */
/*:01-24-1997-02:38-wht@yuriatin-SOURCE RELEASE 4.00 */
/*:09-11-1996-20:01-wht@yuriatin-3.48-major telnet,curses,structural overhaul */
/*:11-23-1995-11:20-wht@kepler-source control 3.37 for tsx-11 */
/*:11-14-1995-10:23-wht@kepler-3.37.80-source control point: SOCKETS */
/*:11-03-1995-17:34-wht@wwtp1-add utmp_status func decl */
/*:05-04-1994-04:40-wht@n4hgf-ECU release 3.30 */
/*:05-29-1993-19:59-wht@n4hgf-rename enum us to enum utmp_status */
/*:09-10-1992-14:00-wht@n4hgf-ECU release 3.20 */
/*:08-22-1992-15:39-wht@n4hgf-ECU release 3.20 BETA */
/*:03-27-1992-16:21-wht@n4hgf-re-include protection for all .h files */
/*:08-10-1991-17:19-wht@n4hgf-add US_WEGOTIT */
/*:07-25-1991-12:59-wht@n4hgf-ECU release 3.10 */
/*:08-14-1990-20:40-wht@n4hgf-ecu3.00-flush old edit history */

#ifndef _utmpstatus_h
#define _utmpstatus_h

/* utmp_status defines */
enum utmp_status
{
	US_UNDEF = 0,			 /* undefined */
	US_NOTFOUND = 100,		 /* not in utmp, or getty dead */
	US_LOGIN,				 /* enabled for login, idle */
	US_LOGGEDIN,			 /* enabled for login, in use */
	US_DIALOUT,				 /* enabled for login, currently dialout */
	US_WEGOTIT				 /* we own the line */
};

enum utmp_status utmp_status();

#endif /* _utmpstatus_h */

/* vi: set tabstop=4 shiftwidth=4: */
/* end of utmpstatus.h */
