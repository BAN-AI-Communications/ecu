/*+-------------------------------------------------------------------------
	pc_scr.h - PC/AT screen definitions
	wht@wht.net
--------------------------------------------------------------------------*/
/*+:EDITS:*/
/*:04-26-2000-11:16-wht@bob-RELEASE 4.42 */
/*:03-16-1997-02:36-rll@felton.felton.ca.us-make boxes for SCO Products */
/*:01-24-1997-02:37-wht@yuriatin-SOURCE RELEASE 4.00 */
/*:09-11-1996-20:00-wht@yuriatin-3.48-major telnet,curses,structural overhaul */
/*:09-07-1996-16:39-wht@kepler-capitalize on WINCH */
/*:09-07-1996-16:18-wht@kepler-CFG_UseACS coined for use outside of config */
/*:11-23-1995-11:20-wht@kepler-source control 3.37 for tsx-11 */
/*:11-14-1995-10:23-wht@kepler-3.37.80-source control point: SOCKETS */
/*:01-12-1995-15:20-wht@n4hgf-apply Andrew Chernov 8-bit clean+FreeBSD patch */
/*:05-04-1994-04:39-wht@n4hgf-ECU release 3.30 */
/*:11-12-1993-11:00-wht@n4hgf-Linux changes by bob@vancouver.zadall.com */
/*:09-10-1992-14:00-wht@n4hgf-ECU release 3.20 */
/*:08-22-1992-15:39-wht@n4hgf-ECU release 3.20 BETA */
/*:03-27-1992-16:21-wht@n4hgf-re-include protection for all .h files */
/*:07-25-1991-12:58-wht@n4hgf-ECU release 3.10 */
/*:12-04-1990-02:48-wht@n4hgf-for nonansi terminals provide either ruling set */
/*:08-14-1990-20:40-wht@n4hgf-ecu3.00-flush old edit history */

#ifndef _pc_scr_h
#define _pc_scr_h

/*
 * AT ROM ruling characters
 */
#define at_TL		0xDA	 /* top left single rule */
#define at_TR		0xBF	 /* top right single rule */
#define at_BL		0xC0	 /* bottom left single rule */
#define at_BR		0xD9	 /* bottom right single rule */
#define at_LT		0xC3	 /* left hand T */
#define at_RT		0xB4	 /* right hand T */
#define at_VR		0xB3	 /* vertical rule */
#define at_HR		0xC4	 /* horizontal rule */

/*
 * vanilla screen ruling
 */
#define vanilla_TL		'.'
#define vanilla_TR		'.'
#define vanilla_BL		'`'
#define vanilla_BR		'\''
#define vanilla_LT		'+'
#define vanilla_RT		'+'
#define vanilla_VR		'|'
#define vanilla_HR		'-'

/*
 * we either use the ACS_... character definitions
 * or we don't
 */
#undef CFG_UseACS
#if defined(linux) || defined(__FreeBSD__) || defined(SCO32v4) || defined(SVR4)
#define CFG_UseACS
#endif

#if defined(CFG_UseACS)
typedef chtype WINCH;
#else
typedef unsigned char WINCH;
#endif

extern WINCH sTL;
extern WINCH sTR;
extern WINCH sBL;
extern WINCH sBR;
extern WINCH sLT;
extern WINCH sRT;
extern WINCH sVR;
extern WINCH sHR;

#endif /* _pc_scr_h */

/* vi: set tabstop=4 shiftwidth=4: */
/* end of pc_scr.h */
