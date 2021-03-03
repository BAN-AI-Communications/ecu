/*+-----------------------------------------------------------------------
	ecu.c - Extended Calling Unit/Call Utility/Call UNIX/whatever
	wht@wht.net

  Defined functions:
	main(argc, argv)
	xmtr()

   "Now tell me, old man, what's your name?"
   "Holloway, sir. Charles William Holloway."
   "Oh yes, the town's librarian."
   "I have the honor, sir, and have had for many years."
   "I believe all that time spent living through other men's lives,
other men's dreams, what a waste."
   "Sometimes, a man can learn more from other men's dreams than he
can from his own.  Come visit me sir, if you wish to improve your
education and I may improve yours."  -- Something Wicked This Way Comes

------------------------------------------------------------------------*/
/*+:EDITS:*/
/*:04-26-2000-11:15-wht@bob-RELEASE 4.42 */
/*:12-19-1997-21:45-wht@kepler-add -k */
/*:12-18-1997-05:06-wht@sidonia-AIX port: sbrk and getpgrp */
/*:05-10-1997-13:53-wht@gyro-strncpy not strcpy to shm->ttyname */
/*:01-24-1997-02:36-wht@yuriatin-SOURCE RELEASE 4.00 */
/*:10-24-1996-03:11-wht@yuriatin-T switch now incremental sw for proc_trace */
/*:09-11-1996-19:59-wht@yuriatin-3.48-major telnet,curses,structural overhaul */
/*:08-21-1996-16:57-wht@fep-add -C switch for show_config */
/*:08-20-1996-12:39-wht@kepler-locale/ctype fixes from ache@nagual.ru */
/*:12-12-1995-14:22-wht@kepler-notify [no line attached] on keystroke */
/*:12-06-1995-13:30-wht@n4hgf-termecu w/errno -1 consideration */
/*:12-03-1995-19:57-wht@gyro-use Setuid */
/*:11-23-1995-11:20-wht@kepler-source control 3.37 for tsx-11 */
/*:11-14-1995-10:23-wht@kepler-3.37.80-source control point: SOCKETS */
/*:11-13-1995-12:07-wht@kepler-xmtr() can live with no line open */
/*:11-12-1995-01:11-wht@gyro-add switch -n to turn off rcvr_ansi_filter */
/*:11-12-1995-00:18-wht@gyro-show_telnet_traffic now in shm */
/*:11-10-1995-18:32-wht@gyro-add switch -z for show_telnet_traffic */
/*:11-04-1995-21:03-root@wwtp1-move windows rattle for quirk earlier in run */
/*:10-14-1995-16:17-wht@kepler-call build_valid_baud_string */
/*:09-17-1995-16:28-wht@kepler-remove obsolete #if 0 code */
/*:06-14-1995-19:08-wht@n4hgf-do not use setuid if root is real user */
/*:06-12-1995-15:03-wht@n4hgf-if ecu has uucp euid, make use of it */
/*:06-12-1995-14:13-wht@n4hgf-no more root euid nice code */
/*:01-12-1995-15:19-wht@n4hgf-apply Andrew Chernov 8-bit clean+FreeBSD patch */
/*:05-04-1994-04:38-wht@n4hgf-ECU release 3.30 */
/*:08-16-1993-17:19-wht@n4hgf-aid share debug with report_initial_line() */
/*:08-07-1993-20:38-wht@n4hgf-add xmtr_wfp_debug_hack */
/*:12-20-1992-12:08-wht@n4hgf-continue -l=Devices support work */
/*:12-04-1992-20:57-wht@n4hgf-add -l=Devices support */
/*:09-10-1992-13:58-wht@n4hgf-ECU release 3.20 */
/*:08-22-1992-15:38-wht@n4hgf-ECU release 3.20 BETA */
/*:08-16-1992-03:43-wht@n4hgf-add -F funckeytype */
/*:08-16-1992-03:08-wht@n4hgf-head off another POSIX plot */
/*:04-30-1992-00:49-wht@n4hgf-remove obsolete -c */
/*:04-24-1992-21:59-wht@n4hgf-more SCO tty name normalizing */
/*:04-17-1992-18:19-wht@n4hgf-"default" keyset read if present */
/*:02-16-1992-02:39-wht@n4hgf-add -P phonedir switch for rll@sco */
/*:02-16-1992-01:41-wht@n4hgf-turn off xterm_title */
/*:08-11-1991-19:56-wht@n4hgf-soup up -l for ISC vs. SCO */
/*:08-06-1991-13:02-wht@n4hgf-jpm@logixwi fix: HZ getenv test wrong sense */
/*:07-29-1991-17:57-wht@n4hgf-add memstat */
/*:07-25-1991-12:55-wht@n4hgf-ECU release 3.10 */
/*:07-17-1991-07:04-wht@n4hgf-avoid SCO UNIX nap bug */
/*:04-27-1991-01:52-wht@n4hgf-overhaul revision numbers */
/*:03-17-1991-13:44-wht@n4hgf-nice and uid revision */
/*:01-09-1991-22:31-wht@n4hgf-ISC port */
/*:11-30-1990-19:04-wht@n4hgf-new ttyinit parameter - see TTYINIT_... */
/*:11-28-1990-15:58-wht@n4hgf-add non-ansi terminal support */
/*:08-14-1990-20:39-wht@n4hgf-ecu3.00-flush old edit history */

#ifdef __FreeBSD__
#include <locale.h>
#endif
#include "ecu.h"
#include "dvent.h"
#include "esd.h"
#include "procedure.h"
#include "ecukey.h"
#include "ecuxkey.h"
#include "termecu.h"
#include "ecu_pwd.h"

char *getenv();
void xmtr_SIGINT_handler();

extern char *makedate;		 /* temporary make date */
extern char *revstr;		 /* ecunumrev.c */
extern char *revision_modifier;	/* ecunumrev.c */
extern ESD *icmd_prompt;
extern char kbdeof;			 /* current input EOF */
extern char kbdeol2;		 /* current secondary input EOL */
extern char kbdeol;			 /* current input EOL */
extern char kbderase;		 /* current input ERASE */
extern char kbdintr;		 /* current input INTR */
extern char kbdkill;		 /* current input KILL */
extern char kbdquit;		 /* current input QUIT */
extern UINT tcap_LINES;
extern UINT tcap_COLS;
extern UINT32 colors_current;
extern int there_is_hdb_on_this_machine;
extern char phonedir_name[PHONEDIR_NAME_SIZE];

char *eculibdir = CFG_EcuLibDir;
char *dash_f_funckeytype;
char hello_str[128];		 /* msg printed upon BOJ */
char errmsg[128];
char initial_procedure[128] = "";
char *default_tty = CFG_DefaultTty;	/* configurable default tty name */
int init_proc_argc = 0;
char *init_proc_argv[MAX_PARGV];
CFG_PidType xmtr_pid = 0;
int quit_on_init_proc_fail = 0;
int quit_on_init_proc_done = 0;
int rc_ep_has_run = 0;
struct TIMEB starting_timeb;

extern int hertz;					 /* HZ from environ or sys/param.h */
extern UINT32 hzmsec;				 /* clock period in msec rounded up */

#ifdef CFG_Malloc3X
char *startbrk;
char *startsp;

#endif

char *_rc = "_rc";			 /* _rc.ep */

/*+-----------------------------------------------------------------------
	xmtr() --  copy stdin to comm line

  THE INITIAL PROCESS EXECUTES THIS PROCESS UNTIL PROGRAM TERMINATION

------------------------------------------------------------------------*/
void
xmtr()
{
	UINT xmtr_char;
	char nlchar = NL;
	char tmpc;

	xmtr_wfp_debug_hack();	 /* don't ask */
	ttymode(1);
	xmtr_signals();
	while (1)
	{

		xmtr_char = ttygetc(1);

		if (xmtr_char & 0x100)
		{
			kbd_escape(xmtr_char);
			continue;
		}

		if (shm->Liofd <= 0)
		{
			UINT32 colors_at_entry = colors_current;

			setcolor(colors_notify);
			fputs("[no line attached]", se);
			setcolor(colors_at_entry);
			ff(se, "\r\n");
			continue;
		}

		lputc(xmtr_char);
		if (!shm->Lfull_duplex)
		{					 /* echo character if asked */
			tmpc = xmtr_char;
			write(TTYERR, &tmpc, 1);
		}

		if (xmtr_char == CRET)
		{
#if defined(CFG_TelnetOption)
			if (shm->Ltelnet && !shm->Ltelnet_raw)
				lputc(0);
#endif
			if (shm->Ladd_nl_outgoing)
				lputc('\n');
			if (!shm->Lfull_duplex)
				write(TTYERR, &nlchar, 1);
		}
	}
	/* NOTREACHED */

}							 /* end of xmtr */

/*+-------------------------------------------------------------------------
	keyboard_test_only() - diagnose keyboard problems
--------------------------------------------------------------------------*/
void
keyboard_test_only()
{
	ttyinit(TTYUSE_NORMAL);
	if (tty_not_char_special)
	{
		ff(se, "Cannot test keyboard unless it is stanmdard input\n");
		exit(1);
	}
	ttymode(1);				 /* put user console in `raw' mode */
	kbd_test();				 /* in ecutty.c */
	ttymode(0);				 /* restore keyboard */
	termecu(0);

}							 /* end of keyboard_test_only */

/*+-------------------------------------------------------------------------
	main(argc,argv)

  main() program forks to create rcvr process; then main()
  becomes the xmtr process
------------------------------------------------------------------------*/
int
main(argc, argv)
int argc;
char **argv;
{
	int swchar;
	int itmp;
	UINT32 colors_save;
	struct passwd *pw;
	char *get_ttyname();
	extern char *optarg;
	extern int optind;

#ifdef CFG_Malloc3X

	/*
	 * absolutely must be first on the list
	 */
	startbrk = sbrk(0);		 /* initial break */
	startsp = (char *)&swchar;	/* initial sp */
	(void)mallopt(M_MXFAST, 256);
	(void)mallopt(M_NLBLKS, 64);
	(void)mallopt(M_GRAIN, 64);
#endif

#ifdef __FreeBSD__
	setlocale(LC_ALL, "");
#endif

	setbuf(stderr, NULL);	 /* rarely necessary */

	build_valid_baud_string();

	Ftime(&starting_timeb);	 /* get startup time */
	xmtr_signals();			 /* catch xmtr signals */
	xmtr_pid = (CFG_PidType) getpid();

	/*
	 * get this off quick, cause we'll be busy for a little while on 286
	 */
	build_revision_string();
	sprintf(hello_str, "ecu %s", revstr);
#ifdef M_I286
	ff(se, "%s\n", hello_str);
#endif

	/*
	 * initialize 'hertz'/'hzmsec' and  Nap() implimentation
	 */
	init_Nap();

	/*
	 * init uucp euid processing
	 */
	uid = getuid();
	euid = geteuid();
	if (pw = getpwnam("uucp"))
		uid_uucp = pw->pw_uid;
	endpwent();

	/*
	 * if we are not root, nor uucp (unlikely) and we are euid uucp, then
	 * use Setuid() to access uucp owned lines
	 */
	setuid_uucp = (uid) && (uid_uucp) && (uid != euid) && (euid == uid_uucp);
	if ((uid != euid) && Setuid(uid))
	{
		ff(se, "\r\n");
		perror("Setuid(uid) failed");
		ff(se, "\r\n");
		exit(-1);
	}

	/*
	 * Initialize shared memory segment. This must be done before any uses
	 * are made of the Lermio variable see ecushm.c and ecu.h.
	 */
	shm_init();

	/*
	 * ecu has its own keyboard handler ... blessing and curse since it
	 * depends on little outside help but can be tedious to configure for
	 * a new host or even instance
	 */
	keyset_init();			 /* intialize keyset */
	keyset_read("default");

	/*
	 * make ~/.ecu if necessary ... but chmod 700 the directory even if it
	 * already existed (needs to be very early before lots of init)
	 */
	make_ecu_subdir();

	get_curr_dir(curr_dir, sizeof(curr_dir));
	cd_array_init();		 /* read %cd directory list */

	hdb_init();
	var_init();				 /* initialize procedure variables */
	poutput_init();			 /* initialize procedure output */

	icmd_prompt = esdalloc(64);
	set_default_escape_prompt();

	/*
	 * init line variables
	 */
	memset(shm->Lline, 0, sizeof(shm->Lline));
	shm->Liofd = -1;		 /* no line open now */
	shm->Lbitrate = CFG_DefaultBitRate;	/* from config.c run */
	shm->Lparity = CFG_DefaultParity;	/* from config.c run */
	if (shm->Lparity == 'n')
		shm->Lparity = 0;
	shm->Ltelno[0] = 0;		 /* no telephone number for remote yet */
	shm->Llogical[0] = 0;	 /* no logical name for remote yet */
	shm->Lrname[0] = 0;		 /* no logical name for remote yet */
	shm->Ldescr[0] = 0;		 /* no description for remote yet */
	shm->Lconnected = 0;	 /* not connected */
	shm->Ladd_nl_incoming = 0;	/* dont add nl to incoming cr */
	shm->Ladd_nl_outgoing = 0;	/* dont add nl to outgoing cr */
	shm->Lfull_duplex = 1;	 /* assume full duplex */
	shm->Lmodem_already_init = 0;	/* modem has not been initialized */
	shm->Lxonxoff = IXON | IXOFF;	/* default to xon/xoff protocol */
	shm->xmtr_pid = (CFG_PidType) getpid();
	shm->xmtr_ppid = (CFG_PidType) getppid();
#if defined(CFG_GetpgrpVoidArg)
	shm->xmtr_pgrp = (CFG_PidType) getpgrp();
#else
	shm->xmtr_pgrp = (CFG_PidType) getpgrp(0);
#endif
	strncpy(shm->tty_name, get_ttyname(), sizeof(shm->tty_name));
	shm->tty_name[sizeof(shm->tty_name) - 1] = 0;
	shm->shm_revision = SHM_REV;
	shm->ttyuse = TTYUSE_NORMAL;
	shm->terminating = 0;

	while ((swchar = getopt(argc, argv, "kCDF:HNP:Tb:defhl:no:p:tz")) != -1)
	{
		switch (swchar)
		{
			case 'k':
				keyboard_test_only();
				/* NOTREACHED */
			case 'b':
				if (valid_baud(shm->Lbitrate = atoi(optarg)) == -1)
				{
					ff(se, "invalid bit rate %u\n", shm->Lbitrate);
					usage();
				}
				break;
			case 'l':
				shm->Lline[0] = 0;
				if ((*optarg != '=') && (*optarg != '/'))
					strcpy(shm->Lline, "/dev/");
				strncat(shm->Lline, optarg,
					sizeof(shm->Lline) - strlen(shm->Lline) -
					strlen(optarg) - 1);
				shm->Lline[sizeof(shm->Lline) - 1] = 0;
				break;
			case 'p':
				strncpy(initial_procedure, optarg, sizeof(initial_procedure));
				initial_procedure[sizeof(initial_procedure) - 1] = 0;
				break;
			case 'h':
				shm->Lfull_duplex = 0;
				break;
			case 'f':
				shm->Lfull_duplex = 1;
				break;
			case 'd':
				quit_on_init_proc_fail = 1;
				break;
			case 'D':
				quit_on_init_proc_done = 1;
				break;
			case 't':
				shm->Ladd_nl_incoming = 1;
				shm->Ladd_nl_outgoing = 1;
				break;
			case 'e':
				shm->Lparity = 'e';
				break;
			case 'o':
				shm->Lparity = 'o';
				break;
			case 'N':
				shm->ttyuse = TTYUSE_FORCE_SIMPLE;
				break;
			case 'T':
				proc_trace++;
				break;
#if defined(CFG_TelnetOption)
			case 'R':
				shm->Ltelnet_raw = 1;
				break;
#endif
			case 'P':
				strncpy(phonedir_name, optarg, PHONEDIR_NAME_SIZE);
				phonedir_name[PHONEDIR_NAME_SIZE - 1] = 0;
				break;
			case 'C':
				show_config();
				termecu(0);
				break;
			case 'F':
				dash_f_funckeytype = optarg;
				break;
			case 'n':
				shm->rcvr_ansi_filter = 0;
				break;
			case 'z':
				shm->show_telnet_traffic = 1;
				break;
			case '?':
				usage();
		}
	}

	/*
	 * check a few options for validity
	 */
	if (!initial_procedure[0] &&
		(quit_on_init_proc_done || quit_on_init_proc_fail))
	{
		ff(se, "no -D/-d without -p\n");
		usage();
	}

	ttyinit(shm->ttyuse);	 /* init console tty mode handler */
	ttymode(3);				 /* put user console in `raw' mode but SIGINT
							  * terms prog */

	if (tty_not_char_special)
		quit_on_init_proc_done = 1;
	else
	{

		/*
		 * rattle curses once - fixes quirk/bug I can't find
		 */
		windows_start();
		windows_end(0);
		fflush(so);
		tcap_clear_screen();
	}

#if defined(WHT2) || defined(XTERM_FRIEND)

	/*
	 * if xterm, put notice in title bar but this really should be done in
	 * _rc.ep
	 */
	xterm_title("ECU", 0);
#endif

	/*
	 * do the _rc.ep execution
	 */
	if (find_procedure(_rc))
	{
		if (do_proc(1, &_rc))
		{
			if (quit_on_init_proc_fail || quit_on_init_proc_done)
				termecu(TERMECU_INIT_PROC_ERROR);
		}
	}
	rc_ep_has_run = 1;

	/*
	 * check out line
	 */
	if (!shm->Lline[0])
	{
		if (!there_is_hdb_on_this_machine)
			strcpy(shm->Lline, default_tty);
		else
		{
			DVE *tdve;

			if (tdve = hdb_choose_Any(shm->Lbitrate))
				sprintf(shm->Lline, "/dev/%s", tdve->line);
			else
				strcpy(shm->Lline, default_tty);
		}
	}
	else if (shm->Lline[0] == '=')	/* Devices lookup */
	{
		DVE *tdve = 0;
		char acutype[128];

		if (!there_is_hdb_on_this_machine)
		{
			ff(se, "\r\n\n");
			tcap_stand_out();
			fputs("[cannot use -l=Devices ... HDB not available]", se);
			tcap_stand_end();
			ff(se, "\r\n");
			usage();
		}
		strncpy(acutype, shm->Lline + 1, sizeof(acutype));
		acutype[sizeof(acutype) - 1] = 0;
		if (!(tdve = getdvtype(shm->Lline + 1)))
		{
			ff(se, "\r\n\n");
			tcap_stand_out();
			fprintf(se, "[no Devices line of type '%s']", acutype);
			tcap_stand_end();
			ff(se, "\r\n");
			errno = -1;
			termecu(TERMECU_SVC_NOT_AVAIL);
		}
		shm->Lline[0] = 0;
		shm->Lbitrate = tdve->high_baud;
		setdvent();			 /* rewind for assured passthru/enddvent by
							  * hdb_choose */
		if (!(tdve = hdb_choose_Device(acutype, shm->Lbitrate)))
		{
			ff(se, "\r\n\n");
			tcap_stand_out();
			fprintf(se, "[no idle Devices line of type '%s' at %u baud]",
				acutype, shm->Lbitrate);
			tcap_stand_end();
			ff(se, "\r\n");
			errno = -1;
			termecu(TERMECU_SVC_NOT_AVAIL);
		}
		strcpy(shm->Lline, "/dev/");
		strncat(shm->Lline, tdve->line, sizeof(shm->Lline) - strlen(shm->Lline));
		shm->Lline[sizeof(shm->Lline) - 1] = 0;
	}

	report_initial_line();	 /* in #ifdef'ed debug event logging context */

	/*
	 * either present startup screen or run initial procedure or both
	 */
	if (initial_procedure[0])
	{
		init_proc_argv[0] = initial_procedure;
		init_proc_argc = 1;
		for (itmp = optind; itmp < argc; itmp++)
		{
			if (*argv[itmp] != '-')
			{
				if (init_proc_argc == MAX_PARGV)
				{
					ff(se, "too many arguments to initial procedure\r\n");
					errno = -1;
					termecu(TERMECU_USAGE);
				}
				init_proc_argv[init_proc_argc++] = argv[itmp];
			}
		}

		if (do_proc(init_proc_argc, init_proc_argv))
		{
			if (quit_on_init_proc_fail || quit_on_init_proc_done)
				termecu(TERMECU_INIT_PROC_ERROR);
		}
		proc_file_reset();
		colors_save = colors_current;
		setcolor(colors_notify);
		pputs("[procedure finished]");
		setcolor(colors_save);
		pputs("\n");
		if (quit_on_init_proc_done)
		{
			errno = -1;
			termecu(TERMECU_OK);
		}
		if (shm->Liofd < 0)
		{
			ff(se, "\r\n\n");
			tcap_stand_out();
			fputs("[no line attached by initial procedure]", se);
			tcap_stand_end();
			ff(se, "\r\n");
			if (quit_on_init_proc_fail)
				termecu(TERMECU_INIT_PROC_ERROR);
			tcap_stand_out();
			fputs("[press ESC to exit or SPACE for setup menu]", se);
			tcap_stand_end();
			itmp = ttygetc(0);
			ff(se, "\r\n");
			if (itmp == ESC)
				termecu(TERMECU_OK);
			shm->Llogical[0] = 0;
			shm->Ltelno[0] = 0;
			shm->Ldescr[0] = 0;
			setup_screen((char *)0);
		}
		else
			start_rcvr_process(0);
	}
	else
		/* no initial procedure */
	{
		if (!tty_not_char_special)
			setup_screen((optind < argc) ? argv[optind] : (char *)0);
		else
		{
			errno = -1;
			pprintf("Cannot run ecu from non-tty without initial procedure.\n");
			termecu(TERMECU_UNRECOVERABLE);
		}
	}

	/* enter xmtr operation */
	xmtr();
	termecu(TERMECU_OK);
	return (0);				 /* never get here, but keep gcc optim from
							  * complaining */
	/* NOTREACHED */
}							 /* end of main */

/* end of ecu.c */
/* vi: set tabstop=4 shiftwidth=4: */
