/*+-------------------------------------------------------------------------
	pcmdif.c - ecu if procedure commands
	wht@wht.net

  Defined functions:
	_cmd_ifrel_common(param, relop)
	_if_common(param, truth)
	get_logicop(param, op_returned)
	get_relop(param, op_returned)
	get_truth_int(param, truth)
	get_truth_str(param, truth)
	pcmd_else(param)
	pcmd_ifge(param)
	pcmd_ifgt(param)
	pcmd_ifi(param)
	pcmd_ifle(param)
	pcmd_iflt(param)
	pcmd_ifnz(param)
	pcmd_ifs(param)
	pcmd_ifz(param)
	test_truth_int(int1, relop, int2)

--------------------------------------------------------------------------*/
/*+:EDITS:*/
/*:04-26-2000-11:16-wht@bob-RELEASE 4.42 */
/*:01-24-1997-02:38-wht@yuriatin-SOURCE RELEASE 4.00 */
/*:09-15-1996-06:40-root@n4hgf-"else <cmd>" executed bass-ackwards */
/*:09-11-1996-20:00-wht@yuriatin-3.48-major telnet,curses,structural overhaul */
/*:11-23-1995-11:20-wht@kepler-source control 3.37 for tsx-11 */
/*:11-14-1995-10:23-wht@kepler-3.37.80-source control point: SOCKETS */
/*:05-04-1994-04:39-wht@n4hgf-ECU release 3.30 */
/*:12-20-1992-00:25-wht@n4hgf-flunk << or >> as relational operator */
/*:11-14-1992-22:37-wht@n4hgf-multiple else loses track of truth */
/*:09-10-1992-14:00-wht@n4hgf-ECU release 3.20 */
/*:08-22-1992-15:39-wht@n4hgf-ECU release 3.20 BETA */
/*:07-25-1991-12:59-wht@n4hgf-ECU release 3.10 */
/*:08-26-1990-22:23-wht@n4hgf-fix zero-relative if commands */
/*:08-14-1990-20:40-wht@n4hgf-ecu3.00-flush old edit history */

#include <ctype.h>
#include "ecu.h"
#include "ecuerror.h"
#include "esd.h"
#include "var.h"
#include "procedure.h"
#include "relop.h"

extern PCB *pcb_stack[];

#define MAX_IF 40			 /* damn enough */
uchar if_level = 0;
uchar truth_already[MAX_IF];

/*+-------------------------------------------------------------------------
    get_relop(param,&op_returned)
--------------------------------------------------------------------------*/
int
get_relop(param, op_returned)
ESD *param;
int *op_returned;
{
	if (end_of_cmd(param))
		return (eInvalidRelOp);

	switch (param->pb[param->index++])	/* index decremented in default */
	{
		case '=':
			if ((param->cb != param->index) && (param->pb[param->index] == '='))
				param->index++;
			*op_returned = OP_EQ;
			return (0);

		case '!':
			if (param->cb == param->index)
				return (eInvalidRelOp);
			switch (param->pb[param->index])
			{
				case '=':
					param->index++;
					*op_returned = OP_NE;
					return (0);
				default:
					return (eInvalidRelOp);
			}

		case '<':
			if (param->cb == param->index)
			{
				*op_returned = OP_LT;
				return (0);
			}
			switch (param->pb[param->index])
			{
				case '<':
					param->index++;
					return (eInvalidRelOp);
				case '>':
					param->index++;
					*op_returned = OP_NE;
					return (0);
				case '=':
					param->index++;
					*op_returned = OP_LE;
					return (0);
				default:
					*op_returned = OP_LT;
					return (0);
			}

		case '>':
			if (param->cb == param->index)
			{
				*op_returned = OP_LT;
				return (0);
			}
			switch (param->pb[param->index])
			{
				case '>':
					param->index++;
					return (eInvalidRelOp);
				case '=':
					param->index++;
					*op_returned = OP_GE;
					return (0);
				default:
					*op_returned = OP_GT;
					return (0);
			}
		default:
			param->index--;
	}
	return (eInvalidRelOp);
}							 /* end of get_relop */

/*+-------------------------------------------------------------------------
	get_logicop(param,op_returned)
--------------------------------------------------------------------------*/
int
get_logicop(param, op_returned)
ESD *param;
int *op_returned;
{
	int erc;
	char *cp;

	if (erc = skip_cmd_break(param))
		return (eInvalidLogicOp);

	if ((param->cb - param->index) < 2)
		return (eInvalidLogicOp);

	cp = param->pb + param->index;
	erc = eInvalidLogicOp;
	if (!strncmp(cp, "&&", 2))
	{
		*op_returned = OP_AND;
		erc = 0;
	}
	else if (!strncmp(cp, "||", 2))
	{
		*op_returned = OP_OR;
		erc = 0;
	}
	if (!erc)
		param->index += 2;
	return (erc);

}							 /* end of get_logicop */

/*+-------------------------------------------------------------------------
	test_truth_int(int1,relop,int2)
--------------------------------------------------------------------------*/
int
test_truth_int(int1, relop, int2)
long int1;
int relop;
long int2;
{
	int truth = 0;

	switch (relop)
	{
		case OP_EQ:
			truth = (int1 == int2);
			break;
		case OP_NE:
			truth = (int1 != int2);
			break;
		case OP_GT:
			truth = (int1 > int2);
			break;
		case OP_LT:
			truth = (int1 < int2);
			break;
		case OP_GE:
			truth = (int1 >= int2);
			break;
		case OP_LE:
			truth = (int1 <= int2);
			break;
	}
	return (truth);

}							 /* end of test_truth_int */

/*+-------------------------------------------------------------------------
	get_truth_int(param,truth)
--------------------------------------------------------------------------*/
int
get_truth_int(param, truth)
ESD *param;
int *truth;
{
	int erc;
	long int1;
	long int2;
	int operator;
	int truth2;

	if (erc = gint(param, &int1))
		return (erc);
	if (erc = get_relop(param, &operator))
		return (erc);
	if (erc = gint(param, &int2))
		return (erc);
	*truth = test_truth_int(int1, operator, int2);

	while (!get_logicop(param, &operator))
	{
		if (erc = get_truth_int(param, &truth2))
			return (erc);
		switch (operator)
		{
			case OP_AND:
				*truth &= truth2;
				break;

			case OP_OR:
				*truth |= truth2;
				break;
		}
	}
	return (0);

}							 /* end of get_truth_int */

/*+-------------------------------------------------------------------------
    get_truth_str(param,truth)
--------------------------------------------------------------------------*/
int
get_truth_str(param, truth)
ESD *param;
int *truth;
{
	int erc;
	ESD *tesd1 = (ESD *) 0;
	ESD *tesd2 = (ESD *) 0;
	int operator;
	int strcmp_result;
	int truth2;

	if (!(tesd1 = esdalloc(ESD_NOMSZ)) || !(tesd2 = esdalloc(ESD_NOMSZ)))
	{
		erc = eNoMemory;
		goto FUNC_RETURN;
	}

	if (erc = gstr(param, tesd1, 1))
		goto FUNC_RETURN;
	if (erc = get_relop(param, &operator))
		goto FUNC_RETURN;
	if (erc = gstr(param, tesd2, 1))
		goto FUNC_RETURN;

	strcmp_result = strcmp(tesd1->pb, tesd2->pb);

	switch (operator)
	{
		case OP_EQ:
			*truth = (strcmp_result == 0);
			break;
		case OP_NE:
			*truth = (strcmp_result != 0);
			break;
		case OP_GT:
			*truth = (strcmp_result > 0);
			break;
		case OP_LT:
			*truth = (strcmp_result < 0);
			break;
		case OP_GE:
			*truth = (strcmp_result >= 0);
			break;
		case OP_LE:
			*truth = (strcmp_result <= 0);
			break;
		default:
			return (eInvalidStrOp);
	}

	while (!get_logicop(param, &operator))
	{
		if (erc = get_truth_str(param, &truth2))
			return (erc);
		switch (operator)
		{
			case OP_AND:
				*truth &= truth2;
				break;

			case OP_OR:
				*truth |= truth2;
				break;
		}
	}

	erc = 0;

  FUNC_RETURN:
	if (tesd1)
		esdfree(tesd1);
	if (tesd2)
		esdfree(tesd2);
	return (erc);

}							 /* end of get_truth_str */

/*+-------------------------------------------------------------------------
	_if_common(param,truth)
--------------------------------------------------------------------------*/
int
_if_common(param, truth)
ESD *param;
int truth;
{
	int erc = 0;
	char s80[80];
	PCB *pcb;
	ESD *else_line;
	int label_on_else_line;
	int truth2;
	int save_index;
	long int1;

	if (proc_trace > 1)
	{
		pprintf("if condition %s", (truth) ? "TRUE: " : "FALSE\n");
		if (truth)
		{
			skip_cmd_break(param);
			pputs(param->pb + param->index);
			pputc('\n');
		}
	}

	truth_already[if_level] |= truth;

/* if end of command, execute frame else conditionally execute rest of esd */
	s80[0] = 0;
	if (end_of_cmd(param))
		erc = execute_frame(truth);
	else if (truth)
		erc = execute_esd(param);
	else
		param->index = param->cb;

	if (erc)
		return (erc);

/* check for else statement */
	pcb = pcb_stack[proc_level - 1];
	if (!pcb->current->next) /* if no next line, no "else" */
		return (0);

	else_line = pcb->current->next->text;
	else_line->index = else_line->old_index = 0;
	if (label_on_else_line = (*else_line->pb != 0x20))
	{						 /* strip label */
		if (get_alphanum_zstr(else_line, s80, sizeof(s80)))
			return (eInvalidLabel);
	}
	if (get_alphanum_zstr(else_line, s80, sizeof(s80)))
		return (0);			 /* not "else" */
	if (strcmp(s80, "else"))
		return (0);			 /* not "else" */
	if (label_on_else_line)
	{
		else_line->old_index = 0;
		pputs("label not allowed on else statement\n");
		return (eFATAL_ALREADY);
	}

/* we have an "else" condition */
	pcb->current = pcb->current->next;

	trace_proc_cmd(pcb);

	if (end_of_cmd(else_line))
	{
		truth = !truth_already[if_level];
		erc = execute_frame(truth);
	}
	else
	{
		save_index = else_line->old_index = else_line->index;
		s80[0] = 0;
		if ((*(else_line->pb + else_line->index) != '$') &&
			get_alpha_zstr(else_line, s80, sizeof(s80)))
		{
			pputs("illegal command after 'else'\n");
			return (eFATAL_ALREADY);
		}
		if (!strcmp(s80, "ifi"))
		{
			if (erc = get_truth_int(else_line, &truth2))
				return (erc);
			erc = _if_common(else_line, !truth_already[if_level] & truth2);
			truth_already[if_level] |= truth2;
		}
		else if (!strcmp(s80, "ifs"))
		{
			if (erc = get_truth_str(else_line, &truth2))
				return (erc);
			erc = _if_common(else_line, !truth_already[if_level] & truth2);
			truth_already[if_level] |= truth2;
		}
		else if (!strcmp(s80, "ifz"))
		{
			if (erc = gint(else_line, &int1))
				return (erc);
			truth2 = test_truth_int(int1, OP_EQ, 0L);
			erc = _if_common(else_line, !truth_already[if_level] & truth2);
			truth_already[if_level] |= truth2;
		}
		else if (!strcmp(s80, "ifnz"))
		{
			if (erc = gint(else_line, &int1))
				return (erc);
			truth2 = test_truth_int(int1, OP_NE, 0L);
			erc = _if_common(else_line, !truth_already[if_level] & truth2);
			truth_already[if_level] |= truth2;
		}
		else if (!strcmp(s80, "iflt"))
		{
			if (erc = gint(else_line, &int1))
				return (erc);
			truth2 = test_truth_int(int1, OP_LT, 0L);
			erc = _if_common(else_line, !truth_already[if_level] & truth2);
			truth_already[if_level] |= truth2;
		}
		else if (!strcmp(s80, "ifle"))
		{
			if (erc = gint(else_line, &int1))
				return (erc);
			truth2 = test_truth_int(int1, OP_LE, 0L);
			erc = _if_common(else_line, !truth_already[if_level] & truth2);
			truth_already[if_level] |= truth2;
		}
		else if (!strcmp(s80, "ifgt"))
		{
			if (erc = gint(else_line, &int1))
				return (erc);
			truth2 = test_truth_int(int1, OP_GT, 0L);
			erc = _if_common(else_line, !truth_already[if_level] & truth2);
			truth_already[if_level] |= truth2;
		}
		else if (!strcmp(s80, "ifge"))
		{
			if (erc = gint(else_line, &int1))
				return (erc);
			truth2 = test_truth_int(int1, OP_GE, 0L);
			erc = _if_common(else_line, !truth_already[if_level] & truth2);
			truth_already[if_level] |= truth2;
		}
		else if (!strncmp(s80, "while", 5))
		{
			pputs("'while' command not allowed as 'else' conditional\n");
			pputs("put the statement inside braces\n");
			return (eFATAL_ALREADY);
		}
		else
		{
			else_line->index = save_index;
			if (!truth)
				erc = execute_esd(else_line);
		}
	}

	return (erc);
}							 /* end of _if_common */

/*+-------------------------------------------------------------------------
    pcmd_ifi(param)
--------------------------------------------------------------------------*/
int
pcmd_ifi(param)
ESD *param;
{
	int erc;
	int truth;

	if (!proc_level)
		return (eNotExecutingProc);

	if (if_level == MAX_IF)
	{
		pputs("if statements nested too deeply\n");
		return (eFATAL_ALREADY);
	}
	if_level++;
	truth_already[if_level] = 0;

	if (!(erc = get_truth_int(param, &truth)))
		erc = _if_common(param, truth);
	if_level--;
	return (erc);

}							 /* end of pcmd_ifi */

/*+-------------------------------------------------------------------------
	_cmd_ifrel_common(param,relop)
--------------------------------------------------------------------------*/
int
_cmd_ifrel_common(param, relop)
ESD *param;
int relop;
{
	int erc;
	int truth;
	long int1;

	if (!proc_level)
		return (eNotExecutingProc);

	if (if_level == MAX_IF)
	{
		pputs("if statements nested too deeply\n");
		return (eFATAL_ALREADY);
	}
	if_level++;
	truth_already[if_level] = 0;

	if (erc = gint(param, &int1))
		return (erc);
	truth = test_truth_int(int1, relop, 0L);
	erc = _if_common(param, truth);
	if_level--;
	return (erc);

}							 /* end of _cmd_ifrel_common */

/*+-------------------------------------------------------------------------
	pcmd_ifz(param)
--------------------------------------------------------------------------*/
int
pcmd_ifz(param)
ESD *param;
{
	return (_cmd_ifrel_common(param, OP_EQ));
}							 /* end of pcmd_ifz */

/*+-------------------------------------------------------------------------
	pcmd_ifnz(param)
--------------------------------------------------------------------------*/
int
pcmd_ifnz(param)
ESD *param;
{
	return (_cmd_ifrel_common(param, OP_NE));
}							 /* end of pcmd_ifnz */

/*+-------------------------------------------------------------------------
	pcmd_ifle(param)
--------------------------------------------------------------------------*/
int
pcmd_ifle(param)
ESD *param;
{
	return (_cmd_ifrel_common(param, OP_LE));
}							 /* end of pcmd_ifle */

/*+-------------------------------------------------------------------------
	pcmd_ifge(param)
--------------------------------------------------------------------------*/
int
pcmd_ifge(param)
ESD *param;
{
	return (_cmd_ifrel_common(param, OP_GE));
}							 /* end of pcmd_ifge */

/*+-------------------------------------------------------------------------
	pcmd_iflt(param)
--------------------------------------------------------------------------*/
int
pcmd_iflt(param)
ESD *param;
{
	return (_cmd_ifrel_common(param, OP_LT));
}							 /* end of pcmd_iflt */

/*+-------------------------------------------------------------------------
	pcmd_ifgt(param)
--------------------------------------------------------------------------*/
int
pcmd_ifgt(param)
ESD *param;
{
	return (_cmd_ifrel_common(param, OP_GT));
}							 /* end of pcmd_ifgt */

/*+-------------------------------------------------------------------------
    pcmd_ifs(param)
--------------------------------------------------------------------------*/
int
pcmd_ifs(param)
ESD *param;
{
	int erc;
	int truth;

	if (!proc_level)
		return (eNotExecutingProc);

	if (if_level == MAX_IF)
	{
		pputs("if statements nested too deeply\n");
		return (eFATAL_ALREADY);
	}
	if_level++;
	truth_already[if_level] = 0;

	if (!(erc = get_truth_str(param, &truth)))
		erc = _if_common(param, truth);
	if_level--;
	return (erc);

}							 /* end of pcmd_ifs */

/*+-------------------------------------------------------------------------
	pcmd_else(param)
--------------------------------------------------------------------------*/
/*ARGSUSED*/
int
pcmd_else(param)
ESD *param;
{
	param = 0; /* unused */
	return (eElseCommand);
}							 /* end of pcmd_else */

/* vi: set tabstop=4 shiftwidth=4: */
/* end of pcmdif.c */
