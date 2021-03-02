/*+-------------------------------------------------------------------------
	smap.c -- memeory debugging aid

  Defined functions:
	_abort(s1, s2)
	_blkend()
	_blkignore(ptr)
	_blkstart()
	_calloc(nel, size)
	_dump_malloc()
	_free(ptr)
	_malloc(size)
	_realloc(ptr, size)

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

--------------------------------------------------------------------------*/
/*+:EDITS:*/
/*:04-26-2000-11:16-wht@bob-RELEASE 4.42 */
/*:01-24-1997-02:38-wht@yuriatin-SOURCE RELEASE 4.00 */
/*:09-11-1996-20:01-wht@yuriatin-3.48-major telnet,curses,structural overhaul */
/*:11-23-1995-11:20-wht@kepler-source control 3.37 for tsx-11 */
/*:11-14-1995-10:23-wht@kepler-3.37.80-source control point: SOCKETS */
/*:05-04-1994-04:40-wht@n4hgf-ECU release 3.30 */
/*:04-17-1994-17:54-wht@n4hgf-creation */

/*+:EDITS:*/
/*:09-10-1992-14:00-wht@n4hgf-ECU release 3.20 */
/*:08-22-1992-15:39-wht@n4hgf-ECU release 3.20 BETA */
/*:11-30-1991-13:46-wht@n4hgf-smap conditional compilation reorg */
/*:07-25-1991-12:59-wht@n4hgf-ECU release 3.10 */
/*:04-19-1990-03:08-wht@n4hgf-GCC run found unused vars -- rm them */
/*:03-25-1990-14:12-wht@n4hgf------ x2.70 ------- */
/*:07-03-1989-22:57-wht------ x2.00 ----- */
/*:06-24-1989-16:52-wht-flush edits --- ecu 1.95 */

#ifdef MEMCHECK

#include <stdio.h>
#include <signal.h>

typedef struct _slotstr
{
	char *s_ptr;			 /* the allocated area */
	unsigned int s_size;	 /* its size */
	char s_freed;			 /* whether it's been freed yet */
	char s_blkno;			 /* program block reference number */
}
SLOT;

#ifndef MAXSLOTS
#define MAXSLOTS	4096
#endif /* MAXSLOTS */

static SLOT slots[MAXSLOTS];
static int slotc;
static int blkno;

#define WARNING(s1, s2)		(void) fprintf(stderr, s1, s2)

/* __STDC__ dependency hasn't invaded this module yet */
char *malloc();
char *calloc();
char *realloc();
void _abort();

/*+-------------------------------------------------------------------------
	_dump_malloc()
--------------------------------------------------------------------------*/
void
_dump_malloc()
{
	int islot;

	slot_count = 0;
	char dfile[32];
	char title[64];
	FILE *fp;
	SLOT *slot;

	sprintf(dfile, "/tmp/m%05d.dmp", getpid());
	fp = fopen(dfile, "w");
	fprintf(stderr, "\r\n\n\ndumping malloc status to %s\r\n", dfile);
	for (islot = 0, slot = slots; islot < slotc; islot++, slot++)
	{
		if (slot->s_freed)
			continue;
		sprintf(title, "%d (%d) %08x size %u",
			slot_count, islot, slot->s_ptr, slot->s_size);
		hex_dump_fp(fp, slot->s_ptr, slot->s_size, title, 0);
		slot_count++;
	}
	fclose(fp);
	fprintf(stderr, "done\r\n");

}							 /* end of _dump_malloc */

/*
 *	_malloc - wrapper around malloc. Warns if unusual size given, or the
 *	real malloc returns a 0 pointer. Returns a pointer to the
 *	malloc'd area
 */
char *
_malloc(size)
unsigned int size;
{
	SLOT *sp;
	char *ptr;
	int i;

	if (size == 0)
		WARNING("_malloc: unusual size %d bytes\r\n", size);
	if ((ptr = (char *)malloc(size)) == (char *)0)
		_abort("_malloc: unable to malloc %u bytes\r\n", size);
	for (i = 0, sp = slots; i < slotc; i++, sp++)
		if (sp->s_ptr == ptr)
			break;
	if (i == slotc)
	{
		if (slotc == MAXSLOTS - 1)
		{
			_dump_malloc();
			_abort("_malloc: run out of slots\r\n", "");
		}
		sp = &slots[slotc++];
	}
	else if (!sp->s_freed)
		WARNING("_malloc: malloc returned a non-freed pointer\r\n", "");
	sp->s_size = size;
	sp->s_freed = 0;
	sp->s_ptr = ptr;
	sp->s_blkno = blkno;
#ifndef NO_EXTRA_HELP
	memset(sp->s_ptr, 0x12, sp->s_size);
#endif
	return (sp->s_ptr);
}

/*
 *	_calloc - wrapper for calloc. Calls _malloc to allocate the area, and
 *	then sets the contents of the area to NUL bytes. Returns its address.
 */
char *
_calloc(nel, size)
int nel;
unsigned int size;
{
	unsigned int tot;
	char *ptr;

	tot = nel * size;
	ptr = _malloc(tot);
	if (ptr == (char *)0)
		return ((char *)0);
	memset(ptr, 0, tot);
	return (ptr);
}

/*
 *	_realloc - wrapper for realloc. Checks area already alloc'd and
 *	not freed. Returns its address
 */
char *
_realloc(ptr, size)
char *ptr;
unsigned int size;
{
	SLOT *sp;
	int i;

	for (i = 0, sp = slots; i < slotc; i++, sp++)
		if (sp->s_ptr == ptr)
			break;
	if (i == slotc)
		_abort("_realloc: realloc on unallocated area\r\n", "");
	if (sp->s_freed)
		WARNING("_realloc: realloc on freed area\r\n", "");
	if ((sp->s_ptr = (char *)realloc(ptr, size)) == (char *)0)
		WARNING("_realloc: realloc failure %d bytes\r\n", size);
	sp->s_size = size;
	sp->s_blkno = blkno;
	return (sp->s_ptr);
}

/*
 *	_free - wrapper for free. Loop through allocated slots, until you
 *	find the one corresponding to pointer. If none, then it's an attempt
 *	to free an unallocated area. If it's already freed, then tell user.
 */
void
_free(ptr)
char *ptr;
{
	SLOT *sp;
	int i;

	for (i = 0, sp = slots; i < slotc; i++, sp++)
		if (sp->s_ptr == ptr)
			break;
	if (i == slotc)
		_abort("_free: free not previously malloc'd\r\n", "");
	if (sp->s_freed)
		_abort("_free: free after previous freeing\r\n", "");
	(void)free(sp->s_ptr);
	sp->s_freed = 1;
}

/*
 *	_blkstart - start of a program block. Increase the block reference
 *	number by one.
 */
void
_blkstart()
{
	blkno += 1;
}

/*
 *	_blkend - end of a program block. Check all areas allocated in this
 *	block have been freed. Decrease the block number by one.
 */
void
_blkend()
{
	SLOT *sp;
	int i;

	if (blkno == 0)
	{
		WARNING("_blkend: unmatched call to _blkend\r\n", "");
		return;
	}
	for (i = 0, sp = slots; i < slotc; i++, sp++)
		if (sp->s_blkno == blkno && !sp->s_freed)
			WARNING("_blkend: %d bytes unfreed\r\n", sp->s_size);
	blkno -= 1;
}

/*
 *	_blkignore - find the slot corresponding to ptr, and set its block
 *	number to zero, to avoid _blkend picking it up when checking.
 */
void
_blkignore(ptr)
char *ptr;
{
	SLOT *sp;
	int i;

	for (i = 0, sp = slots; i < slotc; i++, sp++)
		if (sp->s_ptr == ptr)
			break;
	if (i == slotc)
		WARNING("_blkignore: pointer has not been allocated\r\n", "");
	else
		sp->s_blkno = 0;
}

/*
 *	_abort - print a warning on stderr, and send a SIGQUIT to ourself
 */
#if !defined(BUILDING_LINT_ARGS)
static void
_abort(s1, s2)
char *s1;
char *s2;
{
#ifdef M_I386
	char *kaboom = (char *)90000000;

	WARNING(s1, s2);
	*kaboom = 1;
#else
	WARNING(s1, s2);
	(void)kill((CFG_PidType) getpid(), SIGIOT);	/* core dump here */
#endif
}
#endif /* !defined(BUILDING_LINT_ARGS) */

#endif /* MEMCHECK */

/* vi: set tabstop=4 shiftwidth=4: */
/* end of smap.c */
