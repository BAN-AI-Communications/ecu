
/*+-------------------------------------------------------------------------
	baudtest.c
	wht@wht.net

Alas, on some systems, curses insists on sgtty.h inclusion
which does not get along with termio.h AT ALL
--------------------------------------------------------------------------*/
/*+:EDITS:*/
/*:01-24-1997-02:38-wht@yuriatin-SOURCE RELEASE 4.00 */
/*:09-11-1996-20:01-wht@yuriatin-3.48-major telnet,curses,structural overhaul */
/*:11-14-1995-10:23-wht@kepler-3.37.80-source control point: SOCKETS */
/*:10-14-1995-17:08-wht@kepler-'struct termio' to 'struct TERMIO' */
/*:05-04-1994-04:40-wht@n4hgf-ECU release 3.30 */
/*:09-10-1992-14:00-wht@n4hgf-ECU release 3.20 */
/*:08-22-1992-15:39-wht@n4hgf-ECU release 3.20 BETA */
/*:08-28-1991-14:08-wht@n4hgf2-SVR4 cleanup by aega84!lh */
/*:08-23-1991-18:33-wht@n4hgf2-disable force no curses for tty vs. line speed */
/*:07-25-1991-12:59-wht@n4hgf-ECU release 3.10 */
/*:12-04-1990-05:36-wht-creation */

#include <stdio.h>
#include "../ecu_types.h"
#include "../ecutermio.h"

extern int iofd;
extern int dumbtty;
extern int report_verbosity;
extern int report_init_complete;
extern char *numeric_revision;
extern unsigned long Bitrate;

/*+-------------------------------------------------------------------------
	test_tty_and_line_bitrate()

  if non-multiscreen tty bit rate not at least that
  of the attached line, use no curses, but do be a bit more
  verbose than if tty not char special

--------------------------------------------------------------------------*/
#ifdef TTY_VS_LINE_SPEED_NO_CURSES
void
test_tty_and_line_bitrate()
{
	struct TERMIO tty_termio;

	memset((char *)&tty_termio, 0, sizeof(struct TERMIO));

	if (ecugetattr(0, &tty_termio) ||
		((unsigned long)ecugetspeed(&tty_termio) < Bitrate))
	{
		dumbtty = 1;
		report_verbosity = 1;
		report_init_complete = 1;
	}

}							 /* end of test_tty_and_line_bitrate */
#endif

/* vi: set tabstop=4 shiftwidth=4: */
/* end of baudtest.c */
