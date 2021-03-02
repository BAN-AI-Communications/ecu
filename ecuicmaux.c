/*+------------------------------------------------------------------------
	ecuicmaux.c
	wht@wht.net

  Defined functions:
	display_ascii_names()
	icmd_conversion(token, narg, arg)
	icmd_log(token, narg, arg)
	nlin_nlout_control(token, narg, arg)
	pcmd_rlog(param)
	rcvr_log_control(token, narg, arg)

--------------------------------------------------------------------------*/
/*+:EDITS:*/
/*:04-26-2000-11:15-wht@bob-RELEASE 4.42 */
/*:01-24-1997-02:37-wht@yuriatin-SOURCE RELEASE 4.00 */
/*:09-11-1996-20:00-wht@yuriatin-3.48-major telnet,curses,structural overhaul */
/*:09-06-1996-02:23-wht@kepler-cleanup */
/*:02-13-1996-15:22-wht@kepler-fix stupid semantics in log status */
/*:12-03-1995-20:34-wht@gyro-cosmetics and neatness */
/*:12-03-1995-20:25-wht@gyro-allow shell wild card chars in log filename */
/*:11-23-1995-11:20-wht@kepler-source control 3.37 for tsx-11 */
/*:11-14-1995-10:23-wht@kepler-3.37.80-source control point: SOCKETS */
/*:03-12-1995-01:03-wht@kepler-use ECU_MAXPN */
/*:05-04-1994-04:38-wht@n4hgf-ECU release 3.30 */
/*:09-10-1992-13:58-wht@n4hgf-ECU release 3.20 */
/*:08-22-1992-15:38-wht@n4hgf-ECU release 3.20 BETA */
/*:02-07-1992-19:19-root@n4hgf-fix incorrect log append/write notification */
/*:07-25-1991-12:56-wht@n4hgf-ECU release 3.10 */
/*:07-04-1991-20:03-wht@n4hgf-make pcmd_log */
/*:08-14-1990-20:40-wht@n4hgf-ecu3.00-flush old edit history */

#include "ecu.h"
#include "ecuerror.h"
#include "ecucmd.h"
#include "esd.h"

int rcvr_log = 0;			 /* if non-zero, logging rcvd data */
char rcvr_log_file[ECU_MAXPN];	/* log filename */
int rcvr_log_gen_title = 0;	 /* gen log header on next open (rcvr process) */
FILE *rcvr_log_fp = (FILE *) 0;	/* rcvr log file */
int rcvr_log_append = 1;
int rcvr_log_raw = 0;
int rcvr_log_flusheach = 0;

/*+-----------------------------------------------------------------------
	display_ascii_names()
------------------------------------------------------------------------*/
void
display_ascii_names()
{
	int intval;
	char *hex_to_ascii_name();

	for (intval = 0; intval < 32; intval++)
	{
		ff(se, "%s %3d %03o %02x ^%c | ", hex_to_ascii_name((intval)),
			(intval), (intval), (intval), (intval) | 0x40);
		ff(se, "    %3d %03o %02x  %c | ",
			intval + 32, intval + 32, intval + 32, intval + 32);
		ff(se, "    %3d %03o %02x  %c | ",
			intval + 64, intval + 64, intval + 64, intval + 64);
		if (intval != 31)
		{
			ff(se, "    %3d %03o %02x  %c\r\n",
				intval + 96, intval + 96, intval + 96, intval + 96);
		}
		else
		{
			ff(se, "    %3d %03o %02x  ^?\r\n",
				intval + 96, intval + 96, intval + 96);
		}
	}

}							 /* end of display_ascii_names */

/*+-------------------------------------------------------------------------
	icmd_conversion(token,narg,arg)
--------------------------------------------------------------------------*/
void
icmd_conversion(token, narg, arg)
int token;
int narg;
char **arg;
{
	int itmp;

	switch (token)
	{
		case CTxa:
		case CToa:
		case CTda:
			if (narg > 1)
			{
				int result;
				char format[4];

				sprintf(format, "%%%c", to_lower(*arg[0]));
				if (sscanf(arg[1], format, &result) == 0)
				{
					ff(se, "  invalid argument\r\n");
					return;
				}
				result &= 0xFF;
				if (result == ' ')
					ff(se, " == ' ' 0x20\r\n");
				else
					ff(se, " == %s\r\n", graphic_char_text(result, 1));
			}
			else
			{
				ff(se, "\r\n");
				display_ascii_names();
			}
			break;
		case CTax:
			if (arg[1] == (char *)0)
			{
				ff(se, "\r\n");
				display_ascii_names();
				break;
			}
			switch (strlen(arg[1]))
			{
				case 1:
					ff(se, " == 0x%02x\r\n", *arg[1]);
					break;
				case 2:
					if (*arg[1] == '^')
					{
						itmp = to_upper(*(arg[1] + 1));
						if ((itmp < '@') || (itmp > '_'))
						{
							ff(se, "  not a valid control character\r\n");
							return;
						}
						itmp &= 0x1F;
						ff(se, " == 0x%02x %s\r\n", itmp,
							graphic_char_text(itmp, 1));
						break;
					}		 /* else fall thru */
				case 3:
					if ((itmp = ascii_name_to_hex(arg[1])) > -1)
					{
						ff(se, " == 0x%02x %s\r\n", itmp,
							graphic_char_text(itmp, 1));
						break;
					}		 /* else fall thru */
				default:
					ff(se, "  invalid ... examples of valid parameters:\r\n");
					ff(se, "        ^A ETX  or  printable character\r\n");
					break;
			}
			break;
		default:
			ff(se, "  invalid command\r\n");
	}
}							 /* end of icmd_conversion */

/*+-------------------------------------------------------------------------
	rcvr_log_control(token,narg,arg)
--------------------------------------------------------------------------*/
int
rcvr_log_control(token, narg, arg)
int token;
int narg;
char **arg;
{
	int itmp;
	int itmp2;

#if defined(M_XENIX) || defined(M_UNIX)
	char *lparg = "/dev/lp1";

#endif

	switch (token)
	{
		case CTloff:
			goto LOG_OFF;

#if defined(M_XENIX) || defined(M_UNIX)
		case CTllp:
			narg = 1;
			arg = &lparg;
			/* fall thru */
#endif

		case CTlog:
			if (narg > 1)
			{
				if (minunique("off", arg[1], 3))
				{
				  LOG_OFF:
					shmx_set_rcvr_log("", 0, 0, 0);
					rcvr_log = 0;
					rcvr_log_file[0] = 0;
					return (0);
				}

				/*
				 * turning logging on
				 */
				itmp2 = -1;
				rcvr_log_append = 1;
				rcvr_log_raw = 0;
				for (itmp = 1; itmp < narg; itmp++)
				{
					if (*arg[itmp] == '-')
					{
						switch (arg[itmp][1])
						{
							case 's':
								rcvr_log_append = 0;
								break;
							case 'r':
								rcvr_log_raw = 1;
								break;
							case 'f':
								rcvr_log_flusheach = 1;
								break;
							default:
								pprintf("unrecognized switch -%c\n",
									arg[itmp][1]);
								log_cmd_usage();
								return (eFATAL_ALREADY);
						}
					}
					else
					{
						if (itmp2 > 0)
						{
							pputs("too many arguments\n");
							log_cmd_usage();
							return (eFATAL_ALREADY);
						}
						itmp2 = itmp;
					}
				}
				if (itmp2 < 0)
				{
					pputs("no log file name specified\n");
					log_cmd_usage();
					return (eFATAL_ALREADY);
				}
				if (arg[itmp2][0] != '/')	/* if log file not full path,
											 * ... */
				{			 /* ... supply current directory */
					get_curr_dir(rcvr_log_file,
						sizeof(rcvr_log_file) - strlen(arg[itmp2]) - 2);
					strcat(rcvr_log_file, "/");
					strcat(rcvr_log_file, arg[itmp2]);
				}
				else
					strcpy(rcvr_log_file, arg[itmp2]);

				/* try to open the file if we can */
				rcvr_log_fp = fopen(rcvr_log_file, "a");
				if (rcvr_log_fp)	/* if success */
				{
					fclose(rcvr_log_fp);
					rcvr_log_fp = (FILE *) 0;
					rcvr_log = 1;
					shmx_set_rcvr_log(rcvr_log_file, rcvr_log_append,
						rcvr_log_raw, rcvr_log_flusheach);
				}
				else
					/* xmtr() could not open file */
				{
					pputs("could not open ");
					pperror(rcvr_log_file);
					return (eFATAL_ALREADY);
				}
				rcvr_log_append = 1;
			}				 /* end of if argument to command */

			if (rcvr_log && (!proc_level || proc_trace))
			{
				pprintf("\n%sing %s received text to\n%s\n",
					(rcvr_log_append) ? "append" : "writ",
					(rcvr_log_raw) ? "raw" : "filtered",
					rcvr_log_file);
			}
			else if (!proc_level || proc_trace)
				pputs("not logging\n");
			break;

		default:
			pputs("invalid command\n");
			return (eFATAL_ALREADY);
	}
	return (0);
}							 /* end of rcvr_log_control */

/*+-------------------------------------------------------------------------
	pcmd_rlog(param) - control receiver logging (script)

rlog [-srf] ['filename']  #  see ecuidmc.d
rlog 'off'

This is a hack to use icmd stuff from proc language
--------------------------------------------------------------------------*/
int
pcmd_rlog(param)
ESD *param;
{
	int erc;
	int lnarg = 0;
	char *larg[3];
	ESD *tesd = esdalloc(ESD_NOMSZ);
	char switches[8];

	if (!tesd)
		return (eNoMemory);

	larg[lnarg++] = "log";

	if (!get_switches(param, switches, sizeof(switches)))
		larg[lnarg++] = switches;

	if (!gstr(param, tesd, 1))
		larg[lnarg++] = tesd->pb;

	erc = rcvr_log_control(CTlog, lnarg, larg);
	esdfree(tesd);
	return (erc);

}							 /* end of pcmd_rlog */

/*+-------------------------------------------------------------------------
	icmd_log(token,narg,arg)
--------------------------------------------------------------------------*/
int
icmd_log(token, narg, arg)
int token;
int narg;
char **arg;
{
	int itmp;
	int itmp2;
	char proto_fname[ECU_MAXPN];

	switch (token)
	{
		case CTloff:
			goto LOG_OFF;

#if defined(M_XENIX) || defined(M_UNIX)
		case CTllp:
			icmd("log /dev/lp1");
			break;
#endif

		case CTlog:
			if (narg > 1)
			{
				if (minunique("off", arg[1], 3))
				{
				  LOG_OFF:
					if (rcvr_log == 0)	/* "off", but not logging */
						goto RECORD_REPORT;
					ff(se, "\r\nlogging concluded (file %s)\r\n",
						rcvr_log_file);
					shmx_set_rcvr_log("", 0, 0, 0);
					rcvr_log = 0;
					rcvr_log_file[0] = 0;
					return (0);
				}

				/*
				 * turning logging on
				 */
				itmp2 = -1;
				rcvr_log_append = 1;
				rcvr_log_raw = 0;
				for (itmp = 1; itmp < narg; itmp++)
				{
					if (*arg[itmp] == '-')
					{
						switch (arg[itmp][1])
						{
							case 's':
								rcvr_log_append = 0;
								break;
							case 'r':
								rcvr_log_raw = 1;
								break;
							case 'f':
								rcvr_log_flusheach = 1;
								break;
							default:
								ff(se, "   unrecognized switch -%c\r\n",
									arg[itmp][1]);
								log_cmd_usage();
								return (eFATAL_ALREADY);
						}
					}
					else
					{
						if (itmp2 > 0)
						{
							ff(se, "   too many positional arguments\r\n");
							log_cmd_usage();
							return (eFATAL_ALREADY);
						}
						itmp2 = itmp;
					}
				}
				if (itmp2 < 0)
				{
					ff(se, "   no log file name specified\r\n");
					log_cmd_usage();
					return (eFATAL_ALREADY);
				}

				/*
				 * expand wild card chars
				 */
				strncpy(proto_fname, arg[itmp2], sizeof(proto_fname));
				proto_fname[sizeof(proto_fname) - 1] = 0;
				if (expand_filename(proto_fname, sizeof(proto_fname)))
					goto RECORD_REPORT;

				/*
				 * if log file not full path, supply current directory
				 */
				if (proto_fname[0] != '/')
				{
					get_curr_dir(rcvr_log_file,
						sizeof(rcvr_log_file) - strlen(proto_fname) - 2);
					strcat(rcvr_log_file, "/");
					strcat(rcvr_log_file, proto_fname);
				}
				else
					strcpy(rcvr_log_file, proto_fname);

				/*
				 * try to open the file if we can
				 */
				rcvr_log_fp = fopen(rcvr_log_file, "a");
				if (rcvr_log_fp)	/* if success */
				{
					fclose(rcvr_log_fp);
					rcvr_log_fp = (FILE *) 0;
					rcvr_log = 1;
					shmx_set_rcvr_log(rcvr_log_file, rcvr_log_append,
						rcvr_log_raw, rcvr_log_flusheach);
				}
				else
				{

					/*
					 * xmtr() could not open file
					 */
					ff(se, "   could not open ");
					perror(rcvr_log_file);
					ff(se, "\r\n");
					return (eFATAL_ALREADY);
				}
			}				 /* end of if argument to command */

		  RECORD_REPORT:
			if (rcvr_log)
			{
				ff(se, "\r\n%sing %s received text to\r\n%s\r\n",
					(rcvr_log_append) ? "append" : "writ",
					(rcvr_log_raw) ? "raw" : "filtered",
					rcvr_log_file);
				ff(se, "use \"HOME log off\" to stop logging\r\n");
			}
			else
			{
				ff(se, "   not logging.\r\n");
				ff(se, "use \"HOME slog <filename>\" to start logging\r\n");
			}
			break;
		default:
			ff(se, "  invalid command\r\n");
			return (eFATAL_ALREADY);
	}
	rcvr_log_append = 1;
	return (0);
}							 /* end of icmd_log */

/*+-------------------------------------------------------------------------
	nlin_nlout_control(token,narg,arg)
--------------------------------------------------------------------------*/
void
nlin_nlout_control(token, narg, arg)
int token;
int narg;
char **arg;
{
	switch (token)
	{
		case CTnlin:
			if (narg != 1)
				shm->Ladd_nl_incoming = yes_or_no(arg[1]);
			ff(se, "  %sappending NL to incoming CR\r\n",
				(shm->Ladd_nl_incoming) ? "" : "not ");
			break;
		case CTnlout:
			if (narg != 1)
				shm->Ladd_nl_outgoing = yes_or_no(arg[1]);
			ff(se, "  %sappending NL to outgoing CR\r\n",
				(shm->Ladd_nl_outgoing) ? "" : "not ");
			break;
		default:
		case CTnl:
			ff(se, "  incoming: %s  outgoing: %s\r\n",
				(shm->Ladd_nl_incoming) ? "CR/LF" : "CR",
				(shm->Ladd_nl_outgoing) ? "CR/LF" : "CR");
			break;
	}

}							 /* end of nlin_nlout_control */

/* end of ecuicmaux.c */
/* vi: set tabstop=4 shiftwidth=4: */
