/*+-------------------------------------------------------------------------
	dceT2500.c - DCE-specific portion of generic SCO UUCP dialer
	Driver for Telebit T2500
	wht@wht.net

after init
E0 F1 M0 Q4 P V1 W0 X3 Y0 &P0 &T4     Version GE6.01-T2500
S00=001 S01=000 S02:001 S03=013 S04=010 S05=008 S06=002 S07=040 S08=002 S09=006
S10=007 S11:050 S12=050 S18=000 S25=005 S26=000 S38=000
S41=000 S45:001 S47=004 S48:001 S49=000
S50=000 S51:252 S52:004 S54:003 S55=000 S56=017 S57=019 S58:002 S59=000
S61=150 S62=003 S63=001 S64:001 S65=000 S66=000 S67=000 S68=255 S69=000
S90=000 S91=000 S92:001 S93=008 S94=001 S95=000 S96=001 S97=000 S98=003
S100=000 S101=000 S102=000 S104=000 S105=001 S106=000 S107=020
S110=255 S111=255 S112=001
S121=000 S130=002 S131:001
S150=000 S151=004 S152=001 S153=001 S154=000 S155=000
S160=010 S161=020 S162=002 S163=003 S164=007 S255=000
--------------------------------------------------------------------------*/
/*+:EDITS:*/
/*:01-24-1997-02:37-wht@yuriatin-SOURCE RELEASE 4.00 */
/*:09-11-1996-20:00-wht@yuriatin-3.48-major telnet,curses,structural overhaul */
/*:11-23-1995-11:20-wht@kepler-source control 3.37 for tsx-11 */
/*:11-14-1995-10:23-wht@kepler-3.37.80-source control point: SOCKETS */
/*:05-02-1995-16:01-wht@n4hgf-"--> dialing" report fix */
/*:05-04-1994-04:39-wht@n4hgf-ECU release 3.30 */
/*:09-10-1992-13:59-wht@n4hgf-ECU release 3.20 */
/*:08-22-1992-15:38-wht@n4hgf-ECU release 3.20 BETA */
/*:07-27-1992-05:00-wht@n4hgf-no pacing */
/*:02-10-1992-21:43-root@n4hgf-made it 1.5 years not setting S131 - then drat */
/*:02-10-1992-00:27-wht@n4hgf-improved sync_Telebit */
/*:01-26-1992-15:30-wht@n4hgf-gendial 1.2 for ecu 3.20- better hangup */
/*:07-25-1991-12:58-wht@n4hgf-ECU release 3.10 */
/*:03-12-1991-19:11-wht@n4hgf-if ecu dialing, show complete call progress */
/*:11-29-1990-18:31-r@n4hgf-revision/1st releasable */
/*:08-14-1990-20:40-wht@n4hgf-ecu3.00-flush old edit history */

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
long DCE_write_pace_msec = 0;

/*
 * DCE_name     - short name for DCE
 * DCE_revision - revision number for this module
 */
char *DCE_name = "Telebit T2500";
char *DCE_revision = "1.30";

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
#define rfREL			0x00400000
#define rfFAST			0x00200000
#define rfV32			0x00100000
#define rfMASK			0x0000FFFF	/* mask off rfBits */

/* unique codes */
#define rOk				0
#define rNoCarrier		1
#define rError			2
#define rNoDialTone 	3
#define rBusy			4
#define rNoAnswer		5
#define rRring			6
#define rConnect300		(  300  | rfConnect)
#define rConnect1200	( 1200  | rfConnect)
#define rConnect2400	( 1200  | rfConnect)
#define rConnect300R	(  300  | rfConnect | rfREL)
#define rConnect1200R	( 1200  | rfConnect | rfREL)
#define rConnect2400R	( 2400  | rfConnect | rfREL)
#define rConnectFASTK	(19200  | rfConnect | rfFAST)	/* may be 9600 */
#define rConnectFASTX	(19200  | rfConnect | rfFAST)
#define rConnectFASTU	(19200  | rfConnect | rfFAST)
#define rConnectFAST	(19200  | rfConnect | rfFAST)
#define rConnect9600	( 9600  | rfConnect | rfV32)

DCE_RESULT DCE_results[] =
{
	{"OK", rOk,},
	{"NO CARRIER", rNoCarrier,},
	{"ERROR", rError},
	{"NO DIALTONE", rNoDialTone,},
	{"BUSY", rBusy},
	{"NO ANSWER", rNoAnswer},
	{"RRING", rRring},
	{"CONNECT 300/REL", rConnect300R},
	{"CONNECT 1200/REL", rConnect1200R},
	{"CONNECT 2400/REL", rConnect2400R},
	{"CONNECT 300", rConnect300},
	{"CONNECT 1200", rConnect1200},
	{"CONNECT 2400", rConnect2400},
	{"CONNECT 9600", rConnect9600},
	{"CONNECT FAST/KERM", rConnectFASTK},
	{"CONNECT FAST/XMDM", rConnectFASTX},
	{"CONNECT FAST/UUCP", rConnectFASTU},
	{"CONNECT FAST", rConnectFAST},
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
	init_T2500() - init T2500 from scratch, assuming nothing

	reset to factory defaults, then set
    E0          no local echo in command mode
    F1          no local echo in data transfer mode
    M0          speaker off
    Q4          generate reult codes, but not RING
    V1          verbal result codes
    X3          extended result codes
    S0=1        answer on first ring
    S2=255        escape to unusual value
    S11=50      50 msec DTMF timing
    S45=1       enable remote access
    S48=1       all 8 bits are significant
    S50=0       use automatic connect speed determination
    S51=252     set serial port baud rate automatically (no typeahead)
	S52=4       DTR low: drop connection and go into command mode
    S54=3       pass BREAK signal to remote modem
    S55=0       respond to command escape sequence
    S58=2       DTE uses CTS/RTS flow control.
    S64=1       ignore characters sent by DTE while answering
    S66=0       don't lock interface speed, just go with the flow.
    S68=255     DCE uses whatever flow control DTE uses
    S92=1       PEP tones at the end of answer sequence
    S95=0       no MNP
    S110=255    use data compression when the remote modem requests it.
    S111=255    accept any protocol
	S131=1      DCD follows carrier

The nvram is set to factory + E0 Q0 S52=4 S131=1 S51=252
--------------------------------------------------------------------------*/
void
init_T2500()
{
	register itmp;
	int maxretry = 4;
	char *init0 = "AT&F E0 Q0 S52=4 S131=1 S51=252 &w F1 M0 Q4 V1 X3\r";
	char *init1 = "ATS0=1 S2=255 S11=50 S45=1 S48=1 S50=0 S54=3\r";
	char *init2 = "ATS55=0 S58=2 S64=1 S66=0 S68=255 S92=1 S95=0 S110=255 S111=255\r";

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

}							 /* end of init_T2500 */

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
	init_T2500();

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

T2500-specific comments:
 S0=0        dont allow connect while dialing
 S54=3       pass BREAK signal to remote modem
 S64=0       abort dialing if characters sent by DTE
 S66=1       lock the interface speed
 S110=0      disable data compression unless requested otherwise
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
	char *dialout_default = "ATS0=0S7=40S54=3S64=0S66=1S110=0\r";

#define MDVALID	 "0123456789CcEeFfKkMmNnPpRrSsUuWwXxVv*#,!/()-"
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
	if (dialer_codes['C' - 'A'])
	{
		DEBUG(5, "COMPRESSION requested\n", 0);
		strcat(cmd, "S110=1");
	}
	if (dialer_codes['E' - 'A'])
	{
		DEBUG(5, "ECHO SUPPRESSION requested\n", 0);
		strcat(cmd, "S121=1");
	}
	if (dialer_codes['F' - 'A'])
	{
		DEBUG(5, "XON/XOFF FLOW CONTROL requested\n", 0);
		strcat(cmd, "S58=3");
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
	if (dialer_codes['M' - 'A'])
	{
		DEBUG(5, "MNP requested\n", 0);
		strcat(cmd, "S95=1");
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

	init_T2500();

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

/* indicate non-root should not see DTE->DCE traffic */
	secure = 1;

/*
 * build and issue the actual dialing command
 * if root, let him see number, otherwise just say "remote system"
 */
	DEBUG(1, "--> dialing %s\n", (ecu_calling | !uid)
		? telno_str : "remote system");
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
