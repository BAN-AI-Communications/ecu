/*+-------------------------------------------------------------------------
    gstr.c - ecu get string parameter functions
	wht@wht.net

  Defined functions:
	gstr(param, result, realloc_ok)

--------------------------------------------------------------------------*/
/*+:EDITS:*/
/*:04-26-2000-11:16-wht@bob-RELEASE 4.42 */
/*:01-24-1997-02:37-wht@yuriatin-SOURCE RELEASE 4.00 */
/*:09-11-1996-20:00-wht@yuriatin-3.48-major telnet,curses,structural overhaul */
/*:08-20-1996-12:39-wht@kepler-locale/ctype fixes from ache@nagual.ru */
/*:11-23-1995-11:20-wht@kepler-source control 3.37 for tsx-11 */
/*:11-14-1995-10:23-wht@kepler-3.37.80-source control point: SOCKETS */
/*:05-04-1994-04:39-wht@n4hgf-ECU release 3.30 */
/*:09-10-1992-13:59-wht@n4hgf-ECU release 3.20 */
/*:08-22-1992-15:39-wht@n4hgf-ECU release 3.20 BETA */
/*:07-25-1991-12:58-wht@n4hgf-ECU release 3.10 */
/*:05-02-1991-03:54-wht@n4hgf-new realloc algorithm */
/*:08-14-1990-20:40-wht@n4hgf-ecu3.00-flush old edit history */

#include "ecu.h"
#include "ecuerror.h"
#include "esd.h"
#include "var.h"


/*+-------------------------------------------------------------------------
    gstr(param,result,realloc_ok) - get a string

Examples:

 set $s0='test ... '+%date+' '+%time+%chr(0x0D)+%chr(0x0A)
 hexdump $s0
0000  74 65 73 74 20 2E 2E 2E 20 30 36 2D 30 39 2D 31 | test ... 06-09-1 |
0010  39 38 39 20 31 37 3A 31 35 0D 0A                | 989 17:15..      |

 set $s0='12345678':1-6+'abc'
 set s0
$S00 = '234567abc'

if realloc_ok and string too small, realloc result string as necessary

--------------------------------------------------------------------------*/
int
gstr(param, result, realloc_ok)
ESD *param;
ESD *result;
int realloc_ok;
{
	char param_char;
	char *pb;
	ESD *tesd;
	ESD *svptr;
	int cb = 0;
	int segment_index;
	int next_is_literal = 0; /* last char was not a backslash */
	UINT32 itmp1;
	UINT32 itmp2;
	UINT32 itmp3;
	UINT32 itmp4;
	int erc;
	int param_index_save;
	int result_remaining;
	int in_quoted_string = 0;/* not currently in quoted string */
	int end_of_parameter = 0;

	if (erc = skip_cmd_break(param))
		return (erc);

	segment_index = 0;
	result_remaining = result->maxcb;	/* number we can put into result */
	param_index_save = param->index;

	if ((tesd = esdalloc(ESD_MAXSZ)) == (ESD *) 0)
		return (eNoMemory);
	pb = tesd->pb;

  CONCATENATE:
	while ((param->index < param->cb) && !end_of_parameter)
	{
		param_char = param->pb[param->index];
		if (in_quoted_string)
		{
			++param->index;
			if (next_is_literal)
			{
				next_is_literal = 0;
				switch (param_char)
				{
					case 'b':
						param_char = 0x08;
						break;
					case 'n':
						param_char = 0x0A;
						break;
					case 'r':
						param_char = 0x0D;
						break;
					case 't':
						param_char = 0x09;
						break;
					case '\'':
						param_char = '\'';
						break;
				}
				if ((result_remaining-- == 0) &&
					(!realloc_ok && (cb == ESD_MAXSZ)))
				{
					erc = eBufferTooSmall;
					goto FUNC_RETURN;
				}
				*(pb + cb++) = param_char;
			}
			else if (param_char == '\\')
				next_is_literal = 1;
			else if (param_char == '\'')
				in_quoted_string = 0;
			else
			{
				if ((result_remaining-- == 0) &&
					(!realloc_ok && (cb == ESD_MAXSZ)))
				{
					erc = eBufferTooSmall;
					goto FUNC_RETURN;
				}
				*(pb + cb++) = param_char;
			}
		}
		else
			/* not in quoted string */
		{
			param->old_index = param->index;
			switch (param_char)
			{
				case '\'':	 /* apostrophe denotes literal text */
					++param->index;
					in_quoted_string = 1;
					break;

				case '%':
					++param->index;
					tesd->cb = cb;
					if (erc = feval_str(param, tesd))
						goto FUNC_RETURN;
					cb = tesd->cb;
					result_remaining = (result->maxcb - cb);
					break;

				case '$':	 /* '$Snn' variable reference? */
					/* must be at least two more character */
					if (param->index >= param->cb - 2)
					{
						erc = eSyntaxError;
						goto FUNC_RETURN;
					}
					param->old_index = ++param->index;
					if (to_lower(param->pb[param->index++]) != 's')
					{
						erc = eIllegalVarType;
						goto FUNC_RETURN;
					}
					if (erc = get_svptr(param, &svptr, 0))
						goto FUNC_RETURN;
					if ((!realloc_ok && (svptr->cb > (result->maxcb - cb))) ||
						(svptr->cb > (ESD_MAXSZ - cb)))
					{
						erc = eBufferTooSmall;
						goto FUNC_RETURN;
					}
					else if (svptr->cb)
					{
						memcpy(&pb[cb], svptr->pb, svptr->cb);
						cb += svptr->cb;
						result_remaining -= svptr->cb;
					}
					break;

				case ':':
/*
 * itmp1 holds col 1 (0-n) of substring operation
 * itmp2 holds col 2 (0-n) of operation adjusted to reflect
 *       end of string segment
 * itmp3 holds length of string segment
 * itmp4 holds length of substring segment output by substring operation
*/
					if (erc = gcol_range(param, &itmp1, &itmp2))
						goto FUNC_RETURN;
					if ((itmp3 = cb - segment_index)
						&&
						(itmp4 = ((itmp2 < itmp3) ? itmp2 : itmp3) - itmp1 + 1))
					{
						if (itmp1)
							memcpy(&pb[segment_index],
								&pb[segment_index + (int)itmp1], (int)itmp4);
						cb -= ((int)itmp3 - (int)itmp4);
					}
					break;

				case '+':
					segment_index = cb;
					++param->index;
					goto CONCATENATE;

				case ';':
				case '#':
					end_of_parameter = 1;
					break;

				default:
					erc = 0;
					if ((param->index < param->cb) &&
						isalnum((uchar) * (param->pb + param->index)))
						erc = eSyntaxError;
					else if (param_index_save == param->index)
						erc = eBadParameter;
					end_of_parameter = 1;
					break;
			}				 /* end of switch (param_char) */
		}					 /* end of else not in quoted string */
	}						 /* end of while(index<cb) */

  FUNC_RETURN:
	if (result_remaining < 0)
	{
		if (!realloc_ok)
			erc = eBufferTooSmall;
		else
		{
			int new_size = (cb + 128) & (~127);	/* speed + anti-fragment */
			if (new_size > ESD_MAXSZ)
				new_size = ESD_MAXSZ;
			if (new_size < cb)
				erc = eBufferTooSmall;
			else
				erc = esdrealloc(result, new_size);
		}
		if (erc)
			return (erc);
	}
	if (cb)
		memcpy(result->pb, pb, cb);
	result->cb = cb;
	esd_null_terminate(result);
	esdfree(tesd);
	return (erc);
}							 /* end of gqstr */

/* vi: set tabstop=4 shiftwidth=4: */
/* end of qstr.c */
