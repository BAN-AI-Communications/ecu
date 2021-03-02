/*+-------------------------------------------------------------------------
	relop.h - operator definitions (relative and logical)
	wht@wht.net
--------------------------------------------------------------------------*/
/*+:EDITS:*/
/*:04-26-2000-11:16-wht@bob-RELEASE 4.42 */
/*:01-24-1997-02:38-wht@yuriatin-SOURCE RELEASE 4.00 */
/*:10-28-1996-13:54-wht@yuriatin-add OP_LAND and OP_LOR */
/*:09-11-1996-20:01-wht@yuriatin-3.48-major telnet,curses,structural overhaul */
/*:11-23-1995-11:20-wht@kepler-source control 3.37 for tsx-11 */
/*:11-14-1995-10:23-wht@kepler-3.37.80-source control point: SOCKETS */
/*:05-04-1994-04:40-wht@n4hgf-ECU release 3.30 */
/*:12-20-1992-00:09-wht@n4hgf-all operators go in here + add shift ops */
/*:09-10-1992-14:00-wht@n4hgf-ECU release 3.20 */
/*:08-22-1992-15:39-wht@n4hgf-ECU release 3.20 BETA */
/*:03-27-1992-16:21-wht@n4hgf-re-include protection for all .h files */
/*:07-25-1991-12:59-wht@n4hgf-ECU release 3.10 */
/*:08-14-1990-20:40-wht@n4hgf-ecu3.00-flush old edit history */

#ifndef _relop_h
#define _relop_h

#define OP_EQ   1
#define OP_NE   2
#define OP_LT   3
#define OP_LE   4
#define OP_GT   5
#define OP_GE   6

#define OP_ADD	10
#define OP_SUB	11
#define OP_MUL	12
#define OP_DIV	13

#define OP_AND	20
#define OP_OR	21
#define OP_MOD	22
#define OP_XOR	23
#define OP_SHL	24
#define OP_SHR	25
#define OP_LAND	26
#define OP_LOR	27

#endif /* _relop_h */

/* vi: set tabstop=4 shiftwidth=4: */
/* end of relop.h */
