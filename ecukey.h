/*+-------------------------------------------------------------------------
	ecukey.h -- single key (ASCII) defines
	wht@wht.net
--------------------------------------------------------------------------*/
/*+:EDITS:*/
/*:04-26-2000-11:15-wht@bob-RELEASE 4.42 */
/*:01-24-1997-02:37-wht@yuriatin-SOURCE RELEASE 4.00 */
/*:09-11-1996-20:00-wht@yuriatin-3.48-major telnet,curses,structural overhaul */
/*:11-23-1995-11:20-wht@kepler-source control 3.37 for tsx-11 */
/*:11-14-1995-10:23-wht@kepler-3.37.80-source control point: SOCKETS */
/*:05-11-1995-15:01-wht@n4hgf-add CTL_F */
/*:05-04-1994-04:38-wht@n4hgf-ECU release 3.30 */
/*:09-10-1992-13:58-wht@n4hgf-ECU release 3.20 */
/*:08-22-1992-15:38-wht@n4hgf-ECU release 3.20 BETA */
/*:03-27-1992-16:21-wht@n4hgf-re-include protection for all .h files */
/*:07-25-1991-12:56-wht@n4hgf-ECU release 3.10 */
/*:08-14-1990-20:40-wht@n4hgf-ecu3.00-flush old edit history */

#ifndef _ecukey_h
#define _ecukey_h

#define CTL_B       0x02
#define CTL_C       0x03
#define CTL_D       0x04
#define ENQ         0x05
#define CTL_F       0x06
#define ACK         0x06
#define BEL         0x07
#define BS          0x08
#define NL          0x0A
#define TAB         0x09
#define CTL_L       0x0C
#define CRET        0x0D	 /* @#$#*& termcap curses uses CR as pointer */
#define XON         0x11
#define CTL_R       0x12
#define XOFF        0x13
#define CTL_U       0x15
#define SUB			0x18
#define ESC         0x1B
#define CTL_BSLASH  0x1C
#define CTL_Z       0x1A
#define SPACE       0x20
#define DEL         0x7F

#endif /* _ecukey_h */

/* vi: set tabstop=4 shiftwidth=4: */
/* end of ecukey.h */
