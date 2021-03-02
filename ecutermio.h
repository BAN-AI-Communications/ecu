/*+-------------------------------------------------------------------------
	ecutermio.h

 An attempt to mask the differences between posix termios and sysv termio.
 Daniel Harris.
--------------------------------------------------------------------------*/
/*+:EDITS:*/
/*:04-26-2000-11:15-wht@bob-RELEASE 4.42 */
/*:02-26-1998-03:08-wht@kepler-LINUX 2/Redhat */
/*:01-24-1997-02:37-wht@yuriatin-SOURCE RELEASE 4.00 */
/*:09-11-1996-20:00-wht@yuriatin-3.48-major telnet,curses,structural overhaul */
/*:11-23-1995-11:20-wht@kepler-source control 3.37 for tsx-11 */
/*:11-14-1995-10:23-wht@kepler-3.37.80-source control point: SOCKETS */
/*:10-21-1995-12:40-wht@wwtp1-bad dup/edit on TERMIO for termio */
/*:10-15-1995-15:23-wht@calvin-exclude sys/ioctl.h include */
/*:10-15-1995-15:14-wht@jonah-undef NL0, etc before inc termios.h */
/*:04-09-1995-03:10-wht@gyro-fix Solaris NCC et al redef warnings */
/*:05-04-1994-04:39-wht@n4hgf-ECU release 3.30 */
/*:01-23-1994-14:57-wht@n4hgf-fix type in #undef */
/*:01-16-1994-16:48-wht@n4hgf-organize after new ports */
/*:12-18-1993-??:??-daniel@reubio-transplanted into 3.28.06 */
/*:12-15-1993-??:??-daniel@reubio-moved NCC -> NCCS #define here from ecu.h */
/*:11-27-1993-??:??-daniel@reubio-created */

#ifndef _ecutermio_h
#define _ecutermio_h

#if defined(CFG_TermiosLineio)
/* #include <sys/ioctl.h> */

#if defined(sun)
#undef NL0
#undef NL1
#undef CR0
#undef CR1
#undef CR2
#undef CR3
#undef TAB0
#undef TAB1
#undef TAB2
#undef XTABS
#undef BS0
#undef BS1
#undef FF0
#undef FF1
#undef ECHO
#undef NOFLSH
#undef TOSTOP
#undef FLUSHO
#undef PENDIN
#endif /* sun */

#include <termios.h>

#define TERMIO termios

#undef 	NCC
#define	NCC	NCCS

#undef 	TCSETA
#define	TCSETA	TCSANOW

#undef 	TCSETAF
#define	TCSETAF	TCSAFLUSH

#undef 	TCSETAW
#define	TCSETAW	TCSADRAIN

#else /* use termio */
#include <termio.h>

#define TERMIO termio

/* ecutermio.c and it's callers need these  for termio base */

#if !defined(TCOFF)
#define	TCOOFF	0
#endif

#if !defined(TCOON)
#define	TCOON	1
#endif

#if !defined(TCIFLUSH)
#define	TCIFLUSH	0
#endif

#if !defined(TCOFLUSH)
#define	TCOFLUSH	1
#endif

#if !defined(TCIOFLUSH)
#define	TCIOFLUSH	2
#endif

#endif /* CFG_TermiosLineio */

#ifdef sun
#include <sys/ttold.h>
#if defined (SVR4)
#include <sys/ttydev.h>		 /* To pick up EXTA EXTB */
#endif
#endif /* sun */

#if defined(SVR4)
#include <sys/termios.h>
#include <sys/termiox.h>
extern int hx_flag;

#endif

/*
 * Linux has an unbelievable hack for baud values above
 * 38400. ("baud rate" is a redundant, meaningless term).
 *
 * If you use TIOCGSERIAL/TIOCSSERIAL to get the tty's
 * Linux-specific struct serial structure, and set bits
 * in it and plug it back with TIOCSSERIAL, then you
 * can make Linux treat B38400 as 57600 or 115200.
 */
#if defined(linux)
#include <linux/serial.h>
#if defined(ASYNC_SPD_MASK) && !defined(B115200) && defined(TIOCGSERIAL)
#define LINUX_ASYNC_HACK
#endif /* ASYNC_SPD_MASK/B115200/TIOCGSERIAL */
#endif /* linux */

#endif /* _ecutermio_h */

/* vi: set tabstop=4 shiftwidth=4: */
/* end of ecutermio.h */
