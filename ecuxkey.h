/*+-------------------------------------------------------------------------
	ecuxkey.h -- function key single char value mapping
	wht@wht.net

--------------------------------------------------------------------------*/
/*+:EDITS:*/
/*:04-26-2000-11:15-wht@bob-RELEASE 4.42 */
/*:01-24-1997-02:37-wht@yuriatin-SOURCE RELEASE 4.00 */
/*:09-11-1996-20:00-wht@yuriatin-3.48-major telnet,curses,structural overhaul */
/*:11-23-1995-11:20-wht@kepler-source control 3.37 for tsx-11 */
/*:11-14-1995-10:23-wht@kepler-3.37.80-source control point: SOCKETS */
/*:01-12-1995-15:19-wht@n4hgf-apply Andrew Chernov 8-bit clean+FreeBSD patch */
/*:05-04-1994-04:39-wht@n4hgf-ECU release 3.30 */
/*:09-17-1992-05:16-wht@n4hgf-finally, 0xE1-0xFA as promised in 3.10  */
/*:09-10-1992-13:59-wht@n4hgf-ECU release 3.20 */
/*:08-22-1992-15:38-wht@n4hgf-ECU release 3.20 BETA */
/*:03-27-1992-16:21-wht@n4hgf-re-include protection for all .h files */
/*:09-03-1991-23:16-wht@n4hgf2-alt-[a-z] starts w/0xE1: crisp compatibility */
/*:08-28-1991-14:07-wht@n4hgf2-SVR4 cleanup by aega84!lh */
/*:07-25-1991-12:57-wht@n4hgf-ECU release 3.10 */
/*:05-02-1991-01:57-r@n4hgf-alt-[a-z] range moved from 0x80-0x99 to 0xE0-0xF9 */
/*:08-14-1990-20:40-wht@n4hgf-ecu3.00-flush old edit history */

#ifndef _ecuxkey_h
#define _ecuxkey_h

/*
 * these are not changeable: they map to the last character in
 * an AT/"ANSI" function key sequence
 */

#define XFcurup         (0x100 | 'A')
#define XFcurdn         (0x100 | 'B')
#define XFcurrt         (0x100 | 'C')
#define XFcurlf         (0x100 | 'D')
#define XFcur5          (0x100 | 'E')
#define XFend           (0x100 | 'F')
#define XFpgdn          (0x100 | 'G')
#define XFhome          (0x100 | 'H')
#define XFpgup          (0x100 | 'I')
#define XFins           (0x100 | 'L')
#define XF1                     (0x100 | 'M')
#define XF2                     (0x100 | 'N')
#define XF3                     (0x100 | 'O')
#define XF4                     (0x100 | 'P')
#define XF5                     (0x100 | 'Q')
#define XF6                     (0x100 | 'R')
#define XF7                     (0x100 | 'S')
#define XF8                     (0x100 | 'T')
#define XF9                     (0x100 | 'U')
#define XF10            (0x100 | 'V')
#define XF11            (0x100 | 'W')
#define XF12            (0x100 | 'X')
#define XFbktab         (0x100 | 'Z')

/*
 * special codes for non-ANSI keyboard support
 * These are really cleverly disguised magic numbers:
 * they HAVE to have the values used below.
 */
#define XF_no_way       (0x100 | 0xFE)
#define XF_not_yet      (0x100 | 0xFF)

/*
 * extended ALT+[a-z] codes
 */
#define XF_ALTA (0x100 | 0xE1)	/* depends on /usr/lib/keyboard keys ... */
#define XF_ALTZ ((unsigned)((0x100 | 0xE1)+'z'-'a'))	/* ... mapping ALT-a to
														 * 0xE0, etc */

#endif /* _ecuxkey_h */

/* vi: set tabstop=4 shiftwidth=4: */
/* end of ecuxkey.h */
