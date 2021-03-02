/*+-------------------------------------------------------------------------
	procedure.h
	wht@wht.net
--------------------------------------------------------------------------*/
/*+:EDITS:*/
/*:04-26-2000-11:16-wht@bob-RELEASE 4.42 */
/*:01-08-2000-14:03-wht@menlo-add function declarations */
/*:01-24-1997-02:38-wht@yuriatin-SOURCE RELEASE 4.00 */
/*:01-12-1997-13:49-wht@yuriatin-rename proc.h avoiding conflict sys/proc.h */
/*:01-12-1997-13:45-wht@yuriatin-place LCB next/prev pointers first */
/*:09-11-1996-20:01-wht@yuriatin-3.48-major telnet,curses,structural overhaul */
/*:11-23-1995-11:20-wht@kepler-source control 3.37 for tsx-11 */
/*:11-14-1995-10:23-wht@kepler-3.37.80-source control point: SOCKETS */
/*:05-04-1994-04:40-wht@n4hgf-ECU release 3.30 */
/*:09-10-1992-14:00-wht@n4hgf-ECU release 3.20 */
/*:08-22-1992-15:39-wht@n4hgf-ECU release 3.20 BETA */
/*:03-27-1992-16:21-wht@n4hgf-re-include protection for all .h files */
/*:11-16-1991-14:00-wht@n4hgf-add upon_dcdloss */
/*:07-25-1991-12:59-wht@n4hgf-ECU release 3.10 */
/*:08-14-1990-20:40-wht@n4hgf-ecu3.00-flush old edit history */

#ifndef _proc_h
#define _proc_h

typedef struct lcb_type
{
	struct lcb_type *next;	
	struct lcb_type *prev;
	ESD *text;				 /* line's text buffer */
	UINT lineno;			 /* line number */
} LCB;

LCB *find_labelled_lcb();
LCB *find_cproc_labelled_lcb();

typedef struct pcb_type
{
	int argc;
	char **argv;
	LCB *first;				 /* first in procedure */
	LCB *last;				 /* last in procedure */
	LCB *current;			 /* currently executing or last executed line */
	ESD upon_dcdloss;		 /* pseudo-ESD to execute as a statement upon
							  * DCD loss */
	char *mkvs_last;		 /* actually MKV *, but ... */
	char *mkvi_last;		 /* ... see var.c for details */
}
PCB;

#define MAX_PARGV 20		 /* max args to procedure, including name */
#define PROC_STACK_MAX	10	 /* max proc nest */

#endif /* _proc_h */

/* vi: set tabstop=4 shiftwidth=4: */
/* end of procedure.h */
