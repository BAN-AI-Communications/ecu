/*+-----------------------------------------------------------------------
	eculine.c -- ECU link-independent line handler
	wht@wht.net

  Defined functions:
	lgetc_timeout(msec)
	lgetc_xmtr()
	lgetc_xmtr_raw()
	lgets_timeout(lrwt)
	llookfor(lookfor, msecs, echo_flag)
	lputc(lchar)
	lputc_paced(pace_msec, lchar)
	lputs(string)
	lputs_paced(pace_msec, string)
	lquiet(msecs, echo_flag)
	lrdchk_xmtr()
	process_xmtr_rcvd_char(rchar, echo)

  Men go abroad to wonder at the height of mountains, at the huge
  waves of the sea, at the long courses of the ocean, at the
  circular motion of the stars; and they pass by themselves
  without wondering.  -- Saint Augustine

------------------------------------------------------------------------*/
/*+:EDITS:*/
/*:04-26-2000-11:15-wht@bob-RELEASE 4.42 */
/*:01-02-2000-14:07-wht@menlo-add continue on line error */
/*:02-26-1998-03:11-wht@kepler-linux2/redhat */
/*:01-24-1997-02:37-wht@yuriatin-SOURCE RELEASE 4.00 */
/*:09-11-1996-20:00-wht@yuriatin-3.48-major telnet,curses,structural overhaul */
/*:08-20-1996-12:39-wht@kepler-locale/ctype fixes from ache@nagual.ru */
/*:11-23-1995-11:20-wht@kepler-source control 3.37 for tsx-11 */
/*:11-14-1995-10:23-wht@kepler-3.37.80-source control point: SOCKETS */
/*:11-04-1995-21:02-wht@kepler-add telnet_cmd handling to lgetc_xmtr */
/*:10-18-1995-04:29-wht@kepler-always use select for nap */
/*:10-18-1995-04:16-wht@kepler-break out serial dep into ldserial.c */
/*:10-14-1995-17:08-wht@kepler-'struct termio' to 'struct TERMIO' */
/*:10-14-1995-15:02-wht@kepler-remove Ioctl debug hack */
/*:09-17-1995-16:31-wht@kepler-zero length read under LINUX graceful term */
/*:08-27-1995-06:37-wht@n4hgf-handle hw flow control per shm->Lrtscts_val */
/*:06-12-1995-15:03-wht@n4hgf-if ecu has uucp euid, make use of it */
/*:05-11-1995-15:55-wht@n4hgf-ck_sigint fools optimizing compilers */
/*:03-29-1995-01:09-wht@n4hgf-use ache internationalization code for all */
/*:03-29-1995-01:06-wht@n4hgf-add SIMPLIFY code */
/*:01-12-1995-15:19-wht@n4hgf-apply Andrew Chernov 8-bit clean+FreeBSD patch */
/*:06-19-1994-11:52-wht@gyro-fix lflash_dtr for ESIXSVR4 */
/*:06-03-1994-17:08-wht@kepler-turn off CRTSFL if val < 4 */
/*:05-04-1994-04:38-wht@n4hgf-ECU release 3.30 */
/*:10-27-1993-14:58-wht@n4hgf-bizarre fcntl bug finally fixed */
/*:08-01-1993-02:12-wht@n4hgf-use got_delim in lgets_timeout */
/*:07-20-1993-17:34-wht@n4hgf-set lgets_timeout timeout_counter = qc2 on NUL */
/*:06-12-1993-12:18-wht@n4hgf-move LINST_text to ecuLCK for ecuungetty */
/*:05-29-1993-20:21-wht@n4hgf-change linst_err_text to LINST_text */
/*:05-29-1993-20:18-wht@n4hgf-change ugstat_text to UG_text */
/*:03-01-1993-12:38-wht@n4hgf-defeat make depend on sys/ttold.h */
/*:01-30-1993-12:27-wht@n4hgf-use TIO[CS]DTR on sun to flash DTR */
/*:09-10-1992-13:58-wht@n4hgf-ECU release 3.20 */
/*:08-22-1992-15:38-wht@n4hgf-ECU release 3.20 BETA */
/*:08-21-1992-13:39-wht@n4hgf-rewire direct/modem device use */
/*:08-19-1992-14:03-wht@n4hgf-2nd open in lflash_dtr needs O_NDELAY on SVR4 */
/*:08-16-1992-02:52-wht@n4hgf-some vendors use SCO naming but ttyaa/ttyaA */
/*:07-27-1992-05:49-wht@n4hgf-lopen SCO modem line to make CLOCAL effective */
/*:07-24-1992-14:25-wht@n4hgf-more valiant efforts on lclose failure */
/*:07-19-1992-09:19-wht@n4hgf-lopen validation for char special */
/*:05-11-1992-16:35-wht@gyro-speed up lflash_DTR on sun */
/*:05-04-1992-04:43-kortcs!tim-fix EAGAIN on line open with SVR4 */
/*:04-27-1992-19:57-wht@n4hgf-add LINST_ECUUNGETTY error text */
/*:04-24-1992-21:59-wht@n4hgf-more SCO tty name normalizing */
/*:04-24-1992-21:44-wht@n4hgf-add SCO_direct_tty */
/*:04-12-1992-06:31-wht@gyro-was not canceling alarm on lopen error */
/*:03-29-1992-16:27-wht@n4hgf-put three second timer on lopen */
/*:03-17-1992-18:26-wht@n4hgf-optimize parameter 1 to select() */
/*:12-12-1991-05:14-wht@n4hgf-lgetc_timeout can now return a null character */
/*:11-26-1991-19:47-wht@n4hgf-add ldcdwatch_str */
/*:11-11-1991-22:28-wht@n4hgf-ldcdwatch and lCLOCAL code */
/*:11-11-1991-14:38-wht@n4hgf-lzero_length_read_detected code */
/*:09-01-1991-14:18-wht@n4hgf2-on sun, use termios and improve ldraino */
/*:09-01-1991-02:51-wht@n4hgf2-sun CRTSCTS turn on bug fixed */
/*:08-25-1991-14:39-wht@n4hgf-SVR4 port thanks to aega84!lh */
/*:08-11-1991-18:06-wht@n4hgf-SCO_TTY_NAMING considerations */
/*:08-06-1991-14:18-wht@n4hgf-bit rates below 300 get two stop bits */
/*:07-29-1991-01:55-wht@n4hgf-remove unused externs */
/*:07-25-1991-12:56-wht@n4hgf-ECU release 3.10 */
/*:07-17-1991-07:04-wht@n4hgf-avoid SCO UNIX nap bug */
/*:04-09-1991-16:11-wht@n4hgf-use B0 in lflash_DTR */
/*:02-07-1991-01:00-wht@n4hgf-fix code in for lclose retry on remote XOFF */
/*:01-29-1991-14:54-wht@n4hgf-put code in for lclose retry on remote XOFF */
/*:01-25-1991-05:57-wht@n4hgf-cringe - lflush was flushing console not line */
/*:01-09-1991-22:31-wht@n4hgf-ISC port */
/*:01-09-1991-21:26-wht@n4hgf-don't prototype nap() (ISC port) */
/*:08-14-1990-20:40-wht@n4hgf-ecu3.00-flush old edit history */

#define DECLARE_LINEVARS_PUBLIC
#include "ecu.h"
#include "ecukey.h"
#include "termecu.h"
#include <pwd.h>

#if defined(CFG_TelnetOption)
#include <arpa/telnet.h>
#endif /* defined(CFG_TelnetOption) */

#ifndef O_NONBLOCK
#define O_NONBLOCK 0
#endif

void lreset_ksr();
void lzero_length_read_detected();
void lCLOCAL();

int zero_length_read_detected = 0;


/*
 * with SCO UNIX, nap doesn't work as advertized; param MUST be > granularity
 * or nap will return immediately; not a problem with XENIX
 */
#define LPUTS_NAP_COUNT	(min((hzmsec * 2),20L))

/*+-------------------------------------------------------------------------
	process_xmtr_rcvd_char(rchar,echo) - feed xmtr-rcvd char to rcvr code

echo: 0 no echo
      1 echo literally
      2 "make printable"
--------------------------------------------------------------------------*/
void
process_xmtr_rcvd_char(rchar, echo)
UINT rchar;
int echo;
{
	if (process_rcvd_char(rchar))
		return;

	if (echo == 1)
	{
		if (rchar == NL)
			fputc(CRET, se);
		fputc(rchar, se);
		if (rchar != CRET)
			plogc(rchar);
	}
	else if (echo == 2)
	{
		pputs(graphic_char_text(rchar, 0));
		if (rchar == 0x0A)
			pputs("\n");
	}

}							 /* end of process_xmtr_rcvd_char */

/*+-------------------------------------------------------------------------
	lgetc_xmtr_raw() -- get character from line

  zero_length_read_detected is a public that will set if the
  DCD watcher is turned on and DCD is lost
--------------------------------------------------------------------------*/
UINT
lgetc_xmtr_raw()
{
	int itmp;
	uchar char_rtnd;

  READ_AGAIN:
	if ((itmp = read(shm->Liofd, (char *)&char_rtnd, 1)) < 1)
	{
		if (!itmp)
		{
			if(shm->Lsockserve)
			{
				close(shm->Liofd);
				shm->Lconnected = 0;
				return(0);
			}
			if (shm->Ldcdwatch)
			{
				lzero_length_read_detected();
				return (0);
			}
			errno = EIO;	 /* for termecu processing */
			termecu(TERMECU_LINE_READ_ERROR);
		}
		if (errno == EINTR)	 /* if signal interrupted, ... */
		{
			if (ck_sigint())
				return (0);
			goto READ_AGAIN;
		}
		if(shm->Lsockserve)
		{
			close(shm->Liofd);
			shm->Lconnected = 0;
			return(0);
		}
		termecu(TERMECU_LINE_READ_ERROR);
	}
	shm->rcvd_chars++;
	shm->rcvd_chars_this_connect++;

	return (char_rtnd);

}							 /* end of lgetc_xmtr_raw */

/*+-------------------------------------------------------------------------
	lgetc_xmtr() -- xmtr version of get char from line

  also called by rcvr code when lgetc_buf empty and vmin == 1

  zero_length_read_detected is a public that will set if the
  DCD watcher is turned on and DCD is lost

  if telnet session, intercept and process IAC
--------------------------------------------------------------------------*/
UINT
lgetc_xmtr()
{
	UINT char_rtnd = lgetc_xmtr_raw();

#ifdef CFG_TelnetOption
	if (shm->Ltelnet && !shm->Ltelnet_raw && (char_rtnd == IAC))
	{
		telnet_cmd(0);
		char_rtnd = 0;
	}
#endif

	if (shm->Lparity)
		char_rtnd &= 0x7F;
	return (char_rtnd);

}							 /* end of lgetc_xmtr */

/*+-------------------------------------------------------------------------
	lrdchk_xmtr() -- Rdchk(shm->Liofd) for xmtr
--------------------------------------------------------------------------*/
int
lrdchk_xmtr()
{
	return (Rdchk(shm->Liofd));
}							 /* end of lrdchk_xmtr */

/*+-------------------------------------------------------------------------
	char *lgets_timeout(LRWT *) - may be called by xmtr only

to1 and to2 are unsigned long values in milliseconds (not
currently supported well under BSD4); to1 is the time to wait
for the first character, to2 the time to wait for subsequent
characters.

if raw_flag 0,     non-printables are stripped from beginning
                   and end of received characters (i.e., modem
                   response reads); NULs discarded, parity stripped
if raw_flag 1,     full raw read buffer returned

0x80 in raw_flag indicates console interrupts should be enabled.
if interrupt thus detected, the procedure returns "!Interrupted"
without reseting variable 'interrupt'

buffer is address to read chars into

bufsize is buffer max size (allowing room for terminating null)
which should be at least 2 if raw_size includes 0x80 bit,
else at least 12 characters if 0x80 omitted.

count is a int which, at return, receives the actual count read

zero_length_read_detected is a public that will set if the
DCD watcher is turned on and DCD is lost

--------------------------------------------------------------------------*/
char *
lgets_timeout(lrwt)
LRWT *lrwt;
{
	int actual_count = 0;
	char *cp = lrwt->buffer;
	int max_count = lrwt->bufsize;
	int raw_mode = lrwt->raw_flag & 0x0F;
	int echo_flag = lrwt->echo_flag;
	int check_sigint = (lrwt->raw_flag & 0x80);
	int old_ttymode = get_ttymode();	/* save original tty mode */
	int delim_len;
	struct timeval tval;
	char *rtn_val;

#ifdef CFG_HasFdSet
	CFG_FDSET fdset;

	FD_ZERO(&fdset);
#else
	int fdset;

#endif

	delim_len = (lrwt->delim) ? strlen(lrwt->delim) : 0;

	if ((shm->Lbitrate < 300) && lrwt->to2)
	{
		if (lrwt->to2 < 300L)
			lrwt->to2 = 300L;
		else if ((shm->Lbitrate < 1200) && lrwt->to2)
		{
			if (lrwt->to2 < 200L)
				lrwt->to2 = 100L;
		}
	}

/* perform the lrtw function

  output: lrwt->count is actual count of return result
          lrwt->buffer is return read buffer
*/
	max_count--;			 /* leave room for null */

	if (check_sigint)
		ttymode(2);			 /* let console interrupt long timeouts */

	*cp = 0;				 /* init result string */
	lrwt->got_delim = 0;	 /* no delimiter read yet */
	while (1)
	{
		if (check_sigint && ck_sigint())
			goto INTERRUPTED;

		errno = 0;
		if (actual_count)
		{
			tval.tv_sec = lrwt->to2 / 1000L;
			tval.tv_usec = (lrwt->to2 % 1000L) * 1000L;
		}
		else
		{
			tval.tv_sec = lrwt->to1 / 1000L;
			tval.tv_usec = (lrwt->to1 % 1000L) * 1000L;
		}

#ifdef CFG_HasFdSet
		FD_SET(shm->Liofd, &fdset);
#else
		fdset = 1 << shm->Liofd;	/* Liofd will always be <= 31, right? */
#endif

		errno = 0;
		if (select(shm->Liofd + 1, &fdset,
				(SelBitmask *) 0, (SelBitmask *) 0, &tval) != 1)
		{
			if (errno == EINTR)
				continue;
			break;
		}

#if 0
		while (Rdchk(shm->Liofd))
		{
#endif
			zero_length_read_detected = 0;
			*cp = lgetc_xmtr();

			if (zero_length_read_detected)
				goto BOTTOM;

			if (check_sigint && ck_sigint())
				goto INTERRUPTED;

			if (*cp == 0)
				continue;

			process_xmtr_rcvd_char(*cp, !!echo_flag);

			if (!raw_mode && (*cp == CRET))
				continue;

			*++cp = 0;
			actual_count++;

			if (--max_count == 0)
				goto BOTTOM;

			if (delim_len && (actual_count >= delim_len) &&
				!strncmp(lrwt->delim, cp - delim_len, delim_len))
			{
				lrwt->got_delim = 1;
				goto BOTTOM;
			}
#if 0
		}
		if (!lrwt->to2)
			break;
#endif
	}

/********* common post processing for select() / no select() ************/
  BOTTOM:

	if (check_sigint)
		ttymode(old_ttymode);
	if (raw_mode)
	{
		lrwt->count = actual_count;
		return (lrwt->buffer);
	}
	cp = lrwt->buffer;
	while (*cp && !isprint((uchar) * cp))
		cp++;
	rtn_val = cp;
	actual_count = 0;
	while (*cp && isprint((uchar) * cp))
	{
		cp++;
		actual_count++;
	}
	*cp = 0;
	strcpy(lrwt->buffer, rtn_val);
	lrwt->count = actual_count;
	return (lrwt->buffer);

  INTERRUPTED:
	ttymode(old_ttymode);
	strcpy(lrwt->buffer, "!Interrupted");
	lrwt->count = strlen(lrwt->buffer);
	return ((char *)0);

}							 /* end of lgets_timeout */

/*+-------------------------------------------------------------------------
	lgetc_timeout(msec) - may be called by xmtr only

 reads one character from line unless msec passes with no receipt.
 return char if received, else -1 if timeout
--------------------------------------------------------------------------*/
int
lgetc_timeout(msec)
long msec;
{
	struct timeval tval;
	UINT ch;

#ifdef CFG_HasFdSet
	CFG_FDSET fdset;

	FD_ZERO(&fdset);
	FD_SET(shm->Liofd, &fdset);
#else
	int fdset = 1 << shm->Liofd;	/* Liofd will always be <= 31, right? */
#endif

	tval.tv_sec = msec / 1000L;
	tval.tv_usec = (msec % 1000L) * 1000L;
	if (select(shm->Liofd + 1, &fdset,
			(SelBitmask *) 0, (SelBitmask *) 0, &tval) != 1)
	{
		return (-1);
	}
	if (ck_sigint())
		return (-1);

	ch = lgetc_xmtr();
	return ((int)ch);

}							 /* end of lgetc_timeout */

/*+-------------------------------------------------------------------------
	llookfor(lookfor,msecs,echo_flag)
return 1 if successful, else 0 if no match
echo_flag: 0 no echo
           1 echo literally
           2 "make printable"
--------------------------------------------------------------------------*/
int
llookfor(lookfor, msecs, echo_flag)
char *lookfor;
UINT32 msecs;
int echo_flag;
{
	int lookfor_len = strlen(lookfor);
	int lchar;
	uchar *lastfew = (uchar *)malloc(lookfor_len);
	int success_flag = 0;
	int old_ttymode = get_ttymode();

	if (!lastfew)
	{
		pputs("memory exhausted\n");
		return (0);
	}

	ttymode(2);

	memset(lastfew, 0, lookfor_len);
	while ((lchar = lgetc_timeout(msecs)) >= 0)
	{
		if (!lchar)			 /* skip nulls */
			continue;
		process_xmtr_rcvd_char(lchar, echo_flag);
		mem_cpy(lastfew, lastfew + 1, lookfor_len - 1);
		*(lastfew + lookfor_len - 1) = (uchar) lchar;
		if (!memcmp(lastfew, lookfor, lookfor_len))
		{
			success_flag = 1;
			break;
		}
	}
	free(lastfew);
	ttymode(old_ttymode);
	return (success_flag);
}							 /* end of llookfor */

/*+-------------------------------------------------------------------------
	lquiet(msecs,echo_flag)
--------------------------------------------------------------------------*/
void
lquiet(msecs, echo_flag)
UINT32 msecs;
int echo_flag;
{
	int lchar;
	int old_ttymode = get_ttymode();

	ttymode(2);
	while ((lchar = lgetc_timeout(msecs)) >= 0)
	{
		if (ck_sigint())	 /* if interrupt, return */
			break;
		if (!lchar)			 /* skip nulls */
			continue;
		process_xmtr_rcvd_char(lchar, !!echo_flag);
	}
	ttymode(old_ttymode);

}							 /* end of lquiet */

/*+-----------------------------------------------------------------------
	lputc(lchar) -- write lchar to comm line
------------------------------------------------------------------------*/
void
lputc(lchar)
char lchar;
{
	while (write(shm->Liofd, &lchar, 1) < 0)
	{
		char s80[80];

		if (errno == EINTR)
			continue;
		if(shm->Ltelnet)
		{
			close(shm->Liofd);
			shm->Lconnected = 0;
			return;
		}
		sprintf(s80, "lputc write error fd=%d", shm->Liofd);
		pperror(s80);
		termecu(TERMECU_XMTR_WRITE_ERROR);
	}
	shm->xmit_chars++;
	shm->xmit_chars_this_connect++;

}							 /* end of lputc */

/*+-----------------------------------------------------------------------
	lputc_paced(pace_msec,lchar) -- write lchar to comm line
  with time between each character
------------------------------------------------------------------------*/
void
lputc_paced(pace_msec, lchar)
int pace_msec;
char lchar;
{

	lputc(lchar);
	Nap((long)(pace_msec ? pace_msec : LPUTS_NAP_COUNT));

}							 /* end of lputc_paced */

/*+-----------------------------------------------------------------------
	lputs(string) -- write string to comm line
------------------------------------------------------------------------*/
void
lputs(string)
char *string;
{
	while (*string)
		lputc(*string++);
}

/*+-----------------------------------------------------------------------
	lputs_paced(pace_msec,string) -- write string to comm line
  with time between each character
------------------------------------------------------------------------*/
void
lputs_paced(pace_msec, string)
int pace_msec;
char *string;
{
	while (*string)
		lputc_paced(pace_msec, *string++);

}							 /* end of lputs_paced */

/* vi: set tabstop=4 shiftwidth=4: */
/* end of eculine.c */
