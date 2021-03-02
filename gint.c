/*+-------------------------------------------------------------------------
    gint.c - ecu get integer parameter functions
	wht@wht.net

  Defined functions:
	gcol_range(param, col1, col2)
	gint(param, int_returned)
	gint_base(param, value)
	gint_constant(param, value)
	gintop(param, intop)

--------------------------------------------------------------------------*/
/*+:EDITS:*/
/*:04-26-2000-11:15-wht@bob-RELEASE 4.42 */
/*:01-24-1997-02:37-wht@yuriatin-SOURCE RELEASE 4.00 */
/*:10-28-1996-13:57-wht@yuriatin-support OP_LAND (&&) and OP_LOR (||) */
/*:09-11-1996-20:00-wht@yuriatin-3.48-major telnet,curses,structural overhaul */
/*:08-20-1996-12:39-wht@kepler-locale/ctype fixes from ache@nagual.ru */
/*:11-23-1995-11:20-wht@kepler-source control 3.37 for tsx-11 */
/*:11-14-1995-10:23-wht@kepler-3.37.80-source control point: SOCKETS */
/*:05-04-1994-04:39-wht@n4hgf-ECU release 3.30 */
/*:12-20-1992-00:13-wht@n4hgf-add shift operators */
/*:12-12-1992-13:39-wht@n4hgf-use relop.h for OP_ definitions */
/*:09-10-1992-13:59-wht@n4hgf-ECU release 3.20 */
/*:08-22-1992-15:39-wht@n4hgf-ECU release 3.20 BETA */
/*:07-25-1991-12:58-wht@n4hgf-ECU release 3.10 */
/*:01-31-1991-16:50-wht@n4hgf-reinstate octal with 0o prefix */
/*:01-09-1991-22:31-wht@n4hgf-ISC port */
/*:08-14-1990-20:40-wht@n4hgf-ecu3.00-flush old edit history */

#include "ecu.h"
#include "ecuerror.h"
#include "esd.h"
#include "var.h"
#include "relop.h"

#define BASE_DEC	1
#define BASE_OCT	2
#define BASE_HEX  	3

/*+-------------------------------------------------------------------------
    gint_constant(param,int_returned) - evaluate integer constant
--------------------------------------------------------------------------*/
int
gint_constant(param, value)
ESD *param;
long *value;
{
	int itmp;
	int base = BASE_DEC;
	int erc;
	long new_value;

	if (erc = skip_cmd_break(param))
		return (erc);
	esd_null_terminate(param);

/* get integer from string */
	if ((!strncmp(param->pb + param->index, "0x", 2)) ||
		(!strncmp(param->pb + param->index, "0X", 2)))
	{
		base = BASE_HEX;
		param->index += 2;
	}
	else if ((!strncmp(param->pb + param->index, "0o", 2)) ||
		(!strncmp(param->pb + param->index, "0O", 2)))
	{
		base = BASE_OCT;
		param->index += 2;
	}

	param->old_index = param->index;
	switch (base)
	{
		case BASE_HEX:
			sscanf(param->pb + param->index, "%lx", &new_value);
			itmp = param->index + strspn(param->pb + param->index,
				"0123456789ABCDEFabcdef");
			erc = eInvalidHexNumber;
			break;
		case BASE_DEC:
			sscanf(param->pb + param->index, "%ld", &new_value);
			itmp = param->index + strspn(param->pb + param->index, "0123456789");
			erc = eInvalidDecNumber;
			break;
		case BASE_OCT:
			sscanf(param->pb + param->index, "%lo", &new_value);
			itmp = param->index + strspn(param->pb + param->index, "01234567");
			erc = eInvalidOctNumber;
			break;
		default:
			return (eInternalLogicError);
	}

	param->index = itmp;
	if (isalnum((uchar) * (param->pb + itmp)))
		param->old_index = itmp;

	if (param->old_index != param->index)
	{
		*value = new_value;
		return (0);
	}
	return (erc);

}							 /* end of gint_constant */

/*+-------------------------------------------------------------------------
    gint_base(param,value) - evaluate integer constant, variable or function
--------------------------------------------------------------------------*/
int
gint_base(param, value)
ESD *param;
long *value;
{
	int erc;
	long *varptr;

	if (erc = skip_cmd_break(param))
		return (erc);

	switch (param->pb[param->index])	/* look at first character */
	{
		case '$':			 /* '$i...' variable reference? */
			if (param->index >= param->cb - 2)
				return (eSyntaxError);
			param->old_index = ++param->index;
			if (to_lower(param->pb[param->index++]) != 'i')
				return (eIllegalVarType);
			if (erc = get_ivptr(param, &varptr, 0))
				return (erc);
			*value = *varptr;
			return (0);

		case '%':			 /* '%...' function reference? */
			param->index++;
			if (erc = feval_int(param, value))
				return (erc);
			return (0);

		default:
			break;
	}						 /* end of switch statement */

/* we did not catch any special cases with the switch statement must
be numeric integer */

	return (gint_constant(param, value));

}							 /* end of gint_base() */

/*+-------------------------------------------------------------------------
    gintop(param,intop) - evaluate integer operator
--------------------------------------------------------------------------*/
int
gintop(param, intop)
ESD *param;
int *intop;
{
	int erc;

	if (erc = skip_cmd_break(param))
		return (erc);
	switch (*(param->pb + param->index))
	{
		case '+':
			param->index++;
			*intop = OP_ADD;
			break;

		case '-':
			param->index++;
			*intop = OP_SUB;
			break;

		case '*':
			param->index++;
			*intop = OP_MUL;
			break;

		case '/':
			param->index++;
			*intop = OP_DIV;
			break;

		case '@':
			param->index++;
			*intop = OP_MOD;
			break;

		case '^':
			param->index++;
			*intop = OP_XOR;
			break;

		case '|':
			if (*(param->pb + param->index + 1) == '|')
			{
				*intop = OP_LOR;
				param->index += 2;
				break;
			}
			param->index++;
			*intop = OP_OR;
			break;

		case '&':
			if (*(param->pb + param->index + 1) == '&')
			{
				*intop = OP_LAND;
				param->index += 2;
				break;
			}
			param->index++;
			*intop = OP_AND;
			break;

		case '<':
			if (*(param->pb + param->index + 1) != '<')
				return (eInvalidIntOp);
			param->index += 2;
			*intop = OP_SHL;
			break;

		case '>':
			if (*(param->pb + param->index + 1) != '>')
				return (eInvalidIntOp);
			param->index += 2;
			*intop = OP_SHR;
			break;

		default:
			return (eInvalidIntOp);
	}						 /* end of switch statement */

	return (0);

}							 /* end of gintop() */

/*+-------------------------------------------------------------------------
    gint(param,int_returned) - evaluate integer expression
--------------------------------------------------------------------------*/
int
gint(param, int_returned)
ESD *param;
long *int_returned;
{
	int erc;
	long int1;
	long int_accum = 0;
	int intop;
	int unary_minus = 0;

	if (erc = skip_cmd_break(param))
		return (erc);
	if (param->pb[param->index] == '-')
		unary_minus++, param->index++;

	if (erc = gint_base(param, &int1))
		return (erc);
	int_accum = (unary_minus) ? -int1 : int1;

	while ((erc = gintop(param, &intop)) == 0)
	{
		if (erc = gint_base(param, &int1))
			return (erc);
		switch (intop)
		{
			case OP_ADD:
				int_accum += int1;
				break;
			case OP_SUB:
				int_accum -= int1;
				break;
			case OP_MUL:
				int_accum *= int1;
				break;
			case OP_DIV:
				int_accum /= int1;
				break;
			case OP_MOD:
				int_accum %= int1;
				break;
			case OP_XOR:
				int_accum ^= int1;
				break;
			case OP_AND:
				int_accum &= int1;
				break;
			case OP_OR:
				int_accum |= int1;
				break;
			case OP_LAND:
				int_accum = int_accum && int1;
				break;
			case OP_LOR:
				int_accum = int_accum || int1;
				break;
			case OP_SHL:
				int_accum <<= int1;
				break;
			case OP_SHR:
				int_accum >>= int1;
				break;
			default:
				return (eInvalidIntOp);
		}
	}
	param->index = param->old_index;

	*int_returned = int_accum;
	return (0);
}							 /* end of gint() */

/*+-------------------------------------------------------------------------
    col_range(param,col1,col2) - get a column range
:$i0[-$i1]
argument may be integer constant, function or variable, but not expression
--------------------------------------------------------------------------*/
int
gcol_range(param, col1, col2)
ESD *param;
UINT32 *col1;
UINT32 *col2;
{
	int erc;

	if (skip_cmd_char(param, ':') == 0)
	{
		if (erc = gint_base(param, col1))
			return (erc);

		if (skip_cmd_char(param, '-') == 0)	/* if hyphen found, range */
		{
			if (erc = gint_base(param, col2))
				return (erc);
		}
		else
			*col2 = *col1;	 /* otherwise, first and last columns same */

		if (*col1 > *col2)
		{
			pputs("Invalid column range: column 1 greater than column 2\n");
			return (eFATAL_ALREADY);
		}
	}
	else
		erc = eBadParameter;

	return (erc);
}							 /* end of gcol_range() */

/* vi: set tabstop=4 shiftwidth=4: */
/* end of gint.c */
