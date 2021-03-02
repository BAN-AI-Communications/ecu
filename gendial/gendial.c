/* CHK=0xC086 */
char *revision = "1.32";

/*+-------------------------------------------------------------------------
	gendial.c - SCO UUCP dialer program device independent portion
	wht@wht.net

  Configuration symbols:
	HDB_UUCP		defined if HDB UUCP used on system, else old Version 2

  Defined functions:
	RCE_text(value)
	SIGALRM_alert(sig)
	_lputc(lchar)
	_lputc_paced(pace_msec, lchar)
	_lputs(string)
	_lputs_paced(pace_msec, string)
	_lread(rtime, error_ok)
	call_ungetty(call_type)
	ck_sigint()
	cleanup(code)
	decode_phone_number(userphno, result, resultlen)
	dial_abort(sig)
	display_termio(ttt, text)
	get_uucp_uid()
	instr(s1, s2)
	lbreak()
	lflash_DTR()
	lflush()
	lread(rtime)
	lread_ignore(rtime)
	ltd_report()
	lwrite(str)
	main(argc, argv)
	make_printable(ch)
	myexit(code)
	open_dce()
	Rdchk(fd)
	translate(ttab, str)

  Usage:	dial ttyname telnumber speed
			dial -h ttyname speed

  ttyname may be of style "ttyxx" or "/dev/ttyxx" (this is not standard)

  Returns:
		0x80	bit = 1 if connection failed
		0x10	bit = 1 if line is also used for dialin #if !defined(OLDUUCP)
		0x0f	if msb=1: error code
				if msb=0: connected baud rate (0=same as dialed baud)
                Note: this dialer always returns 0 in the low nibble
                since cu and uucp expect it

  Note: getty calls the dialer with -h whenever it starts up on a line
  enabled in /etc/ttys and listed in Devices with this dialer.

  Error codes are split into two categories:

    1) (codes 0-11) Local problems are defined as tty port, or DCE
    problems: problems that can be worked around by using a different
    device.

    2) (codes 12-15) Remote problems are phone busy, no answer, etc.:
    attempt to connect to this remote system should be stopped.

  Note: This dialer can be used both for the old "Version 2"
  new HoneyDanBer UUCP.  In HDB, uugetty is used and ungetty is not
  necessary. Define HDB_UUCP for HDB UUCP.

  Note: This version of the dialer will NOT display the telephone number
  on the console unless the actual uid is root or ECU is calling as
  detected by a parse of argv[0].  Now, if uucico would just suppress
  username and password information it emits to debug (some do).

  See below for a description of lread() timing debug.

--------------------------------------------------------------------------*/
/*+:EDITS:*/
/*:01-24-1997-02:37-wht@yuriatin-SOURCE RELEASE 4.00 */
/*:10-24-1996-14:31-wht@yuriatin-fix errors in code long dormant */
/*:09-11-1996-20:00-wht@yuriatin-3.48-major telnet,curses,structural overhaul */
/*:08-20-1996-12:39-wht@kepler-locale/ctype fixes from ache@nagual.ru */
/*:11-23-1995-11:20-wht@kepler-source control 3.37 for tsx-11 */
/*:11-14-1995-10:23-wht@kepler-3.37.80-source control point: SOCKETS */
/*:05-04-1994-04:39-wht@n4hgf-ECU release 3.30 */
/*:05-19-1993-11:49-wht@n4hgf-use default DCE_results code */
/*:09-10-1992-13:59-wht@n4hgf-ECU release 3.20 */
/*:08-22-1992-15:38-wht@n4hgf-ECU release 3.20 BETA */
/*:08-16-1992-03:08-wht@n4hgf-head off another POSIX plot */
/*:08-10-1992-04:01-wht@n4hgf-use init_Nap */
/*:07-17-1992-18:28-wht@n4hgf-remove Nap() and use common ../nap.o */
/*:05-11-1992-17:54-wht@gyro-no naps in lflash_DTR on sun */
/*:05-11-1992-16:43-wht@gyro-fix WORKING_SELECT nap once and for all */
/*:03-30-1992-14:18-root@n4hgf-add lbreak */
/*:03-29-1992-12:30-cma-removed sigint reference in nap */
/*:02-02-1992-19:37-root@n4hgf-add ltd */
/*:02-02-1992-17:45-root@n4hgf-_lread: allow rtime secs after each character */
/*:01-26-1992-15:31-wht@n4hgf-gendial 1.2 for ecu 3.20- better hangup */
/*:08-13-1991-14:36-wht@n4hgf-perror on dce open error */
/*:07-25-1991-12:58-wht@n4hgf-ECU release 3.10 */
/*:05-01-1991-21:28-wht@n4hgf-add dial timing */
/*:03-12-1991-19:11-wht@n4hgf-if ecu dialing, show complete call progress */
/*:07-19-1990-17:14-root@n4hgf-modify lread fata timeout handler */
/*:05-26-1990-02:15-wht@n4hgf-creation */

#include "../ecu_config.h"
#include "dialer.h"

/*
 * lread() timing debug
 *
 * if LTD_DEBUG_LEVEL is defined, it should be set to the DEBUG level
 * at which a DCE result arrival report should be printed in the event
 * of a DCE timeout.  This normally would be set at 6, the debugging
 * level for the timeout report itself.  Sample output:
 *
 * Arrival Report for 16 characters (in msec)
 *      0 A    19624 ^M   19624 ^J   19624 C    19624 O    19624 N    19624 N
 *  19624 E    19624 C    19624 T    19872      19872 F    19872 A    19872 S
 *  19888 T    19888 ^M
 *
 * comment out the following line for no LTD.
 */
#define LTD_DEBUG_LEVEL 6
#if defined(LTD_DEBUG_LEVEL)
struct ltd
{
	unsigned char rdchar;
	struct TIMEB timeb;
};
struct ltd ltds[MAXLINE];
int ltd_count = 0;
struct TIMEB ltd_initial_timeb;

#endif /* LTD_DEBUG_LEVEL */

#undef NULL					 /* some stdio and param.h define these
							  * differently */
#include <sys/param.h>
#ifndef NULL				 /* fake usual sys/param.h value */
#define NULL 0
#endif

long Nap();

/* must be defined by device dependent module */
extern long DCE_DTR_low_msec;/* msecs DTR must be low to be recognized */
extern long DCE_DTR_high_msec;	/* msecs for DCE to recover */
extern long DCE_write_pace_msec;	/* msecs between chars written to DCE */
extern DCE_RESULT DCE_results[];	/* DCE result codes */
extern char *DCE_name;		 /* name of DCE */
extern char *DCE_revision;	 /* DCE-dependent code revision */
extern short DCE_hangup_CBAUD;	/* BXXX DCE hangup baud rate or zero */

/* globals available to device dependent module */
int gargc;					 /* global copy of main's argv */
char **gargv;				 /* global copy of main's argv */
char *dce_name;				 /* full pathname of ACU device */
char *telno = (char *)0;	 /* phone number if dial type request */
struct termio dce_termio;	 /* last termio for device */
int Debug = DBG;			 /* set per -x flag */
int dialing = 0;			 /* set while dialing in progress */
int dce_fd = -1;			 /* file descriptor for dce_name */
int DialerExitCode = RC_FAIL;/* return code */
int status = 0;				 /* set on errors */
int hangup_flag = 0;		 /* set when DCE being hung up */
int hiCBAUD;				 /* highest permissible baud rate */
int loCBAUD;				 /* lowest permissible baud rate */
struct passwd *passwd;
int uid;					 /* user id of executor */
int uid_uucp;				 /* user id of uucp */
int secure = 0;				 /* non-zero to suppress display of secure DCE
							  * traffic */
int ecu_calling = 0;		 /* true if ecu is dialing */
int sigint = 0;				 /* dummy for nap.c */

unsigned char dialer_codes[26];	/* A-Z embedded phone number codes */

jmp_buf SIGALRM_alert_jmpbuf;
DCE_RESULT *last_result;

/*+-------------------------------------------------------------------------
	get_uucp_uid()
--------------------------------------------------------------------------*/
int
get_uucp_uid()
{

	passwd = getpwnam("uucp");
	endpwent();
	if (passwd)
		return (passwd->pw_uid);
	else
		return (-1);
}							 /* end of get_uucp_uid */

/*+-------------------------------------------------------------------------
	instr(s1,s2)

  find s2 in s1; returns 1 if found, 0 if not found
--------------------------------------------------------------------------*/
instr(s1, s2)
register char *s1;
char *s2;
{
	register len = strlen(s2);

	while (s1 = strchr(s1, *s2))
	{
		if (!strncmp(s2, s1, len))
			return (1);
		s1++;
	}
	return (0);
}							 /* end of instr */

/*+-------------------------------------------------------------------------
	translate(ttab,str)

  translate the pairs of characters present in the first string
  whenever the first of the pair appears in the second string
  (this routine from standard SCO dialer code)
--------------------------------------------------------------------------*/
void
translate(ttab, str)
register char *ttab;
char *str;
{
	register char *cptr;

	while (*ttab && *(ttab + 1))
	{
		for (cptr = str; *cptr; cptr++)
		{
			if (*ttab == *cptr)
				*cptr = *(ttab + 1);
		}
		ttab += 2;
	}
}							 /* end of translate */

/*+-------------------------------------------------------------------------
	decode_phone_number(userphno,result,resultlen)

decode user flags in phone number, returning phone number in
result, character flags in global dialer_codes[], 'A' or 'a'
results in dialer_codes[0] being 1, etc.  Only letter codes are
extracted.

For example, if userphno contains "123,D45f", result returned
"123,45" and only elements 3 and 5 of dialer_codes set to 1

Function returns 0 if successful, -1 if result buffer too small
--------------------------------------------------------------------------*/
int
decode_phone_number(userphno, result, resultlen)
register unsigned char *userphno;
register unsigned char *result;
int resultlen;
{
	register itmp;

	for (itmp = 0; itmp < sizeof(dialer_codes); itmp++)
		dialer_codes[itmp] = 0;

	if (!resultlen)
		return (-1);
	resultlen--;			 /* leave room for null */

	while (*userphno)
	{
		if (isalpha(*userphno))
			dialer_codes[*userphno - ((isupper(*userphno)) ? 'A' : 'a')] = 1;
		else
		{
			if (!resultlen--)
				return (-1);
			*result++ = *userphno;
		}
		userphno++;
	}
	*result = 0;
	return (0);
}							 /* end of decode_phone_number */

/*+-------------------------------------------------------------------------
	make_printable(ch) - make a character "printable"
--------------------------------------------------------------------------*/
char *
make_printable(ch)
unsigned char ch;
{
	static char buffer[10];
	char *cptr;

#define	to_print(CFG_SigType)	((CFG_SigType)<' '?((CFG_SigType)+'@'):'?')

	cptr = buffer;
	/* if not root or uucp and info needs securing */
	if (!ecu_calling && uid && (uid != uid_uucp) && secure)
	{
		*cptr++ = '?';		 /* hide it */
		*cptr = 0;
		return (buffer);
	}

	if (iscntrl(ch) || !isprint(ch))
	{
		if (!isascii(ch))
		{					 /* Top bit is set */
			*cptr++ = 'M';
			*cptr++ = '-';
			ch = toascii(ch);/* Strip it */
		}
		if (iscntrl(ch))
		{
			*cptr++ = '^';
			ch = to_print(ch);	/* Make it printable */
		}
	}
	*cptr++ = ch;
	*cptr = 0;
	return (buffer);
}							 /* end of make_printable */

/*+-------------------------------------------------------------------------
	RCE_text(value)
--------------------------------------------------------------------------*/
char *
RCE_text(value)
int value;
{
	static char errant[32];

	switch (value & 0x0F)
	{
		case RCE_NULL:
			return ("unknown or unclassified error");
		case RCE_INUSE:
			return ("line in use");
		case RCE_SIG:
			return ("killed with signal");
		case RCE_ARGS:
			return ("invalid arguments");
		case RCE_PHNO:
			return ("invalid phone number");
		case RCE_SPEED:
			return ("invalid line speed or bad connect speed");
		case RCE_OPEN:
			return ("cannot open line");
		case RCE_IOCTL:
			return ("ioctl error");
		case RCE_TIMOUT:
			return ("timeout");
		case RCE_NOTONE:
			return ("NO DIAL TONE");
		case RCE_HANGUP:
			return ("hangup failed\n");
		case RCE_NORESP:
			return ("DCE didn't respond.\n");
		case RCE_BUSY:
			return ("BUSY");
		case RCE_NOCARR:
			return ("NO CARRIER");
		case RCE_ANSWER:
			return ("NO ANSWER");
	}
	sprintf(errant, "code 0x%04x", value);
	return (errant);

}							 /* end of RCE_text */

/*+-------------------------------------------------------------------------
	myexit(code) - all threads exit() thru here
--------------------------------------------------------------------------*/
void
myexit(code)
int code;
{
	alarm(0);
	if (dialing)
	{
		if (code & RC_FAIL)
		{
			DEBUG(1, "dial failed: %s\n", RCE_text(code));
		}
		else
		{
			DEBUG(1, "dial succeeded\n", 0);
		}
	}
	DCE_exit(code);			 /* should not return */
	exit(code);				 /* in case it does */

}							 /* end of myexit */

/*+-------------------------------------------------------------------------
	dial_abort(sig)
--------------------------------------------------------------------------*/
CFG_SigType
dial_abort(sig)
int sig;
{
	if (sig)
	{
		DEBUG(1, "\ndialer received signal %d\n\n", sig);
	}
	else
	{
		DEBUG(1, "\ndialer aborted, fail status=0x%02x\n", DialerExitCode);
	}
	DCE_abort(sig);
	if (dce_fd != -1)
	{
		ioctl(dce_fd, TCGETA, &dce_termio);
		dce_termio.c_cflag |= HUPCL;	/* make sure DCE hangs up */
		ioctl(dce_fd, TCSETA, &dce_termio);
		close(dce_fd);
	}
	if (sig)
		DialerExitCode |= (RC_FAIL | RCE_SIG);
	myexit(DialerExitCode);
}							 /* end of dial_abort */

/*+-------------------------------------------------------------------------
	cleanup(code) - close device and exit
--------------------------------------------------------------------------*/
void
cleanup(code)
int code;
{
	if (code & RC_FAIL)
	{						 /* if we failed, drop DTR (in dial_abort) */
		DialerExitCode = code;
		dial_abort(0);
	}
	else
		myexit(code);
}							 /* end of cleanup */

/*+-------------------------------------------------------------------------
	SIGALRM_alert(sig) - catch alarm call and do longjmp
--------------------------------------------------------------------------*/
CFG_SigType
SIGALRM_alert(sig)
int sig;
{
	longjmp(SIGALRM_alert_jmpbuf, 1);
}							 /* end of SIGALRM_alert */

/*+-------------------------------------------------------------------------
	ltd_report()
--------------------------------------------------------------------------*/
#if defined(LTD_DEBUG_LEVEL)
void
ltd_report()
{
	struct ltd *l = ltds;
	long msec;
	int col = 0;

	if (!ltd_count)
	{
		fputs("DCE was completely silent\n", stderr);
		return;
	}
	fprintf(stderr, "Arrival Report for %d characters (in msec)\n", ltd_count);
	while (ltd_count--)
	{
		msec = ((l->timeb.time - ltd_initial_timeb.time) * 1000L) +
			(l->timeb.millitm - ltd_initial_timeb.millitm);
		fprintf(stderr, "%6ld %-3.3s ", msec, make_printable(l->rdchar));
		l++;
		if (++col > 6)
		{
			col = 0;
			fputs("\n", stderr);
		}
	}
	if (col)
		fputs("\n", stderr);
	ltd_count = 0;

}							 /* end of ltd_report */
#endif /* LTD_DEBUG_LEVEL */

/*+-------------------------------------------------------------------------
	_lread(rtime,error_ok)

  Common code for lread() and lread_ignore()

  Returns DCE_RESULT->code from matching DCE_RESULT->result
  or if no match is found and the first digit of the modem
  response is numeric, the the numeric value is returned ored
  with 0x4000.

  If error_ok is true and a timeout occurs, -1 is returned.
  If error_ok is false and a timeout occurs,
     cleanup(RC_FAIL | RCE_TIMOUT | DialerExitCode);
     is called, which results in dial_abort(0) thus DCE_abort(0)
     being called.
--------------------------------------------------------------------------*/
int
_lread(rtime, error_ok)
int rtime;
int error_ok;
{
	int itmp;
	char rdchar;
	DCE_RESULT *mr;
	char buf[MAXLINE];
	char *bp;
	char *cptr;

#if defined(LTD_DEBUG_LEVEL)
	ltd_count = 0;
	if (Debug >= LTD_DEBUG_LEVEL)
		Ftime(&ltd_initial_timeb);
#endif /* LTD_DEBUG_LEVEL */

	if (error_ok)
	{
		signal(SIGALRM, SIGALRM_alert);
		if (setjmp(SIGALRM_alert_jmpbuf) != 0)
		{
			DEBUG(6, ">>-%s\n", "TIMEOUT");
#if defined(LTD_DEBUG_LEVEL)
			if (Debug >= LTD_DEBUG_LEVEL)
				ltd_report();
#endif /* LTD_DEBUG_LEVEL */
			return (-1);
		}
	}
	else
	{
		signal(SIGALRM, SIGALRM_alert);
		if (setjmp(SIGALRM_alert_jmpbuf) != 0)
		{
			DEBUG(6, ">>-%s\n", "TIMEOUT (FATAL)");
#if defined(LTD_DEBUG_LEVEL)
			if (Debug >= LTD_DEBUG_LEVEL)
				ltd_report();
#endif /* LTD_DEBUG_LEVEL */
			cleanup(RC_FAIL | RCE_TIMOUT | DialerExitCode);
		}
	}

	bp = buf;
	alarm(rtime);
	DEBUG(6, "DCE returned %s", "<<");

	while ((itmp = read(dce_fd, &rdchar, 1)) == 1)
	{
		alarm(rtime);		 /* allow rtime secs after each character */
		if (bp >= (buf + MAXLINE))
		{
			alarm(0);
			DEBUG(6, "\n>>-FAIL (%s)\n", "BUFFER OVERFLOW");
			myexit(RC_FAIL | RCE_NULL);
		}
		*bp++ = (rdchar &= 0x7F);
		*bp = 0;
#if defined(LTD_DEBUG_LEVEL)
		if (Debug >= LTD_DEBUG_LEVEL)
		{
			ltds[ltd_count].rdchar = rdchar;
			Ftime(&ltds[ltd_count++].timeb);
		}
#endif /* LTD_DEBUG_LEVEL */
		DEBUG(6, "%s", make_printable(rdchar));
		if (rdchar == 0x0A)
			DEBUG(6, "\n", 0);
		if (rdchar == '\r')
		{
			cptr = buf;
			if (*cptr == 0x0A)
				cptr++;
			for (mr = DCE_results; mr->result; ++mr)
			{
				if (instr(buf, mr->result))
				{
					alarm(0);
					DEBUG(6, ">>-%s\n", "SUCCESS");
					if (strcmp(mr->result, "OK"))	/* not so modem
													 * independent */
						DEBUG(6, "got %s\n", mr->result);
					else
						DEBUG(8, "got %s\n", mr->result);
					last_result = mr;
					return (mr->code);
				}
			}

			/*
			 * we ran through the whole table and are positioned on the
			 * last entry ... if the code is not -1, look for CONNECT as
			 * an initial sub string ... in this way, maybe an otherwise
			 * incompatible dialer can be pressed into service in a pinch
			 */
			if ((mr->code != -1) && instr(buf, "CONNECT"))
				return (mr->code);

			if (isdigit((unsigned char)*cptr))
			{
				alarm(0);
				itmp = atoi(cptr);
				DEBUG(6, ">>-SUCCESS (NUMERIC RESULT %d)\n", itmp);
				return (rfNumeric | itmp);
			}

			bp = buf;
		}
	}

	alarm(0);
	if (Debug >= 6)
	{
		ff(se, ">>-FAIL (%s %d)",
			(itmp < 0) ? "READ ERRNO" : "READ LENGTH",
			(itmp < 0) ? errno : 0);
	}
	DEBUG(4, " incomplete or no response\n", 0);
	return (-1);
}							 /* end of _lread */

/*+-------------------------------------------------------------------------
	lread_ignore(rtime)

  Reads from the ACU until it finds a valid response (found in
  DCE_results), a numeric result code (e.g., S-register value), or times
  out after rtime seconds.  The numeric response feature is designed
  for Hayes-style DCEs and may not be useful for other DCE types

  Returns: DCE_RESULT code, numeric result + 128, or -1 on timeout or error
--------------------------------------------------------------------------*/
int
lread_ignore(rtime)
int rtime;
{
	return (_lread(rtime, 1));
}							 /* end of lread_ignore */

/*+-------------------------------------------------------------------------
	lread(rtime)

  Same as lread_ignore, but does not return on timeout or error
--------------------------------------------------------------------------*/
int
lread(rtime)
int rtime;
{
	int rtn = _lread(rtime, 0);

	if (rtn < 0)
		myexit(RC_FAIL | RCE_TIMOUT);
	return (rtn);
}							 /* end of lread */

/*+-------------------------------------------------------------------------
	lflush() - flushes input clists for DCE
--------------------------------------------------------------------------*/
void
lflush()
{
	ioctl(dce_fd, TCFLSH, 0);
}							 /* end of lflush */

/*+-----------------------------------------------------------------------
	_lputc(lchar) -- write char to comm line
------------------------------------------------------------------------*/
void
_lputc(lchar)
char lchar;
{
	write(dce_fd, &lchar, 1);
	DEBUG(6, "%s", make_printable(lchar));
}							 /* end of _lputc */

/*+-----------------------------------------------------------------------
	_lputc_paced(pace_msec,lchar) -- write char to comm line with pacing
------------------------------------------------------------------------*/
void
_lputc_paced(pace_msec, lchar)
register long pace_msec;
register char lchar;
{
	_lputc(lchar);
	if (pace_msec)
		Nap(pace_msec);
}							 /* end of _lputc_paced */

/*+-----------------------------------------------------------------------
	_lputs(string) -- write string to comm line
------------------------------------------------------------------------*/
void
_lputs(string)
register char *string;
{
	while (*string)
		_lputc(*string++);
}

/*+-----------------------------------------------------------------------
	_lputs_paced(pace_msec,string) -- write string to comm line
  with time between each character
------------------------------------------------------------------------*/
void
_lputs_paced(pace_msec, string)
register long pace_msec;
register char *string;
{
	while (*string)
		_lputc_paced(pace_msec, *string++);

}							 /* end of _lputs_paced */

/*+-------------------------------------------------------------------------
	lwrite(str) - output string to dce_name
  Returns:	0 on completion, -1 on write errors.
--------------------------------------------------------------------------*/
void
lwrite(str)
register char *str;
{

	Nap(200L);
	DEBUG(6, "Sent DCE %s", "<<");
	_lputs_paced(DCE_write_pace_msec, str);
	DEBUG(6, ">>-%s\n", "SUCCESS");
	ioctl(dce_fd, TCSETAW, &dce_termio);	/* wait for I/O to drain */

}							 /* end of lwrite */

/*+-------------------------------------------------------------------------
	lbreak()
--------------------------------------------------------------------------*/
void
lbreak()
{
	DEBUG(6, "Sent BREAK to DCE %s", "<<");
	ioctl(dce_fd, TCSBRK, (char *)1);
	DEBUG(6, ">>-%s\n", "SUCCESS");
}							 /* end of lbreak */

/*+-------------------------------------------------------------------------
	lflash_DTR() - flash DTR

DTR is lowered and raised again.  The timing can be modified on a
per-DCE basis.

On SunOS and SVR4, an open/close of the line is required to get DTR back
up. SVR3 does not seem to need this (ISC asy, SCO sio, Uwe Doering's FAS)
but we do it anyway
--------------------------------------------------------------------------*/
void
lflash_DTR()
{
#undef NEED_REOPEN
#if defined(sun) || defined(SVR4)
#define NEED_REOPEN
	int tempfd;

#endif
	struct termio b0t;

	b0t = dce_termio;
	b0t.c_cflag &= ~CBAUD;

	ioctl(dce_fd, TCSETA, (char *)&b0t);	/* drop DTR */
	DEBUG(6, "setting DTR low ... ", "");
#ifdef NEED_REOPEN
	if ((tempfd = open(dce_name, O_NDELAY | O_RDWR, 0777)) != -1)
		close(tempfd);
#else
	Nap((DCE_DTR_low_msec) ? DCE_DTR_low_msec : 300L);
#endif
	ioctl(dce_fd, TCSETA, (char *)&dce_termio);	/* raise DTR */
	DEBUG(6, "back to high", "");
	Nap((DCE_DTR_high_msec) ? DCE_DTR_high_msec : 300L);
	DEBUG(6, "\n", "");
#undef NEED_REOPEN
}							 /* end of lflash_DTR */

/*+-------------------------------------------------------------------------
	call_ungetty(call_type)

type: 'a' - acquire dce_name
      't' - test to see if dce_name should be returned
      'r' - return dce_name

This function is a no-op in HDB UUCP versions
--------------------------------------------------------------------------*/
call_ungetty(call_type)
char call_type;
{
#if defined(HDB_UUCP)
	switch (call_type)
	{
		case 'a':
			return (UG_NOTENAB);	/* simulate complete success */
		case 't':
			return (UG_RESTART);	/* simulate need for re-setup */
		case 'r':
			return (0);		 /* simulate complete success */
	}
	return (0);
#else /* HDB_UUCP */
	int itmp;
	int pid;
	unsigned int wait_status;
	static char *ungetty = "/usr/lib/uucp/ungetty";

	if ((pid = fork()) == 0)
	{
		if (Debug >= 5)
			ff(se, "%s: %s %s called\n", *gargv, ungetty, dce_name);
		switch (call_type)
		{
			case 'a':
				execl(ungetty, "ungetty", dce_name + 5, (char *)0);
				break;
			case 't':
				execl(ungetty, "ungetty", "-t", dce_name + 5, (char *)0);
				break;
			case 'r':
				execl(ungetty, "ungetty", "-r", dce_name + 5, (char *)0);
				break;
		}
		ff(se, "%s exec error %d (%s)\n", ungetty, errno, sys_errlist[errno]);
		_exit(-1);
	}

	while (((itmp = wait(&wait_status)) != pid) && itmp != -1)
		;

	if (Debug >= 6)
		ff(se, "%s pid %d exit status 0x%04x\n", ungetty, itmp, wait_status);

	return ((wait_status >> 8) & 0xFF);
#endif /* HDB_UUCP */
}							 /* end of call_ungetty */

/*+-----------------------------------------------------------------------
	display_termio(ttt)
  display termio 'ttt' on se
------------------------------------------------------------------------*/
void
display_termio(ttt, text)
struct termio *ttt;
char *text;
{
	register flag;
	register i_cc;
	register char *cptr;
	int dbits;
	char parity;

	ff(se, "----->> %s\n", text);

	flag = ttt->c_iflag;
	ff(se,
		"iflag: %07o IGNBRK:%d BRKINT:%d IGNPAR:%d PARMRK:%d INPCK:%d ISTRIP:%d\n",
		flag,
		(flag & IGNBRK) ? 1 : 0,
		(flag & BRKINT) ? 1 : 0,
		(flag & IGNPAR) ? 1 : 0,
		(flag & PARMRK) ? 1 : 0,
		(flag & INPCK) ? 1 : 0,
		(flag & ISTRIP) ? 1 : 0);
	ff(se,
		"       INLCR:%d IGNCR:%d ICRNL:%d IUCLC:%d IXON:%d IXANY:%d IXOFF:%d\n",
		(flag & INLCR) ? 1 : 0,
		(flag & IGNCR) ? 1 : 0,
		(flag & ICRNL) ? 1 : 0,
		(flag & IUCLC) ? 1 : 0,
		(flag & IXON) ? 1 : 0,
		(flag & IXANY) ? 1 : 0,
		(flag & IXOFF) ? 1 : 0);

	flag = ttt->c_oflag;
	ff(se,
		"oflag: %07o OPOST:%d OLCUC:%d ONLCR:%d OCRNL:%d ONOCR:%d ONLRET:%d OFDEL:%d\n",
		flag,
		(flag & OPOST) ? 1 : 0,
		(flag & OLCUC) ? 1 : 0,
		(flag & ONLCR) ? 1 : 0,
		(flag & OCRNL) ? 1 : 0,
		(flag & ONOCR) ? 1 : 0,
		(flag & ONLRET) ? 1 : 0,
		(flag & OFDEL) ? 1 : 0);

	flag = ttt->c_cflag;
	ff(se, "cflag: %07o ", ttt->c_cflag);
	switch (flag & CBAUD)
	{
		case B0:
			cptr = "HUP";
			break;
		case B50:
			cptr = "50";
			break;
		case B75:
			cptr = "75";
			break;
		case B110:
			cptr = "110";
			break;
		case B134:
			cptr = "134.5";
			break;
		case B150:
			cptr = "150";
			break;
		case B200:
			cptr = "200";
			break;
		case B300:
			cptr = "300";
			break;
		case B600:
			cptr = "600";
			break;
		case B1200:
			cptr = "1200";
			break;
		case B1800:
			cptr = "1800";
			break;
		case B2400:
			cptr = "2400";
			break;
		case B4800:
			cptr = "4800";
			break;
		case B9600:
			cptr = "9600";
			break;
		case EXTA:
			cptr = "EXTA(19200?)";
			break;
		case EXTB:
			cptr = "EXTB(38400?)";
			break;
		default:
			cptr = "????";
			break;
	}
	dbits = 5 + ((flag & CSIZE) >> 4);
	parity = (flag & PARENB) ? ((flag & PARODD) ? 'O' : 'E') : 'N';
	ff(se, "%s-%d-%c-%d ", cptr, dbits, parity, (flag & CSTOPB) ? 2 : 1);
	switch (flag & CS8)
	{
		case CS8:
			fputs("CS8 ", se);
			break;
		case CS7:
			fputs("CS7 ", se);
			break;
		case CS6:
			fputs("CS6 ", se);
			break;
		case CS5:
			fputs("CS5 ", se);
			break;
	}
	ff(se, "CREAD:%d HUPCL:%d CLOCAL:%d",
		(flag & CREAD) ? 1 : 0,
		(flag & HUPCL) ? 1 : 0,
		(flag & CLOCAL) ? 1 : 0);
#if defined(RTSFLOW)
	ff(se, " RTSFLOW:%d  CTSFLOW:%d",
		(flag & RTSFLOW) ? 1 : 0,
		(flag & CTSFLOW) ? 1 : 0);
#endif
	ff(se, "\n");

	flag = ttt->c_lflag;
	ff(se, "lflag: %07o ISIG:%d ICANON:%d XCASE:%d ECHO:%d ECHOE:%d\n",
		flag,
		(flag & ISIG) ? 1 : 0,
		(flag & ICANON) ? 1 : 0,
		(flag & XCASE) ? 1 : 0,
		(flag & ECHO) ? 1 : 0,
		(flag & ECHOE) ? 1 : 0);
	ff(se, "       ECHOK:%d  ECHONL:%d  NOFLSH:%d  \n",
		(flag & ECHOK) ? 1 : 0,
		(flag & ECHONL) ? 1 : 0,
		(flag & NOFLSH) ? 1 : 0);

	ff(se,
		"           INTR QUIT ERAS KILL EOF  EOL  EOL2 SWTCH VMIN==EOF VTIME==EOL\n");
	ff(se, "ctl chars: ");
	for (i_cc = 0; i_cc < NCC; i_cc++)
		ff(se, "%02x   ", ttt->c_cc[i_cc]);
	ff(se, "  (hex)\n");

}							 /* end of display_termio */

/*+-------------------------------------------------------------------------
	open_dce() - open the dce_name (DCE)
plugs global 'dce_fd' and returns fd of open DCE line
--------------------------------------------------------------------------*/
int
open_dce()
{
	int fd1;
	int fd2;

	/* open with O_NDELAY set or the open probably will hang */
	if ((fd1 = open(dce_name, O_RDWR | O_NDELAY)) < 0)
	{
		ff(se, "%s: Can't open device: ", *gargv);
		perror(dce_name);
		myexit(RC_FAIL | RCE_OPEN | DialerExitCode);
	}

	ioctl(fd1, TCGETA, &dce_termio);
	dce_termio.c_oflag = 0;
	dce_termio.c_iflag = 0;
	dce_termio.c_cflag &= ~(CBAUD | HUPCL);
	dce_termio.c_cflag |= CLOCAL |
		((hangup_flag) ? HUPCL : 0) |
		((hangup_flag && DCE_hangup_CBAUD) ? DCE_hangup_CBAUD : hiCBAUD);
	dce_termio.c_cflag |= CS8;
	dce_termio.c_cc[VMIN] = 1;
	dce_termio.c_cc[VTIME] = 0;
	dce_termio.c_cflag &= ~PARENB;
	dce_termio.c_lflag &= ~(ECHO | ICANON);
#ifdef IEXTEN
	dce_termio.c_lflag &= ~IEXTEN;
#endif

	if (Debug >= 10)

		display_termio(&dce_termio, "setting line termio");

	if (status = ioctl(fd1, TCSETA, &dce_termio))
	{
		DEBUG(1, "%s: ioctl error on %s", dce_name);
		DEBUG(1, "%s", dce_name);
		DEBUG(1, " errno=%d\n", errno);
		cleanup(RC_FAIL | RCE_IOCTL | DialerExitCode);
	}

	/* reopen line without O_NDELAY */
	fd2 = fd1;
	if ((fd1 = open(dce_name, O_RDWR)) < 0)
	{
		ff(se, "%s: Can't open device w/o O_NDELAY: %s\n", *gargv, dce_name);
		myexit(RC_FAIL | RCE_OPEN | DialerExitCode);
	}
	close(fd2);

	ioctl(fd1, TCFLSH, 2);	 /* flush any residue in clists */
	dce_fd = fd1;			 /* save fd in global */
	return (fd1);

}							 /* end of open_dce */

/*+-------------------------------------------------------------------------
	main(argc,argv)
--------------------------------------------------------------------------*/
main(argc, argv)
int argc;
char **argv;
{
	int itmp;
	int unrecognized_switches = 0;
	char *cptr;
	long startsec;
	extern int optind;
	extern int opterr;
	extern char *optarg;

	setbuf(stderr, NULL);
	setbuf(stdout, NULL);

/* security considerations */
	uid = getuid();
	uid_uucp = get_uucp_uid();
	secure = 0;
	if (!strncmp(argv[0], "ECU", 3))
		ecu_calling = 1;

	gargv = argv;
	gargc = argc;

	signal(SIGILL, dial_abort);
	signal(SIGIOT, dial_abort);
	signal(SIGFPE, dial_abort);
	signal(SIGBUS, dial_abort);
	signal(SIGSEGV, dial_abort);
	signal(SIGTERM, dial_abort);
	signal(SIGINT, dial_abort);

	signal(SIGCLD, SIG_DFL);

#if	defined(SIGSTOP)

	/*
	 * call Roto-Rooter on POSIX plots
	 */
	signal(SIGSTOP, SIG_IGN);
	signal(SIGTSTP, SIG_IGN);
	signal(SIGCONT, SIG_IGN);
	signal(SIGTTIN, SIG_IGN);
	signal(SIGTTOU, SIG_IGN);
#endif

	opterr = 0;
	while ((itmp = getopt(argc, argv, "hx:")) != EOF)
	{
		switch (itmp)
		{
			case 'h':
				hangup_flag++;
				break;
			case 'x':
				Debug = atoi(optarg);
				break;
			case '?':		 /* dialer-specific code may want to handle
							  * these */
				unrecognized_switches++;
				break;
		}
	}

/* learn tick rate for various timers */
	init_Nap();

/* announce who we are and our arguments if debugging */
	if (Debug > 5)
	{
		ff(se, "\ngeneric dialer %s (%s %s)\n", revision,
			DCE_name, DCE_revision);
		if (ecu_calling || !uid)
		{
			ff(se, "(args ");
			for (itmp = 0; itmp < argc; itmp++)
				ff(se, "%s ", argv[itmp]);
			fputs(")\n", se);
		}
	}
	if (Debug >= 8)
		ff(se, "uid = %d  euid=%d\n", uid, geteuid());

	chdir("/tmp");			 /* in case of core dump */

	/* give DCE-specific code a chance at the entire command line */
	if (status = DCE_argv_hook(argc, argv, optind, unrecognized_switches))
	{
		DEBUG(1, "dialer failed: %s\n", RCE_text(status));
		exit(status);
	}

/* check argument count */
	if (hangup_flag)
	{
		if ((argc - optind) != 2)
			status++;
	}
	else if ((argc - optind) != 3)
		status++;

/* die with usage if argument error */
	if (status)
	{
		if (hangup_flag)
			ff(se, "Usage: %s -h devicename speed\n", argv[0]);
		else
			ff(se, "Usage: %s devicename number speed\n", argv[0]);
		myexit(RC_FAIL | RCE_ARGS);
	}

/* if called with "ttyxx" style ttyname, convert to "/dev/ttyxx" */
	cptr = argv[optind++];
	if (!strncmp(cptr, "tty", 3))
	{
		char s32[32];

		strcpy(s32, "/dev/");
		strcat(s32, cptr);
		dce_name = strdup(s32);
	}
	else
		dce_name = cptr;

/* save phone number */
	if (!hangup_flag)
		telno = argv[optind++];

/* get baud rates (validated by DCE-dependent code) */
	loCBAUD = hiCBAUD = DCE_baud_to_CBAUD(atoi(argv[optind]));
	if (cptr = strchr(argv[optind], '-'))
		hiCBAUD = DCE_baud_to_CBAUD(atoi(++cptr));

	(void)open_dce();		 /* open the line */

/*   H A N G U P     R E Q U E S T   */
	if (hangup_flag)
	{
		if (call_ungetty('t') != UG_RESTART)
			cleanup(SUCCESS);
		DCE_hangup();
		cleanup((call_ungetty('r')) ? RC_FAIL : SUCCESS);
	}

/*   D I A L    R E Q U E S T    */

	switch (call_ungetty('a'))
	{
		case UG_NOTENAB:	 /* line acquired: not enabled */
			status = SUCCESS;
			break;
		case UG_ENAB:		 /* line acquired: need ungetty -r when done */
			status = RC_ENABLED;
			break;
		case UG_FAIL:		 /* could not acquire line */
			myexit(RC_FAIL | RCE_INUSE);
		case 255:
			myexit(RC_FAIL | RCE_NULL);
	}

	/*
	 * indicate we are dialing record the time and, oh yeah, dial
	 */
	dialing = 1;
	time(&startsec);
	status = DCE_dial(telno);/* if dial succeeded ... */
	if (Debug >= 2)
	{
		ff(se, "connect %s after %ld seconds\n",
			(status) ? "FAILED" : "succeeded",
			time((long *)0) - startsec);
	}

	/*
	 * success or not
	 */
	myexit(((status) ? RC_FAIL : 0) | status);
}							 /* end of main */

/*+-------------------------------------------------------------------------
	Rdchk(fd) - for systems without it but with FIONREAD
--------------------------------------------------------------------------*/
#if defined(CFG_FionreadRdchk)
int
Rdchk(fd)
int fd;
{
	int chars_waiting;

	if (ioctl(fd, FIONREAD, &chars_waiting))
	{
		if (Debug > 2)
		{
			fprintf(stderr, "!!! fd %d ", fd);
			perror("Rdchk FIONREAD");
		}
		return (0);
	}
	else
	{
		DEBUG(11, "Rdchk-%d\n", chars_waiting);
		return (!!chars_waiting);
	}
}							 /* end of Rdchk */
#endif

/*+-------------------------------------------------------------------------
    ck_sigint() - stub for ../nap.c
--------------------------------------------------------------------------*/
int
ck_sigint()
{
	return (0);
}							 /* end of ck_sigint */

/* end of gendial.c */
/* vi: set tabstop=4 shiftwidth=4: */
