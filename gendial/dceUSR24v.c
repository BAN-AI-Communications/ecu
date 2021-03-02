/*+-------------------------------------------------------------------------
  dceUSR24v.c - DCE-specific portion of generic SCO UUCP dialer
	Driver for USR Courier 2400 - optimized for rapid use as voice dialer
	wht@wht.net

 This is for attended voice applications where speed of operation is
 more important than superb reliability.  The modem I had in mind
 here is an old (simple and reliable) USR 2400 I had lying around
 doing nothing.  Now it is a voice line dialer.  Autoanswer is always
 turned off whenever we can do it.  No one goes after the modem
 anywhere near once per minute.  We can dispense with delays and
 brick outhouse initialization.

 Necessary DCE switch setting or other configuration:
   enable onhook upon loss of DTR

This dialer does not use the X6 quick dial feature, nor voice detection.
Quick dial tone recognition often fqails due to the telco granting
dial tone ahead of the actual time it is ready to accept dialing.
Voice recognition fails when dialing 0+ with a credit card, as in "Thank
you for using ATT" - CLICK - "VOICE".

--------------------------------------------------------------------------*/
/*+:EDITS:*/
/*:01-24-1997-02:37-wht@yuriatin-SOURCE RELEASE 4.00 */
/*:09-11-1996-20:00-wht@yuriatin-3.48-major telnet,curses,structural overhaul */
/*:08-20-1996-12:39-wht@kepler-locale/ctype fixes from ache@nagual.ru */
/*:11-23-1995-11:20-wht@kepler-source control 3.37 for tsx-11 */
/*:11-14-1995-10:23-wht@kepler-3.37.80-source control point: SOCKETS */
/*:05-04-1994-04:39-wht@n4hgf-ECU release 3.30 */
/*:09-10-1992-13:59-wht@n4hgf-ECU release 3.20 */
/*:08-22-1992-15:38-wht@n4hgf-ECU release 3.20 BETA */
/*:02-02-1992-18:01-root@n4hgf-proper ordering of DCE_result entries */
/*:01-26-1992-15:30-wht@n4hgf-gendial 1.2 for ecu 3.20- better hangup */
/*:07-25-1991-12:58-wht@n4hgf-ECU release 3.10 */
/*:07-25-1991-04:34-wht@n4hgf-add naps around writes */
/*:04-16-1991-18:18-wht@n4hgf-creation from template */

#include "dialer.h"

/*
 * DCE_DTR_low_msec - milliseconds to hold DTR low to ensure DCE
 *                    sees the transition; this value may be changed
 *                    as necessary before each call to lflash_DTR(),
 * but, generally, a constant value will do.
 */
long DCE_DTR_low_msec = 100L;

/*
 * DCE_DTR_high_msec - milliseconds DTR must remain high before the
 *                     DCE may be expected to be ready to be commanded
 */
long DCE_DTR_high_msec = 50L;

/*
 * DCE_write_pace_msec - milliseconds to pause between each character
 *                       sent to the DCE (zero if streaming I/O is
 *                       permitted); this value may be changed as
 * necessary before each call to lwrite(), but, generally, a constant
 * value will do.  Note that this value is used to feed a value to Nap(),
 * which has a granularity of .010 seconds on UNIX/386, .020 on XENIX/286
 * and .050 seconds on XENIX/86.
 */
long DCE_write_pace_msec = 0;

/*
 * DCE_name     - short name for DCE
 * DCE_revision - revision number for this module
 */
char *DCE_name = "USR Courier 2400 quick voice";
char *DCE_revision = "1.00";

/*
 * DCE_hangup_CBAUD - baud rate to use for hanging up DCE
 *                    and readying it for dial in access
 *                    (BXXX mask); use a value of zero if the speed
 *                    specified by the invoker is to be used.
 * This value is useful for DCEs such as the early Hayes 2400
 * which are so unfortunately compatible with their 1200 predecessor
 * that they refuse to answer at 2400 baud unless you last spoke to
 * them at that rate. For such bad boys, use B2400 below.
 */
int DCE_hangup_CBAUD = B2400;

/*
 * DCE_results - a table of DCE response strings and a token
 *               code for each; when you call lread() or lread_ignore(),
 *               if the read routine detects one of the strings,
 * the appropriate code is returned.  If no string matches, then
 * lread()/lread_ignore examines the DCE result string for a
 * numeric value; if one is found, the numeric value or'd with
 * 0x4000 is returned (in this way, e.g., you can read "modem
 * S registers."  If nothing agrees with this search, lread()
 * will abort the program with RC|FAIL|RCE_TIMOUT, lread_ignore()
 * will return -1.  You may use any value between 0 and 0x3FFFFFFF.
 * This module is the only consumer  of the codes, although they
 * are decoded by gendial.c's _lread()
 *
 * If one possible result is an "early substring" of another, like
 * "CONNECT" is of "CONNECT 1200", then put such results later in the
 * table than the larger result.
 *
 */
#define rfConnect		0x00400000
#define rfMASK			0x000000FF

#define rOk				0
#define rNoCarrier		1
#define rError			2
#define rNoDialTone		3
#define rBusy			4
#define rNoAnswer		5
#define rVoice			6
#define rConnect300		(7 | rfConnect)
#define rConnect1200	(8 | rfConnect)
#define rConnect2400	(9 | rfConnect)

DCE_RESULT DCE_results[] =
{
	{"OK", rOk,},
	{"NO CARRIER", rNoCarrier,},
	{"ERROR", rError},
	{"NO DIALTONE", rNoDialTone,},
	{"BUSY", rBusy},
	{"NO ANSWER", rNoAnswer},
	{"VOICE", rVoice},
	{"CONNECT 1200", rConnect1200},
	{"CONNECT 2400", rConnect2400},
	{"CONNECT", rConnect300},
	{(char *)0, -1}			 /* end table */
};

/*+-------------------------------------------------------------------------
	DCE_baud_to_CBAUD(baud) - check for valid baud rates supported by DCE

  DCE dependent function must validate baud rates supported by DCE
  returns baud rate in struct termio c_cflag fashion
  or terminates program with error
--------------------------------------------------------------------------*/
int
DCE_baud_to_CBAUD(baud)
unsigned int baud;
{
	switch (baud)
	{
		case 50:
			return (B50);	 /* delete the ones you dont handle */
		case 75:
			return (B75);
		case 110:
			return (B110);
		case 134:
			return (B134);
		case 150:
			return (B150);
		case 300:
			return (B300);
		case 1200:
			return (B1200);
		case 2400:
			return (B2400);
		case 4800:
			return (B4800);
		case 9600:
			return (B9600);

#if defined(B19200)
		case 19200:
			return (B19200);
#else
#ifdef EXTA
		case 19200:
			return (EXTA);
#endif
#endif

#if defined(B38400)
		case 38400:
			return (B38400);
#else
#ifdef EXTB
		case 38400:
			return (EXTB);
#endif
#endif

	}
	myexit(RC_FAIL | RCE_SPEED);
#if defined(OPTIMIZE) || defined(__OPTIMIZE__)
	return (0);				 /* I wish this wasn't necessary to avoid
							  * warnings */
#endif
}							 /* end of DCE_baud_to_CBAUD */

/*+-------------------------------------------------------------------------
	DCE_hangup() - issue hangup command to DCE

This function should do whatever is necessary to ensure
1) any active connection is terminated
2) the DCE is ready to receive an incoming call if DTR is asserted
3) the DCE will not accept an incoming call if DTR is false

The function should return when done.

You must set any switches necessary to make modem hang up on loss of DTR
--------------------------------------------------------------------------*/
void
DCE_hangup()
{

	DEBUG(1, "--> hanging up %s\n", dce_name);
	lflash_DTR();

	lwrite("ATQ0S0=0\r");
	Nap(20L);

}							 /* end of DCE_hangup */

/*+-------------------------------------------------------------------------
	DCE_dial(telno_str) - dial a remote DCE

This function should connect to the remote DCE and use any success
indication to modify the tty baud rate if necessary before returning.

Upon successful connection, return 0.

Upon unsuccessful connection, return RC_FAIL or'd with an appropriate
RCE_XXX value from dialer.h.

lwrite() is used to write to the DCE.

lread() and lread_ignore() are used to read from the DCE.  Read timeouts
from calling lread() will result automatically in the proper error
termination of the program.  Read timeouts from calling lread_ignore()
return -1; you handle the execption here.

Any necessary coding of phone numbers, switch settings or other
configuration necessary for this function to succeed should be
documented at the top of the module.
--------------------------------------------------------------------------*/
int
DCE_dial(telno_str)
char *telno_str;
{
	char cmd[128];
	char phone[50];
	int timeout;
	int result;
	char *sptr;
	char *dptr;
	char ch;
	int len;

	sptr = telno_str;
	dptr = phone + 1;
	*dptr = 0;
	while (ch = *sptr++)
	{
		if (isdigit((unsigned char)ch) || (ch == ','))
		{
			*dptr++ = ch;
			*dptr = 0;
		}
	}

	telno_str = phone + 1;
	if (((len = strlen(phone + 1)) == 10) &&
		((phone[1 + 1] == '0') || (phone[1 + 1] == '1')))
	{
		phone[0] = '1';
		telno_str = phone;
	}

/*
 * build and issue the actual dialing command
 */
	DEBUG(1, "--> dialing %s\n", telno_str);
	sprintf(cmd, "ATe1Q0V1S0=0S7=120X3DT%s\r", telno_str);
	DCE_write_pace_msec = 20;
	lwrite("AA");
	DCE_write_pace_msec = 0;
	lwrite(cmd);

/* wait for connect */
	result = lread(timeout);
	if (!(result & rfConnect))
	{
		switch (result & rfMASK)
		{
			case rNoCarrier:
				return (RC_FAIL | RCE_NOCARR);
			case rVoice:	 /* if you get voice, certainly wrong number */
			case rNoDialTone:
				return (RC_FAIL | RCE_NOTONE);
			case rBusy:
				return (RC_FAIL | RCE_BUSY);
			case rNoAnswer:
				return (RC_FAIL | RCE_ANSWER);
			case rError:
			default:
				return (RC_FAIL | RCE_NULL);
		}
	}

/* indicate non-root can see DTE->DCE traffic */
	secure = 0;
	return (0);				 /* succeeded */

}							 /* end of DCE_dial */

/**********************************************************
*  You probably do not need to modify the code below here *
**********************************************************/

/*+-------------------------------------------------------------------------
	DCE_abort(sig) - dial attempt aborted

 sig =  0 if non-signal abort (read timeout, most likely)
     != 0 if non-SIGALRM signal caught

 extern int dialing set  1 if dialing request was active,
                    else 0 if hangup request was active

This is a chance for the DCE-specific code to do anything it
needs to cl,ean up after a failure.  Note that if a dialing
call fails, it is the responsibility of the higher-level
program calling the dialer to call it again with a hangup request, so
this function is usually a no-op.
--------------------------------------------------------------------------*/
void
DCE_abort(sig)
int sig;
{
	DEBUG(10, "DCE_abort(%d);\n", sig);
}							 /* end of DCE_abort */

/*+-------------------------------------------------------------------------
	DCE_exit(exitcode) - "last chance for gas" in this incarnation

The independent portion of the dialer program calls this routine in
lieu of exit() in every case except one (see DCE_argv_hook() below).
Normally, this function just passes it's argument to exit(), but
any necessary post-processing can be done.  The function must,
however, eventually call exit(exitcode);
--------------------------------------------------------------------------*/
void
DCE_exit(exitcode)
int exitcode;
{
	DEBUG(10, "DCE_exit(%d);\n", exitcode);
	exit(exitcode);
}							 /* end of DCE_exit */

/*+-------------------------------------------------------------------------
	DCE_argv_hook(argc,argv,optind,unrecognized_switches)

This hook gives DCE-specific code a chance to look over the entire
command line, such as for -z Telebit processing.

argc andf argv are the same values passed to main(),

optind is the value of optind at the end of normal getopt processing.

unrecognized_switches is the count of switches not handled by main().
Specifically, -h and -x are standard switches.

Normally, this function should just return RC_FAIL|RCE_ARGS if there are
any unrecognized switches, otherwise zero.  If you keep your nose clean
though, you can do anything you need to do here and exit the program.

Note: only simple switches (with no argument) may be used with this
facility if the functrion is to return,' since main()'s getopt() will
stop processing switches if it runs into an unrecognized switch with an
argument.

If the function returns a non-zero value, then the value will be passed
DIRECTLY to exit() with no further ado.  Thus, a non-zero value must be
of the format expected by dialer program callers, with RC_FAIL set as a
minimum.
--------------------------------------------------------------------------*/
int
DCE_argv_hook(argc, argv, optind, unrecognized_switches)
int argc;
char **argv;
int optind;
int unrecognized_switches;
{
	if (unrecognized_switches)
		return (RC_FAIL | RCE_ARGS);
	return (0);
}							 /* end of DCE_argv_hook */

/* vi: set tabstop=4 shiftwidth=4: */
