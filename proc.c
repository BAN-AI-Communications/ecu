/*+-------------------------------------------------------------------------
	proc.c - procedure command and control
	wht@wht.net

  Defined functions:
	_cmd_gosub_common(param, type)
	_get_goto_label(param)
	do_proc(argc, argv)
	dump_proc(pcb)
	execute_esd(tesd)
	execute_goto(pcb, goto_type)
	execute_labelled_lcb(tesd)
	execute_proc(pcb, use_goto_label)
	find_cproc_labelled_lcb(label)
	find_labelled_lcb(label, first, last)
	find_proc_cmd(cmd_list, cmd)
	find_procedure(name)
	free_lcb_chain(lcb)
	pcmd_do(param)
	pcmd_gosub(param)
	pcmd_gosubb(param)
	pcmd_goto(param)
	pcmd_gotob(param)
	pcmd_return(param)
	pcmd_upon(param)
	proc_dcdloss_handler(pcb)
	show_error_position(pcb)
	trace_proc_cmd(pcb)

--------------------------------------------------------------------------*/
/*+:EDITS:*/
/*:04-26-2000-11:16-wht@bob-RELEASE 4.42 */
/*:01-08-2000-14:04-wht@menlo-add find_cproc_labelled_lcb */
/*:10-26-1998-14:46-wht@menlo-obscure bug unreported: goto_label overflow */
/*:11-11-1997-12:39-wht@gyro-more microfidgeting */
/*:11-10-1997-17:52-wht@kepler-obscure unlikely memory leak */
/*:11-03-1997-02:23-wht@kepler-4.08a-option command+doc primping */
/*:02-09-1997-19:43-wht@yuriatin-static procpath from 256 to ECU_MAXPN in len */
/*:01-24-1997-02:38-wht@yuriatin-SOURCE RELEASE 4.00 */
/*:12-10-1996-19:29-wht@yuriatin-allow # comment after semicolon */
/*:10-16-1996-02:07-wht@yuriatin-retain variables created during gosub */
/*:09-11-1996-20:01-wht@yuriatin-3.48-major telnet,curses,structural overhaul */
/*:08-20-1996-12:39-wht@kepler-locale/ctype fixes from ache@nagual.ru */
/*:11-23-1995-11:20-wht@kepler-source control 3.37 for tsx-11 */
/*:11-14-1995-10:23-wht@kepler-3.37.80-source control point: SOCKETS */
/*:09-17-1995-16:28-wht@kepler-remove obsolete #if 0 code */
/*:05-11-1995-15:55-wht@n4hgf-ck_sigint fools optimizing compilers */
/*:03-11-1995-17:55-wht@kepler-no more use of 'index' as name of auto */
/*:05-04-1994-04:39-wht@n4hgf-ECU release 3.30 */
/*:03-10-1994-17:46-wht@n4hgf-was not closing files on proc exit */
/*:09-10-1992-14:00-wht@n4hgf-ECU release 3.20 */
/*:08-22-1992-15:39-wht@n4hgf-ECU release 3.20 BETA */
/*:11-16-1991-15:39-wht@n4hgf2-add pcmd_upon stub */
/*:11-16-1991-14:53-wht@n4hgf2-add proc_dcdloss_handler */
/*:11-16-1991-14:01-wht@n4hgf-calloc pcb instead of malloc */
/*:07-25-1991-12:59-wht@n4hgf-ECU release 3.10 */
/*:07-01-1991-01:53-wht@n4hgf-fix return with value */
/*:05-01-1991-04:18-wht@n4hgf-new find_procedure failed on home subdir match */
/*:04-30-1991-03:19-root@n4hgf-add search for .ep in ecu lib ep subdir */
/*:08-14-1990-20:40-wht@n4hgf-ecu3.00-flush old edit history */

#include "ecu.h"
#include "ecuerror.h"
#include "ecukey.h"
#include "esd.h"
#include "var.h"
#include "procedure.h"

#define NEED_P_CMD
#include "ecucmd.h"

PCB *pcb_stack[PROC_STACK_MAX];

int proc_level = 0;
int proc_trace = 0;
int proc_option_localvars = 0;

char goto_label[64];

/*+-------------------------------------------------------------------------
	_get_goto_label(param)
--------------------------------------------------------------------------*/
int
_get_goto_label(param)
ESD *param;
{
	int erc;
	ESD *label_esd;

	goto_label[0] = 0;
	if (erc = get_alphanum_zstr(param, goto_label, sizeof(goto_label)))
	{
		if (!(label_esd = esdalloc(ESD_NOMSZ)))
			return (eNoMemory);
		if (!(erc = gstr(param, label_esd, 0)))
		{
			strncpy(goto_label, label_esd->pb,sizeof(goto_label));
			goto_label[sizeof(goto_label) - 1] = 0;
		}
		esdfree(label_esd);
	}

	return (erc);

}							 /* end of _get_goto_label */

/*+-------------------------------------------------------------------------
	pcmd_goto(param)
--------------------------------------------------------------------------*/
int
pcmd_goto(param)
ESD *param;
{

	if (!proc_level)
		return (eNotExecutingProc);
	if (_get_goto_label(param))
		return (eInvalidLabel);
	return (eProcAttn_GOTO);

}							 /* end of pcmd_goto */

/*+-------------------------------------------------------------------------
	pcmd_gotob(param)
--------------------------------------------------------------------------*/
int
pcmd_gotob(param)
ESD *param;
{

	if (!proc_level)
		return (eNotExecutingProc);
	if (_get_goto_label(param))
		return (eInvalidLabel);
	return (eProcAttn_GOTOB);

}							 /* end of pcmd_gotob */

/*+-------------------------------------------------------------------------
	_cmd_gosub_common(param,type)
--------------------------------------------------------------------------*/
int
_cmd_gosub_common(param, type)
ESD *param;
int type;
{
	int erc;
	LCB *current_save;
	int index_save;

	if (_get_goto_label(param))
		return (eInvalidLabel);
	current_save = pcb_stack[proc_level - 1]->current;
	index_save = current_save->text->index;
	if (!(erc = execute_proc(pcb_stack[proc_level - 1], type)))
	{
		pcb_stack[proc_level - 1]->current = current_save;
		current_save->text->index = index_save;
	}
	return (erc);

}							 /* end of _cmd_gosub_common */

/*+-------------------------------------------------------------------------
	pcmd_gosub(param)
--------------------------------------------------------------------------*/
int
pcmd_gosub(param)
ESD *param;
{
	if (!proc_level)
		return (eNotExecutingProc);
	return (_cmd_gosub_common(param, eProcAttn_GOTO));
}							 /* end of pcmd_gosub */

/*+-------------------------------------------------------------------------
	pcmd_return(param)
--------------------------------------------------------------------------*/
int
pcmd_upon(param)
ESD *param;
{
	pprintf("'upon' not implimented\n");
	param->index = param->cb;
	return (0);
}							 /* end of pcmd_return */

/*+-------------------------------------------------------------------------
	pcmd_gosubb(param)
--------------------------------------------------------------------------*/
int
pcmd_gosubb(param)
ESD *param;
{
	if (!proc_level)
		return (eNotExecutingProc);
	return (_cmd_gosub_common(param, eProcAttn_GOTO));
}							 /* end of pcmd_gosubb */

/*+-------------------------------------------------------------------------
	pcmd_return(param)
--------------------------------------------------------------------------*/
int
pcmd_return(param)
ESD *param;
{
	long value = 0;

	if (!gint(param, &value))
	{
		if ((value < 0) || (value > 255))
			value = 255;
		if (proc_trace)
			pprintf("return value %ld\n", value);
		if (value)
			value += e_USER;
		return ((int)value);
	}
	return (eProcAttn_RETURN);
}							 /* end of pcmd_return */

/*+-------------------------------------------------------------------------
	find_labelled_lcb(label,first,last)
search for match between label
--------------------------------------------------------------------------*/
LCB *
find_labelled_lcb(label, first, last)
char *label;
LCB *first;
LCB *last;
{
	int llen = strlen(label);
	ESD *text;

	while (first)
	{
		text = first->text;
		if ((text->cb >= llen) && (!strncmp(text->pb, label, llen))
			&& (!text->pb[llen] || isspace((uchar) text->pb[llen])))
			return (first);

		if (first == last)
			return ((LCB *) 0);
		first = first->next;
	}
	pputs("find_labelled_lab logic error\n");
	return ((LCB *) 0);

}							 /* end of find_labelled_lcb */

/*+-------------------------------------------------------------------------
	find_cproc_labelled_lcb(label)
--------------------------------------------------------------------------*/
LCB *
find_cproc_labelled_lcb(label)
char *label;
{
	PCB *pcb;
	if(!proc_level)
		return(0);
	pcb = pcb_stack[proc_level - 1];
	return(find_labelled_lcb(label, pcb->first, pcb->last));

}	/* end of find_cproc_labelled_lcb */

/*+-------------------------------------------------------------------------
	execute_goto(pcb,goto_type)
--------------------------------------------------------------------------*/
execute_goto(pcb, goto_type)
PCB *pcb;
int goto_type;
{
	LCB *next = (LCB *) 0;	 /* next lcb to execute */

	switch (goto_type)
	{
		case eProcAttn_GOTO:
			if (!(next = find_labelled_lcb(goto_label, pcb->current, pcb->last)))
				next = find_labelled_lcb(goto_label, pcb->first, pcb->current);
			break;
		case eProcAttn_GOTOB:
			if (!(next = find_labelled_lcb(goto_label, pcb->first, pcb->current)))
				next = find_labelled_lcb(goto_label, pcb->current, pcb->last);
			break;
	}
	if (next)
	{
		pcb->current = next;
		return (0);
	}
	pprintf("goto/gosub label not found: %s\n", goto_label);
	return (eFATAL_ALREADY);

}							 /* end of execute_goto */

/*+-------------------------------------------------------------------------
	show_error_position(pcb)
cursor MUST be at left margin when this is called
--------------------------------------------------------------------------*/
void
show_error_position(pcb)
PCB *pcb;
{
	ESD *tesd = pcb->current->text;
	int itmp = tesd->old_index;
	char tag[64];

	sprintf(tag, "%s %u> ", pcb->argv[0], pcb->current->lineno);
	pputs(tag);
	pputs(tesd->pb);
	pputs("\n");
	itmp = strlen(tag) + tesd->old_index;
	while (itmp--)
		pputc(' ');
	pputs("^\n");

}							 /* end of show_error_position */

/*+-------------------------------------------------------------------------
	find_proc_cmd(cmd_list,cmd)
--------------------------------------------------------------------------*/
P_CMD *
find_proc_cmd(cmd_list, cmd)
P_CMD *cmd_list;
char *cmd;
{
	while (cmd_list->token != -1)
	{
		if (!strcmp(cmd_list->cmd, cmd))
			break;
		cmd_list++;
	}
	return ((cmd_list->token == -1) ? (P_CMD *) 0 : cmd_list);

}							 /* end of find_proc_cmd */

/*+-------------------------------------------------------------------------
	execute_esd(tesd)
--------------------------------------------------------------------------*/
int
execute_esd(tesd)
ESD *tesd;
{
	int erc;
	P_CMD *pcmd;
	static P_CMD *set_pcmd = (P_CMD *) 0;	/* quick access to 'set' */
	char cmd[32];
	extern int proc_interrupt;

	/* if interrupt, exit */
	if (ck_sigint() | proc_interrupt)
		return (eCONINT);

	/* if blank, skip it */
	if (skip_cmd_break(tesd))
		return (0);

	/* if comment, skip it */
	if (!skip_cmd_char(tesd, '#'))
		return (0);

	if (*(tesd->pb + tesd->index) == '{')
	{
		pputs("invalid '{'\n");
		return (eFATAL_ALREADY);
	}

	while (1)
	{
		/* get command -- allow leading '$' to assume 'set' command */
		if (*(tesd->pb + tesd->index) == '#')
			break;
		if (*(tesd->pb + tesd->index) == '$')
		{
			/* find 'set' in the list -- save for rapid access later */
			if (set_pcmd)
				pcmd = set_pcmd;
			else if (!(pcmd = find_proc_cmd(icmd_cmds, "set")))
				return (eInternalLogicError);
			else
				set_pcmd = pcmd;
		}
		else
		{
			if (get_alphanum_zstr(tesd, cmd, sizeof(cmd)))
				return (eIllegalCommand);
			/* find it in the list */
			if ((pcmd = find_proc_cmd(icmd_cmds, cmd)) == (P_CMD *) 0)
				return (eIllegalCommand);
		}

		/* check to see if this command available for procedure */
		if (!pcmd->proc)
			return (eInteractiveCmd);

		/* execute the command */
		if (erc = (*pcmd->proc) (tesd))
			return (erc);

		/* look for comment */
		if (!skip_cmd_char(tesd, '#'))
			break;

		/* look for multiple commands on line */
		if (skip_cmd_char(tesd, ';'))
			break;

		/* if blank after ';', skip it */
		if (skip_cmd_break(tesd))
			break;
	}
	return (0);

}							 /* end of execute_esd */

/*+-------------------------------------------------------------------------
	execute_labelled_lcb(tesd)
--------------------------------------------------------------------------*/
execute_labelled_lcb(tesd)
ESD *tesd;
{
	int itmp = 0;
	int cb = tesd->cb;
	char *pb = tesd->pb;

	/* reset indices */
	tesd->index = itmp;
	tesd->old_index = itmp;

	/* if comment, skip it */
	if (!skip_cmd_char(tesd, '#'))
		return (0);

	/* skip over any label */
	while (!isspace((uchar) * (pb + itmp)) && (itmp < cb))
		itmp++;
	tesd->index = itmp;
	tesd->old_index = itmp;

	return (execute_esd(tesd));
}							 /* end of execute_labelled_lcb */

/*+-------------------------------------------------------------------------
	dump_proc(pcb)
--------------------------------------------------------------------------*/
#if 0
void
dump_proc(pcb)
PCB *pcb;
{
	int itmp;
	LCB *lcb;

	pprintf("------ pcb @ 0x%08lx -----------------\n", pcb);
	pprintf("argc=%d first=0x%08lx last=0x%08lx\n", pcb->argc,
		pcb->first, pcb->last);
	for (itmp = 0; itmp < pcb->argc; itmp++)
	{
		pprintf("argv(%d) @ 0x%lx: '%s'\n", itmp, pcb->argv[itmp],
			pcb->argv[itmp]);
	}
	pputs("\n");
	lcb = pcb->first;
	while (lcb)
	{
		pprintf("lcb @ 0x%08lx   lineno=%u\n", lcb, lcb->lineno);
		pputs("\n");
		lcb = lcb->next;
	}
	pflush();
}							 /* end of dump_proc */
#endif

/*+-------------------------------------------------------------------------
	trace_proc_cmd(pcb) - if asked, show command
--------------------------------------------------------------------------*/
void
trace_proc_cmd(pcb)
PCB *pcb;
{
	if (proc_trace)
	{
		pprintf("%s %u> ", pcb->argv[0], pcb->current->lineno);
		pputs(pcb->current->text->pb);
		pputc('\n');
	}

}							 /* end of trace_proc_cmd */

/*+-------------------------------------------------------------------------
	proc_dcdloss_handler(pcb) - a statement execution found DCD loss
--------------------------------------------------------------------------*/
int
proc_dcdloss_handler(pcb)
PCB *pcb;
{
	int erc = 0;
	int itmp;
	ESD esdcopy;
	ESD *tesd;

	if (pcb->upon_dcdloss.pb)
	{
		esdcopy = pcb->upon_dcdloss;	/* a copy to preserve pcb->index */
		tesd = &esdcopy;
		if (proc_trace)
		{
			pprintf("%s DCDLOSS> ", pcb->argv[0]);
			pputs(tesd->pb + tesd->index);
			pputc('\n');
		}
		if (erc = execute_esd(tesd))
		{
			if (erc != eFATAL_ALREADY)
				proc_error(erc);
			pprintf("error in 'upon dcdloss' statement\n");
			pputs(tesd->pb + pcb->upon_dcdloss.index);
			pputs("\n");
			itmp = tesd->old_index - pcb->upon_dcdloss.index;;
			while (itmp--)
				pputc(' ');
			pputs("^\ninvoked while executing:\n");
			erc = eFATAL_ALREADY;
		}
	}
	else
		/* DCD watch enabled but no 'upon dcdloss' in effect */
	{
		pprintf("Connection terminated during procedure execution\n");
		pputs("while executing:\n");
		erc = eFATAL_ALREADY;
	}
	return (erc);
}							 /* end of proc_dcdloss_handler */

/*+-------------------------------------------------------------------------
	execute_proc(pcb,use_goto_label) - execute a memory-resident procedure
--------------------------------------------------------------------------*/
int
execute_proc(pcb, use_goto_label)
PCB *pcb;
int use_goto_label;
{
	int erc = 0;
	extern int proc_interrupt;

	/*
	 * if first level, revert to keeping "local" variables created in
	 * nested procedures
	 */
	if (!proc_level)
		proc_option_localvars = 0;

	/*
	 * nested too deeply?
	 */
	if (proc_level == PROC_STACK_MAX)
		return (eProcStackTooDeep);

	/*
	 * nest procedure level
	 */
	pcb_stack[proc_level++] = pcb;
	if (use_goto_label)
	{
		if (erc = execute_goto(pcb, use_goto_label))
			return (erc);
	}
	else
		pcb->current = pcb->first;

	/*
	 * mark variable "stack frame"
	 */
	mkv_proc_starting(pcb);

	/*
	 * walk the procedure step by step while Turing watches
	 */
	while (pcb->current)
	{
		/* execute the command */
		trace_proc_cmd(pcb);
		if (erc = execute_labelled_lcb(pcb->current->text))
		{
			/* handle other classes of errors */
			switch (erc & 0xF000)
			{
				case e_WARNING:	/* warning */
					erc = 0;
					break;

				case e_FATAL:	/* fatal */
					goto FUNC_RETURN;

				case e_ProcAttn:	/* proc attention */
					switch (erc)
					{
						case eProcAttn_GOTO:
						case eProcAttn_GOTOB:
							if (erc = execute_goto(pcb, erc))
								break;	/* didn't find it */
							continue;	/* pcb->current is now goto target */

						case eProcAttn_RETURN:
							erc = 0;
							break;

						case eProcAttn_Interrupt:
						case eProcAttn_ESCAPE:
							pprintf(
								"procedure %s interrupted.\n", pcb->argv[0]);
							erc = eFATAL_ALREADY;
							break;

						case eProcAttn_DCDloss:
							erc = proc_dcdloss_handler(pcb);
							break;

						default:
							pprintf("procedure error 0x%x\n", erc);
							erc = eFATAL_ALREADY;
							break;
					}
					goto FUNC_RETURN;

				default:	 /* must be proc return error code */
					goto FUNC_RETURN;
			}
		}

		if (ck_sigint() || proc_interrupt)
		{
			proc_interrupt = 0;
			sigint = 0;
			pprintf("procedure %s interrupted\n", pcb->argv[0]);
			erc = eFATAL_ALREADY;
		}

		if (erc)
			break;
		pcb->current = pcb->current->next;
	}

	/*
	 * all exits through here
	 */

  FUNC_RETURN:

	/*
	 * we want to clean up the variable stack here (delete variables
	 * created by the exiting procedure level) IF AND ONLY IF:
	 * 
	 * 1. this is a real procedure exiting (and not just a gosub) 2. the user
	 * has requested the feature with 'option localvars'
	 */
	if (!use_goto_label || proc_option_localvars)
		mkv_proc_terminating(pcb);

	/*
	 * handle errors: any non-zero value will terminate procedure
	 * execution precipitously by unraveling the stack at every level
	 * right here, spilling diagnostics as we recede to level 0
	 */
	if (erc)
	{
		if ((erc > 0) && (erc < e_USER))
		{
			pprintf(">>procedure %s returned %d\n", pcb->argv[0], erc);
			erc |= e_USER;
		}
		else if ((erc > e_USER) && (erc <= 0x1FFF))
		{
			;				 /* already said it */
		}
		else
		{
			if (erc != eFATAL_ALREADY)
			{
				proc_error(erc);
				erc = eFATAL_ALREADY;
			}
			show_error_position(pcb);
		}
	}

	/*
	 * un-nest a level (returning from a gosub or exiting a procedure
	 */
	pcb_stack[--proc_level] = (PCB *) 0;

	/*
	 * if last procedure is exiting, make sure any opened files are closed
	 */
	if (!proc_level)
		proc_file_reset();

	return (erc);

}							 /* end of execute_proc */

/*+-------------------------------------------------------------------------
	free_lcb_chain(lcb)
--------------------------------------------------------------------------*/
void
free_lcb_chain(lcb)
LCB *lcb;
{
	LCB *plcb;

	while (lcb)
	{
		if (lcb->text)
			esdfree(lcb->text);
		plcb = lcb;
		lcb = lcb->next;
		free((char *)plcb);
	}

}							 /* end of free_lcb_chain */

/*+-------------------------------------------------------------------------
	find_procedure(name) - find procedure if it exists
--------------------------------------------------------------------------*/
char *
find_procedure(name)
char *name;
{
	static char procpath[ECU_MAXPN];

	/*
	 * try to find proc file in current directory
	 */
	strcpy(procpath, name);
	strcat(procpath, ".ep");
	if (!access(procpath, 4))
		goto RETURN_PATH;
	if (proc_trace && (errno != ENOENT))
		pperror(procpath);

	/*
	 * try to find proc file in home .ecu subdirectory
	 */
	get_home_dir(procpath);
	strcat(procpath, "/.ecu/");
	strcat(procpath, name);
	strcat(procpath, ".ep");
	if (!access(procpath, 4))
		goto RETURN_PATH;
	if (proc_trace && (errno != ENOENT))
		pperror(procpath);

	/*
	 * try to find proc file in library ep subdirectory
	 */
	strcpy(procpath, CFG_EcuLibDir);
	strcat(procpath, "/ep/");
	strcat(procpath, name);
	strcat(procpath, ".ep");
	if (!access(procpath, 4))
		goto RETURN_PATH;
	if (proc_trace && (errno != ENOENT) && (errno != ENOTDIR))
		pperror(procpath);

	/*
	 * no luck
	 */
	return (0);

	/*
	 * luck -- hack away //
	 */
  RETURN_PATH:
	if ((procpath[0] == '/') && (procpath[1] == '/'))
		return (procpath + 1);
	return (procpath);

}							 /* end of find_procedure */

/*+-------------------------------------------------------------------------
	do_proc(argc,argv) - read in a disk-based procedure and execute it

  Acute and particular attention has been paid to the order of
  memory allocation and object linking in this function; if any
  error occurs, branching to FUNC_EXIT with erc non-zero
  guarrantees proper unravelling/freeing of buffers allocated by
  this pass through.
--------------------------------------------------------------------------*/
do_proc(argc, argv)
int argc;
char **argv;
{
	int itmp;
	int itmp2;
	int erc;
	int iargv;
	char *pargv[MAX_PARGV];
	int ipargv = 0;
	char cmdbuf[ESD_NOMSZ];
	char *procpath;
	FILE *fp = 0;
	PCB *pcb = (PCB *) 0;
	LCB *lcb = (LCB *) 0;
	LCB *plcb;
	UINT16 line_count = 0;
	extern UINT32 colors_current;
	UINT32 colors_at_entry = colors_current;
	extern int proc_interrupt;

	proc_interrupt = 0;		 /* ok to reset here because no one ... */
	sigint = 0;				 /* ... would call here if interrupted */

	for (iargv = 0; iargv < argc; iargv++)
	{
		if (ipargv == MAX_PARGV)
		{
			pputs("\nMax arguments to procedure invocation exceeded\n");
			erc = eFATAL_ALREADY;
			goto FUNC_RETURN;
		}
		pargv[ipargv++] = argv[iargv];
	}

	if (!ipargv)
	{
		pputs("\nno procedure name given\n");
		erc = eFATAL_ALREADY;
		goto FUNC_RETURN;
	}

	if (!(procpath = find_procedure(pargv[0])))
	{
		pprintf("\nprocedure %s not found\n", pargv[0]);
		erc = eFATAL_ALREADY;
		goto FUNC_RETURN;
	}
	fp = fopen(procpath, "r");
	if (!fp)
	{
		pperror(procpath);
		erc = eFATAL_ALREADY;
		goto FUNC_RETURN;
	}
	if (proc_trace)
		pprintf("DO: %s\n", procpath);

	if (!(pcb = (PCB *) calloc(1, sizeof(PCB))))
	{
		erc = eNoMemory;
		goto FUNC_RETURN;
	}

	pcb->argv = pargv;
	pcb->argc = ipargv;

	plcb = 0;
	line_count = 0;
	while (1)
	{

		/*
		 * read procedure file
		 */
		if (!(fgets(cmdbuf, sizeof(cmdbuf), fp)))
			break;
		line_count++;

		/*
		 * housekeeping
		 */
		itmp = strlen(cmdbuf) - 1;	/* skip blank lines */
		if (!itmp)
			continue;
		cmdbuf[itmp] = 0;	 /* kill trailing NL */
		if (cmdbuf[0] == '#')/* skip comments */
			continue;

		/*
		 * convert tabs to spaces so we don't have to scan for each
		 */
		for (itmp2 = 0; itmp2 < itmp; itmp2++)
		{
			if (cmdbuf[itmp2] == TAB)
				cmdbuf[itmp2] = SPACE;
		}

		/*
		 * get a line control block
		 */
		if (!(lcb = (LCB *) malloc(sizeof(LCB))))
		{
			erc = eNoMemory;
			goto FUNC_RETURN;
		}

		/*
		 * link it into the pcb chain
		 */
		lcb->prev = plcb;
		lcb->next = 0;
		lcb->lineno = line_count;

		if (plcb)
			plcb->next = lcb;
		else
			pcb->first = lcb;

		/*
		 * now copy in the text
		 */
		if (!(lcb->text = esdalloc(itmp)))
		{
			erc = eNoMemory;
			goto FUNC_RETURN;
		}
		strcpy(lcb->text->pb, cmdbuf);
		lcb->text->cb = itmp;
		esd_null_terminate(lcb->text);
		plcb = lcb;
		pcb->last = lcb;
		lcb = 0;
	}

	/*
	 * error or not, all done reading the proc; if no error, execute the
	 * procedure
	 */
	if (line_count)
	{
		fclose(fp);
		fp = 0;
		erc = execute_proc(pcb, 0);
	}
	else
		erc = eProcEmpty;

  FUNC_RETURN:
	if (fp)
		fclose(fp);
	if (lcb)
	{
		if (lcb->text)
			esdfree(lcb->text);
		free((char *)lcb);
	}
	if (pcb)
	{
		if (pcb->first)
			free_lcb_chain(pcb->first);
		free((char *)pcb);
	}
	if ((erc > e_USER) && (erc <= 0x1FFF))
		erc -= e_USER;
	if (erc > e_USER)
		setcolor(colors_at_entry);
	return (erc);

}							 /* end of do_proc */

/*+-------------------------------------------------------------------------
	pcmd_do(param)
--------------------------------------------------------------------------*/
pcmd_do(param)
ESD *param;
{
	int erc;
	int ipargv;
	char *cmd_copy;
	char *pargv[MAX_PARGV];
	ESD *pargv_esd[MAX_PARGV];
	int pargc = 0;

	if (!(cmd_copy = (char *)malloc(param->cb)))
		return (eNoMemory);
	strcpy(cmd_copy, param->pb + param->old_index);
	while (pargc != MAX_PARGV)
	{
		if (end_of_cmd(param))
			break;
		if (!(pargv_esd[pargc] = esdalloc(ESD_NOMSZ)))
		{
			erc = eNoMemory;
			goto FUNC_RETURN;
		}
		if (erc = gstr(param, pargv_esd[pargc], 1))
			goto FUNC_RETURN;
		pargv[pargc] = pargv_esd[pargc]->pb;
		pargc++;
	}

	if (pargc < MAX_PARGV)
		erc = do_proc(pargc, pargv);
	else
	{
		pprintf("too many arguments to procedure\n");
		erc = eFATAL_ALREADY;
	}

  FUNC_RETURN:
	free(cmd_copy);
	for (ipargv = 0; ipargv < pargc; ipargv++)
		esdfree(pargv_esd[ipargv]);
	return (erc);

}							 /* end of pcmd_do */

/* vi: set tabstop=4 shiftwidth=4: */
/* end of proc.c */
