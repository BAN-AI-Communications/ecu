/*+-------------------------------------------------------------------------
	ecuicmhist.c - ECU interactive command history handler
	wht@wht.net

  Defined functions:
	icmd_history_add(icmd_buf)
	icmd_history_manager(func, newicmd, icmdsize)

  I met this girl in Macy's.  She was shopping, and I was putting
  slinkeys on the escalator.  -- Steven Wright

--------------------------------------------------------------------------*/
/*+:EDITS:*/
/*:04-26-2000-11:15-wht@bob-RELEASE 4.42 */
/*:01-08-2000-14:08-wht@menlo-strdup can be a macro */
/*:01-24-1997-02:37-wht@yuriatin-SOURCE RELEASE 4.00 */
/*:09-11-1996-20:00-wht@yuriatin-3.48-major telnet,curses,structural overhaul */
/*:11-23-1995-11:20-wht@kepler-source control 3.37 for tsx-11 */
/*:11-14-1995-10:23-wht@kepler-3.37.80-source control point: SOCKETS */
/*:01-12-1995-15:19-wht@n4hgf-apply Andrew Chernov 8-bit clean+FreeBSD patch */
/*:05-04-1994-04:38-wht@n4hgf-ECU release 3.30 */
/*:09-13-1992-02:05-wht@n4hgf-redisplay escape prompt on error exit */
/*:09-10-1992-13:58-wht@n4hgf-ECU release 3.20 */
/*:08-22-1992-15:38-wht@n4hgf-ECU release 3.20 BETA */
/*:08-28-1991-14:07-wht@n4hgf2-SVR4 cleanup by aega84!lh */
/*:08-11-1991-14:58-wht@n4hgf-new ttygets botched command history handler */
/*:07-25-1991-12:56-wht@n4hgf-ECU release 3.10 */
/*:07-14-1991-18:18-wht@n4hgf-new ttygets functions */
/*:08-14-1990-20:40-wht@n4hgf-ecu3.00-flush old edit history */

#include <string.h>
#include "ecucurses.h"

#define STDIO_H_INCLUDED
#define OMIT_TERMIO_REFERENCES
#include "ecu.h"

#include "pc_scr.h"
#include "ecukey.h"
#include "ecuxkey.h"
#include "ecutty.h"


#define ICMDH_MAXCNT	50
#define ICMDH_MAXLEN	72

typedef struct icmd_hist
{
	struct icmd_hist *prev;
	struct icmd_hist *next;
	uchar *icmd;
} ICMDH;

ICMDH *icmdh_head = (ICMDH *) 0;
ICMDH *icmdh_tail = (ICMDH *) 0;
int icmdh_count = 0;

/*+-------------------------------------------------------------------------
	icmd_history_add(icmd_buf)
--------------------------------------------------------------------------*/
void
icmd_history_add(icmd_buf)
char *icmd_buf;
{
	ICMDH *icmdh = (ICMDH *) malloc(sizeof(ICMDH));

	if (!icmdh)
		return;
	if (!(icmdh->icmd = (uchar *) strdup(icmd_buf)))
	{
		free((char *)icmdh);
		return;
	}
	if (strlen((char *)icmdh->icmd) > (unsigned)ICMDH_MAXLEN)
		icmdh->icmd[ICMDH_MAXLEN] = 0;
	if (icmdh_tail)
	{
		icmdh_tail->next = icmdh;
		icmdh->prev = icmdh_tail;
		icmdh->next = (ICMDH *) 0;
		icmdh_tail = icmdh;
	}
	else
	{
		icmdh->prev = (ICMDH *) 0;
		icmdh->next = (ICMDH *) 0;
		icmdh_head = icmdh;
		icmdh_tail = icmdh;
	}
	if (++icmdh_count > ICMDH_MAXCNT)
	{
		icmdh = icmdh_head;
		icmdh_head = icmdh->next;
		icmdh_head->prev = (ICMDH *) 0;
		free((char *)icmdh->icmd);
		free((char *)icmdh);
		icmdh_count--;
	}

}							 /* end of icmd_history_add */

/*+-------------------------------------------------------------------------
	icmd_history_manager(func,newicmd,icmdsize) - entered by Home Xkey

return new icmd string to execute
returns 0 if ok to exce new cmd, else 1 if not
(returns 0 if null or ESC, so caller can handle exit condition)
--------------------------------------------------------------------------*/
/*ARGSUSED*/
int
icmd_history_manager(func, newicmd, icmdsize)
uchar func;
uchar *newicmd;
int icmdsize;
{
	int itmp;
	ICMDH *icmdh = icmdh_tail;
	UINT delim;

	if (!icmdh)
	{
		ff(se, "  no interactive commands saved\r\n");
		show_escape_prompt();
		return (1);
	}
	while (1)
	{
		strncpy((char *)newicmd, (char *)icmdh->icmd, icmdsize - 1);
		*(newicmd + icmdsize - 1) = 0;

		ttygets(newicmd, icmdsize, TG_XDELIM | TG_EDIT, &delim, (int *)0);
		if (!newicmd[0])
			return (0);

		switch (delim)
		{
			case ESC:
				*newicmd = 0;
				return (0);

			case XFhome:
				icmdh = icmdh_head;
				break;

			case XFend:
				icmdh = icmdh_tail;
				break;

			case XFpgup:
			case XFpgdn:
				ring_bell();
				break;

			case XFcurup:
				if (icmdh->prev)
					icmdh = icmdh->prev;
				break;

			case XFcurdn:
				if (icmdh->next)
					icmdh = icmdh->next;
				break;

			default:
				return (0);
		}

		itmp = strlen((char *)newicmd);
		while (itmp--)
			fputc(BS, se);
		itmp = strlen((char *)newicmd);
		while (itmp--)
			fputc(' ', se);
		itmp = strlen((char *)newicmd);
		while (itmp--)
			fputc(BS, se);
	}
}							 /* end of icmd_history_manager */

/* vi: set tabstop=4 shiftwidth=4: */
/* end of ecuicmhist.c */
