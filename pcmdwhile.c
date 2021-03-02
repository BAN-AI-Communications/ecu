/*+-------------------------------------------------------------------------
	pcmdwhile.c - ecu while procedure commands
	wht@wht.net

  Defined functions:
	_pcmd_while_z_nz(param,nz)
	pcmd_whilei(param)
	pcmd_whilenz(param)
	pcmd_whiles(param)
	pcmd_whilez(param)

--------------------------------------------------------------------------*/
/*+:EDITS:*/
/*:04-26-2000-11:16-wht@bob-RELEASE 4.42 */
/*:01-24-1997-02:38-wht@yuriatin-SOURCE RELEASE 4.00 */
/*:10-09-1996-21:00-wht@yuriatin-add whilez/whilenz */
/*:09-11-1996-20:00-wht@yuriatin-3.48-major telnet,curses,structural overhaul */
/*:11-23-1995-11:20-wht@kepler-source control 3.37 for tsx-11 */
/*:11-14-1995-10:23-wht@kepler-3.37.80-source control point: SOCKETS */
/*:05-04-1994-04:39-wht@n4hgf-ECU release 3.30 */
/*:09-10-1992-14:00-wht@n4hgf-ECU release 3.20 */
/*:08-22-1992-15:39-wht@n4hgf-ECU release 3.20 BETA */
/*:07-25-1991-12:59-wht@n4hgf-ECU release 3.10 */
/*:08-14-1990-20:40-wht@n4hgf-ecu3.00-flush old edit history */

#include <ctype.h>
#include "ecu.h"
#include "ecuerror.h"
#include "esd.h"
#include "var.h"
#include "procedure.h"
#include "relop.h"

extern PCB *pcb_stack[];

/*+-------------------------------------------------------------------------
    _pcmd_while_z_nz(param,nz)
--------------------------------------------------------------------------*/
int
_pcmd_while_z_nz(param,nz)
ESD *param;
int nz;		/* if nz==1 whilenz, ==0  whilez */
{
	int erc;
	long truth;
	PCB *pcb;
	LCB *condition_lcb;
	int condition_index = param->index;

	if (!proc_level)
		return (eNotExecutingProc);

	pcb = pcb_stack[proc_level - 1];
	condition_lcb = pcb->current;

  REPEAT_WHILE:

	if (erc = gint(param, &truth))
		return (erc);
	truth = !!truth;
	if(!nz)
		truth = !truth;

/* if end of command, execute frame */
	if (end_of_cmd(param))
	{
		if (erc = execute_frame(truth))
		{
			if (erc == eContinueCommand)
				goto CONTINUE;
			if (erc == eBreakCommand)
				erc = 0;
			return (erc);
		}
	}
	else if (truth)
	{
		if (erc = execute_esd(param))
			return (erc);
	}

/* repeat if indicated */
  CONTINUE:
	if (truth)
	{
		pcb->current = condition_lcb;
		param->index = param->old_index = condition_index;
		goto REPEAT_WHILE;
	}

	return (0);
}							 /* end of _pcmd_while_z_nz */

/*+-------------------------------------------------------------------------
    pcmd_whilenz(param)
--------------------------------------------------------------------------*/
int
pcmd_whilenz(param)
ESD *param;
{
	return(_pcmd_while_z_nz(param,1));
}		/* end of pcmd_whilenz */

/*+-------------------------------------------------------------------------
    pcmd_whilez(param)
--------------------------------------------------------------------------*/
int
pcmd_whilez(param)
ESD *param;
{
	return(_pcmd_while_z_nz(param,0));
}		/* end of pcmd_whilez */

/*+-------------------------------------------------------------------------
    pcmd_whilei(param)
--------------------------------------------------------------------------*/
int
pcmd_whilei(param)
ESD *param;
{
	int erc;
	int truth;
	PCB *pcb;
	LCB *condition_lcb;
	int condition_index = param->index;

	if (!proc_level)
		return (eNotExecutingProc);

	pcb = pcb_stack[proc_level - 1];
	condition_lcb = pcb->current;

  REPEAT_WHILE:

	if (erc = get_truth_int(param, &truth))
		return (erc);

/* if end of command, execute frame */
	if (end_of_cmd(param))
	{
		if (erc = execute_frame(truth))
		{
			if (erc == eContinueCommand)
				goto CONTINUE;
			if (erc == eBreakCommand)
				erc = 0;
			return (erc);
		}
	}
	else if (truth)
	{
		if (erc = execute_esd(param))
			return (erc);
	}

/* repeat if indicated */
  CONTINUE:
	if (truth)
	{
		pcb->current = condition_lcb;
		param->index = param->old_index = condition_index;
		goto REPEAT_WHILE;
	}

	return (0);
}							 /* end of pcmd_whilei */

/*+-------------------------------------------------------------------------
    pcmd_whiles(param)
--------------------------------------------------------------------------*/
int
pcmd_whiles(param)
ESD *param;
{
	int erc;
	int truth;
	PCB *pcb;
	LCB *condition_lcb;
	int condition_index = param->index;

	if (!proc_level)
		return (eNotExecutingProc);

	pcb = pcb_stack[proc_level - 1];
	condition_lcb = pcb->current;

  REPEAT_WHILE:

	if (erc = get_truth_str(param, &truth))
		return (erc);

/* if end of command, execute frame */
	if (end_of_cmd(param))
	{
		if (erc = execute_frame(truth))
		{
			if (erc == eContinueCommand)
				goto CONTINUE;
			if (erc == eBreakCommand)
				erc = 0;
			return (erc);
		}
	}
	else if (truth)
	{
		if (erc = execute_esd(param))
			return (erc);
	}

/* repeat if indicated */
  CONTINUE:
	if (truth)
	{
		pcb->current = condition_lcb;
		param->index = param->old_index = condition_index;
		goto REPEAT_WHILE;
	}

	return (0);
}							 /* end of pcmd_whiles */

/* vi: set tabstop=4 shiftwidth=4: */
/* end of pcmdwhile.c */
