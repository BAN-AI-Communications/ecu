/*+-----------------------------------------------------------------------
	ldserial.c -- ECU serial line discipline
	wht@wht.net

  Defined functions:
	display_hw_flow_config()
	lCLOCAL(flag)
	lRTSCTS_control(flag)
	lbreak()
	lclear_xmtr_xoff()
	lclose()
	lclose_failed(sig)
	ldcdwatch(flag)
	ldcdwatch_str(flagstr)
	ldraino(inflush_flag)
	lflash_dtr()
	lflush(flush_type)
	lget_xon_xoff(ixon, ixoff)
	lnew_bitrate(new_bitrate)
	lopen()
	lopen_failed(sig)
	lreset_ksr()
	lset_baud(ioctl_flag)
	lset_parity(ioctl_flag)
	lxon_xoff(flag)
	lzero_length_read_detected()
	set_xon_xoff_by_arg(arg)
	valid_baud(baud)
	xon_status()

  Note: With -DCFG_TermiosLineio, termios is used in lieu of
  termio.  See ecu.h and ecutermio.[ch] and search for
  CFG_TermiosLineio for vague illumination.

------------------------------------------------------------------------*/
/*+:EDITS:*/
/*:04-26-2000-11:16-wht@bob-RELEASE 4.42 */
/*:02-26-1998-03:10-wht@kepler-linux2/redhat */
/*:12-12-1997-21:24-wht@kepler-keep errno2 away from longjmp */
/*:01-24-1997-02:37-wht@yuriatin-SOURCE RELEASE 4.00 */
/*:09-11-1996-20:00-wht@yuriatin-3.48-major telnet,curses,structural overhaul */
/*:08-11-1996-02:10-wht@kepler-rename ecu_log_event to logevent */
/*:07-24-1996-21:04-wht@n4hgf-fix stupid termiox lapse of reason */
/*:07-24-1996-20:52-wht@n4hgf-need ecutermio.h */
/*:01-01-1996-18:06-wht@kepler-repair LINUX_ASYNC_HACK */
/*:12-28-1995-12:54-wht@kepler-Andrey Chernov FreeBSD fixes */
/*:12-12-1995-16:22-wht@kepler-no more telnet intercept in lopen */
/*:12-06-1995-13:30-wht@n4hgf-termecu w/errno -1 consideration */
/*:12-03-1995-20:47-wht@gyro-sun toggle DTR takes forever withOUT naps */
/*:12-03-1995-19:57-wht@gyro-use Setuid */
/*:11-23-1995-11:20-wht@kepler-source control 3.37 for tsx-11 */
/*:11-14-1995-10:23-wht@kepler-3.37.80-source control point: SOCKETS */
/*:11-03-1995-16:54-wht@wwtp1-use CFG_TelnetOption */
/*:10-18-1995-04:29-wht@kepler-always use select for nap */
/*:10-18-1995-04:16-wht@kepler-break out of eculine.c */

#include "ecu.h"
#include "ecukey.h"
#include "termecu.h"
#include "ecutermio.h"
#include <setjmp.h>
#include <pwd.h>

void lreset_ksr();
void lzero_length_read_detected();
void lCLOCAL();

extern int lgetc_count;

char lopen_err_str[64] = "";
int dcdwatch_set = 0;		 /* if true, ldcdwatch() has been called */
static jmp_buf _insurance_jmpbuf;
static int errno2;		/* keep away from longjmp */
static int attempt2;	/* keep away from longjmp */

#ifdef SVR4
int hx_flag;

#endif


/*
 * with SCO UNIX, nap doesn't work as advertized; param MUST be > granularity
 * or nap will return immediately; not a problem with XENIX
 */
#define LPUTS_NAP_COUNT	(min((hzmsec * 2),20L))

/*+-------------------------------------------------------------------------
	lflush(flush_type) -- flush line driver input &/or output buffers

0 == input buffer
1 == output buffer
2 == both buffers
--------------------------------------------------------------------------*/
void
lflush(flush_type)
int flush_type;
{

	if (shm->Ltelnet)
		return;

	switch (flush_type)
	{
		case 0:
			lgetc_count = 0;
			ecuflush(shm->Liofd, TCIFLUSH);
			break;
		case 1:
			ecuflush(shm->Liofd, TCOFLUSH);
			break;
		case 2:
			lgetc_count = 0;
			ecuflush(shm->Liofd, TCIOFLUSH);
			break;
	}
}							 /* end of lflush */

/*+-------------------------------------------------------------------------
	lreset_ksr()

  This procedure restores the termio for the
  comm line to the values in Ltermio
--------------------------------------------------------------------------*/
void
lreset_ksr()
{

	if (shm->Ltelnet)
		return;

	ecusetattr(shm->Liofd, TCSETA, Ltermio);

}							 /* end of lreset_ksr */

/*+-------------------------------------------------------------------------
	ldraino(inflush_flag) - wait for output to drain

If inflush_flag is set, also flush input after output drains
--------------------------------------------------------------------------*/
void
ldraino(inflush_flag)
int inflush_flag;
{
#if defined(CFG_TermiosLineio)
#if defined(sun) && !defined (SVR4)
	int retries = 50;
	int outq_count;
	int old_outq_count = 0;

	if (shm->Ltelnet)
		return;

	do
	{
		ioctl(shm->Liofd, TIOCOUTQ, &outq_count);
		if (!outq_count)
			break;
		if (old_outq_count == outq_count)	/* don't hang if flow control
											 * lock */
			retries--;
		old_outq_count = outq_count;
		Nap(50L);
	}
	while (outq_count && retries);
	if (inflush_flag)
		ecuflush(shm->Liofd, TCIFLUSH);
#else /* termios but not sunos */
	if (shm->Ltelnet)
		return;

	tcdrain(shm->Liofd);
	if (inflush_flag)
		ecuflush(shm->Liofd, TCIFLUSH);
#endif
#else /* !CFG_TermiosLineio */
	if (shm->Ltelnet)
		return;

	ecusetattr(shm->Liofd, (inflush_flag) ? TCSETAF : TCSETAW, Ltermio);
#endif

}							 /* end of ldraino */

/*+-------------------------------------------------------------------------
	valid_baud(baud) -- returns (positive) bit rate selector
  or -1 if invalid bit rate

  Thanks to Linux, this has become an unbelivable hack, but I
  have no time to handle things any better now.

  Linux has an unbelievable hack for baud values above 38400.
  (We draw a line here -- "baud rate" is a redundant, meaningless
  term).

  If you use TIOCGSERIAL/TIOCSSERIAL to get the tty's
  Linux-specific struct serial_struct structure, and set bits in
  it and plug it back with TIOCSSERIAL, then you can make Linux
  treat B38400 as 57600 or 115200.

  For Linux, 38400, 57600 and 115200 the CBAUD-style values returned
  values have a bit extra in them: the sign bit (2^31 anyway) is set
  and the proper ASYNC_SPD_MASK bits for a TIOCSSERIAL operation
  appear in the ASYNC_SPD_MASK << 16 position.

  Again, since I have so little time to document, you'll just have
  to look at code calling valid_baud() to see how this hack is used.

--------------------------------------------------------------------------*/
int
valid_baud(baud)
UINT baud;
{
	int rtn = -1;

	switch (baud)
	{
		case 110:
			rtn = B110;
			break;
		case 300:
			rtn = B300;
			break;
		case 600:
			rtn = B600;
			break;
		case 1200:
			rtn = B1200;
			break;
		case 2400:
			rtn = B2400;
			break;
		case 4800:
			rtn = B4800;
			break;
		case 9600:
			rtn = B9600;
			break;
#ifdef hpux
		case 19200:
			rtn = B19200;
			break;
		case 38400:
			rtn = B38400;
			break;
#else
#if 1
#ifdef B19200
		case 19200:
			rtn = B19200;
			break;
#else
		case 19200:
			rtn = EXTA;
			break;
#endif /* B19200 */
#ifdef B38400
#if defined(LINUX_ASYNC_HACK)
		case 38400:
			rtn = B38400 | (0x8000 << 16);
			break;
#else
		case 38400:
			rtn = B38400;
			break;
#endif
#else
		case 38400:
			rtn = EXTB;
			break;
#endif /* B38400 */
#endif /* 1 */
#endif /* hpux */

#if !defined(LINUX_ASYNC_HACK)
#ifdef B57600
		case 57600:
			rtn = B57600;
			break;
#endif
#ifdef B115200
		case 115200:
			rtn = B115200;
			break;
#endif
#endif /* ~LINUX_ASYNC_HACK */

/*
 * linux excruciating 57600 and 115200 hack
 */

#if defined(LINUX_ASYNC_HACK)
		case 57600:
			rtn = B38400 | ((0x8000 | ASYNC_SPD_HI) << 16);
			break;

		case 115200:
			rtn = B38400 | ((0x8000 | ASYNC_SPD_VHI) << 16);
			break;
#endif /* LINUX_ASYNC_HACK */

		default:
			rtn = -1;
			break;
	}

	return (rtn);

}							 /* end of valid_baud */

/*+-----------------------------------------------------------------------
	lset_baud(ioctl_flag)

  If 'ioctl_flag' is set, then perform ioctl call
  is executed after setting bit rate
------------------------------------------------------------------------*/
int
lset_baud(ioctl_flag)
int ioctl_flag;
{
	int cbaud_value = valid_baud(shm->Lbitrate);

	if (shm->Ltelnet)
		return (0);

	if (shm->Liofd < 0)
		return (-1);

	if (cbaud_value == -1)
		cbaud_value = valid_baud(shm->Lbitrate = CFG_DefaultBitRate);

	shm->Lmodem_already_init = 0;

#if defined(LINUX_ASYNC_HACK)
	if (cbaud_value & (0x8000 << 16))	/* 38400 or above */
	{
		struct serial_struct linux_serial_hack;

		ioctl(shm->Liofd, TIOCGSERIAL, (char *)&linux_serial_hack);
		linux_serial_hack.flags &= ~ASYNC_SPD_MASK;
		linux_serial_hack.flags |= (cbaud_value >> 16) & ASYNC_SPD_MASK;
		ioctl(shm->Liofd, TIOCSSERIAL, (char *)&linux_serial_hack);
		cbaud_value &= CBAUD;
	}
#endif /* LINUX_ASYNC_HACK */

	ecusetspeed(Ltermio, cbaud_value);

	if (cbaud_value < B300)
		Ltermio->c_cflag |= CSTOPB;
	else
		Ltermio->c_cflag &= ~CSTOPB;

	/*
	 * for more info on the following monkey pus, refer to the definition
	 * of valid_baud() somewhere in here
	 */
	if (ioctl_flag)
		ecusetattr(shm->Liofd, TCSETA, Ltermio);
	return (0);

}							 /* end of lset_baud */

/*+-------------------------------------------------------------------------
	display_hw_flow_config() - display hardware flow control configuration
--------------------------------------------------------------------------*/
#if defined(HW_FLOW_CONTROL) /* see ecu.h */
void
display_hw_flow_config()
{
#if defined(CFG_TelnetOption)
	if (shm->Ltelnet)
	{
		pprintf("no flow control for telnet\n");
		return;
	}
#endif /* defined(CFG_TelnetOption) */

#undef ____HANDLED
#ifdef RTSFLOW				 /* SCO */
#define ____HANDLED
	pprintf("RTSFLOW %s CTSFLOW %s",
		(Ltermio->c_cflag & RTSFLOW) ? "on" : "off",
		(Ltermio->c_cflag & CTSFLOW) ? "on" : "off");
#ifdef CRTSFL
	pprintf(" CRTSFL %s",
		(Ltermio->c_cflag & CRTSFL) ? "on" : "off");
#endif /* CRTSFL */
	pprintf("\n");
#endif /* RTSFLOW */

#ifdef RTSXOFF				 /* SVR4 */
#define ____HANDLED
	pprintf("RTSXOFF %s CTSXON %s\n",
		(hx_flag & RTSXOFF) ? "on" : "off",
		(hx_flag & CTSXON) ? "on" : "off");
#endif /* RTSXOFF */

#if defined(CRTSCTS)		 /* sun */
#define ____HANDLED
	pprintf(" CRTSCTS %s\n",
		(Ltermio->c_cflag & CRTSCTS) ? "on" : "off");
#endif /* sun */

#ifndef ____HANDLED
	porting_attention_needed_here;	/* HW_FLOW_CONTROL but no recognized
									 * flags */
/*
 * if you are reading this because of a compilation error, you may wish to
 * go ahead and grep for 'RTSFLOW' and 'display_hw_flow_control' to find other
 * hardware control dependencies (like in lRTSCTS_control() below).  This is
 * the only rigrous test in ECU for making sure that if HW_FLOW_CONTROL is on
 * we know what to do about it.
 */
#endif /* ____HANDLED */

	pprintf("Flow control setting: %d\n", shm->Lrtscts_val);
}							 /* end of display_hw_flow_config */
#endif /* HW_FLOW_CONTROL */

/*+-------------------------------------------------------------------------
	lRTSCTS_control(flag)
--------------------------------------------------------------------------*/
void
lRTSCTS_control(flag)
int flag;
{
#ifdef RTSXOFF				 /* SVR4 */
	struct termiox flowctrl;

#endif

	if (shm->Liofd < 0)
		return;

	if (shm->Ltelnet)
		return;

#ifdef RTSXOFF				 /* SVR4 */
	ioctl(shm->Liofd, TCGETX, &flowctrl);
	switch (flag)
	{
		case 0:
			flowctrl.x_hflag &= ~(RTSXOFF | CTSXON);
			Ltermio->c_iflag |= (IXOFF);
			break;

		case 1:
			flowctrl.x_hflag |= CTSXON;
			flowctrl.x_hflag &= ~RTSXOFF;
			Ltermio->c_iflag &= ~(IXON | IXOFF | IXANY);
			break;
		case 2:
			flowctrl.x_hflag |= RTSXOFF;
			flowctrl.x_hflag &= ~CTSXON;
			Ltermio->c_iflag &= ~(IXON | IXOFF | IXANY);
			break;
		case 3:
			flowctrl.x_hflag |= (RTSXOFF | CTSXON);
			Ltermio->c_iflag &= ~(IXON | IXOFF | IXANY);
			break;
	}
	shm->Lxonxoff = Ltermio->c_iflag & (IXON | IXOFF);
	ecusetattr(shm->Liofd, TCSETA, Ltermio);
	ecusetattr(shm->Liofd, TCSETX, &flowctrl);
	hx_flag = flowctrl.x_hflag;
#else /* !SVR4 */
#if defined(RTSFLOW)		 /* only SCO */
	switch (flag & 3)
	{
		case 0:
			Ltermio->c_iflag |= (IXOFF);
			Ltermio->c_cflag &= ~(RTSFLOW | CTSFLOW);
			break;

		case 1:
			Ltermio->c_iflag &= ~(IXON | IXOFF | IXANY);
			Ltermio->c_cflag |= CTSFLOW;
			Ltermio->c_cflag &= ~RTSFLOW;
			break;

		case 2:
			Ltermio->c_iflag &= ~(IXON | IXOFF | IXANY);
			Ltermio->c_cflag |= RTSFLOW;
			Ltermio->c_cflag &= ~CTSFLOW;
			break;

		case 3:
			Ltermio->c_iflag &= ~(IXON | IXOFF | IXANY);
			Ltermio->c_cflag |= (RTSFLOW | CTSFLOW);
			break;
	}
#if defined(CRTSFL)
	if (flag & 4)
	{
		Ltermio->c_iflag &= ~(IXON | IXOFF | IXANY);
		Ltermio->c_cflag &= ~(RTSFLOW | CTSFLOW);
		Ltermio->c_cflag |= CRTSFL;
	}
	else
	{
		Ltermio->c_cflag &= ~CRTSFL;
	}
#endif
	shm->Lxonxoff = Ltermio->c_iflag & (IXON | IXOFF);
	ecusetattr(shm->Liofd, TCSETA, Ltermio);

#else
#if defined(CRTSCTS)		 /* sun */

	/*
	 * as late as Linux 1.1.59, * CRTSCTS was #defined 020000000000 which
	 * exceeds 32 bits; so do not expect it to work; do expect "warning:
	 * integer overflow in expression"
	 */

	switch (flag)
	{
		case 0:
			Ltermio->c_iflag |= (IXOFF);
			Ltermio->c_cflag &= -1 - CRTSCTS;
			break;

		default:
			Ltermio->c_iflag &= ~(IXON | IXOFF | IXANY);
			Ltermio->c_cflag |= CRTSCTS;
			break;

	}
	shm->Lxonxoff = Ltermio->c_iflag & (IXON | IXOFF);
	ecusetattr(shm->Liofd, TCSETA, Ltermio);

#endif /* sun */
#endif /* RTSFLOW */
#endif /* SVR4 */
}							 /* end of lRTSCTS_control */

/*+-------------------------------------------------------------------------
	lnew_bitrate(new_bitrate) - set new bit rate if valid
--------------------------------------------------------------------------*/
int
lnew_bitrate(new_bitrate)
UINT new_bitrate;
{
#if defined(CFG_TelnetOption)
	if (shm->Ltelnet)
	{
		if (!new_bitrate)	 /* disallow zero !!! */
			return (1);
		shm->Lbitrate = new_bitrate;
		return (0);
	}
#endif /* defined(CFG_TelnetOption) */

	if (valid_baud(new_bitrate) == -1)
		return (-1);
	if (shm->Lbitrate != new_bitrate)
		shm->Lmodem_already_init = 0;
	shm->Lbitrate = new_bitrate;
	lset_baud(1);
	return (0);
}							 /* end of lnew_bitrate */

/*+-----------------------------------------------------------------------
	lset_parity(ioctl_flag)

  If 'ioctl_flag' is set, then perform ioctl call
  is executed after setting parity
------------------------------------------------------------------------*/
void
lset_parity(ioctl_flag)
int ioctl_flag;
{
	if (shm->Liofd < 0)
		return;

	if (shm->Ltelnet)
		return;

	Ltermio->c_cflag &= ~(CS8 | PARENB | PARODD);
	switch (to_lower(shm->Lparity))
	{
		case 'e':
			Ltermio->c_cflag |= CS7 | PARENB;
			Ltermio->c_iflag |= ISTRIP;
			break;
		case 'o':
			Ltermio->c_cflag |= CS7 | PARENB | PARODD;
			Ltermio->c_iflag |= ISTRIP;
			break;
		default:
			ff(se, "invalid parity: '%c' ... defaulting to no parity\r\n",
				to_lower(shm->Lparity));
		case 'n':
			shm->Lparity = 0;
		case 0:
			Ltermio->c_cflag |= CS8;
			Ltermio->c_iflag &= ~(ISTRIP);
			shm->Lparity = 0;
			break;
	}

	if (ioctl_flag)
		ecusetattr(shm->Liofd, TCSETA, Ltermio);

}							 /* end of lset_parity */

/*+-------------------------------------------------------------------------
	lclear_xmtr_xoff()
--------------------------------------------------------------------------*/
void
lclear_xmtr_xoff()
{
	if (shm->Ltelnet)
		return;

	ecuflow(shm->Liofd, TCOON);
}							 /* end of lclear_xmtr_xoff */

/*+-------------------------------------------------------------------------
	lbreak()
--------------------------------------------------------------------------*/
void
lbreak()
{
	if (shm->Ltelnet)
		return;

	ecubreak(shm->Liofd);
}							 /* end of lbreak */

/*+-------------------------------------------------------------------------
	lopen_failed(sig) - see lopen() below
--------------------------------------------------------------------------*/
void
lopen_failed(sig)
int sig;
{
	if (sig != SIGALRM)
		ff(se, "error %d in lopen_failed: tell wht@n4hgf\r\n", sig);
	longjmp(_insurance_jmpbuf, 1);

}							 /* end of lopen_failed */

/*+----------------------------------------------------------------------
	lopen() - open a line using shm globals for arguments

returns negative LINST_ codes if failure else positive pid using line
else 0 if successful open
------------------------------------------------------------------------*/
enum linst
lopen()
{
	int itmp;
	int dummy;
	int tries;
	enum linst linst;
	struct stat line_stat;
	SIGPTR sighup_handler;

#ifdef SHARE_DEBUG
	vlogevent(getpid(), "LOPEN1 line=%s line_lock_status=%s",
		shm->Lline, LINST_text(line_lock_status(shm->Lline)));
#endif

	lopen_error_reset();	 /* reset any lingering lopen_err_str contents */

	shm->Ltelnet = 0;

	errno2 = 0;
	/*
	 * system independent checks
	 */
	if (shm->Liofd >= 0)
		return (LINST_ALREADY);
	if (!strcmp(shm->Lline, "/dev/tty"))
		return (LINST_INVALID);
	if (stat(shm->Lline, &line_stat) < 0)
	{
		if (errno == ENOENT)
			return (LINST_NODEV);
		return (LINST_OPNFAIL);
	}
	if ((line_stat.st_mode & S_IFMT) != S_IFCHR)
		return (LINST_NOTCHR);
	if (ulindex(shm->Lline, "pty") > -1)
		return (LINST_NOPTY);

	/*
	 * lock the tty
	 */
	if ((linst = lock_tty(shm->Lline)) && (linst != LINST_WEGOTIT))
		return (linst);

	/*
	 * if appropriate, make sure we have ungetty'd the line
	 */
#if defined(CFG_UseUngetty)
	if (linst != LINST_WEGOTIT)
	{
		ungetty_return_all_but(shm->Lline);
		if (!in_ungetty_list(shm->Lline))
		{
			if (linst = ungetty_get_line(shm->Lline))
			{
				sprintf(lopen_err_str, "ecuungetty: %s", LINST_text(linst));
				unlock_tty(shm->Lline);
				return (linst);
			}
		}
	}
#endif

	/*
	 * disable SIGHUP for now
	 */
	sighup_handler = signal(SIGHUP, SIG_IGN);

	/*
	 * rarely an open will hang despite our wisdom and prayer
	 */
	if (setjmp(_insurance_jmpbuf))
	{
		alarm(0);
		signal(SIGALRM, SIG_IGN);
		errno = EIO;
		sprintf(lopen_err_str, "open error: %s", strerror(errno));
		unlock_tty(shm->Lline);
		signal(SIGHUP, sighup_handler);
		return (LINST_OPNFAIL);
	}

	/*
	 * open the tty using non-blocking I/O to bypass DCD wait handle
	 * EAGAIN for SVR4 per kortcs!tim
	 */

	for (tries = 0;; ++tries)
	{
		signal(SIGALRM, lopen_failed);
#ifdef sun
		alarm(10);
#else
		alarm(5);
#endif
		errno = 0;
		if (((shm->Liofd = open(shm->Lline, O_RDWR | O_NDELAY, 0666)) < 0) &&
			(errno == EACCES) && (setuid_uucp))
		{
			errno2 = EACCES;
			Setuid(uid_uucp);
			shm->Liofd = open(shm->Lline, O_RDWR | O_NDELAY, 0666);
			Setuid(uid);
		}
		alarm(0);
		signal(SIGALRM, SIG_IGN);
		if (shm->Liofd >= 0)
			break;
		if ((tries < 5) && (errno == EAGAIN))
		{
			(void)signal(SIGALRM, SIG_DFL);
			alarm(0);
			sleep(2);
			continue;
		}
		if ((errno == EACCES) || (errno2 == EACCES))
		{
			struct passwd *pw = getpwuid(line_stat.st_uid);

			endpwent();
			if (pw)
			{
				sprintf(lopen_err_str,
					"cannot open line owned by %s (mode=%3o)",
					pw->pw_name, line_stat.st_mode & 0777);
			}
			else
			{
				sprintf(lopen_err_str,
					"open error - try chmod +rw %s", shm->Lline);
			}
		}
		else
			sprintf(lopen_err_str, "open error: %s", strerror(errno));
	  OPNFAIL:
		unlock_tty(shm->Lline);
		signal(SIGHUP, sighup_handler);
		return (LINST_OPNFAIL);
	}

	if (!isatty(shm->Liofd))
	{
		close(shm->Liofd);
		unlock_tty(shm->Lline);
		signal(SIGHUP, sighup_handler);
		return (LINST_NOTCHR);
	}

	/*
	 * turn off non-blocking I/O and set initial termio, including CLOCAL
	 * (MUST have CLOCAL on some systems)
	 */
	if ((itmp = fcntl(shm->Liofd, F_GETFL, &dummy)) < 0)
	{
		sprintf(lopen_err_str, "fcntl() F_GETFL failed: %s", strerror(errno));
		close(shm->Liofd);
		unlock_tty(shm->Lline);
		signal(SIGHUP, sighup_handler);
		return (LINST_OPNFAIL);
	}

#ifndef O_NONBLOCK
#define O_NONBLOCK 0
#endif

	itmp &= ~(O_NDELAY | O_NONBLOCK);
	if (fcntl(shm->Liofd, F_SETFL, itmp) < 0)
	{
		sprintf(lopen_err_str, "fcntl() F_SETFL failed: %s", strerror(errno));
		close(shm->Liofd);
		goto OPNFAIL;
	}

	ecugetattr(shm->Liofd, Ltermio);
	Ltermio->c_iflag = (IGNPAR | IGNBRK | shm->Lxonxoff);
	Ltermio->c_oflag = 0;
	Ltermio->c_cflag |= (CLOCAL | CREAD | HUPCL);
	Ltermio->c_lflag = 0;
	Ltermio->c_cc[VMIN] = 1;
	Ltermio->c_cc[VTIME] = 1;
	lset_baud(0);			 /* do not perform ioctl */
	lset_parity(0);			 /* do not perform ioctl */
	lRTSCTS_control(shm->Lrtscts_val);	/* set hw flow control per
										 * variable */

	/*
	 * restore SIGHUP handler
	 */
	signal(SIGHUP, sighup_handler);

#if defined(SVR4)
	hx_flag = 0;			 /* hardware flow control "memory" */
#endif

#ifdef SHARE_DEBUG
	vlogevent(getpid(),
		"LOPEN2 line=%s open on fd=%d", shm->Lline, shm->Liofd);
#endif

	lopen_error_reset();
	return (LINST_OK);

}							 /* end of lopen */

/*+-------------------------------------------------------------------------
	lclose_failed(sig) - see lclose() below
--------------------------------------------------------------------------*/
void
lclose_failed(sig)
int sig;
{
	if (sig != SIGALRM)
		ff(se, "error %d in lclose_failed: tell wht@n4hgf\r\n", sig);
	longjmp(_insurance_jmpbuf, 1);

}							 /* end of lclose_failed */

/*+-----------------------------------------------------------------------
	lclose() - close the line

The FAS driver and others hang on a close until all output for a line
has drained.  Sometimes during a hangup, a noisy XOFF can be received.
Other changeces for failure include a DCE which drops CTS and leaves
it off, locking the line up if there is output waiting to go out.
To make sure the line is actually closed in these situations, a SIGLARM
handler is used.
------------------------------------------------------------------------*/
void
lclose()
{
	struct TERMIO ttio;
	SIGPTR sighup_handler;

	attempt2 = 0;

#ifdef SHARE_DEBUG
	vlogevent(getpid(), "LCLOSE fd=%d tty=%s lock status=%s",
		shm->Liofd, shm->Lline,
		shm->Ltelnet ? "N/A" : LINST_text(line_lock_status(shm->Lline)));
#endif

	if (shm->Liofd < 0)
		return;

	/*
	 * telnet intercept
	 */
#if defined(CFG_TelnetOption)
	if (shm->Ltelnet)
	{
		close(shm->Liofd);
		shm->Liofd = -1;
		shm->Ltelnet = 0;
		return;
	}
#endif /* defined(CFG_TelnetOption) */

	/*
	 * disable SIGHUP for now
	 */
	sighup_handler = signal(SIGHUP, SIG_IGN);

	/*
	 * endless loop because we cannot get out anyway unless success
	 */
	attempt2 = 0;
  ATTEMPT:
	signal(SIGALRM, lclose_failed);
#ifdef sun
	alarm(10);
#else
	alarm(5);
#endif
	if (setjmp(_insurance_jmpbuf))
	{						 /* close failed */
		signal(SIGALRM, SIG_IGN);
		alarm(0);
		ff(se, "\r\nclose failed (remote XOFF?) ... retrying close\r\n");
		lclear_xmtr_xoff();
		ttio = *Ltermio;
		ttio.c_iflag &= ~(IXON | IXOFF);
		ttio.c_cflag &= (CSIZE | CSTOPB | CREAD | PARENB | PARODD);
		ecusetattr(shm->Liofd, TCSETA, &ttio);
		lflush(2);
		attempt2 = 1;
		goto ATTEMPT;
	}
	if (!attempt2)
	{
		lclear_xmtr_xoff();
		ldraino(1);
	}
	lCLOCAL(1);
	close(shm->Liofd);
	shm->Liofd = -1;
	signal(SIGALRM, SIG_IGN);
	alarm(0);
	unlock_tty(shm->Lline);
	shm->Lmodem_already_init = 0;
	shm->Lconnected = 0;

	/*
	 * restore SIGHUP handler
	 */
	signal(SIGHUP, sighup_handler);

}							 /* end of lclose */

/*+-------------------------------------------------------------------------
	lflash_dtr() - flash DTR

DTR is lowered for 500 msec and raised again.  After raising,
we pause a while for a possibly slow DCE to rereap it's fecal material.
This not long enough for some DCE, but -hey- this isn't an X11 program
with a 50Kb app-defaults file.

expects:  Ltermio - current termio status of line
          shm->Liofd - current line fd
          shm->Lline - /dev/ttyxx name of line

On SVR4, an open/close of the line is required to get DTR back
up. SVR3 does not seem to need this (ISC asy, SCO sio, Uwe Doering's FAS)
but we do it anyway

On SunOS, we use an old mechanism, but one that is very effective
and painless
--------------------------------------------------------------------------*/
void
lflash_dtr()
{
#if defined(ESIXSVR4)		 /* thanx for fix to yzrnur!rene (Rene Lampe) */
	int z = TIOCM_DTR | TIOCM_RTS;

	if (shm->Ltelnet)
		return;

	ioctl(shm->Liofd, TIOCMBIC, &z);	/* Clear DTR */
	Nap(500L);				 /* time for DTR to be low */
	ioctl(shm->Liofd, TIOCMBIS, &z);	/* Set DTR */
	Nap(300L);				 /* nap to give a lazy DCE some time */
#else
#if defined(TIOCCDTR) && defined(TIOCSDTR)	/* sun */
	if  (shm->Ltelnet)
		    return;

	ioctl(shm->Liofd, TIOCCDTR, 0);	/* drop DTR */
#ifndef sun
	Nap(500L);				 /* sun takes forEVER */
#endif
	ioctl(shm->Liofd, TIOCSDTR, 0);	/* raise DTR */
#ifndef sun
	Nap(100L);
#endif
#else /* not BSD-style */
#undef NEED_REOPEN
#if defined(SVR4)
#define NEED_REOPEN
	int tempfd;

#endif
	struct TERMIO b0t;

	if (shm->Ltelnet)
		return;

	/*
	 * copy termio but CBAUD to B0
	 */
	b0t = *Ltermio;
	b0t.c_cflag &= ~CBAUD;	 /* B0 */

	/*
	 * drop DTR for a while
	 */
	ecusetattr(shm->Liofd, TCSETA, &b0t);	/* drop DTR */

	/*
	 * DTR will not come back on some systems without open/close line
	 */
#ifdef NEED_REOPEN

	if (((tempfd = open(shm->Lline, O_NDELAY | O_RDWR, 0666)) < 0) &&
		(errno == EACCES) && (setuid_uucp))
	{
		Setuid(uid_uucp);
		tempfd = open(shm->Lline, O_NDELAY | O_RDWR, 0666);
		Setuid(uid);
	}
	if (tempfd != -1)
		close(tempfd);
	else
	{
		int save_errno = errno;
		char s128[128];

		sprintf(s128, "FLASH DTR line reopen failed (%.90s)",
			strerror(errno));
		logevent(shm->xmtr_pid, s128);
		pputs(s128);
		pputs("\n");
		errno = save_errno;
		termecu(TERMECU_LINE_OPEN_ERROR);
	}
#else

	/*
	 * ensure DTR low for 500 msec (the tempfd open/close takes plenty
	 * long enough)
	 */
	Nap(500L);
#endif

	/*
	 * reestablish bit rate (raise DTR if the open/close line did not do
	 * it)
	 */
	ecusetattr(shm->Liofd, TCSETA, Ltermio);	/* raise DTR */
	Nap(300L);				 /* nap to give a lazy DCE some time */

#undef NEED_REOPEN

#endif /* defined(TIOCCDTR) && defined(TIOCSDTR) */
#endif /* defined(ESIXSVR4) */

}							 /* end of lflash_dtr */

/*+-------------------------------------------------------------------------
	lxon_xoff(flag)
IXON specifies whether or not we respond to xon/xoff characters
IXOFF specifies whether or not we generate XON/XOFF characters
--------------------------------------------------------------------------*/
void
lxon_xoff(flag)
int flag;
{
	if (shm->Ltelnet)
		return;

	if (flag & IXON)
		Ltermio->c_iflag |= IXON;
	else
		Ltermio->c_iflag &= ~IXON;

	if (flag & IXOFF)
		Ltermio->c_iflag |= IXOFF;
	else
		Ltermio->c_iflag &= ~IXOFF;
	shm->Lxonxoff = Ltermio->c_iflag & (IXON | IXOFF);
	ecusetattr(shm->Liofd, TCSETA, Ltermio);

}							 /* end of lflash_dtr */

/*+-------------------------------------------------------------------------
	lget_xon_xoff(ixon,ixoff)
--------------------------------------------------------------------------*/
void
lget_xon_xoff(ixon, ixoff)
int *ixon;
int *ixoff;
{
#if defined(CFG_TelnetOption)
	if (shm->Ltelnet)
	{
		*ixon = 0;
		*ixoff = 0;
		return;
	}
#endif /* defined(CFG_TelnetOption) */

	*ixon = Ltermio->c_iflag & IXON;
	*ixoff = Ltermio->c_iflag & IXOFF;
}							 /* end of lget_xon_xoff */

/*+-------------------------------------------------------------------------
	set_xon_xoff_by_arg(arg)
--------------------------------------------------------------------------*/
int
set_xon_xoff_by_arg(arg)
char *arg;
{
	if (ulcmpb(arg, "on") < 0)
		shm->Lxonxoff = IXON | IXOFF;
	else if (ulcmpb(arg, "off") < 0)
		shm->Lxonxoff = 0;
	else if (ulcmpb(arg, "out") < 0)
		shm->Lxonxoff = IXON;
	else if (ulcmpb(arg, "in") < 0)
		shm->Lxonxoff = IXOFF;
	else
		return (-1);

	if (shm->Ltelnet)
		return (0);

	if (shm->Liofd < 0)
		return (0);

	Ltermio->c_iflag &= ~(IXON | IXOFF);
	Ltermio->c_iflag |= shm->Lxonxoff;
	ecusetattr(shm->Liofd, TCSETA, Ltermio);
	return (0);

}							 /* end of set_xon_xoff_by_arg */

/*+-------------------------------------------------------------------------
	xon_status()
--------------------------------------------------------------------------*/
char *
xon_status()
{
#if defined(CFG_TelnetOption)
	if (shm->Ltelnet)
		return ("telnet");
#endif /* defined(CFG_TelnetOption) */

	switch (shm->Lxonxoff)
	{
		case 0:
			return ("off");
		case IXON:
			return ("in off, out on");
		case IXOFF:
			return ("in on, out off");
		case IXON | IXOFF:
			return ("on");
	}
	return ("logic error");
}							 /* end of xon_status */

/*+-------------------------------------------------------------------------
	lCLOCAL(flag) - set line CLOCAL state

flag == 0: turn off CLOCAL to catch DCD loss
     == 1: turn on CLOCAL to ignore modem signals

does not let CLOCAL be turned off if not Lconnected
also resets global zero_length_read_detected
--------------------------------------------------------------------------*/
void
lCLOCAL(flag)
int flag;
{

	if (shm->Ltelnet)
		return;
	if (flag)
		Ltermio->c_cflag |= CLOCAL;
	else if (shm->Lconnected)
		Ltermio->c_cflag &= ~CLOCAL;
	else
		Ltermio->c_cflag |= CLOCAL;

	zero_length_read_detected = 0;
	lreset_ksr();			 /* do the ioctl */

#ifdef CLOCAL_DEBUG
	{
		char s128[128];

		sprintf(s128, "lCLOCAL(%d) connected=%c CLOCAL set %o",
			flag, shm->Lconnected ? 'y' : 'n', Ltermio->c_cflag & CLOCAL ? 1 : 0);
		logevent((int)xmtr_pid, s128);
		pprintf("%s\n", s128);
	}
#endif

}							 /* end of lCLOCAL */

/*+-------------------------------------------------------------------------
	ldcdwatch(flag) - set DCD watcher state
--------------------------------------------------------------------------*/
void
ldcdwatch(flag)
int flag;
{
	shm->Ldcdwatch = flag;
	dcdwatch_set = 1;
	lCLOCAL(!flag);
}							 /* end of ldcdwatch */

/*+-------------------------------------------------------------------------
	ldcdwatch_str(flagstr) - string version of ldcdwatch

return 0 if successful or -1 if bad flagstr
--------------------------------------------------------------------------*/
int
ldcdwatch_str(flagstr)
char *flagstr;
{
	static STR_CLASSIFY sc[] =
	{
		{"1", 1, DCDW_ON},
		{"yes", 1, DCDW_ON},
		{"on", 2, DCDW_ON},
		{"0", 1, DCDW_ON},
		{"no", 1, DCDW_OFF},
		{"off", 3, DCDW_OFF},
		{"terminate", 1, DCDW_TERMINATE},
		{(char *)0, 0, -1}
	};
	int token;

	if ((token = str_classify(sc, flagstr)) < 0)
		return (-1);

	ldcdwatch(token);
	return (0);

}							 /* end of ldcdwatch_str */

/*+-------------------------------------------------------------------------
	lzero_length_read_detected() - read from line returned zero length

This must mean CLOCAL was off and DCD is/went low.  We do different things
depending in the xmtr and rcvr process

If we return, the condition has ben handled and reads may be retried
safely or other appropriate operations performed; otherwise ECU is
terminated.
--------------------------------------------------------------------------*/
void
lzero_length_read_detected()
{

	zero_length_read_detected = 1;
	if (getpid() == xmtr_pid)/* if we are in the transmitter */
	{
#ifdef CLOCAL_DEBUG
		logevent((int)xmtr_pid, "lzero xmtr");
		pprintf("lzero xmtr\n");
#endif
		if (!shm->Lconnected)
			lCLOCAL(1);
		else
		{
			extern UINT32 colors_current;
			UINT32 colors_at_entry = colors_current;

			fputs("\r\n", se);
			setcolor(colors_notify);
			fputs("[connection terminated]", se);
			setcolor(colors_at_entry);
			fputs("\r\n", se);
			DCE_now_on_hook();	/* does a lCLOCAL(1); */
		}
		Nap(1000L);
		lflush(2);

		if (shm->Ldcdwatch == DCDW_TERMINATE)
			termecu(TERMECU_OK);
		shmx_unpause_rcvr();
	}
	else
	{

		/*
		 * we are in the receiver
		 */

#ifdef CLOCAL_DEBUG
		logevent((int)xmtr_pid, "lzero rcvr");
		pprintf("lzero rcvr\n");
#endif

		/*
		 * make line "safe" to read from immediately; however, if CLOCAL
		 * was set and we get a zero length read, we are in some kind of
		 * unknown trouble
		 */
		if (Ltermio->c_cflag & CLOCAL)	/* zero len read with CLOCAL? */
		{					 /* then die ECU */
#ifdef linux
			termecu(TERMECU_OK);	/* exit gracefully */
#else
			errno = EIO;
			termecu(TERMECU_LINE_READ_ERROR);
#endif
		}
		lCLOCAL(1);
		shmr_notify_xmtr_of_DCD_loss();
		pause();			 /* wait for unpause */
	}
}							 /* end of lzero_length_read_detected */
