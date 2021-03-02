/*+-------------------------------------------------------------------------
	ecuxfer.c - file transfer
	wht@wht.net

   000000000011111111112222222222333333333344444444445555555555666666666actually
   012345678901234567890123456789012345678901234567890123456789012345678wider
00:.--[ Send ZMODEM/CRC32 ]----------------------------------------...-.
01:|                                                               ... |
02:|  File(s) to send:                                             ... |
03:|  _____________________________________________________________... |
04:|                                                               ... |
05:|  Binary? Y (no NL-CR/LF translation)                          ... |
06:|  Overwrite destination files? Y                               ... |
07:|  Send full pathames?  N                                       ... |
08:|  Transfer only newer files?   N                               ... |
09:|  Resume interrupted transfer? N                               ... |
10:|  Window size: ______  Escape all control characters? _        ... |         |
11:|                                                               ... |
-1:| TAB:next  ^B:prev  END:perform transfer  ESC:abort            ... |
-0:`---------------------------------------------------------------...-'

  Defined functions:
	file_xfer_done_bell()
	file_xfer_start()
	receive_files_from_remote(argc, argv)
	report_send_status()
	send_files_to_remote(argc, argv)
	xfer_title_fragment()
	xfrw_bot_msg(msg)
	xfrw_display_cmd_line()
	xfrw_get_single(nondelim_list)
	xfrws_display_allvars()
	xfrws_display_binary()
	xfrws_display_literals()
	xfrws_display_name()
	xfrws_display_resume()
	xfrws_display_xfernew()

  'I like folks!  All types and kinds!  People are a weakness o'
  mine...  Swat my hind with melon rind, But dat's my penguin
  state o' mind!' - Penguin Opus (Bloom County)

--------------------------------------------------------------------------*/
/*+:EDITS:*/
/*:04-26-2000-11:15-wht@bob-RELEASE 4.42 */
/*:03-16-1997-02:45-rll@felton.felton.ca.us-Make nice boxes for SCO */
/*:01-24-1997-02:37-wht@yuriatin-SOURCE RELEASE 4.00 */
/*:09-11-1996-20:00-wht@yuriatin-3.48-major telnet,curses,structural overhaul */
/*:07-24-1996-21:37-wht@n4hgf-no more wvline/whline */
/*:01-01-1996-20:34-wht@kepler-chk tty_not_char_special for passing -_ */
/*:01-01-1996-18:59-wht@kepler-fix transfer status */
/*:12-06-1995-13:30-wht@n4hgf-termecu w/errno -1 consideration */
/*:11-24-1995-09:52-wht@kepler-receive_files_from_remote returns error code */
/*:11-23-1995-11:20-wht@kepler-source control 3.37 for tsx-11 */
/*:11-14-1995-10:23-wht@kepler-3.37.80-source control point: SOCKETS */
/*:11-06-1995-17:24-wht@kepler-further distance xfer stats below banner */
/*:11-05-1995-10:02-wht@kepler-add support for sz escape all ctrl characters */
/*:10-14-1995-23:39-wht@kepler-pass bit rate to ecurz/sz with -@ */
/*:10-14-1995-23:22-wht@kepler-drop SEAlink support */
/*:03-12-1995-03:27-wht@kepler-use ECU_MAXPN */
/*:01-12-1995-15:19-wht@n4hgf-apply Andrew Chernov 8-bit clean+FreeBSD patch */
/*:05-04-1994-04:39-wht@n4hgf-ECU release 3.30 */
/*:12-19-1992-15:57-wht@gyro-reformat functions out of alignment */
/*:09-10-1992-13:59-wht@n4hgf-ECU release 3.20 */
/*:09-05-1992-15:31-wht@n4hgf-add resume choice  */
/*:08-22-1992-15:38-wht@n4hgf-ECU release 3.20 BETA */
/*:08-21-1992-15:21-wht@n4hgf-look for ecu xfer programs in CFG_EcuLibDir */
/*:09-25-1991-16:26-wht@n4hgf2-flexible C-Kermit filename */
/*:09-17-1991-19:41-wht@n4hgf-restore console termio after xfer prog runs */
/*:08-28-1991-14:07-wht@n4hgf2-SVR4 cleanup by aega84!lh */
/*:08-25-1991-14:39-wht@n4hgf-SVR4 port thanks to aega84!lh */
/*:07-25-1991-12:57-wht@n4hgf-ECU release 3.10 */
/*:07-17-1991-07:04-wht@n4hgf-avoid SCO UNIX nap bug */
/*:07-14-1991-18:18-wht@n4hgf-new ttygets functions */
/*:06-04-1991-14:18-wht@n4hgf-sometimes I forget 286: p_zwindwsz now UINT */
/*:08-14-1990-20:40-wht@n4hgf-ecu3.00-flush old edit history */

#include "ecucurses.h"

#define STDIO_H_INCLUDED
#define OMIT_TERMIO_REFERENCES
#include "ecu.h"
#include "ecuerror.h"
#include "ecukey.h"
#include "ecuxkey.h"
#include "ecutty.h"
#include "pc_scr.h"

long atol();
char *find_executable();

#ifdef M_I286
#define ZWINDW_LIMIT 65472
#else
#define ZWINDW_LIMIT 65536
#endif

/* -- protocol xfer types -- */
#define ECUSZ_X			1
#define ECUSZ_Y			2
#define ECUSZ_Z			3
#define CKERMIT			5
#define ECURZ_X			7
#define ECURZ_Y			8
#define ECURZ_Z			9

/* --------------------- send window ----------------------------------- */
/*      SNDW_LINES			calculated (xfrw_lines) */
#define SNDW_COLS			79
#define SNDW_TLY			2
/*      SNDW_TLX			calculated (xfrw_tlx) */

#define SNDW_FILE_LY		2
#define SNDW_FILE_LX		3
#define SNDW_FILE_Y			3
#define SNDW_FILE_X			3
#define SNDW_FILE_LEN		(SNDW_COLS - SNDW_FILE_X - 2)

#define SNDW_BIN_Y			5
#define SNDW_BIN_LX			3
#define SNDW_BIN_X			11
#define SNDW_BIN_LX2		13

#define SNDW_OVERW_Y		6
#define SNDW_OVERW_LX		3
#define SNDW_OVERW_X		32

#define SNDW_SENDFULL_Y		7
#define SNDW_SENDFULL_LX	3
#define SNDW_SENDFULL_X		24

#define SNDW_XFERNEW_Y		8
#define SNDW_XFERNEW_LX		3
#define SNDW_XFERNEW_X		32

#define SNDW_RESUME_Y		9
#define SNDW_RESUME_LX		3
#define SNDW_RESUME_X		32

#define SNDW_ZWINDW_Y		10
#define SNDW_ZWINDW_LX		3
#define SNDW_ZWINDW_X		16
#define SNDW_ZWINDW_LEN		6

#define SNDW_ZCTLESC_Y		10
#define SNDW_ZCTLESC_LX		24
#define SNDW_ZCTLESC_X		55
#define SNDW_ZCTLESC_LEN	7

extern char curr_dir[ECU_MAXPN];	/* current working directory */
extern int protocol_log_packets;
extern int last_child_wait_status;

long file_xfer_start_time;	 /* time() value at beginning of file xfer */
char xfertype = -1;			 /* file xfer type */
WINDOW *xfrw;
char xfrw_cols;
char xfrw_lines;
char xfrw_tlx;

char p_binary;				 /* xfer options -- not all apply to all
							  * protocols */
char p_sendfull;
char p_overwrite;
char p_xfernew;
char p_resume;
char p_zctlesc;
char p_filelist[80];
UINT p_zwindwsz;

/*+-------------------------------------------------------------------------
	file_xfer_start()
--------------------------------------------------------------------------*/
void
file_xfer_start()
{
	time(&file_xfer_start_time);
}							 /* end of file_xfer_start */

/*+-------------------------------------------------------------------------
	file_xfer_done_bell()
--------------------------------------------------------------------------*/
void
file_xfer_done_bell()
{
	long xfer_time;
	int xbell_count = 0;

	time(&xfer_time);
	xfer_time -= file_xfer_start_time;
	sleep(1);
	xbell_count = 1;
	if (xfer_time >= 3600L)	 /* >= one hour */
		xbell_count = 3;
	else if (xfer_time >= 1800L)	/* >= 1/2 hour */
		xbell_count = 2;

	if (shm->bell_notify_state)
		bell_notify(XBELL_ATTENTION);
	xbell(XBELL_DONE, xbell_count);

}							 /* end of file_xfer_done_bell */

/*+-------------------------------------------------------------------------
	xfrw_bot_msg(msg)
--------------------------------------------------------------------------*/
void
xfrw_bot_msg(msg)
char *msg;
{
	int itmp;
	int itmp2;
	static int last_msglen = 0;
	char msg2[80];

	wmove(xfrw, xfrw_lines - 1, 3);

	if ((itmp = strlen(msg)) == 0)
	{
		itmp2 = last_msglen + 2;
		for (itmp = 0; itmp < itmp2; itmp++)
#if defined(SVR4) || defined(SCO32v4) || defined(SCO32v5)
			waddch(xfrw, sHR);
#else
			waddch(xfrw, sHR & 0xFF);
#endif
		last_msglen = 0;
	}
	else
	{
		waddch(xfrw, ' ');
		if (itmp > xfrw_cols - 3 - 2)
		{
			strncpy(msg2, msg, xfrw_cols - 3 - 2);
			msg2[xfrw_cols - 3 - 2 + 1] = 0;
			waddstr(xfrw, msg2);
			itmp = strlen(msg2);
		}
		else
		{
			waddstr(xfrw, msg);
			itmp = strlen(msg);
		}
		waddch(xfrw, ' ');
		if ((itmp2 = last_msglen - itmp) > 0)
		{
			while (itmp2--)
#if defined(SVR4) || defined(SCO32v4) || defined(SCO32v5)
				waddch(xfrw, sHR);
#else
				waddch(xfrw, sHR & 0xFF);
#endif
		}
		last_msglen = itmp;	 /* remember last message length */
	}
	wrefresh(xfrw);
}							 /* end of xfrw_bot_msg */

/*+-------------------------------------------------------------------------
	xfrw_get_single(nondelim_list)
assumes cursor is already positioned
--------------------------------------------------------------------------*/
UINT
xfrw_get_single(nondelim_list)
UINT *nondelim_list;
{
	UINT itmp;
	static UINT xfrw_nondelim_list[] =
	{
		CRET, NL, XFcurup, XFcurdn, CTL_B, TAB, ESC,
		CTL_L, CTL_R, XFend, 0
	};

	itmp = winget_single(xfrw, nondelim_list, xfrw_nondelim_list);
	if ((itmp & 0x1FF) == CRET)
		itmp = NL | 0x1000;
	return (itmp);
}							 /* end of xfrw_get_single */

/*+-------------------------------------------------------------------------
	xfer_title_fragment()
--------------------------------------------------------------------------*/
char *
xfer_title_fragment()
{
	char *cp = "UNKNOWN";

	switch (xfertype)
	{
		case ECURZ_X:
		case ECUSZ_X:
			cp = "XMODEM/CRC";
			break;
		case ECURZ_Y:
		case ECUSZ_Y:
			cp = "YMODEM/CRC";
			break;
		case ECURZ_Z:
		case ECUSZ_Z:
			cp = "ZMODEM/CRC32";
			break;
		case CKERMIT:
			cp = "KERMIT/CRC";
			break;
	}
	return (cp);
}							 /* end of xfer_title_fragment */

/*+-------------------------------------------------------------------------
	xfrw_display_cmd_line()
--------------------------------------------------------------------------*/
void
xfrw_display_cmd_line()
{
	int itmp;
	char *cmd_string = "TAB:next  ^B:prev  END:perform transfer  ESC:abort";
	int left_spaces = ((xfrw_cols - 2) - strlen(cmd_string)) / (unsigned)2;
	int x;
	int y;

	wmove(xfrw, xfrw_lines - 2, 1);
	wstandout(xfrw);
	for (itmp = 0; itmp < left_spaces; itmp++)
		waddch(xfrw, ' ');
	waddstr(xfrw, cmd_string);
	getyx(xfrw, y, x);
	while (++x < xfrw_cols)
		waddch(xfrw, ' ');
	wstandend(xfrw);

}							 /* end of xfrw_display_cmd_line */

/*+-------------------------------------------------------------------------
	xfrws_display_literals()
--------------------------------------------------------------------------*/
void
xfrws_display_literals()
{
	wmove(xfrw, SNDW_FILE_LY, SNDW_FILE_LX);
	if (xfertype == ECUSZ_X)
		waddstr(xfrw, "File");
	else
		waddstr(xfrw, "File(s)");
	waddstr(xfrw, " to send:");

	switch (xfertype)
	{
		case ECUSZ_Z:
			wmove(xfrw, SNDW_XFERNEW_Y, SNDW_XFERNEW_LX);
			waddstr(xfrw, "Transfer only newer files?");
			wmove(xfrw, SNDW_SENDFULL_Y, SNDW_SENDFULL_LX);
			waddstr(xfrw, "Send full pathames?");
			wmove(xfrw, SNDW_RESUME_Y, SNDW_RESUME_LX);
			waddstr(xfrw, "Resume interrupted transfer?");
			wmove(xfrw, SNDW_ZWINDW_Y, SNDW_ZWINDW_LX);
			waddstr(xfrw, "Window size:");
			wmove(xfrw, SNDW_ZCTLESC_Y, SNDW_ZCTLESC_LX);
			waddstr(xfrw, "Escape all control characters?");
		case CKERMIT:
			wmove(xfrw, SNDW_OVERW_Y, SNDW_OVERW_LX);
			waddstr(xfrw, "Overwrite destination files?");
		case ECUSZ_Y:
		case ECUSZ_X:
			wmove(xfrw, SNDW_BIN_Y, SNDW_BIN_LX);
			waddstr(xfrw, "Binary?");
	}
}							 /* end of xfrws_display_literals */

/*+-------------------------------------------------------------------------
	xfrws_display_name()
--------------------------------------------------------------------------*/
void
xfrws_display_name()
{
	clear_area(xfrw, SNDW_FILE_Y, SNDW_FILE_X, SNDW_FILE_LEN);
	waddstr(xfrw, p_filelist);

}							 /* end of xfrws_display_name */

/*+-------------------------------------------------------------------------
	xfrws_display_binary()
--------------------------------------------------------------------------*/
void
xfrws_display_binary()
{
	wmove(xfrw, SNDW_BIN_Y, SNDW_BIN_X);
	waddch(xfrw, (p_binary) ? 'Y' : 'N');
	if (p_binary)
		waddstr(xfrw, " (no NL-CR/LF translation)     ");
	else
		waddstr(xfrw, " (NL-CR/LF translation enabled)");

}							 /* end of xfrws_display_binary */

/*+-------------------------------------------------------------------------
	xfrws_display_xfernew()
--------------------------------------------------------------------------*/
void
xfrws_display_xfernew()
{
	wmove(xfrw, SNDW_XFERNEW_Y, SNDW_XFERNEW_X);
	waddch(xfrw, (p_xfernew) ? 'Y' : 'N');
	if (p_xfernew)
		waddstr(xfrw, " (if receiver supports)");
	else
		waddstr(xfrw, "                       ");
}							 /* end of xfrws_display_xfernew */

/*+-------------------------------------------------------------------------
	xfrws_display_resume()
--------------------------------------------------------------------------*/
void
xfrws_display_resume()
{
	wmove(xfrw, SNDW_RESUME_Y, SNDW_RESUME_X);
	waddch(xfrw, (p_resume) ? 'Y' : 'N');
	if (p_resume)
		waddstr(xfrw, " (if receiver supports)");
	else
		waddstr(xfrw, "                       ");
}							 /* end of xfrws_display_resume */

/*+-------------------------------------------------------------------------
	xfrws_display_allvars()
--------------------------------------------------------------------------*/
void
xfrws_display_allvars()
{
	char s32[32];

	xfrws_display_name();
	switch (xfertype)
	{
		case ECUSZ_Z:
			xfrws_display_xfernew();
			xfrws_display_resume();
			wmove(xfrw, SNDW_SENDFULL_Y, SNDW_SENDFULL_X);
			waddch(xfrw, (p_sendfull) ? 'Y' : 'N');
			wmove(xfrw, SNDW_ZWINDW_Y, SNDW_ZWINDW_X);
			sprintf(s32, "%*u", SNDW_ZWINDW_LEN, p_zwindwsz);
			waddstr(xfrw, s32);
			wmove(xfrw, SNDW_ZCTLESC_Y, SNDW_ZCTLESC_X);
			waddch(xfrw, (p_zctlesc) ? 'Y' : 'N');
		case CKERMIT:
			wmove(xfrw, SNDW_OVERW_Y, SNDW_OVERW_X);
			waddch(xfrw, (p_overwrite) ? 'Y' : 'N');
		case ECUSZ_Y:
		case ECUSZ_X:
			xfrws_display_binary();
	}
}							 /* end of xfrws_display_allvars */

/*+-------------------------------------------------------------------------
	report_send_status() - report file transmission result
returns proc-type erc
for "ecu knowledgeable" protocols only
--------------------------------------------------------------------------*/
int
report_send_status()
{
	int erc = 0;
	UINT16 int ustmp;
	UINT32 colors_at_entry = colors_current;
	char *signal_name_text();

	ustmp = last_child_wait_status;
	if ((ustmp & 0xFF) == 0) /* exit() called */
	{
		ustmp >>= 8;
		if (ustmp == 0)
		{
			setcolor(colors_notify);
			pputs("[transfer successful]");
			erc = 0;
		}
		else if (ustmp == 255)
		{
			setcolor(colors_error);
			pputs("[ecu error: transfer program usage error]");
			erc = eFATAL_ALREADY;
		}
		else if (ustmp == 254)
		{
			setcolor(colors_error);
			pputs(
				"[protocol failure: bad line conditions or remote not ready]");
			erc = eFATAL_ALREADY;
		}
		else if (ustmp == 253)
		{
			setcolor(colors_alert);
			pputs("[no requested files exist]");
			erc = eFATAL_ALREADY;
		}
		else if (ustmp < 128)
		{
			setcolor(colors_alert);
			if (ustmp == 127)
				pputs("[127 or more files skipped]");
			else
				pprintf("[%u files rejected]", ustmp);
			erc = 0;
		}
		else
		{
			setcolor(colors_alert);
			pprintf("[transfer aborted by %s]",
				signal_name_text(ustmp & 0x7F));
			erc = eProcAttn_Interrupt;
		}
	}
	else
	{
		tcap_curbotleft();
		setcolor(colors_error);
		pprintf("[transfer killed by %s]",
			signal_name_text(ustmp & 0x7F));
		erc = eProcAttn_Interrupt;
	}

	setcolor(colors_at_entry);
	pputs("\n");
	return (erc);

}							 /* end of report_send_status */

/*+-----------------------------------------------------------------------
	send_files_to_remote(argc,argv)
------------------------------------------------------------------------*/
void
send_files_to_remote(argc, argv)
int argc;
char **argv;
{
	int itmp;
	int input_state = 0;
	int input_state_mod;
	long ltmp;
	int input_done;
	int used_argv = 0;
	char *ckufnm;
	char execcmd[256];
	char s80[80];
	char flst[80];
	UINT delim;				 /* important to be unsigned to avoid sign
							  * extension */
	WINDOW *window_create();
	char bottom_label[64];
	int old_ttymode = get_ttymode();
	static UINT *use_delim;
	static UINT use_ny_delim[] =
	{'n', 'y', 0};

#if defined(CFG_TelnetOption)
	static UINT use_y_delim[] =
	{'n', 'y', 0};

#endif

	p_binary = 1;			 /* assume p_binary xfer */
	p_sendfull = 0;			 /* assume no full pathnames */
	p_overwrite = 1;		 /* assume overwrite */
	p_xfernew = 0;			 /* assume send only newer */
	p_resume = 0;			 /* assume send full */
	p_zctlesc = 0;			 /* assume no exhaustive ctrl char esc */
#if defined(WHT)
	p_zwindwsz = 65536;		 /* my preference */
#else
#if defined(DEFAULT_SZ_WINDOW)
	p_zwindwsz = DEFAULT_SZ_WINDOW_SIZE;	/* your preference */
#else
	p_zwindwsz = 0;			 /* default to full streaming sz */
#endif /* DEFAULT_SZ_WINDOW */
#endif /* WHT */

#if defined(CFG_TelnetOption)
	if (shm->Ltelnet)
	{
		p_zwindwsz = 16384;	 /* recommended */
		p_zctlesc = 1;		 /* required */
	}
#endif

	p_filelist[0] = 0;		 /* no filenames yet */
	switch (to_lower(*(argv[0] + 1)))
	{
		case 'x':
			xfertype = ECUSZ_X;
			break;
		case 'y':
			xfertype = ECUSZ_Y;
			break;
		case 'z':
			xfertype = ECUSZ_Z;
			break;
		case 'k':
			xfertype = CKERMIT;
			break;
		default:
			ff(se, "send command invalid\n");
			return;
	}

	kill_rcvr_process(SIGUSR1);	/* SIGUSR1 gives chance to close log file */

	/* define and open window */
	input_state_mod = 0;
	xfrw_tlx = (COLS - SNDW_COLS) / 2;
	xfrw_cols = SNDW_COLS;
	switch (xfertype)
	{
		case ECUSZ_X:
			input_state_mod = 2;
			xfrw_lines = 9;
			break;
		case ECUSZ_Z:
			input_state_mod = 8;
			xfrw_lines = input_state_mod + 6;	/* last line has two */
			break;
		case CKERMIT:
			input_state_mod = 3;
			xfrw_lines = input_state_mod + 7;
			break;
		case ECUSZ_Y:
			input_state_mod = 2;
			xfrw_lines = input_state_mod + 7;
			break;
		default:
			ff(se, "\r\nlogic error: unknown xfertype\r\n");
			errno = -1;
			termecu(TERMECU_LOGIC_ERROR);
	}

	windows_start();
	sprintf(execcmd, "Send %s", xfer_title_fragment());
	xfrw = window_create(execcmd, -3, SNDW_TLY, (int)xfrw_tlx,
		(int)xfrw_lines, (int)xfrw_cols);
	xfrw_display_cmd_line();
	xfrws_display_literals();
	xfrws_display_allvars();
	wmove(xfrw, 0, 27);
	waddstr(xfrw, " dir: ");
	if (strlen(curr_dir) > (unsigned)(xfrw_cols - 32))
	{
		strncpy(s80, curr_dir, xfrw_cols - 32);
		s80[xfrw_cols - 32] = 0;
		waddstr(xfrw, s80);
	}
	else
		waddstr(xfrw, curr_dir);
	waddch(xfrw, ' ');
	flst[0] = 0;

  REENTER_INPUT_LOOP:
	input_done = 0;
	while (!input_done)
	{
		switch (input_state)
		{
			case 0:		 /* filename(s) */
				xfrw_bot_msg("enter file(s) to send");
			  CASE_0_AGAIN:
				if (used_argv || (argc == 1))
				{
					itmp = wingets(xfrw, SNDW_FILE_Y, SNDW_FILE_X, flst,
						SNDW_FILE_LEN + 1, &delim,
						(p_filelist[0] != 0), (int *)0);
				}
				else
				{
					used_argv = 1;
					flst[0] = 0;
					for (itmp = 1; itmp < argc; itmp++)
					{
						if ((strlen(flst) + strlen(argv[itmp]) + 1) >
							sizeof(flst))
						{
							xfrw_bot_msg("arguments too long ... reenter list");
							ring_bell();
							goto CASE_0_AGAIN;
						}
						strcat(flst, argv[itmp]);
						if (itmp != (argc - 1))
							strcat(flst, " ");
					}
					delim = NL;
				}
				if (delim == ESC)
					break;
				if (strlen(flst))
				{
					strcpy(p_filelist, flst);
					xfrws_display_name();
					if (find_shell_chars(p_filelist))
					{
						char *expcmd;

						if (expand_wildcard_list(p_filelist, &expcmd))
						{
							xfrw_bot_msg(expcmd);
							ring_bell();
							goto CASE_0_AGAIN;
						}
						expcmd[SNDW_FILE_LEN - 1] = 0;
						clear_area(xfrw, SNDW_FILE_Y, SNDW_FILE_X,
							SNDW_FILE_LEN);
						waddstr(xfrw, expcmd);
						free(expcmd);
					}
				}
				break;

			case 1:		 /* binary */
				xfrw_bot_msg("Y: no conversion, N: NLs converted to CR/LF");
				wmove(xfrw, SNDW_BIN_Y, SNDW_BIN_X);
				wrefresh(xfrw);
				delim = NL;
				switch (itmp = xfrw_get_single(use_ny_delim))
				{
					case 0:
					case 1:
						p_binary = itmp;
						xfrws_display_binary();
						break;
					default:
						delim = itmp & 0x1FF;
						break;
				}
				break;

			case 2:		 /* overwrite */
				xfrw_bot_msg("Y: overwrite, N: protect destination files");
				wmove(xfrw, SNDW_OVERW_Y, SNDW_OVERW_X);
				wrefresh(xfrw);
				delim = NL;
				switch (itmp = xfrw_get_single(use_ny_delim))
				{
					case 0:
					case 1:
						p_overwrite = itmp;
						break;
					default:
						delim = itmp & 0x1FF;
						break;
				}
				break;

			case 3:		 /* send full pathnames */
				xfrw_bot_msg(
					"Y: full pathnames, N: strip directory portion from names");
				wmove(xfrw, SNDW_SENDFULL_Y, SNDW_SENDFULL_X);
				wrefresh(xfrw);
				delim = NL;
				switch (itmp = xfrw_get_single(use_ny_delim))
				{
					case 0:
					case 1:
						p_sendfull = itmp;
						break;
					default:
						delim = itmp & 0x1FF;
						break;
				}
				break;

			case 4:		 /* src newer than dest */

				xfrw_bot_msg(
					"Y: send only if source newer than destination, N send all");
				wmove(xfrw, SNDW_XFERNEW_Y, SNDW_XFERNEW_X);
				wrefresh(xfrw);
				delim = NL;
				switch (itmp = xfrw_get_single(use_ny_delim))
				{
					case 0:
					case 1:
						p_xfernew = itmp;
						xfrws_display_xfernew();
						break;
					default:
						delim = itmp & 0x1FF;
						break;
				}
				break;

			case 5:		 /* resume interrupted transfer */

				xfrw_bot_msg(
					"Y: resume transfer at remote file EOF, N send all");
				wmove(xfrw, SNDW_RESUME_Y, SNDW_RESUME_X);
				wrefresh(xfrw);
				delim = NL;
				switch (itmp = xfrw_get_single(use_ny_delim))
				{
					case 0:
					case 1:
						p_resume = itmp;
						xfrws_display_resume();
						break;
					default:
						delim = itmp & 0x1FF;
						break;
				}
				break;

			case 6:		 /* window size */

				xfrw_bot_msg(
					"window size (max bytes sent before ACK required) 0 = stream");
			  CASE_6_AGAIN:
				sprintf(s80, "%u", p_zwindwsz);
				clear_area(xfrw, SNDW_ZWINDW_Y, SNDW_ZWINDW_X,
					SNDW_ZWINDW_LEN);
				itmp = wingets(xfrw, SNDW_ZWINDW_Y, SNDW_ZWINDW_X, s80,
					SNDW_ZWINDW_LEN + 1, &delim, 1, (int *)0);
				if ((delim == ESC))
					break;
				if (((ltmp = atol(s80)) != 0) &&
					((ltmp < 256) || (ltmp > ZWINDW_LIMIT)))
				{
					char xbmsg[80];

					ring_bell();
					sprintf(xbmsg, "window size must be 0 or 256 <= w <= %u",
						ZWINDW_LIMIT);
					xfrw_bot_msg(xbmsg);
					goto CASE_6_AGAIN;
				}
				p_zwindwsz = (UINT) (ltmp / 64L) * 64L;
				sprintf(s80, "%u%s", p_zwindwsz,
					(p_zwindwsz != (UINT) ltmp) ? " adjusted" : "");
				clear_area(xfrw, SNDW_ZWINDW_Y, SNDW_ZWINDW_X,
					SNDW_ZWINDW_LEN + 10);
				waddstr(xfrw, s80);
				break;
			case 7:		 /* window size */

				use_delim = use_ny_delim;
#if defined(CFG_TelnetOption)
				if (shm->Ltelnet)
				{
					p_zctlesc = 1;
					use_delim = use_y_delim;
					xfrw_bot_msg(
						"Over telnet, escaping 0xFF is REQUIRED");
				}
				else
#endif
				{
					xfrw_bot_msg(
						"On some networks, escaping 0x01-0x1F,0x7F,0xFF required");
				}
				wmove(xfrw, SNDW_ZCTLESC_Y, SNDW_ZCTLESC_X);
				wrefresh(xfrw);
				delim = NL;
				switch (itmp = xfrw_get_single(use_delim))
				{
					case 0:
					case 1:
						p_zctlesc = itmp;
						break;
					default:
						delim = itmp & 0x1FF;
						break;
				}
				break;
		}

		switch (delim)
		{
			case XFcurup:
			case CTL_B:
				input_state = (input_state) ? input_state - 1
					: input_state_mod - 1;
				break;

			case XFcurdn:
			case TAB:
			case NL:
				input_state++;
				input_state %= input_state_mod;
				break;

			case CTL_L:
			case CTL_R:
				tcap_clear_screen();
				touchwin(stdscr);
				wrefresh(stdscr);
				touchwin(xfrw);
				wrefresh(xfrw);
				break;

			case ESC:
				xfrw_bot_msg("transfer abandoned");
				input_done = 1;
				break;

			case XFend:
				input_done = 1;
				break;
		}
	}

	if (delim == XFend)
	{
		if (!p_filelist[0])
		{
			ring_bell();
			xfrw_bot_msg("No filenames entered!  Press <ENTER>");
			(void)ttygetc(1);
			input_state = 0;
			goto REENTER_INPUT_LOOP;
		}
		xfrw_bot_msg("starting file transfer");
	}

	wrefresh(xfrw);
	delwin(xfrw);
	windows_end(0);
	tcap_cursor(SNDW_TLY + xfrw_lines + 2, 0);

	if (delim == ESC)
	{
		start_rcvr_process(1);
		return;
	}

	bottom_label[0] = 0;
	if (shm->Llogical[0])
		sprintf(bottom_label, "-C \"'Connected to %.20s'\" ", shm->Llogical);

	/* we are going to do a transfer! */
	switch (xfertype)
	{
		case ECUSZ_X:
			sprintf(execcmd, "%s/ecusz -X -@ %d -. %d ",
				eculibdir, shm->Lbitrate, shm->Liofd);
			strcat(execcmd, bottom_label);
			if (protocol_log_packets)
				strcat(execcmd, "-, ");
			if (p_binary)
				strcat(execcmd, "-b ");
			else
				strcat(execcmd, "-a ");
			if (tty_not_char_special || (shm->ttyuse == TTYUSE_FORCE_SIMPLE))
				strcat(execcmd, "-_ ");
			strcat(execcmd, p_filelist);
			break;
		case ECUSZ_Y:
			sprintf(execcmd, "%s/ecusz -Y -@ %d -. %d -k ",
				eculibdir, shm->Lbitrate, shm->Liofd);
			strcat(execcmd, bottom_label);
			if (protocol_log_packets)
				strcat(execcmd, "-, ");
			if (p_binary)
				strcat(execcmd, "-b ");
			else
				strcat(execcmd, "-a ");
			if (tty_not_char_special || (shm->ttyuse == TTYUSE_FORCE_SIMPLE))
				strcat(execcmd, "-_ ");
			strcat(execcmd, p_filelist);
			break;

		case ECUSZ_Z:
			sprintf(execcmd, "%s/ecusz -Z -@ %d -. %d ",
				eculibdir, shm->Lbitrate, shm->Liofd);
			strcat(execcmd, bottom_label);
			if (p_zwindwsz)
				sprintf(&execcmd[strlen(execcmd)], " -w %u ", p_zwindwsz);
#ifdef CFG_TelnetOption
			if (shm->Ltelnet)
				strcat(execcmd, "-T ");
#endif
			if (protocol_log_packets)
				strcat(execcmd, "-, ");
			if (p_overwrite)
				strcat(execcmd, "-y ");
			else
				strcat(execcmd, "-p ");
			if (p_binary)
				strcat(execcmd, "-b ");
			else
				strcat(execcmd, "-a ");
			if (p_xfernew)
				strcat(execcmd, "-n ");	/* overrides -y/-p choice earlier */
			if (p_resume)
				strcat(execcmd, "-r ");
			if (p_zctlesc)
				strcat(execcmd, "-e ");
			if (p_sendfull)
				strcat(execcmd, "-f ");
			if (tty_not_char_special || (shm->ttyuse == TTYUSE_FORCE_SIMPLE))
				strcat(execcmd, "-_ ");
			strcat(execcmd, p_filelist);
			break;

		case CKERMIT:		 /* flexible kermit filename */
#ifdef WHT
			if (ckufnm = find_executable("ck5a"))	/* private alpha version */
				;
#else
			if (ckufnm = (char *)0)	/* we MEAN to use '=' here */
				;
#endif
			else if (ckufnm = find_executable("kermit"))
				;
			else if (ckufnm = find_executable("ckermit"))
				;			 /* our old 286 4E hack (and alternate name
							  * used by many) */
			else
			{
				ff(se, "Cannot find C-Kermit ('kermit' or 'ckermit')\r\n");
				last_child_wait_status = -1;
				goto EXIT;
			}

			sprintf(execcmd, "%s -l %d -b %u -p %c%s%s -s %s",
				ckufnm,		 /* kermit flexible name */
				shm->Liofd,
				shm->Lbitrate,
				(shm->Lparity) ? shm->Lparity : 'n',
				(p_binary) ? " -i" : "",
				(p_overwrite) ? "" : " -w",
				p_filelist);
			break;

		default:
			pprintf("logic error in send_files_to_remote xfertype=%d\n",
				xfertype);
			errno = -1;
			termecu(TERMECU_LOGIC_ERROR);
			/* NOTREACHED */
	}

	file_xfer_start();

	if (!find_shell_chars(execcmd))
		exec_cmd(execcmd);
	else
	{
		char *expcmd;

		if (expand_wildcard_list(execcmd, &expcmd))
		{
			ff(se, "No files match\r\n");
			return;
		}
		exec_cmd(expcmd);
		free(expcmd);
	}

  EXIT:
	lreset_ksr();			 /* ensure line termio back to our config */
	ttymode(old_ttymode);	 /* xfer prog may screw up tty too */

	switch (xfertype)
	{
		case ECUSZ_X:
		case ECUSZ_Y:
		case ECUSZ_Z:
			xfertype = 1;	 /* was ecusz */
			break;
		default:
			xfertype = 0;
			break;
	}

	tcap_cursor(SNDW_TLY + xfrw_lines + 6, 0);
	if (xfertype)			 /* ecu knowledgable */
		report_send_status();
	else
	{
		tcap_stand_out();
		ff(se, " transfer status %04x ", last_child_wait_status);
		tcap_stand_end();
		ff(se, "\r\n");
	}

	ff(se, "\r\n");

	file_xfer_done_bell();
	start_rcvr_process(1);

}							 /* end of send_files_to_remote */

/*+-------------------------------------------------------------------------
	receive_files_from_remote(argc,argv)

  arg[0] = "rk", "rx", "ry", or "rz"
  also used by "automatic rz"

  returns wait_status from executed xfer program
--------------------------------------------------------------------------*/
int
receive_files_from_remote(argc, argv)
int argc;
char **argv;
{
	int itmp;
	UINT delim;
	char execcmd[256];
	char bottom_label[64];
	char *ckufnm;
	int old_ttymode = get_ttymode();
	int restart_rcvr = need_rcvr_restart();
	int rtn = -1;

	if (!argc)				 /* this should never happen, but ... */
	{
		pputs("LOGIC ERROR: no args to receive_files_from_remote()\n");
		return (-1);
	}

	sprintf(bottom_label, "-C 'Connected to %s' ",
		(shm->Llogical[0]) ? shm->Llogical : "?");

	switch (to_lower(*(argv[0] + 1)))
	{
		case 'k':
			xfertype = CKERMIT;
			break;
		case 'x':
			xfertype = ECURZ_X;
			break;
		case 'y':
			xfertype = ECURZ_Y;
			break;
		case 'z':
			xfertype = ECURZ_Z;
			break;
		default:
			pputs("LOGIC ERROR: receive command invalid\n");
			return (-1);
	}

	if (xfertype == ECURZ_X)
	{
		char xfilenam[128];

		if (restart_rcvr)
			kill_rcvr_process(SIGUSR1);
		sprintf(execcmd, "%s/ecurz -X -@ %d -. %d -c ",
			eculibdir, shm->Lbitrate, shm->Liofd);
		strcat(execcmd, bottom_label);
		if (protocol_log_packets)
			strcat(execcmd, "-, ");

		if (argc > 1)
		{
			strncpy(xfilenam, argv[1], sizeof(xfilenam) - 1);
			xfilenam[sizeof(xfilenam) - 1] = 0;
		}
		else
		{
			ff(se, "    File name to receive via XMODEM/CRC:  ");
			ttygets(xfilenam, sizeof(xfilenam), TG_CRLF, &delim, (int *)0);
			if (!xfilenam[0] || (delim == ESC))
			{
				ff(se, "transfer abandoned\r\n");
				if (restart_rcvr)
					start_rcvr_process(1);
				return (0);
			}
		}
		strcat(execcmd, "-b ");	/* xmodem binary */
		if (tty_not_char_special || (shm->ttyuse == TTYUSE_FORCE_SIMPLE))
			strcat(execcmd, "-_ ");
		strcat(execcmd, xfilenam);
		file_xfer_start();
		rtn = exec_cmd(execcmd);
	}
	else if (xfertype == ECURZ_Y)
	{
		ff(se, "\r\n");
		if (restart_rcvr)
			kill_rcvr_process(SIGUSR1);
		sprintf(execcmd, "%s/ecurz -Y -@ %d -. %d -y ",
			eculibdir, shm->Lbitrate, shm->Liofd);
		strcat(execcmd, bottom_label);
		if (protocol_log_packets)
			strcat(execcmd, "-, ");
		if (tty_not_char_special || (shm->ttyuse == TTYUSE_FORCE_SIMPLE))
			strcat(execcmd, "-_ ");
		file_xfer_start();
		rtn = exec_cmd(execcmd);
	}
	else if (xfertype == ECURZ_Z)
	{
		int dash_C_seen = 0;

		ff(se, "\r\n");
		if (restart_rcvr)
			kill_rcvr_process(SIGUSR1);
		sprintf(execcmd, "%s/ecurz -Z -@ %d -. %d ",
			eculibdir, shm->Lbitrate, shm->Liofd);
		if (protocol_log_packets)
			strcat(execcmd, "-, ");
#ifdef CFG_TelnetOption
		if (shm->Ltelnet)
			strcat(execcmd, "-e -T ");	/* escape ctl+telnet */
#endif
		if (tty_not_char_special || (shm->ttyuse == TTYUSE_FORCE_SIMPLE))
			strcat(execcmd, "-_ ");
		for (itmp = 1; itmp < argc; itmp++)
		{
			dash_C_seen |= !strcmp(argv[itmp], "-C");
			sprintf(execcmd + strlen(execcmd), "%s ", argv[itmp]);
		}
		if (!dash_C_seen)
			strcat(execcmd, bottom_label);
		file_xfer_start();
		rtn = exec_cmd(execcmd);
	}
	else if (xfertype == CKERMIT)
	{
		ckufnm = 0;
#ifdef WHT
		if (ckufnm = find_executable("ck5a"))	/* private alpha version */
			;
		else
#endif /* WHT */
		if (ckufnm = find_executable("kermit"))
			;
		else if (ckufnm = find_executable("ckermit"))
			;				 /* our old 286 4E hack (and alternate name
							  * used by many) */
		else
		{
			ff(se, "Cannot find C-Kermit ('kermit' or 'ckermit')\r\n");
			last_child_wait_status = 255 << 8;
			return (-1);	 /* <<=================================== */
		}
		if (restart_rcvr)
			kill_rcvr_process(SIGUSR1);
		sprintf(execcmd, "%s -r -e 512 -l %d -b %d -p %c",
			ckufnm, shm->Liofd, shm->Lbitrate,
			(shm->Lparity) ? shm->Lparity : 'n');
#if 0
		ff(se, "    Translate CR/LF to NL (y,n)? ");
		switch (itmp = to_lower(ttygetc(0)))
		{
			case 'y':
				strcat(execcmd, " -i");
				break;
			case 'n':
				break;
			default:
				pputs("transfer abandoned\n");
				if (restart_rcvr)
					start_rcvr_process(1);
				return (-1);
		}
		ff(se, "%s\r\n", (itmp == 'a') ? "yes" : "no");
#endif
		file_xfer_start();
		rtn = exec_cmd(execcmd);
	}
	lreset_ksr();			 /* ensure line termio back to our config */
	ttymode(old_ttymode);	 /* xfer prog may screw up tty too */
	pputs("\n\n");
	file_xfer_done_bell();
	Nap(20L);
	if (restart_rcvr)
		start_rcvr_process(1);
	return (rtn);

}							 /* end of receive_files_from_remote */

/* end of ecuxfer.c */
/* vi: set tabstop=4 shiftwidth=4: */
