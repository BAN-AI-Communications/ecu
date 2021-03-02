/*+-------------------------------------------------------------------------
	dvent.h - HDB UUCP Devices file entry (a la pwent.h)
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
/*:12-01-1991-12:38-wht@n4hgf-new typedef for striuct dvent */
/*:07-25-1991-12:55-wht@n4hgf-ECU release 3.10 */
/*:08-14-1990-20:39-wht@n4hgf-ecu3.00-flush old edit history */

#ifndef _dvent_h
#define _dvent_h

typedef struct dvent
{
	char *type;				 /* ACU or Direct */
	char *line;				 /* tty name "ttyxx"-style */
	char *dialer;			 /* "801" dialer line */
	UINT low_baud;			 /* lowest bit rate */
	UINT high_baud;			 /* highest bit rate */
	char *dialprog;			 /* dialer program */
	char *token;			 /* token to be passed to the dialer */
}
DVE;

DVE *getdvent();
DVE *getdvbaud();
DVE *getdvline();
DVE *getdvtype();
void enddvent();
DVE *hdb_choose_Any();
DVE *hdb_choose_Device();

#endif /* _dvent_h */

/* vi: set tabstop=4 shiftwidth=4: */
/* end of dvent.h */
