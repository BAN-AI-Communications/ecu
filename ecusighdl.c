/*+-----------------------------------------------------------------------
	ecusighdl.c - xmtr/rcvr individual process signal handlers
	wht@wht.net

  Defined functions:
	child_signals()
	ck_sigint()
	gag_me()
	kill_rcvr_process(sig)
	rcvr_SIGUSR2_handler(sig)
	rcvr_common_signal_handler(sig)
	rcvr_death_handler(sig)
	rcvr_signals()
	sig_report(sig)
	termecu(code)
	termecu_code_text(code)
	xmtr_SIGCLD_handler(sig)
	xmtr_SIGHUP_handler(sig)
	xmtr_SIGINT_handler(sig)
	xmtr_SIGTERM_handler(sig)
	xmtr_SIGUSR2_handler(sig)
	xmtr_death_handler(sig)
	xmtr_signals()

------------------------------------------------------------------------*/
/*+:EDITS:*/
/*:04-26-2000-11:15-wht@bob-RELEASE 4.42 */
/*:01-24-1997-02:37-wht@yuriatin-SOURCE RELEASE 4.00 */
/*:10-29-1996-18:27-wht@yuriatin-immediate SIG_IGN upon termecu entry */
/*:09-11-1996-19:59-wht@yuriatin-3.48-major telnet,curses,structural overhaul */
/*:08-25-1996-02:15-wht@fep-fix MOTSVR4 SIGTTOU */
/*:08-11-1996-02:10-wht@kepler-rename ecu_log_event to logevent */
/*:08-10-1996-13:44-wht@kepler-harden telnet close */
/*:07-03-1996-14:54-wht@kepler-amplify TERMECU_CURSES_ERROR message */
/*:07-03-1996-14:49-wht@kepler-do not print termecu errno if normal */
/*:07-03-1996-14:29-wht@kepler-tcap_curbotleft used made permanent */
/*:01-27-1996-20:26-wht@n4hgf-use child_pid decl in ecu.h */
/*:12-11-1995-17:43-wht@kepler-kill any child in child_pid on termecu */
/*:12-06-1995-13:30-wht@n4hgf-termecu w/errno -1 consideration */
/*:11-23-1995-11:20-wht@kepler-source control 3.37 for tsx-11 */
/*:11-14-1995-10:23-wht@kepler-3.37.80-source control point: SOCKETS */
/*:11-13-1995-16:02-wht@kepler-restore placing cursor at bot/left on exit */
/*:11-13-1995-12:26-wht@kepler-start_rcvr_process moved to ecurcvr.c */
/*:11-12-1995-01:52-wht@gyro-use shmr_notify_termecu instead of SIGHUP xmtr */
/*:11-04-1995-14:40-wht@wwtp1-telnet termecu fixes: omit DCE_hangup */
/*:09-17-1995-16:25-wht@kepler-belated add TERMECU_XMTR_WRITE_ERROR text */
/*:05-11-1995-15:55-wht@n4hgf-ck_sigint fools optimizing compilers */
/*:05-04-1994-04:39-wht@n4hgf-ECU release 3.30 */
/*:11-25-1993-14:47-wht@n4hgf-call shm_done AFTER restore_initial_colors */
/*:10-04-1993-03:57-wht@n4hgf-simplify rcvr signal service + better reporting */
/*:10-04-1993-01:58-wht@n4hgf-spice up term diags + report sigs for 1st time */
/*:08-30-1993-12:39-wht@n4hgf-revert WHT to catch SEGV */
/*:08-07-1993-20:25-wht@n4hgf-if WHT, do not catch SEGV, etc. */
/*:06-26-1993-16:33-wht@n4hgf-check for rcvr active in rcvr death test */
/*:10-18-1992-14:11-wht@n4hgf-FAS 2.10 users getting SIGUSR1 on xmtr */
/*:09-16-1992-13:29-wht@n4hgf-add TERMECU_UNRECOVERABLE text */
/*:09-10-1992-13:59-wht@n4hgf-ECU release 3.20 */
/*:08-22-1992-15:38-wht@n4hgf-ECU release 3.20 BETA */
/*:08-17-1992-04:55-wht@n4hgf-keep rcvr pid in shm for friend code */
/*:08-16-1992-03:08-wht@n4hgf-head off another POSIX plot */
/*:08-16-1992-01:54-wht@n4hgf-job control signals get SIG_IGN */
/*:04-29-1992-19:04-wht@n4hgf-make a pass at handling job control signals */
/*:04-29-1992-13:46-wht@n4hgf-ignore SIGQUIT */
/*:04-23-1992-16:20-wht@n4hgf-disable mysterious rcvr SIGCLD events */
/*:02-16-1992-01:42-wht@n4hgf-turn off xterm_title + add _terminate.ep */
/*:08-25-1991-23:56-wht@n4hgf2-handle xmtr core dump gracefully */
/*:07-25-1991-12:56-wht@n4hgf-ECU release 3.10 */
/*:07-17-1991-07:04-wht@n4hgf-avoid SCO UNIX nap bug */
/*:06-29-1991-15:42-wht@n4hgf-if WHT and xterm, play with title bar */
/*:01-29-1991-12:57-wht@n4hgf-on exit, restore setcolor colors if possible */
/*:12-18-1990-20:02-wht@n4hgf-add rcvr_death_handler */
/*:09-19-1990-19:36-wht@n4hgf-logevent now gets pid for log from caller */
/*:08-14-1990-20:40-wht@n4hgf-ecu3.00-flush old edit history */

#include "ecu.h"
#include "ecufork.h"

extern int windows_active;
extern int current_ttymode;
extern int ttymode_termecu_on_sigint;
extern int rcvr_log;
extern FILE *rcvr_log_fp;
extern char rcvr_log_file[]; /* if rcvr_log!= 0,log filename */
extern int rcvr_log_append;

int sigint = 0;				 /* interrupt indicator */
int proc_interrupt = 0;		 /* procedure interrupt indicator */
int last_child_wait_status;
int last_child_wait_pid;
int xmtr_killed_rcvr;

char *signal_name_text();

void xmtr_signals();
void rcvr_signals();
void child_signals();

CFG_SigType kill_rcvr_process();

CFG_SigType rcvr_SIGUSR2_handler();
CFG_SigType xmtr_SIGINT_handler();
CFG_SigType xmtr_SIGHUP_handler();
CFG_SigType xmtr_SIGTERM_handler();
CFG_SigType xmtr_SIGCLD_handler();
CFG_SigType xmtr_death_handler();
CFG_SigType rcvr_common_signal_handler();
CFG_SigType rcvr_death_handler();

/*
 * macros for wait() status ... in case they aren't defined
 */
#undef WIFEXITED			 /* in case they were */
#undef WEXITSTATUS
#undef WIFSIGNALED
#undef WTERMSIG
#undef WIFSTOPPED
#undef WSTOPSIG

#define WIFEXITED(status)	((status & 0xFF) == 0)
#define WEXITSTATUS(status)	((status >> 8) & 0xFF)
#define WIFSIGNALED(status)	((status) && ((status & 0x00FF) == 0xFF))
#define WTERMSIG(status)	(status & 0x7F)
#define WIFSTOPPED(status)	((status & 0xFF) == 0xFF)
#define WSTOPSIG(status)	((status >> 8) & 0xFF)

/*+-------------------------------------------------------------------------
	termecu_code_text(code)
--------------------------------------------------------------------------*/
char *
termecu_code_text(code)
int code;
{
	static char errant[16];
	char *signal_name_text();

	if ((code >= TERMECU_SIG1) && (code <= TERMECU_SIGN))
		return (signal_name_text(code));

	switch (code)
	{
		case TERMECU_BSD4_IOCTL:
			return ("BSD4 ioctl error");
		case TERMECU_CONFIG_ERROR:
			return ("configuration error");
		case TERMECU_CURSES_ERROR:
			return ("error in curses use/terminal configuration");
		case TERMECU_GEOMETRY:
			return ("unsupported screen geometry");
		case TERMECU_INIT_PROC_ERROR:
			return ("error during initial procedure");
		case TERMECU_IPC_ERROR:
			return ("IPC (shm/sem) init failed");
		case TERMECU_LINE_OPEN_ERROR:
			return ("line open error");
		case TERMECU_LINE_READ_ERROR:
			return ("line read error");
		case TERMECU_LOGIC_ERROR:
			return ("internal logic error");
		case TERMECU_MALLOC:
			return ("critical memory allocation failure");
		case TERMECU_NO_FORK_FOR_RCVR:
			return ("can't fork for RCVR");
		case TERMECU_PWENT_ERROR:
			return ("password entry error");
		case TERMECU_RCVR_FATAL_ERROR:
			return ("detected RCVR FATAL ERROR");
		case TERMECU_SHM_ABL:
			return ("SHM ABL error");
		case TERMECU_SHM_RTL:
			return ("SHM RTL error");
		case TERMECU_SVC_NOT_AVAIL:
			return ("service not available");
		case TERMECU_TTYIN_READ_ERROR:
			return ("keyboard read error");
		case TERMECU_UNRECOVERABLE:
			return ("unrecoverable error");
		case TERMECU_USAGE:
			return ("usage");
		case TERMECU_XMTR_FATAL_ERROR:
			return ("detected XMTR FATAL ERROR");
		case TERMECU_XMTR_WRITE_ERROR:
			return ("XMTR could not write to line");
		default:
			sprintf(errant, "code %u?", code);
			return (errant);
	}

}							 /* end of termecu_code_text */

/*+-------------------------------------------------------------------------
	gag_me() - adb/sdb/gdb/dbx/dbxtra hack for catching trauma
--------------------------------------------------------------------------*/
void
gag_me()
{
	char *x = (char *)0xffff0000;

	*x = 1;
}							 /* end of gag_me */

/*+-----------------------------------------------------------------------
	termecu(code) -- terminate program (with cleanup)

  see termecu.h for a list of codes

  Separate processing for rcvr and xmtr processes;  rcvr entry
  is only upon some kind of serious error and it more less just dies,
  causing xmtr process to wake up with SIGCLD and come in here.

  Upon entry by xmtr process:
    close comm line
    run any _terminate.ep procedure
    return any ungetty'd line
    return user's console to normal status
    remove shm segment
    terminate program

------------------------------------------------------------------------*/
void
termecu(code)
int code;
{
	static int already_in_termecu = 0;	/* one per fork */
	int isig;
	int save_errno = errno;
	char s256[256];
	extern char initial_procedure[];
	char *signal_name_text();

	for (isig = 1; isig < NSIG; isig++)
		signal(isig, SIG_IGN);

	if (already_in_termecu)
	{
		gag_me();
		pprintf("\n\n\n%s REENTERED TERMECU ... CANNOT RECOVER\n",
			(getpid() == xmtr_pid) ? "XMTR" : "RCVR");
		ttymode(0);			 /* normal tty status */
		exit(255);
	}
	already_in_termecu = 1;

	if (shm)				 /* tell friends goodbye */
		shm->terminating = 1;

	if (xmtr_pid == getpid())/* if we are xmtr */
	{
		kill_rcvr_process(SIGUSR1);
		if (child_pid > 0)
			kill(child_pid, SIGKILL);
		if (windows_active)
			windows_end_signal();
		tcap_curbotleft();
		tcap_eeod();

		if (shm && (shm->Liofd != -1))
			lclose();

#if defined(CFG_TelnetOption)
		if (shm && !shm->Ltelnet)
#endif
		{
			if (shm && shm->Lconnected)
				DCE_hangup();
		}

		if (find_procedure("_terminate"))
		{
			char code_str[16];
			char *_doproc_args[2];

			_doproc_args[0] = "_terminate";	/* _terminate.ep */
			sprintf(code_str, "%d", code);
			_doproc_args[1] = code_str;
			(void)do_proc(2, _doproc_args);
		}

		/*
		 * make SURE we release any line(s) acquired from getty
		 */
		ungetty_return_line((char *)0, "terminating");

		ttymode(0);			 /* normal tty status */
		if (!code)
			;
		else if (code <= TERMECU_SIGN)
		{
			pprintf("## XMTR caught signal %d (%s)\n",
				code, signal_name_text(code));
		}
		else
		{
			setcolor(colors_error);
			if (code == TERMECU_INIT_PROC_ERROR)
				pprintf("initial procedure '%s' failed\n", initial_procedure);
			else if ((code > TERMECU_INIT_PROC_ERROR) &&
				(code <= TERMECU_INIT_PROC_ERROR + 32))
			{
				pprintf("procedure command: exit %d\n",
					code - TERMECU_INIT_PROC_ERROR);
			}
			else
			{
				sprintf(s256, "## XMTR: %s", termecu_code_text(code));
				pputs(s256);
				if (save_errno > 0)
				{
					char *cp = s256 + strlen(s256);

					sprintf(cp, ", errno = %d", save_errno);
					pputs(cp);
				}
				pputs("\n");
				if (lopen_err_str[0])
				{
					pputs(lopen_err_str);
					pputs("\n");
				}
				logevent(getpid(), s256);
				errno = save_errno;
				if (errno > 0)
					pperror("errno may not apply, but");
			}
		}
		restore_initial_colors();
		shm_done();
	}
	else
	{
		/* * we are in the rcvr */
		if (code <= TERMECU_SIGN)
		{
			pprintf("## RCVR caught signal %d (%s)\n",
				code, signal_name_text(code));
		}
		else
		{
			sprintf(s256, "## RCVR: %s", termecu_code_text(code));
			pputs(s256);
			if (save_errno > 0)
			{
				char *cp = s256 + strlen(s256);

				sprintf(cp, ", errno = %d", save_errno);
				pputs(cp);
			}
			pputs("\n");
			logevent(getpid(), s256);
			errno = save_errno;
			if (errno > 1)
				pperror("errno may not apply, but");
		}
		ttymode(0);			 /* normal tty status */
		restore_initial_colors();
		shmr_notify_termecu();
	}
	exit(code);
	/* NOTREACHED */

}							 /* end of termecu */

/*+-----------------------------------------------------------------------
	kill_rcvr_process(sig) -- kill rcvr process with signal 'sig'
------------------------------------------------------------------------*/
CFG_SigType
kill_rcvr_process(sig)
int sig;
{
	int wait_count = 70;

	if (shm->rcvr_pid > 0)	 /* if we have forked a rcvr process */
	{
		xmtr_killed_rcvr = 1;
		rcvr_log_fp = (FILE *) 0;
		xmtr_signals();
		kill(shm->rcvr_pid, sig);
		if (sig != SIGUSR2)	 /* rcvr does not die on SIGUSR2 */
		{
			errno = 0;
			while (wait_count)
			{
				if (kill(shm->rcvr_pid, 0) && (errno == ESRCH))
					break;
				errno = 0;
				Nap(40L);
				wait_count--;
			}
			if (!wait_count)
			{
				while (!kill(shm->rcvr_pid, SIGKILL))
				{
					wait((int *)0);
					Nap(40L);
				}
			}
			shm->rcvr_pid = -1;	/* no receiver active */
			if (rcvr_log && rcvr_log_file[0])
				rcvr_log_fp = fopen(rcvr_log_file, "a");

			rcvrdisp_actual();	/* write any buffered screen data */
		}
	}

}							 /* end of kill_rcvr_process */

/*+-------------------------------------------------------------------------
	rcvr_common_signal_handler(sig)
--------------------------------------------------------------------------*/
CFG_SigType
rcvr_common_signal_handler(sig)
int sig;
{
	extern int rcvr_log;
	extern int rcvr_log_raw;
	extern FILE *rcvr_log_fp;

	if (rcvr_log)
	{
		if (!rcvr_log_raw)
			fputs("\n", rcvr_log_fp);
		fclose(rcvr_log_fp);
	}

	if (sig == SIGUSR1)
		_exit(0);

	termecu(sig - 1 + TERMECU_SIG1);

}							 /* end of rcvr_common_signal_handler */

/*+-------------------------------------------------------------------------
	rcvr_SIGUSR2_handler(sig)
--------------------------------------------------------------------------*/
CFG_SigType
rcvr_SIGUSR2_handler(sig)
int sig;
{
	signal(SIGUSR2, rcvr_SIGUSR2_handler);
	shmr_process_rcvr_SIGUSR2(sig);
}							 /* end of rcvr_SIGUSR2_handler */

/*+-------------------------------------------------------------------------
	rcvr_death_handler(sig) - unexpected signal; try to dump core
--------------------------------------------------------------------------*/
CFG_SigType
rcvr_death_handler(sig)
int sig;
{
	int itmp;

#ifdef WHT	
	int *open_elevator_shaft = (int *)0xb0000001;

#endif

	ttymode(0);
	pprintf("\nreceiver process caught signal %d (%s)\n",
		sig, signal_name_text(sig));
	pprintf("screen cursor (y,x) = (%u,%u)\n", shm->cursor_y, shm->cursor_x);
	for (itmp = 1; itmp < NSIG; itmp++)
		signal(itmp, SIG_DFL);
#ifdef WHT
	signal(SIGSEGV, SIG_DFL);
	printf("trapping at 08lx\n", (unsigned long)open_elevator_shaft);
	fflush(stdout);
	*open_elevator_shaft = itmp;
#else
	rcvr_common_signal_handler(sig);
#endif
	_exit(-1);

}							 /* end of rcvr_death_handler */

/*+-------------------------------------------------------------------------
	ck_sigint() - return 'sigint' value to fool optimizers
Some optimizing compilers elide if(sigint) checks
--------------------------------------------------------------------------*/
int
ck_sigint()
{
	return (sigint);
}							 /* end of ck_sigint */

/*+-------------------------------------------------------------------------
	xmtr_SIGINT_handler()
--------------------------------------------------------------------------*/
CFG_SigType
xmtr_SIGINT_handler(sig)
int sig;
{
	if (ttymode_termecu_on_sigint)
		termecu(SIGINT);

	signal(sig, xmtr_SIGINT_handler);
	sigint = 1;
	proc_interrupt = 1;
}							 /* end of xmtr_SIGINT_handler */

/*+-------------------------------------------------------------------------
	xmtr_SIGHUP_handler(sig)
--------------------------------------------------------------------------*/
CFG_SigType
xmtr_SIGHUP_handler(sig)
int sig;
{
	errno = -1;
	termecu(sig - 1 + TERMECU_SIG1);
}							 /* end of xmtr_SIGHUP_handler */

/*+-------------------------------------------------------------------------
	xmtr_SIGTERM_handler(sig)
--------------------------------------------------------------------------*/
CFG_SigType
xmtr_SIGTERM_handler(sig)
int sig;
{
	errno = -1;
	termecu(sig - 1 + TERMECU_SIG1);
}							 /* end of xmtr_SIGTERM_handler */

/*+-------------------------------------------------------------------------
	xmtr_SIGUSR2_handler()
--------------------------------------------------------------------------*/
CFG_SigType
xmtr_SIGUSR2_handler(sig)
int sig;
{
	CFG_SigType xmtr_SIGUSR2_handler();

	signal(sig, xmtr_SIGUSR2_handler);
	shmx_process_xmtr_SIGUSR2();

}							 /* end of xmtr_SIGUSR2_handler */

/*+-------------------------------------------------------------------------
	xmtr_death_handler(sig) - unexpected signal; try to dump core
--------------------------------------------------------------------------*/
CFG_SigType
xmtr_death_handler(sig)
int sig;
{
	int itmp;

#ifdef WHT
	int *open_elevator_shaft = (int *)0xb0000000;

#endif

	ttymode(0);
	pprintf("\ntransmitter process caught signal %d (%s)\n",
		sig, signal_name_text(sig));
	kill_rcvr_process(SIGUSR1);
	for (itmp = 1; itmp < NSIG; itmp++)
		signal(itmp, SIG_DFL);
#ifdef WHT
	signal(SIGSEGV, SIG_DFL);
	printf("trapping @ %08lx\n", (unsigned long)open_elevator_shaft);
	fflush(stdout);
	*open_elevator_shaft = itmp;
#else
	kill((CFG_PidType) getpid(), SIGIOT);
#endif
	errno = -1;
	termecu(sig - 1 + TERMECU_SIG1);
}							 /* end of xmtr_death_handler */

/*+-------------------------------------------------------------------------
	xmtr_SIGCLD_handler(sig)
--------------------------------------------------------------------------*/
CFG_SigType
xmtr_SIGCLD_handler(sig)
int sig;
{
	int itmp;

#if defined(FORK_DEBUG)
	char s512[512];

#endif

  WAIT:
	errno = 0;
	if ((last_child_wait_pid = wait(&last_child_wait_status)) < 0)
	{
		if (errno == EINTR)
			goto WAIT;
	}

#if defined(FORK_DEBUG)
	sprintf(s512, "XMTR SIGCLD pid %d (%s) s=%04x ",
		last_child_wait_pid,
		(last_child_wait_pid == shm->rcvr_pid) ? "RCVR!" : "AUXOP",
		last_child_wait_status);
	if (WIFEXITED(last_child_wait_status))
	{
		sprintf(s512 + strlen(s512), "exit status=%d ",
			WEXITSTATUS(last_child_wait_status));
	}
	if (WIFSIGNALED(last_child_wait_status))
	{
		sprintf(s512 + strlen(s512), "signal=%d ",
			WTERMSIG(last_child_wait_status));
	}
	logevent(getpid(), s512);/* xmtr_SIGCLD_handler() */
#endif

	if ((last_child_wait_pid == shm->rcvr_pid) && (shm->rcvr_pid > 0) &&
		!xmtr_killed_rcvr)
	{
		pprintf("\nECU receiver unexpectedly: wait status=0x%04x\n",
			last_child_wait_status);
		itmp = 0;
		if (WIFEXITED(last_child_wait_status))
		{
			itmp = 1;
			pprintf("exit status=%d ", WEXITSTATUS(last_child_wait_status));
		}
		if (WIFSIGNALED(last_child_wait_status))
		{
			itmp = 1;
			pprintf("signal=%d (%s)",
			signal_name_text(WTERMSIG(last_child_wait_status)));
		}
		if (itmp)
			pputs("\n");
		termecu(TERMECU_RCVR_FATAL_ERROR);
	}
	signal(sig, xmtr_SIGCLD_handler);

}							 /* end of xmtr_SIGCLD_handler */

/*+-------------------------------------------------------------------------
	sig_report(sig)
--------------------------------------------------------------------------*/
#if 0
CFG_SigType
sig_report(sig)
int sig;
{
	signal(sig, sig_report);

}							 /* end of sig_report */
#endif

/*+-------------------------------------------------------------------------
	child_signals() - signal() calls for children processes
--------------------------------------------------------------------------*/
void
child_signals()
{
	int isig;

	for (isig = 0; isig < NSIG; isig++)
		signal(isig, SIG_DFL);

}							 /* end of child_signals */

/*+-------------------------------------------------------------------------
	xmtr_signals()
--------------------------------------------------------------------------*/
void
xmtr_signals()
{
	int sig;

	for (sig = 1; sig < NSIG; sig++)
	{
		switch (sig)
		{

#ifdef WHT
			case SIGSEGV:
				break;
#endif

			case SIGHUP:
				signal(sig, xmtr_SIGHUP_handler);
				break;

#if	defined(SIGSTOP)

				/*
				 * call Roto-Rooter on POSIX plots
				 */
			case SIGSTOP:
			case SIGTSTP:
			case SIGCONT:
			case SIGTTIN:
				signal(sig, SIG_IGN);
				break;

				/*
				 * ditto, except on Motorola SVR4, we get a funky SIGTTOU
				 * after fork/exec/wait child exits; this happens to ECU
				 * when Motorola's csh and to most anybody else upon
				 * executing any shell
				 */
			case SIGTTOU:
#if defined(SVR4)
				signal(sig, SIG_DFL);
#else
				signal(sig, SIG_IGN);
#endif
				break;
#endif

#ifdef SIGWINCH
			case SIGWINCH:
				signal(sig, SIG_DFL);
				break;
#endif
#ifndef WHT
			case SIGQUIT:
				signal(sig, SIG_IGN);
				break;
#endif
			case SIGINT:
				signal(sig, xmtr_SIGINT_handler);
				break;
			case SIGTERM:
				signal(sig, xmtr_SIGTERM_handler);
				break;
			case SIGCLD:
				signal(sig, xmtr_SIGCLD_handler);
				break;
			case SIGUSR1:
				signal(sig, SIG_IGN);
				break;
			case SIGUSR2:
				signal(sig, xmtr_SIGUSR2_handler);
				break;
			default:
#ifndef WHT					 /* I want the bloody crash */
				signal(sig, xmtr_death_handler);
#endif
				break;
		}
	}

}							 /* end of xmtr_signals */

/*+-------------------------------------------------------------------------
	rcvr_signals()
--------------------------------------------------------------------------*/
void
rcvr_signals()
{
	int sig;

	for (sig = 1; sig < NSIG; sig++)
	{
		switch (sig)
		{

#if	defined(SIGSTOP)
			case SIGSTOP:
			case SIGTSTP:
			case SIGCONT:
			case SIGTTIN:
				signal(sig, SIG_IGN);
				break;

				/*
				 * ditto, except on Motorola SVR4, we get a funky SIGTTOU
				 * after fork/exec/wait child exits; this happens to ECU
				 * when Motorola's csh and to most anybody else upon
				 * executing any shell
				 */
			case SIGTTOU:
#if defined(SVR4)
				signal(sig, SIG_DFL);
#else
				signal(sig, SIG_IGN);
#endif
				break;
#endif

#ifdef SIGWINCH
			case SIGWINCH:
#endif
			case SIGCLD:
				signal(sig, SIG_DFL);
				break;
			case SIGQUIT:
				signal(sig, SIG_IGN);
				break;
			case SIGHUP:
			case SIGINT:
			case SIGTERM:
			case SIGUSR1:
				signal(sig, rcvr_common_signal_handler);
				break;
			case SIGUSR2:
				signal(sig, rcvr_SIGUSR2_handler);
				break;
			default:
#ifndef WHT					 /* I want the bloody crash */
				signal(sig, rcvr_death_handler);
#endif
				break;
		}
	}
}							 /* end of rcvr_signals */

/* vi: set tabstop=4 shiftwidth=4: */
