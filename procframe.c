/*+-------------------------------------------------------------------------
	procframe.c - execute frame of procedure statements
	wht@wht.net

  Defined functions:
	execute_frame(truth)
	pcmd_break()
	pcmd_continue()

--------------------------------------------------------------------------*/
/*+:EDITS:*/
/*:04-26-2000-11:16-wht@bob-RELEASE 4.42 */
/*:01-24-1997-02:38-wht@yuriatin-SOURCE RELEASE 4.00 */
/*:09-11-1996-20:01-wht@yuriatin-3.48-major telnet,curses,structural overhaul */
/*:11-23-1995-11:20-wht@kepler-source control 3.37 for tsx-11 */
/*:11-14-1995-10:23-wht@kepler-3.37.80-source control point: SOCKETS */
/*:05-04-1994-04:40-wht@n4hgf-ECU release 3.30 */
/*:09-10-1992-14:00-wht@n4hgf-ECU release 3.20 */
/*:08-22-1992-15:39-wht@n4hgf-ECU release 3.20 BETA */
/*:07-25-1991-12:59-wht@n4hgf-ECU release 3.10 */
/*:08-14-1990-20:40-wht@n4hgf-ecu3.00-flush old edit history */

#include <ctype.h>
#include "ecu.h"
#include "ecukey.h"
#include "ecuerror.h"
#include "esd.h"
#include "var.h"
#include "procedure.h"

extern PCB *pcb_stack[PROC_STACK_MAX];

/*+-------------------------------------------------------------------------
	pcmd_break()
--------------------------------------------------------------------------*/
int
pcmd_break()
{
	return (eBreakCommand);
}							 /* end of pcmd_break */

/*+-------------------------------------------------------------------------
	pcmd_continue()
--------------------------------------------------------------------------*/
int
pcmd_continue()
{
	return (eContinueCommand);
}							 /* end of pcmd_continue */

/*+-------------------------------------------------------------------------
	execute_frame(truth)

  pcb_stack[proc_level - 1]->current points to lcb behind frame: one
  statement or { statements }

  if truth true, execute frame, else skip it
--------------------------------------------------------------------------*/
int
execute_frame(truth)
int truth;
{
	int itmp;
	int erc = 0;
	PCB *pcb = pcb_stack[proc_level - 1];
	LCB *original_lcb = pcb->current;
	LCB *begin_lcb;
	ESD *text;
	int nest_level = 0;
	int remember_break = 0;
	extern int proc_interrupt;

	if (!(pcb->current = pcb->current->next))
	{
		pcb->current = original_lcb;
		return (eNoFrame);
	}

	text = pcb->current->text;
	text->old_index = text->index = 0;

	if (*text->pb != SPACE)	 /* tabs were converted to spaces at read time */
		return (eLabelInvalidHere);
	skip_cmd_break(text);

/* handle single statement frame */
	if (*(text->pb + text->index) != '{')
	{
		itmp = text->cb - text->index;
		if (((itmp > 2) && !strncmp(text->pb + text->index, "if", 2)))
		{
			pputs("command must appear inside {} or on same line as else\n");
			erc = eFATAL_ALREADY;
		}
		else if (((itmp > 5) && !strncmp(text->pb + text->index, "while", 5)))
		{
			pputs("command must appear inside {} within this context\n");
			erc = eFATAL_ALREADY;
		}
		else if (truth)
		{
			trace_proc_cmd(pcb);
			erc = execute_esd(text);
		}
		return (erc);
	}

/* we've got a {} frame */
	begin_lcb = pcb->current;
	pcb->current = pcb->current->next;
	while (pcb->current)
	{
		if (proc_interrupt)
			return (eCONINT);
		text = pcb->current->text;
		text->old_index = text->index = 0;
		if (*text->pb != SPACE)	/* tabs were converted to spaces at read
								 * time */
			return (eLabelInvalidHere);
		skip_cmd_break(text);
		if (*(text->pb + text->index) == '}')
		{
			if (!nest_level)
			{
				text->index = text->cb;
				if (remember_break)
					return (eBreakCommand);
				return (0);
			}
			nest_level--;
		}
		else if (truth)
		{
			trace_proc_cmd(pcb);
			if (erc = execute_esd(text))
			{
				if (erc != eBreakCommand)
					return (erc);
				remember_break = 1;
				truth = 0;
			}
		}
		else if (*(text->pb + text->index) == '{')
			nest_level++;
		pcb->current = pcb->current->next;
	}
	pcb->current = begin_lcb;
	return (eNoCloseFrame);

}							 /* end of execute_frame */

/* vi: set tabstop=4 shiftwidth=4: */
/* end of procframe.c */
