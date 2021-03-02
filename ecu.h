/*+-----------------------------------------------------------------------
	ecu.h -- TuckerWare Extended Calling Unit
	wht@wht.net
------------------------------------------------------------------------*/
/*+:EDITS:*/
/*:04-26-2000-11:15-wht@bob-RELEASE 4.42 */
/*:01-08-2000-12:52-wht@menlo-rethink sockserve I/O */
/*:01-02-2000-14:01-wht@menlo-add proc_option_continueOnLineError */
/*:11-03-1997-02:16-wht@kepler-4.08a-add proc_option_localvars */
/*:02-09-1997-19:37-wht@yuriatin-mv find_procedure defn here */
/*:01-24-1997-02:36-wht@yuriatin-SOURCE RELEASE 4.00 */
/*:09-11-1996-19:59-wht@yuriatin-3.48-major telnet,curses,structural overhaul */
/*:09-09-1996-03:39-wht@yuriatin-add LINST_TELNETFAIL */
/*:01-27-1996-20:25-wht@n4hgf-move child_pid decl here */
/*:11-23-1995-11:20-wht@kepler-source control 3.37 for tsx-11 */
/*:11-14-1995-10:23-wht@kepler-3.37.80-source control point: SOCKETS */
/*:11-13-1995-12:16-wht@kepler-need_rcvr_restart now func moved to ecurcvr.c */
/*:11-03-1995-17:32-wht@wwtp1-add enum linst func decls */
/*:10-14-1995-17:08-wht@kepler-'struct termio' to 'struct TERMIO' */
/*:06-12-1995-15:27-wht@n4hgf-add externs for uucp euid globals */
/*:03-12-1995-00:40-wht@kepler-eliminate obsolete CURR_DIRSIZ */
/*:01-12-1995-15:19-wht@n4hgf-apply Andrew Chernov 8-bit clean+FreeBSD patch */
/*:05-04-1994-04:38-wht@n4hgf-ECU release 3.30 */
/*:03-15-1994-14:23-wht@n4hgf-correct doc: ecuxenix.c now ecugrabbag.c */
/*:01-16-1994-17:01-wht@n4hgf-restore SIGCHLD equate for NetBSD */
/*:12-12-1993-13:32-wht@n4hgf-MOTSVR3 port */
/*:12-12-1993-13:03-wht@fep-use ecu_time.h for Ftime/gettimeofday */
/*:12-11-1993-11:27-wht@n4hgf-lost an endif */
/*:12-10-1993-22:09-wht@n4vu-more Linux fixes + no default sun termios */
/*:11-14-1993-15:47-wht@n4hgf-more hpux port */
/*:10-02-1993-22:45-wht@n4hgf-add SIGPTR */
/*:08-01-1993-02:12-wht@n4hgf-add got_delim to LRWT */
/*:09-13-1992-12:52-wht@n4hgf-add tty_is_scoterm */
/*:09-10-1992-13:58-wht@n4hgf-ECU release 3.20 */
/*:08-22-1992-15:38-wht@n4hgf-ECU release 3.20 BETA */
/*:08-21-1992-13:39-wht@n4hgf-rewire direct/modem device use */
/*:07-21-1992-12:09-wht@n4hgf-3.2v4 only has sys/time.h if TCPRT installed */
/*:04-17-1992-20:10-wht@gyro-default tty, baud and parity moved to config.c */
/*:03-27-1992-16:21-wht@n4hgf-re-include protection for all .h files */
/*:02-13-1992-06:35-wht@n4hgf-when port to many time() can be int/long/time_t */
/*:11-30-1991-13:46-wht@n4hgf-smap conditional compilation reorg */
/*:11-26-1991-20:17-wht@n4hgf-add shm->Ldcdwatch values */
/*:11-26-1991-19:37-wht@n4hgf-add STR_CLASSIFY */
/*:11-13-1991-16:29-wht@n4hgf-use if __STDC__ instead of defined(__STDC__) */
/*:11-11-1991-22:45-wht@n4hgf-redefinition of Ltermio's place in life */
/*:08-25-1991-14:39-wht@n4hgf-SVR4 port thanks to aega84!lh */
/*:08-13-1991-13:53-wht@n4hgf-UNIX and ISC nap() broken; XENIX still wins */
/*:08-09-1991-11:07-wht@n4hgf-configurable lock directory */
/*:07-25-1991-12:55-wht@n4hgf-ECU release 3.10 */
/*:01-25-1991-06:08-wht@n4hgf-mulltiple #define of CFG_EcuLibDir */
/*:01-22-1991-14:33-wht@n4hgf-XENIX calloc/memmove fix */
/*:01-01-1991-21:36-wht@n4hgf-add GCC implies STDC */
/*:08-14-1990-20:39-wht@n4hgf-ecu3.00-flush old edit history */

#ifndef _ecu_h
#define _ecu_h

#include "ecu_config.h"

#if !defined(STDIO_H_INCLUDED)
#include <stdio.h>
#endif

#include <signal.h>

/*
 * small dialect correction
 */
#if !defined(SIGCLD)
#define SIGCLD SIGCHLD
#endif

#include "ecu_types.h"
#include <ctype.h>
#include <errno.h>
#include <string.h>
#include <memory.h>
#include <fcntl.h>
#include "ecu_stat.h"
#include "ecu_time.h"

#ifndef CFG_HasStrerror /* let string.h do it if it will */
char *strerror();
#endif

#if !defined(OMIT_TERMIO_REFERENCES)
#include "ecutermio.h"
#endif /* OMIT_TERMIO_REFERENCES */

#include "ecushm.h"
#include "termecu.h"
#include "ttynaming.h"

/*
 * The various flavors of hardware flow control, most of them
 * flakey or incomplete .... follow the identifer road
 * (fgrep HW_FLOW_CONTROL) to see what it takes to
 * include your vendor's attempt.
 */
#if defined(RTSFLOW) && !defined(HW_FLOW_CONTROL)	/* SCO */
#define HW_FLOW_CONTROL
#endif
#if defined(CRTSFL) && !defined(HW_FLOW_CONTROL)	/* 3.2v4 and later */
#define HW_FLOW_CONTROL
#endif
#if defined(RTSXOFF) && !defined(HW_FLOW_CONTROL)	/* SVR4 */
#define HW_FLOW_CONTROL
#endif
#if defined(CRTSCTS) && !defined(HW_FLOW_CONTROL)	/* sun */
#define HW_FLOW_CONTROL
#endif

/*
 * Communication line variables that are not in shared memory
 *
 * TERMIO is defined 'termio' or 'termios' as appropriate
 *
 * One special note: Ltermio points to a buffer in shared memory
 * used to hold the line's current termio structure.  The shared
 * memory buffer is defined as a simple array of longs so that
 * friend code need not include termio.h if it does not need it.
 * What's more, due to sgtty-based curses code, XENIX versions
 * cannot include termio.h in some modules.  ecushm.c has runtime
 * code to initialize Ltermio to point to shm->Ltiobuf and to
 * make sure shm->Ltiobuf is long enough.
 */
#if defined(DECLARE_LINEVARS_PUBLIC)
#if !defined(OMIT_TERMIO_REFERENCES)
struct TERMIO *Ltermio;		 /* attributes for the line to remote */
#endif
uchar Ldial_debug_level;

#else
#if !defined(OMIT_TERMIO_REFERENCES)
extern struct TERMIO *Ltermio;

#endif
extern uchar Ldial_debug_level;

#endif

extern int zero_length_read_detected;	/* see lgetc_xmtr in eculine.c */

#ifdef M_I286
long time();

#endif

long Nap();

#if __STDC__ == 1				 /* sigh ... malloc and such types;
							  * just a guess */
#define VTYPE void
#define VOLATILE volatile
#ifndef CFG_SigType			 /* doesnt belong here */
#define CFG_SigType void
#endif
#else
#define VTYPE char
#define VOLATILE
#ifndef CFG_SigType			 /* doesnt belong here */
#define CFG_SigType int
#endif
#endif

#include "smap.h"
#if defined(CFG_Malloc3X)
#include <malloc.h>
#else
VTYPE *malloc();
VTYPE *calloc();
VTYPE *realloc();

#endif

typedef UINT (*PFU) ();		 /* pointer to function returning UINT */
typedef char (*PFC) ();		 /* pointer to function returning char */
typedef int (*PFI) ();		 /* pointer to function returning integer */
typedef long (*PFL) ();		 /* pointer to function returning long */
typedef void (*PFV) ();		 /* pointer to function returning nothing */
typedef CFG_SigType (*SIGPTR) (); /* pointer to signal handler */

/*
 * undef M_XENIX must come after any system header inclusion
 */
#if defined(M_UNIX)
#undef M_XENIX				 /* now can truly distinquish between SCO
							  * XENIX and UNIX */
#endif /* M_UNIX */

/*
 * for better source line utilization, frequent use of
 * fprintf and stderr warrants the following
 */
#define pf	printf
#define ff	fprintf
#define se	stderr
#define so	stdout

/*
 * console tty information
 */
extern int tty_is_multiscreen;	/* SCO multiscreen */
extern int tty_is_scoterm;	 /* SCO scoterm */
extern int tty_is_pty;		 /* bursty network connection? */
extern int tty_not_char_special;	/* /dev/null not considered char
									 * special */

/*
 * uucp euid control
 */
extern int setuid_uucp;		 /* true if ecu is owned by uucp and chmod +s */
extern short uid;
extern short euid;
extern short uid_uucp;

/*
 * current working directory of process
 */
extern char curr_dir[ECU_MAXPN];

/*
 * useful macros
 */
#undef max					 /* just in case they ... */
#undef min					 /* ... were already defined */
#define max(a,b) (((a) > (b)) ? (a) : (b))
#define min(a,b) (((a) < (b)) ? (a) : (b))

/*
 * decide how to write to logfile
 */
#define LOGPUTC putc		 /* fputc() or putc() */

#define TTYIN	0
#define TTYOUT  1			 /* ditto tty output */
#define TTYERR  2			 /* ditty tty output error channel */

/*
 * xbell codes (see ecugrabbag.c)
 */
enum xbell
{
	XBELL_DONE = 1,			 /* octaves or morse 'd' */
	XBELL_ATTENTION,		 /* morse .-.-.- ATTENTION */
	XBELL_C,				 /* morse -.-. C */
	XBELL_3T				 /* 3 Ts --- really 'o' */
};

/*
 * lopen() and related routines error codes
 */
enum linst
{
	LINST_OK = 0,			 /* no error */
	LINST_INVALID = -1000,	 /* for invalid tty name */
	LINST_UNKPID,			 /* unknown pid using line */
	LINST_LCKERR,			 /* lock file open error */
	LINST_NODEV,			 /* device does not exist */
	LINST_OPNFAIL,			 /* could not access line */
	LINST_ALREADY,			 /* line already open */
	LINST_ENABLED,			 /* line enabled for login */
	LINST_ENABLED_IN_USE,	 /* line in use by incoming login */
	LINST_DIALOUT_IN_USE,	 /* line in use by another dial out */
	LINST_NOPTY,			 /* pty not supported */
	LINST_WEGOTIT,			 /* not really an error: we already own the
							  * line (used by check_utmp()) */
	LINST_ECUUNGETTY,		 /* ecuungetty unexpected response */
	LINST_ECUUNGETTY2,		 /* ecuungetty execution error */
	LINST_NOTCHR,			 /* not a character special */
	LINST_TELNETFAIL		 /* telnet open failed */
};

char *LINST_text();			 /* routine to return text for error code */
enum linst lock_tty();
enum linst line_lock_status();
enum linst telnet_open();
char *find_procedure();
char *base_name();
char *erc_text();

extern char lopen_err_str[];

/*
 * filename sizes
 */
#define PHONEDIR_NAME_SIZE 256	/* phone directory */

/*
 * in case errno.h doesn't pick this up
 */
extern int errno;

char *getenv();

char *graphic_char_text(); /* ecuutil.c */

/*
 * process IDs
 */
extern CFG_PidType xmtr_pid;
extern CFG_PidType child_pid; /* ecufork.c */

/*
 * both of the following are set by xmtr_SIGINT_handler()
 * 'sigint' reset by 1st detector/processor
 * 'proc_interrupt' reset/handled by procedure monitor
 *                  (execute_esd and execute_proc)
 */
extern int sigint;

/* extern int proc_interrupt; <------ force using modules to declare it */

/*
 * procedure nesting level
 * non-zero if procedure executing (see proc.c)
 */
extern int proc_level;
extern int proc_trace;
extern int proc_option_localvars;

/*
 * 'hertz' is getenv("HZ"); that not found, the value from sys/param.h
 * 'hzmsec' is ceiling(clock period) in milliseconds
 */
extern int hertz;				 /* HZ from environ or sys/param.h */
extern UINT32 hzmsec;		 /* clock period in msec rounded up */

/*
 * lock file directory
 */
extern char *lock_dir_name;	 /* defined in ecuLCK.c */

/*
 * setcolor variables - see setcolor()
 */
extern UINT32 colors_current;
extern UINT32 colors_normal;
extern UINT32 colors_success;
extern UINT32 colors_alert;
extern UINT32 colors_error;
extern UINT32 colors_notify;

/*
 * miscellaneuous
 */
extern char *eculibdir;		 /* lib dir, i.e., "/usr/local/lib/ecu" */
extern char *ttype;			 /* getenv("TERM") */

/*
 * param to lgets_timeout in eculine.c
 */
typedef struct lrwt
{
	UINT32 to1;				 /* timeout for 1st character (granularity 20) */
	UINT32 to2;				 /* timeout for each next char (granularity
							  * 20) */
	int raw_flag;			 /* !=0, rtn full buffer, ==0, rtn filtered
							  * hayes result */
	char *buffer;			 /* buffer to fill */
	int bufsize;			 /* size of buffer */
	int count;				 /* from proc, count rcvd */
	char *delim;			 /* ending string for lgets_timeout_or_delim */
	int echo_flag;			 /* echo incoming chars to screen */
	int got_delim;			 /* set true if delim found */
}
LRWT;

/*
 * parameter structure for str_classify()
 */
typedef struct str_classify
{
	char *str;				 /* string to match */
	int min_ch;				 /* minimum characters required */
	int token;				 /* token for match */
}
STR_CLASSIFY;

/*
 * shm->Ldcdwatch values
 */
#define DCDW_OFF		0
#define DCDW_ON			1
#define DCDW_TERMINATE	2

#ifdef DEBUG_SRP
#define start_rcvr_process(flag) _start_rcvr_process(flag,__FILE__,__LINE__)
#else
#define start_rcvr_process(flag) _start_rcvr_process(flag)
#endif

#endif /* _ecu_h */

/* vi: set tabstop=4 shiftwidth=4: */
/* end of ecu.h */
