/*+-----------------------------------------------------------------------
	esd.h -- support header for users of esdutil.c
	wht@wht.net
------------------------------------------------------------------------*/
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
/*:03-20-1992-06:28-wht@n4hgf-max size of esd now 16384 */
/*:08-29-1991-02:02-wht@n4hgf2-larger max string size for sun and SVR4 */
/*:07-25-1991-12:57-wht@n4hgf-ECU release 3.10 */
/*:04-24-1991-18:49-wht@n4hgf-add ESD_MAXSZ */
/*:08-14-1990-20:40-wht@n4hgf-ecu3.00-flush old edit history */

#ifndef _esd_h
#define _esd_h

#define ESD_MAXSZ 16384
#define ESD_NOMSZ 256

typedef struct esd
{
	char *pb;				 /* pointer to string buffer */
	short cb;				 /* count of bytes */
	short maxcb;			 /* maximum bytes allowed */
	short index;			 /* next character of significance */
	short old_index;		 /* last token (backup or error reporting) */
}
ESD;

typedef struct keyword_table_type	/* table terminated with null key_word */
{
	char *key_word;			 /* 12 char max key word */
	int key_token;			 /* token returned on match */
}
KEYTAB;

ESD *esdalloc();

#endif /* _esd_h */

/* vi: set tabstop=4 shiftwidth=4: */
/* end of esd.h */
