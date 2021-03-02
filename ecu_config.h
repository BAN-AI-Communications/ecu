/*+-------------------------------------------------------------------------
	ecu_config.h
--------------------------------------------------------------------------*/
/*+:EDITS:*/
/*:04-26-2000-11:15-wht@bob-RELEASE 4.42 */
/*:02-26-1998-01:11-wht@menlo-SelBitMask doings moved here */
/*:02-26-1998-01:10-wht@menlo-use PATH_MAX if defined */
/*:12-15-1997-19:34-wht@fep-rename CFG_FilioH to CFG_FionreadInFilioH */
/*:01-24-1997-02:37-wht@yuriatin-SOURCE RELEASE 4.00 */
/*:09-16-1996-07:27-wht@yuriatin-add SCO_UNIX */
/*:09-11-1996-20:00-wht@yuriatin-3.48-major telnet,curses,structural overhaul */
/*:12-03-1995-20:01-wht@gyro-add Setuid */
/*:11-23-1995-11:20-wht@kepler-source control 3.37 for tsx-11 */
/*:11-14-1995-10:23-wht@kepler-3.37.80-source control point: SOCKETS */
/*:03-12-1995-00:59-wht@kepler-ECU_MAXPN */
/*:01-12-1995-15:19-wht@n4hgf-apply Andrew Chernov 8-bit clean+FreeBSD patch */
/*:05-04-1994-04:38-wht@n4hgf-ECU release 3.30 */
/*:01-16-1994-16:55-wht@n4hgf-added framework */

#ifndef _ecu_config_h
#define _ecu_config_h

/*
 * config.c, compiled without benefit of it's own wizardry
 * and it's user input, must know how to use the tty
 */
#if (defined(__NetBSD__) || defined(__FreeBSD__)) && !defined(CFG_TermiosLineio)
#define CFG_TermiosLineio
#endif

#if defined(M_SYSV) || defined(SCO32v5)
#define SCO_UNIX
#endif

/*
 * ANSI dweebery + ensure defined with '1', not just defined
 */
#if defined(__sun__) && !defined(sun)
#define sun 1
#endif
#if defined(__i386__) || defined(i386)
#undef i386
#define i386 1
#endif
#if defined(__SVR4__) || defined(SVR4)
#undef SVR4
#define SVR4 1
#endif
#if defined(hp9000s300) || defined(hppa) || defined(hpux) || defined(__hpux__)
#undef hpux
#define hpux 1
#endif
#if defined(__NetBSD__)
#undef netbsd
#define	netbsd 1
#endif

#if defined(PATH_MAX)
#define ECU_MAXPN PATH_MAX		 /* max pathname length */
#else
#define ECU_MAXPN 1024		 /* max pathname length */
#endif

/*
 * setuid() vs. seteuid
 */
#if defined(CFG_UseSeteuid)
#define Setuid(uid) seteuid(uid)
#else
#define Setuid(uid) setuid(uid)
#endif

/*
 * the search for FIONREAD
 */

#if defined(CFG_FionreadRdchk)

#ifdef CFG_FionreadInFilioH
#include <sys/filio.h>
#else
#ifdef CFG_FionreadInSocketH
#include <sys/socket.h>
#else
#include <sys/ioctl.h>
#endif
#endif

#else /* dev-sys supplied rdchk() */
#define Rdchk(val) rdchk(val)
#endif /* CFG_FionreadRdchk */

#if defined(CFG_NeedStdlibH)
#include <stdlib.h>
#else
char *sbrk(); /* go figure */
#endif

#ifdef CFG_HasFdSet
#define SelBitmask CFG_FDSET
#else
#define SelBitmask int
#endif /* CFG_HasFdSet */

#endif /* _ecu_config_h */

#if defined(WHT) && defined(CFG_TelnetOption) && !defined(CFG_TelnetServer)
#define CFG_TelnetServer
#endif

/* vi: set tabstop=4 shiftwidth=4: */
/* end of ecu_config.h */
