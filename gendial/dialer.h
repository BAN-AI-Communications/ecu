/*+-------------------------------------------------------------------------
	dialer.h - SCO UUCP generic dialer program definitions
	wht%n4hgf.uucp@emory.mathcs.emory.edu -or- emory!n4hgf!wht
--------------------------------------------------------------------------*/
/*+:EDITS:*/
/*:01-24-1997-02:37-wht@yuriatin-SOURCE RELEASE 4.00 */
/*:09-11-1996-20:00-wht@yuriatin-3.48-major telnet,curses,structural overhaul */
/*:11-23-1995-11:20-wht@kepler-source control 3.37 for tsx-11 */
/*:11-14-1995-10:23-wht@kepler-3.37.80-source control point: SOCKETS */
/*:05-04-1994-04:39-wht@n4hgf-ECU release 3.30 */
/*:09-10-1992-13:59-wht@n4hgf-ECU release 3.20 */
/*:08-22-1992-15:38-wht@n4hgf-ECU release 3.20 BETA */
/*:03-29-1992-12:47-cma@ifsbd-added <sys/filio.h> */
/*:03-29-1992-12:30-cma@ifsbd-added <sys/time.h> */
/*:01-26-1992-15:30-wht@n4hgf-gendial 1.2 for ecu 3.20- better hangup */
/*:07-25-1991-12:58-wht@n4hgf-ECU release 3.10 */
/*:03-12-1991-19:11-wht@n4hgf-if ecu dialing, show complete call progress */
/*:08-14-1990-20:40-wht@n4hgf-ecu3.00-flush old edit history */

#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <setjmp.h>
#include <signal.h>
#include <string.h>
#include <memory.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <sys/stat.h>
#include <sys/timeb.h>
#include <sys/time.h>
#include <termio.h>
#include <time.h>
#include <pwd.h>

#ifdef sun
#define BSD
#include <sys/filio.h>
#endif

#define ff fprintf
#define se stderr

long time();
struct passwd *getpwnam();

extern int errno;
extern char *sys_errlist[];

extern int gargc;			 /* global copy of main's argv */
extern char **gargv;		 /* global copy of main's argv */
extern char *dce_name;		 /* full pathname of ACU device */
extern char *telno;			 /* phone number if dial type request */
extern struct termio dce_termio;	/* last termio for device */
extern int Debug;			 /* set per -x flag */
extern int dialing;			 /* set while dialing in progress */
extern int dce_fd;			 /* file descriptor for dce_name */
extern int DialerExitCode;	 /* return code */
extern int status;			 /* set on errors */
extern int hangup_flag;		 /* set when DCE being hung up */
extern int hiCBAUD;			 /* highest permissible baud rate */
extern int loCBAUD;			 /* lowest permissible baud rate */
extern struct passwd *passwd;/* pointer to password entry of invoker */
extern int uid;				 /* user id of executor */
extern int uid_uucp;		 /* user id of uucp */
extern int secure;			 /* non-zero to suppress display of secure DCE
							  * traffic */
extern int ecu_calling;		 /* true if ecu dialing */

unsigned char dialer_codes[26];	/* A-Z embedded phone number codes */
/* return codes: these are set up so that an abort signal at any time can */
/* set the fail bit and return to the caller with the correct status */
#define	SUCCESS		0
#define	RC_FAIL		0x80	 /* 1 = failed to connect */
#define	RC_ENABLED	0x10	 /* enabled flag: 1 = ungetty -r required to
							  * restore the line */
#define	RC_BAUD		0x0f	 /* CBAUD connected at (0=same as dialed
							  * speed) */

/* DCE result device independent flag */
#define	rfNumeric	0x40000000

/* program exit codes */
#define	RCE_NULL	0		 /* general purpose or unknown error code */
#define	RCE_INUSE	1		 /* line in use */
#define	RCE_SIG		2		 /* signal aborted dialer */
#define	RCE_ARGS	3		 /* invalid arguments */
#define	RCE_PHNO	4		 /* invalid phone number */
#define	RCE_SPEED	5		 /* invalid baud rate -or- bad connect baud */
#define	RCE_OPEN	6		 /* can't open line */
#define	RCE_IOCTL	7		 /* ioctl error */
#define	RCE_TIMOUT	8		 /* timeout */
#define	RCE_NOTONE	9		 /* no dial tone */
#define	RCE_HANGUP	10		 /* hangup failed */
#define RCE_NORESP	11		 /* Modem didn't respond. */
#define	RCE_BUSY	13		 /* phone is busy */
#define	RCE_NOCARR	14		 /* no carrier */
#define	RCE_ANSWER	15		 /* no answer */

/* ungetty return codes */
#define	UG_NOTENAB	0
#define	UG_ENAB		1
#define	UG_RESTART	1
#define	UG_FAIL		2

/* size for various buffers */
#define MAXLINE		2048

/* How many errors allowed before call retry fails */
#define	DIAL_ERRORS_MAX	4

/* DCE message to code mapping struct ... array DCE_results of these
 * must be terminated with { (char *)0,0 } */
typedef struct dce_result
{
	char *result;
	long code;
}
DCE_RESULT;

#define DEBUG(level,fmt,arg) if (Debug >= level) fprintf(stderr,fmt,arg)
#if !defined(DBG)
#define	DBG	0
#endif

/*
 * what the hell does __STDC__ mean in reality?  An __STDC__ compiler is
 * more nouveau than an older one.  ANSI C (or 'D') just stirred new
 * food for "standard" readers who went off and did what they wanted
 * to do.  We use __STDC__ to decide between two opinions of
 * what constitute "ANSI prototypes."  As of this writing, __STDC__ is
 * defined by the UNIX (MSC 5) compiler and not by the XENIX (MSC 4)
 * compiler.  We handle the GNU C compiler too.
 */
#if defined(__STDC__) && !defined(__GNUC__)
int DCE_baud_to_CBAUD(unsigned int);
void DCE_hangup(void);
int DCE_dial(char *);
void DCE_abort(int);
void DCE_exit(int);
int DCE_argv_hook(int, char **, int, int);
int get_uucp_uid(void);
int instr(char *, char *);
void translate(char *, char *);
int decode_phone_number(char *, char *, int);
char *make_printable(unsigned char);
char *RCE_text(int);
void myexit(int);
CFG_SigType dial_abort(int);
void cleanup(int);
int SIGALRM_abort(int);
CFG_SigType SIGALRM_alert(int);
int _lread(int, int);
int lread_ignore(int);
int lread(int);
void lflush(void);
void _lputc(char);
void _lputc_paced(long, char);
void _lputs(char *);
void _lputs_paced(long, char *);
void lwrite(char *);
void lflash_DTR(void);
int call_ungetty(char);
void display_termio(struct termio *, char *);
int open_dce(void);
int main(int, char **);

#else
int DCE_baud_to_CBAUD();
void DCE_hangup();
int DCE_dial();
void DCE_abort();
void DCE_exit();
int DCE_argv_hook();
int get_uucp_uid();
int instr();
void translate();
int decode_phone_number();
char *make_printable();
char *RCE_text();
void myexit();
void cleanup();
int SIGALRM_abort();
int _lread();
int lread_ignore();
int lread();
void lflush();
void _lputc();
void _lputc_paced();
void _lputs();
void _lputs_paced();
void lwrite();
void lflash_DTR();
int call_ungetty();
void display_termio();
int open_dce();
int main();

#endif

/* vi: set tabstop=4 shiftwidth=4: */
/* end of dialer.h */
