/* CHK=0xFF53 */
/*+-------------------------------------------------------------------------
	dceMPAD.c - DCE-specific portion of generic SCO UUCP dialer
	Driver for ATT Tridom MPAD VSAT modem emulation
	wht@wht.net
--------------------------------------------------------------------------*/
/*+:EDITS:*/
/*:01-24-1997-02:37-wht@yuriatin-SOURCE RELEASE 4.00 */
/*:09-11-1996-20:00-wht@yuriatin-3.48-major telnet,curses,structural overhaul */
/*:11-23-1995-11:20-wht@kepler-source control 3.37 for tsx-11 */
/*:11-14-1995-10:23-wht@kepler-3.37.80-source control point: SOCKETS */
/*:05-04-1994-04:39-wht@n4hgf-ECU release 3.30 */
/*:09-10-1992-13:59-wht@n4hgf-ECU release 3.20 */
/*:08-22-1992-15:38-wht@n4hgf-ECU release 3.20 BETA */
/*:02-02-1992-18:01-root@n4hgf-proper ordering of DCE_result entries */
/*:01-26-1992-15:30-wht@n4hgf-gendial 1.2 for ecu 3.20- better hangup */
/*:07-25-1991-12:58-wht@n4hgf-ECU release 3.10 */
/*:03-12-1991-19:11-wht@n4hgf-if ecu dialing, show complete call progress */
/*:11-29-1990-18:31-r@n4hgf-revision/1st releasable */
/*:11-29-1990-17:48-wht@n4hgf-creation */

#include "dialer.h"

/*
 * DCE_DTR_low_msec - milliseconds to hold DTR low to ensure DCE
 *                    sees the transition; this value may be changed
 *                    as necessary before each call to lflash_DTR(),
 * but, generally, a constant value will do.
 */
long DCE_DTR_low_msec = 50;

/*
 * DCE_DTR_high_msec - milliseconds DTR must remain high before the
 *                     DCE may be expected to be ready to be commanded
 */
long DCE_DTR_high_msec = 50;

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
char *DCE_name = "ATT Tridom MPAD";
char *DCE_revision = "1.37";

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
 * will abort the program with RC|FAIL|RCE_TIMOUT, lread_ignore()
 * will return -1.  You may use any value between 0 and 0x3FFFFFFF.
 * This module is the only consumer  of the codes, although they
 * are decoded by gendial.c's _lread().
 *
 * If one possible result is an "early substring" of another, like
 * "CONNECT" is of "CONNECT 1200", then put such results later in the
 * table than the larger result.
 *
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
#define rConnect300		(  300  | rfConnect)
#define rConnect1200	( 1200  | rfConnect)
#define rConnect2400	( 2400  | rfConnect)
#define rConnect4800	( 4800  | rfConnect)
#define rConnect9600	( 9600  | rfConnect)
#define rConnect9600	( 9600  | rfConnect)
#define rConnect19200	(19200  | rfConnect)
#define rConnect38400	(38400  | rfConnect)

DCE_RESULT DCE_results[] =
{
	{"OK", rOk,},
	{"NO CARRIER", rNoCarrier,},
	{"ERROR", rError},
	{"BUSY", rBusy},
	{"NO ANSWER", rNoAnswer},
	{"NO DIAL TONE", rNoDialTone},
	{"KDIR SENDX ERROR", rNoDialTone},	/* MPAD software error */
	{"KDIR UNRESPONSIVE", rNoDialTone},
	{"KDIR BUSY", rBusy},
	{"KDIR BAD REQUEST", rError},	/* number 0 or >11 length */
	{"NO SUCH NUMBER", rNoAnswer},
	{"KDIR LOGIC ERROR", rNoDialTone},	/* MPAD software error */
	{"KDIR NOT AVAIL", rNoDialTone},
	{"TP4 STATE ERROR", rNoDialTone},	/* rain fade */
	{"TP4 GIVE UP", rBusy},	 /* rain fade */
	{"TP4 ERROR RESP", rBusy},	/* transient problem */
	{"NET DEACT", rNoDialTone},	/* net op deactivated port */
	{"CONNECT 300", rConnect300},
	{"CONNECT 1200", rConnect1200},
	{"CONNECT 4800", rConnect4800},
	{"CONNECT 9600", rConnect9600},
	{"CONNECT 19200", rConnect19200},
	{"CONNECT 38400", rConnect38400},
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
	sync_MPAD() - sync modem with our DTE speed
--------------------------------------------------------------------------*/
void
sync_MPAD()
{
	register int maxretry = 8;
	register int count;
	unsigned char rdchar;

	while (maxretry--)
	{
		lflush();
		write(dce_fd, "a", 1);
		count = 5;
		while (count)		 /* wait 50-200 msec for character, depending
							  * on HZ */
		{
			if (Rdchk(dce_fd))
				break;
			Nap(50L);
			count--;
		}
		if (count && (read(dce_fd, &rdchar, 1) == 1) && (rdchar == 'a'))
			return;
		write(dce_fd, "atq0v1e1\r", 9);
		Nap(500L);
	}

	DEBUG(1, "MPAD SYNC FAILED\n", 0);
	myexit(RC_FAIL | RCE_TIMOUT);

}							 /* end of sync_MPAD */

/*+-------------------------------------------------------------------------
	init_MPAD() - init MPAD from scratch, assuming nothing
--------------------------------------------------------------------------*/
void
init_MPAD()
{
	register itmp;
	int maxretry = 4;
	char *init0 = "ATE0Q0V1X99S0=1S2=255\r";

	DEBUG(1, "--> reseting %s\n", dce_name);
	lflash_DTR();
	sync_MPAD();

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
		DEBUG(1, "reset failed\n", 0);
		myexit(RC_FAIL | RCE_TIMOUT);
	}

}							 /* end of init_MPAD */

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
	DEBUG(1, "--> hanging up %s\n", dce_name);
	lflash_DTR();
	init_MPAD();

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

MPAD Plus-specific comments:
 Q0          do not be quiet
 E0          do not echo
 V1          verbal result codes
 S0=0        dont allow connect while dialing
 X99         full result codes
--------------------------------------------------------------------------*/
int
DCE_dial(telno_str)
char *telno_str;
{
	char cmd[128];
	char phone[50];
	int timeout;
	int result;
	char *cptr;
	char *dialout_default = "ATQ0E0V1E0S0=0X99\r";

#define MDVALID	 "0123456789NnSs()-"

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

	DEBUG(1, "--> issuing default setup command\n", 0);
	sync_MPAD();
	lwrite(dialout_default);
	if (lread(5) != rOk)
	{
		DEBUG(1, "default dialout setup failed\n", 0);
		return (RC_FAIL | RCE_NULL);
	}

/* issue the custom setup command */
	if (*cptr)
	{
		DEBUG(1, "--> issuing custom setup cmd\n", 0);
		strcat(cmd, "\r");
		sync_MPAD();
		lwrite(cmd);
		if (lread(5) != rOk)
		{
			DEBUG(1, "custom modem setup failed\n", 0);
			return (RC_FAIL | RCE_NULL);
		}
	}

/*
 * calculate a timeout for the connect
 */
	timeout = 20;
	DEBUG(6, "wait for connect = %d seconds\n", timeout);

/* indicate non-root should not see DTE->DCE traffic */
	secure = 1;

/*
 * build and issue the actual dialing command
 * if root, let him see number, otherwise just say "remote system"
 */
	DEBUG(1, "--> dialing %s\n", (!ecu_calling & uid) ? "remote system" : phone);
	sprintf(cmd, "ATS7=%dDT%s\r", (timeout * 9) / 10, phone);

	/* cmd string can only be 40 characters including "AT" */
	if (strlen(cmd) > 40)
	{
		DEBUG(1, "phone number string too long\n", 0);
		cleanup(RC_FAIL | RCE_PHNO);
	}

	sync_MPAD();
	lwrite(cmd);

/* indicate non-root can see DTE->DCE traffic */
	secure = 0;

/* wait for connect */
	result = lread(timeout);
	if (!(result & rfConnect))
	{
		switch (result & rfMASK)
		{
			case rNoCarrier:
				return (RC_FAIL | RCE_NOCARR);
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
command line, such as for -z processing.

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
