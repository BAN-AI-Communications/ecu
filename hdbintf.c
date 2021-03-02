#if defined(SHARE_DEBUG)
#undef LOG_UNGETTY
#undef LOG_HDBDIAL
#define LOG_UNGETTY
#define LOG_HDBDIAL
#endif

 /* #define ILLOGICAL *//* has holes - does not work */

/*+-------------------------------------------------------------------------
	hdbintf.c - HDB UUCP database and /etc/utmp interface routines
	wht@wht.net

  Defined functions:
	_ungetty_return_line(line)
	add_to_ungetty_list(line)
	dialcodes_translate(phone)
	display_ungetty_list()
	dvtype_match(typespec, dvtype)
	enddlent()
	enddvent()
	getdlent()
	getdlentname(name)
	getdvbaud(baud)
	getdvent()
	getdvline(line)
	getdvtype(type)
	hdb_choose_Any(baud)
	hdb_choose_Device(type, baud)
	hdb_dial(presult)
	hdb_dial_error_text(errcode)
	hdb_init()
	in_ungetty_list(line)
	malformed_Devices_entry(text, ntokens, tokens)
	remove_from_ungetty_list(line)
	report_initial_line()
	reserve_line(line)
	setdlent()
	setdvent()
	strip_phone_num(sptr)
	ungetty_get_line(line)
	ungetty_return_all_but(line)
	ungetty_return_line(line, cause)

  Date: Fri, 23 Aug 91 18:30:06 +0300 (MSD)
  From: ache@hq.demos.su (Andrew A. Chernov, canton Uri's citizen)
  1) HDB dialers may return connect speed as return code (!= 0)
  2) Using HDB Dialcodes file for phone numbers translation now
     (\D,\T escape sequence)

  Many [Nobel physics] prizes  have been given  to people for  telling us
  the universe is not as simple as we thought it was. -Stephen Hawking in
  A Brief History of Time     In computing, there are no such prizes. -me

--------------------------------------------------------------------------*/
/*+:EDITS:*/
/*:04-26-2000-11:16-wht@bob-RELEASE 4.42 */
/*:11-16-1997-22:15-wht@kepler-regexp_compile changed shape */
/*:01-24-1997-02:37-wht@yuriatin-SOURCE RELEASE 4.00 */
/*:09-11-1996-20:00-wht@yuriatin-3.48-major telnet,curses,structural overhaul */
/*:08-11-1996-02:10-wht@kepler-rename ecu_log_event to logevent */
/*:12-06-1995-13:30-wht@n4hgf-termecu w/errno -1 consideration */
/*:12-03-1995-19:57-wht@gyro-use Setuid */
/*:11-23-1995-11:20-wht@kepler-source control 3.37 for tsx-11 */
/*:11-14-1995-10:23-wht@kepler-3.37.80-source control point: SOCKETS */
/*:11-03-1995-16:54-wht@wwtp1-use CFG_TelnetOption */
/*:09-17-1995-16:28-wht@kepler-remove obsolete #if 0 code */
/*:06-14-1995-19:16-wht@n4hgf-if appropriate, setuid before fork dial prog */
/*:06-12-1995-15:25-wht@n4hgf-do not ungetty lines owned by user */
/*:06-12-1995-15:24-wht@n4hgf-if uucp euid, do not ungetty uucp-owned lines */
/*:05-11-1995-15:55-wht@n4hgf-ck_sigint fools optimizing compilers */
/*:04-01-1995-18:41-wht@n4hgf-configure use of ungetty on non-Devices lines */
/*:04-01-1995-17:48-wht@n4hgf-change hdb_choose_Device current line checking */
/*:03-21-1995-14:45-wht@n4hgf-expresp_verbosity now set to proc_trace */
/*:01-15-1995-02:49-wht@gyro-_ungetty_return_line might have been unused */
/*:01-12-1995-15:20-wht@n4hgf-apply Andrew Chernov 8-bit clean+FreeBSD patch */
/*:05-04-1994-04:39-wht@n4hgf-ECU release 3.30 */
/*:01-23-1994-14:48-wht@n4hgf-do not die on illogical utmp status, just flunk */
/*:01-04-1994-06:31-wht@n4hgf-last_Speed_result improvement */
/*:12-02-1993-15:30-wht@n4hgf-hbd_choose_Any could not handle DIALOUT */
/*:08-30-1993-12:04-wht@n4hgf-with SCO companion lines, skip bad ecuungetty */
/*:08-16-1993-17:19-wht@n4hgf-aid choose debug with report_initial_line() */
/*:05-30-1993-15:25-wht@n4hgf-strip RC_ENABLED from wait_status */
/*:05-29-1993-20:21-wht@n4hgf-change linst_err_text to LINST_text */
/*:05-29-1993-20:18-wht@n4hgf-change ugstat_text to UG_text */
/*:05-29-1993-20:13-wht@n4hgf-beef up debug log events */
/*:03-27-1993-17:45-wht@n4hgf-SVR4 cc complained about strlen <= constant */
/*:02-27-1993-13:55-wht@n4hgf-dialer prog in CFG_HdbLibDir can have simple path */
/*:12-20-1992-12:20-wht@n4hgf-add setdvent and setdlent */
/*:12-20-1992-12:17-wht@n4hgf-add getdvtype */
/*:09-14-1992-04:05-wht@n4hgf-rcvr process was not going away reliably */
/*:09-10-1992-13:59-wht@n4hgf-ECU release 3.20 */
/*:09-10-1992-03:35-wht@n4hgf-change the way we flunk a line=="-" */
/*:09-04-1992-19:08-wht@n4hgf-harden Devices parsing */
/*:08-29-1992-15:37-wht@n4hgf-absolutely prohibit /dev/tty fed to ecuungetty */
/*:08-22-1992-15:39-wht@n4hgf-ECU release 3.20 BETA */
/*:07-21-1992-17:20-wht@n4hgf-hdb_dial of "/=" type bug fixed */
/*:07-19-1992-22:12-wht@n4hgf-move old check_utmp here+call it reserve_line */
/*:07-19-1992-10:07-wht@n4hgf-add ungetty_return_all_but */
/*:07-19-1992-09:11-wht@n4hgf-validate tty line name before ungetty get */
/*:06-07-1992-17:05-wht@n4hgf-lock tty before ungetty get */
/*:05-13-1992-13:27-wht@n4hgf-active_pde use */
/*:05-13-1992-10:30-cma@ifsbd-Add bit rate checking to hdb_dial function */
/*:05-04-1992-04:45-wht@n4hgf-wrong sense of strcmp in ,M test for SVR4 */
/*:04-28-1992-03:29-wht@n4hgf-more fixes for abend due to line problems */
/*:04-27-1992-20:02-wht@n4hgf-add ecuungetty error reporting */
/*:04-25-1992-13:02-wht@n4hgf-some exits from hdb_choose_Any omitted enddvent */
/*:04-24-1992-21:59-wht@n4hgf-more SCO tty name normalizing */
/*:04-19-1992-03:21-jhpb@sarto.budd-lake.nj.us-3.18.37 has ESIX SVR4 */
/*:01-18-1992-13:29-root@n4hgf-use proc_trace value for expresp_verbosity */
/*:11-15-1991-04:02-wht@n4hgf-SCO tty naming now observed in getdvline */
/*:09-01-1991-16:20-wht@n4hgf2-generalize HDB configuration files location */
/*:09-01-1991-02:27-wht@n4hgf2-dialer gets file name instead of "ECUdial" */
/*:08-25-1991-13:07-wht@n4hgf-apply ache@hq.demos.su patches */
/*:08-10-1991-17:39-wht@n4hgf-US_WEGOTIT handling */
/*:07-25-1991-12:58-wht@n4hgf-ECU release 3.10 */
/*:06-02-1991-17:42-wht@n4hgf-add getdvtype */
/*:06-02-1991-17:27-wht@n4hgf-hdb_choose_Device + move hdb_choose_Any here */
/*:10-16-1990-20:43-wht@n4hgf-add SHARE_DEBUG */
/*:09-19-1990-19:36-wht@n4hgf-logevent now gets pid for log from caller */
/*:08-14-1990-20:40-wht@n4hgf-ecu3.00-flush old edit history */

#include "ecu.h"
#include "ecupde.h"
#include "esd.h"
#include "var.h"
#include "termecu.h"
#include "utmpstatus.h"
#include "ecuungetty.h"
#include "dvent.h"
#include "dlent.h"
#include "dialprog.h"
#include <errno.h>
#include <utmp.h>

char *arg_token();
char *skip_ld_break();
char *dialcodes_translate();
char *strip_phone_num();
char *US_text();
char *LINST_text();

#if defined(CFG_UseUngetty)
char *bamboozle();

#endif

extern char kbdintr;		 /* current input INTR */
extern UINT32 colors_current;
extern int expresp_verbosity;
extern char last_Speed_result[];

int there_is_hdb_on_this_machine = 0;
char *hdblibdir = CFG_HdbLibDir;	/* system independent location */
char *Devices_file = "/usr/lib/uucp/Devices";	/* overriden by config */
char *Dialers_file = "/usr/lib/uucp/Dialers";
char *Dialcodes_file = "/usr/lib/uucp/Dialcodes";
uchar last_ugstat = 0;

#define UNGETTY_LIST_MAX	3
char *ungetty_list[UNGETTY_LIST_MAX];

static FILE *fpdv = (FILE *) 0;
static FILE *fpdl = (FILE *) 0;

/*+-------------------------------------------------------------------------
	display_ungetty_list() - display ungetty list with pputs()
--------------------------------------------------------------------------*/
#if defined(CFG_UseUngetty)
void
display_ungetty_list()
{
	int itmp;
	int found_one = 0;

	for (itmp = 0; itmp < UNGETTY_LIST_MAX; itmp++)
	{
		if (*ungetty_list[itmp])
		{
			found_one = 1;
			break;
		}
	}

	if (!found_one)
	{
		pputs("No lines acquired by ecuungetty\n");
		return;
	}

	pputs("Acquired by ecuungetty: ");
	for (itmp = 0; itmp < UNGETTY_LIST_MAX; itmp++)
	{
		if (*ungetty_list[itmp])
		{
			pputs(ungetty_list[itmp]);
			pputs(" ");
		}
	}
	pputs("\n");

}							 /* end of display_ungetty_list */
#endif /* defined(CFG_UseUngetty) */

/*+-------------------------------------------------------------------------
	in_ungetty_list(line) - check for line present in ungetty list
--------------------------------------------------------------------------*/
#if defined(CFG_UseUngetty)
int
in_ungetty_list(line)
char *line;
{
	int itmp;

	for (itmp = 0; itmp < UNGETTY_LIST_MAX; itmp++)
	{
		if (!strcmp(line, ungetty_list[itmp]))
			return (1);
	}
	return (0);

}							 /* end of in_ungetty_list */
#endif /* defined(CFG_UseUngetty) */

/*+-------------------------------------------------------------------------
	add_to_ungetty_list(line) - add line to ungetty list
--------------------------------------------------------------------------*/
#if defined(CFG_UseUngetty)
void
add_to_ungetty_list(line)
char *line;
{
	int itmp;
	char *lptr;

	if (in_ungetty_list(line))
		return;

	for (itmp = 0; itmp < UNGETTY_LIST_MAX; itmp++)
	{
		if (!*(lptr = ungetty_list[itmp]))
		{
			strcpy(lptr, line);
			return;
		}
	}
	logevent(getpid(), "UNGETTY_LIST OVERFLOW");
	errno = -1;
	termecu(TERMECU_LOGIC_ERROR);
	/* NOTREACHED */

}							 /* end of add_to_ungetty_list */
#endif /* defined(CFG_UseUngetty) */

/*+-------------------------------------------------------------------------
	remove_from_ungetty_list(line) - remove line from ungetty list
--------------------------------------------------------------------------*/
#if defined(CFG_UseUngetty)
void
remove_from_ungetty_list(line)
char *line;
{
	int itmp;
	char *lptr;

	for (itmp = 0; itmp < UNGETTY_LIST_MAX; itmp++)
	{
		if (!strcmp((lptr = ungetty_list[itmp]), line))
		{
			*lptr = 0;
			return;
		}
	}

#ifdef CHOOSE_DEBUG
	vlogevent(getpid(), "REMOVE_FROM_UNGETTY_LIST FAILED FOR %s", line);
#endif

}							 /* end of remove_from_ungetty_list */
#endif /* defined(CFG_UseUngetty) */

/*+-------------------------------------------------------------------------
	ungetty_get_line(line) - acquire a line through ecuungetty protocol
--------------------------------------------------------------------------*/
enum linst
ungetty_get_line(line)
char *line;
{
#if !defined(CFG_UseUngetty)
	line = 0; /* -Wunused */
	return (LINST_OK);
#else
	int itmp;
	int linst;
	int rtn = 0;
	int we_locked = 0;
	int ungetty_pid;

	CFG_SigType(*original_sighdlr) ();
	int wait_status;
	char ungetty[128];
	char bamboozlement[20];
	struct stat st;

#if !defined(CFG_UngettyAllLines)
	struct dvent *dv = 0;

#endif /* !CFG_UngettyAllLines */

	if (shm->Ltelnet)
		return (0);

	/*
	 * quick check - ecuungetty does a much better job
	 */
	if (!strcmp(line, "/dev/tty"))	/* some keep getting /dev/tty chown'd! */
		return (LINST_INVALID);
	if (stat(line, &st))
	{
		if (errno == ENOENT)
			return (LINST_NODEV);
		return (LINST_OPNFAIL);
	}
	if ((st.st_mode & S_IFMT) != S_IFCHR)
		return (LINST_NOTCHR);
	if (st.st_uid == uid)
		return (0);
	if (setuid_uucp && (st.st_uid == uid_uucp))
		return (0);
	if (!there_is_hdb_on_this_machine)
		return (0);
	if (in_ungetty_list(line))
		return (0);

	/*
	 * if device is not listed in devices, we do not want to change the
	 * device ownership/mode
	 */
#if !defined(CFG_UngettyAllLines)
	enddvent();				 /* should not be necessary -- but be safe */
	dv = getdvline(line + 5);
	enddvent();
	if (!dv)
		return (0);
#endif /* !CFG_UngettyAllLines */

	/*
	 * lock line before ecuungetty call
	 */
	if ((linst = lock_tty(line)) && (linst != LINST_WEGOTIT))
		return (linst);
	we_locked = (!(int)itmp);

	sprintf(ungetty, "%s/ecuungetty", CFG_EcuLibDir);
	strcpy(bamboozlement, bamboozle(getpid()));
	if (access(ungetty, 1))
	{
		pperror(ungetty);
		rtn = LINST_ENABLED;
		goto FUNC_RETURN;
	}
	original_sighdlr = signal(SIGCLD, SIG_DFL);
	if (!(ungetty_pid = smart_fork()))
	{
		execl(ungetty, "ungetty", line, bamboozlement, (char *)0);
		_exit(UGE_DNE);		 /* did not execute */
	}
	while (((itmp = wait(&wait_status)) != ungetty_pid) && (itmp != -1))
		;
	signal(SIGCLD, original_sighdlr);

	if (wait_status & 0xFF)
		last_ugstat = UGE_BOMB;
	else
		last_ugstat = (uchar) (wait_status >> 8);
	switch (last_ugstat)
	{
		case UG_NOTENAB:	 /* line acquired: not enabled */
			break;

		case UG_RESTART:	 /* line acquired: need ungetty -r when done */
#if defined(LOG_UNGETTY)
			vlogevent(getpid(), "UNGETTY acquired %s", shm->Lline);
#endif
			add_to_ungetty_list(line);
			break;

		case UG_FAIL:		 /* line in use */
			rtn = LINST_ENABLED_IN_USE;
			break;

		default:
			vlogevent(getpid(), "UNGETTY unknown status 0x%04x: %s",
				wait_status, UG_text(last_ugstat));
			rtn = (last_ugstat == UGE_BOMB)
				? LINST_ECUUNGETTY2 : LINST_ECUUNGETTY;
			break;
	}

  FUNC_RETURN:
	if (rtn && we_locked)
		unlock_tty(line);
	return (rtn);

#endif /* !defined(CFG_UseUngetty) */
}							 /* end of ungetty_get_line */

/*+-------------------------------------------------------------------------
    reserve_line(line)
'line' is "/dev/ttyxx"-style
return 0 if line reserved, else LINST code
--------------------------------------------------------------------------*/
enum linst
reserve_line(line)
char *line;
{
	enum utmp_status utmpst;
	enum linst linst = LINST_OK;

#ifdef CFG_TelnetOption
	if (shm->Ltelnet)
		return (LINST_OK);
#endif

	switch (utmpst = utmp_status(line))
	{
		case US_DIALOUT:	 /* enabled for login, currently dialout */
			linst = LINST_DIALOUT_IN_USE;
			break;
		case US_LOGGEDIN:	 /* enabled for login, in use */
			linst = LINST_ENABLED_IN_USE;
			break;
		case US_NOTFOUND:	 /* not in utmp, or getty dead */
			linst = LINST_OK;
			break;
		case US_WEGOTIT:	 /* we own the line */
			linst = LINST_WEGOTIT;	/* not really an error */
			break;
		case US_LOGIN:		 /* enabled for login, idle */
			linst = ungetty_get_line(line);
			break;
		default:
#if defined(ILLOGICAL)
			ff(se, "reserve_line illogical utmp status %s for %s\r\n",
				US_text(utmpst), line);
			errno = -1;
			termecu(TERMECU_LOGIC_ERROR);
#else
			break;
#endif
	}

#if defined(CHOOSE_DEBUG)
	vlogevent(getpid(), "RESERVE %s '%s'", line, LINST_text(linst));
#endif

	return (linst);

}							 /* end of reserve_line */

/*+-------------------------------------------------------------------------
	_ungetty_return_line(line) - return line to "getty" status
--------------------------------------------------------------------------*/
#if defined(CFG_UseUngetty)
static void
_ungetty_return_line(line)
char *line;
{
	int ungetty_pid;
	int itmp;

	CFG_SigType(*original_sighdlr) ();
	int wait_status = 0xDEAD;
	char ungetty[128];
	char bamboozlement[20];

#if !defined(CFG_UngettyAllLines)
	struct dvent *dv = 0;

#endif /* !CFG_UngettyAllLines */

	if (shm->Ltelnet)
		return;
	if (!there_is_hdb_on_this_machine)
		return;
	if (!in_ungetty_list(line))
		return;

	/*
	 * if device is not listed in Devices, we do not want to change the
	 * device ownership/mode
	 */
#if !defined(CFG_UngettyAllLines)
	enddvent();				 /* should not be necessary -- but be safe */
	dv = getdvline(line + 5);
	enddvent();
	if (!dv)
		return;
#endif /* !CFG_UngettyAllLines */

	sprintf(ungetty, "%s/ecuungetty", CFG_EcuLibDir);
	strcpy(bamboozlement, bamboozle(getpid()));

	/* call ungetty to see if we need to switch to dialin */
#ifdef NEVER				 /* if in_ungetty_list, trust we need to -r */
	if (access(ungetty, 1))
	{
		pperror(ungetty);
		return;
	}
	original_sighdlr = signal(SIGCLD, SIG_DFL);
	if ((ungetty_pid = smart_fork()) == 0)
	{
		execl(ungetty, "ungetty", "-t", line, bamboozlement, (char *)0);
		logevent(getppid(), "could not exec ecuungetty -t");
		_exit(UGE_DNE);		 /* did not execute */
	}
	while (((itmp = wait(&wait_status)) != ungetty_pid) &&
		(itmp != -1))
		;
	signal(SIGCLD, original_sighdlr);

#if defined(LOG_UNGETTY)
	vlogevent(getpid(), "UNGETTY -t %s status %04x", line, wait_status);
#endif

	switch ((uchar) (wait_status >> 8))
	{
		case UG_RESTART:
			break;

		default:
			remove_from_ungetty_list(line);
			return;
	}
#endif /* NEVER */

	original_sighdlr = signal(SIGCLD, SIG_DFL);
	if ((ungetty_pid = smart_fork()) == 0)
	{
		execl(ungetty, "ungetty", "-r", line, bamboozlement, (char *)0);
		logevent(getppid(), "could not exec ecuungetty -r");
		_exit(UGE_DNE);		 /* did not execute */
	}

	while (((itmp = wait(&wait_status)) != ungetty_pid) &&
		(itmp != -1))
		;

#if defined(LOG_UNGETTY)
	if (wait_status)
		vlogevent(getpid(), "UNGETTY -r %s status 0x%04x", line, wait_status);
	else
		vlogevent(getpid(), "UNGETTY returned %s", line);
#endif

	remove_from_ungetty_list(line);

}							 /* end of _ungetty_return_line */
#endif /* defined(CFG_UseUngetty) */

/*+-------------------------------------------------------------------------
	ungetty_return_line(line,cause) - return one or all lines to "getty"

cause is a debugging feature
--------------------------------------------------------------------------*/
void
ungetty_return_line(line, cause)
char *line;
char *cause;
{
#if !defined(CFG_UseUngetty)
	line = 0; /* -Wunused */
	cause = 0; /* -Wunused */
	return;
#else
	int itmp;

#if defined(CHOOSE_DEBUG)
	vlogevent(getpid(), "ungetty_return_line '%s' because '%s'",
		(line) ? line : "<ALL>", cause);
#endif /* CHOOSE_DEBUG */

	if (shm->Ltelnet)
		return;

	if (line)
	{
#ifdef SCO_TTY_NAMING
		if (strcmp(shm->Lline, line) && (!strcmpi(shm->Lline, line)) &&
			in_ungetty_list(shm->Lline))
		{
#ifdef CHOOSE_DEBUG
			vlogevent(getpid(), "ecuungetty -r skipped for companion %s",
				line);
#endif
			return;
		}
#endif /* SCO_TTY_NAMING */
		_ungetty_return_line(line);
	}
	else
	{
		for (itmp = 0; itmp < UNGETTY_LIST_MAX; itmp++)
		{
			if (*(line = ungetty_list[itmp]))
				_ungetty_return_line(line);
		}
	}
#endif /* defined(CFG_UseUngetty) */

}							 /* end of ungetty_return_line */

/*+-------------------------------------------------------------------------
	ungetty_return_all_but(line) - return all lines but 'line'
--------------------------------------------------------------------------*/
void
ungetty_return_all_but(line)
char *line;
{
#if !defined(CFG_UseUngetty)
	line = 0; /* -Wunused */
	return;
#else
	int itmp;

	if (shm->Ltelnet)
		return;

	for (itmp = 0; itmp < UNGETTY_LIST_MAX; itmp++)
	{
		if (ungetty_list[itmp][0] && strcmp(line, ungetty_list[itmp]))
			_ungetty_return_line(line);
	}

#endif /* !defined(CFG_UseUngetty) */
}							 /* end of ungetty_return_all_but */

/*+-------------------------------------------------------------------------
	malformed_Devices_entry(text,ntokens,tokens)
--------------------------------------------------------------------------*/
void
malformed_Devices_entry(text, ntokens, tokens)
char *text;
int ntokens;
char **tokens;
{
	char s512[512];
	char *cp;
	char *nlptr;
	static int already = 0;
	extern int tty_not_char_special;

	sprintf(s512, "malformed Devices entry (%s):\n", text);
	cp = s512 + strlen(s512);
	nlptr = cp - 1;

	while (ntokens--)
	{
		if (((cp - s512) + strlen(*tokens) + 2) > sizeof(s512))
			break;
		sprintf(cp, "%s ", *tokens++);
		cp += strlen(cp);
	}

	if (!already && !tty_not_char_special)
	{
		pputs("\7\n");
		pputs(s512);
		pputs("\nFurther Devices errors will not be displayed,\n");
		pputs("but are logged in ~/.ecu/log.  Press any key to continue.\7");
		ttyflush(0);
		ttygetc(1);
		pputs("\n");
	}
	already = 1;

	memcpy(s512, "MALFORMED", 9);	/* mod for log file */
	*nlptr = ' ';
	logevent(xmtr_pid, s512);

}							 /* end of malformed_Devices_entry */

/*+-------------------------------------------------------------------------
	getdvent() - get first or next Devices entry (a la getpwent)
--------------------------------------------------------------------------*/
DVE *
getdvent()
{
	int itmp;

#define MAX_DV_TOKENS 9
	char *tokens[MAX_DV_TOKENS];
	int ntokens;

	char *cp;
	static DVE dve;
	static char dvstr[512];
	char *skip_ld_break();

	if (!there_is_hdb_on_this_machine)
		goto RETURN_NULL;

	if (!fpdv)
	{
		if (!(fpdv = fopen(Devices_file, "r")))
		{
			pperror(Devices_file);
			goto RETURN_NULL;
		}
	}

	while (1)
	{

		/*
		 * read a Devices line
		 */
		if (!fgets(dvstr, sizeof(dvstr), fpdv))
		{
		  RETURN_NULL:
#ifdef CHOOSE_DEBUG
			logevent(xmtr_pid, "getdvent returning NULL");
#endif
			return ((DVE *) 0);
		}

		/*
		 * weed out comments and blank lines
		 */
		if ((unsigned)strlen(dvstr) <= (unsigned)1)	/* blank line */
			continue;
		cp = skip_ld_break(dvstr);	/* first non-blank */
		if (!*cp || strchr("#\n", *cp))	/* comment or all white space */
			continue;

		/*
		 * tokenize
		 */
		build_arg_array(dvstr, tokens, MAX_DV_TOKENS, &ntokens);

		/*
		 * a bit of validation
		 */
		if (ntokens < 4)
		{
			malformed_Devices_entry("too few fields", ntokens, tokens);
			continue;
		}

		break;
	}

	/*
	 * we have a good entry
	 */
	dve.type = tokens[0];
	dve.line = tokens[1];

	/*
	 * get rid of possible SVR4 ",M" modem control suffix
	 */
	itmp = strlen(dve.line);
	if ((itmp > 2) && !strcmp(dve.line + itmp - 2, ",M"))
		dve.line[itmp - 2] = 0;

	dve.dialer = tokens[2];
	if (!strcmpi(tokens[3], "Any"))
	{
		dve.low_baud = 1;
		dve.high_baud = 65535;
	}
	else
	{
		dve.low_baud = atoi(tokens[3]);
		if (!(cp = strchr(tokens[3], '-')))
			dve.high_baud = dve.low_baud;
		else
			dve.high_baud = atoi(cp + 1);
	}
	dve.dialprog = tokens[4];
	dve.token = tokens[5];

#ifdef CHOOSE_DEBUG
	vlogevent(getpid(), "getdvent returning '%s' type='%s'",
		dve.line, dve.type);
#endif
	return (&dve);

}							 /* end of getdvent */

/*+-------------------------------------------------------------------------
	setdvent()
--------------------------------------------------------------------------*/
void
setdvent()
{
	if (fpdv)
		rewind(fpdv);
}							 /* end of setdvent */

/*+-------------------------------------------------------------------------
	enddvent() - close Devices file
--------------------------------------------------------------------------*/
void
enddvent()
{
	if (fpdv)
	{
		fclose(fpdv);
		fpdv = (FILE *) 0;
	}
}							 /* end of enddvent */

/*+-------------------------------------------------------------------------
	getdvbaud(baud) - get Devices entry matching bit rate
--------------------------------------------------------------------------*/
DVE *
getdvbaud(baud)
UINT baud;
{
	DVE *tdve;

#ifdef CHOOSE_DEBUG
	vlogevent(getpid(), "getdvbaud looking for %u baud", baud);
#endif

	while (1)
	{
		if ((tdve = getdvent()) == (DVE *) 0)
			return (tdve);
#ifdef CHOOSE_DEBUG
		vlogevent(getpid(), "getdvbaud found %s type '%s' baud lo=%d hi=%d",
			tdve->line, tdve->type, tdve->low_baud, tdve->high_baud);
#endif
		if (!strcmp(tdve->line, "-"))	/* neo-entries like TCP have "-"
										 * line */
			continue;
		if ((tdve->low_baud <= baud) && (baud <= tdve->high_baud))
		{
#ifdef CHOOSE_DEBUG
			vlogevent(getpid(), "getdvbaud returning %s", tdve->line);
#endif
			return (tdve);
		}
	}
	/* NOTREACHED */

}							 /* end of getdvbaud */

/*+-------------------------------------------------------------------------
	getdvline(line) - get Devices entry matching line
calling argument 'line's is string AFTER "/dev/"
--------------------------------------------------------------------------*/
DVE *
getdvline(line)
char *line;
{
	DVE *tdve;

#ifdef CHOOSE_DEBUG
	vlogevent(getpid(), "getdvline looking for %s", line);
#endif

	while (1)
	{
		if ((tdve = getdvent()) == (DVE *) 0)
			return (tdve);
#ifdef CHOOSE_DEBUG
		vlogevent(getpid(), "getdvline %s found baud lo=%d hi=%d",
			tdve->line, tdve->low_baud, tdve->high_baud);
#endif
		if (!TTYNAME_STRCMP(tdve->line, line))
			return (tdve);
	}
	/* NOTREACHED */

}							 /* end of getdvline */

/*+-------------------------------------------------------------------------
	dvtype_match(typespec,dvtype) - match between pde typespec and dvtype

returns 1 if pde type specification 'typespec' matches Devices device 'type'
--------------------------------------------------------------------------*/
int
dvtype_match(typespec, dvtype)
char *typespec;
char *dvtype;
{
	char *match;
	int matchlen;
	int re_match = 1;
	char cmpbuf[128];

	if (*typespec == '=')
		typespec++;
	else if (*typespec == '/')
	{
		re_match = 0;
		typespec++;
	}

	if (re_match)
	{
		if (!strcmp(dvtype, typespec))
			return (1);
	}
	else
	{
		if (regexp_compile(typespec, cmpbuf, sizeof(cmpbuf)))
			return (0);
		if (regexp_scan(cmpbuf, dvtype, &match, &matchlen))
			return (1);
	}
	return (0);

}							 /* end of dvtype_match */

/*+-------------------------------------------------------------------------
	getdvtype(type) - get Devices entry matching type

type is either 'Device_type'   search for exact match on Device_type
               '=Device_type'  search for exact match on Device_type
               '/regexp'       search for match with regular expression

you must make sure any supplied regexp is a valid one, for regexp
compilation errors are indistinguishable from other seach failures

uses optimized implementation of dvtype_match functionality
--------------------------------------------------------------------------*/
DVE *
getdvtype(type)
char *type;
{
	DVE *tdve;
	char *emsg;
	char *match;
	int matchlen;
	int re_match = 0;		 /* regular expression match */
	char cmpbuf[128];

	if (*type == '=')
		type++;
	else if (*type == '/')
	{
		re_match = 1;
		type++;
		if (regexp_compile(type, cmpbuf, sizeof(cmpbuf), &emsg))
			return ((DVE *) 0);
	}

	while (tdve = getdvent())
	{
		if (!strcmp(tdve->line, "-"))	/* neo-entries like TCP have "-"
										 * line */
			continue;
		if (re_match)
		{
			if (regexp_scan(cmpbuf, tdve->type, &match, &matchlen))
				break;
		}
		else
		{
			if (!strcmp(tdve->type, type))
				break;
		}
	}
	return (tdve);

}							 /* end of getdvtype */

/*+-------------------------------------------------------------------------
	getdlent() - get first or next Dialers entry (a la getpwent)
--------------------------------------------------------------------------*/
struct dlent *
getdlent()
{
	int itmp;

#define MAX_DL_TOKENS 3
	char *tokens[MAX_DL_TOKENS];
	static struct dlent dle;
	static char dlstr[128];

	if (!there_is_hdb_on_this_machine)
		return ((struct dlent *)0);

	if (!fpdl)
	{
		if (!(fpdl = fopen(Dialers_file, "r")))
		{
			pperror(Dialers_file);
			return ((struct dlent *)0);
		}
	}

	while (1)
	{
		if (!fgets(dlstr, sizeof(dlstr), fpdl))
			return ((struct dlent *)0);
		if (((itmp = strlen(dlstr)) <= 1) || (dlstr[0] == '#') ||
			(dlstr[0] == ' '))
		{
			continue;
		}
		dlstr[--itmp] = 0;
		for (itmp = 0; itmp < MAX_DL_TOKENS; itmp++)
			tokens[itmp] = "";
		if (tokens[0] = arg_token(dlstr, " \t\r\n"))
		{
			if (tokens[1] = arg_token((char *)0, " \t\r\n"))
			{
				extern char *str_token_static;

				tokens[2] = skip_ld_break(str_token_static);
			}
		}
		break;
	}

	dle.name = tokens[0];
	dle.tlate = tokens[1];
	dle.script = tokens[2];
	return (&dle);

}							 /* end of getdlent */

/*+-------------------------------------------------------------------------
	setdlent()
--------------------------------------------------------------------------*/
void
setdlent()
{
	if (fpdl)
		rewind(fpdl);
}							 /* end of setdlent */

/*+-------------------------------------------------------------------------
	enddlent() - close Dialers file
--------------------------------------------------------------------------*/
void
enddlent()
{
	if (fpdl)
	{
		fclose(fpdl);
		fpdl = (FILE *) 0;
	}
}							 /* end of enddlent */

/*+-------------------------------------------------------------------------
	getdlentname(name) - get Dialers entry by name
--------------------------------------------------------------------------*/
struct dlent *
getdlentname(name)
char *name;
{
	DLE *tdle;

	while (tdle = getdlent())
	{
		if (!strcmp(name, tdle->name))
			break;
	}
	return (tdle);

}							 /* end of getdlentname */

/*+-------------------------------------------------------------------------
	hdb_choose_Any(baud) - user will take 'Any' line

give preference to current line
--------------------------------------------------------------------------*/
DVE *
hdb_choose_Any(baud)
UINT baud;
{
	DVE *tdve = (DVE *) 0;
	char newtty[sizeof(shm->Lline)];
	enum linst linst = LINST_OK;
	enum utmp_status utmpst = US_UNDEF;

#ifdef CHOOSE_DEBUG
	char s512[512];

	vlogevent(getpid(), "CHOOSEANY REQUEST baud=%u (current line='%s')",
		baud, shm->Lline);
#endif

	enddvent();				 /* krock but safe */

	/*
	 * see if shm->Lline in use by someone else; if not and bit rate ok,
	 * no further
	 */

	if (shm->Lline[0])
	{
		while (tdve = getdvline(shm->Lline + 5))
		{
			if ((tdve->low_baud <= baud) && (baud <= tdve->high_baud))
				break;
		}

		if (tdve)
		{
			switch (utmpst = utmp_status(shm->Lline))
			{
				case US_WEGOTIT:	/* the line is already ours */
				case US_LOGIN:	/* enabled for login, idle */
					goto FUNC_RETURN;
				case US_DIALOUT:
				case US_LOGGEDIN:
					break;
				case US_NOTFOUND:	/* not in utmp, or getty dead */
#if !defined(CFG_UseUngetty)
					if (access(shm->Lline, 6))
						break;
#endif
					if ((linst = line_lock_status(shm->Lline)) &&
						(linst != LINST_WEGOTIT))
					{
						break;
					}
					goto FUNC_RETURN;
				default:
					break;
			}
		}

		enddvent();
	}

	/*
	 * we've got to pick a new line
	 */

#ifdef CHOOSE_DEBUG
	strcpy(s512, "CHOOSEANY pick new line: ");
	if (!shm->Lline[0])
		strcat(s512, "none selected at all");
	else
	{
		sprintf(s512 + strlen(s512), "utmpst=%s linst=%s",
			US_text(utmpst), LINST_text(linst));
	}
	logevent(getpid(), s512);
#endif

	linst = LINST_OK;
	while (tdve = getdvbaud(baud))
	{
		/* by now, we know shm->Lline wont work */
		if (!TTYNAME_STRCMP(tdve->line, shm->Lline + 5))
			continue;

		/* if not acu, dont use it */
		if (ulindex(tdve->type, "ACU") < 0)
			continue;

		sprintf(newtty, "/dev/%s", tdve->line);
		switch (utmpst = utmp_status(newtty))
		{
			case US_NOTFOUND:	/* not in utmp, or getty dead */
#if !defined(CFG_UseUngetty)
				if (access(newtty, 6))	/* ecuungetty won't be able to
										 * help us */
					break;
#endif
				if ((linst = line_lock_status(newtty)) &&
					(linst != LINST_WEGOTIT))
				{
					break;
				}
				goto FUNC_RETURN;

			case US_LOGIN:	 /* enabled for login, idle */
				goto FUNC_RETURN;

			case US_WEGOTIT:
				goto FUNC_RETURN;

			default:
				break;
		}
	}

  FUNC_RETURN:
	enddvent();

#ifdef LOG_HDBDIAL
	vlogevent(getpid(), "CHOOSEANY RESULT %s (status utmp=%s line=%s)",
		(tdve) ? tdve->line : "<none>", US_text(utmpst), LINST_text(linst));
#endif
	return (tdve);

}							 /* end of hdb_choose_Any */

/*+-------------------------------------------------------------------------
	hdb_choose_Device(type,baud) - need line with 'type' and 'baud'

return DVE pointer if line chosen, 0 if failed to find a line
Priority is NO LONGER given to retaining the current line as of rev 3.34
--------------------------------------------------------------------------*/
DVE *
hdb_choose_Device(type, baud)
char *type;
UINT baud;
{
	DVE *tdve = (DVE *) 0;
	char s32[32];
	int itmp = 0;
	enum utmp_status utmpst = US_UNDEF;
	int done;

#ifdef CHOOSE_DEBUG
	vlogevent(getpid(), "CHOOSEDEV REQUEST type='%s' baud=%u", type, baud);
#endif

	/*
	 * we may have to pick a new line
	 */
	done = 0;
	while (!done)
	{

		/*
		 * get Devices entry matching type
		 */
		if (!(tdve = getdvtype(type)))
		{
#ifdef CHOOSE_DEBUG
			vlogevent(getpid(), "CHOOSEDEV no line matches type '%s'", type);
#endif
			break;
		}

		/*
		 * does bit rate match?
		 */
		if ((tdve->low_baud > baud) || (baud > tdve->high_baud))
			continue;

		sprintf(s32, "/dev/%s", tdve->line);
		utmpst = utmp_status(s32);
#ifdef CHOOSE_DEBUG
		vlogevent(getpid(), "%s: utmp status=%s  lock status=%s", s32,
			US_text(utmpst), LINST_text(line_lock_status(s32)));
#endif
		switch (utmpst)
		{
			case US_NOTFOUND:	/* not in utmp, or getty dead */
				if (!(itmp = line_lock_status(s32)) || (itmp == LINST_WEGOTIT))
					done = 1;
				break;

			case US_WEGOTIT:/* we own the line */
				/* this would be a curious case to succeed */
				done = 1;
				break;

			case US_LOGIN:	 /* enabled for login, idle */
				done = 1;
				break;

			default:
				break;
		}
	}

	enddvent();

#ifdef CHOOSE_DEBUG
	vlogevent(getpid(), "CHOOSEDEV RESULT %s utmp status=%s",
		(tdve) ? tdve->line : "<none>",
		(tdve) ? US_text(utmpst) : "N/A");
#endif

	return (tdve);

}							 /* end of hdb_choose_Device */

/*+-------------------------------------------------------------------------
	hdb_dial_error(errcode) - dialer program error code to text

also sets iv[0] to dial command status
--------------------------------------------------------------------------*/
char *
hdb_dial_error_text(errcode)
int errcode;
{
	static char errant[64];

	iv[0] = 1;
	switch (errcode &= 0x7F)
	{
		case RCE_INUSE:
			return ("!Line in use");
		case RCE_SIG:
			iv[0] = 2;
			return ("!Interrupted");
		case RCE_ARGS:
			return ("!Invalid arguments");
		case RCE_PHNO:
			return ("!Invalid phone number");
		case RCE_SPEED:
			return ("!Bad bit rate");
		case RCE_OPEN:
			return ("!Line open error");
		case RCE_IOCTL:
			return ("!Ioctl error");
		case RCE_TIMOUT:
			iv[0] = 3;
			return ("!Modem Error");
		case RCE_NOTONE:
			return ("NO DIAL TONE");
		case RCE_BUSY:
			return ("BUSY");
		case RCE_NOCARR:
			return ("NO CARRIER");
		case RCE_ANSWER:
			return ("NO ANSWER");
		default:
		case RCE_NULL:
			sprintf(errant, "unknown dialer error code %d", errcode);
			return (errant);
	}
	/* NOTREACHED */
}							 /* end of hdb_dial_error */

/*+-------------------------------------------------------------------------
	hdb_dial(presult) - dial with uucp dialer if we can

return 0 if connected
       1 if dial failed
       2 if interrupted
       3 if modem error
       4 if use ecu DCE dialer
--------------------------------------------------------------------------*/
int
hdb_dial(presult)
char **presult;
{
	int itmp;
	FILE *fp;
	CFG_PidType dial_pid;
	CFG_SigType(*original_sighdlr) ();
	DVE *tdve;
	char token[128];		 /* translated dialer token */
	int wait_status;
	struct dlent *tdle = (struct dlent *)0;
	char baudstr[16];
	char dbgstr[16];
	char dial_log[128];
	char dialprog_path[ECU_MAXPN];
	char *stripped_num;
	char *sptr;
	char *dptr;
	struct stat st;
	UINT32 colors_at_entry = colors_current;
	char credit_file[ECU_MAXPN];
	char *error_name = "";
	int error_baud = 0;
	int rcvr_restart = need_rcvr_restart();
	int old_ttymode = get_ttymode();

	static char stat_s64[64];

	/*
	 * we may do nothing
	 */
	if (ck_sigint())		 /* don't even start if console interrupt
							  * posted */
	{
		sigint = 0;
		return (2);
	}

	if (!there_is_hdb_on_this_machine)
		return (4);

	/*
	 * kill receiver if it is active
	 */
	if (rcvr_restart)
		kill_rcvr_process(SIGUSR1);

#if defined(CHOOSE_DEBUG)
	vlogevent(getpid(), "HDB_DIAL Lline=%s Lbitrate=%d",
		shm->Lline, shm->Lbitrate);
#endif

	/*
	 * get a Devices entry appropriate for dialing on the line;
	 */
	enddvent();
	while (tdve = getdvline(shm->Lline + 5))
	{
		if ((tdve->low_baud <= shm->Lbitrate) &&
			(shm->Lbitrate <= tdve->high_baud))
		{
			break;
		}
	}
	error_name = shm->Lline + 5;
	error_baud = shm->Lbitrate;
	enddvent();

	if (!tdve)
	{
		pprintf("no Devices entry for %s at %u baud ... trying ecu dialer\n",
			error_name, error_baud);
		return (4);
	}

	dial_log[0] = 0;
	if (*tdve->dialprog != '/')
	{

		/*
		 * per user demand, check for dialer program w/o leading /
		 */

		strcpy(dialprog_path, hdblibdir);
		strcat(dialprog_path, "/");
		itmp = strlen(dialprog_path);
		strncat(dialprog_path, tdve->dialprog, sizeof(dialprog_path) - itmp);
		dialprog_path[sizeof(dialprog_path) - 1] = 0;
		if (!stat(dialprog_path, &st) && ((st.st_mode & S_IFMT) == S_IFREG) &&
			(st.st_mode & 0111))	/* if regular file exists and
									 * executable */
		{
			tdve->dialprog = dialprog_path;
		}
		else
		{
			tdle = getdlentname(tdve->dialprog);
			enddlent();
			if (!tdle)
			{
				sprintf(dial_log,
					"UUCPDIAL Devices entry %s: '%s' not found in Dialers",
					shm->Lline + 5, tdve->dialprog);
			}
		}
	}
	else if (access(tdve->dialprog, 1))
	{
		sprintf(dial_log, "UUCPDIAL Devices entry %s: (%s) %s",
			shm->Lline + 5, tdve->dialprog, strerror(errno));
	}

	if (dial_log[0])
	{
#if defined(LOG_HDBDIAL)
		logevent(getpid(), dial_log);
#endif
		pputs(dial_log + 9);
		pputs("\n");
		return (4);
	}

	stripped_num = strip_phone_num(shm->Ltelno);

	/*
	 * if trailing '$', read and append ~/.ecu/.credit
	 */
	dptr = stripped_num;
	if (*(dptr - 1) == '$')
	{
		*--dptr = 0;
		get_home_dir(credit_file);
		strcat(credit_file, "/.ecu/.credit");
		chmod(credit_file, 0400);	/* let's keep this one quiet */
		if (fp = fopen(credit_file, "r"))
		{
			fgets(dptr, 30, fp);
			fclose(fp);
		}
		if (!fp || !(*dptr))
		{
			strcpy(sv[0]->pb, "!CREDIT CARD ERROR");
			sv[0]->cb = strlen(sv[0]->pb);
			pputs("\ncredit card error\n");
			iv[0] = 1;
			return (1);
		}
		if (*(dptr + strlen(dptr) - 1) == 0x0A)
			*(dptr + strlen(dptr) - 1) = 0;	/* kill NL */
	}

	/* Translate Token now (thanks to ache@hq.demos.su) */

	if (tdve->token == (char *)0 || !tdve->token[0])
		strcpy(token, stripped_num);
	else
	{
		dptr = token;
		for (sptr = tdve->token; *sptr; sptr++)
			if (*sptr != '\\')
				*dptr++ = *sptr;
			else
			{
				char *s, *t;

				switch (*(sptr + 1))
				{

						/* Direct */
					case 'D':
						sptr++;
						s = stripped_num;
						while (*s)
							*dptr++ = *s++;
						break;

						/* Dialcodes Translate */
					case 'T':
						sptr++;
						s = stripped_num;
						t = dialcodes_translate(&s);
						while (*t)
							*dptr++ = *t++;
						while (*s)
							*dptr++ = *s++;
						break;

					default:
						*dptr++ = '\\';
						break;
				}
			}
		*dptr = 0;
	}

	pprintf("Type %s to abort ... ",
		(kbdintr == 0x7F) ? "DEL" : graphic_char_text(kbdintr, 0));
	setcolor(colors_normal);

	if (Ldial_debug_level)
	{
		ttymode(0);
		pputs("\n");
	}
	else
		ttymode(2);

	if (!tdle)
	{
		int accessible = (access(tdve->dialprog, 1) == 0);
		int dial_as_uucp = 0;

		/*
		 * access() checks to see if the ORIGINAL uid
		 * can access, not the euid
		 */
		if(!accessible && setuid_uucp)
		{
			if (!stat(tdve->dialprog, &st) &&
				((unsigned)st.st_uid == (unsigned)uid_uucp) &&
				!(st.st_mode & 1))
			{
				dial_as_uucp++;
				accessible = 1;
			}
		}

		/*
		 * check final word on accessibility
		 */
		if (!accessible)
		{
			pperror(tdve->dialprog);
			pputs("trying ecu dialer\n");
			return (4);
		}

		/*
		 * stops a bug but is only a catch for one case
		 * (more work here)
		 */
		if (!dial_as_uucp && setuid_uucp &&
			!stat(shm->Lline, &st) &&
			((unsigned)st.st_uid == (unsigned)uid_uucp))
		{
			dial_as_uucp++;
		}

		sprintf(baudstr, "%u", shm->Lbitrate);
		sprintf(dbgstr, "-x%d", Ldial_debug_level);
		original_sighdlr = signal(SIGCLD, SIG_DFL);
		if ((dial_pid = smart_fork()) == 0)
		{

			signal(SIGINT, SIG_DFL);
			if(dial_as_uucp)
				Setuid(uid_uucp);
			execl(tdve->dialprog,
#if defined(WHT) || defined(CFG_UseEcuDial)
				"ECUdial",	 /* tell dialer ECU is calling */
#else
				tdve->dialprog,
#endif
				dbgstr, shm->Lline, token, baudstr, (char *)0);
			_exit(0xFF);	 /* did not execute */
		}

		wait_status = (RC_FAIL | RCE_SIG) << 8;
		while (((itmp = wait(&wait_status)) != dial_pid) && (itmp != -1))
			;
		signal(SIGCLD, original_sighdlr);
		ttymode(old_ttymode);
		ttyflush(0);

		if (ck_sigint())	 /* keyboard interrupt? */
		{
			kill((CFG_PidType) dial_pid, 9);	/* kill dialer */
			lflash_dtr();	 /* drop line */
			sigint = 0;		 /* reset SIGINT indication */
		}
		lreset_ksr();		 /* uucp dialers are nice guys, but lets use
							  * our termio */

#if defined(LOG_HDBDIAL)
		if (wait_status)
		{
			vlogevent(getpid(), "UUCPDIAL %s %s exit status0x%04x",
				tdve->dialprog, token, wait_status & 0xFFFF);
		}
#endif

		/*
		 * if system reports interrupt, fake dial-reported status
		 */
		if (wait_status & 0xFF)
			wait_status = (RC_FAIL | RCE_SIG) << 8;

		if ((wait_status & 0xFF00) == 0xFF00)
		{
			pprintf("%s did not execute ... trying ecu dialer\n", tdve->dialprog);
			return (4);
		}

		wait_status = (wait_status >> 8) & 0xFF;
		wait_status &= ~RC_ENABLED;

		if (!(wait_status & ~RC_BAUD))
		{
			char *cp;

			wait_status &= RC_BAUD;
			switch (wait_status)
			{
				case 0:
					cp = baudstr;
					break;	 /* SAME */
				case B50:
					cp = "50";
					break;
				case B75:
					cp = "75";
					break;
				case B110:
					cp = "110";
					break;
				case B134:
					cp = "134.5";
					break;
				case B150:
					cp = "150";
					break;
				case B200:
					cp = "200";
					break;
				case B300:
					cp = "300";
					break;
				case B600:
					cp = "600";
					break;
				case B1200:
					cp = "1200";
					break;
				case B1800:
					cp = "1800";
					break;
				case B2400:
					cp = "2400";
					break;
				case B4800:
					cp = "4800";
					break;
				case B9600:
					cp = "9600";
					break;
#if defined(B19200)
				case B19200:
					cp = "19200";
					break;
#endif
#if defined(B38400)
				case B38400:
					cp = "38400";
					break;
#endif
#if defined(B57600)
				case B57600:
					cp = "57600";
					break;
#endif
#if defined(B115200)
				case B115200:
					cp = "115200";
					break;
#endif
				default:
					switch (wait_status)
					{
						case EXTA:
							cp = "EXTA";
							break;
						case EXTB:
							cp = "EXTB";
							break;
						default:
							cp = "????";
							break;
					}
			}

			sprintf(stat_s64, "CONNECT %s", cp);
			*presult = stat_s64;	/* DCE_dial will report result code */
			return (0);
		}

		*presult = hdb_dial_error_text(wait_status);
		setcolor(colors_error);
		pputs(*presult);
		setcolor(colors_at_entry);
		pputc('\n');
		lflash_dtr();
	}
	else
	{
		pprintf("using Dialers entry '%s'\n", tdle->name);
		last_Speed_result[0] = 0;
		if (execute_expresp(tdle->script))
		{
			*presult = "DIALER SCRIPT FAILED";
			setcolor(colors_error);
			pputs(*presult);
			setcolor(colors_at_entry);
			pputc('\n');
			iv[0] = 1;
		}
		else
		{
			if (last_Speed_result[0])
				*presult = last_Speed_result;
			else
			{
				sprintf(stat_s64, "CONNECT %u", shm->Lbitrate);
				*presult = stat_s64;	/* DCE_dial will report result
										 * code */
			}
			setcolor(colors_at_entry);
			iv[0] = 0;
		}
	}

	/*
	 * restart receiver if we killed it
	 */
	if (rcvr_restart)
		start_rcvr_process(1);

	return ((int)iv[0]);

}							 /* end of hdb_dial */

/*+-------------------------------------------------------------------------
	hdb_init() - initialize HoneyDanBerInterface
--------------------------------------------------------------------------*/
void
hdb_init()
{
	int itmp;
	int buflen = strlen(hdblibdir) + 64;
	char *emsg = "hdb_init memory allocation failed!\n";
	struct stat st;

	if (!(Devices_file = malloc(buflen)))
	{
		pputs(emsg);
		errno = -1;
		termecu(TERMECU_MALLOC);
	}
	strcpy(Devices_file, hdblibdir);
	strcat(Devices_file, "/Devices");

	if (!(Dialers_file = malloc(buflen)))
	{
		pputs(emsg);
		errno = -1;
		termecu(TERMECU_MALLOC);
	}
	for (itmp = 0; itmp < UNGETTY_LIST_MAX; itmp++)
	{
		if (!(ungetty_list[itmp] = malloc(64)))
		{
			pputs(emsg);
			errno = -1;
			termecu(TERMECU_MALLOC);
		}
	}
	strcpy(Dialers_file, hdblibdir);
	strcat(Dialers_file, "/Dialers");

	if (!(Dialcodes_file = malloc(buflen)))
	{
		pputs(emsg);
		errno = -1;
		termecu(TERMECU_MALLOC);
	}
	strcpy(Dialcodes_file, hdblibdir);
	strcat(Dialcodes_file, "/Dialcodes");

	there_is_hdb_on_this_machine = !stat(Devices_file, &st);

}							 /* end of hdb_init */

/*+-------------------------------------------------------------------------
dialcodes_translate(phone)  -  translate first part of phone using Dialcodes
--------------------------------------------------------------------------*/
char *
dialcodes_translate(phone)
char **phone;
{
	FILE *f;
	int itmp;

#define MAX_DLC_TOKENS 2
	char *tokens[MAX_DLC_TOKENS];
	static char dlstr[128];

	if (!(f = fopen(Dialcodes_file, "r")))
		return ("");

	while (fgets(dlstr, sizeof(dlstr), f))
	{
		if (((itmp = strlen(dlstr)) > 0) && (dlstr[itmp - 1] == '\n'))
			dlstr[--itmp] = 0;
		if ((dlstr[0] == '#') || (dlstr[0] == ' ') || (!itmp))
			continue;
		if (tokens[0] = arg_token(dlstr, " \t\r\n"))
		{
			if (!(tokens[1] = arg_token((char *)0, " \t\r\n")))
				tokens[1] = "";
			itmp = strlen(tokens[0]);
			if (strncmp(*phone, tokens[0], itmp))
				continue;
			fclose(f);
			*phone += itmp;
			fclose(f);
			return (tokens[1]);
		}
		break;
	}

	fclose(f);
	return ("");
}

/*+-------------------------------------------------------------------------
      strip_phone_num(sptr) - remove junk characters from phone
--------------------------------------------------------------------------*/
char *
strip_phone_num(sptr)
char *sptr;
{
	static char stripped_num[64];
	char *dptr;

	dptr = stripped_num;
	while (*sptr)
	{
		if ((*sptr == '(') || (*sptr == ')')
#if defined(WHT) || defined(STRIP_TELNO_HYPHENS)
			|| (*sptr == '-')/* some want '-' for pauses; I use ',' */
#endif
			)
		{
			sptr++;
			continue;
		}
		*dptr++ = *sptr++;
	}
	*dptr = 0;

	return (stripped_num);
}

/*+-------------------------------------------------------------------------
	report_initial_line()
--------------------------------------------------------------------------*/
void
report_initial_line()
{
#ifdef CHOOSE_DEBUG
	vlogevent(getpid(), "INITIAL_LINE '%s'", shm->Lline);
#endif /* CHOOSE_DEBUG */
}							 /* end of report_initial_line */

/* vi: set tabstop=4 shiftwidth=4: */
/* end of hdbintf.c */
