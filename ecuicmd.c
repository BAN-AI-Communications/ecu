/*+-----------------------------------------------------------------------
	ecuicmd.c - ECU interactive command handler
	wht@wht.net

  Defined functions:
	icmd(icmd_cmd)
	icmd_do_proc(ndoarg, doarg)
	search_cmd_list(cmd_list, cmd)

  '...  With this equipment harnessed to the back, a watertight
  glass mask over the eyes and nose, and rubber foot fins, we
  intended to make unencumbered flights in the depths of the
  sea.' -- Jacques-Yves Cousteau SILENT WORLD

------------------------------------------------------------------------*/
/*+:EDITS:*/
/*:04-26-2000-11:15-wht@bob-RELEASE 4.42 */
/*:01-24-1997-02:37-wht@yuriatin-SOURCE RELEASE 4.00 */
/*:10-20-1996-19:48-wht@yuriatin-bad test in baud cmd */
/*:10-19-1996-11:58-wht@yuriatin-tommfoolery with stat */
/*:10-18-1996-01:26-wht@yuriatin-clean up, verify, harden */
/*:09-29-1996-14:59-wht@yuriatin-break during telnet gets telnet_xmit_IP */
/*:09-18-1996-06:25-wht@yuriatin-CTayt Are You There added */
/*:09-18-1996-00:35-wht@yuriatin-add CR to CTredial "no line attached" */
/*:09-13-1996-09:30-wht@kepler-allow duplex control during telnet */
/*:09-11-1996-20:00-wht@yuriatin-3.48-major telnet,curses,structural overhaul */
/*:09-09-1996-04:04-wht@yuriatin-add shm->Lipaddr_str */
/*:09-06-1996-02:23-wht@kepler-cleanup */
/*:08-21-1996-16:55-wht@fep-add show_config to rev command */
/*:08-20-1996-12:39-wht@kepler-locale/ctype fixes from ache@nagual.ru */
/*:11-27-1995-12:06-wht@kepler-re-add lost xlog command */
/*:11-23-1995-11:20-wht@kepler-source control 3.37 for tsx-11 */
/*:11-14-1995-23:30-wht@kepler-stat when no Liofd shows NONE */
/*:11-14-1995-10:23-wht@kepler-3.37.80-source control point: SOCKETS */
/*:11-12-1995-01:39-wht@gyro-stat cmd output more appropriate for telnet */
/*:11-12-1995-00:47-wht@gyro-add telopt and ansif */
/*:11-04-1995-13:42-wht@wwtp1-ignore some commands if telnet */
/*:11-03-1995-16:53-wht@wwtp1-use CFG_TelnetOption */
/*:10-20-1995-18:54-wht@wwtp1-stat cmd output cleanup */
/*:10-19-1995-01:14-wht@kepler-Ltelnet xon intercept */
/*:10-14-1995-23:57-wht@kepler-use valid_baud_string */
/*:10-14-1995-23:22-wht@kepler-drop SEAlink support */
/*:09-26-1995-12:21-wht@kepler-mkdir ack echoed '(null)' for dir name */
/*:09-16-1995-16:32-root@kepler-add td - tcap display command */
/*:08-27-1995-06:24-wht@n4hgf-use shm->Lrtscts_val */
/*:06-13-1995-20:52-wht@n4hgf-add max screen geometry to stat cmd */
/*:05-09-1995-17:23-wht@kepler-add attrtest */
/*:04-02-1995-04:48-wht@n4hgf-added sgrto1 and sgrto2 */
/*:03-29-1995-01:24-wht@n4hgf-TIOCMGET now self-recognizing */
/*:03-29-1995-01:24-wht@n4hgf-doc and hardening */
/*:03-21-1995-19:56-wht@n4hgf-reset sigint+proc_interrupt on HOME or do cmd */
/*:03-21-1995-15:53-wht@n4hgf-add erto and erverbose */
/*:03-12-1995-03:27-wht@kepler-use ECU_MAXPN */
/*:01-12-1995-15:19-wht@n4hgf-apply Andrew Chernov 8-bit clean+FreeBSD patch */
/*:05-04-1994-04:38-wht@n4hgf-ECU release 3.30 */
/*:08-07-1993-11:03-wht@n4hgf-resurrect mkdir cmd */
/*:12-16-1992-19:03-wht@n4hgf-fix dial error help suggestion */
/*:10-18-1992-14:26-wht@n4hgf-add conxon */
/*:10-08-1992-01:12-wht@n4hgf-no more obsolete Metro Link PTS */
/*:09-17-1992-06:27-wht@n4hgf-add <7-bit kbd> to stat */
/*:09-13-1992-12:52-wht@n4hgf-show tty_is_scoterm during stat */
/*:09-10-1992-13:58-wht@n4hgf-ECU release 3.20 */
/*:08-30-1992-23:15-wht@n4hgf-add fkmap */
/*:08-22-1992-15:38-wht@n4hgf-ECU release 3.20 BETA */
/*:04-17-1992-18:22-wht@n4hgf-fkey command has -r reset switch */
/*:12-13-1991-17:14-wht@n4hgf-formalize bell notify */
/*:11-30-1991-13:46-wht@n4hgf-smap conditional compilation reorg */
/*:10-04-1991-17:23-wht@n4hgf-reset proc_interrupt before interactive pcmd */
/*:08-25-1991-14:39-wht@n4hgf-SVR4 port thanks to aega84!lh */
/*:08-17-1991-18:29-wht@n4hgf-add kbdtest command */
/*:07-29-1991-20:56-wht@n4hgf-turn off memstat after frustrating evening */
/*:07-29-1991-17:57-wht@n4hgf-add memstat */
/*:07-25-1991-12:56-wht@n4hgf-ECU release 3.10 */
/*:05-21-1991-18:22-wht@n4hgf-add pushd/popd commands */
/*:05-21-1991-00:46-wht@n4hgf-added -3 error code to keyset_read */
/*:03-20-1991-05:25-root@n4hgf-experimental eto command */
/*:03-20-1991-04:55-root@n4hgf-Metro Link support + stat cmd changes */
/*:02-04-1991-19:03-wht@n4hgf-add multiscreen tag to stat */
/*:01-09-1991-22:31-wht@n4hgf-ISC port */
/*:12-24-1990-04:31-wht@n4hgf-experimental fasi driver command */
/*:08-14-1990-20:40-wht@n4hgf-ecu3.00-flush old edit history */

#include "ecu.h"
#include "esd.h"
#include "ecufkey.h"

#define NEED_P_CMD
#include "ecucmd.h"

#if defined(FASI)
#include <local/fas.h>
#endif

#if defined(SVR4)
#include <sys/termiox.h>
extern int hx_flag;

#endif

char *bell_notify_text();

extern char kbd_is_7bit;	 /* keyboard has parity */
extern KDE keyset_table[];
extern char *makedate;
extern char curr_dir[ECU_MAXPN];
extern char hello_str[];
extern char keyset_name[];
extern char rcvr_log_file[]; /* if rcvr_log!= 0,log filename */
extern int current_ttymode;
extern long tty_escape_timeout;	/* timeout on waiting for char after ESC */
extern UINT tcap_COLS;
extern UINT tcap_LINES;
extern int errno;
extern int proc_interrupt;
extern int rcvr_log;		 /* rcvr log active if != 0 */
extern int rcvr_log_append;	 /* if true, append, else scratch */
extern int rcvr_log_raw;	 /* if true, log all, else filter ctl chrs */
extern UINT32 colors_current;
extern FILE *plog_fp;
extern ESD *plog_name;
extern char screen_dump_file_name[];
extern int expresp_verbosity;
extern UINT32 expect_timeout_msecs;

int protocol_log_packets = 0;

/*+-------------------------------------------------------------------------
	search_cmd_list(cmd_list,cmd)
returns -1 if cmd not found or insufficient chars for match
else token value for command
proc-only commands have 'min_ch' values 0
--------------------------------------------------------------------------*/
search_cmd_list(cmd_list, cmd)
P_CMD *cmd_list;
char *cmd;
{
	while (cmd_list->token != -1)
	{
		if (cmd_list->min_ch && minunique(cmd_list->cmd, cmd, cmd_list->min_ch))
			return (cmd_list->token);
		cmd_list++;
	}
	return (-1);

}							 /* end of search_cmd_list */

/*+-------------------------------------------------------------------------
	icmd_do_proc(ndoarg,doarg)
--------------------------------------------------------------------------*/
icmd_do_proc(ndoarg, doarg)
int ndoarg;
char **doarg;
{
	int erc;
	UINT32 colors_at_entry = colors_current;

	kill_rcvr_process(SIGUSR1);
	ttymode(2);
	erc = do_proc(ndoarg, doarg);
	proc_file_reset();
	ttymode(1);
	sigint = 0;
	proc_interrupt = 0;
	setcolor(colors_notify);
	ff(se, "[procedure finished]");
	setcolor(colors_at_entry);
	ff(se, "\r\n");
	start_rcvr_process(0);
	return (erc);
}							 /* end of icmd_do_proc */

/*+-----------------------------------------------------------------------
	icmd(cmd)
 This function implements the built in commands
 It returns non-zero if program should terminate
------------------------------------------------------------------------*/
int
icmd(icmd_cmd)
char *icmd_cmd;
{
#define ARG_MAX_QUAN 40
	char *arg[ARG_MAX_QUAN];
	char *cp = "???";
	char cmd[128];
	int itmp, itmp2, itmp3;
	int token;
	int narg = 0;
	ESD *tesd;
	char s256[256];

#ifdef CFG_TelnetOption
	char *not_telnet_msg = "  not applicable to telnet\r\n";

#endif /* CFG_TelnetOption */
	char *epoch_secs_to_str();
	long atol();
	char *xon_status();
	char *console_xon_status();

	/*
	 * reset ^C hard and procedure interrupt flags if only because
	 * interactive 'pc' cmd may bhe used
	 */
	sigint = 0;
	proc_interrupt = 0;

	/*
	 * process command, looking for unusual ones first
	 */
	icmd_history_add(icmd_cmd);	/* add to history list */
	strcpy(cmd, icmd_cmd);	 /* get local copy of cmd string */
	switch (cmd[0])
	{
		case '.':
			strcpy(cmd, "exit");
			break;
		case '!':
		case '$':
		case '>':
			ff(se, "\r\n");
			shell(cmd);
			return (0);
		case '-':
			ff(se, "\r\n");
			exec_cmd(&cmd[1]);
			return (0);
		case '^':
			ff(se, "\r\n");
			phrase_help();
			return (0);
		case '?':
			icmd_help(0, (char **)0);
			return (0);
		default:
			break;
	}

	/*
	 * not single character argument
	 */
	build_arg_array(cmd, arg, ARG_MAX_QUAN, &narg);

	/*
	 * handle phrases
	 */
	if (isdigit((uchar) * arg[0]))
	{
		phrases(narg, arg);
		return (0);
	}

	/*
	 * search command list
	 */
	if ((token = search_cmd_list(icmd_cmds, arg[0])) < 0)
	{
		ff(se, "\r\n");
		if (find_procedure(arg[0]))
		{
			icmd_do_proc(narg, arg);
			return (0);
		}
		ff(se, "no such command or procedure ... HOME ? for help\r\n");
		return (0);
	}

	switch (token)
	{						 /* keep alphabetized PLEASE */
		case CTrx:
		case CTry:
		case CTrz:
		case CTrk:
			receive_files_from_remote(narg, arg);
			break;

		case CTsx:
		case CTsy:
		case CTsz:
		case CTsk:
			send_files_to_remote(narg, arg);
			break;

		case CTansif:
#if defined(CFG_NoAnsiEmulation)
			ff(se, "  rcvr ANSI filter not configured\r\n");
#else
			if (narg > 1)
			{
				itmp = yes_or_no(arg[1]);
				ff(se, "   ");
				rcvr_ansi_filter_control(itmp);
			}
			else
			{
				ff(se, "  rcvr ANSI filter is %s\r\n",
					shm->rcvr_ansi_filter ? "on" : "off");
			}
#endif
			break;

		case CTattrtest:
			ff(se, "\r\n");
			console_attribute_test();
			break;

		case CTautorz:
			if (narg > 1)
			{
				shm->autorz = yes_or_no(arg[1]);
				shm->autorz_pos = 0;
			}
			ff(se, "  automatic ZMODEM receive is %s",
				shm->autorz ? "on" : "off");
			ff(se, "\r\n");
			break;

		case CTbaud:
			if (narg == 1)
				ff(se, "   bit rate is %u\r\n", shm->Lbitrate);
			else
			{
				itmp = atoi(arg[1]);
				if (!lnew_bitrate(itmp))
					ff(se, "   bit rate set to %u\r\n", shm->Lbitrate);
				else
				{
					extern char *valid_baud_string;

					ff(se, "   invalid baud: %u\r\n", itmp);
					ff(se, "valid rates: %s\r\n", valid_baud_string);
				}
			}
#if defined(CFG_TelnetOption)
			if (shm->Ltelnet)
				ff(se, "\
though bit rate on telnet connections means little.\r\n");
#endif /* defined(CFG_TelnetOption) */
			break;

		case CTbreak:
#ifdef CFG_TelnetOption
			if (shm->Ltelnet)
			{
				telnet_xmit_IP();
				break;
			}
#endif /* CFG_TelnetOption */
			lbreak();
			ff(se, "   break sent\r\n");
			break;

		case CTcd:
			(void)change_directory(arg[1], (narg == 1) ? 0 : 1);
			break;

		case CTpushd:
			(void)push_directory(arg[1], (narg == 1) ? 0 : 1, 0);
			break;

		case CTpopd:
			(void)pop_directory(arg[1], (narg == 1) ? 0 : 1, 0);
			break;

		case CTclrx:
#ifdef CFG_TelnetOption
			if (shm->Ltelnet)
			{
				ff(se, not_telnet_msg);
				break;
			}
#endif /* CFG_TelnetOption */
			lclear_xmtr_xoff();
			ff(se, "  output xoff cleared\r\n");
			break;

		case CTdial:
			if (narg < 2)
				phdir_manager();
			else
			{
				ff(se, "\r\n");
				call_logical_telno(arg[1]);
			}
			break;

		case CTdo:
			ff(se, "\r\n");
			icmd_do_proc(narg - 1, &arg[1]);
			break;

		case CTptrace:
			if (narg > 1)
				proc_trace = yes_or_no(arg[1]);
			ff(se, "  procedure trace %s", proc_trace ? "on" : "off");
			if (proc_trace > 1)
				ff(se, " (%d)", proc_trace);
			ff(se, "\r\n");
			break;

		case CTpcmd:
			itmp = strlen(arg[0]);
			if (!(tesd = esdalloc(ESD_NOMSZ)))
			{
				ff(se, "  no memory?!\r\n");
				break;
			}
			strcpy(tesd->pb, icmd_cmd + itmp + 1);
			tesd->cb = strlen(tesd->pb);
			ff(se, "\r\n");
			kill_rcvr_process(SIGUSR1);
			ttymode(2);
			proc_interrupt = 0;	/* in case last proc was sigint-ed */
			if (itmp = execute_esd(tesd))
			{
				proc_error(itmp);
				esdshow(tesd, "");
			}
			esdfree(tesd);
			ttymode(1);
			start_rcvr_process(0);
			break;

		case CTplog:
			fputs("  ", se);
			if (narg > 1)
			{
				if (!strcmp(arg[1], "off"))
					plog_control((char *)0);
				else
					plog_control(arg[1]);
			}

			if (plog_fp)
				ff(se, "procedure logging: %s\r\n", plog_name->pb);
			else
				fputs("procedure logging off\r\n", se);
			break;

		case CTduplex:
			if (narg > 1)
			{
				switch (to_lower(*arg[1]))
				{
					case 'f':
						shm->Lfull_duplex = 1;
						ff(se, "  now ");
						break;
					case 'h':
						shm->Lfull_duplex = 0;
						ff(se, "  now ");
						break;
					default:
						ff(se,
							"\r\nfirst argument character must be F or H\r\n");
						break;
				}
			}
			else			 /* no argument */
				ff(se, "  currently ");

			ff(se, "%s duplex\r\n", (shm->Lfull_duplex) ? "full" : "half");
			break;

		case CTerto:
			if (narg > 1)
			{
				itmp = atoi(arg[1]);
				if (itmp < 0)
					ff(se, " must be >=0 ... keeping value of");
				else
				{
					expect_timeout_msecs = (UINT32) itmp;
					ff(se, "   set to");
				}
			}
			ff(se, " %ld msec\r\n", expect_timeout_msecs);
			break;

		case CTerverbose:
			if (narg > 1)
			{
				expresp_verbosity = yes_or_no(arg[1]);
				ff(se, "  set to");
			}
			switch (expresp_verbosity)
			{
				case 0:
					ff(se, " no\r\n");
					break;
				case 1:
					ff(se, " yes\r\n");
					break;
				default:
					ff(se, " extra\r\n");
					break;
			}
			break;

		case CTexit:
			ff(se, "  disconnecting from line %s\r\n", shm->Lline);
			return (1);

		case CTfi:
			file_insert_to_line(narg, arg);
			break;

		case CThangup:
			ff(se, "  hanging up ...\r\n");
			DCE_hangup();
#if defined(FASI)
			{
				uchar msr;

				if (!shm->Ltelnet)
				{
					msr = fasi_msr();
					ff(se, "hangup complete ... DCD is %s\r\n",
						(msr & MS_DCD_PRESENT) ? "STILL HIGH" : "low");
				}
				else
					ff(se, "hangup complete\r\n");
			}
#else
			ff(se, "hangup complete\r\n");
#endif
			break;

		case CThelp:
			icmd_help(narg, arg);
			break;

		case CTsdname:
			if (narg > 1)
			{
				char *new_file_name;

				itmp = 0;	 /* do not need to free(new_file_name) */
				if (find_shell_chars(arg[1]))
				{

					if (expand_wildcard_list(arg[1], &new_file_name))
					{
						ff(se, "  %s\r\n", new_file_name);
						break;
					}
					itmp = 1;/* need to free(new_file_name) */
				}
				else
					new_file_name = arg[1];

				screen_dump_file_name[0] = 0;
				if (*new_file_name != '/')
				{
					strcpy(screen_dump_file_name, curr_dir);
					strcat(screen_dump_file_name, "/");
				}
				strcat(screen_dump_file_name, arg[1]);
				if (itmp)
					free(new_file_name);
			}
			ff(se, "\r\nscreen dump name: %s\r\n", screen_dump_file_name);
			break;

		case CTlog:
		case CTloff:
		case CTllp:
			icmd_log(token, narg, arg);
			break;

		case CTnl:
		case CTnlin:
		case CTnlout:
#ifdef CFG_TelnetOption
			if (shm->Ltelnet)
			{
				ff(se, not_telnet_msg);
				break;
			}
#endif /* CFG_TelnetOption */
			nlin_nlout_control(token, narg, arg);
			break;

		case CTparity:
#ifdef CFG_TelnetOption
			if (shm->Ltelnet)
			{
				ff(se, not_telnet_msg);
				break;
			}
#endif /* CFG_TelnetOption */
			if (narg > 1)
			{
				switch (to_lower(*arg[1]))
				{
					case 'n':
						shm->Lparity = 0;
						break;
					case 'e':
					case 'o':
						shm->Lparity = to_lower(*arg[1]);
						break;
					default:
						ff(se, "   unrecognized parity: %c\r\n", *arg[1]);
						break;
				}
				lset_parity(1);
			}
			ff(se, "   parity set to %c\r\n",
				(shm->Lparity) ? to_upper(shm->Lparity) : 'N');
			break;

		case CTpid:
			if (shm->rcvr_pid > 0)
				sprintf(s256, "%d", shm->rcvr_pid);
			else
				strcpy(s256, "NONE");
			ff(se, ": xmtr=%d rcvr=%s parent=%d pgrp=%d\r\n",
				xmtr_pid, s256, shm->xmtr_ppid, shm->xmtr_pgrp);
			break;

		case CTpwd:
			ff(se, " %s\r\n", curr_dir);
			break;

		case CTredial:
			if (shm->Liofd < 0)
			{
				ff(se, "  no line attached\r\n");
				break;
			}
#ifdef CFG_TelnetOption
			if (shm->Ltelnet)
			{
				ff(se, not_telnet_msg);
				break;
			}
#endif /* CFG_TelnetOption */
			(void)DCE_redial(arg, narg);
			break;

		case CTrev:
			fputs("\r\n", se);
			fputs(hello_str, se);
			fputs("\r\n", se);
			ff(se, "%s\r\n", makedate);
#if defined(MEMCHECK)
			_dump_malloc();
#endif
			show_config();
			break;

		case CTtime:
			timeofday_text(4, cmd);
			ff(se, ": %s\r\n", cmd);
			break;

		case CTdcdwatch:
#ifdef CFG_TelnetOption
			if (shm->Ltelnet)
			{
				ff(se, not_telnet_msg);
				break;
			}
#endif /* CFG_TelnetOption */
			if (narg > 1)
			{
				if (ldcdwatch_str(arg[1]))
					ff(se, " ... argument error; remains set to ");
				else
					ff(se, " ... set to ");
			}
			else
				ff(se, " is ");
			cp = "???";
			switch (shm->Ldcdwatch)
			{
				case DCDW_OFF:
					cp = "off";
					break;
				case DCDW_ON:
					cp = "on";
					break;
				case DCDW_TERMINATE:
					cp = "TERMINATE";
					break;
			}
			ff(se, "%s\r\n", cp);
			break;

		case CTtd:
			ff(se, "\r\n");
			tcap_display();
			break;

		case CTts:
			ff(se, "\r\n");
			sprintf(s256, "TTYIN %s (ttymode=%d)",
				get_ttyname(), current_ttymode);
			disp_line_termio(TTYIN, s256);
			ff(se, "\r\n");
#ifdef CFG_TelnetOption
			if (!strcmp(shm->Lline, "/dev/telnet"))
			{
				ff(se, "Communications line is a socket to %s\r\n",
					shm->Lipaddr_str);
			}
			else
#endif
			{
				sprintf(s256, "comm line %s", shm->Lline);
				disp_line_termio(shm->Liofd, s256);
			}
#ifdef LINUX_ASYNC_HACK		 /* see ldserial.c and ecutermio.h */
			ff(se, "ACTUAL bitrate = %d\r\n", shm->Lbitrate);
#endif /* LINUX_ASYNC_HACK */
#if defined(TIOCMGET)		 /* Linux for one */
			if ((ioctl(shm->Liofd, TIOCMGET, (char *)&itmp)) != -1)
			{
				ff(se, "Current modem signals:   RTS:%c   CTS:%c   DCD:%c\r\n",
					(itmp & TIOCM_RTS) ? '1' : '0',
					(itmp & TIOCM_CTS) ? '1' : '0',
					(itmp & TIOCM_CD) ? '1' : '0');
			}
#endif
			break;

#if	defined(FASI)
		case CTfasi:
#ifdef CFG_TelnetOption
			if (shm->Ltelnet)
			{
				ff(se, not_telnet_msg);
				break;
			}
#endif /* CFG_TelnetOption */
			icmd_fasi(narg, arg);
			break;
#endif /* FASI */
		case CTtty:
			ff(se, "   %s\r\n", get_ttyname());
			break;

		case CTda:
		case CToa:
		case CTxa:
		case CTax:
			icmd_conversion(token, narg, arg);
			break;

		case CTbn:
			if (narg < 2)
				ff(se, "  is ");
			else
			{
				if ((itmp = parse_bell_notify_argument(arg[1])) < 0)
					ff(se, "\r\nbad argument (try help bn); remains set to ");
				else
				{
					shm->bell_notify_state = itmp;
					ff(se, "  set to ");
				}
			}
			ff(se, "%s\r\n", bell_notify_text(shm->bell_notify_state));
			break;

		case CTrtscts:
#ifdef CFG_TelnetOption
			if (shm->Ltelnet)
			{
				ff(se, not_telnet_msg);
				break;
			}
#endif /* CFG_TelnetOption */
#if defined(HW_FLOW_CONTROL) /* see ecu.h */
			if (narg > 1)
			{
				lRTSCTS_control(shm->Lrtscts_val = yes_or_no(arg[1]));
				pputs("\nNew c");
			}
			else
				pputs("\nC");
			pputs("onfiguration:  ");
			display_hw_flow_config();
#else /* !HW_FLOW_CONTROL */
			ff(se, "\r\nhardware flow control not available\r\n");
#endif /* HW_FLOW_CONTROL */
			break;

		case CTeto:
			if (narg > 1)
			{
				itmp = atoi(arg[1]);
				if ((itmp < 20) || (itmp > 1000))
				{
					ff(se, " invalid\r\n");
				}
				tty_escape_timeout = (long)itmp;
				ff(se, " set to");
			}
			ff(se, " %ld msec\r\n", tty_escape_timeout);
			break;

		case CTnice:
			itmp = nice(0) + 20;
			if (narg > 1)
			{
				kill_rcvr_process(SIGUSR1);
				itmp2 = atoi(arg[1]);
				itmp3 = nice(-itmp + itmp2);
				ff(se, " -> desired=%d, was %d, ", itmp2, itmp);
				if (itmp3 < 0)
					ff(se, "nice failed: %s\r\n", strerror(errno));
				else
					ff(se, "set to %d\r\n", itmp3);
				start_rcvr_process(0);
			}
			else
				ff(se, " is %d\r\n", itmp);
			break;

		case CTfkey:
			if (narg < 2)
				keyset_display();
			else if (!strcmp(arg[1], "-r"))
			{
				keyset_init();
				keyset_read("default");
				ff(se, "  keyboard reset done ...\r\n");
			}
			else
			{
				switch (keyset_read(arg[1]))
				{
					case 0:
						keyset_display();
						break;
					case -1:
						ff(se, " cannot find ~/.ecu/keys\r\n");
						break;
					case -2:
						ff(se, " not found in ~/.ecu/keys\r\n");
						break;
					case -3:
						ff(se, " syntax error ... default restored\r\n");
					default:
						keyset_init();
						keyset_read("default");
						break;
				}
			}
			break;

		case CTfkmap:
			ff(se, "\r\n");
			fkmap_command(narg, arg);
			break;

		case CTstat:
			timeofday_text(4, s256);
			ff(se, "\r\nDate: %s\r\n", s256);

			if (!shm->Lconnected)
				ff(se, "Not connected to a remote\r\n");
			else
			{
				long now;

				time(&now);
				ff(se, "Connected to %s: %s (%s)\r\n",
					shm->Lrname, shm->Ldescr, shm->Ltelno);
				ff(se, "          since %s (elapsed %s)\r\n",
					epoch_secs_to_str(shm->Loff_hook_time, 1, s256),
					elapsed_time_text(now - shm->Loff_hook_time));
			}

			if (shm->Liofd < 0)
				ff(se,"No line or socket attached\r\n");
#ifdef CFG_TelnetOption
			else if (shm->Ltelnet)
			{
				ff(se, "IP address = %s  network TERM=`%s'\r\n",
					shm->Lipaddr_str, shm->telnet_ttype);
			}
#endif /* CFG_TelnetOption */
			else
			{
				ff(se, "Asynchronous line: %s %u-%c-1  %s duplex\r\n",
					(shm->Liofd < 0) ? "NONE" : shm->Lline, shm->Lbitrate,
					(shm->Lparity) ? to_upper(shm->Lparity) : 'N',
					(shm->Lfull_duplex) ? "full" : "half");
			}
			ff(se, "Current directory: %s\r\n", curr_dir);

			if (!shm->Ltelnet)
			{
				lget_xon_xoff(&itmp2, &itmp3);
				ff(se, "Line XON/XOFF: input %s, output %s\r\n",
					(itmp2) ? "on" : "off",	    /* IXON */
					(itmp3) ? "on" : "off");	/* IXOFF */
#if defined(HW_FLOW_CONTROL) /* see ecu.h */
				ff(se, "Hardware flow control configuration: ");
				display_hw_flow_config();
#else
				ff(se, "no hardware flow control available\r\n");
#endif
				cp = "???";
				switch (shm->Ldcdwatch)
				{
					case DCDW_OFF:
						cp = "off";
						break;
					case DCDW_ON:
						cp = "on";
						break;
					case DCDW_TERMINATE:
						cp = "TERMINATE";
						break;
				}
				ff(se, "DCD watcher: %s\r\n", cp);
			}

			ff(se, "Console: %s %dx%d ",
				get_ttyname(), tcap_COLS, tcap_LINES);
			if (tty_is_multiscreen)
				ff(se, "<mscreen> ");
			if (tty_is_scoterm)
				ff(se, "<scoterm> ");
			if (tty_is_pty)
				ff(se, "<pty> ");
			if (kbd_is_7bit)
				ff(se, "<7-bit kbd> ");
			ff(se, "\r\n         ");
			fputs(console_xon_status(), se);
			ff(se, "\r\n");

#if defined(CFG_UseUngetty)
			display_ungetty_list();
#endif
			ff(se, "Total chars transmitted: %ld", shm->xmit_chars);
			if (shm->Lconnected)
				ff(se, " (since CONNECT %ld)\r\n",
					shm->xmit_chars_this_connect);
			else
				fputs("\r\n", se);
			ff(se, "Total chars received:    %ld", shm->rcvd_chars);
			if (shm->Lconnected)
			{
				ff(se, " (since CONNECT %ld)\r\n",
					shm->rcvd_chars_this_connect);
			}
			else
				fputs("\r\n", se);

			ff(se,"ANSI filter is %s;  ",
				(shm->rcvr_ansi_filter) ? "ON" : "off");

			if (keyset_name[0])
				ff(se, "Function key set '%s' loaded\r\n", keyset_name);
			else
				ff(se, "No function key set loaded\r\n");

			if (rcvr_log)
			{
				ff(se, "Session log open: %s (%s mode)\r\n",
					rcvr_log_file, (rcvr_log_raw) ? "raw" : "filtered");
			}
			else
				ff(se, "Session logging not active\r\n");

			ff(se, "Bell notify is %s\r\n",
				bell_notify_text(shm->bell_notify_state));

			ff(se, "CR conversion:  incoming %s  outgoing %s\r\n",
				(shm->Ladd_nl_incoming) ? "CR/LF" : "CR",
				(shm->Ladd_nl_outgoing) ? "CR/LF" : "CR");

			ff(se, "Keyboard ESC/funckey time constant = %ld msec\r\n",
				tty_escape_timeout);

			if (shm->rcvr_pid > 0)
				sprintf(s256, "%d", shm->rcvr_pid);
			else
				strcpy(s256, "NONE");
			ff(se, "Pids: xmtr=%d rcvr=%s parent=%d pgrp=%d\r\n",
				xmtr_pid, s256, shm->xmtr_ppid, shm->xmtr_pgrp);
			break;

		case CTxon:
#ifdef CFG_TelnetOption
			if (shm->Ltelnet)
			{
				ff(se, not_telnet_msg);
				break;
			}
#endif /* CFG_TelnetOption */
			if (narg > 1)
			{
				if (set_xon_xoff_by_arg(arg[1]))
					ff(se, "  argument error\r\n");
				else
					ff(se, "\r\n");
				break;
			}
			ff(se, "  xon/xoff flow control: %s\r\n", xon_status());
			break;

		case CTconxon:
			if (narg > 1)
			{
				if (set_console_xon_xoff_by_arg(arg[1]))
					ff(se, "  argument error\r\n");
				else
					ff(se, "\r\n");
				break;
			}
			ff(se, "\r\nconsole xon/xoff flow control: %s\r\n", console_xon_status());
			break;

		case CTsgr:
		case CTsgrto1:
		case CTsgrto2:
			send_get_response(narg, arg);
			break;

		case CTmkdir:
			if (narg < 2)
			{
				ff(se, "  need a name to mkdir\r\n");
				break;
			}
			ff(se, "  ");
			if (itmp = !strcmp(arg[1], "-a"))
			{
				itmp = mkdir_auto(cp = arg[2]);
				if (itmp < 0)
					pperror(cp);
				else
					ff(se, "created %d levels to make %s\r\n", itmp, cp);
			}
			else if (!mkdir(cp = arg[1], 0755))
				ff(se, "  made directory %s\r\n", cp);
			else
				pperror(cp);
			break;

		case CTmemstat:
#if defined(CFG_Malloc3X)
			{
				struct mallinfo minfo;
				extern char *startbrk;
				extern char *startsp;

				minfo = mallinfo();
				pputs("\n"); /* all this casting for 16- vs 32- bit ints */
				pprintf("%10lu total space in arena\n", (UINT32) minfo.arena);
				pprintf("%10lu number of ordinary blocks\n",
					(UINT32) minfo.ordblks);
				pprintf("%10lu number of small blocks\n",
					(UINT32) minfo.smblks);
				pprintf("%10lu number of holding blocks\n",
					(UINT32) minfo.hblks);
				pprintf("%10lu space in holding block headers\n",
					(UINT32) minfo.hblkhd);
				pprintf("%10lu space in small blocks in use\n",
					(UINT32) minfo.usmblks);
				pprintf("%10lu space in free small blocks\n",
					(UINT32) minfo.fsmblks);
				pprintf("%10lu space in ordinary blocks in use\n",
					(UINT32) minfo.uordblks);
				pprintf("%10lu space in free ordinary blocks\n",
					(UINT32) minfo.fordblks);
				pprintf("%10lu cost of enabling keep option\n",
					(UINT32) minfo.keepcost);
				pprintf("  %08lx startbrk\n", startbrk);
				pprintf("  %08lx sbrk(0)  ", sbrk(0));
				pprintf("  (break delta %10ld)\n",
					(char *)sbrk(0) - (char *)startbrk);
				pprintf("  %08lx startsp\n", startsp);
				pprintf("  %08lx sp       ", &minfo);
				pprintf("  (stack size  %10ld)\n",
					(long)(startsp - (char *)&minfo));
			}
#else
			ff(se, "  not available\r\n");
#endif
			break;

		case CTkbdtest:
			ff(se, "\r\n");
			kbd_test();
			break;

		case CTtelopt:
#ifdef CFG_TelnetOption
			if (narg > 1)
			{
				shm->show_telnet_traffic = yes_or_no(arg[1]);
			}
			ff(se, "\r\ndisplay of telnet option traffic is %s\r\n",
				shm->show_telnet_traffic ? "on" : "off");
			if (!shm->Ltelnet)
				ff(se, " ... though you are not in a telnet connection\r\n");
#else
			goto TELNET_NOT_AVAIL; 	/* save a string */
#endif
			break;

		case CTayt:
#ifdef CFG_TelnetOption
			ff(se, "    sending Are You There?\r\n");
			telnet_xmit_AYT();
#else
TELNET_NOT_AVAIL:
			ff(se, "  not available\r\n");
#endif
			break;

		case CTxlog:
			if (narg > 1)
				protocol_log_packets = yes_or_no(arg[1]);
			ff(se, "  protocol packet logging is %s",
				protocol_log_packets ? "on" : "off");
			ff(se, "\r\n");
			break;

		case 0:
			ff(se, "   procedure command not allowed in interactive mode\r\n");
			break;

		default:
			do_proc(narg, arg);
			sigint = 0;
			break;

	}
	return (0);				 /* 0 == do not end program */

}							 /* end of icmd */

/* end of ecuicmd.c */
/* vi: set tabstop=4 shiftwidth=4: */
