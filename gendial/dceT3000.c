/*+-------------------------------------------------------------------------
	dceT3000.c - DCE-specific portion of generic SCO UUCP dialer
	Driver for Telebit T3000
	wht@wht.net

dialing registers for V.32

T3000SA - Version LA3.00 - Active Configuration
 B1  E0  L2  M0  P   Q2  V1  X12  Y0
&C1 &D2 &G0 &J0 &L0 &Q0 &R3 &S0 &T4 &X0
S000=0   S001=0   S002:1   S003=13  S004=10  S005=8   S006=2   S007=40
S008=2   S009=6   S010=14  S011:50  S012=50  S018=0   S025=5   S026=1
S038=0   S041=0   S045=0   S046=0   S047=4   S048:1   S050:6   S051:252
S056=17  S057=19  S058:2   S059=0   S060=0   S061=1   S062:25  S063=0
S064=0   S068=255 S069=0   S090=0   S093=8   S094=1   S100=0   S102=0
S104=0   S105=1   S111=255 S112=1   S180=2   S181=1   S183=25  S190=1
S253=10  S254=255 S255=255
--------------------------------------------------------------------------*/
/*+:EDITS:*/
/*:01-24-1997-02:37-wht@yuriatin-SOURCE RELEASE 4.00 */
/*:09-11-1996-20:00-wht@yuriatin-3.48-major telnet,curses,structural overhaul */
/*:11-23-1995-11:20-wht@kepler-source control 3.37 for tsx-11 */
/*:11-14-1995-10:23-wht@kepler-3.37.80-source control point: SOCKETS */
/*:05-04-1994-04:39-wht@n4hgf-ECU release 3.30 */
/*:09-10-1992-13:59-wht@n4hgf-ECU release 3.20 */
/*:08-22-1992-15:38-wht@n4hgf-ECU release 3.20 BETA */
/*:05-11-1992-17:51-wht@gyro-convert dceT2500 for a preliminary version */

#include "dialer.h"

/*
 * DCE_DTR_low_msec - milliseconds to hold DTR low to ensure DCE
 *                    sees the transition; this value may be changed
 *                    as necessary before each call to lflash_DTR(),
 * but, generally, a constant value will do.
 */
long DCE_DTR_low_msec = 500;

/*
 * DCE_DTR_high_msec - milliseconds DTR must remain high before the
 *                     DCE may be expected to be ready to be commanded
 */
long DCE_DTR_high_msec = 1000L;

/*
 * DCE_write_pace_msec - milliseconds to pause between each character
 *                       sent to the DCE (zero if streaming I/O is
 *                       permitted); this value may be changed as
 * necessary before each call to lwrite(), but, generally, a constant
 * value will do.  Note that this value is used to feed a value to Nap(),
 * which has a granularity of .010 seconds on UNIX/386, .020 on XENIX/286
 * and .050 seconds on XENIX/86.
 */
long DCE_write_pace_msec = 10;

/*
 * DCE_name     - short name for DCE
 * DCE_revision - revision number for this module
 */
char *DCE_name = "Telebit T3000";
char *DCE_revision = "x1.10";

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
int DCE_hangup_CBAUD = 0;

/* int DCE_hangup_CBAUD = B2400; */

/*
 * DCE_results - a table of DCE response strings and a token
 *               code for each; when you call lread() or lread_ignore(),
 *               if the read routine detects one of the strings,
 * the appropriate code is returned.  If no string matches, then
 * lread()/lread_ignore examines the DCE result string for a
 * numeric value; if one is found, the numeric value or'd with
 * 0x40000000 is returned (in this way, e.g., you can read "modem
 * S registers").  If nothing agrees with this search, lread()
 * will abort the program with RC_FAIL|RCE_TIMOUT, lread_ignore()
 * will return -1.  You may use any value between 0 and 0x3FFFFFFF.
 * This module is the only consumer  of the codes, although they
 * are decoded by gendial.c's _lread()
 */

/* flag bits */
#define rfConnect		0x00800000
#define rfMASK			0x0000FFFF	/* mask off rfBits */

/* unique codes */
#define rOk				0
#define rNoCarrier		1
#define rError			2
#define rNoDialTone 	3
#define rBusy			4
#define rNoAnswer		5
#define rRring			6
#define rDialing		7
#define rConnect300		(   300 | rfConnect)
#define rConnect1200	(  1200 | rfConnect)
#define rConnect2400	(  1200 | rfConnect)
#define rConnect9600	(  9600 | rfConnect)
#define rConnect19200	( 19200 | rfConnect)
#define rConnect38400	( 38400 | rfConnect)

DCE_RESULT DCE_results[] =
{
	{"OK", rOk,},
	{"NO CARRIER", rNoCarrier,},
	{"ERROR", rError},
	{"NO DIALTONE", rNoDialTone,},
	{"BUSY", rBusy},
	{"NO ANSWER", rNoAnswer},
	{"DIALING", rDialing},
	{"RRING", rRring},
	{"CONNECT 300", rConnect300},
	{"CONNECT 1200", rConnect1200},
	{"CONNECT 2400", rConnect2400},
	{"CONNECT 9600", rConnect9600},
	{"CONNECT 19200", rConnect19200},
	{"CONNECT 38400", rConnect38400},
	{(char *)0, -1}			 /* end table */
};

#include "tbit.sync.h"

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
		case 110:
			return (B110);
		case 300:
			return (B300);
		case 1200:
			return (B1200);
		case 2400:
			return (B2400);
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
#if defined(OPTIMIZE) || defined(__OPTIMIZE__)	/* don't complain */
	return (0);				 /* keep gcc from complaining about no rtn at
							  * end */
#endif
}							 /* end of DCE_baud_to_CBAUD */

/*+-------------------------------------------------------------------------
	init_T3000() - init T3000 from scratch, assuming nothing

	reset to factory defaults, then set
    E0          no local echo in command mode
    &C1         DCD follows carrier
    &D2         disconnect on DTR loss
    M0          speaker off
    Q2          generate reult codes only for originating use
    V1          verbal result codes
    X12         fullest result code set
    S0=1        answer on first ring
    S2=255        escape to unusual value
    S11=50      50 msec DTMF timing
    S45=0       disable remote access
    S48=1       all 8 bits are significant
    S50=0       use automatic connect speed determination
    S51=252     set serial port baud rate automatically (no typeahead)
    S58=2       DTE uses CTS/RTS flow control.
	S61=0       send BREAK (rather than go to command mode)
	S62=25      BREAK duration 250 msec (default is 150)
	S63=0       send BREAK in sequence
    S64=1       ignore characters sent by DTE while answering
    S66=0       don't lock interface speed, just go with the flow.
    S69=0       omit XON/XOFF flow control
    S68=255     DCE uses whatever flow control DTE uses
    S111=255    accept any protocol

The nvram is set to factory + E0 Q0 &C1 &D2 S51=252
--------------------------------------------------------------------------*/
void
init_T3000()
{
	register itmp;
	int maxretry = 4;
	char *init0 = "AT&F E0 Q0 &C1 &D2 S51=252 &w M0 Q2 V1 X12\r";
	char *init1 = "ATS0=1 S2=255 S11=50 S45=0 S48=1 S50=0 \r";
	char *init2 = "ATS58=2 S61=0 S62=25 S63=0 S62=25 S64=1 S66=0 S68=255 S111=255\r";

	DEBUG(1, "--> initializing %s on ", DCE_name);
	DEBUG(1, "%s\n", dce_name);

	lflash_DTR();
	sync_Telebit();

	/*
	 * set to factory default (bless them for this command) and a few
	 * initial beachhead values
	 */
	for (itmp = 0; itmp < maxretry; itmp++)
	{
		lwrite(init0);
		if (lread(5) == rOk)
			break;
	}
	if (itmp == maxretry)
	{
		DEBUG(1, "INIT FAILED (init0)\n", 0);
		myexit(RC_FAIL | RCE_TIMOUT);
	}

	/*
	 * send initialization string 1
	 */
	for (itmp = 0; itmp < maxretry; itmp++)
	{
		lwrite(init1);
		if (lread(5) == rOk)
			break;
	}
	if (itmp == maxretry)
	{
		DEBUG(1, "INIT FAILED (init1)\n", 0);
		myexit(RC_FAIL | RCE_TIMOUT);
	}

	/*
	 * send initialization string 2
	 */
	for (itmp = 0; itmp < maxretry; itmp++)
	{
		lwrite(init2);
		if (lread(5) == rOk)
			break;
	}
	if (itmp == maxretry)
	{
		DEBUG(1, "INIT FAILED (init2)\n", 0);
		myexit(RC_FAIL | RCE_TIMOUT);
	}

}							 /* end of init_T3000 */

/*+-------------------------------------------------------------------------
	DCE_hangup() - issue hangup command to DCE

This function should do whatever is necessary to ensure
1) any active connection is terminated
2) the DCE is ready to receive an incoming call if DTR is asserted
3) the DCE will not accept an incoming call if DTR is false

The function should return when done.

Any necessary switch setting or other configuration necessary for this
function to succeed should be documented at the top of the module.
--------------------------------------------------------------------------*/
void
DCE_hangup()
{
	DEBUG(4, "--> hanging up %s\n", dce_name);
	init_T3000();

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

T3000-specific comments:
 S0=0        dont allow connect while dialing
 S63=0       pass BREAK signal to remote modem in sequence
 S64=0       abort dialing if characters sent by DTE
 S66=1       lock the interface speed
--------------------------------------------------------------------------*/
int
DCE_dial(telno_str)
char *telno_str;
{
	char cmd[128];
	char phone[50];
	int s111_set = 0;
	int timeout;
	int result;
	int rrings = 0;
	long then;
	long now;
	char *cptr;
	char *dialout_default = "AT S0=0 S7=40 S63=0 S64=0 S66=1\r";

#define MDVALID	 "0123456789FfKkMmNnRrSsUuWwXxVv*#,!/()-"
#ifdef WHT
#define RRING_MAX 3
#else
#define RRING_MAX 6
#endif

/* preliminary setup */
	translate("=,-,", telno_str);
	if (strspn(telno_str, MDVALID) != strlen(telno_str))
	{
		DEBUG(1, "phone number has invalid characters\n", 0);
		return (RC_FAIL | RCE_PHNO);
	}
	if (decode_phone_number(telno_str, phone, sizeof(phone)))
	{
		DEBUG(1, "phone number too long\n", 0);
		return (RC_FAIL | RCE_PHNO);
	}

/* walk through dialer codes, doing custom setup */
	strcpy(cmd, "AT");
	cptr = cmd + strlen(cmd);
	if (dialer_codes['F' - 'A'])
	{
		DEBUG(5, "XON/XOFF FLOW CONTROL requested\n", 0);
		strcat(cmd, "S69=2");
	}
	if (dialer_codes['K' - 'A'])
	{
		DEBUG(5, "KERMIT requested\n", 0);
		strcat(cmd, "S111=10");
		s111_set++;
	}
	if (dialer_codes['X' - 'A'])
	{
		DEBUG(5, "XMODEM requested\n", 0);
		strcat(cmd, "S111=20");
		s111_set++;
	}
	if (dialer_codes['U' - 'A'])
	{
		DEBUG(5, "UUCP requested\n", 0);
		strcat(cmd, "S111=30");
		s111_set++;
	}

	if (dialer_codes['V' - 'A'])
	{
		DEBUG(5, "V.32 requested\n", 0);
		if (hiCBAUD != B9600)
		{
			DEBUG(1, "V.32 baud rate not 9600\n", 0);
			return (RC_FAIL | RCE_SPEED);
		}
		if ((dialer_codes['P' - 'A']) || s111_set)
		{
			DEBUG(1, "both PEP and V.32 requested\n", 0);
			return (RC_FAIL | RCE_ARGS);
		}
		strcat(cmd, "S50=6");
	}

	if ((dialer_codes['P' - 'A']) || s111_set ||
		((hiCBAUD >= B9600) && (!dialer_codes['V' - 'A'])))
	{
		if (hiCBAUD < B9600)
		{
			DEBUG(1, "baud rate not high enough for PEP\n", 0);
			return (RC_FAIL | RCE_SPEED);
		}
		if (dialer_codes['P' - 'A'])
			DEBUG(5, "PEP requested\n", 0);
		else
			DEBUG(5, "PEP inferred: speed >= 9600 and no V.32 requested\n", 0);

		dialer_codes['P' - 'A'] = 1;
		strcat(cmd, "S50=255");
	}

	init_T3000();

	DEBUG(2, "--> issuing default setup command\n", 0);
	lwrite(dialout_default);
	if (lread(5) != rOk)
	{
		DEBUG(1, "default dialout setup failed\n", 0);
		return (RC_FAIL | RCE_NULL);
	}

/* issue the custom setup command */
	if (*cptr)
	{
		DEBUG(2, "--> issuing custom setup cmd\n", 0);
		strcat(cmd, "\r");
		lwrite(cmd);
		if (lread(5) != rOk)
		{
			DEBUG(1, "custom modem setup failed\n", 0);
			return (RC_FAIL | RCE_NULL);
		}
	}

/*
 * calculate a timeout for the connect
 * allow a minimum of 40 seconds, but if V.32 or PEP, 90
 * also if long distance (North American calculation here)
 * make it 132 (S7 is calculated as timeout * .9)
 */
	timeout = 40;
	if ((phone[0] == '1') && (phone[0] != '0'))
		timeout = 132;
	if ((timeout < 90) && (dialer_codes['V' - 'A'] || dialer_codes['P' - 'A']))
		timeout = 90;
	for (cptr = phone; cptr = strchr(cptr, ','); cptr++)
		timeout += 2;		 /* add extra time for pause characters */
	DEBUG(4, "wait for connect = %d seconds\n", timeout);

	if (Debug > 8)
	{
		lwrite("AT&V\r");
		lread_ignore(40);
	}

/* indicate non-root should not see DTE->DCE traffic */
	secure = 1;

/*
 * build and issue the actual dialing command
 * if root, let him see number, otherwise just say "remote system"
 */
	DEBUG(1, "--> dialing %s\n", (!ecu_calling & uid)
		? "remote system" : telno_str);
#ifdef WHT
	if (!strncmp(*gargv, "ECU", 3))
		dialer_codes['S' - 'A'] = 1;
#endif
	sprintf(cmd, "ATM%dS7=%dDT%s\r",
		((dialer_codes['S' - 'A']) && !(dialer_codes['N' - 'A'])) ? 1 : 0,
		(timeout * 9) / 10, phone);

	/* cmd string can only be 80 characters including "AT" */
	if (strlen(cmd) > 80)
	{
		DEBUG(1, "phone number string too long\n", 0);
		cleanup(RC_FAIL | RCE_PHNO);
	}

	lwrite(cmd);

/* indicate non-root can see DTE->DCE traffic */
	secure = 0;

/* wait for connect */
  WAIT_FOR_CONNECT:
	time(&then);
	result = lread(timeout);
	if (!(result & rfConnect))
	{
		switch (result & rfMASK)
		{
			case rNoCarrier:
				return (RC_FAIL | ((rrings > 2) ? RCE_ANSWER : RCE_NOTONE));
			case rNoDialTone:
				return (RC_FAIL | RCE_NOTONE);
			case rBusy:
				return (RC_FAIL | RCE_BUSY);
			case rNoAnswer:
				return (RC_FAIL | RCE_ANSWER);
			case rRring:
				if (rrings++ >= RRING_MAX)
					return (RC_FAIL | RCE_ANSWER);
			case rDialing:
				time(&now);
				if ((timeout -= ((int)(then - now))) > 0)
					goto WAIT_FOR_CONNECT;
			case rError:
			default:
				return (RC_FAIL | RCE_NULL);
		}
	}

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
needs to clean up after a failure.  Note that if a dialing
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
