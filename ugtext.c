/*+-------------------------------------------------------------------------
	ugtext.c
	wht@atl.ga.us

  Defined functions:
	UG_text(ugstat)

--------------------------------------------------------------------------*/
/*+:EDITS:*/
/*:04-26-2000-11:16-wht@bob-RELEASE 4.42 */
/*:01-24-1997-02:38-wht@yuriatin-SOURCE RELEASE 4.00 */
/*:09-11-1996-20:01-wht@yuriatin-3.48-major telnet,curses,structural overhaul */
/*:11-23-1995-11:20-wht@kepler-source control 3.37 for tsx-11 */
/*:11-14-1995-10:23-wht@kepler-3.37.80-source control point: SOCKETS */
/*:01-12-1995-15:50-wht@n4hgf-remove unnecessary break statements */
/*:05-04-1994-04:40-wht@n4hgf-ECU release 3.30 */
/*:04-17-1994-17:58-wht@n4hgf-creation */

#include "ecuungetty.h"

/*+-------------------------------------------------------------------------
	UG_text(ugstat) - text for ecuungetty code
--------------------------------------------------------------------------*/
char *
UG_text(ugstat)
int ugstat;
{
	static char errant[32];

	switch (ugstat)
	{
		case UG_NOTENAB:
			return ("line not enabled");
		case UG_RESTART:
			return ("restart needed");
		case UG_FAIL:
			return ("line in use");
		case UGE_T_LOGIN:
			return ("-t found US_LOGIN");
		case UGE_T_LOGGEDIN:
			return ("-t found US_LOGGGEDIN");
		case UGE_T_NOTFOUND:
			return ("not found");
		case UGE_BADSWITCH:
			return ("usage: bad switch");
		case UGE_BADARGC:
			return ("usage: bad arg count");
		case UGE_BADARGV:
			return ("this a valid tty??");
		case UGE_NOTROOT:
			return ("not setuid root");
		case UGE_CALLER:
			return ("invalid caller");
		case UGE_NOUUCP:
			return ("cannot find uucp passwd entry");
		case UGE_LOGIC:
			return ("logic error");
		case UGE_BOMB:
			return ("core dumped or killed");
		case UGE_DNE:
			return ("did not execute");
	}
	sprintf(errant, "error %u", ugstat);
	return (errant);
}							 /* end of UG_text */

/* vi: set tabstop=4 shiftwidth=4: */
/* end of ugtext.c */
