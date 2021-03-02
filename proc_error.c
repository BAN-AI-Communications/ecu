/*+-------------------------------------------------------------------------
	proc_error.c - ecu error code handler

  D O   N O T   E D I T   B Y   H A N D
  Created automagically by bperr/bperr.c

  Defined functions:
	erc_text(erc)
	proc_error(erc)

--------------------------------------------------------------------------*/
/*+:EDITS:*/
/*:04-26-2000-11:16-wht@bob-RELEASE 4.42 */
/*:04-14-1999-10:15-auto-creation from ecuerror.h by build_err 1.2 */

#include "ecu.h"
#include "ecuerror.h"

/*+-------------------------------------------------------------------------
	erc_text(erc) - error code to text

 e.*ALREADY and eProcAttn.* excluded because they are never printed
--------------------------------------------------------------------------*/
char *
erc_text(erc)
int erc;
{
	static char errant[54];

	switch(erc)
	{
		case eProcEmpty:
			return("empty procedure");
		case eConnectFailed:
			return("failed to connect");
		case eNoSwitches:
			return("no switch(es) to command");
		case eIllegalCommand:
			return("invalid command");
		case eNoMemory:
			return("no more memory available");
		case eSyntaxError:
			return("syntax error");
		case eMissingIpAddress:
			return("missing IP address");
		case eBadIpAddress:
			return("invalid IP address");
		case eIllegalVarNumber:
			return("number is invalid or out of range");
		case eIllegalVarType:
			return("unrecognized variable type");
		case eNotInteger:
			return("integer expected and not found");
		case eCONINT:
			return("abort due to interrupt");
		case eInvalidFunction:
			return("invalid function name");
		case eMissingLeftParen:
			return("did not find expected left paren");
		case eMissingRightParen:
			return("did not find expected right paren");
		case eCommaExpected:
			return("expected comma not found");
		case eProcStackTooDeep:
			return("procedure stack depth exceeded");
		case eInvalidRelOp:
			return("invalid relational operator");
		case eInvalidIntOp:
			return("invalid integer operator");
		case eInvalidStrOp:
			return("invalid string operator");
		case eNotExecutingProc:
			return("not executing a procedure");
		case eInvalidLabel:
			return("invalid label");
		case eInternalLogicError:
			return("internal logic error ... whoops");
		case eEOF:
			return("end of file or read error");
		case eBufferTooSmall:
			return("string too long");
		case eNoParameter:
			return("expected parameter not found");
		case eBadParameter:
			return("bad parameter");
		case eInvalidHexNumber:
			return("invalid hexadecimal digit");
		case eInvalidDecNumber:
			return("invalid decimal digit");
		case eInvalidOctNumber:
			return("invalid octal digit");
		case eInteractiveCmd:
			return("interactive command");
		case eNoLineAttached:
			return("no line (modem) attached");
		case eBadFileNumber:
			return("file number out of range");
		case eNotImplemented:
			return("not implemented");
		case eDuplicateMatch:
			return("more than one condition matches");
		case eColonExpected:
			return("expected colon not found");
		case eLabelInvalidHere:
			return("label not allowed on this statement");
		case eNoCloseFrame:
			return("missing '}' for '{'");
		case eNoFrame:
			return("missing command or command");
		case eMissingCommand:
			return("expected command not found");
		case eBreakCommand:
			return("'break' outside 'while'");
		case eContinueCommand:
			return("'continue' outside 'while'");
		case eElseCommand:
			return("'else' without matching 'if'");
		case eInvalidVarName:
			return("invalid variable name");
		case eNoSuchVariable:
			return("variable by this name not defined");
		case eInvalidLogicOp:
			return("invalid logical operator");
		case eExpectRespondFail:
			return("expect-respond failed");
		case eSwitchesTooLong:
			return("switches too long");
		case eInvalidStringOp:
			return("invalid operator for strings");
		case eNoFreeFile:
			return("no free ECU file number");
		case eRegNoParen:
			return("parenthetical groups not supported");
		case eRegRange:
			return("range endpoint too large");
		case eRegBadNum:
			return("bad number in regular expression");
		case eRegDigit:
			return("\"\\digit\" out of range");
		case eRegDelim:
			return("illegal or missing delimiter");
		case eRegNullRE:
			return("null expression not allowed");
		case eReg2FewOpens:
			return("more \\)'s than \\('s in regexp");
		case eReg2ManyOpens:
			return("more \\('s than \\)'s in regexp");
		case eReg2ManyArgs:
			return("more than 2 numbers in \\{ \\}");
		case eRegBraceExp:
			return("} expected after \\");
		case eRegBraceRange:
			return("first number exceeds second in \\{ \\}");
		case eRegBracketImb:
			return("[] imbalance");
		case eReg2Complex:
			return("regular expression too complex");

		default:
			sprintf(errant,"unknown error %04X",erc);
			return(errant);
	}
} /* end of erc_text */


/*+-------------------------------------------------------------------------
	proc_error(erc) - print error text with newline appended
--------------------------------------------------------------------------*/
void
proc_error(erc)
int erc;
{
	pputs(erc_text(erc));
	pputs("\n");

} /* end of proc_error */


/* vi: set tabstop=4 shiftwidth=4: */
/* end of proc_error.c */
