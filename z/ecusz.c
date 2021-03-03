char *version = "@(#)ecusz 3.38";

#define _XOPEN_SOURCE
#define BUFFERED_WRITE
/*+-------------------------------------------------------------------------
	ecusz.c - X/Y/ZMODEM send program
  Derived from public domain source by Chuck Forsberg, Omen Technologies
  Adaptation for ecu 1989 wht@wht.net

	Usage:	ecusz [-X -Y -Z] [-12+abdefkLlNnquvwy] [-] file ...
		(Y) = Option applies to YMODEM only
		(Z) = Option applies to ZMODEM only
		a (ASCII) change NL to CR/LF
		b Binary file transfer override
		f send Full pathname (Y/Z)
		k Send 1024 byte packets (Y)
		L N Limit subpacket length to N bytes (Z)
		l N Limit frame length to N bytes (l>=L) (Z)
		n send file if source newer (Z)
		N send file if source newer or longer (Z)
		o Use 16 bit CRC instead of 32 bit CRC (Z)
		p Protect existing destination file (Z)
		r Resume/Recover interrupted file transfer (Z)
		q Quiet (no progress reports)
		u Unlink file after transmission
		w N Window is N bytes (Z)
		y Yes,overwrite existing file (Z)
		@file reads a list of filenames from 'file'

  Defined functions:
	SIGALRM_handler()
	bye_bye(sig)
	cancel_transaction(sig)
	determine_transaction_time()
	flushline()
	get_file_list_name(namep)
	getinsync(flag)
	getnak()
	getzrxinit()
	log_packet_buffer(buf, len)
	main(argc, argv)
	onintr()
	purgeline()
	readline(timeout)
	readock(timeout, count)
	report_rcvr_cancelled(place_happened)
	report_rcvr_skipped()
	report_send_progress(filepos)
	report_send_transaction()
	rewind_file_list()
	saybibi()
	send_cancel(error)
	sendline(ch)
	sendzsinit()
	set_file_list(pathc, pathv)
	substr(str, str2)
	usage()
	wcputsec(buf, sectnum, cseclen)
	wcs(oname)
	wcsend()
	wctx(flen)
	wctxpn(name)
	xbuf_build(buf, count)
	xputc(ch)
	zbuf_build(buf, count)
	zsendfdata()
	zsendfile(buf, blen)

--------------------------------------------------------------------------*/
/*+:EDITS:*/
/*:01-24-1997-02:38-wht@yuriatin-SOURCE RELEASE 4.00 */
/*:09-11-1996-20:01-wht@yuriatin-3.48-major telnet,curses,structural overhaul */
/*:09-06-1996-03:18-wht@kepler-cleanup */
/*:08-20-1996-12:39-wht@kepler-locale/ctype fixes from ache@nagual.ru */
/*:08-11-1996-02:10-wht@kepler-rename ecu_log_event to logevent */
/*:01-26-1996-05:08-wht@kepler-always READCHECK */
/*:12-28-1995-12:54-wht@kepler-Andrey Chernov FreeBSD fixes */
/*:11-27-1995-20:42-wht@kepler-add zputc_init */
/*:11-14-1995-10:23-wht@kepler-3.37.80-source control point: SOCKETS */
/*:11-03-1995-15:52-wht@wwtp1-uns char * vs. char * corrections */
/*:10-14-1995-23:46-wht@kepler-switch -@ passes line bitrate value */
/*:10-14-1995-17:08-wht@kepler-'struct termio' to 'struct TERMIO' */
/*:05-13-1995-11:43-wht@n4hgf-apply Andrey BSD 4.4 st_size patch */
/*:05-09-1995-17:08-wht@kepler-no more use of VOLATILE */
/*:03-12-1995-02:41-wht@kepler-flushline reorganization around TIOCOUTQ */
/*:01-15-1995-01:52-wht@n4hgf-clean up creeping port rot */
/*:01-15-1995-00:51-wht@n4hgf-unsigned & signed ptr conflicts w/FreeBSD gcc */
/*:01-12-1995-15:20-wht@n4hgf-apply Andrew Chernov 8-bit clean+FreeBSD patch */
/*:05-04-1994-04:40-wht@n4hgf-ECU release 3.30 */
/*:04-01-1994-20:56-wht@n4hgf-pedantic what-compatible version */
/*:02-16-1993-13:14-wht@n4hgf-make Bitrate long for 286 */
/*:09-10-1992-14:00-wht@n4hgf-ECU release 3.20 */
/*:09-05-1992-14:26-wht@n4hgf-zrpos_seen was not set 1 on first ZRPOS */
/*:08-22-1992-15:39-wht@n4hgf-ECU release 3.20 BETA */
/*:08-16-1992-03:08-wht@n4hgf-head off another POSIX plot */
/*:08-10-1992-04:01-wht@n4hgf-use init_Nap */
/*:07-20-1992-13:39-wht@n4hgf-need hzmsec for nap.c */
/*:09-01-1991-14:18-wht@n4hgf2-improve sun flushline */
/*:08-29-1991-02:17-wht@n4hgf2-flush "rz" to line before nap */
/*:08-28-1991-14:08-wht@n4hgf2-SVR4 cleanup by aega84!lh */
/*:07-25-1991-12:59-wht@n4hgf-ECU release 3.10 */
/*:02-03-1991-17:27-wht@n4hgf-version number change - see zcurses.c */
/*:12-18-1990-21:26-wht@n4hgf-better output control */
/*:09-19-1990-19:36-wht@n4hgf-logevent now gets pid for log from caller */
/*:08-14-1990-20:40-wht@n4hgf-ecu3.00-flush old edit history */

/*
  Error return conditions
	255:     usage
	254:     protocol failed (bad line conditions,brain dead remote)
	253:     curses problem
	253:     could not open any files
	128-192: process terminated with signal==code-128 (64 signals allowed for)
             signal 0 == program logic error (see cancel_transaction)
	127:     127 or more files not transmitted (see ~/.ecu/log)
	1-126:   count of files not transmitted (see ~/.ecu/log)
	0:       file transfer completely successful
*/

char *substr();
char *getenv();

#include <stdio.h>
#include <signal.h>
#include <setjmp.h>
#include <ctype.h>
#include <string.h>
#include <fcntl.h>
#include "../ecu_config.h"
#ifdef __FreeBSD__
#include <locale.h>
#endif
#include "zmodem.h"
#include <sys/param.h>
#include <errno.h>

extern unsigned short crctab[];	/* wht */
extern unsigned long total_data_bytes_xfered;	/* zcurses.c */
//extern int errno;
extern int show_window;
extern int Rxtimeout;		 /* Tenths of seconds to wait for something */
extern char Rxhdr[4];		 /* Received header */
extern char Txhdr[4];		 /* Transmitted header */
extern int Txfcs32;			 /* TURE means send binary frames with 32 bit
							  * FCS */
extern long Rxpos;			 /* Received file position */
extern long Txpos;			 /* Transmitted file position */
extern char *frametypes[];
extern char Attn[];			 /* Attention string rx sends to tx on err */
extern char s128[128];

#define RETRYMAX 10			 /* non-zmodem retry count on block send */
#define VMIN_COUNT 2		 /* must not exceed 255 */
unsigned char vmin_count = VMIN_COUNT;
int iofd = 0;				 /* line io fd */
#ifdef BUFFERED_WRITE
FILE *iofp;

#endif

int telnet;

char Myattn[] =
{0};

FILE *in;

char *Cmdstr;				 /* Pointer to the command string */
char *bottom_label = (char *)0;
char Crcflg;
char Lastrx;
char Lzconv;				 /* Local ZMODEM file conversion request */
char Lzmanag;				 /* Local ZMODEM file management request */
char Lztrans;
char Pathname[PATHLEN];
char curr_dir[1024];
unsigned char txbuf[1024];
char zconv;					 /* ZMODEM file conversion request */
char zmanag;				 /* ZMODEM file management request */
char ztrans;				 /* ZMODEM file transport request */
int Ascii;					 /* Add CR's for brain damaged programs */
int Cmdack1;				 /* Rx ACKs command,then do it */
int Cmdtries = 11;
int Command;				 /* Send a command,then exit. */
int Dontread;				 /* Don't read the buffer,it's still there */
int Dottoslash;				 /* Change foo.bar.baz to foo/bar/baz */
int Exitcode;
int Filcnt;					 /* count of number of files opened */
int FilesTotal;
int Filesleft;
int Fullname;				 /* transmit full pathname */
int Lastn;					 /* Count of last buffer read or -1 */
int Lfseen;
int Noeofseen;
int Nozmodem;
int Optiong;				 /* no wait for block ACK's */
int Quiet;					 /* overrides logic that would otherwise set
							  * verbose */
int Rxflags;
int SameZrposAgain;			 /* How many times we've been ZRPOS'd same
							  * place (wht) */
unsigned Tframlen;			 /* Override for tx frame length */
int Totsecs;				 /* total number of blocks this file */
int Twostop;				 /* use two stop bits */
int Unlinkafter;			 /* Unlink file after it is sent */
int Wantfcs32 = TRUE;		 /* want to send 32 bit FCS */
int Xmodem;					 /* XMODEM Protocol - don't send pathnames */
int Zctlesc;				 /* Encode control characters */
int Zmodem;					 /* ZMODEM protocol requested by receiver */
int Zrwindow = 1400;		 /* RX window size (controls garbage count) */
unsigned blklen = 128;		 /* length of transmitted records */
int blklen_original;
unsigned blkopt;			 /* Override value for zmodem blklen */
int ecusz_flag = 1;
int force_dumbtty;
int got_xfer_type;
int seen_zrpos;
int skip_count;				 /* skipped files */
int errors;
int firstsec;
int log_packets;
int no_files;
int npats;
long Lastread;				 /* Beginning offset of last buffer read */
long Lastsync;				 /* Last offset to which we got a ZRPOS */
long Lrxpos;				 /* Receiver's last reported offset */
long TotalLeft;
long TotalToSend;
long bytcnt;
long rx_char_count;
long this_file_length;
long tx_char_count;
long initial_filepos;		 /* initial file position */
unsigned long Bitrate;
unsigned Rxbuflen = 16384;	 /* Receiver's max buffer length */
unsigned Txwcnt;			 /* Counter used to space ack requests */
unsigned Txwindow;			 /* Control the size of the transmitted window */
unsigned Txwspac;			 /* Spacing between zcrcq requests */
unsigned int bad_condx_blklen;	/* if !=0,blklen has been reduced (wht) */
unsigned int bad_condx_frame_count;	/* frame # last SameZrposAgain (wht) */
unsigned int this_file_frame_count;	/* count of frames sent this file
									 * (wht) */

#define MAX_PATHS 512
char *paths[MAX_PATHS];

jmp_buf tohere;				 /* For the interrupt on RX timeout */
jmp_buf intrjmp;			 /* For the interrupt on RX CAN */

int file_list_pathc;
int file_list_path_current;
char **file_list_pathv;
FILE *fpflst = (FILE *) 0;

void send_cancel();
void purgeline();
void usage();
void saybibi();
void determine_transaction_time();
void sendline();
void xputc();

/*+-------------------------------------------------------------------------
	log_packet_buffer(buf,len)
--------------------------------------------------------------------------*/
void
log_packet_buffer(buf, len)
register unsigned char *buf;
register int len;
{
	char xbuf[32];

	while (len--)
	{
		sprintf(xbuf, "%02x ", *buf++);
		write(log_packets, xbuf, strlen(xbuf));
	}
	write(log_packets, "\n", 1);

}							 /* end of log_packet_buffer */

/*+-------------------------------------------------------------------------
	rewind_file_list()
--------------------------------------------------------------------------*/
void
rewind_file_list()
{
	file_list_path_current = 0;
	if (fpflst)
	{
		fclose(fpflst);
		fpflst = (FILE *) 0;
	}
}							 /* end of rewind_file_list */

/*+-------------------------------------------------------------------------
	set_file_list(pathc,pathv)
--------------------------------------------------------------------------*/
void
set_file_list(pathc, pathv)
int pathc;
char **pathv;
{
	file_list_pathc = pathc;
	file_list_pathv = pathv;
	rewind_file_list();

}							 /* end of set_file_list */

/*+-------------------------------------------------------------------------
	get_file_list_name(namep)
return 1 if @lst found else 0
--------------------------------------------------------------------------*/
int
get_file_list_name(namep)
char **namep;
{
	register char *cptr;
	static char name[256];

  try_fpflst:
	if (fpflst)
	{
		if (fgets(name, sizeof(name), fpflst) != NULL)
		{
			name[strlen(name) - 1] = 0;
			*namep = name;
			return (1);
		}
		fclose(fpflst);
		fpflst = (FILE *) 0;
	}

  next_arg:
	if (file_list_path_current == file_list_pathc)
		return (0);
	cptr = file_list_pathv[file_list_path_current++];
	if (*cptr != '@')
	{
		*namep = cptr;
		return (1);
	}
	cptr++;
	if ((fpflst = fopen(cptr, "r")) == NULL)
		goto next_arg;
	goto try_fpflst;

}							 /* end of get_file_list_name */

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
	cancel_transaction(sig)
called by signal interrupt or terminate to clean things up
--------------------------------------------------------------------------*/
void
cancel_transaction(sig)
int sig;
{
	if (Zmodem)
		zmputs(Attn);
	send_cancel(1);
	mode(0);
	if (sig >= 0)
	{
		sprintf(s128, "ecusz aborted (signal %d)", sig);
		report_str(s128, 0);
	}
	report_uninit();
	bye_bye(sig);
}							 /* end of cancel_transaction */

/*+-------------------------------------------------------------------------
	onintr() - Called when ZMODEM gets an interrupt (^X)
--------------------------------------------------------------------------*/
onintr()
{
	signal(SIGINT, SIG_IGN);
	report_rx_ind(0);
	report_tx_ind(0);
	longjmp(intrjmp, -1);
}							 /* end of onintr */

/*+-------------------------------------------------------------------------
	report_send_transaction()
--------------------------------------------------------------------------*/
void
report_send_transaction()
{
	if (Xmodem)
	{
		long blocks = (TotalToSend >> 7) + ((TotalToSend % 128) != 0);
		long secs = 7		 /* slightly worse than average first nak
							  * delay */
		+ (blocks / 5L)		 /* assume .2 sec ack time */
		+ ((blocks * (128L + 16L)) / (Bitrate / 10));
		if (!secs)
			secs = 10L;
		sprintf(s128, "Sending %ld blocks time ~= %ld:%02ld",
			blocks, secs / 60, secs % 60);
	}
	else
	{
		long min_100 =
		(FilesTotal * 2L) + (((TotalToSend * 11L)) * 10L) / (Bitrate * 6L);

		if (!min_100)
			min_100 = 4L;
#if defined(M_I286)			 /* slower */
		else if (Bitrate > 4800)
		{
			min_100 *= 13;
			min_100 /= 9;	 /* yech ... empirical */
		}
#endif
		sprintf(s128,
			"Total transaction %ld bytes (xfer time ~= %2lu:%02lu)",
			TotalToSend, min_100 / 100, ((min_100 % 100) * 60L) / 100L);
	}
	report_transaction(s128);

}							 /* end of report_send_transaction */

/*+-------------------------------------------------------------------------
	report_send_progress(filepos)
--------------------------------------------------------------------------*/
void
report_send_progress(filepos)
long filepos;
{

	if (Xmodem)
	{
		sprintf(s128, "File %d%% complete",
			(this_file_length == 0) ? (int)100 :
			(int)((filepos * 100L) / this_file_length));
	}
	else
	{
		sprintf(s128, "This file %d%%, transaction %d%% complete",
			(this_file_length == 0) ? (int)100 :
			(int)((filepos * 100L) / this_file_length),
			(TotalToSend == 0) ? (int)100 :
			(int)(((total_data_bytes_xfered + filepos) * 100L)
				/ TotalToSend));
	}
	report_str(s128, 0);
	report_txpos(filepos);

}							 /* end of report_send_progress */

/*+-------------------------------------------------------------------------
	report_rcvr_cancelled(place_happened)
--------------------------------------------------------------------------*/
void
report_rcvr_cancelled(place_happened)
char *place_happened;
{
	strcpy(s128, "SEND CANCELLED");
	report_str(s128 + 5, 1);
#if defined(CFG_LogXfer)
	strcat(s128, " (");
	strcat(s128, place_happened);
	strcat(s128, ")");
	logevent(getppid(), s128);
#endif
	skip_count++;
	report_error_count();
}							 /* end of report_rcvr_cancelled */

/*+-------------------------------------------------------------------------
	report_rcvr_skipped()
--------------------------------------------------------------------------*/
void
report_rcvr_skipped()
{
	sprintf(s128, "SEND skipped: %s", Pathname);
	report_str(s128 + 5, -1);
#if defined(LOG_SKIP) && defined(CFG_LogXfer)
	logevent(getppid(), s128);
#endif
	skip_count++;
	report_error_count();
	TotalToSend -= this_file_length;
	report_send_transaction();
}							 /* end of report_rcvr_skipped */

/*+-------------------------------------------------------------------------
	xputc(ch)
--------------------------------------------------------------------------*/
void
xputc(ch)
char ch;
{
#ifdef BUFFERED_WRITE
	fputc(ch, iofp);
#else
	write(iofd, &ch, 1);
#endif
	++tx_char_count;
}							 /* end of xputc */

/*+-------------------------------------------------------------------------
	sendline(ch)
--------------------------------------------------------------------------*/
void
sendline(ch)
char ch;
{
	xputc(ch);
}							 /* end of sendline */

/*+-------------------------------------------------------------------------
	flushline() - ensure all queued data to line is on the wire
--------------------------------------------------------------------------*/
void
flushline()
{

#ifdef BUFFERED_WRITE
	fflush(iofp);
#endif

	if (telnet)
		return;

#if defined(CFG_TermiosLineio)
#if defined(TIOCOUTQ)
	{
		int retries = 50;
		int outq_count;
		int old_outq_count = 0;

		do
		{
			ioctl(iofd, TIOCOUTQ, &outq_count);
			if (!outq_count)
				break;
			if (old_outq_count == outq_count)	/* don't hang if flow
												 * control lock */
				retries--;
			old_outq_count = outq_count;
			Nap(50);
		}
		while (outq_count && retries);
	}
#else /* CFG_TermiosLineio with tcdrain() */
	{
		tcdrain(iofd);
	}
#endif
#else /* NOT CFG_TermiosLineio */
	{
		struct TERMIO tio;

		ecugetattr(iofd, &tio);
		ecusetattr(iofd, TCSETAW, &tio);
	}
#endif
}							 /* end of flushline */

/*+-------------------------------------------------------------------------
	main(argc,argv)
--------------------------------------------------------------------------*/
int
main(argc, argv)
int argc;
char **argv;
{
	register char *cp;
	char **patts = paths;
	char **gargv = argv;
	int gargc = argc;

#ifdef __FreeBSD__
	setlocale(LC_ALL, "");
#endif
	signal(SIGINT, bye_bye);
	signal(SIGTERM, bye_bye);

	/*
	 * call Roto-Rooter on POSIX plots
	 */
#if	defined(SIGSTOP)
	signal(SIGSTOP, SIG_IGN);
	signal(SIGTSTP, SIG_IGN);
	signal(SIGCONT, SIG_IGN);
	signal(SIGTTIN, SIG_IGN);
	signal(SIGTTOU, SIG_IGN);
#endif

	get_curr_dir(curr_dir, sizeof(curr_dir));

	Rxtimeout = 600;
	npats = 0;
	if (argc < 2)
		usage();
	while (--argc)
	{
		cp = *++argv;
		if (*cp == '-')
		{
			cp++;
			switch (*cp++)
			{
				case 'X':
					got_xfer_type = 1;
					Xmodem = TRUE;
					break;
				case 'Y':
					got_xfer_type = 1;
					Nozmodem = TRUE;
					blklen = 1024;
					break;
				case 'Z':
					show_window = 1;
					got_xfer_type = 1;
					break;

				case '+':
					Lzmanag = ZMAPND;
					break;
				case 'a':
					Lzconv = ZCNL;
					Ascii = TRUE;
					break;
				case 'b':
					Lzconv = ZCBIN;
					break;
				case 'd':
					++Dottoslash;
					/* **** FALL THROUGH TO **** */
				case 'f':
					Fullname = TRUE;
					break;
				case ',':
					log_packets = 1;
					break;
				case '@':
					if (--argc < 1)
						usage("no bitrate after -@");
					Bitrate = atoi(*++argv);
					break;
				case '_':
					force_dumbtty = 1;
					break;
				case '/':
					if (--argc < 1)
						usage();
					strcpy(curr_dir, *++argv);
					break;
				case '.':
					if (--argc < 1)
						usage();
					iofd = atoi(*++argv);
					break;
				case 'C':
					if (--argc < 1)
						usage("no label after -C");
					bottom_label = *++argv;
					break;
				case 'e':
					Zctlesc = 1;
					break;
				case 'T':
					telnet = 1;
					break;
				case 'k':
					blklen = 1024;
					break;
				case 'L':
					if (--argc < 1)
					{
						usage();
					}
					blkopt = atoi(*++argv);
					if (blkopt < 24 || blkopt > 1024)
						usage();
					break;
				case 'l':
					if (--argc < 1)
					{
						usage();
					}
					Tframlen = atoi(*++argv);
					if (Tframlen < 32 || Tframlen > 1024)
						usage();
					break;
				case 'N':
					Lzmanag = ZMNEWL;
					break;
				case 'n':
					Lzmanag = ZMNEW;
					break;
				case 'o':
					Wantfcs32 = FALSE;
					break;
				case 'p':
					Lzmanag = ZMPROT;
					break;
				case 'r':
					Lzconv = ZCRESUM;
					break;
				case 't':
					if (--argc < 1)
					{
						usage();
					}
					Rxtimeout = atoi(*++argv);
					if (Rxtimeout < 10 || Rxtimeout > 1000)
						usage();
					break;
				case 'u':
					++Unlinkafter;
					break;
				case 'w':
					if (--argc < 1)
					{
						usage();
					}
					Txwindow = atoi(*++argv);
					if (Txwindow < 256)
						Txwindow = 256;
					Txwindow = (Txwindow / 64) * 64;
					Txwspac = Txwindow / 4;
					if ((blkopt > Txwspac) || (!blkopt && Txwspac < 1024))
						blkopt = Txwspac;
					break;
				case 'y':
					Lzmanag = ZMCLOB;
					break;
				default:
					usage();
			}
		}
		else if (argc > 0)
		{
			if (npats < MAX_PATHS)
			{
				npats++;
				*patts++ = cp;
			}
			else
			{
				printf("too many filenames to send\n");
				exit(255);
			}
		}
	}
	if (!got_xfer_type || !iofd)
	{
		printf("can only be run by ecu\n");
		exit(255);
	}

	if (determine_output_mode())
	{
		setbuf(stdout, NULL);
		setbuf(stderr, NULL);
	}

	if (npats < 1 && !Command)
		usage();

	set_file_list(npats, paths);

	zputc_init();			 /* choose zputc mode */

	if (log_packets)
	{
		char log_packets_name[64];
		FILE *ftmp;
		int iargv;

		sprintf(log_packets_name, "/tmp/sz%05d.plog", getpid());
		unlink(log_packets_name);
		ftmp = fopen(log_packets_name, "w");
		fclose(ftmp);
		log_packets = open(log_packets_name, O_WRONLY, 0644);
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

	report_init(version + 4);
	mode(1);

	if (signal(SIGINT, cancel_transaction) == SIG_IGN)
		signal(SIGINT, SIG_IGN);
	else
		signal(SIGINT, cancel_transaction);
	signal(SIGTERM, cancel_transaction);

	report_str("calculating transaction time", -1);
	determine_transaction_time();
#ifdef BUFFERED_WRITE
	iofp = fdopen(iofd, "w");
#endif
	if (!Xmodem)
	{
		TotalToSend = TotalLeft;
		report_send_transaction();
		report_str("starting remote receiver", -1);
		report_last_txhdr("begin transfer", 0);
		if (!Nozmodem)
			write(iofd, "rz\r", 3);
		else				 /* wht -- why not? */
			write(iofd, "rb\r", 3);	/* wht */
		flushline();
		Nap(750L);
		report_str("", -1);
		if (!Nozmodem)
		{
			stohdr(0L);
			zshhdr(ZRQINIT, Txhdr);
		}
	}
	else
	{
		report_str("", -1);
		report_last_txhdr("begin transfer", 0);
	}

	if (wcsend() == ERROR)
	{
		Exitcode = 254;		 /* wht was 0200 */
		send_cancel(1);
	}
	mode(0);
	report_uninit();
	if (no_files)
		Exitcode = 253;
	exit(Exitcode ? Exitcode : (skip_count > 127) ? 127 : skip_count);
	return (0);				 /* never get here, but keep gcc optim from
							  * complaining */
	/* NOTREACHED */
}							 /* end of main */

/*+-------------------------------------------------------------------------
	wcsend(argc,argp) -- send group of files
--------------------------------------------------------------------------*/
int
wcsend()
{
	char *name;

	Crcflg = FALSE;
	firstsec = TRUE;
	bytcnt = -1;
	rewind_file_list();
	while (get_file_list_name(&name))
	{
		Totsecs = 0;
		if (wcs(name) == ERROR)
			return (ERROR);
	}
	Totsecs = 0;
	if (Filcnt == 0)
	{						 /* bitch if we couldn't open ANY files */
		send_cancel(1);
		strcpy(s128, "SEND cannot open any requested files");
		report_str(s128 + 5, 1);
#if defined(CFG_LogXfer)
		logevent(getppid(), s128);
#endif
		sleep(2);			 /* allow time for other rz to get ready */
		no_files = 1;
		return (ERROR);		 /* ... then cancel */
	}
	if (Zmodem)
		saybibi();
	else if (!Xmodem)
		wctxpn("");
	return (OK);

}

/*+-------------------------------------------------------------------------
	wcs(oname) -- send a file
--------------------------------------------------------------------------*/
int
wcs(oname)
char *oname;
{
	register c;
	struct stat f;

	strcpy(Pathname, oname); /* global copy of name */

	if ((in = fopen(oname, "r")) == NULL)
	{
		sprintf(s128, "SEND %s: %s", oname, strerror(errno));
#if defined(CFG_LogXfer)
		logevent(getppid(), s128);
#endif
		report_str(s128 + 5, 1);
		skip_count++;
		report_error_count();
		return (OK);		 /* pass over it,there may be others */
	}
	seen_zrpos = 0;
	++Noeofseen;
	Lastread = 0;
	Lastn = -1;
	Dontread = FALSE;
	/* Check for directory or block special files */
	fstat(fileno(in), &f);
	c = f.st_mode & S_IFMT;
	if (c == S_IFDIR || c == S_IFBLK)
	{
		sprintf(s128, "SEND %s: %s",
			(c == S_IFDIR) ? "directory" : "block device", oname);
		report_str(s128 + 5, 1);
#if defined(LOG_SKIP) && defined(CFG_LogXfer)
		logevent(getppid(), s128);
#endif
		skip_count++;
		report_error_count();
		fclose(in);
		return (OK);
	}
	f.st_mode &= ~(S_ISUID | S_ISGID);
	Filcnt++;
	report_file_send_open(oname, &f);
	this_file_length = f.st_size;
	report_send_progress(0L);
	switch (wctxpn(Pathname))
	{
		case ERROR:
			sprintf(s128, "SEND protocol failure: %s", oname);
			report_str(s128 + 5, 1);
#if defined(CFG_LogXfer)
			logevent(getppid(), s128);
#endif
			skip_count++;
			report_error_count();
			report_file_close(2);
			fclose(in);
			return (ERROR);
		case ZSKIP:
			report_rcvr_skipped();
			return (OK);
	}
	if (!Zmodem && wctx((long)f.st_size) == ERROR)
		return (ERROR);
	if (Unlinkafter)
		unlink(oname);
	return (0);
}

/*+-------------------------------------------------------------------------
	wctxpn(name)

generate and transmit pathname block consisting of pathname (null
terminated), file length,mode time and file mode in octal as
provided by the Unix fstat call.  N.B.: modifies the passed
name,may extend it!
--------------------------------------------------------------------------*/
int
wctxpn(name)
char *name;
{
	register char *p, *q;
	char name2[PATHLEN];
	struct stat f;

	if (Xmodem)
	{
		if ((in != stdin) && *name && fstat(fileno(in), &f) != -1)
		{
			TotalToSend = f.st_size;
			report_protocol_type("XMODEM");
			report_send_transaction();
			report_xfer_mode((Ascii) ? "ASCII" : "BINARY");
			report_last_txhdr("Waiting on NAK", 0);
		}
		return (OK);
	}
	if (!Zmodem)
	{
		report_last_txhdr("START PENDING", 0);
		if (getnak())
		{
			report_str("Timeout on pathname nak", 1);
			return (ERROR);
		}
	}

	q = 0;
	if (Dottoslash)
	{						 /* change / to . */
		for (p = name; *p; ++p)
		{
			if (*p == '/')
				q = p;
			else if (*p == '.')
				*(q = p) = '/';
		}
		if (q && strlen(++q) > (unsigned)8)
		{					 /* If name>8 chars */
			q += 8;			 /* make it .ext */
			strcpy(name2, q);/* save excess of name */
			*q = '.';
			strcpy((char *)++q, name2);	/* add it back */
		}
	}

	for (p = name, q = (char *)txbuf; *p;)
	{
		if ((*q++ = *p++) == '/' && !Fullname)
			q = (char *)txbuf;
	}
	*q++ = 0;
	p = q;
	while (q < (char *)(txbuf + sizeof(txbuf)))
		*q++ = 0;
	if (!Ascii && (in != stdin) && *name && !fstat(fileno(in), &f))
	{
		f.st_mode &= ~(S_ISUID | S_ISGID);
		sprintf(p, "%lu %lo %o 0 %d %ld", (long)f.st_size, f.st_mtime,
			f.st_mode, Filesleft, TotalLeft);
	}
	report_xfer_mode((Lzconv == ZCNL) ? "ASCII" : "BINARY");
	TotalLeft -= f.st_size;
	if (--Filesleft <= 0)
		TotalLeft = 0;
	if (TotalLeft < 0)
		TotalLeft = 0;

	/* force 1k blocks if name won't fit in 128 byte block */
	if (txbuf[125])
		blklen = 1024;
	else
	{						 /* A little goodie for IMP/KMD */
		txbuf[127] = (f.st_size + 127) >> 7;
		txbuf[126] = (f.st_size + 127) >> 15;
	}
	if (Zmodem)
		return (zsendfile(txbuf, 1 + strlen(p) + (p - (char *)txbuf)));
	report_protocol_type("YMODEM");
	if (wcputsec(txbuf, 0, 128) == ERROR)
		return (ERROR);
	return (OK);
}							 /* end of wctxpn */

/*+-------------------------------------------------------------------------
	getnak()
--------------------------------------------------------------------------*/
int
getnak()
{
	register firstch;

	Lastrx = 0;
	for (;;)
	{
		switch (firstch = readock(50, 1))
		{
			case ZPAD:
				if (getzrxinit())
					return (ERROR);
				Ascii = 0;	 /* Receiver does the conversion */
				return (FALSE);
			case TIMEOUT:
				report_str("Timeout", 1);
				return (TRUE);
			case WANTG:
#if defined(MODE2OK)
				mode(2);	 /* Set cbreak,XON/XOFF,etc. */
#endif
				Optiong = TRUE;
				blklen = 1024;
			case WANTCRC:
				Crcflg = TRUE;
			case NAK:
				return (FALSE);
			case CAN:
				if ((firstch = readock(20, 1)) == CAN && Lastrx == CAN)
					return (TRUE);
			default:
				break;
		}
		Lastrx = firstch;
	}
	/* NOTREACHED */
}							 /* end of getnak */

/*+-------------------------------------------------------------------------
	wctx(flen)
--------------------------------------------------------------------------*/
wctx(flen)
long flen;
{
	register int thisblklen;
	register int firstch;
	register int sectnum;
	register int attempts;
	long charssent;

	charssent = 0;
	firstsec = TRUE;
	thisblklen = blklen;
	report_txblklen(blklen);

	attempts = 8;
	while (((firstch = readock(Rxtimeout, 2)) != NAK) &&
		(firstch != WANTCRC) && (firstch != WANTG) &&
		(firstch != TIMEOUT) && (firstch != CAN))
	{
		if (!--attempts)
		{
			report_str("bad start stimulus", 1);
			send_cancel(1);
			return (ERROR);
		}
	}

	if (firstch == CAN)
	{
		report_str("receiver CAN", 1);
		return (ERROR);
	}

	if ((firstch == WANTCRC) || (firstch == WANTG))
		Crcflg = TRUE;

	report_protocol_crc_type((Crcflg)
		? ((firstch == WANTG) ? "/CRC-g" : "/CRC")
		: "/CHK");

	sectnum = 0;
	for (;;)
	{
		if (flen <= (charssent + 896L))
		{
			thisblklen = 128;
			report_txblklen(thisblklen);
		}
		if (!xbuf_build(txbuf, thisblklen))
			break;
		if (wcputsec(txbuf, ++sectnum, thisblklen) == ERROR)
			return (ERROR);
		charssent += thisblklen;
	}

	/* file transfer completed */
	report_file_byte_io(this_file_length - initial_filepos);
	report_file_close(0);
	fclose(in);

#if defined(CFG_LogXfer)
	sprintf(s128, "SEND success: %s", Pathname);
	logevent(getppid(), s128);
#endif

	attempts = 0;
	do
	{
		purgeline();
		sendline(EOT);
		flushline();
		report_last_txhdr("EOT", 0);
		++attempts;
	}
	while ((firstch = (readock(Rxtimeout, 1)) != ACK) && attempts < RETRYMAX);
	if (attempts == RETRYMAX)
	{
		report_str("No ACK on EOT", 1);
		return (ERROR);
	}
	else
		return (OK);
}

/*+-------------------------------------------------------------------------
	wcputsec(buf,sectnum,cseclen)
--------------------------------------------------------------------------*/
int
wcputsec(buf, sectnum, cseclen)
unsigned char *buf;
int sectnum;
int cseclen;				 /* data length of this block to send */
{
	register int checksum;
	register int wcj;
	register unsigned char *cp;
	unsigned short oldcrc;
	int firstch;
	int attempts;

	firstch = 0;			 /* part of logic to detect CAN CAN */

	sprintf(s128, "Sending block %d", sectnum);
	report_last_txhdr(s128, 0);
	if (log_packets)
	{
		log_packet_buffer(buf, cseclen);
	}

	for (attempts = 0; attempts <= RETRYMAX; attempts++)
	{
		Lastrx = firstch;
		sendline(cseclen == 1024 ? STX : SOH);
		sendline(sectnum);
		sendline(-sectnum - 1);
		oldcrc = checksum = 0;

		for (wcj = cseclen, cp = buf; --wcj >= 0;)
		{
			sendline(*cp);
			oldcrc = updcrc(*cp, oldcrc);
			checksum += *cp++;
		}
		if (Crcflg)
		{
			oldcrc = updcrc(0, updcrc(0, oldcrc));
			sendline((int)(oldcrc >> 8));
			sendline((int)(oldcrc & 0xFF));
		}
		else
			sendline(checksum);
		flushline();

		if (Optiong)
		{
			firstsec = FALSE;
			return (OK);
		}
		firstch = readock(Rxtimeout, (Noeofseen && sectnum) ? 2 : 1);
	  gotnak:
		switch (firstch)
		{
			case CAN:
				if (Lastrx == CAN)
				{
				  cancan:
					report_last_rxhdr("CAN", 1);
					return (ERROR);
				}
				break;
			case TIMEOUT:
				report_last_rxhdr("Timeout", 1);
				continue;
			case WANTCRC:
				if (firstsec)
					Crcflg = TRUE;
			case NAK:
				report_last_rxhdr("NAK", 1);
				continue;
			case ACK:
				report_last_rxhdr("ACK", 0);
				firstsec = FALSE;
				Totsecs += (cseclen >> 7);
				return (OK);
			case ERROR:
				report_last_rxhdr("Noise", 0);
				break;
			default:
				sprintf(s128, "0x%02x ???", firstch);
				report_last_rxhdr(s128, 1);
				break;
		}
		for (;;)
		{
			Lastrx = firstch;
			if ((firstch = readock(Rxtimeout, 2)) == TIMEOUT)
				break;
			if (firstch == NAK || firstch == WANTCRC)
				goto gotnak;
			if (firstch == CAN && Lastrx == CAN)
				goto cancan;
		}
	}
	report_str("retry count exceeded", 1);
	return (ERROR);
}							 /* end of wcputsec */

/*+-------------------------------------------------------------------------
	xbuf_build(buf,count)
  fill buf with count chars padding with ^Z for CPM and DOS
--------------------------------------------------------------------------*/
xbuf_build(buf, count)
register unsigned char *buf;
int count;
{
	register c, m;

#ifndef __FreeBSD__
	long lseek();

#endif
	long X_txpos = lseek(fileno(in), 0L, 1);
	char diag_str[64];

	report_send_progress(X_txpos);
	if (!Ascii)
	{
		m = read(fileno(in), buf, count);
		if (log_packets)
		{
			sprintf(diag_str, "read rtnd %d of %d", m, count);
			report_str(diag_str, 1);
		}
		if (m <= 0)
			return (0);
		while (m < count)
			buf[m++] = 032;
		return (count);
	}
	m = count;
	if (Lfseen)
	{
		*buf++ = 012;
		--m;
		Lfseen = 0;
	}
	while ((c = fgetc(in)) != EOF)
	{
		if (c == 012)
		{
			*buf++ = 015;
			if (--m == 0)
			{
				Lfseen = TRUE;
				break;
			}
		}
		*buf++ = c;
		if (--m == 0)
			break;
	}
	if (m == count)
		return (0);
	else
		while (--m >= 0)
			*buf++ = CPMEOF;
	return (count);
}							 /* end of xbuf_build */

/*+-------------------------------------------------------------------------
	zbuf_build(buf,count) - fill buf with count chars
--------------------------------------------------------------------------*/
int
zbuf_build(buf, count)
register char *buf;
int count;
{
	register c, m;

	m = count;
	while ((c = fgetc(in)) != EOF)
	{
		*buf++ = c;
		if (--m == 0)
			break;
	}
	return (count - m);
}							 /* end of zbuf_build */

/*+-------------------------------------------------------------------------
	SIGALRM_handler()
--------------------------------------------------------------------------*/
CFG_SigType
SIGALRM_handler()
{
	report_rx_ind(0);
	report_tx_ind(0);
	longjmp(tohere, -1);
}							 /* end of SIGALRM_handler */

/*+-------------------------------------------------------------------------
	readock(timeout,count)
timeout is in tenths of seconds reads character(s) from file
descriptor 'fd' read 'count' characters, (1 <= count <= 3) if more than
one received, return ERROR unless all are CAN normal response is NAK,
ACK, CAN, G or C
--------------------------------------------------------------------------*/
int
readock(timeout, count)
int timeout;
int count;
{
	int c;
	static char byt[5];

	if (setjmp(tohere))
	{
		report_str("TIMEOUT", 1);
		return (TIMEOUT);
	}
	c = timeout / 10;
	if (c < 2)
		c = 2;
	signal(SIGALRM, SIGALRM_handler);
	alarm(c);
	c = read(iofd, byt, count);
	alarm(0);
	if (c < 1)
		return (TIMEOUT);
	rx_char_count += c;
	if (c == 1)
		return (byt[0] & 0xFF);
	while (c)
	{
		if (byt[--c] != CAN)
			return (ERROR);
	}

	return (CAN);
}							 /* end of readock */

/*+-------------------------------------------------------------------------
	readline(timeout)
--------------------------------------------------------------------------*/
int
readline(timeout)
int timeout;
{
	return (readock(timeout, 1));
}							 /* end of readline */

/*+-------------------------------------------------------------------------
	purgeline()
--------------------------------------------------------------------------*/
void
purgeline()
{
	ecuflush(iofd, TCIFLUSH);
}							 /* end of purgeline */

/*+-------------------------------------------------------------------------
	send_cancel(error) - send cancel to remote
--------------------------------------------------------------------------*/
void
send_cancel(error)
int error;
{
	static char canistr[] =
	{
		24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 0
	};
	register char *cptr = canistr;

	report_last_txhdr("^X CAN", !!error);
	while (*cptr)
		sendline(*cptr++);
	flushline();
}							 /* end of send_cancel */

/*+-------------------------------------------------------------------------
	substr(str,str2) - searches for str2 in string str
--------------------------------------------------------------------------*/
char *
substr(str, str2)
register char *str;
register char *str2;
{
	register char *sptr;
	register char *ssptr;

	for (sptr = str; *str; str++)
	{
		if (*str == *str2)
		{
			sptr = str;
			ssptr = str2;
			while (1)
			{
				if (*ssptr == 0)
					return (str);
				if (*sptr++ != *ssptr++)
					break;
			}
		}
	}
	return (NULL);
}							 /* end of substr */

/*+-------------------------------------------------------------------------
	usage()
--------------------------------------------------------------------------*/
void
usage()
{
	exit(255);
}							 /* end of usage */

/*+-------------------------------------------------------------------------
	getzrxinit() - Get the receiver's init parameters
--------------------------------------------------------------------------*/
int
getzrxinit()
{
	register n;
	struct stat f;

	for (n = 10; --n >= 0;)
	{
		switch (zgethdr(Rxhdr))
		{
			case ZCHALLENGE:/* Echo receiver's challenge numbr */
				stohdr(Rxpos);
				zshhdr(ZACK, Txhdr);
				continue;
			case ZCOMMAND:	 /* They didn't see out ZRQINIT */
				stohdr(0L);
				zshhdr(ZRQINIT, Txhdr);
				continue;
			case ZRINIT:
				Rxflags = 0377 & Rxhdr[ZF0];
				Txfcs32 = (Wantfcs32 && (Rxflags & CANFC32));
				report_protocol_type("ZMODEM");
				report_protocol_crc_type((Txfcs32) ? "/CRC32" : "/CRC16");
				Zctlesc |= Rxflags & TESCCTL;
				Rxbuflen = (0377 & Rxhdr[ZP0]) + ((0377 & Rxhdr[ZP1]) << 8);
				if (!(Rxflags & CANFDX))
					Txwindow = 0;
#if defined(MODE2OK)
				mode(2);	 /* Set cbreak,XON/XOFF,etc. */
#endif
				/* Override to force shorter frame length */
				if (Rxbuflen && (Rxbuflen > Tframlen) && (Tframlen >= 32))
					Rxbuflen = Tframlen;
				if (!Rxbuflen && (Tframlen >= 32) && (Tframlen <= 1024))
					Rxbuflen = Tframlen;

				/* If using a pipe for testing set lower buf len */
				fstat(iofd, &f);
				if ((f.st_mode & S_IFMT) != S_IFCHR
					&& (Rxbuflen == 0 || Rxbuflen > 4096))
					Rxbuflen = 4096;
				sprintf(s128, "Remote: CRC32 %c  duplex %c",
					(Rxflags & CANFC32) ? 'y' : 'n',
					(Rxflags & CANFDX) ? 'y' : 'n');
				if (Rxbuflen)
					sprintf(&s128[strlen(s128)], "  buflen %u", Rxbuflen);
				else
					strcat(s128, "  continuous stream y");
				report_str(s128, 2);

				/*
				 * If input is not a regular file,force ACK's each 1024 (A
				 * smarter strategey could be used here ...)
				 */
				if (!Command)
				{
					fstat(fileno(in), &f);
					if (((f.st_mode & S_IFMT) != S_IFREG)
						&& (Rxbuflen == 0 || Rxbuflen > 1024))
						Rxbuflen = 1024;
				}

				if (Bitrate > 300)	/* Set initial subpacket len */
					blklen = 256;
				if (Bitrate > 1200)
					blklen = 512;
				if (Bitrate >= 2400)	/* original code had > 2400 here *** */
					blklen = 1024;
				if (Rxbuflen && blklen > Rxbuflen)
					blklen = Rxbuflen;
				if (blkopt && blklen > blkopt)
					blklen = blkopt;
				blklen_original = blklen;
				report_txblklen(blklen);
				return (sendzsinit());
			case ZCAN:
			case TIMEOUT:
				return (ERROR);
			case ZRQINIT:
				if (Rxhdr[ZF0] == ZCOMMAND)
					continue;
			default:
				zshhdr(ZNAK, Txhdr);
				continue;
		}
	}
	return (ERROR);
}							 /* end of getzrxinit */

/*+-------------------------------------------------------------------------
	sendzsinit() - send send-init information
--------------------------------------------------------------------------*/
sendzsinit()
{
	register c;

	if (Myattn[0] == '\0' && (!Zctlesc || (Rxflags & TESCCTL)))
		return (OK);
	errors = 0;
	for (;;)
	{
		stohdr(0L);
		if (Zctlesc)
		{
			Txhdr[ZF0] |= TESCCTL;
			zshhdr(ZSINIT, Txhdr);
		}
		else
			zsbhdr(ZSINIT, Txhdr);
		zsdata(Myattn, 1 + strlen(Myattn), ZCRCW);
		c = zgethdr(Rxhdr);
		switch (c)
		{
			case ZCAN:
				return (ERROR);
			case ZACK:
				return (OK);
			default:
				if (++errors > 19)
					return (ERROR);
				continue;
		}
	}
}							 /* end of sendzsinit */

/*+-------------------------------------------------------------------------
	zsendfile(buf,blen) - send file name & info
--------------------------------------------------------------------------*/
zsendfile(buf, blen)
unsigned char *buf;
int blen;
{
	register c;

	for (;;)
	{
		blklen = blklen_original;
		report_txblklen(blklen);
		Txhdr[ZF0] = Lzconv; /* file conversion request */
		Txhdr[ZF1] = Lzmanag;/* file management request */
		Txhdr[ZF2] = Lztrans;/* file transport request */
		Txhdr[ZF3] = 0;
		zsbhdr(ZFILE, Txhdr);
		zsdata(buf, blen, ZCRCW);
	  again:
		c = zgethdr(Rxhdr);
		switch (c)
		{
			case ZRINIT:
				while ((c = readline(50)) > 0)
					if (c == ZPAD)
					{
						goto again;
					}
				/* **** FALL THRU TO **** */
			default:
				continue;
			case ZCAN:
			case TIMEOUT:
			case ZABORT:
			case ZFIN:
				return (ERROR);
			case ZSKIP:
				report_file_close(3);
				fclose(in);
				return (c);
			case ZRPOS:
				if (!seen_zrpos)
					initial_filepos = Rxpos;
				seen_zrpos = 1;

				/*
				 * Suppress zcrcw request otherwise triggered by
				 * lastyunc==bytcnt
				 */
				Lastsync = (bytcnt = Txpos = Rxpos) - 1;
				fseek(in, Rxpos, 0);
				Dontread = FALSE;
				report_send_progress(Txpos);
				return (zsendfdata());
		}
	}
}							 /* end of zsendfile */

/*+-------------------------------------------------------------------------
	zsendfdata() - send data in the file
--------------------------------------------------------------------------*/
zsendfdata()
{
	int c = 0, e;
	unsigned n;
	int newcnt;
	unsigned long tcount = 0;
	int junkcount;			 /* Counts garbage chars received by TX */
	int err;

	Lrxpos = 0;
	junkcount = 0;
	SameZrposAgain = FALSE;	 /* variable was named Beenhereb4 (wht) */
	this_file_frame_count = 0;	/* we've sent no frames (wht) */
  somemore:
	if (setjmp(intrjmp))
	{
	  waitack:
		junkcount = 0;
		c = getinsync(0);
	  gotack:
		switch (c)
		{
			default:
			case ZCAN:
				report_rcvr_cancelled("zfdata-1");
				report_file_close(4);
				fclose(in);
				return (ERROR);
			case ZSKIP:
				report_file_close(5);
				fclose(in);
				return (c);
			case ZACK:
			case ZRPOS:
				if (!seen_zrpos)
					initial_filepos = Rxpos;
				seen_zrpos = 1;
				break;
			case ZRINIT:
				return (OK);
		}

		/*
		 * If the reverse channel can be tested for data, this logic may
		 * be used to detect error packets sent by the receiver, in place
		 * of setjmp/longjmp Rdchk(fdes) returns non 0 if a character is
		 * available
		 */
		while (Rdchk(iofd))
		{
			switch (readline(1))
			{
				case CAN:
				case ZPAD:
					c = getinsync(1);
					goto gotack;
				case XOFF:	 /* Wait a while for an XON */
				case XOFF | 0200:
					readline(100);
			}
		}
	}

	newcnt = Rxbuflen;
	Txwcnt = 0;
	stohdr(Txpos);
	zsbhdr(ZDATA, Txhdr);

	do
	{
		if (Dontread)
		{
			n = Lastn;
		}
		else
		{
			n = zbuf_build(txbuf, blklen);
			Lastread = Txpos;
			Lastn = n;
		}
		Dontread = FALSE;
		if (n < blklen)
			e = ZCRCE;
		else if (junkcount > 3)
			e = ZCRCW;
		else if (bytcnt == Lastsync)
			e = ZCRCW;
		else if (Rxbuflen && (newcnt -= n) <= 0)
			e = ZCRCW;
		else if (Txwindow && (Txwcnt += n) >= Txwspac)
		{
			Txwcnt = 0;
			e = ZCRCQ;
		}
		else
			e = ZCRCG;
		zsdata(txbuf, n, e);
		this_file_frame_count++;	/* wht */
		if (bad_condx_blklen)/* wht */
		{

			/*
			 * if we have sent four frames since last ZRPOS to same pos
			 */
			if ((this_file_frame_count - bad_condx_frame_count) > 4)
			{
				if (blklen == bad_condx_blklen)
					bad_condx_blklen = 0;
				else
				{
					blklen *= 2;
					report_txblklen(blklen);
				}
				SameZrposAgain = 0;
			}
		}
		bytcnt = Txpos += n;
		report_send_progress(Txpos);
		if (e == ZCRCW)
			goto waitack;

		/*
		 * If the reverse channel can be tested for data, this logic may
		 * be used to detect error packets sent by the receiver,in place
		 * of setjmp/longjmp Rdchk(fdes) returns non 0 if a character is
		 * available
		 */
		while (Rdchk(iofd))
		{
			switch (readline(1))
			{
				case CAN:
				case ZPAD:
					c = getinsync(1);
					if (c == ZACK)
						break;
					ecuflush(iofd, TCOFLUSH);
					/* zcrce - dinna wanna starta ping-pong game */
					zsdata(txbuf, 0, ZCRCE);
					goto gotack;

				case XOFF:	 /* Wait a while for an XON */
				case XOFF | 0200:
					readline(100);

				default:
					++junkcount;
			}
		}
		if (Txwindow)
		{
			while ((tcount = Txpos - Lrxpos) >= Txwindow)
			{
				if (e != ZCRCQ)
					zsdata(txbuf, 0, e = ZCRCQ);
				c = getinsync(1);
				if (c != ZACK)
				{
					ecuflush(iofd, TCOFLUSH);
					zsdata(txbuf, 0, ZCRCE);
					goto gotack;
				}
			}
		}
	}
	while (n == blklen);

	for (;;)
	{
		stohdr(Txpos);
		zsbhdr(ZEOF, Txhdr);
		switch (err = getinsync(0))
		{
			case ZACK:
				continue;
			case ZRPOS:
				if (!seen_zrpos)
					initial_filepos = Rxpos;
				seen_zrpos = 1;
				goto somemore;
			case ZRINIT:
				return (OK);
			case ZSKIP:
				report_file_close(6);
				fclose(in);
				return (c);
			default:
				sprintf(s128, "SEND protocol sync error 0x%04x: %s", err, Pathname);
				logevent(getppid(), s128);	/* always log this */
				report_str(s128 + 5, 1);
				skip_count++;
				report_error_count();
				report_file_byte_io(this_file_length - initial_filepos);
				report_file_close(7);
				fclose(in);
				return (ERROR);
		}
	}
}							 /* end of zsendfdata */

/*+-------------------------------------------------------------------------
	getinsync(flag) - get back in sync with receiver
--------------------------------------------------------------------------*/
int
getinsync(flag)
int flag;
{
	register c;

	for (;;)
	{
		switch (c = zgethdr(Rxhdr))
		{
			case ZCAN:
			case ZABORT:
			case ZFIN:
			case TIMEOUT:
				sprintf(s128, "Receiver %s", frametypes[c + FTOFFSET]);
				report_str(s128, 1);
				return (ERROR);
			case ZRPOS:
				report_str("Receiver ZRPOS", 1);
				if (!seen_zrpos)
					initial_filepos = Rxpos;
				seen_zrpos = 1;
				/* ************************************* */
				/* If sending to a modem buffer,you     */
				/* might send a break at this point to */
				/* dump the modem's buffer.            */
				/* ************************************* */
				if (Lastn >= 0 && Lastread == Rxpos)
				{
					Dontread = TRUE;
				}
				else
				{
					clearerr(in);	/* In case file EOF seen */
					fseek(in, Rxpos, 0);
				}
				bytcnt = Lrxpos = Txpos = Rxpos;
				if (Lastsync == Rxpos)	/* wht - original code */
				{			 /* wht - original code */
					/* save frame count at time of each occurrence (wht) */
					bad_condx_frame_count = this_file_frame_count;	/* wht */
					/* save block length at time of error (wht) */
					if (++SameZrposAgain > 4)	/* wht - original code */
					{		 /* wht */
						if (bad_condx_blklen == 0)	/* wht */
							bad_condx_blklen = blklen;	/* wht */
						if (blklen > 256)	/* wht - 32->256 */
						{
							blklen /= 2;	/* wht - original code */
							report_txblklen(blklen);
						}
					}		 /* wht */
				}			 /* wht - original code */
				Lastsync = Rxpos;
				report_send_progress(Txpos);
				return (c);
			case ZACK:
				report_str("", -1);
				Lrxpos = Rxpos;
				if (flag || Txpos == Rxpos)
					return (ZACK);
				continue;

			case ZRINIT:
				report_str("", -1);
#if defined(CFG_LogXfer)
				sprintf(s128, "SEND success: %s", Pathname);
				logevent(getppid(), s128);
#endif
			case ZSKIP:
				report_file_byte_io(this_file_length);
				report_file_close(0);
				fclose(in);
				return (c);
			case ERROR:
			default:
				report_str("Sending ZNAK", 0);
				zsbhdr(ZNAK, Txhdr);
				continue;
		}
	}
}							 /* end of getinsync */

/*+-------------------------------------------------------------------------
	saybibi() - Say "bibi" to the receiver, try to do it cleanly
--------------------------------------------------------------------------*/
void
saybibi()
{
	for (;;)
	{
		stohdr(0L);			 /* CAF Was zsbhdr - minor change */
		zshhdr(ZFIN, Txhdr); /* to make debugging easier */
		switch (zgethdr(Rxhdr))
		{
			case ZFIN:
				sendline('O');
				sendline('O');
				flushline();
			case ZCAN:
			case TIMEOUT:
				return;
		}
	}
}							 /* end of saybibi */

/*+-------------------------------------------------------------------------
	determine_transaction_time()
--------------------------------------------------------------------------*/
void
determine_transaction_time()
{
	register c;
	struct stat f;
	char *name;

	rewind_file_list();
	TotalLeft = 0;
	Filesleft = 0;
	while (get_file_list_name(&name))
	{
		f.st_size = -1;
		if ((access(name, 04) >= 0) && (stat(name, &f) >= 0))
		{
			c = f.st_mode & S_IFMT;
			if (c != S_IFDIR && c != S_IFBLK)
			{
				++Filesleft;
				TotalLeft += f.st_size;
			}
		}
	}
	FilesTotal = Filesleft;
	rewind_file_list();
}							 /* end of determine_transaction_time */

/* vi: set tabstop=4 shiftwidth=4: */
/* end of ecusz.c */
