/*+-------------------------------------------------------------------------
	zrdchk.c - Rdchk clone for CFG_FionreadRdchk
--------------------------------------------------------------------------*/
/*+:EDITS:*/
/*:12-15-1997-19:29-wht@fep-move search for FIONREAD to ecu_config.h */
/*:12-15-1997-19:24-wht@fep-update search for FIONREAD */
/*:01-24-1997-02:38-wht@yuriatin-SOURCE RELEASE 4.00 */
/*:09-11-1996-20:01-wht@yuriatin-3.48-major telnet,curses,structural overhaul */
/*:11-14-1995-10:23-wht@kepler-3.37.80-source control point: SOCKETS */
/*:05-04-1994-04:40-wht@n4hgf-ECU release 3.30 */
/*:01-12-1994-06:02-wht@gyro-creation */

#include "../ecu_types.h"
#include "../ecu_config.h"

/*+-------------------------------------------------------------------------
	Rdchk(f)
--------------------------------------------------------------------------*/
#if defined(CFG_FionreadRdchk)
int
Rdchk(fd)
int fd;
{
	int waiting = 0;

	ioctl(fd, FIONREAD, &waiting);
	return (!!waiting);

}							 /* end of Rdchk */
#endif /* CFG_FionreadRdchk */

/* vi: set tabstop=4 shiftwidth=4: */
/* end of zrdchk.c */
