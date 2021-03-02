/*+-------------------------------------------------------------------------
	dialprog.h - HDB UUCP dialer program return code error codes
	wht@wht.net
--------------------------------------------------------------------------*/
/*+:EDITS:*/
/*:04-26-2000-11:15-wht@bob-RELEASE 4.42 */
/*:01-24-1997-02:36-wht@yuriatin-SOURCE RELEASE 4.00 */
/*:09-11-1996-19:59-wht@yuriatin-3.48-major telnet,curses,structural overhaul */
/*:11-23-1995-11:20-wht@kepler-source control 3.37 for tsx-11 */
/*:11-14-1995-10:23-wht@kepler-3.37.80-source control point: SOCKETS */
/*:05-04-1994-04:38-wht@n4hgf-ECU release 3.30 */
/*:09-10-1992-13:58-wht@n4hgf-ECU release 3.20 */
/*:08-22-1992-15:38-wht@n4hgf-ECU release 3.20 BETA */
/*:03-27-1992-16:21-wht@n4hgf-re-include protection for all .h files */
/*:07-25-1991-12:55-wht@n4hgf-ECU release 3.10 */
/*:08-14-1990-20:39-wht@n4hgf-ecu3.00-flush old edit history */

#ifndef _dialprog_h
#define _dialprog_h

/*  Return code masks:
 *            0x80    bit = 1 if connection failed
 *            0x10    bit = 1 if line is also used for dialin #ifndef HDUU
 *            0x0f    if msb=1: error code
 *                    if msb=0: connected bit rate (0=same as dialed baud)
 */
/* return codes: these are set up so that an abort signal at any time can */
/* set the fail bit and return to the caller with the correct status */
#define RC_BAUD     0x0f	 /* CBAUD connected at (0=same as dialed
							  * speed) */
#define RC_ENABLED  0x10	 /* enabled flag: 1 = ungetty -r required to
							  * restore the line */
#define	RC_FAIL		0x80	 /* 1 = failed to connect */
#define	RCE_NULL	0		 /* general purpose or unknown error code */
#define	RCE_INUSE	1		 /* line in use */
#define	RCE_SIG		2		 /* signal aborted dialer */
#define	RCE_ARGS	3		 /* invalid arguments */
#define	RCE_PHNO	4		 /* invalid phone number */
#define	RCE_SPEED	5		 /* invalid bit rate -or- bad connect baud */
#define	RCE_OPEN	6		 /* can't open line */
#define	RCE_IOCTL	7		 /* ioctl error */
#define	RCE_TIMOUT	8		 /* timeout */
#define	RCE_NOTONE	9		 /* no dial tone */
#define	RCE_BUSY	13		 /* phone is busy */
#define	RCE_NOCARR	14		 /* no carrier */
#define	RCE_ANSWER	15		 /* no answer */

#endif /* _dialprog_h */

/* vi: set tabstop=4 shiftwidth=4: */
/* end of dialprog.h */
