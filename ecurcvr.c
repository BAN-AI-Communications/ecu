/* #define DEBUG_RAWLOG */

/* #define DEFENSIVE */
/* #define ANSI_DEBUG */
/* #define ANSI_DEBUG_AAS */
/* #define ANSI_DEBUG_TEXT */
/* #define ANSI_DEBUG_SEQ */
/* #define ANSI_DEBUG_NOBUF */
/* #define ANSI_DEBUG_LOGFILE	"/dev/tty2f" */
/* #define CURSOR_DEBUG */

#ifndef LIMIT_BELL
#define LIMIT_BELL
#endif

#if defined(WHT) && defined(WHT_CONFUSED)
#define ANSI_DEBUG
#define ANSI_DEBUG_AAS		 /* show "aas:" accumulation */
#define ANSI_DEBUG_TEXT		 /** show text */
#define ANSI_DEBUG_SEQ
#define ANSI_DEBUG_NOBUF
#define ANSI_DEBUG_LOGFILE	"./ansi.log"
#endif

/*+-------------------------------------------------------------------------
	ecurcvr.c - rcvr process + ANSI filter + non-ANSI<->ANSI hoop jumping
	wht@wht.net

  Defined functions:
	accumulate_ansi_sequence(rchar)
	ansi_CNL()
	ansi_CPL()
	ansi_CUB()
	ansi_CUD()
	ansi_CUF()
	ansi_CUP()
	ansi_CUU()
	ansi_DCH()
	ansi_DL()
	ansi_DSR()
	ansi_ECH()
	ansi_ED()
	ansi_EL()
	ansi_HPA()
	ansi_ICH()
	ansi_IL()
	ansi_SD()
	ansi_SGR()
	ansi_SU()
	ansi_VPA()
	is_ansi_terminator(rchar)
	lgetc_rcvr()
	lgetc_rcvr_raw()
	need_rcvr_restart()
	process_ansi_sequence()
	process_rcvd_char(rchar)
	rcvd_ESC()
	rcvr()
	rcvr_ansi_filter(rchar)
	rcvr_ansi_filter_control(on_off, display)
	rcvr_conditional_restart(restart_flag, display_flag)
	rcvr_log_open()
	rcvrdisp(buf, buflen)
	rcvrdisp_actual()
	rcvrdisp_actual2()
	rcvrdisp_p()
	rcvrdisp_v()
	redisplay_rcvr_screen()
	saved_cursor_restore_cursor()
	saved_cursor_save_cursor()
	spaces(buf, buflen)
	spaces_trap(code, buf, buflen)
	start_rcvr_process(notify_flag)
	xmtr_wfp_debug_hack()

    Any perceptible delay will eventually get on your nerves. --Bob Hyers

--------------------------------------------------------------------------*/
/*+:EDITS:*/
/*:04-26-2000-11:15-wht@bob-RELEASE 4.42 */
/*:01-24-1997-02:37-wht@yuriatin-SOURCE RELEASE 4.00 */
/*:09-28-1996-23:52-wht@yuriatin-shmr_notify_xmtr_of_telnet_close */
/*:09-24-1996-15:24-wht@yuriatin-fix rcvrdisp/ff-stderr synch */
/*:09-11-1996-20:00-wht@yuriatin-3.48-major telnet,curses,structural overhaul */
/*:08-11-1996-02:10-wht@kepler-rename ecu_log_event to logevent */
/*:12-12-1995-14:22-wht@kepler-[no line attached] on such rcvr start */
/*:11-23-1995-11:20-wht@kepler-source control 3.37 for tsx-11 */
/*:11-14-1995-10:23-wht@kepler-3.37.80-source control point: SOCKETS */
/*:11-13-1995-12:26-wht@kepler-start_rcvr_process moved ecusighdl.c */
/*:11-13-1995-12:22-wht@kepler-need_rcvr_restart checks Liofd for open line */
/*:11-13-1995-12:21-wht@kepler-need_rcvr_restart was a macro in ecu.h */
/*:11-04-1995-21:02-wht@kepler-if telnet_cmd called, rtn 0 to lgetc caller */
/*:11-03-1995-16:54-wht@wwtp1-use CFG_TelnetOption */
/*:10-09-1995-15:41-wht@kepler-SU newlines were sent to stderr not rcvrdisp */
/*:10-09-1995-15:03-wht@kepler-label ease case in process_ansi_sequence */
/*:09-16-1995-15:06-root@kepler-rename specific ANSI_DEBUG partitions */
/*:01-12-1995-15:19-wht@n4hgf-apply Andrew Chernov 8-bit clean+FreeBSD patch */
/*:05-04-1994-04:39-wht@n4hgf-ECU release 3.30 */
/*:10-03-1993-20:09-wht@n4hgf-document read errors w/fd on screen */
/*:08-18-1993-05:49-wht@n4hgf-rcvr seems ready for release */
/*:08-07-1993-20:38-wht@n4hgf-add xmtr_wfp_debug_hack */
/*:07-23-1993-15:42-wht@n4hgf-detect/ignore ESC = or ESC > VT100 keypad */
/*:07-17-1993-12:36-wht@n4hgf-no more rcvrdisp_actual2_xmtr_buffer junk */
/*:12-31-1992-15:34-wht@n4hgf-handle VT100 save/restore cursor */
/*:12-03-1992-14:24-wht@n4hgf-differentiate between type 5 and other DSR */
/*:09-10-1992-13:58-wht@n4hgf-ECU release 3.20 */
/*:09-06-1992-13:29-wht@n4hgf-add receiver process buffered screen write */
/*:08-22-1992-15:38-wht@n4hgf-ECU release 3.20 BETA */
/*:05-29-1992-13:28-wht@n4hgf-no banner - phone numbers are security risk */
/*:11-11-1991-14:25-wht@n4hgf-lzero_length_read_detected code */
/*:11-11-1991-12:45-wht@n4hgf-add LIMIT_BELL code */
/*:08-26-1991-16:39-wht@n4hgf2-SD was still hopelessly manic */
/*:07-25-1991-12:56-wht@n4hgf-ECU release 3.10 */
/*:07-05-1991-06:13-wht@n4hgf-SD was in baaaaadd shape */
/*:01-09-1991-22:31-wht@n4hgf-ISC port */
/*:12-26-1990-14:32-wht@n4hgf-use memset in spaces() */
/*:12-21-1990-21:06-wht@n4hgf-CUF and CUB set non-ansi cursor incorrectly */
/*:12-20-1990-16:27-wht@n4hgf-had SU and SD swapped */
/*:11-30-1990-18:39-wht@n4hgf-non-ansi console rcvr appears to be working */
/*:11-28-1990-14:13-wht@n4hgf-start non-ansi console support */
/*:08-14-1990-20:40-wht@n4hgf-ecu3.00-flush old edit history */

#include "ecu.h"
#include "ecukey.h"

#if defined(CFG_TelnetOption)
#include <arpa/telnet.h>
#endif /* defined(CFG_TelnetOption) */

#ifdef CFG_SemWithShm
#include <sys/ipc.h>
#include <sys/sem.h>
#endif /* CVRDISP_PV */

extern int errno;
extern char rcvr_log_file[]; /* if rcvr_log!= 0,log filename */
extern int rcvr_log;		 /* rcvr log active if != 0 */
extern FILE *rcvr_log_fp;	 /* rcvr log file */
extern int rcvr_log_raw;	 /* if true, log all, else filter ctl chrs */
extern int rcvr_log_append;	 /* if true, append, else scratch */
extern int rcvr_log_flusheach;	/* if true, flush log on each char */
extern int rcvr_log_gen_title;

extern UINT tcap_LINES;		 /* terminal line quantity - see ecutcap.c */
extern UINT tcap_COLS;		 /* terminal column quantity - see ecutcap.c */
extern UINT LINESxCOLS;

static char esc = ESC;

#define MAX_ANSI_LEN	30	 /* generous */
char ansibuf[MAX_ANSI_LEN];
char *ansi;
int ansilen = 0;
int in_ansi_accumulation = 0;

int saved_cursor_y;
int saved_cursor_x;

#define RCVR_RDQUAN		250
uchar lgetc_buf[RCVR_RDQUAN];
uchar *lgetc_ptr;
int lgetc_count;

uchar autorz_frame[] =
{SUB, 'B', '0', '0'};

#ifdef ANSI_DEBUG
FILE *wfp = (FILE *) 0;

#endif

/*
 * the main purpose of this is to map ruling characters, but as a
 * side effect, also map others to reasonable, near, amusing, or
 * random printing characters as well
 */
uchar non_multiscreen_hi_map[128] =
{
	/* 80 */ 'c', 'u', 'e', 'a', 'a', 'a', 'a', 'c',
	/* 88 */ 'e', 'e', 'e', 'i', 'i', 'i', 'a', 'a',
	/* 90 */ 'e', 'e', 'a', 'a', 'a', 'o', 'u', 'u',
	/* 98 */ 'y', 'o', 'u', 'X', '#', 'Y', 'P', 'f',
	/* A0 */ 'a', 'i', 'o', 'u', 'n', 'n', 'a', 'o',
	/* A8 */ '?', '-', '-', '%', '%', '|', '<', '>',
	/* B0 */ '#', '#', '#', '|', '+', '+', '+', '.',
	/* B8 */ '.', '+', '|', '.', '\'', '\'', '\'', '.',
	/* C0 */ '`', '+', '+', '+', '-', '+', '+', '+',
	/* C8 */ '`', '.', '+', '+', '+', '=', '+', '+',
	/* D0 */ '+', '+', '+', '`', '`', '.', '.', '+',
	/* D8 */ '+', '\'', '.', '#', '_', '|', '|', '-',
	/* E0 */ 'a', 'b', 'F', 'T', 'E', 'o', 'u', 't',
	/* E8 */ 'I', '0', 'O', 'o', 'o', 'o', 'e', 'n',
	/* F0 */ '=', '+', '>', '<', 'f', 'j', '%', '=',
	/* F8 */ 'o', '.', '.', 'V', 'n', '2', '*', ' '
};

/*
 * prototypes for ansi filter code gaggled at bottom
 * of this module
 */

#if (__STDC__ == 1)
#define pp(s) s
#else
#define pp(s) ()
#endif

#if !defined(CFG_NoAnsiEmulation)
void redisplay_rcvr_screen pp((void));
void spaces_trap pp((int code, uchar * buf, UINT buflen));
void spaces pp((uchar * buf, UINT buflen));
void ansi_SGR pp((void));
void ansi_CUP pp((void));
void ansi_CUU pp((void));
void ansi_CUD pp((void));
void ansi_CUF pp((void));
void ansi_CUB pp((void));
void ansi_DSR pp((void));
void ansi_ED pp((void));
void ansi_EL pp((void));
void ansi_ECH pp((void));
void ansi_SU pp((void));
void ansi_SD pp((void));
void ansi_HPA pp((void));
void ansi_VPA pp((void));
void ansi_IL pp((void));
void ansi_ICH pp((void));
void ansi_DL pp((void));
void ansi_DCH pp((void));
void ansi_CPL pp((void));
void ansi_CNL pp((void));
void saved_cursor_save_cursor pp((void));
void saved_cursor_restore_cursor pp((void));
void rcvd_ESC pp((void));
int is_ansi_terminator pp((UINT rchar));
void accumulate_ansi_sequence pp((UINT rchar));
void process_ansi_sequence pp((void));
int rcvr_ansi_filter pp((int rchar));

#endif /* !defined(CFG_NoAnsiEmulation) */

void rcvrdisp pp((char *buf, int buflen));
void rcvrdisp_actual pp((void));
void rcvrdisp_actual2 pp((void));
void rcvrdisp_p pp((void));
void rcvrdisp_v pp((void));
void rcvr();
void rcvr_notify();

#undef pp

/*+-----------------------------------------------------------------------
	start_rcvr_process(notify_flag) - start RCVR process if not extant
------------------------------------------------------------------------*/
void
start_rcvr_process(notify_flag)
int notify_flag;
{
	extern UINT32 colors_current;
	extern int xmtr_killed_rcvr;
	CFG_PidType rcvr_pid;

	fflush(so);
	fflush(se);

	if (shm->Liofd <= 0)
	{
		UINT32 colors_at_entry = colors_current;

		setcolor(colors_notify);
		ff(se, "[no line attached]");
		setcolor(colors_at_entry);
		ff(se, "\r\n");
		return;
	}

	if (shm->rcvr_pid > 0)	 /* if process already active, just ... */
		return;

	if (rcvr_log && rcvr_log_file[0] && rcvr_log_fp)
	{
		fclose(rcvr_log_fp);
		rcvr_log_fp = 0;
	}

	xmtr_killed_rcvr = 0;
	rcvr_pid = smart_fork();
	if (!rcvr_pid)			 /* if we are the (spawned) rcvr process */
	{
		if (notify_flag)
		{
			char *text = "[interactive mode]";
			UINT32 colors_at_entry = colors_current;

			setcolor(colors_notify);
			rcvrdisp(text, strlen(text));
			setcolor(colors_at_entry);
			rcvrdisp("\r\n", 2);
			rcvrdisp_actual();
		}

#if defined(FORK_DEBUG)
		vlogevent(getppid(), "RCVR-START pid %d", getpid());
#endif
		rcvr();				 /* run until killed */
		/* NOTREACHED */
	}
	else if (rcvr_pid > 0)	 /* we are the father (xmtr) process */
	{
		shm->rcvr_pid = rcvr_pid;
#if defined(FORK_DEBUG)
		sleep(2);
#endif
		if (rcvr_log)
			rcvr_log_append = 1;	/* until next %log -s */
		xmtr_signals();
		return;
	}

	shm->rcvr_pid = -1;		 /* no receiver active */

	pprintf("\n\nECU could not fork for receive process\n");
	termecu(TERMECU_NO_FORK_FOR_RCVR);
	/* NOTREACHED */

}							 /* end of _start_rcvr_process */

/*+-------------------------------------------------------------------------
	rcvr_conditional_restart(restart_flag,display_flag)

  This function is called by the XMTR to decide if the receiver
  should be restarted (yes if line open, no if not)
--------------------------------------------------------------------------*/
void
rcvr_conditional_restart(restart_flag, display_flag)
int restart_flag;
int display_flag;
{
	if (restart_flag)
	{
		if (shm->Liofd == -1)
		{
			if (display_flag)
			{
				UINT32 colors_at_entry = colors_current;
				extern UINT32 colors_current;

				setcolor(colors_notify);
				ff(se, "[no line attached]");
				setcolor(colors_at_entry);
				ff(se, "\r\n");
			}
			return;
		}
		start_rcvr_process(display_flag);
	}

}							 /* end of rcvr_conditional_restart */

/*+-------------------------------------------------------------------------
	need_rcvr_restart() - will rcvr need restart?

  We want to kill the receiver to get it out of the picture
  for a while.  If it is already dead, we do not want to
  start it up when we are done.

  If shm->rcvr_pid == -2, the receiver is not active, but queued restart
  has been requested by some function.

  We don't want to do it if a procedure is executing or if no line is open.

--------------------------------------------------------------------------*/
int
need_rcvr_restart()
{
	return ((shm->rcvr_pid > 0) || ((shm->rcvr_pid == -2) &&
			!proc_level));	 /* && (shm->Liofd >= 0)); */

}							 /* end of need_rcvr_restart */

/*+-------------------------------------------------------------------------
	rcvrdisp_p() - lock rcvrdisp mechanism
--------------------------------------------------------------------------*/
#ifdef CFG_SemWithShm
void
rcvrdisp_p()
{
	int retn;
	struct sembuf sembuf;

	sembuf.sem_num = 0;
	sembuf.sem_op = -1;
	sembuf.sem_flg = 0;

	while (1)
	{
		if (((retn = semop(shm->rcvrdisp_semid, &sembuf, 1)) >= 0) ||
			(errno != EINTR))
		{
			break;
		}
	}

	if ((retn < 0) && (errno != EINVAL))
	{
		strcpy(lopen_err_str, "rcvrdisp_p failed: SysV IPC error");
		termecu(TERMECU_IPC_ERROR);
	}

}							 /* end of rcvrdisp_p */
#endif /* CFG_SemWithShm */

/*+-------------------------------------------------------------------------
	rcvrdisp_v() - unlock rcvrdisp mechanism
--------------------------------------------------------------------------*/
#ifdef CFG_SemWithShm
void
rcvrdisp_v()
{
	int retn;
	struct sembuf sembuf;

	sembuf.sem_num = 0;
	sembuf.sem_op = 1;
	sembuf.sem_flg = 0;

	while (1)
	{
		if (((retn = semop(shm->rcvrdisp_semid, &sembuf, 1)) >= 0) ||
			(errno != EINTR))
		{
			break;
		}
	}

	if ((retn < 0) && (errno != EINVAL))
	{
		strcpy(lopen_err_str, "rcvrdisp_v failed: SysV IPC error");
		termecu(TERMECU_IPC_ERROR);
	}
}							 /* end of rcvrdisp_v */
#endif /* CFG_SemWithShm */

/*+-------------------------------------------------------------------------
	rcvrdisp_actual() - actual write to screen
--------------------------------------------------------------------------*/
void
rcvrdisp_actual()
{
#ifdef CFG_SemWithShm
	rcvrdisp_p();
#endif /* CFG_SemWithShm */
	if (shm->rcvrdisp_count)
		write(TTYOUT, shm->rcvrdisp_buffer, shm->rcvrdisp_count);
	shm->rcvrdisp_ptr = shm->rcvrdisp_buffer;
	shm->rcvrdisp_count = 0;
#ifdef CFG_SemWithShm
	rcvrdisp_v();
#endif /* CFG_SemWithShm */

}							 /* end of rcvrdisp_actual */

/*+-------------------------------------------------------------------------
	rcvrdisp_actual2() - for tcap, flush only if not receiver
--------------------------------------------------------------------------*/
void
rcvrdisp_actual2()
{
	if (getpid() == shm->rcvr_pid)
		return;
#ifdef CFG_SemWithShm
	rcvrdisp_p();
#endif /* CFG_SemWithShm */
	if (shm->rcvrdisp_count)
		write(TTYOUT, shm->rcvrdisp_buffer, shm->rcvrdisp_count);
	shm->rcvrdisp_ptr = shm->rcvrdisp_buffer;
	shm->rcvrdisp_count = 0;
#ifdef CFG_SemWithShm
	rcvrdisp_v();
#endif /* CFG_SemWithShm */

}							 /* end of rcvrdisp_actual2 */

/*+-------------------------------------------------------------------------
	rcvrdisp(buf,buflen) - logical write to screen
--------------------------------------------------------------------------*/
void
rcvrdisp(buf, buflen)
char *buf;
int buflen;
{

	if ((unsigned)(buflen + shm->rcvrdisp_count) >
		(unsigned)sizeof(shm->rcvrdisp_buffer))
	{
		rcvrdisp_actual();
	}
	if (((unsigned)buflen + shm->rcvrdisp_count) >
		(unsigned)sizeof(shm->rcvrdisp_buffer))
	{
		write(TTYOUT, buf, buflen);
		return;
	}
#ifdef CFG_SemWithShm
	rcvrdisp_p();
#endif /* CFG_SemWithShm */
	memcpy(shm->rcvrdisp_ptr, buf, buflen);
	shm->rcvrdisp_ptr += buflen;
	shm->rcvrdisp_count += buflen;
#ifdef CFG_SemWithShm
	rcvrdisp_v();
#endif /* CFG_SemWithShm */

}							 /* end of rcvrdisp */

/*+-------------------------------------------------------------------------
	lgetc_rcvr_raw() - rcvr_raw version of get char from line
--------------------------------------------------------------------------*/
UINT
lgetc_rcvr_raw()
{
	extern int errno;

#ifdef DEBUG_RAWLOG
	static int fd_rawlog;

	if (!fd_rawlog)
	{
		char *fnm = "/tmp/ecurcvr_raw.log";

#if 1
		fd_rawlog = open(fnm, O_WRONLY | O_APPEND | O_CREAT, 0666);
#else
		creat(fnm, 0666);
		fd_rawlog = open(fnm, O_WRONLY | O_APPEND, 0);
#endif
	}
#endif

	if (!lgetc_count)
	{
		rcvrdisp_actual();
		while (lgetc_count <= 0)
		{
			errno = 0;
			if ((lgetc_count =
					read(shm->Liofd, lgetc_buf, RCVR_RDQUAN)) < 0)
			{
				if (errno == EINTR)	/* if signal interrupted, ... */
					continue;/* ... read again */
				termecu(TERMECU_LINE_READ_ERROR);
			}
			if (!lgetc_count)
			{
				if (shm->Ltelnet)	/* remote reject or disconnect */
					shmr_notify_xmtr_of_telnet_close(); /* never return */
				lzero_length_read_detected();	/* maybe terminate program
												 * ... */
				continue;	 /* ... but if not, read again */
			}
#ifdef DEBUG_RAWLOG
			if (fd_rawlog)
				write(fd_rawlog, lgetc_buf, lgetc_count);
#endif
		}
		shm->rcvd_chars += lgetc_count;
		shm->rcvd_chars_this_connect += lgetc_count;
		lgetc_ptr = lgetc_buf;
	}

	lgetc_count--;
	return (*lgetc_ptr++);
}							 /* end of lgetc_rcvr_raw */

/*+-------------------------------------------------------------------------
	lgetc_rcvr() - rcvr version of get char from line
--------------------------------------------------------------------------*/
UINT
lgetc_rcvr()
{
	UINT char_rtnd = lgetc_rcvr_raw();

#if defined(CFG_TelnetOption)
	if ((shm->Ltelnet) && !shm->Ltelnet_raw && (char_rtnd == IAC))
	{
		telnet_cmd(1);
		char_rtnd = 0;
	}
#endif /* defined(CFG_TelnetOption) */

	if (shm->Lparity)
		return (char_rtnd & 0x7F);
	else
		return (char_rtnd);

}							 /* end of lgetc_rcvr */

/*+-------------------------------------------------------------------------
	rcvr_log_open()
--------------------------------------------------------------------------*/
void
rcvr_log_open()
{

	if (rcvr_log)			 /* if xmtr set us up for logging */
	{
		rcvr_log_fp = fopen(rcvr_log_file, rcvr_log_append ? "a" : "w");
		rcvr_log_append = 1; /* until next %log -s */
		if (!rcvr_log_fp)
		{
			ff(se, "ecu RCVR: Could not open log file: %s\r\n", rcvr_log_file);
			ff(se, "recording aborted.\r\n");
			rcvr_log = 0;
		}
		rcvr_log_gen_title = 0;
	}
}							 /* end of rcvr_log_open */

/*+-------------------------------------------------------------------------
	process_rcvd_char(rchar) - process a received character

  return 0 if char should be written to console, 1 otherwise
--------------------------------------------------------------------------*/
int
process_rcvd_char(rchar)
UINT rchar;
{
	register int itmp;

#ifdef LIMIT_BELL
	long now;
	static long last_bell_time = -1L;

#endif

	/*
	 * automatic ZMODEM frame detection (expensive CPU burners for lazy
	 * folks)
	 */
	if (shm->autorz)
	{
		if ((uchar) rchar == autorz_frame[shm->autorz_pos])
		{
			itmp = shm->autorz_pos;	/* copy to register trying to be quick */
			if (++itmp == sizeof(autorz_frame))
			{
				if (lgetc_count)
				{
					rcvrdisp(lgetc_ptr, lgetc_count);
					lgetc_count = 0;
				}
				shmr_notify_zmodem_frame();
				pause();	 /* wait for death */
				itmp = 0;	 /* in case something starts us up */
			}
			shm->autorz_pos = itmp;
			return (!itmp);	 /* don't try to print ^X */
		}
		else
			shm->autorz_pos = 0;
	}

	/*
	 * BEL and alarm-on-incoming-data processing
	 */
	if (shm->bell_notify_state == 2)
	{
		shm->bell_notify_state = 1;
		bell_notify(XBELL_3T);
	}
	else if (rchar == BEL)
	{
#ifdef LIMIT_BELL
		time(&now);
		if ((now - last_bell_time) < 2L)
			return (1);
		last_bell_time = now;
#endif
		bell_notify(XBELL_ATTENTION);
		return (0);
	}
#ifdef TANDEM_ENQ_ACK		 /* for my friend John Dashner at Tandem */
	else if (rchar == ENQ)
	{
		lputc(ACK);
		return (0);
	}
#endif

	/*
	 * receiver logging
	 */
	if (rcvr_log && rcvr_log_fp)
	{
		/* if raw mode or character not excluded from "cooked" logging */
		if (rcvr_log_raw || isprint(rchar) ||
			(rchar == NL) || (rchar == TAB))
		{
			LOGPUTC(rchar, rcvr_log_fp);
		}
		/* back if log file if not raw and char is backspace */
		else if (!rcvr_log_raw && (rchar == BS))
		{
			long logpos = 0;

			if (logpos = ftell(rcvr_log_fp))
				fseek(rcvr_log_fp, logpos - 1, 0);
		}

		if (rcvr_log_flusheach)
			fflush(rcvr_log_fp);
	}

#if !defined(CFG_NoAnsiEmulation)
	if (shm->rcvr_ansi_filter)
		return (rcvr_ansi_filter(rchar));
#endif

	return (0);

}							 /* end of process_rcvd_char */

/*+-----------------------------------------------------------------------
	rcvr() - copy characters from remote line to screen
------------------------------------------------------------------------*/
void
rcvr()
{
	uchar rchar;
	uchar nlchar = NL;
	char *get_ttyname();

#if defined(ANSI_DEBUG)
	char s80[80];

#endif /* ANSI_DEBUG */

#ifdef ANSI_DEBUG
	wfp = fopen(ANSI_DEBUG_LOGFILE, "a");
	if (ulindex(ANSI_DEBUG_LOGFILE, "/dev/tty") != -1)
	{
		sprintf(s80, "stty opost ocrnl < %s", ANSI_DEBUG_LOGFILE);
		system(s80);
	}
#ifdef ANSI_DEBUG_NOBUF
	setbuf(wfp, NULL);
#endif /* ANSI_DEBUG_NOBUF */
	fprintf(wfp, "******** %s tty_is_multiscreen=%d\n",
		get_ttyname(), tty_is_multiscreen);
#endif /* ANSI_DEBUG */

	/*
	 * remember receiver pid
	 */
	shm->rcvr_pid = getpid();

	/*
	 * reset autorz detector
	 */
	shm->autorz_pos = 0;

	/*
	 * reset line read function
	 */
	lgetc_count = 0;
	lgetc_ptr = lgetc_buf;

	in_ansi_accumulation = 0;
	ansi = ansibuf;
	*ansi = 0;
	ansilen = 0;
	shm->rcvrdisp_ptr = shm->rcvrdisp_buffer;
	shm->rcvrdisp_count = 0;

	/*
	 * yetch - magic number gretching for lines and columns
	 */
	if (!tcap_LINES || !tcap_COLS)
	{
		tcap_LINES = 25;
		tcap_COLS = 80;
	}
	if (tcap_LINES > CFG_ScreenLinesMax)
		tcap_LINES = CFG_ScreenLinesMax;
	if (tcap_COLS > CFG_ScreenColsMax)
		tcap_COLS = CFG_ScreenColsMax;
	LINESxCOLS = tcap_LINES * tcap_COLS;

	rcvr_signals();
	rcvr_log_open();

	saved_cursor_y = shm->cursor_y;
	saved_cursor_x = shm->cursor_x;

	/*
	 * finally! - the receive loop
	 */

	while (1)
	{
		rchar = lgetc_rcvr();

		if (!tty_is_multiscreen && (rchar >= 0x80))
			rchar = non_multiscreen_hi_map[rchar - 0x80];

		if (process_rcvd_char(rchar))
			continue;

		rcvrdisp((char *)&rchar, 1);

		if (shm->Ladd_nl_incoming && (rchar == CRET))
			rcvrdisp((char *)&nlchar, 1);
	}

	/* NOTREACHED */

}							 /* end of rcvr */

/*+-------------------------------------------------------------------------
	xmtr_wfp_debug_hack() - keep xmtr use of rcvr code from bombing

This function is called once by xmtr() before it does much else.
This is a horrible hack only necessary when the chips are down.
If ANSI_DEBUG has wfp open in rcvr, this opens it in the xmtr too.
The function has scope in the production binary only so ecu.c
has no need to know the ANSI debug status.  This is the kind of
hack you never find out about in binary programs you buy :->.

--------------------------------------------------------------------------*/
void
xmtr_wfp_debug_hack()
{
#ifdef ANSI_DEBUG
	wfp = fopen("/dev/null", "w");
#endif
}							 /* end of xmtr_wfp_debug_hack */

/*+-------------------------------------------------------------------------
	rcvr_ansi_filter_control(on_off,display)

  does nothing meaningful if no emulation configured
--------------------------------------------------------------------------*/
void
rcvr_ansi_filter_control(on_off, display)
int on_off;
int display;
{
#if defined(CFG_NoAnsiEmulation)
	if (display)
		pprintf("ignored ... ANSI filter not configured\n");
#else
	if (on_off)
	{
		spaces((char *)shm->screen, LINESxCOLS);
		shm->cursor_y = 0;
		shm->cursor_x = 0;
		shm->rcvr_ansi_filter = 1;
	}
	else
	{
		shm->rcvr_ansi_filter = 0;
	}

	if (display)
	{
		int pointless = 0;

		if (tty_not_char_special)
			pointless = 1;
		if (pointless)
			pprintf("although pointless, ");
		pprintf("ANSI filter set to %s\n",
			(shm->rcvr_ansi_filter) ? "ON" : "off");
	}
#endif /* ndefined(CFG_NoAnsiEmulation) */

}							 /* end of rcvr_ansi_filter_control */

/***************************************************************************
*           R E C E I V E R      A N S I      F I L T E R
*
* The code below is included only in versions which have an "ANSI filter"
* (which may or may not be enabled with shm->rcvr_ansi_filter)
***************************************************************************/

#if !defined(CFG_NoAnsiEmulation)

/*+-------------------------------------------------------------------------
	redisplay_rcvr_screen() - redisplay logical receiver screen
As of writing, this function is called only by the XMTR process
--------------------------------------------------------------------------*/
void
redisplay_rcvr_screen()
{
	UINT y;
	extern int tty_not_char_special;

	if (!shm->rcvr_ansi_filter || tty_not_char_special)
		return;

	setcolor(colors_current);
	tcap_stand_end();
	rcvrdisp_actual();
	for (y = 0; y < tcap_LINES; y++)
	{
		tcap_cursor(y, 0);
		fwrite(&shm->screen[y][0],
			((y != tcap_LINES - 1) ? tcap_COLS : tcap_COLS - 1), 1, se);
	}
	tcap_eeol();
	tcap_cursor(shm->cursor_y, shm->cursor_x);
	rcvrdisp_actual();

}							 /* end of redisplay_rcvr_screen */

/*+-------------------------------------------------------------------------
	spaces_trap(code,buf,buflen)
--------------------------------------------------------------------------*/
#ifdef CURSOR_DEBUG
void
spaces_trap(code, buf, buflen)
int code;
uchar *buf;
UINT buflen;
{
	char *xyz = (char *)0x90000000;

	ff(se, "rcvr 'spaces trap' code %d: cursor x,y=%d,%d\r\n",
		code, shm->cursor_y, shm->cursor_x);
	ff(se, "buf=%08lx len=%08lx offs=%08lx\r\n", buf, buflen,
		(UINT32) buf - (UINT32) shm->screen);
	*xyz = 0;
	abort();
}							 /* end of spaces_trap */
#endif

/*+-------------------------------------------------------------------------
	spaces(buf,buflen) - fill with spaces
--------------------------------------------------------------------------*/
void
spaces(buf, buflen)
uchar *buf;
UINT buflen;
{
#ifdef CURSOR_DEBUG
	if ((UINT32) buf > (((UINT32) shm->screen) + LINESxCOLS))
		spaces_trap(1, buf, buflen);
	if ((UINT32) buf < (UINT32) shm->screen)
		spaces_trap(2, buf, buflen);
	if ((UINT32) (buf + buflen) > (((UINT32) shm->screen) + LINESxCOLS))
		spaces_trap(3, buf, buflen);
	if ((UINT32) (buf + buflen) < (UINT32) shm->screen)
		spaces_trap(4, buf, buflen);
#endif

	if (!buflen)
		return;

#ifdef DEFENSIVE
	if ((UINT32) buf < (UINT32) shm->screen)
	{
		ff(se,">< defensive 1\r\n");
		return;
	}
	if ((UINT32) (buf + buflen) > (((UINT32) shm->screen) + LINESxCOLS))
	{
		ff(se,">< defensive 2\r\n");
		return;
	}
#endif

	memset(buf, SPACE, buflen);

}							 /* end of spaces */

/*+-------------------------------------------------------------------------
	ansi_SGR() - Set Graphics Rendition

The DOS ANSI world expects to be able to be able to chain 0,1 and
3x,4x params together with semicolons.

  Supported modifiers for non-ansi terminals
  0       normal
  1       bold
  4       underscore
  5       blink
  7       reverse video
--------------------------------------------------------------------------*/
void
ansi_SGR()
{
	int itmp;
	char *cp;
	char SGRstr[MAX_ANSI_LEN];
	char *token;
	char *str_token();

#ifdef ANSI_DEBUG_SEQ
	if (wfp)
		ff(wfp, "SGR\n");
#endif

	if (!tty_is_multiscreen)
	{
		ansibuf[ansilen - 1] = 0;	/* get rid of 'm' */
		cp = ansibuf + 1;	 /* get rid of '[' */
		if (!strlen(cp))
			goto SGR_0;
		while (token = str_token(cp, ";"))
		{
			cp = (char *)0;	 /* further calls to str_token need NULL */
			switch (atoi(token))
			{
				default:
				case 0:	 /* normal */
				  SGR_0:
					tcap_stand_end();
					tcap_blink_off();
					tcap_underscore_off();
					tcap_bold_off();
					break;
				case 1:	 /* bold */
					tcap_bold_on();
					break;
				case 4:	 /* underscore */
					tcap_underscore_on();
					break;
				case 5:	 /* blink */
					tcap_blink_on();
					break;
				case 7:	 /* reverse video */
					tcap_stand_out();
					break;
			}
		}
		return;
	}

	if (ansilen <= 3)		 /* 'ESC[<0-9>m' and 'ESC[m' - quickly handled */
	{
		rcvrdisp(&esc, 1);
		rcvrdisp(ansibuf, ansilen);
		return;
	}

/* check XENIX 'ESC[<2,3,7>m' extensions */
	switch (itmp = atoi(ansibuf + 1))
	{
		case 7:			 /* XENIX 'ESC[7;<0-15>;<0-15>m' set
							  * fore/background color */
			itmp = atoi(ansibuf + 3);	/* second parameter */
			if (itmp > 15)	 /* not XENIX extension */
				break;
			/* fall through */
		case 2:			 /* XENIX 'ESC[2;<0-15>;<0-15>m' set
							  * fore/background color */
		case 3:			 /* XENIX 'ESC[3;<0-1>m' color only set/clear
							  * blink */
			rcvrdisp(&esc, 1);
			rcvrdisp(ansibuf, ansilen);
			return;
		default:
			break;
	}

/* not XENIX extension */
	ansibuf[ansilen - 1] = 0;/* get rid of 'm' */
	cp = ansibuf + 1;		 /* get rid of '[' */

	while (token = str_token(cp, ";"))
	{
		cp = (char *)0;		 /* further calls to str_token need NULL */
		sprintf(SGRstr, "\033[%sm", token);
		rcvrdisp(SGRstr, strlen(SGRstr));
	}

}							 /* end of ansi_SGR */

/*+-------------------------------------------------------------------------
	ansi_CUP() - cursor position (also HVP horiz/vertical position)
--------------------------------------------------------------------------*/
void
ansi_CUP()
{
	UINT param_count = 0;
	char ansicopy[MAX_ANSI_LEN];
	char *cp = ansicopy;
	char *token;
	char *str_token();

	strcpy(cp, ansibuf + 1);
	*(cp + ansilen - 2) = 0;

	while (token = str_token(cp, ";"))
	{
		cp = (char *)0;		 /* further calls to str_token need NULL */
		switch (++param_count)
		{
			case 1:
				shm->cursor_y = atoi(token) - 1;
				break;
			case 2:
				shm->cursor_x = atoi(token) - 1;
				break;
		}
	}
#ifdef ANSI_DEBUG_SEQ
	if (wfp)
	{
		ff(wfp, "CUP: param_count=%u p1=%u p2=%u\n", param_count,
			shm->cursor_y + 1, shm->cursor_x + 1);
	}
#endif

	switch (param_count)
	{
		case 0:
			shm->cursor_y = 0;
		case 1:
			shm->cursor_x = 0;
	}
	if (shm->cursor_x >= tcap_COLS)
		shm->cursor_x = tcap_COLS - 1;
	if (shm->cursor_y >= tcap_LINES)
		shm->cursor_y = tcap_LINES - 1;

	if (!tty_is_multiscreen)
		tcap_cursor(shm->cursor_y, shm->cursor_x);

}							 /* end of ansi_CUP */

/*+-------------------------------------------------------------------------
	ansi_CUU() - cursor up
--------------------------------------------------------------------------*/
void
ansi_CUU()
{
	UINT param;
	UINT y;

	if (ansilen == 2)		 /* no param */
		param = 1;
	else
		param = atoi(ansibuf + 1);

#ifdef ANSI_DEBUG_SEQ
	if (wfp)
		ff(wfp, "CU: param=%u\n", param);
#endif

	y = shm->cursor_y - param;
	if (y >= tcap_LINES)	 /* unsigned comparison */
		y = 0;

	if (y != shm->cursor_y)
	{
		shm->cursor_y = y;
		if (!tty_is_multiscreen)
			tcap_cursor(shm->cursor_y, shm->cursor_x);
	}

}							 /* end of ansi_CUU */

/*+-------------------------------------------------------------------------
	ansi_CUD() - cursor down (also VPR vertical position relative)
--------------------------------------------------------------------------*/
void
ansi_CUD()
{
	UINT param;
	UINT y;

	if (ansilen == 2)		 /* no param */
		param = 1;
	else
		param = atoi(ansibuf + 1);

#ifdef ANSI_DEBUG_SEQ
	if (wfp)
		ff(wfp, "CUD: param=%u\n", param);
#endif

	y = shm->cursor_y + param;
	if (y >= tcap_LINES)
	{
		y = tcap_LINES - 1;
		if ((shm->cursor_y == tcap_LINES - 1) && !tty_is_multiscreen)
		{
			process_rcvd_char(0x0A);
			return;
		}
	}

	if (y != shm->cursor_y)
	{
		shm->cursor_y = y;
		if (!tty_is_multiscreen)
			tcap_cursor(shm->cursor_y, shm->cursor_x);
	}

}							 /* end of ansi_CUD */

/*+-------------------------------------------------------------------------
	ansi_CUF() - cursor forward (also HPR horizontal position relative)
--------------------------------------------------------------------------*/
void
ansi_CUF()
{
	UINT param;
	UINT x;

	if (ansilen == 2)		 /* no param */
		param = 1;
	else
		param = atoi(ansibuf + 1);

#ifdef ANSI_DEBUG_SEQ
	if (wfp)
		ff(wfp, "CUF: param=%u\n", param);
#endif

	x = shm->cursor_x + param;
	if (x >= tcap_COLS)
		x = tcap_COLS - 1;

	if (x != shm->cursor_x)
	{
		shm->cursor_x = x;
		if (!tty_is_multiscreen)
			tcap_cursor(shm->cursor_y, shm->cursor_x);
	}

}							 /* end of ansi_CUF */

/*+-------------------------------------------------------------------------
	ansi_CUB() - cursor forward
--------------------------------------------------------------------------*/
void
ansi_CUB()
{
	UINT param;
	UINT x;

	if (ansilen == 2)		 /* no param */
		param = 1;
	else
		param = atoi(ansibuf + 1);

#ifdef ANSI_DEBUG_SEQ
	if (wfp)
		ff(wfp, "CUB: param=%u\n", param);
#endif

	x = shm->cursor_x - param;
	if (x >= tcap_COLS)		 /* unsigned comparison */
		x = 0;

	if (x != shm->cursor_x)
	{
		shm->cursor_x = x;
		if (!tty_is_multiscreen)
			tcap_cursor(shm->cursor_y, shm->cursor_x);
	}

}							 /* end of ansi_CUB */

/*+-------------------------------------------------------------------------
	ansi_DSR() - device status report

  ESC [ Ps n  - Device Status Report (DSR)           ECU Response
        Ps = 5 -> Status Report                      ESC [ 0 n
        Ps = 6 -> Report Cursor Position (CPR)       ESC [ r ; c R

--------------------------------------------------------------------------*/
void
ansi_DSR()
{
	UINT param;
	char response_buf[MAX_ANSI_LEN];
	char *response = 0;

	if (ansilen < 2)		 /* no param */
		param = 0;
	else
		param = atoi(ansibuf + 1);

#ifdef ANSI_DEBUG_SEQ
	if (wfp)
	{
		ff(wfp, "DSR: param=%u ", param);
		if (!param && (ansilen == 3))
			ff(wfp, ": ignoring \"ESC 0 ] n\"");
		ff(wfp, "\n");
	}
#endif

	switch (param)
	{
		case 0:			 /* sanity */
		case 5:			 /* sanity */
			response = "\033[0n";
			break;

		case 6:			 /* report cursor position */
			sprintf(response_buf, "\033[%d;%dR",
				shm->cursor_y + 1, shm->cursor_x + 1);
			response = response_buf;
			break;
	}

	if (response)
	{
		Nap(300L);
		write(shm->Liofd, response, strlen(response));
	}

#ifdef ANSI_DEBUG_SEQ
	if (response)
	{
		char s80[80];

		sprintf(s80, "strlen(DSR response) = %d", strlen(response));
		hex_dump_fp(wfp, response, strlen(response), s80, 0);
	}
#endif

}							 /* end of ansi_DSR */

/*+-------------------------------------------------------------------------
	ansi_ED() - erase in display
--------------------------------------------------------------------------*/
void
ansi_ED()
{
	UINT param;
	UINT y;

	if (ansilen == 2)		 /* no param */
		param = 0;
	else
		param = atoi(ansibuf + 1);

#ifdef ANSI_DEBUG_SEQ
	if (wfp)
		ff(wfp, "ED: param=%u LINESxCOLS=%d\n", param,LINESxCOLS);
#endif

	switch (param)
	{
		case 0:			 /* erase to end of display */
			spaces(&shm->screen[shm->cursor_y][shm->cursor_x],
				LINESxCOLS - ((shm->cursor_y * tcap_COLS) + shm->cursor_x));
			if (!tty_is_multiscreen)
				tcap_eeod();
			break;
		case 1:			 /* erase from beginning of display */
			spaces((char *)shm->screen, (shm->cursor_y * tcap_COLS) +
				shm->cursor_x);
			if (!tty_is_multiscreen)
			{
				for (y = 0; y < shm->cursor_y - 1; y++)
				{
					tcap_cursor(y, 0);
					tcap_eeol();
				}
				if (shm->cursor_x)
				{
					tcap_cursor(shm->cursor_y, 0);
					tcap_clear_area_char(shm->cursor_x, ' ');
				}
				else
					tcap_cursor(shm->cursor_y, shm->cursor_x);
			}
			break;
		case 2:			 /* clear display */
			shm->cursor_y = 0;
			shm->cursor_x = 0;
			spaces((char *)shm->screen, LINESxCOLS);
			if (!tty_is_multiscreen)
			{
				tcap_clear_screen();
				tcap_cursor(shm->cursor_y, shm->cursor_x);
			}
			break;
	}

}							 /* end of ansi_ED */

/*+-------------------------------------------------------------------------
	ansi_EL() - erase in line
--------------------------------------------------------------------------*/
void
ansi_EL()
{
	UINT param;
	char cr = CRET;

	if (ansilen == 2)		 /* no param */
		param = 0;
	else
		param = atoi(ansibuf + 1);

#ifdef ANSI_DEBUG_SEQ
	if (wfp)
		ff(wfp, "EL: param=%u\n", param);
#endif

	switch (param)
	{
		case 2:			 /* clear line */
			shm->cursor_x = 0;
			if (!tty_is_multiscreen)
				rcvrdisp(&cr, 1);
			/* fall thru */
		case 0:			 /* erase to end of line */
			spaces(&shm->screen[shm->cursor_y][shm->cursor_x],
				tcap_COLS - shm->cursor_x);
			if (!tty_is_multiscreen)
				tcap_eeol();
			break;
		case 1:			 /* erase from beginning of line */
			spaces(&shm->screen[shm->cursor_y][0], shm->cursor_x);
			if (!tty_is_multiscreen && shm->cursor_x)
			{
				rcvrdisp(&cr, 1);
				tcap_clear_area_char(shm->cursor_x, ' ');
			}
			break;
	}

}							 /* end of ansi_EL */

/*+-------------------------------------------------------------------------
	ansi_ECH() - erase characters
--------------------------------------------------------------------------*/
void
ansi_ECH()
{
	UINT param;
	UINT screen_pos;

	if (ansilen == 2)		 /* no param */
		param = 1;
	else
		param = atoi(ansibuf + 1);

#ifdef ANSI_DEBUG_SEQ
	if (wfp)
		ff(wfp, "ECH: param=%u\n", param);
#endif

	if ((shm->cursor_x + param) >= tcap_COLS)
		return;

	screen_pos = (shm->cursor_y * tcap_COLS) + shm->cursor_x;
	mem_cpy((char *)shm->screen + screen_pos,
		(char *)shm->screen + screen_pos + param, param);
	spaces((char *)shm->screen + ((shm->cursor_y + 1) * tcap_COLS) -
		param, param);

	if (!tty_is_multiscreen)
		tcap_delete_chars(param);

}							 /* end of ansi_ECH */

/*+-------------------------------------------------------------------------
	ansi_SU() - scroll up (new blank lines at the bottom)
--------------------------------------------------------------------------*/
void
ansi_SU()
{
	UINT param;
	UINT count;

	if (ansilen == 2)		 /* no param */
		param = 1;
	else
		param = atoi(ansibuf + 1);

	if (param > tcap_LINES)
		param = tcap_LINES;

#ifdef ANSI_DEBUG_SEQ
	if (wfp)
		ff(wfp, "SU: param=%u\n", param);
#endif

	if (!param)
		return;

#ifdef ANSI_DEBUG_SEQ
	if (wfp)
	{
		fprintf(wfp, "SU: param=%u y,x=%d,%d\n", param,
			shm->cursor_y, shm->cursor_x);
	}
#endif

	count = tcap_COLS * param;
	mem_cpy((char *)shm->screen, (char *)shm->screen + count,
		LINESxCOLS - count);
	spaces((char *)shm->screen + LINESxCOLS - count, count);

	if (!tty_is_multiscreen)
	{
		tcap_cursor(tcap_LINES - 1, 0);
		while (param--)
			rcvrdisp("\n", 1);
		tcap_cursor(shm->cursor_y, shm->cursor_x);
	}

}							 /* end of ansi_SU */

/*+-------------------------------------------------------------------------
	ansi_SD() - scroll down (new blank lines at the top)
--------------------------------------------------------------------------*/
void
ansi_SD()
{
	UINT param;
	UINT count;

	if (ansilen == 2)		 /* no param */
		param = 1;
	else
		param = atoi(ansibuf + 1);

	if (param > tcap_LINES)
		param = tcap_LINES;

#ifdef ANSI_DEBUG_SEQ
	if (wfp)
		ff(wfp, "SD: param=%u\n", param);
#endif

	if (!param)
		return;

#ifdef ANSI_DEBUG_SEQ
	if (wfp)
	{
		fprintf(wfp, "SU: param=%u y,x=%d,%d\n", param,
			shm->cursor_y, shm->cursor_x);
	}
#endif

	count = tcap_COLS * param;
	mem_cpy((char *)shm->screen, (char *)shm->screen + count,
		LINESxCOLS - count);
	spaces((char *)shm->screen + LINESxCOLS - count, count);

	if (!tty_is_multiscreen)
	{
		tcap_cursor(0, 0);
		tcap_insert_lines(param);
		tcap_cursor(shm->cursor_y, shm->cursor_x);
	}

}							 /* end of ansi_SD */

/*+-------------------------------------------------------------------------
	ansi_HPA() - horizontal position absolute
--------------------------------------------------------------------------*/
void
ansi_HPA()
{
	UINT param;

	if (ansilen == 2)		 /* no param */
		param = 1;
	else
		param = atoi(ansibuf + 1);

#ifdef ANSI_DEBUG_SEQ
	if (wfp)
		ff(wfp, "HPA: param=%u\n", param);
#endif

	if (param >= tcap_LINES)
		return;

	if ((unsigned)(shm->cursor_x = param) >= (unsigned)tcap_COLS)
		shm->cursor_x = tcap_COLS - 1;

	if (!tty_is_multiscreen)
		tcap_cursor(shm->cursor_y, shm->cursor_x);

}							 /* end of ansi_HPA */

/*+-------------------------------------------------------------------------
	ansi_VPA() - vertical position absolute
--------------------------------------------------------------------------*/
void
ansi_VPA()
{
	UINT param;

	if (ansilen == 2)		 /* no param */
		param = 1;
	else
		param = atoi(ansibuf + 1);

#ifdef ANSI_DEBUG_SEQ
	if (wfp)
		ff(wfp, "VPA: param=%u\n", param);
#endif

	if (param >= tcap_COLS)
		return;

	if ((unsigned)(shm->cursor_y = param) >= (unsigned)tcap_LINES)
		shm->cursor_y = tcap_LINES - 1;

	if (!tty_is_multiscreen)
		tcap_cursor(shm->cursor_y, shm->cursor_x);

}							 /* end of ansi_VPA */

/*+-------------------------------------------------------------------------
	ansi_IL() - insert lines
--------------------------------------------------------------------------*/
void
ansi_IL()
{
	UINT param;
	UINT count;
	UINT screen_pos;

	if (ansilen == 2)		 /* no param */
		param = 1;
	else
		param = atoi(ansibuf + 1);

#ifdef ANSI_DEBUG_SEQ
	if (wfp)
		ff(wfp, "IL: param=%u\n", param);
#endif

	if ((shm->cursor_y + param) >= tcap_LINES)
		return;

	count = tcap_COLS * param;
	screen_pos = shm->cursor_y * tcap_COLS;
	mem_cpy((char *)shm->screen + screen_pos + count,
		(char *)shm->screen + screen_pos,
		LINESxCOLS - screen_pos - count);
	spaces((char *)shm->screen + screen_pos, count);

	if (!tty_is_multiscreen)
		tcap_insert_lines(param);

}							 /* end of ansi_IL */

/*+-------------------------------------------------------------------------
	ansi_ICH() - insert characters
--------------------------------------------------------------------------*/
void
ansi_ICH()
{
	UINT param;
	UINT count;
	UINT screen_pos;

	if (ansilen == 2)		 /* no param */
		param = 1;
	else
		param = atoi(ansibuf + 1);

	if (param > tcap_COLS - shm->cursor_x)
		param = tcap_COLS - shm->cursor_x;

#ifdef ANSI_DEBUG_SEQ
	if (wfp)
		ff(wfp, "ICH: param=%u\n", param);
#endif

	if (!param)
		return;

	screen_pos = (shm->cursor_y * tcap_COLS) + shm->cursor_x;
	count = tcap_COLS - shm->cursor_x - param;
	mem_cpy((char *)shm->screen + screen_pos + param,
		(char *)shm->screen + screen_pos, count);
	spaces((char *)shm->screen + screen_pos, param);

	if (!tty_is_multiscreen)
		tcap_insert_chars(param);

}							 /* end of ansi_ICH */

/*+-------------------------------------------------------------------------
	ansi_DL() - delete lines
--------------------------------------------------------------------------*/
void
ansi_DL()
{
	UINT param;
	UINT count;
	UINT screen_pos;

	if (ansilen == 2)		 /* no param */
		param = 1;
	else
		param = atoi(ansibuf + 1);

	if (param > (tcap_LINES - shm->cursor_y))
		param = tcap_LINES - shm->cursor_y;

#ifdef ANSI_DEBUG_SEQ
	if (wfp)
		ff(wfp, "DL: param=%u\n", param);
#endif

	if (!param)
		return;

	count = tcap_COLS * param;
	screen_pos = shm->cursor_y * tcap_COLS;
	mem_cpy((char *)shm->screen + screen_pos,
		(char *)shm->screen + screen_pos + count,
		LINESxCOLS - screen_pos - count);
	spaces((char *)shm->screen + LINESxCOLS - count, count);

	if (!tty_is_multiscreen)
		tcap_delete_lines(param);

}							 /* end of ansi_DL */

/*+-------------------------------------------------------------------------
	ansi_DCH() - delete characters
--------------------------------------------------------------------------*/
void
ansi_DCH()
{
	UINT param;
	UINT count;
	UINT screen_pos;

	if (ansilen == 2)		 /* no param */
		param = 1;
	else
		param = atoi(ansibuf + 1);

	if (ansilen == 2)		 /* no param */
		param = 1;
	else
		param = atoi(ansibuf + 1);

	if (param > tcap_COLS - shm->cursor_x)
		param = tcap_COLS - shm->cursor_x;

#ifdef ANSI_DEBUG_SEQ
	if (wfp)
		ff(wfp, "DCH: param=%u\n", param);
#endif

	if (!param)
		return;

	screen_pos = (shm->cursor_y * tcap_COLS) + shm->cursor_x;
	count = tcap_COLS - shm->cursor_x - param;
	mem_cpy((char *)shm->screen + screen_pos,
		(char *)shm->screen + screen_pos + param, count);
	screen_pos = ((shm->cursor_y + 1) * tcap_COLS) - param;
	spaces((char *)shm->screen + screen_pos, param);

	if (!tty_is_multiscreen)
		tcap_delete_chars(param);

}							 /* end of ansi_DCH */

/*+-------------------------------------------------------------------------
	ansi_CPL() - cursor to previous line
--------------------------------------------------------------------------*/
void
ansi_CPL()
{
	UINT param;

	if (ansilen == 2)		 /* no param */
		param = 1;
	else
		param = atoi(ansibuf + 1);

	if ((shm->cursor_y -= param) >= tcap_LINES)	/* unsigned comparison */
		shm->cursor_y = 0;
	shm->cursor_x = 0;

#ifdef ANSI_DEBUG_SEQ
	if (wfp)
		ff(wfp, "CPL: param=%u\n", param);
#endif

	if (!tty_is_multiscreen)
		tcap_cursor(shm->cursor_y, shm->cursor_x);

}							 /* end of ansi_CPL */

/*+-------------------------------------------------------------------------
	ansi_CNL() - cursor to next line
--------------------------------------------------------------------------*/
void
ansi_CNL()
{
	UINT param;

	if (ansilen == 2)		 /* no param */
		param = 1;
	else
		param = atoi(ansibuf + 1);

	if ((shm->cursor_y += param) >= tcap_LINES)
		shm->cursor_y = tcap_LINES - 1;
	shm->cursor_x = 0;

#ifdef ANSI_DEBUG_SEQ
	if (wfp)
		ff(wfp, "CNL: param=%u\n", param);
#endif

	if (!tty_is_multiscreen)
		tcap_cursor(shm->cursor_y, shm->cursor_x);

}							 /* end of ansi_CNL */

/*+-------------------------------------------------------------------------
	saved_cursor_save_cursor() - nice but unfortunate IBM extension

I can't find this used anywhere but in the DOS world.  Supporting this
pair of sequences is what started this whole complex mess.
--------------------------------------------------------------------------*/
void
saved_cursor_save_cursor()
{
	saved_cursor_y = shm->cursor_y;
	saved_cursor_x = shm->cursor_x;
}							 /* end of saved_cursor_save_cursor */

/*+-------------------------------------------------------------------------
	saved_cursor_restore_cursor() - nice but unfortunate IBM extension

I can't find this used anywhere but in the DOS world.  Supporting this
pair of sequences is what started this whole complex mess.
--------------------------------------------------------------------------*/
void
saved_cursor_restore_cursor()
{
	shm->cursor_y = saved_cursor_y;
	shm->cursor_x = saved_cursor_x;
	tcap_cursor(shm->cursor_y, shm->cursor_x);
}							 /* end of saved_cursor_restore_cursor */

/*+-------------------------------------------------------------------------
	rcvd_ESC() - ESC seen-prepare to accumulate ansi sequence
--------------------------------------------------------------------------*/
void
rcvd_ESC()
{
#ifdef ANSI_DEBUG
	if (wfp)
		fprintf(wfp, "ESC ");
#endif

	ansi = ansibuf;
	*ansi = 0;
	ansilen = 0;
	in_ansi_accumulation = 1;

}							 /* end of rcvd_ESC */

/*+-------------------------------------------------------------------------
	is_ansi_terminator(rchar) - is character terminator for ansi sequence?
--------------------------------------------------------------------------*/
int
is_ansi_terminator(rchar)
UINT rchar;
{
	return (isalpha(rchar) || strchr("@>=", rchar));
}							 /* end of is_ansi_terminator */

/*+-------------------------------------------------------------------------
	accumulate_ansi_sequence(rchar)
--------------------------------------------------------------------------*/
void
accumulate_ansi_sequence(rchar)
UINT rchar;
{
	if (ansilen == (MAX_ANSI_LEN - 2))
	{
		in_ansi_accumulation = 0;
		return;
	}

#ifdef ANSI_DEBUG_AAS
	if (wfp)
	{
		fprintf(wfp, "\naas: %02x %c ansilen=%d",
			rchar, (rchar & 0x7F < SPACE) ? '.' : (rchar & 0x7F), ansilen);
	}
#endif

	*ansi++ = (uchar) rchar;
	*ansi = 0;
	ansilen++;

}							 /* end of accumulate_ansi_sequence */

/*+-------------------------------------------------------------------------
	process_ansi_sequence() - a full ansi sequence is to be decoded
--------------------------------------------------------------------------*/
void
process_ansi_sequence()
{
	int itmp;

#ifdef ANSI_DEBUG
	if (wfp)
	{
		fprintf(wfp, "\nANSI SEQUENCE: '%s' len=%d y,x=%d,%d:: ",
			ansibuf, ansilen, shm->cursor_y, shm->cursor_x);
	}
#endif

	if (!in_ansi_accumulation)
		return;
	in_ansi_accumulation = 0;

	itmp = 1;				 /* assume write needed */
	if (ansibuf[0] == '[')
	{
		switch (ansibuf[ansilen - 1])
		{
			case '@':
				ansi_ICH();	 /* insert characters */
				break;
			case 'A':
				ansi_CUU();	 /* cursor up */
				break;
			case 'B':
				ansi_CUD();	 /* cursor down */
				break;
			case 'C':
				ansi_CUF();	 /* cursor forward */
				break;
			case 'D':
				ansi_CUB();	 /* cursor backward */
				break;
			case 'E':
				ansi_CNL();	 /* cursor to next line */
				break;
			case 'F':
				ansi_CPL();	 /* cursor to previous line */
				break;
			case 'H':
				ansi_CUP();	 /* cursor position */
				break;
			case 'J':
				ansi_ED();	 /* erase in display */
				break;
			case 'K':
				ansi_EL();	 /* erase in line */
				break;
			case 'L':
				ansi_IL();	 /* insert lines */
				break;
			case 'M':
				ansi_DL();	 /* delete lines */
				break;
			case 'P':
				ansi_DCH();	 /* delete characters */
				break;
			case 'S':
				ansi_SU();	 /* scroll up */
				break;
			case 'T':
				ansi_SD();	 /* scroll down */
				break;
			case 'X':
				ansi_ECH();	 /* erase characters */
				break;
			case '`':
				ansi_HPA();	 /* horizontal position absolute */
				break;
			case 'a':
				ansi_CUF();	 /* HPR - horizontal position relative */
				break;
			case 'd':
				ansi_VPA();	 /* vertical position absolute */
				break;
			case 'e':
				ansi_CUD();	 /* VPR - vertical position relative */
				break;
			case 'f':
				ansi_CUP();	 /* HVP - horizontal vertical position */
				break;
			case 'm':
				ansi_SGR();	 /* set graphics rendition */
				itmp = 0;
				break;
			case 'n':
				ansi_DSR();	 /* device status report */
				itmp = 0;
				break;
			case 's':
				saved_cursor_save_cursor();
				itmp = 0;
				break;
			case 'u':
				saved_cursor_restore_cursor();
				itmp = 0;
				break;
#ifdef FUTURES
			case 'h':
				ansi_SM();	 /* Set Mode: SCO: lock keybd MSDOS: lots */
				break;
			case 'i':
				ansi_MC();	 /* Media Copy: send screen to line */
				break;
			case 'l':
				ansi_RM();	 /* Reset Mode: SCO: unlock keybd MSDOS:lots */
				break;
#endif /* FUTURES */
			default:
				break;
		}
	}

/* if proper ansi console and indicated, write the buffer to the screen */
	if (tty_is_multiscreen && itmp)
	{
		rcvrdisp(&esc, 1);
		rcvrdisp(ansibuf, ansilen);
	}

#ifdef ANSI_DEBUG
	if (wfp)
	{
		fprintf(wfp, "pas: new cursor y,x=%d,%d\n",
			shm->cursor_y, shm->cursor_x);
	}
#endif
}							 /* end of process_ansi_sequence */

/*+-------------------------------------------------------------------------
	rcvr_ansi_filter(rchar)
--------------------------------------------------------------------------*/
int
rcvr_ansi_filter(rchar)
int rchar;
{
	int itmp;

	/*
	 * video control sequences
	 */
	if (rchar == ESC)
	{
		rcvd_ESC();
		return (1);
	}
	else if (in_ansi_accumulation)
	{

		/*
		 * we handle some VT-100 two character sequences (ESC + one
		 * character)
		 */
		int consumed = 0;

		if (!ansilen)
		{
			switch (rchar & 0x7F)
			{
				case '7':	 /* VT100 save cursor position */
					consumed = 1;
					saved_cursor_save_cursor();
					break;
				case '8':	 /* VT100 save cursor position */
					consumed = 1;
					saved_cursor_restore_cursor();
					break;
			}
			if (consumed)
			{
#ifdef ANSI_DEBUG
				if (wfp)
					fprintf(wfp, "single: '%c'\n", rchar);
#endif
				in_ansi_accumulation = 0;
				return (1);
			}
		}

		/*
		 * other sequences go through the ANSI decode path
		 */
		accumulate_ansi_sequence(rchar);
		if (is_ansi_terminator(rchar))
			process_ansi_sequence();
		return (1);
	}

	/*
	 * the bread and butter of the receiver: print printable characters
	 * and obey formatting characters
	 */
	if (rchar < SPACE)
	{
		switch (rchar)
		{
			case CTL_L:
				spaces((char *)shm->screen, LINESxCOLS);
				shm->cursor_y = 0;
				shm->cursor_x = 0;
				break;

			case BS:
				if (shm->cursor_x)
					shm->cursor_x--;
				break;

			case NL:
				if (shm->cursor_y != tcap_LINES - 1)
					shm->cursor_y++;
				else
				{
					mem_cpy((char *)shm->screen,
						(char *)shm->screen + tcap_COLS,
						LINESxCOLS - tcap_COLS);
					spaces(&shm->screen[shm->cursor_y][0], tcap_COLS);
				}
				break;

			case CRET:
				shm->cursor_x = 0;
				break;

			case TAB:
				itmp = 8 - (shm->cursor_x % 8);
				shm->cursor_x += itmp;
				if (shm->cursor_x >= tcap_COLS)
				{
					shm->cursor_x = 0;
					if (++shm->cursor_y >= tcap_LINES)
						shm->cursor_y = tcap_LINES - 1;
				}
				spaces(&shm->screen[shm->cursor_y][shm->cursor_x], itmp);
				break;

		}
	}
	else
	{
		shm->screen[shm->cursor_y][shm->cursor_x++] = (uchar) rchar;
		if (shm->cursor_x >= tcap_COLS)
		{
			shm->cursor_x = 0;
			if (shm->cursor_y != tcap_LINES - 1)
				shm->cursor_y++;
			else
			{
				mem_cpy((char *)shm->screen, (char *)shm->screen + tcap_COLS,
					LINESxCOLS - tcap_COLS);
				spaces(&shm->screen[shm->cursor_y][shm->cursor_x], tcap_COLS);
			}
		}
	}

#ifdef ANSI_DEBUG_TEXT
	if (wfp)
	{
		if ((rchar & 0x7F) == NL)
			fputs("\n", wfp);
		else
			fputc(((rchar & 0x7F) < SPACE) ? '.' : (rchar & 0x7F), wfp);
	}
#endif

	return (0);

}							 /* end of rcvr_ansi_filter */

#endif /* CFG_NoAnsiEmulation */

/* end of ecurcvr.c */
/* vi: set tabstop=4 shiftwidth=4: */
