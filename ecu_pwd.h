/*+-------------------------------------------------------------------------
	ecu_pwd.h
	wht@wht.net

We cannot always guarantee against multiple inclusion because
some old development environments don't protect against it.
We try and do it here ..... 1/2 of my incoming long distance
support calls are because old XENIX systems don't do this.

This is sort of moot since ecu code does not multiply
include <pwd.h> and there is no ubiquitous #define in
<pwd.h> to protect other system headers from including it too
--------------------------------------------------------------------------*/
/*+:EDITS:*/
/*:04-26-2000-11:15-wht@bob-RELEASE 4.42 */
/*:01-24-1997-02:37-wht@yuriatin-SOURCE RELEASE 4.00 */
/*:09-11-1996-20:00-wht@yuriatin-3.48-major telnet,curses,structural overhaul */
/*:11-23-1995-11:20-wht@kepler-source control 3.37 for tsx-11 */
/*:11-14-1995-10:23-wht@kepler-3.37.80-source control point: SOCKETS */
/*:05-04-1994-04:38-wht@n4hgf-ECU release 3.30 */
/*:09-10-1992-13:58-wht@n4hgf-ECU release 3.20 */
/*:08-22-1992-15:38-wht@n4hgf-ECU release 3.20 BETA */
/*:03-27-1992-16:21-wht@n4hgf-re-include protection for all .h files */
/*:01-29-1992-16:49-wht@n4hgf-creation */

#ifndef _ecu_pwd_h
#define _ecu_pwd_h

#include <pwd.h>

#endif /* _ecu_pwd_h */

/* vi: set tabstop=4 shiftwidth=4: */
/* end of ecu_pwd.h */
