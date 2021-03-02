/*+-------------------------------------------------------------------------
	cmdtbl.c - interactive/procedure command table data
	wht@wht.net

This module merely declares the public instance of the ecu command
table.  DECLARE_P_CMD performs magic in ecucmd.h
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
/*:07-25-1991-12:55-wht@n4hgf-ECU release 3.10 */
/*:03-25-1991-11:40-wht@n4hgf-get termio.h for ecucmd.h conditonal compile */
/*:08-14-1990-20:39-wht@n4hgf-ecu3.00-flush old edit history */

#define DECLARE_P_CMD
#include "ecu.h"
#include "ecucmd.h"

/* vi: set tabstop=4 shiftwidth=4: */
/* end of cmdtbl.c */
