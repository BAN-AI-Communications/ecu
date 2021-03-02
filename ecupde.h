/*+-----------------------------------------------------------------------
	ecupde.h - phone directory entry definition
	wht@wht.net
------------------------------------------------------------------------*/
/*+:EDITS:*/
/*:04-26-2000-11:15-wht@bob-RELEASE 4.42 */
/*:01-24-1997-02:37-wht@yuriatin-SOURCE RELEASE 4.00 */
/*:09-22-1996-19:41-root@yuriatin-tty len to 20 for such as port/m332_c0d0 */
/*:09-11-1996-20:00-wht@yuriatin-3.48-major telnet,curses,structural overhaul */
/*:11-23-1995-11:20-wht@kepler-source control 3.37 for tsx-11 */
/*:11-14-1995-10:23-wht@kepler-3.37.80-source control point: SOCKETS */
/*:08-27-1995-07:43-wht@n4hgf-add rtscts_val */
/*:05-04-1994-04:38-wht@n4hgf-ECU release 3.30 */
/*:09-10-1992-13:58-wht@n4hgf-ECU release 3.20 */
/*:08-22-1992-15:38-wht@n4hgf-ECU release 3.20 BETA */
/*:03-27-1992-16:21-wht@n4hgf-re-include protection for all .h files */
/*:11-28-1991-14:56-wht@n4hgf-add dcdwatch */
/*:07-25-1991-12:56-wht@n4hgf-ECU release 3.10 */
/*:06-01-1991-23:53-wht@n4hgf-use PDE_..._LEN identifiers */
/*:08-14-1990-20:40-wht@n4hgf-ecu3.00-flush old edit history */

#ifndef _ecupde_h
#define _ecupde_h

#define PDE_TTY_LEN			20
#define PDE_DESCR_LEN		40

#ifdef futures
#define PDE_FC_NONE			0/* no FAS: none   FAS: RTS/CTS */
#define PDE_FC_XON_XOFF		1/* XON/XOFF */
#define PDE_FC_RTS_CTS		2/* no FAS: SCO RTS/CTS */
#endif

typedef struct phone_directory_entry
{
	struct phone_directory_entry *next;
	struct phone_directory_entry *prev;
	UINT baud;
	short parity;			 /* 0,'e','o', maybe 'm','s' */
	UINT16 redial;			 /* if non-zero, marked for redial */
	char logical[LOGICAL_LEN + 1];	/* logical name of remote */
	char telno[DESTREF_LEN + 1];	/* telno for remote or null */
	char tty[PDE_TTY_LEN + 1];	/* ttyname for access */
	char descr[PDE_DESCR_LEN + 1];	/* description of remote */
#ifdef futures
	UINT zwindw_size;		 /* default ZMODEM window size */
	uchar flow_control;		 /* PDE_FC_... flow control */
#endif
	uchar debug_level;		 /* -x debug level for dialer */
	uchar dcdwatch;			 /* how to set shm->Ldcdwatch on successful
							  * connect '0': off '1': on 't': terminate
							  * 'n': no change */
	uchar rtscts_val;		 /* how to set shm->Lrtscts_val */
}
PDE;

#endif /* _ecupde_h */

/* vi: set tabstop=4 shiftwidth=4: */
/* end of ecupde.h */
