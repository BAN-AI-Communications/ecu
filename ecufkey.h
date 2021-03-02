/*+-------------------------------------------------------------------------
	ecufkey.h -- AT XENIX/UNIX function key phrases
	wht@wht.net
--------------------------------------------------------------------------*/
/*+:EDITS:*/
/*:04-26-2000-11:15-wht@bob-RELEASE 4.42 */
/*:01-24-1997-02:37-wht@yuriatin-SOURCE RELEASE 4.00 */
/*:09-11-1996-20:00-wht@yuriatin-3.48-major telnet,curses,structural overhaul */
/*:11-23-1995-11:20-wht@kepler-source control 3.37 for tsx-11 */
/*:11-14-1995-10:23-wht@kepler-3.37.80-source control point: SOCKETS */
/*:01-12-1995-15:19-wht@n4hgf-apply Andrew Chernov 8-bit clean+FreeBSD patch */
/*:05-04-1994-04:38-wht@n4hgf-ECU release 3.30 */
/*:01-01-1993-12:52-wht@n4hgf-add procedure binding for function keys */
/*:01-01-1993-12:35-wht@n4hgf-KDE_OUTSTR_MAX from 32 to 64 for proc exec */
/*:09-10-1992-13:58-wht@n4hgf-ECU release 3.20 */
/*:08-22-1992-15:38-wht@n4hgf-ECU release 3.20 BETA */
/*:03-27-1992-16:21-wht@n4hgf-re-include protection for all .h files */
/*:07-25-1991-12:55-wht@n4hgf-ECU release 3.10 */
/*:08-14-1990-20:40-wht@n4hgf-ecu3.00-flush old edit history */

#ifndef _ecufkey_h
#define _ecufkey_h

/*
 * indices to key mapping tables
 *
 * these are close to being magic numbers - don't change w/o looking
 * at IKDE_to_XF in ecutty.c
 */
enum ikde_enum
{
	IKDE_HOME,
	IKDE_END,
	IKDE_PGUP,
	IKDE_PGDN,
	IKDE_F1,
	IKDE_F2,
	IKDE_F3,
	IKDE_F4,
	IKDE_F5,
	IKDE_F6,
	IKDE_F7,
	IKDE_F8,
	IKDE_F9,
	IKDE_F10,
	IKDE_F11,
	IKDE_F12,
	IKDE_CUU,
	IKDE_CUD,
	IKDE_CUR,
	IKDE_CUL,
	IKDE_CU5,
	IKDE_INS,
	IKDE_BKTAB
};

#define IKDE_lastKey	IKDE_BKTAB
#define IKDE_InitStr	(IKDE_BKTAB + 1)	/* initialization string
											 * kludge */
#define KDE_COUNT		(IKDE_BKTAB + 2)

#define KDE_LOGICAL_MAX		12	/* max length of keystroke sequence */
#define KDE_OUTSTR_MAX		64	/* max length of output sequence */

/*
 * KDE input line types
*/
#define KDETYPE_NAME		1
#define KDETYPE_ENTRY		2
#define KDETYPE_COMMENT		3
#define KDETYPE_EOF			4

typedef struct kde
{
	int count;
	uchar ikde;
	char logical[KDE_LOGICAL_MAX];
	char str[KDE_OUTSTR_MAX];
}
KDE;

/* when a count is not a count but an action: */
#define KACT_COMMAND			-1	/* "Home" key */
#define KACT_LOCAL_SHELL		-2	/* start local shell */
#define KACT_REDISPLAY			-3	/* "Shift-Tab" key */
#define KACT_PROC				-4	/* run proc (invocation in ->str) */

typedef struct kdemap
{
	UINT xf;				 /* extended function code (0x100+ from
							  * ttygetc()) */
	short ikde;				 /* index into keyset */
	char *name;				 /* ECU "formal" name for key */
	char *init;				 /* key sends this string to port by default */
}
KDEMAP;

char *keyset_idstr();

#endif /* _ecufkey_h */

/* vi: set tabstop=4 shiftwidth=4: */
/* end of ecufkey.h */
