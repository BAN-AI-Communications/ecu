/*+-------------------------------------------------------------------------
	ldtelnet.h
--------------------------------------------------------------------------*/
/*+:EDITS:*/
/*:04-26-2000-11:16-wht@bob-RELEASE 4.42 */
/*:01-24-1997-02:37-wht@yuriatin-SOURCE RELEASE 4.00 */
/*:09-11-1996-20:00-wht@yuriatin-3.48-major telnet,curses,structural overhaul */
/*:11-23-1995-11:20-wht@kepler-source control 3.37 for tsx-11 */
/*:11-14-1995-10:23-wht@kepler-3.37.80-source control point: SOCKETS */
/*:11-10-1995-15:17-wht@kepler-add TELOPTs missed by some old guys */
/*:10-14-1993-16:33-wht@gyro-rename telnet_block_... telnet_rbuf_... */
/*:11-28-1992-03:34-wht@gyro-source control point 1.37 */
/*:11-25-1992-17:20-wht@gyro-add extern __hostnm/portnum */
/*:03-16-1992-23:42-wht@n4hgf2-first cut at porting rterm proc language */
/*:03-07-1992-14:20-wht@n4hgf-creation */

#ifndef _ldtelnet_h
#define _ldtelnet_h

#include <arpa/telnet.h>
#include <arpa/inet.h>

#ifndef	TELOPT_TUID
#define	TELOPT_TUID	26	/* TACACS user identification */
#endif

#ifndef	TELOPT_OUTMRK
#define	TELOPT_OUTMRK	27	/* output marking */
#endif

#ifndef	TELOPT_TTYLOC
#define	TELOPT_TTYLOC	28	/* terminal location number */
#endif

#ifndef	TELOPT_3270REGIME
#define	TELOPT_3270REGIME 29	/* 3270 regime */
#endif

#ifndef	TELOPT_X3PAD
#define	TELOPT_X3PAD	30	/* X.3 PAD */
#endif

#ifndef	TELOPT_NAWS
#define	TELOPT_NAWS	31	/* window size */
#endif

#ifndef	TELOPT_TSPEED
#define	TELOPT_TSPEED	32	/* terminal speed */
#endif

#ifndef	TELOPT_LFLOW
#define	TELOPT_LFLOW	33	/* remote flow control */
#endif

#ifndef TELOPT_LINEMODE
#define TELOPT_LINEMODE	34	/* Linemode option */
#endif

#ifndef TELOPT_XDISPLOC
#define TELOPT_XDISPLOC	35	/* X Display Location */
#endif

#ifndef TELOPT_ENVIRON
#define TELOPT_ENVIRON	36	/* Environment variables */
#endif

#ifndef	TELOPT_AUTHENTICATION
#define	TELOPT_AUTHENTICATION 37/* Authenticate */
#endif

#ifndef	TELOPT_ENCRYPT
#define	TELOPT_ENCRYPT	38	/* Encryption option */
#endif

extern char *host_name;	
extern u_long hostaddr;

char *inet_utoa();

#endif			   /* _ldtelnet_h */

/* vi: set tabstop=4 shiftwidth=4: */
/* end of ldtelnet.h */
