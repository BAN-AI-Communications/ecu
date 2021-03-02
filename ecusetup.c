/*+-------------------------------------------------------------------------
	ecusetup.c -- ecu visual "argv"
	wht@wht.net

  0000000000111111111122222222223333333333444444444455555555556666
  0123456789012345678901234567890123456789012345678901234567890123
00.--[ ecu rev ]-------------------------------------------------.
01|                                                              |
02|  Destination   .......................................       |
03|    Phone/host    ....................                        |
04|    Description   ........................................    |
05|                                                              |
06|  tty: /dev/........................   (opened)               |
07|                                                              |
08|  duplex: .  baud: ...... parity: . (data bits .)             |
09|                                                              |
10|                                                              |
11| TAB:next END:proceed ^C: cmd mode ^D:phone dir ESC:quit ecu  |
12`--------------------------------------------------------------'

  Defined functions:
	setup_display_bitrate()
	setup_display_name()
	setup_display_screen(write_lits)
	setup_display_single_char()
	setup_display_tty()
	setup_line_open()
	setup_screen(argv_logical)
	setw_bot_msg(msg)
	setw_err_msg(msg)
	setw_get_single(permissible_set)
	setw_msg(msg, y, fillch, last_msglen)
	strUINT(str, delim)

  All readers cannot be leaders, but all leaders must be readers.
  - Harry Truman

--------------------------------------------------------------------------*/
/*+:EDITS:*/
/*:04-26-2000-11:15-wht@bob-RELEASE 4.42 */
/*:01-20-1998-17:08-wht@kepler-remove space pad around "TAB: ..." */
/*:05-12-1997-17:52-wht@kepler-use H instead of E for half duplex display */
/*:01-24-1997-02:37-wht@yuriatin-SOURCE RELEASE 4.00 */
/*:09-22-1996-19:22-root@yuriatin-tty field from 8 to 20 characters */
/*:09-11-1996-20:00-wht@yuriatin-3.48-major telnet,curses,structural overhaul */
/*:09-10-1996-03:12-wht@yuriatin-drop telnet_open kludge */
/*:09-07-1996-16:39-wht@kepler-move WINCH definition to pc_scr.h */
/*:08-11-1996-02:10-wht@kepler-rename ecu_log_event to logevent */
/*:12-12-1995-15:34-wht@kepler-allow telnet open to go through setup screen */
/*:12-06-1995-13:30-wht@n4hgf-termecu w/errno -1 consideration */
/*:11-23-1995-11:20-wht@kepler-source control 3.37 for tsx-11 */
/*:11-14-1995-10:23-wht@kepler-3.37.80-source control point: SOCKETS */
/*:11-13-1995-12:11-wht@kepler-run ^D without opening line */
/*:11-03-1995-18:26-wht@wwtp1-setup_display_tty corrupted stack a bit */
/*:11-03-1995-16:54-wht@wwtp1-use CFG_TelnetOption */
/*:10-19-1995-03:38-wht@kepler-first crude Ltelnet access */
/*:10-14-1995-16:29-wht@kepler-use valid_baud_string */
/*:09-17-1995-16:28-wht@kepler-remove obsolete #if 0 code */
/*:02-07-1995-16:06-wht@n4hgf-unANSIfy strUINT */
/*:01-12-1995-15:19-wht@n4hgf-apply Andrew Chernov 8-bit clean+FreeBSD patch */
/*:05-17-1994-20:29-wht@n4hgf-terminate setw_nondelim_list */
/*:05-04-1994-04:39-wht@n4hgf-ECU release 3.30 */
/*:12-02-1993-14:34-wht@n4hgf-line lock retries from 8 to 20 */
/*:11-12-1993-11:00-wht@n4hgf-Linux changes by bob@vancouver.zadall.com */
/*:05-29-1993-20:21-wht@n4hgf-change linst_err_text to LINST_text */
/*:04-12-1993-12:16-wht@n4hgf-if delim == ^D, do not parse input */
/*:09-10-1992-13:58-wht@n4hgf-ECU release 3.20 */
/*:09-05-1992-14:49-wht@n4hgf-parity field was one to the left of proper pos */
/*:08-22-1992-15:38-wht@n4hgf-ECU release 3.20 BETA */
/*:04-28-1992-01:34-wht@n4hgf-default tty in tty prompt had slash */
/*:04-24-1992-21:59-wht@n4hgf-more SCO tty name normalizing */
/*:08-28-1991-14:07-wht@n4hgf2-SVR4 cleanup by aega84!lh */
/*:08-25-1991-14:39-wht@n4hgf-SVR4 port thanks to aega84!lh */
/*:08-12-1991-00:58-wht@n4hgf-ISC tty names */
/*:07-25-1991-12:56-wht@n4hgf-ECU release 3.10 */
/*:07-17-1991-07:04-wht@n4hgf-avoid SCO UNIX nap bug */
/*:06-05-1991-18:07-wht@n4hgf-rework */
/*:04-27-1991-01:52-wht@n4hgf-overhaul revision numbers */
/*:08-14-1990-20:40-wht@n4hgf-ecu3.00-flush old edit history */

#include "ecucurses.h"

#define STDIO_H_INCLUDED
#define OMIT_TERMIO_REFERENCES
#include "ecu.h"
#include "ecukey.h"
#include "ecuxkey.h"
#include "ecupde.h"
#include "pc_scr.h"

PDE *logical_telno_to_pde();

#define SETW_LINES	13
#ifdef CFG_TelnetOption
#define SETW_COLS	74
#else
#define SETW_COLS	64
#endif
#define SETW_TLY	1
#define SETW_TLX	((80 - SETW_COLS) / 2)

#define NAME_Y		2
#define NAME_X		17
#define NAME_LEN	DESTREF_LEN
#define NAME_LX		3

#define PHNUM_Y		3
#define PHNUM_X		19
#define PHNUM_LEN	DESTREF_LEN
#define PHNUM_LX	5

#define DESCR_Y		4
#define DESCR_X		19
#define DESCR_LEN	PDE_DESCR_LEN
#define DESCR_LX	5

#define TTY_Y		6
#define TTY_X		13
#define TTY_LEN		20
#define TTY_LX		3

#define TTYOPN_LY	6
#define TTYOPN_LX	36

#define DPX_Y		8
#define DPX_X		11
#define DPX_LX		3

#define BAUD_Y		8
#define BAUD_X		20
#define BAUD_LEN        6
#define BAUD_LX		14

#define PAR_Y		8
#define PAR_X		35
#define PAR_LX		27

#define DB_Y		8
#define DB_X		48
#define DB_LX		37
#define DB_LX2		49

extern char *revstr;		 /* ecunumrev.c */
extern char errmsg[];
extern char *valid_baud_string;

WINDOW *setw;

#define SETW_MSG_LEFTX 2
#define SETW_MSG_MAXLEN (SETW_COLS - SETW_MSG_LEFTX - 6)
#define SETW_MSG_BOT_Y  (SETW_LINES - 1)
#define SETW_MSG_ERR_Y  (SETW_LINES - 3)

/*+-------------------------------------------------------------------------
	strUINT(str,delim) - find unit in array of uints
--------------------------------------------------------------------------*/
static int
strUINT(str, delim)
UINT *str;
UINT delim;
{
	while (*str)
	{
		if (*str++ == delim)
			return (1);
	}
	return (0);
}							 /* end of strUINT */

/*+-------------------------------------------------------------------------
	setw_msg(msg,y,fillch) - setw message display workhorse
--------------------------------------------------------------------------*/
void
setw_msg(msg, y, fillch, last_msglen)
char *msg;
int y;
WINCH fillch;
int *last_msglen;
{
	int itmp;
	int itmp2;
	char msg2[80];

	if (!*last_msglen && !strlen(msg))
		return;

	wmove(setw, y, SETW_MSG_LEFTX);

	if ((itmp = strlen(msg)) == 0)
	{
		itmp2 = *last_msglen + 2;
		for (itmp = 0; itmp < itmp2; itmp++)
			waddch(setw, fillch);
		*last_msglen = 0;
	}
	else
	{
		waddch(setw, ' ');
		if (itmp > SETW_MSG_MAXLEN)
		{
			strncpy(msg2, msg, SETW_MSG_MAXLEN + 1);
			msg2[SETW_MSG_MAXLEN + 1] = 0;
			waddstr(setw, msg2);
			itmp = strlen(msg2);
		}
		else
		{
			waddstr(setw, msg);
			itmp = strlen(msg);
		}
		waddch(setw, ' ');
		if ((itmp2 = *last_msglen - itmp) > 0)
		{
			while (itmp2--)
				waddch(setw, fillch);
		}
		*last_msglen = itmp; /* remember last message length */
	}
	wrefresh(setw);
}							 /* end of setw_msg */

/*+-------------------------------------------------------------------------
	setw_bot_msg(msg) - msg in bottom border of setw
--------------------------------------------------------------------------*/
void
setw_bot_msg(msg)
char *msg;
{
	static int last_msglen = 0;

	setw_msg(msg, SETW_MSG_BOT_Y, sHR, &last_msglen);
}							 /* end of setw_bot_msg */

/*+-------------------------------------------------------------------------
	setw_err_msg(msg) - error message
--------------------------------------------------------------------------*/
void
setw_err_msg(msg)
char *msg;
{
	static int last_msglen = 0;

	setw_msg(msg, SETW_MSG_ERR_Y, ' ', &last_msglen);
}							 /* end of setw_err_msg */

/*+-------------------------------------------------------------------------
	setup_display_name() - display name,num/host/description
--------------------------------------------------------------------------*/
void
setup_display_name()
{
	clear_area(setw, NAME_Y, NAME_X, NAME_LEN);
	waddstr(setw, shm->Llogical);

	wmove(setw, PHNUM_Y, PHNUM_LX);
	if (shm->Ltelno[0])
		waddstr(setw, "Phone/Host");
	else
		waddstr(setw, "          ");
	clear_area(setw, PHNUM_Y, PHNUM_X, PHNUM_LEN);
	waddstr(setw, shm->Ltelno);

	wmove(setw, DESCR_Y, DESCR_LX);
	if (shm->Ldescr[0])
		waddstr(setw, "Description");
	else
		waddstr(setw, "           ");
	clear_area(setw, DESCR_Y, DESCR_X, DESCR_LEN);
	waddstr(setw, shm->Ldescr);

}							 /* end of setup_display_name */

/*+-------------------------------------------------------------------------
	setup_display_tty() - show device
--------------------------------------------------------------------------*/
void
setup_display_tty()
{
	char s[TTY_LEN + 1];

	strncpy(s, shm->Lline + 5, TTY_LEN);
	s[TTY_LEN] = 0;
	clear_area(setw, TTY_Y, TTY_X, TTY_LEN);
	waddstr(setw, s);
	wmove(setw, TTYOPN_LY, TTYOPN_LX);
	if (shm->Liofd >= 0)
		waddstr(setw, "(opened)");
	else
		waddstr(setw, "        ");
	wrefresh(setw);

}							 /* end of setup_display_tty */

/*+-------------------------------------------------------------------------
	setup_display_single_char() - display single character field
--------------------------------------------------------------------------*/
void
setup_display_single_char()
{
	wmove(setw, DPX_Y, DPX_X);
	waddch(setw, (shm->Lfull_duplex) ? 'F' : 'H');
	wmove(setw, PAR_Y, PAR_X);
	waddch(setw, (shm->Lparity) ? to_upper(shm->Lparity) : 'N');
	wmove(setw, DB_Y, DB_X);
	waddch(setw, (shm->Lparity) ? '7' : '8');

}							 /* end of setup_display_single_char */

/*+-------------------------------------------------------------------------
	setup_display_bitrate() - show bit rate
--------------------------------------------------------------------------*/
void
setup_display_bitrate()
{
	char s8[8];

	clear_area(setw, BAUD_Y, BAUD_X, BAUD_LEN);
	sprintf(s8, "%u", shm->Lbitrate);
	waddstr(setw, s8);

}							 /* end of setup_display_bitrate */

/*+-------------------------------------------------------------------------
	setup_display_screen(write_lits) - refersh screen
--------------------------------------------------------------------------*/
void
setup_display_screen(write_lits)
int write_lits;
{

	if (write_lits)
	{
		wmove(setw, NAME_Y, NAME_LX);
		waddstr(setw, "Destination");
		wmove(setw, TTY_Y, TTY_LX);
		waddstr(setw, "tty: /dev/");
		wmove(setw, DPX_Y, DPX_LX);
		waddstr(setw, "duplex:");
		wmove(setw, BAUD_Y, BAUD_LX);
		waddstr(setw, "baud:");
		wmove(setw, PAR_Y, PAR_LX);
		waddstr(setw, "parity:");
		wmove(setw, DB_Y, DB_LX);
		waddstr(setw, "(data bits");
		wmove(setw, DB_Y, DB_LX2);
		waddch(setw, ')');
		wstandout(setw);
		clear_area_char(setw, SETW_LINES - 2, 1, SETW_COLS - 2, ' ');
		wmove(setw, SETW_LINES - 2, 4);
		waddstr(setw,
			"TAB:next END:proceed ^C: cmd mode ^D:phone dir ESC:quit ecu");
		wstandend(setw);
	}
	setup_display_name();
	setup_display_tty();
	setup_display_single_char();
	setup_display_bitrate();
	wrefresh(setw);
}							 /* end of setup_display_screen */

/*+-------------------------------------------------------------------------
	setup_line_open() - open line in setup context
--------------------------------------------------------------------------*/
int
setup_line_open()
{
	int itmp;
	int retries = 20;
	char *LINST_text();
	char *cp;
	char msg[80];
	long wait_msec;
	int displayed_single_char_exit = 0;

	while (itmp = lopen())
	{
		if (retries)
		{
			if ((itmp != LINST_ENABLED_IN_USE) &&
				(itmp != LINST_DIALOUT_IN_USE) &&
				(itmp < 0))
			{
				goto FAIL;
			}
			cp = LINST_text(itmp);
			sprintf(msg, "%s - waiting %d sec", cp, retries);
			cp = msg;
			if (!displayed_single_char_exit)
			{
				setw_bot_msg("Press any key to skip retries");
				displayed_single_char_exit = 1;
			}
		}
		else
		{
		  FAIL:
			retries = 0;
			cp = LINST_text(itmp);
			ring_bell();
		}
		setw_err_msg(cp);
		if (!retries--)
			break;
		wait_msec = 1000L;
		while (wait_msec > 0)
		{
			if (ttyrdchk())
			{
				(void)ttygetc(1);
				goto FAIL;
			}
			wait_msec -= Nap(100L);
		}
		setw_err_msg("");
	}

#ifdef SHARE_DEBUG
	{
		char s256[256];

		sprintf(s256, "SETUP-OPENED line=%s fd=%d", shm->Lline, shm->Liofd);
		logevent((int)xmtr_pid, s256);
	}
#endif

	setup_display_tty();
	lopen_error_reset();	 /* clear static error area */
	return (itmp);

}							 /* end of setup_line_open */

/*+-------------------------------------------------------------------------
	setw_get_single(permissible_set) - get a single char in a set

  assumes cursor is already positioned
--------------------------------------------------------------------------*/
UINT
setw_get_single(permissible_set)
UINT *permissible_set;
{
	UINT itmp;
	static UINT setw_delim_list[] =
	{
		CRET, NL, CTL_B, CTL_C, CTL_D, TAB, ESC, CTL_L, CTL_R,
		XFend, XFcurup, XFcurdn, 0
	};

	itmp = winget_single(setw, permissible_set, setw_delim_list);
	if ((itmp & 0x1FF) == CRET)
		itmp = NL | 0x1000;
	return (itmp);
}							 /* end of setw_get_single */

/*+-------------------------------------------------------------------------
	setup_screen(argv_logical) - manage setup screen
--------------------------------------------------------------------------*/
void
setup_screen(argv_logical)
char *argv_logical;
{
	int itmp;
	int input_state = 0;
	char s128[128];
	char *cp;
	char logical[NAME_LEN + 1];
	int done = 0;
	UINT baud;
	UINT delim;				 /* important to be unsigned to avoid sign
							  * extension */
	PDE *tpde;
	WINDOW *window_create();
	static UINT use_input_delim[] =
		{TAB, NL, XFcurdn, XFcurup, XFend, CTL_C, CTL_D, 0};
	static UINT duplex_choices[] =
		{'f', 'h', 0};
	static UINT parity_choices[] =
		{'n', 'e', 'o', 0};

	windows_start();
	sprintf(s128, "ecu %s", revstr);
	setw = window_create(s128, -3, SETW_TLY, SETW_TLX, SETW_LINES, SETW_COLS);
	shm->Llogical[0] = 0;
	logical[0] = 0;
	setup_display_screen(1);

#ifdef TURNKEY
	if (!argv_logical)
	{
		done = 1;
		delim = CTL_D;
	}
#endif /* TURNKEY */

  REENTER_INPUT_LOOP:
	while (!done)
	{
		wrefresh(setw);
		switch (input_state)
		{
			case 0:
				if (argv_logical)
				{
					itmp = 0;/* 'ecu -' means dont dial */
					if (strcmp(argv_logical, "-"))	/* if not "-" */
					{
						strncpy(s128, argv_logical, NAME_LEN + 1);
						s128[NAME_LEN + 1] = 0;
						itmp = strlen(s128);
					}
					argv_logical = (char *)0;
					delim = XFend;
				}
				else
				{
					if (logical[0])
						strcpy(s128, logical);
					setw_bot_msg(
						"logical phone directory entry, phone number or empty");
					itmp = wingets(setw, NAME_Y, NAME_X, s128,
						NAME_LEN + 1, &delim,
						(logical[0] != 0), (int *)0);
					setw_err_msg("");
				}
				/* if going to command mode or to directory */
				if ((delim == CTL_C) || (delim == CTL_D))
					break;
				if (strUINT(use_input_delim, delim))
				{
					strcpy(logical, s128);
					if ((tpde = logical_telno_to_pde(logical)) &&
						!copy_pde_to_Lvariables(tpde, 1))
					{
						strcpy(logical, tpde->logical);
						setup_display_screen(0);
						break;
					}
					else
					{
						setw_err_msg(errmsg);
						setup_display_screen(0);
						ring_bell();
						argv_logical = (char *)0;
						continue;
					}
				}
				setup_display_name();
				break;

			case 1:
				cp = strchr(CFG_DefaultTty, '/');
				if (!cp)	 /* cover unlikely bug */
					cp = "/ttya";
				cp++;
				sprintf(s128, "comm line: i.e., %s", cp);
				setw_bot_msg(s128);
				setup_display_tty();
				strcpy(s128, "/dev/");
				strcpy(s128, &shm->Lline[5]);
				itmp = wingets(setw, TTY_Y, TTY_X, s128, TTY_LEN + 1, &delim,
					1, (int *)0);
				setw_err_msg("");
				/* if going to command mode or to directory */
				if ((delim == CTL_C) || (delim == CTL_D))
					break;
				if (strUINT(use_input_delim, delim))
				{
					strcpy(shm->Lline, "/dev/");
					strcpy(&shm->Lline[5], s128);
				}
				setup_display_tty();
				break;

			case 2:
				setw_bot_msg("duplex F:full H:half");
				wmove(setw, DPX_Y, DPX_X);
				wrefresh(setw);
				delim = NL;
				switch (itmp = setw_get_single(duplex_choices))
				{
					case 0:
					case 1:
						shm->Lfull_duplex = itmp;
						break;
					default:
						delim = itmp & 0x1FF;
						break;
				}
				break;

			case 3:		 /* baud */
				setw_bot_msg(valid_baud_string);

			  CASE_3:
				sprintf(s128, "%u", shm->Lbitrate);
				itmp = wingets(setw, BAUD_Y, BAUD_X, s128, BAUD_LEN + 1, &delim,
					1, (int *)0);
				/* if going to command mode or to directory */
				if ((delim == CTL_C) || (delim == CTL_D))
					break;
				if (strUINT(use_input_delim, delim))
				{
					if (valid_baud(baud = atoi(s128)) == -1)
					{
						setup_display_bitrate();
						ring_bell();
						goto CASE_3;
					}
					shm->Lbitrate = baud;
				}
				setup_display_bitrate();
				break;

			case 4:
				setw_bot_msg("parity: N:none E:even O:odd");
				wmove(setw, PAR_Y, PAR_X);
				wrefresh(setw);
				delim = NL;
				switch (itmp = setw_get_single(parity_choices))
				{
					case 0:
						shm->Lparity = 0;
						break;
					case 1:
						shm->Lparity = 'e';
						break;
					case 2:
						shm->Lparity = 'o';
						break;
					default:
						delim = itmp & 0x1FF;
						break;
				}
				wmove(setw, DB_Y, DB_X);
				waddch(setw, (shm->Lparity) ? '7' : '8');
				break;

		}

#if 0
		if (argv_logical && (delim != CTL_C) && (delim != CTL_D))
			break;
#endif

		switch (delim)
		{
			case XFcurup:
			case CTL_B:
				if (input_state)
					input_state--;
				else
					input_state = 6;
				break;

			case XFcurdn:
			case TAB:
			case NL:
				input_state++;
				input_state %= 5;
				break;

			case ESC:
				if (shm->Liofd >= 0)
					lclose();
				setw_bot_msg("");
				setup_display_tty();
				termecu(TERMECU_OK);
				break;

			case CTL_L:
			case CTL_R:
				tcap_clear_screen();
				touchwin(stdscr);
				wrefresh(stdscr);
				setup_display_screen(1);
				touchwin(setw);
				wrefresh(setw);
				break;

			case XFend:
			case CTL_C:
			case CTL_D:
				done = 1;
				break;
		}
	}

	if ((shm->Liofd < 0) && (delim != CTL_D) && (delim != CTL_C)
#ifdef CFG_TelnetOption
		&& !shm->Ltelnet
#endif
		)
	{
		wmove(setw, TTY_Y, TTY_X);
		wrefresh(setw);
		if (setup_line_open())
		{
			done = 0;
			input_state = 1;
			argv_logical = (char *)0;
			goto REENTER_INPUT_LOOP;
		}
	}

	wstandout(setw);
	clear_area_char(setw, SETW_LINES - 2, 1, SETW_COLS - 2, '-');
	wmove(setw, SETW_LINES - 2, 8);
	waddstr(setw,
		" Press HOME then 'help' for further assistance ");
	wstandend(setw);
	setw_bot_msg("");
	wrefresh(setw);
	delwin(setw);
	windows_end(0);
	ttymode(1);
	tcap_cursor(SETW_TLY + SETW_LINES + 2, 0);
	shm->rcvr_pid = -2;
	if (delim == CTL_D)
		phdir_manager();
	else if (logical[0])
	{
		tpde = logical_telno_to_pde(logical);	/* error return not likely
												 * now */
		if (!tpde)
		{
			ff(se, "re-call of logical_telno_to_pde '%s' failed\r\n", logical);
			errno = -1;
			termecu(TERMECU_LOGIC_ERROR);
		}

		pde_dial(tpde);
	}
	else
		start_rcvr_process(1);

#ifdef SHARE_DEBUG
	{
		char s256[256];

		sprintf(s256, "SETUP-DONE line=%s fd=%d", shm->Lline, shm->Liofd);
		logevent((int)xmtr_pid, s256);
	}
#endif

}							 /* end of setup_screen */

/* vi: set tabstop=4 shiftwidth=4: */
