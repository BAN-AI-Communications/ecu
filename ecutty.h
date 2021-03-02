/*+-------------------------------------------------------------------------
	ecutty.h
	wht@wht.net
--------------------------------------------------------------------------*/
/*+:EDITS:*/
/*:04-26-2000-11:15-wht@bob-RELEASE 4.42 */
/*:01-24-1997-02:37-wht@yuriatin-SOURCE RELEASE 4.00 */
/*:09-11-1996-20:00-wht@yuriatin-3.48-major telnet,curses,structural overhaul */
/*:11-23-1995-11:20-wht@kepler-source control 3.37 for tsx-11 */
/*:11-14-1995-10:23-wht@kepler-3.37.80-source control point: SOCKETS */
/*:05-04-1994-04:39-wht@n4hgf-ECU release 3.30 */
/*:09-10-1992-13:59-wht@n4hgf-ECU release 3.20 */
/*:08-22-1992-15:38-wht@n4hgf-ECU release 3.20 BETA */
/*:03-27-1992-16:21-wht@n4hgf-re-include protection for all .h files */
/*:07-25-1991-12:57-wht@n4hgf-ECU release 3.10 */
/*:07-14-1991-18:19-wht@n4hgf-new ttygets functions */
/*:08-14-1990-20:40-wht@n4hgf-ecu3.00-flush old edit history */

#ifndef _ecutty_h
#define _ecutty_h

typedef struct color_type
{
	char *name;
	int num;
}
COLOR;

#ifdef DEFINE_TTY_DATA
COLOR colors[] =
{
	{"black", 0},
	{"blue", 1},
	{"brown", 6},
	{"cyan", 3},
	{"gray", 8},
	{"green", 2},
	{"hi_white", 15},
	{"lt_blue", 9},
	{"lt_cyan", 11},
	{"lt_green", 10},
	{"lt_magenta", 13},
	{"lt_red", 12},
	{"magenta", 5},
	{"red", 4},
	{"white", 7},
	{"yellow", 14},
	{(char *)0, -1}
};

#else
extern COLOR colors[];

#endif

/* color words are UINT32:
   MSB:  reverse video foreground
         reverse video background
         normal  video foreground
   LSB:  normal  video background
*/

/*
 * ttygets flag bits
 */
#define TG_CRLF		1		 /* echo cr/lf terminator */
#define TG_XDELIM	2		 /* extended delimiter set */
#define TG_EDIT		4		 /* redisplay/edit current string */

#endif /* _ecutty_h */

/* vi: set tabstop=4 shiftwidth=4: */
/* end of ecutty.h */
