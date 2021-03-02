/*+-------------------------------------------------------------------------
	ttynaming.c - direct/modem tty name management
	wht@wht.net

  Defined functions:
	direct_tty(tty)
	modem_tty(tty)

  For now, meaningful only on SCO.  In the future, perhaps, we'll
  manage an installation-dependent table of what line names refer
  to the same device and which are modem, direct, etc.

  These modules mean even less with the advent of ODT3 and 32v5

--------------------------------------------------------------------------*/
/*+:EDITS:*/
/*:04-26-2000-11:16-wht@bob-RELEASE 4.42 */
/*:01-24-1997-02:38-wht@yuriatin-SOURCE RELEASE 4.00 */
/*:09-11-1996-20:01-wht@yuriatin-3.48-major telnet,curses,structural overhaul */
/*:08-20-1996-12:39-wht@kepler-locale/ctype fixes from ache@nagual.ru */
/*:11-23-1995-11:20-wht@kepler-source control 3.37 for tsx-11 */
/*:11-14-1995-10:23-wht@kepler-3.37.80-source control point: SOCKETS */
/*:06-13-1995-20:40-wht@n4hgf-was returning address of auto stack temp! */
/*:05-04-1994-04:40-wht@n4hgf-ECU release 3.30 */
/*:09-10-1992-14:00-wht@n4hgf-ECU release 3.20 */
/*:08-22-1992-15:39-wht@n4hgf-ECU release 3.20 BETA */
/*:08-21-1992-13:39-wht@n4hgf-rewire direct/modem device use */
/*:08-21-1992-04:47-wht@n4hgf-creation */

#include "ecu.h"

/*+-------------------------------------------------------------------------
	direct_tty(tty) - make non-modem line out of SCO ttyname
--------------------------------------------------------------------------*/
#ifdef NEED_TTY_NAME_CONVERSION
char *
direct_tty(tty)
char *tty;
{
	static char stat_tty[64];

	int itmp;

	strncpy(stat_tty, tty, sizeof(stat_tty));
	stat_tty[sizeof(stat_tty) - 1] = 0;

	if ((itmp = strlen(stat_tty)) > 2)
	{
		itmp--;
		if (
#if 0
			isdigit((uchar) stat_tty[itmp - 1]) &&
#endif
			isupper((uchar) stat_tty[itmp])
			)
		{
			stat_tty[itmp] = tolower((uchar) stat_tty[itmp]);
		}
	}

	return (stat_tty);

}							 /* end of direct_tty */
#endif /* NEED_TTY_NAME_CONVERSION */

/*+-------------------------------------------------------------------------
	modem_tty(tty) - make modem line out of SCO ttyname
--------------------------------------------------------------------------*/
#ifdef NEED_TTY_NAME_CONVERSION
char *
modem_tty(tty)
char *tty;
{
	static char stat_tty[64];

	int itmp;

	strncpy(stat_tty, tty, sizeof(stat_tty));
	stat_tty[sizeof(stat_tty) - 1] = 0;

	if ((itmp = strlen(stat_tty)) > 2)
	{
		itmp--;
		if (
#if 0
			isdigit((uchar) stat_tty[itmp - 1]) &&
#endif
			islower((uchar) stat_tty[itmp])
			)
		{
			stat_tty[itmp] = toupper((uchar) stat_tty[itmp]);
		}
	}

	return (stat_tty);

}							 /* end of modem_tty */
#endif /* NEED_TTY_NAME_CONVERSION */

/* vi: set tabstop=4 shiftwidth=4: */
/* end of ttynaming.c */
