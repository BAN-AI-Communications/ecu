/*
 *	@(#)smap.h	1.1	30/08/88	16:07:36	agc
 *
 *	Copyright 1988, Joypace Ltd., UK. This product is "careware".
 *	If you find it useful, I suggest that you send what you think
 *	it is worth to the charity of your choice.
 *
 *	Alistair G. Crooks,				+44 5805 3114
 *	Joypace Ltd.,
 *	2 Vale Road,
 *	Hawkhurst,
 *	Kent TN18 4BU,
 *	UK.
 *
 *	UUCP Europe                 ...!mcvax!unido!nixpbe!nixbln!agc
 *	UUCP everywhere else ...!uunet!linus!nixbur!nixpbe!nixbln!agc
 *
 *	smap.h - include file for debugging aids. This file must be included,
 *	before any calls, in any source file that calls malloc, calloc,
 *	realloc, or free. (Note alloca is not included in this list).
 */
/*+:EDITS:*/
/*:04-26-2000-11:16-wht@bob-RELEASE 4.42 */
/*:01-24-1997-02:38-wht@yuriatin-SOURCE RELEASE 4.00 */
/*:09-11-1996-20:01-wht@yuriatin-3.48-major telnet,curses,structural overhaul */
/*:11-23-1995-11:20-wht@kepler-source control 3.37 for tsx-11 */
/*:11-14-1995-10:23-wht@kepler-3.37.80-source control point: SOCKETS */
/*:05-04-1994-04:40-wht@n4hgf-ECU release 3.30 */
/*:09-10-1992-14:00-wht@n4hgf-ECU release 3.20 */
/*:08-22-1992-15:39-wht@n4hgf-ECU release 3.20 BETA */
/*:03-27-1992-16:21-wht@n4hgf-re-include protection for all .h files */
/*:11-30-1991-13:46-wht@n4hgf-smap conditional compilation reorg */
/*:07-25-1991-12:59-wht@n4hgf-ECU release 3.10 */
/*:08-14-1990-20:40-wht@n4hgf-ecu3.00-flush old edit history */

#ifndef _smap_h
#define _smap_h

#ifdef MEMCHECK

#if !defined(VTYPE)
#if __STDC__				 /* sigh ... malloc and such types */
#define VTYPE void
#else
#define VTYPE char
#endif /* __STDC__ */
#endif /* VTYPE */

#define malloc	_malloc
#define calloc	_calloc
#define realloc	_realloc
#define free	_free
VTYPE *_malloc();
VTYPE *_calloc();
VTYPE *_realloc();

#if !defined(sun)
void _free();

#endif /* sun */
void _blkstart();
void _blkend();
void _blkignore();

#endif /* MEMCHECK */

#endif /* _smap_h */

/* vi: set tabstop=4 shiftwidth=4: */
/* end of smap.h */
