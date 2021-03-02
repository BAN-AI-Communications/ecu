/*+-------------------------------------------------------------------------
	ecuerror.h
	wht@wht.net

e_... values must not be changed without careful looking through code
error numbers should be <= 0x7FFF to avoid problems with M_I286 versions
--------------------------------------------------------------------------*/
/*+:EDITS:*/
/*:04-26-2000-11:15-wht@bob-RELEASE 4.42 */
/*:03-31-1998-17:57-wht@kepler-add eNoFreeFile */
/*:02-01-1998-18:36-wht@kepler-update regular expression errors */
/*:11-16-1997-21:52-wht@kepler-add regexp error codes */
/*:01-25-1997-13:56-wht@yuriatin-add eInvalidStringOp */
/*:01-24-1997-02:37-wht@yuriatin-SOURCE RELEASE 4.00 */
/*:09-11-1996-20:00-wht@yuriatin-3.48-major telnet,curses,structural overhaul */
/*:11-23-1995-11:20-wht@kepler-source control 3.37 for tsx-11 */
/*:11-14-1995-10:23-wht@kepler-3.37.80-source control point: SOCKETS */
/*:05-04-1994-04:38-wht@n4hgf-ECU release 3.30 */
/*:09-10-1992-13:58-wht@n4hgf-ECU release 3.20 */
/*:08-22-1992-15:38-wht@n4hgf-ECU release 3.20 BETA */
/*:03-27-1992-16:21-wht@n4hgf-re-include protection for all .h files */
/*:08-25-1991-23:45-root@n4hgf2-add eSwitchesTooLong */
/*:07-25-1991-12:55-wht@n4hgf-ECU release 3.10 */
/*:08-14-1990-20:40-wht@n4hgf-ecu3.00-flush old edit history */

#ifndef _ecuerror_h
#define _ecuerror_h

#define e_USER				0x1000	/* user error differentiation */

/* warning errors - do not stop proc execution */
#define e_WARNING			0x3000
#define eProcEmpty			0x3001	/* empty procedure */
#define eWARNING_ALREADY	0x3002	/* warning already printed */
#define eConnectFailed		0x3003	/* failed to connect */
#define eNoSwitches			0x3004	/* no switch(es) to command */

/* fatal errors - stop proc execution */
#define e_FATAL				0x4000
#define eIllegalCommand		0x4003	/* invalid command */
#define eNoMemory			0x4004	/* no more memory available */
#define eSyntaxError		0x4005	/* syntax error */
#define eIllegalVarNumber	0x4006	/* number is invalid or out of range */
#define eIllegalVarType		0x4007	/* unrecognized variable type */
#define eNotInteger			0x4008	/* integer expected and not found */
#define eMissingIpAddress   0x4009	/* missing IP address */
#define eBadIpAddress       0x400A	/* invalid IP address */
#define eFATAL_ALREADY		0x4011	/* fatal to proc, info already printed */
#define eCONINT				0x4012	/* abort due to interrupt */
#define eInvalidFunction	0x4013	/* invalid function name */
#define eMissingLeftParen	0x4014	/* did not find expected left paren */
#define eMissingRightParen	0x4015	/* did not find expected right paren */
#define eCommaExpected		0x4016	/* expected comma not found */
#define eProcStackTooDeep	0x4017	/* procedure stack depth exceeded */
#define eInvalidRelOp		0x4018	/* invalid relational operator */
#define eInvalidIntOp		0x4019	/* invalid integer operator */
#define eInvalidStrOp		0x4020	/* invalid string operator */
#define eNotExecutingProc	0x4022	/* not executing a procedure */
#define eInvalidLabel		0x4023	/* invalid label */
#define eInternalLogicError	0x4025	/* internal logic error ... whoops */
#define eEOF				0x4026	/* end of file or read error */
#define eBufferTooSmall		0x4027	/* string too long */
#define eNoParameter		0x4028	/* expected parameter not found */
#define eBadParameter		0x4029	/* bad parameter */
#define eInvalidHexNumber	0x402A	/* invalid hexadecimal digit */
#define eInvalidDecNumber	0x402B	/* invalid decimal digit */
#define eInvalidOctNumber	0x402C	/* invalid octal digit */
#define eInteractiveCmd		0x402E	/* interactive command */
#define eNoLineAttached		0x402F	/* no line (modem) attached */
#define eBadFileNumber		0x4030	/* file number out of range */
#define eNotImplemented		0x4031	/* not implemented */
#define eDuplicateMatch		0x4032	/* more than one condition matches */
#define eColonExpected		0x4033	/* expected colon not found */
#define eLabelInvalidHere	0x4034	/* label not allowed on this statement */
#define eNoCloseFrame		0x4035	/* missing '}' for '{' */
#define eNoFrame			0x4036	/* missing command or command group
									 * after 'while' or 'if' */
#define eMissingCommand		0x4037	/* expected command not found */
#define eBreakCommand		0x4038	/* 'break' outside 'while' */
#define eContinueCommand	0x4039	/* 'continue' outside 'while' */
#define eElseCommand		0x403A	/* 'else' without matching 'if' */
#define eInvalidVarName		0x403B	/* invalid variable name */
#define eNoSuchVariable		0x403C	/* variable by this name not defined */
#define eInvalidLogicOp		0x403D	/* invalid logical operator */
#define eExpectRespondFail	0x403E	/* expect-respond failed */
#define eSwitchesTooLong	0x403F	/* switches too long */
#define eInvalidStringOp	0x40F0	/* invalid operator for strings */
#define eNoFreeFile			0x40F1	/* no free ECU file number */

#define eRegNoParen			0x40F3	/* parenthetical groups not supported */
#define eRegRange			0x40F4	/* range endpoint too large */
#define eRegBadNum			0x40F5	/* bad number in regular expression */
#define eRegDigit			0x40F6	/* \"\\digit\" out of range */
#define eRegDelim			0x40F7	/* illegal or missing delimiter */
#define eRegNullRE          0x40F8  /* null expression not allowed */
#define eReg2FewOpens		0x40F9	/* more \\)'s than \\('s in regexp */
#define eReg2ManyOpens		0x40FA	/* more \\('s than \\)'s in regexp */
#define eReg2ManyArgs		0x40FB	/* more than 2 numbers in \\{ \\} */
#define eRegBraceExp		0x40FC	/* } expected after \\ */
#define eRegBraceRange		0x40FD	/* first number exceeds second in \\{ \\} */
#define eRegBracketImb		0x40FE	/* [] imbalance */
#define eReg2Complex		0x40FF	/* regular expression too complex */


/* DO attention getter */
#define e_ProcAttn			0x7000
#define eProcAttn_GOTO		0x7000	/* GOTO detected */
#define eProcAttn_GOTOB		0x7001	/* GOTOB detected */
#define eProcAttn_RETURN	0x7002	/* RETURN detected */
#define eProcAttn_ESCAPE	0x7003	/* ESCAPE detected */
#define eProcAttn_Interrupt	0x7004	/* procedure interrupted */
#define eProcAttn_DCDloss	0x7005	/* DCD lost during procedure execution */

#endif /* _ecuerror_h */

/* vi: set tabstop=4 shiftwidth=4: */
/* end of ecuerror.h */
