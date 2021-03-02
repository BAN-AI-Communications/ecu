/*+-------------------------------------------------------------------------
	ecuungetty.h
	wht@wht.net
--------------------------------------------------------------------------*/
/*+:EDITS:*/
/*:04-26-2000-11:15-wht@bob-RELEASE 4.42 */
/*:01-24-1997-02:37-wht@yuriatin-SOURCE RELEASE 4.00 */
/*:09-11-1996-20:00-wht@yuriatin-3.48-major telnet,curses,structural overhaul */
/*:11-23-1995-11:20-wht@kepler-source control 3.37 for tsx-11 */
/*:11-14-1995-10:23-wht@kepler-3.37.80-source control point: SOCKETS */
/*:05-04-1994-04:39-wht@n4hgf-ECU release 3.30 */
/*:09-10-1992-13:59-wht@n4hgf-ECU release 3.20 */
/*:08-22-1992-15:38-wht@n4hgf-ECU release 3.20 BETA */
/*:04-27-1992-18:49-wht@n4hgf-ecuungetty grows up to chown ttys */
/*:03-27-1992-16:21-wht@n4hgf-re-include protection for all .h files */
/*:07-25-1991-12:57-wht@n4hgf-ECU release 3.10 */
/*:08-14-1990-20:40-wht@n4hgf-ecu3.00-flush old edit history */

#ifndef _ecuungetty_h
#define _ecuungetty_h

/* ungetty definitions */
#define	UG_NOTENAB		0	 /* on no-switch exec, line not enabled */
#define	UG_RESTART		1	 /* on -t exec, restart needed */
#define	UG_FAIL			2	 /* on no switch exec, line in use */

/* extended ecuungetty codes */
#define UGE_T_LOGIN		200	 /* -t found utmp status US_LOGIN */
#define UGE_T_LOGGEDIN	201	 /* -t found utmp status US_LOGGGEDIN */
#define UGE_T_NOTFOUND	202	 /* not found */
#define UGE_BADARGC		230	 /* usage: bad arg count */
#define UGE_BADSWITCH	231	 /* usage: bad switch */
#define UGE_BADARGV		232	 /* usage: bad argument */
#define UGE_NOTROOT		233	 /* ecuungetty found it had no root privileges */
#define UGE_NOUUCP		234	 /* cannot find uucp passwd entry */
#define UGE_LOGIC		235	 /* logic error */
#define UGE_CALLER		236	 /* caller is not ecu or root */
#define UGE_BOMB		254	 /* ungetty core dumped or killed */
#define UGE_DNE			255	 /* ungetty did not execute */

/* vi: set tabstop=4 shiftwidth=4: */
/* end of ecuungetty.h */

#endif /* _ecuungetty_h */
