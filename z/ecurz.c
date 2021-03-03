char *version = "@(#)ecurz 3.38";

#define _XOPEN_SOURCE
/*+-------------------------------------------------------------------------
	ecurz.c - X/Y/ZMODEM receive program
  Derived from public domain source by Chuck Forsberg, Omen Technologies
  Adaptation for ecu 1989 wht@wht.net

  Defined functions:
	SIGALRM_handler(sig)
	arg_token(parsestr, termchars)
	bye_bye(sig)
	cancel_transaction(can_code)
	close_and_report()
	flushline()
	fname_split(cmd, arg, arg_max_quan, narg_rtn)
	fname_too_long(fname)
	fname_truncated()
	getfree()
	isanylc(str)
	main(argc, argv)
	make_dirs(pathname)
	mkdir(dpath, dmode)
	our_fopen(pathname, openmode)
	procheader(name)
	purgeline()
	readline(timeout)
	report_receive_progress(pos)
	rzfile()
	rzfiles()
	send_ZFIN()
	send_cancel(error)
	sendline(c)
	substr(str, token)
	sys2(shellcmd)
	tryz()
	uncaps(str)
	usage(fail_reason)
	wcgetsec(rxbuf, maxtime)
	wcreceive(argc, argp)
	wcrx()
	wcrxpn(rpn)
	write_sec_to_disk(buf, n)
	xputc(c)

      Usage:    ecurz -Z [-abeuy]    (ZMODEM)
                ecurz -Y [-abuy]     (YMODEM)
                ecurz -X [-abc] file (XMODEM or XMODEM-1k)

          -a ASCII transfer (strip CR)
          -b Binary transfer for all files
          -c Use 16 bit CRC (XMODEM)
          -e Escape control characters  (ZMODEM)
          -p protect local files (ZMODEM)
          -t <tenths> rx timeout seconds
          -+ force append
          -u convert uppercase filenames to lower case
          -y Yes, clobber existing file if any
          -@ line bitrate
          -. line fd to use
          -, log protocol packets

--------------------------------------------------------------------------*/
/*+:EDITS:*/
/*:01-24-1997-02:38-wht@yuriatin-SOURCE RELEASE 4.00 */
/*:09-11-1996-20:01-wht@yuriatin-3.48-major telnet,curses,structural overhaul */
/*:09-06-1996-03:18-wht@kepler-cleanup */
/*:08-20-1996-12:39-wht@kepler-locale/ctype fixes from ache@nagual.ru */
/*:08-11-1996-02:10-wht@kepler-rename ecu_log_event to logevent */
/*:12-28-1995-12:54-wht@kepler-Andrey Chernov FreeBSD fixes */
/*:11-28-1995-19:59-wht@kepler-add log_packets to readline */
/*:11-27-1995-20:42-wht@kepler-add zputc_init */
/*:11-14-1995-10:23-wht@kepler-3.37.80-source control point: SOCKETS */
/*:10-14-1995-23:46-wht@kepler-switch -@ passes line bitrate value */
/*:05-13-1995-11:43-wht@n4hgf-apply Andrey BSD 4.4 st_size patch */
/*:05-09-1995-17:08-wht@kepler-no more use of VOLATILE */
/*:02-17-1995-14:21-wht@n4hgf-apply Andrew Chernov to last 3.33 prerelease */
/*:01-12-1995-15:20-wht@n4hgf-apply Andrew Chernov 8-bit clean+FreeBSD patch */
/*:05-04-1994-04:40-wht@n4hgf-ECU release 3.30 */
/*:04-01-1994-20:56-wht@n4hgf-pedantic what-compatible version */
/*:02-16-1993-13:14-wht@n4hgf-make Bitrate long for 286 */
/*:09-10-1992-14:00-wht@n4hgf-ECU release 3.20 */
/*:08-22-1992-15:39-wht@n4hgf-ECU release 3.20 BETA */
/*:08-16-1992-03:08-wht@n4hgf-head off another POSIX plot */
/*:08-10-1992-04:01-wht@n4hgf-use init_Nap */
/*:07-30-1992-16:35-wht@n4hgf-our_fopen fixes 3.2v4 ENAMETOOLONG ambiguity */
/*:07-20-1992-13:39-wht@n4hgf-need hzmsec for nap.c */
/*:04-24-1992-15:28-wht@n4hgf-start thinking about M_UNIX with long filenames */
/*:04-24-1992-15:23-wht@n4hgf-fix mkdir/make_dirs conditionals */
/*:01-27-1992-23:43-wht@n4hgf-more efficient fopen processing */
/*:01-20-1992-23:25-root@n4hgf-ZMAPND works now */
/*:12-16-1991-12:59-wht@n4hgf-support ZCRESUM */
/*:08-28-1991-14:08-wht@n4hgf2-SVR4 cleanup by aega84!lh */
/*:07-25-1991-12:59-wht@n4hgf-ECU release 3.10 */
/*:04-30-1991-18:33-wht@n4hgf-gcc version coredumping on putc(); use fputc() */
/*:03-27-1991-21:21-wht@n4hgf-dont bump error count on send ZRPOS */
/*:02-03-1991-17:27-wht@n4hgf-version number change - see zcurses.c */
/*:12-18-1990-21:26-wht@n4hgf-better output control */
/*:10-04-1990-14:01-wht@n4hgf-add file finish warning for me */
/*:09-19-1990-19:36-wht@n4hgf-logevent now gets pid for log from caller */
/*:08-23-1990-14:14-wht@n4hgf-sending ZACK was erroneously counted as error */
/*:08-14-1990-20:40-wht@n4hgf-ecu3.00-flush old edit history */

#include <stdio.h>
#include <signal.h>
#include <setjmp.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include "../ecu_config.h"
#ifdef __FreeBSD__
#include <locale.h>
#endif
#include "zmodem.h"
#include <sys/param.h>

#if defined(ENAMETOOLONG)
char *fname_truncated();

#endif

extern unsigned short crctab[];
extern int force_dumbtty;
extern int errno;

extern char Attn[];			 /* Attention string rx sends to tx on err */
extern int Crc32;			 /* Display flag indicating 32 bit CRC being
							  * received */
extern int Rxcount;			 /* Count of data bytes received */
extern char Rxhdr[];		 /* Received header */
extern char Txhdr[];		 /* Transmitted header */
extern int Rxtimeout;		 /* Tenths of seconds to wait for something */
extern char s128[128];

/*
 * Max value for VMIN_COUNT is 255.  A larger value reduces system
 * overhead but may evoke kernel bugs.  133 corresponds to an XMODEM/CRC
 * sector.

Paul Slootman said, though:
:PS: Something somewhere in the SVR4 kernel is a signed char, which causes
:PS: VMIN values of more than 127 to return *immediately* without ever
:PS: reading...
:PS:
:PS: I had troubles running the regular rz, which was where I saw
:PS: the bug the first time. I've also heard of this from someone
:PS: else, running something else than the ICL SPARC port for SVR4:
:PS:
:PS: Date: Sat, 3 Aug 91 11:41:16 EDT
:PS: From: tompkins@cat.syr.edu (Terry Tompkins)
:PS: Subject: Re:  Zmodem
:PS:
:PS: Thanks for the info.  I just returned from vacation - sorry for the delay.
:PS: We are running AT&T 5.4 UNIX on an Osicom 25mhz 386.  If you hear of a
:PS: fix for the OS, let me know - I feel a little apprehensive about a kernel
:PS: bug of this nature.  (The machine is a network server that we are using
:PS: for all kinds of things).
*/

#if !defined(VMIN_COUNT)
#ifdef SVR4
#define VMIN_COUNT 127
#else
#define VMIN_COUNT 133
#endif
#endif
unsigned char vmin_count = VMIN_COUNT;
int Readnum = VMIN_COUNT;	 /* num bytes to ask for in read() from modem */

#define DEFBYTL 2000000000L	 /* default rx file size */
#define RETRYMAX 5

FILE *fout;
long rxfilepos;				 /* received file seek position */
long initial_filepos;		 /* initial file position */
char Lzmanag;				 /* Local file management request */
char Pathname[PATHLEN];
char curr_dir[256];
unsigned char linbuf[VMIN_COUNT];
char secbuf[1025];
char zconv;					 /* ZMODEM file conversion request */
char zmanag;				 /* ZMODEM file management request */
char ztrans;				 /* ZMODEM file transport request */
int Batch;
int Blklen;					 /* record length of received packets */
int Crcflg;
int Eofseen;				 /* indicates cpm eof (^Z) has been received */
int Filcnt;					 /* count of number of files opened */
int Filemode;				 /* Unix style mode for incoming file */
int Firstsec;
int Lastrx;
int Lleft;					 /* number of characters in linbuf */
int MakeLCPathname = 1;		 /* make received pathname lower case */
int Nozmodem;				 /* If invoked as "rb" */
int Rxascii;				 /* receive files in ascii (translate) mode */
int Rxbinary;				 /* receive all files in bin mode */
int Rxclob;					 /* Clobber existing file */
int Thisbinary;				 /* current file is to be received in bin mode */
int Twostop;				 /* use two stop bits */
int Zctlesc;				 /* Encode control characters */
int Zmodem;					 /* ZMODEM protocol requested */
int Zrwindow = 1400;		 /* RX window size (controls garbage count) */
int ecusz_flag;
int skip_count;				 /* skipped files */
int errors;
int expect_zrpos;
int iofd;
int force_dumbtty;
int can_on_eof;
int log_packets;
int npats;
int oldBlklen = -1;			 /* last block length */
int telnet;
extern int this_file_errors;
int tryzhdrtype = ZRINIT;	 /* Header type to send corresponding to Last
							  * rx close */
jmp_buf tohere;				 /* For the interrupt on RX timeout */
long Bytesleft;				 /* number of bytes of incoming file left */
long Modtime;				 /* Unix style mod time for incoming file */
long TotalToReceive;
long rx_char_count;
long tx_char_count;
struct stat fout_stat;
time_t timep[2];
unsigned long Bitrate;
unsigned long this_file_length;
int required_type;
char *bottom_label = (char *)0;
char *got_garbage_txt = "got garbage (0x%02x)";
char **gargv;
int gargc;

void purgeline();
void send_cancel();

/*+-----------------------------------------------------------------------
	arg_token(parsestr,termchars)

Get next token from string parsestr ((char *)0 on 2nd, 3rd, etc.
calls), where tokens are nonempty strings separated by runs of chars
from termchars.  Writes nulls into parsestr to end tokens.
termchars need not remain constant from call to call.

Treats multiple occurrences of a termchar as one delimiter (does not
allow null fields).
------------------------------------------------------------------------*/
#if defined(ENAMETOOLONG)
static char *arg_token_static = (char *)0;
char *
arg_token(parsestr, termchars)
char *parsestr;
char *termchars;
{
	register char *parseptr;
	char *token;

	if (parsestr == (char *)0 && arg_token_static == (char *)0)
		return ((char *)0);

	if (parsestr)
		parseptr = parsestr;
	else
		parseptr = arg_token_static;

	while (*parseptr)
	{
		if (!strchr(termchars, *parseptr))
			break;
		parseptr++;
	}

	if (!*parseptr)
	{
		arg_token_static = (char *)0;
		return ((char *)0);
	}

	token = parseptr;
	if (*token == '\'')
	{
		token++;
		parseptr++;
		while (*parseptr)
		{
			if (*parseptr == '\'')
			{
				arg_token_static = parseptr + 1;
				*parseptr = 0;
				return (token);
			}
			parseptr++;
		}
		arg_token_static = (char *)0;
		return (token);
	}
	while (*parseptr)
	{
		if (strchr(termchars, *parseptr))
		{
			*parseptr = 0;
			arg_token_static = parseptr + 1;
			while (*arg_token_static)
			{
				if (!strchr(termchars, *arg_token_static))
					break;
				arg_token_static++;
			}
			return (token);
		}
		parseptr++;
	}
	arg_token_static = (char *)0;
	return (token);
}							 /* end of arg_token */
#endif

/*+-------------------------------------------------------------------------
	fname_split(cmd,arg,arg_max_quan,&narg)
--------------------------------------------------------------------------*/
#if defined(ENAMETOOLONG)
void
fname_split(cmd, arg, arg_max_quan, narg_rtn)
char *cmd;
char **arg;
int arg_max_quan;
int *narg_rtn;
{
	register itmp;
	register narg;

	for (itmp = 0; itmp < arg_max_quan; itmp++)
		arg[itmp] = (char *)0;
	arg[0] = arg_token(cmd, "/");

	for (narg = 1; narg < arg_max_quan; ++narg)
	{
		if ((arg[narg] = arg_token((char *)0, "/")) == (char *)0)
			break;
	}

	*narg_rtn = narg;

}							 /* end of fname_split */
#endif

#if defined(ENAMETOOLONG)
#define MAX_COMPONENT_LEN	14
#define MAX_PATH_COMPONENTS	16
static char trunc_fname[257];
static char *trunc_components[MAX_PATH_COMPONENTS];
static int trunc_components_quan;
static int trunc_absolute_path;

#endif

/*+-------------------------------------------------------------------------
	fname_too_long(fname) - check for any pathname component too long
--------------------------------------------------------------------------*/
#if defined(ENAMETOOLONG)
int
fname_too_long(fname)
register char *fname;
{
	register int itmp;
	register char **cpptr;

	if (trunc_absolute_path = (*fname == '/'))
		fname++;
	strncpy(trunc_fname, fname, sizeof(trunc_fname) - 1);
	fname_split(trunc_fname, trunc_components,
		MAX_PATH_COMPONENTS, &trunc_components_quan);
	itmp = trunc_components_quan;
	cpptr = trunc_components;
	while (itmp--)
	{
		if (strlen(*cpptr) > MAX_COMPONENT_LEN)
			return (1);
		cpptr++;
	}
	return (0);
}							 /* end of fname_too_long */
#endif

/*+-------------------------------------------------------------------------
	fname_truncated() - build truncated path last checked by fname_too_long
--------------------------------------------------------------------------*/
#if defined(ENAMETOOLONG)
char *
fname_truncated()
{
	register int icomp;
	char new_fname[257];
	register char *cptr = new_fname;

	if (trunc_absolute_path)
	{
		*cptr = '/';
		*(cptr + 1) = 0;
	}
	else
		*cptr = 0;
	for (icomp = 0; icomp < trunc_components_quan; icomp++)
	{
		if (strlen(trunc_components[icomp]) > MAX_COMPONENT_LEN)
			*(trunc_components[icomp] + MAX_COMPONENT_LEN) = 0;
		strcat(cptr, trunc_components[icomp]);
		if (icomp < trunc_components_quan - 1)
			strcat(cptr, "/");
	}
	strcpy(trunc_fname, cptr);
	return (trunc_fname);

}							 /* end of fname_truncated */
#endif

/*+-------------------------------------------------------------------------
	our_fopen(pathname,openmode) - fopen for write considering ENAMETOOLONG

This can modify the pathname argument
--------------------------------------------------------------------------*/
FILE *
our_fopen(pathname, openmode)
char *pathname;
char *openmode;
{
	FILE *fp;

	if (!(fp = fopen(pathname, openmode)))
	{
#if defined(ENAMETOOLONG)
		if (errno == ENAMETOOLONG)
		{
			if (fname_too_long(pathname))
			{
				strcpy(s128, "truncated: ");
				strncat(s128, pathname, sizeof(s128) - 12);
#if defined(CFG_LogXfer)
				logevent(getppid(), s128);
#endif
				report_str(s128, -1);
				strcpy(pathname, fname_truncated());
				fp = fopen(pathname, openmode);
			}
		}
#else
		;					 /* dummy statement for anti new-fangled
							  * compiler warnings */
#endif
	}

	return (fp);

}							 /* end of our_fopen */

/*+-------------------------------------------------------------------------
	substr(str,token)

  searches for token in string str returns pointer to token within
  string if found,NULL otherwise
--------------------------------------------------------------------------*/
char *
substr(str, token)
register char *str, *token;
{
	register char *ss;
	register char *tt;

	/* search for first char of token */
	for (ss = str; *str; str++)
		if (*str == *token)
			/* compare token with substring */
			for (ss = str, tt = token;;)
			{
				if (!*tt)
					return (str);
				if (*ss++ != *tt++)
					break;
			}
	return (NULL);
}							 /* end of substr */

/*+-------------------------------------------------------------------------
	getfree()

  Routine to calculate the free bytes on the current file system ~0
  means many free bytes (unknown)
--------------------------------------------------------------------------*/
long
getfree()
{
	return (~0L);			 /* many free bytes ... */
}							 /* end of getfree */

/*+-------------------------------------------------------------------------
	usage(fail_reason)
--------------------------------------------------------------------------*/
void
usage(fail_reason)
char *fail_reason;
{
	fprintf(stderr, "%s\n", fail_reason);
	exit(255);
}							 /* end of usage */

/*+-------------------------------------------------------------------------
	SIGALRM_handler()
--------------------------------------------------------------------------*/
CFG_SigType
SIGALRM_handler(sig)
int sig;
{
	report_tx_ind(0);
	report_rx_ind(0);
	longjmp(tohere, -1);
	sig = 0;				 /* prevent unused warnings */
}							 /* end of SIGALRM_handler */

/*+-------------------------------------------------------------------------
	bye_bye(sig)
--------------------------------------------------------------------------*/
void
bye_bye(sig)
int sig;
{
	exit(sig + 128);
}							 /* end of bye_bye */

/*+-------------------------------------------------------------------------
	cancel_transaction(can_code)
called by signal interrupt or terminate to clean things up
--------------------------------------------------------------------------*/
void
cancel_transaction(can_code)
int can_code;
{
	purgeline();
	if (Zmodem)
		zmputs(Attn);
	send_cancel(1);
	mode(0);
	if (can_code >= 0)
	{
		sprintf(s128, "ecurz aborted (signal %d)", can_code);
		report_str(s128, 0);
	}
	report_uninit();
	bye_bye(can_code);

}							 /* end of cancel_transaction */

/*+-------------------------------------------------------------------------
	sendline(c) -  send a character to DCE
--------------------------------------------------------------------------*/
void
sendline(c)
char c;
{
	write(iofd, &c, 1);
	++tx_char_count;
}							 /* end of sendline */

/*+-------------------------------------------------------------------------
	xputc(c)
--------------------------------------------------------------------------*/
void
xputc(c)
int c;
{
	sendline(c);
}							 /* end of xputc */

/*+-------------------------------------------------------------------------
	flushline()
--------------------------------------------------------------------------*/
void
flushline()
{
}							 /* end of flushline */

/*+-------------------------------------------------------------------------
	purgeline() - purge the modem input queue of all characters
--------------------------------------------------------------------------*/
void
purgeline()
{
	Lleft = 0;
#if defined(TCFLSH)
	ioctl(iofd, TCFLSH, 0);
#else
	lseek(iofd, 0L, 2);
#endif
}							 /* end of purgeline */

/*+-------------------------------------------------------------------------
	wcreceive(argc,argp)
--------------------------------------------------------------------------*/
wcreceive(argc, argp)
int argc;
char **argp;
{
	register c;

	if (Batch || argc == 0)
	{
		Crcflg = 1;
		c = tryz();
		if (Zmodem)
		{
			report_protocol_type("ZMODEM");
			report_protocol_crc_type((Crc32) ? "/CRC32" : "/CRC16");
		}
		if (c)
		{
			if (c == ZCOMPL)
				return (OK);
			if (c == ERROR)
				goto FAIL;
			c = rzfiles();
			if (c)
				goto FAIL;
		}
		else
		{
			report_protocol_type("YMODEM");
			report_protocol_crc_type((Crcflg) ? "/CRC" : "/CHK");
			for (;;)
			{
				if (wcrxpn(secbuf) == ERROR)
					goto FAIL;
				if (secbuf[0] == 0)
					return (OK);
				if (procheader(secbuf) == ERROR)
					goto FAIL;
				report_str("Receiving data", 0);
				if (wcrx() == ERROR)
					goto FAIL;
			}
		}
	}
	else
	{
		report_protocol_type("XMODEM");
		report_protocol_crc_type((Crcflg) ? "/CRC" : "/CHK");
		Bytesleft = DEFBYTL;
		Filemode = 0;
		Modtime = 0L;
		procheader("");
		strcpy(Pathname, *argp);
		if (!(fout = our_fopen(Pathname, "w")))
		{
			sprintf(s128, "%-.64s: %-.40s", Pathname, strerror(errno));
			report_str(s128, 1);
#if defined(CFG_LogXfer)
			logevent(getppid(), s128);
#endif
			goto FAIL;
		}

		report_file_rcv_started(Pathname,
			(unsigned short)Filemode);
		this_file_length = 0;
		report_rxpos(0L);
		report_str("Receiving data", 0);
		if (wcrx() == ERROR)
			goto FAIL;
	}
	return (OK);

  FAIL:
	send_cancel(1);
	if (fout)
	{
		fflush(fout);
		fstat(fileno(fout), &fout_stat);
		report_file_byte_io((long)fout_stat.st_size - initial_filepos);
		report_file_close(0);
		fclose(fout);
		fout = (FILE *) 0;
	}
	return (ERROR);
}							 /* end of wcreceive */

/*+-------------------------------------------------------------------------
	wcgetsec(rxbuf,maxtime)

  Wcgetsec fetches a Ward Christensen type sector.  Returns sector
  number encountered or ERROR if valid sector not received, or CAN CAN
  received or WCEOT if eot sector time is timeout for first char,set to
  4 seconds thereafter. NO ACK IS SENT IF SECTOR IS RECEIVED OK. Caller
  must do that when he is good and ready to get next sector.
--------------------------------------------------------------------------*/
unsigned int
wcgetsec(rxbuf, maxtime)
char *rxbuf;
int maxtime;
{
	register int firstch;
	register unsigned short oldcrc;
	register unsigned char checksum;
	register wcj;
	register char *p;
	int sectcurr;

	for (Lastrx = errors = 0; errors < RETRYMAX; errors++)
	{

		firstch = readline(maxtime);
		if ((firstch == STX) || (firstch == SOH))
		{
			oldBlklen = Blklen;
			if (firstch == STX)
				Blklen = 1024;
			else
				Blklen = 128;
			if (oldBlklen != Blklen)
				report_rxblklen(Blklen);

			sectcurr = readline(1);
			if ((sectcurr + (oldcrc = readline(1))) == 0xFF)
			{
				oldcrc = checksum = 0;
				for (p = rxbuf, wcj = Blklen; --wcj >= 0;)
				{
					if ((int)(firstch = readline(1)) < 0)
						goto bilge;
					oldcrc = updcrc(firstch, oldcrc);
					checksum += (*p++ = firstch);
				}
				if ((int)(firstch = readline(1)) < 0)
					goto bilge;
				if (Crcflg)
				{
					oldcrc = updcrc(firstch, oldcrc);
					if ((int)(firstch = readline(1)) < 0)
						goto bilge;
					oldcrc = updcrc(firstch, oldcrc);
					if (oldcrc)
					{
						sprintf(s128, "CRC error = 0x%04x", oldcrc);
						report_str(s128, 1);
					}
					else
					{
						Firstsec = 0;
						return (sectcurr);
					}
				}
				else if ((checksum - firstch) == 0)
				{
					Firstsec = 0;
					return (sectcurr);
				}
				else
					report_str("checksum error", 1);
			}
			else
			{
				report_last_txhdr("Noise", 0);
				sprintf(s128, "Sector garbled 0x%x 0x%x", sectcurr, oldcrc);
				report_str(s128, 1);
			}
		}
		/* make sure eot really is eot and not just mixmash */
#if defined(NFGVMIN)
		else if (firstch == EOT && readline(1) == TIMEOUT)
			return (WCEOT);
#else
		else if (firstch == EOT && Lleft == 0)
			return (WCEOT);
#endif
		else if (firstch == EOT)
		{
			report_str("Noisy EOT", 2);
		}
		else if (firstch == CAN)
		{
			if (Lastrx == CAN)
			{
				report_str("Sender CANcelled", 1);
				report_last_rxhdr("CAN", 1);
				return (ERROR);
			}
			else
			{
				Lastrx = CAN;
				continue;
			}
		}
		else if (firstch == TIMEOUT)
		{
			if (Firstsec)
				goto humbug;
		  bilge:
			report_str("Timeout", 1);
		}
		else
		{
			sprintf(s128, "Got 0x%02x sector header", firstch);
			report_str(s128, 1);
		}

	  humbug:
		Lastrx = 0;
		while (readline(1) != TIMEOUT)
			;
		if (Firstsec)
		{
			sendline(Crcflg ? WANTCRC : NAK);
			report_last_txhdr(Crcflg ? "WANTCRC" : "NAK", 0);
			Lleft = 0;		 /* Do read next time ... */
		}
		else
		{
			maxtime = 40;
			sendline(NAK);
			report_last_txhdr("NAK", 1);
			Lleft = 0;		 /* Do read next time ... */
		}
	}
	/* try to stop the bubble machine. */
	send_cancel(1);
	return (ERROR);
}							 /* end of wcgetsec */

/*+-------------------------------------------------------------------------
	wcrxpn(rpn)

  Fetch a pathname from the other end.  Length is indeterminate as long
  as less than Blklen.  During YMODEM xfers, a null string represents no
  more files.
--------------------------------------------------------------------------*/
wcrxpn(rpn)
char *rpn;					 /* receive a pathname */
{
	register int c;

#if defined(NFGVMIN)
	readline(1);
#else
	purgeline();
#endif

  et_tu:
	Firstsec = 1;
	Eofseen = 0;
	sendline(Crcflg ? WANTCRC : NAK);
	report_last_txhdr(Crcflg ? "WANTCRC" : "NAK", 0);
	Lleft = 0;				 /* Do read next time ... */
	while (c = wcgetsec(rpn, 100))
	{
		if (c == WCEOT)
		{
			sprintf(s128, "Pathname fetch returned %d", c);
			report_str(s128, 1);
			sendline(ACK);
			report_last_txhdr("ACK", 0);
			Lleft = 0;		 /* Do read next time ... */
			readline(1);
			goto et_tu;
		}
		return (ERROR);
	}
	sendline(ACK);
	report_last_txhdr("ACK", 0);
	return (OK);
}							 /* end of wcrxpn */

/*+-------------------------------------------------------------------------
	report_receive_progress(pos)
--------------------------------------------------------------------------*/
void
report_receive_progress(pos)
long pos;
{

	report_rxpos(pos);
	if (this_file_length)
	{
		sprintf(s128, "Receiving data (%lu%% complete)",
			(int)((unsigned long)pos * (unsigned long)100) /
			this_file_length);
		report_str(s128, 0);
	}
}							 /* end of report_receive_progress */

/*+-------------------------------------------------------------------------
	write_sec_to_disk(buf,n)

  Putsec writes the n characters of buf to receive file fout.  If not in
  binary mode, carriage returns, and all characters starting with CPMEOF
  are discarded.
--------------------------------------------------------------------------*/
write_sec_to_disk(buf, n)
char *buf;
register n;
{
	register char *p;

	if (!n)
		return (OK);
	if (Thisbinary)
	{
		for (p = buf; --n >= 0;)
			fputc(*p++, fout);
	}
	else
	{
		if (Eofseen)
			return (OK);
		for (p = buf; --n >= 0; ++p)
		{
			if (*p == '\r')
				continue;
			if (*p == CPMEOF)
			{
				Eofseen = 1;
				fflush(fout);
				fstat(fileno(fout), &fout_stat);
				report_rxpos((long)fout_stat.st_size);
				return (OK);
			}
			fputc(*p, fout);
		}
	}
	if (!Zmodem)
	{
		fflush(fout);
		fstat(fileno(fout), &fout_stat);
		report_rxpos((long)fout_stat.st_size);
	}
	return (OK);
}							 /* end of write_sec_to_disk */

/*+-------------------------------------------------------------------------
	wcrx() - receive an X/YMODEM sector

  Adapted from CMODEM13.C,written by Jack M.  Wierda and Roderick W. Hart
--------------------------------------------------------------------------*/
int
wcrx()
{
	register int sectnum, sectcurr;
	register unsigned char sendchar;
	int cblklen;			 /* bytes to dump this block */

	Firstsec = 1;
	sectnum = 0;
	Eofseen = 0;
	sendchar = Crcflg ? WANTCRC : NAK;
	report_last_txhdr(Crcflg ? "WANTCRC" : "NAK", 0);

	for (;;)
	{
		sendline(sendchar);	 /* send it now,we're ready! */
		if (sendchar == ACK)
			report_last_txhdr("ACK", 0);
		Lleft = 0;			 /* Do read next time ... */
		sectcurr = wcgetsec(secbuf, (sectnum & 0177) ? 50 : 130);
		sprintf(s128, "Block %d received", sectnum);
		report_last_rxhdr(s128, 0);
		fstat(fileno(fout), &fout_stat);
		report_rxpos((long)fout_stat.st_size);
		if (sectcurr == ((sectnum + 1) & 0xFF))
		{
			sectnum++;
			cblklen = Bytesleft > Blklen ? Blklen : Bytesleft;
			if (write_sec_to_disk(secbuf, cblklen) == ERROR)
				return (ERROR);
			if ((Bytesleft -= cblklen) < 0)
				Bytesleft = 0;
			sendchar = ACK;
		}
		else if (sectcurr == sectnum)
		{
			report_str("Received duplicate Sector", -1);
			sendchar = ACK;
		}
		else if (sectcurr == WCEOT)
		{
			if (close_and_report())
				return (ERROR);
			sendline(ACK);
			report_last_txhdr("ACK", 0);
			Lleft = 0;		 /* Do read next time ... */
			return (OK);
		}
		else if (sectcurr == ERROR)
			return (ERROR);
		else
		{
			report_str("Sync Error", 1);
			return (ERROR);
		}
	}
}							 /* end of wcrx */

#if 0						 /* <> */
#include <sys/time.h>
#endif

/*+-------------------------------------------------------------------------
	readline(timeout)

  read one or more characters timeout is in tenths of seconds
--------------------------------------------------------------------------*/
int
readline(timeout)
int timeout;
{
	int n = 0;
	static unsigned char *cdq;	/* pointer for removing chars from linbuf */

	if (--Lleft >= 0)
		return (*cdq++);

	if (setjmp(tohere))
	{
		Lleft = 0;
		return (TIMEOUT);
	}
	n = timeout / 10;
	if (n < 2)
		n = 3;
	signal(SIGALRM, SIGALRM_handler);
#if 0						 /* <> */
	{
		struct timeval tval, *tv = &tval;
		static FILE *fp = 0;
		struct tm *lt;

		gettimeofday(tv, 0);
		lt = localtime(&tv->tv_sec);
		sprintf(s128, "<>%d=%2d.%06d->",
			lt->tm_sec,
			tv->tv_usec);
		alarm(n);
		Lleft = read(iofd, (char *)(cdq = linbuf), Readnum);
		alarm(0);
		rx_char_count += Lleft;
		gettimeofday(tv, 0);
		lt = localtime(&tv->tv_sec);
		sprintf(s128 + strlen(s128), "%2d.%06d (%d)",
			lt->tm_sec, tv->tv_usec, Lleft);
		if (!fp)
			fp = fopen("/tmp/hack.log", "a");
		if (fp)
		{
			hex_dump_fp(fp, cdq, (Lleft > 0) ? Lleft : 0, s128, 0);
			fflush(fp);
		}
	}						 /* <> */
#else
	alarm(n);
	Lleft = read(iofd, (char *)(cdq = linbuf), Readnum);
	alarm(0);
	if (Lleft > 1)
		rx_char_count += Lleft;
	if (log_packets)
	{
		static FILE *fp = 0;

		if (!fp)
			fp = fdopen(log_packets, "a");
		if (fp && (Lleft > 1))
		{
			hex_dump_fp(fp, cdq, Lleft, "readline data", 0);
			fflush(fp);
		}
	}
#endif

	if (Lleft < 1)
		return (TIMEOUT);

	--Lleft;
	return (*cdq++);

}							 /* end of readline */

/*+-------------------------------------------------------------------------
	mkdir(dpath,dmode)
 Directory-creating routines from Public Domain TAR by John Gilmore
 Make a directory.  Compatible with the mkdir() system call on 4.2BSD.
--------------------------------------------------------------------------*/
#if defined(M_XENIX)
#define	TERM_SIGNAL(status)		((status) & 0x7F)
#define TERM_COREDUMP(status)	(((status) & 0x80) != 0)
#define TERM_VALUE(status)		((status) >> 8)
mkdir(dpath, dmode)
char *dpath;
int dmode;
{
	int cpid, status;
	struct stat statbuf;

	if (!stat(dpath, &statbuf))
	{
		errno = EEXIST;		 /* Stat worked,so it already exists */
		return (-1);
	}

	/* If stat fails for a reason other than non-existence,return error */
	if (errno != ENOENT)
		return (-1);

	switch (cpid = fork())
	{

		case -1:			 /* Error in fork() */
			return (-1);	 /* Errno is set already */

		case 0:			 /* Child process */

			/*
			 * Cheap hack to set mode of new directory.  Since this child
			 * process is going away anyway,we zap its umask. FIXME,this
			 * won't suffice to set SUID,SGID,etc. on this directory. Does
			 * anybody care?
			 */
			status = umask(0);	/* Get current umask */
			status = umask(status | (0777 & ~dmode));	/* Set for mkdir */
			execl("/bin/mkdir", "mkdir", dpath, (char *)0);
			_exit(-1);		 /* Can't exec /bin/mkdir */

		default:			 /* Parent process */
			while (cpid != wait(&status)) ;	/* Wait for kid to finish */
	}

	if (TERM_SIGNAL(status) != 0 || TERM_VALUE(status) != 0)
	{
		errno = EIO;		 /* We don't know why,but */
		return (-1);		 /* /bin/mkdir failed */
	}

	return (0);
}							 /* end of mkdir */
#endif /* M_XENIX */

/*+-------------------------------------------------------------------------
	make_dirs(pathname)

  Directory-creating routines from Public Domain TAR by John Gilmore
  After a file/link/symlink/dir creation has failed, see if it's because
  some required directory was not present, and if so, create all
  required dirs.
--------------------------------------------------------------------------*/
int
make_dirs(pathname)
register char *pathname;
{
	register char *p;		 /* Points into path */
	int madeone = 0;		 /* Did we do anything yet? */
	int save_errno = errno;	 /* Remember caller's errno */

	if (errno != ENOENT)
		return (0);			 /* Not our problem */

	for (p = strchr(pathname, '/'); p != NULL; p = strchr(p + 1, '/'))
	{
		/* Avoid mkdir of empty string,if leading or double '/' */
		if (p == pathname || p[-1] == '/')
			continue;
		/* Avoid mkdir where last part of path is '.' */
		if (p[-1] == '.' && (p == pathname + 1 || p[-2] == '/'))
			continue;
		*p = 0;				 /* Truncate the path there */
		if (!mkdir(pathname, 0777))	/* Try to create it as a dir */
		{
			sprintf(s128, "Made directory %s", pathname);
			report_str(s128, -1);
			madeone++;		 /* Remember if we made one */
			*p = '/';
			continue;
		}
		*p = '/';
		if (errno == EEXIST) /* Directory already exists */
			continue;

		/*
		 * Some other error in the mkdir.  We return to the caller.
		 */
		break;
	}
	errno = save_errno;		 /* Restore caller's errno */
	return (madeone);		 /* Tell them to retry if we made one */
}							 /* end of make_dirs */

/*+-------------------------------------------------------------------------
	uncaps(str) - make string str lower case
--------------------------------------------------------------------------*/
void
uncaps(str)
register char *str;
{
	register int itmp;

	while (itmp = *str & 0xFF)
	{
		if (isupper(itmp))
			*str = tolower(itmp);
		str++;
	}
}							 /* end of uncaps */

/*+-------------------------------------------------------------------------
	isanylc(str) - returns 1 if string str has any lower case letters
--------------------------------------------------------------------------*/
int
isanylc(str)
register char *str;
{
	while (*str)
	{
		if (islower((unsigned char)*str))
			return (1);
		str++;
	}
	return (0);
}							 /* end of isanylc */

/*+-------------------------------------------------------------------------
	procheader(name) - process incoming file information header

returns with 0 and FILE *fout open to receive file if good headers
and all is right with the filesystem, else returns error code
--------------------------------------------------------------------------*/
int
procheader(name)
char *name;
{
	register char *openmode, *p;
	char zmanag2;

	/* set default parameters and overrides */
	fout = (FILE *) 0;
	openmode = "w";
	rxfilepos = 0L;
	Thisbinary = (!Rxascii) || Rxbinary;
	if (Lzmanag)
		zmanag = Lzmanag;
	zmanag2 = zmanag & ZMMASK;

	/*
	 * Process ZMODEM remote file management requests
	 */
	if (!Rxbinary && zconv == ZCNL)	/* Remote ASCII override */
		Thisbinary = 0;
	if (zconv == ZCBIN)		 /* Remote Binary override */
		Thisbinary = 1;

	report_xfer_mode(Thisbinary ? "BINARY" : "ASCII");
	this_file_errors = 0;

	Bytesleft = DEFBYTL;
	Filemode = 0;
	Modtime = 0L;
	this_file_length = 0L;
	initial_filepos = 0L;

	if (strlen(name))
		p = name + 1 + strlen(name);
	else
		p = name;

	if (*p)
	{						 /* header has attributes */
		int sscanf_count = 0;
		int SerialNumber = 0;
		int Filesleft = 0;
		long TotalLeft = 0;

		sscanf_count = sscanf(p, "%ld%lo%o%d&d&ld&ld",
			&Bytesleft,		 /* file size */
			&Modtime,		 /* secs since 1970 */
			&Filemode,		 /* unix st_mode */
			&SerialNumber,	 /* vaxism */
			&Filesleft, &TotalLeft);

		switch (sscanf_count)
		{
			case 6:		 /* TotalLeft */
				if (!TotalToReceive)
					TotalToReceive = TotalLeft;
			case 5:		 /* Filesleft */
				if (!npats)
					npats = Filesleft;
			default:
				break;
		}

		if (Thisbinary && (zconv == ZCRESUM))
		{
			if (!stat(name, &fout_stat))	/* if file accessible ... */
			{
				openmode = "r+";
				rxfilepos = fout_stat.st_size - 1024;	/* re-get last 1024 */
				if (Bytesleft < rxfilepos)
					rxfilepos = 0;
				if (rxfilepos < 0)
					rxfilepos = 0;
				initial_filepos = rxfilepos;
				expect_zrpos = 1;	/* don't count first ZRPOS as error */
			}
		}
		else if (zmanag2 == ZMNEW)
		{
			if (!stat(name, &fout_stat))	/* if file accessible ... */
			{
				if (Modtime <= fout_stat.st_mtime)	/* ... and not older */
				{
					sprintf(s128, "RECEIVE skipped: %s (same or later date)",
						name);
					report_str(s128 + 8, -1);
					skip_count++;
					report_error_count();
#if defined(LOG_SKIP) && defined(CFG_LogXfer)
					logevent(getppid(), s128);
#endif
					return (ERROR);
				}
				openmode = "w";
				if (!(fout = our_fopen(name, openmode)))
					return ZFERR;
			}
		}
		else if (zmanag2 == ZMAPND)
		{
			if (!stat(name, &fout_stat))	/* if file accessible ... */
				initial_filepos = fout_stat.st_size;
		}
		else if (!Rxclob && ((zmanag2 != ZMCLOB)) && !access(name, 0))
		{
			sprintf(s128, "RECEIVE skipped: %s (already exists)", name);
			report_str(s128 + 8, -1);
			skip_count++;
			report_error_count();
#if defined(LOG_SKIP) && defined(CFG_LogXfer)
			logevent(getppid(), s128);
#endif
			return (ERROR);
		}

		if (Filemode & UNIXFILE)
			++Thisbinary;

		report_rxpos(0L);
		report_str("", 0);	 /* get rid of End of File */
		if (Bytesleft != DEFBYTL)
		{
			long min_100;

			this_file_length = Bytesleft;
			min_100 = 2L + ((((Bytesleft - initial_filepos) * 11L)) * 10L) /
				(Bitrate * 6L);
			report_file_rcv_started(name, Bytesleft,
				Modtime, (unsigned short)Filemode);
			sprintf(s128, "Receive time this file ~= %2lu:%02lu",
				min_100 / 100, ((min_100 % 100) * 60L) / 100L);
			if (TotalToReceive)
			{
				min_100 = 2L +
					(((TotalToReceive * 11L)) * 10L) / (Bitrate * 6L);
#if 0
				if (Bitrate > 4800)	/* nothing over 4800 ever ... */
				{			 /* occurs on time */
					min_100 *= 13;
					min_100 /= 9;	/* yech ... empirical */
				}
#endif
				sprintf(&s128[strlen(s128)], ", transaction ~= %2lu:%02lu",
					min_100 / 100, ((min_100 % 100) * 60L) / 100L);
			}
			report_transaction(s128);
			sprintf(s128, "Receiving data (%d%% complete)", (int)0);
			report_str(s128, 0);
		}
	}
	else
	{
		long now;

		for (p = name; *p; ++p)	/* change / to _ */
		{
			if (*p == '/')
				*p = '_';
		}

		if (*--p == '.')	 /* zap trailing period */
			*p = 0;
		time(&now);
		report_file_rcv_started(name, 0, now, 0);
	}

	if (!Zmodem && MakeLCPathname && !isanylc(name) && !(Filemode & UNIXFILE))
		uncaps(name);

	strcpy(Pathname, name);
	report_xfer_mode(Thisbinary ? "BINARY" : "ASCII");
	if (!fout)
		fout = our_fopen(name, openmode);
	if (!fout)
	{
		if (make_dirs(name))
			fout = our_fopen(name, openmode);
	}
	if (!fout)
	{
#if defined(CFG_LogXfer)
		vlogevent(getppid(), "%-.64s: open error: %s", name, strerror(errno));
#endif
		skip_count++;
		report_error_count();
		return (ERROR);
	}
	if (fseek(fout, rxfilepos, 0))
	{
		fclose(fout);
		fout = (FILE *) 0;
#if defined(CFG_LogXfer)
		vlogevent(getppid(), "%-.64s: seek error: %s", name, strerror(errno));
#endif
		skip_count++;
		report_error_count();
		return (ERROR);
	}
	this_file_errors = 0;
	return (OK);
}							 /* end of procheader */

/*+-------------------------------------------------------------------------
	send_cancel(error) - send cancel string
--------------------------------------------------------------------------*/
void
send_cancel(error)
int error;
{
	static char canistr[] =
	{
		24, 24, 24, 24, 24, 24, 24, 24, 24, 24,
		8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 0
	};
	register char *cptr = canistr;

	purgeline();
	report_str("", 0);
	report_last_txhdr("^X CAN", !!error);
	while (*cptr)
		sendline(*cptr++);
	Lleft = 0;
}							 /* end of send_cancel */

/*+-------------------------------------------------------------------------
	send_ZFIN() - send ZFIN frame and wait for "OO" ack
--------------------------------------------------------------------------*/
void
send_ZFIN()
{
	int tries = 4;

	Readnum = 1;
	stohdr(0L);
	while (tries--)
	{
		purgeline();
		zshhdr(ZFIN, Txhdr);
		switch (readline(100))
		{
			case 'O':
				readline(1); /* Discard 2nd 'O' */
				return;
			case RCDO:
				return;
			case TIMEOUT:
			default:
				break;
		}
	}
}							 /* end of send_ZFIN */

/*+-------------------------------------------------------------------------
	tryz()

  Initialize for Zmodem receive attempt, try to activate Zmodem sender
  Handles ZSINIT frame
  Return ZFILE if Zmodem filename received,-1 on error,
         ZCOMPL if transaction finished, else 0
--------------------------------------------------------------------------*/
int
tryz()
{
	register c;
	register n;

	if (Nozmodem)			 /* Check for "rb" program name */
		return (0);

	for (n = Zmodem ? 15 : 5; --n >= 0;)
	{
		/* Set buffer length (0) and capability flags */
		stohdr(0L);

#if defined(CANBREAK)
		Txhdr[ZF0] = CANFC32 | CANFDX | CANOVIO | CANBRK;
#else
		Txhdr[ZF0] = CANFC32 | CANFDX | CANOVIO;
#endif
		if (Zctlesc)
			Txhdr[ZF0] |= TESCCTL;
		zshhdr(tryzhdrtype, Txhdr);
		if (tryzhdrtype == ZSKIP)	/* Don't skip too far */
			tryzhdrtype = ZRINIT;	/* CAF 8-21-87 */
	  again:
		report_rx_tx_count();/* which will do a refresh */
		switch (zgethdr(Rxhdr))
		{
			case ZRQINIT:
				continue;
			case ZEOF:
				continue;
			case TIMEOUT:
				continue;
			case ZFILE:
				zconv = Rxhdr[ZF0];
				zmanag = Rxhdr[ZF1];
				ztrans = Rxhdr[ZF2];

				strcpy(s128, "Transfer type: ");
				if (zconv == ZCRESUM)
					strcat(s128, "resume interrupted transfer");
				else
				{
					switch (c = zmanag & ZMMASK)
					{
						case 0:
							strcat(s128, "if destination nonexistent");
							break;
						case ZMAPND:
							strcat(s128, "append to destination");
							break;
						case ZMCLOB:
							strcat(s128, "unconditional (overwrite)");
							break;
						case ZMNEW:
							strcat(s128, "if source newer");
							break;
						default:
							strcat(s128, "management option ");
							switch (c)
							{
								case ZMNEWL:
									strcat(s128, "ZMNEWL");
									break;
								case ZMCRC:
									strcat(s128, "ZMCRC");
									break;
								case ZMDIFF:
									strcat(s128, "ZMDIFF");
									break;
								case ZMPROT:
									strcat(s128, "ZMPROT");
									break;
								default:
									sprintf(s128 + strlen(s128), "%u", c);
									break;
							}
							break;
					}
				}
				report_str(s128, 2);

				tryzhdrtype = ZRINIT;
				c = zrdata(secbuf, 1024);
				mode(3);
				if (c == GOTCRCW)
					return (ZFILE);
				zshhdr(ZNAK, Txhdr);
				goto again;
			case ZSINIT:
				Zctlesc = TESCCTL & Rxhdr[ZF0];
				if (zrdata(Attn, ZATTNLEN) == GOTCRCW)
				{
					stohdr(1L);
					zshhdr(ZACK, Txhdr);
					report_str("", -1);
					goto again;
				}
				zshhdr(ZNAK, Txhdr);
				goto again;
			case ZFREECNT:
				stohdr(getfree());
				zshhdr(ZACK, Txhdr);
				report_str("", -1);
				goto again;
			case ZCOMMAND:
				if (zrdata(secbuf, 1024) == GOTCRCW)
				{
					stohdr(-1L);
					purgeline();	/* dump impatient questions */
					while (errors < 20)
					{
						zshhdr(ZCOMPL, Txhdr);
						if (zgethdr(Rxhdr) == ZFIN)
							break;
					}
					send_ZFIN();
					return (ZCOMPL);
				}
				zshhdr(ZNAK, Txhdr);
				goto again;
			case ZCOMPL:
				goto again;
			default:
				continue;
			case ZFIN:
				send_ZFIN();
				return (ZCOMPL);
			case ZCAN:
				return (ERROR);
		}
	}
	return (0);
}							 /* end of tryz */

/*+-------------------------------------------------------------------------
	rzfile() - receive a file with ZMODEM protocol

  assumes file name frame is in secbuf
--------------------------------------------------------------------------*/
int
rzfile()
{
	register c, n;
	char s64[64];

	Eofseen = 0;
	rxfilepos = 0L;
	if (procheader(secbuf) == ERROR)
	{
		return (tryzhdrtype = ZSKIP);
	}

	n = 20;

	for (;;)
	{
		if (rxfilepos && !expect_zrpos)
		{
			sprintf(s64, "Sending ZRPOS (%ld)", rxfilepos);
			report_str(s64, 1);
		}
		else
			report_str("Starting sender", 0);
		expect_zrpos = 0;
		stohdr(rxfilepos);
		zshhdr(ZRPOS, Txhdr);
	  nxthdr:
		report_receive_progress(rxfilepos);
		switch (c = zgethdr(Rxhdr))
		{
			default:
				sprintf(s128, got_garbage_txt, c);
				report_str(s128, 0);
				return (ERROR);
			case ZNAK:
			case TIMEOUT:
				if (--n < 0)
				{
					sprintf(s128, got_garbage_txt, c);
					report_str(s128, 0);
					return (ERROR);
				}
			case ZFILE:
				zrdata(secbuf, 1024);
				continue;
			case ZEOF:
				if (rclhdr(Rxhdr) != rxfilepos)
				{

					/*
					 * Ignore eof if it's at wrong place - force a timeout
					 * because the eof might have gone out before we sent
					 * our zrpos.
					 */
					errors = 0;
					goto nxthdr;
				}
				if (can_on_eof)
				{
					send_cancel(0);
					send_cancel(0);
					close_and_report();
					report_uninit();
					exit(0);
				}
				if (close_and_report())
				{
					tryzhdrtype = ZFERR;
					return (ERROR);
				}
				report_str("End of file", 0);
				return (c);
			case ERROR:	 /* Too much garbage in header search error */
				if (--n < 0)
				{
					sprintf(s128, got_garbage_txt, c);
					report_str(s128, 0);
					return (ERROR);
				}
				zmputs(Attn);
				continue;
			case ZSKIP:
				close_and_report();
				sprintf(s128, "Sender SKIPPED file");
				report_str(s128, -1);
				return (c);
			case ZDATA:
				if (rclhdr(Rxhdr) != rxfilepos)
				{
					if (--n < 0)
					{
						return (ERROR);
					}
					zmputs(Attn);
					continue;
				}
			  moredata:
				report_receive_progress(rxfilepos);
				switch (c = zrdata(secbuf, 1024))
				{
					case ZCAN:
						sprintf(s128, got_garbage_txt, c);
						report_str(s128, 0);
						return (ERROR);
					case ERROR:	/* CRC error */
						if (--n < 0)
						{
							sprintf(s128, got_garbage_txt, c);
							report_str(s128, 0);
							return (ERROR);
						}
						zmputs(Attn);
						continue;
					case TIMEOUT:
						if (--n < 0)
						{
							sprintf(s128, got_garbage_txt, c);
							report_str(s128, 0);
							return (ERROR);
						}
						continue;
					case GOTCRCW:
						n = 20;
						write_sec_to_disk(secbuf, Rxcount);
						rxfilepos += Rxcount;
						stohdr(rxfilepos);
						zshhdr(ZACK, Txhdr);
						sendline(XON);
						report_str("", -1);
						goto nxthdr;
					case GOTCRCQ:
						n = 20;
						write_sec_to_disk(secbuf, Rxcount);
						rxfilepos += Rxcount;
						stohdr(rxfilepos);
						zshhdr(ZACK, Txhdr);
						report_str("", -1);
						goto moredata;
					case GOTCRCG:
						n = 20;
						write_sec_to_disk(secbuf, Rxcount);
						rxfilepos += Rxcount;
						goto moredata;
					case GOTCRCE:
						n = 20;
						write_sec_to_disk(secbuf, Rxcount);
						rxfilepos += Rxcount;
						goto nxthdr;
				}
		}
	}
	/* NOTREACHED */

}							 /* end of rzfile */

/*+-------------------------------------------------------------------------
	rzfiles() - receive file(s) with ZMODEM protocol
--------------------------------------------------------------------------*/
int
rzfiles()
{
	register c;

	for (;;)
	{
		switch (c = rzfile())
		{
			case ZEOF:
			case ZSKIP:
				switch (tryz())
				{
					case ZCOMPL:
						return (OK);
					default:
						return (ERROR);
					case ZFILE:
						break;
				}
				continue;
			default:
				return (c);
			case ERROR:
				return (ERROR);
		}
	}
	/* NOTREACHED */
}							 /* end of rzfiles */

/*+-------------------------------------------------------------------------
	close_and_report() - close the received file, set mod time and chmod
(specifically exclude set uid and gid from chmod)
--------------------------------------------------------------------------*/
int
close_and_report()
{
	fflush(fout);
	fstat(fileno(fout), &fout_stat);
	report_file_byte_io((long)fout_stat.st_size - initial_filepos);

	report_file_close(0);
	if (fclose(fout) == ERROR)
	{
#if defined(CFG_LogXfer)
		vlogevent(getppid(), "finish close error: %s", strerror(errno));
#endif
		fout = (FILE *) 0;
		return (ERROR);
	}

#if defined(CFG_LogXfer)
	vlogevent(getppid(), "RECEIVE success: %s (%ld bytes)",
		Pathname, (long)fout_stat.st_size);
#endif

	if (Modtime)
	{
		timep[0] = time(NULL);
		timep[1] = Modtime;
		utime(Pathname, timep);
	}

	if ((Filemode & S_IFMT) == S_IFREG)
	{
		Filemode &= ~(S_ISUID | S_ISGID);
		chmod(Pathname, (unsigned short)(07777 & Filemode));
	}

	return (OK);

}							 /* end of close_and_report */

/*+-------------------------------------------------------------------------
	sys2(shellcmd) - execute shell command

 Strip leading ! if present
--------------------------------------------------------------------------*/
int
sys2(shellcmd)
register char *shellcmd;
{
	if (*shellcmd == '!')
		++shellcmd;
	return (system(shellcmd));
}							 /* end of sys2 */

/*+-------------------------------------------------------------------------
	main(argc,argv)
--------------------------------------------------------------------------*/
int
main(argc, argv)
int argc;
char **argv;
{
	register char *cp;
	char **patts = (char **)0;
	char *getenv();
	int exitcode = 0;

	gargv = argv;
	gargc = argc;
#ifdef __FreeBSD__
	setlocale(LC_ALL, "");
#endif

	signal(SIGINT, bye_bye);
	signal(SIGTERM, bye_bye);
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

	get_curr_dir(curr_dir, sizeof(curr_dir));

	Rxtimeout = 100;

	npats = 0;
	while (--argc)
	{
		cp = *++argv;
		if (*cp == '-')
		{
			while (*++cp)
			{
				switch (*cp)
				{
					case 'X':
						required_type = 1;
						Batch = 0;
						break;
					case 'Y':
						required_type = 1;
						Nozmodem = 1;
						Batch = 1;
						break;
					case 'Z':
						required_type = 1;
						Nozmodem = 0;
						Batch = 1;
						break;
					case '+':
						Lzmanag = ZMAPND;
						break;
					case 'a':
						Rxascii = 1;
						break;
					case 'b':
						Rxbinary = 1;
						break;
					case 'c':
						Crcflg = 1;
						break;
					case 'e':
						Zctlesc = 1;
						break;
					case 'T':
						telnet = 1;
						break;
					case 'p':
						Lzmanag = ZMPROT;
						break;
					case '@':
						if (--argc < 1)
							usage("no bitrate after -@");
						Bitrate = atoi(*++argv);
						break;
					case '_':
						force_dumbtty = 1;
						break;
					case ',':
						log_packets = 1;
						break;
					case ':':
						can_on_eof = 1;
						break;
					case '.':
						if (--argc < 1)
						{
							usage("no iofd after -.");
						}
						iofd = atoi(*++argv);
						break;
					case 't':
						if (--argc < 1)
						{
							usage("no rcvr timeout after -t");
						}
						Rxtimeout = atoi(*++argv);
						if (Rxtimeout < 10 || Rxtimeout > 1000)
							usage("illegal timeout: must be 10 <= t <= 1000");
						break;
					case 'w':
						if (--argc < 1)
						{
							usage("no Zrwindow after -w");
						}
						Zrwindow = atoi(*++argv);
						break;
					case 'C':
						if (--argc < 1)
							usage("no label after -C");
						bottom_label = *++argv;
						break;
					case 'u':
						MakeLCPathname = 0;
						break;
					case 'y':
						Rxclob = 1;
						break;
					default:
						sprintf(s128, "Unknown switch -%c", *cp);
						usage(s128);
				}
			}
		}
		else if (!npats && argc > 0)
		{
			if (argv[0][0])
			{
				npats = argc;
				patts = argv;
			}
		}
	}

	if (determine_output_mode())
	{
		setbuf(stdout, NULL);
		setbuf(stderr, NULL);
	}

	zputc_init();			 /* choose zputc mode */

	if (!required_type || !iofd)
	{
		printf("can only be run by ecu\n");
		exit(255);
	}

	if (log_packets)
	{
		char log_packets_name[64];
		int iargv;

		sprintf(log_packets_name, "/tmp/rz%05d.plog", getpid());
		unlink(log_packets_name);
		log_packets = open(log_packets_name, O_CREAT | O_WRONLY, 0644);
		if (log_packets < 0)
			log_packets = 0;
		else
		{
			write(log_packets, "exec: ", 6);
			for (iargv = 0; iargv < gargc; iargv++)
			{
				write(log_packets, gargv[iargv], strlen(gargv[iargv]));
				write(log_packets, " ", 1);
			}
			write(log_packets, "\n", 1);
		}
	}

	/*
	 * learn tick rate for various timers
	 */
	init_Nap();

	if (Batch && npats)
		usage("Cannot specify batch receive and filename");
	if (npats > 1)
		usage("only one filename allowed");
	report_init(version + 4);
	mode(1);
	signal(SIGINT, cancel_transaction);
	signal(SIGTERM, cancel_transaction);
	signal(SIGQUIT, cancel_transaction);
	if (wcreceive(npats, patts) == ERROR)
	{
		exitcode = 0200;
		send_cancel(1);
	}
	mode(0);
	if (exitcode && !Zmodem) /* bellow again with all thy might. */
		send_cancel(1);
	report_uninit(0);
	exit(exitcode);
	return (0);				 /* never get here, but keep gcc optim from
							  * complaining */
	/* NOTREACHED */
}							 /* end of main */

/* vi: set tabstop=4 shiftwidth=4: */
/* end of ecurz.c */
