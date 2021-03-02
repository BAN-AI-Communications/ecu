/*+-------------------------------------------------------------------------
	ecuicmhelp.c -- help for interactive commands
	wht@wht.net

  Defined functions:
	help_category_menu()
	help_choose_cmd()
	help_cmd_line_setup(prompt)
	help_display_on_stderr(cmd)
	help_interactively()
	help_right_column()
	help_search_pcmds(cmd)
	help_show_category(category)
	icmd_help(narg, arg)

  Whenever the literary German dives into a sentence, that is the
  last you are going to see of him until he emerges on the other
  side of his Atlantic with his verb in his mouth.  -- Mark Twain

--------------------------------------------------------------------------*/
/*+:EDITS:*/
/*:04-26-2000-11:15-wht@bob-RELEASE 4.42 */
/*:03-16-1997-02:45-rll@felton.felton.ca.us-Make nice boxes for SCO */
/*:01-24-1997-02:37-wht@yuriatin-SOURCE RELEASE 4.00 */
/*:09-11-1996-20:00-wht@yuriatin-3.48-major telnet,curses,structural overhaul */
/*:07-24-1996-21:37-wht@n4hgf-no more wvline/whline */
/*:11-27-1995-11:50-wht@kepler-if rcvr_ansi_filter off, cr/lf at end of help */
/*:11-23-1995-11:20-wht@kepler-source control 3.37 for tsx-11 */
/*:11-14-1995-10:23-wht@kepler-3.37.80-source control point: SOCKETS */
/*:01-12-1995-15:19-wht@n4hgf-apply Andrew Chernov 8-bit clean+FreeBSD patch */
/*:05-04-1994-04:38-wht@n4hgf-ECU release 3.30 */
/*:09-10-1992-13:58-wht@n4hgf-ECU release 3.20 */
/*:08-22-1992-15:38-wht@n4hgf-ECU release 3.20 BETA */
/*:01-11-1992-16:01-wht@n4hgf-bug in help + F1 reverts to cat menu */
/*:08-25-1991-14:39-wht@n4hgf-SVR4 port thanks to aega84!lh */
/*:08-01-1991-03:52-wht@n4hgf-when editing string, set cursor to end */
/*:07-25-1991-12:56-wht@n4hgf-ECU release 3.10 */
/*:08-14-1990-20:40-wht@n4hgf-ecu3.00-flush old edit history */

#include "ecucurses.h"
#define OMIT_TERMIO_REFERENCES
#define STDIO_H_INCLUDED
#include "ecu.h"
#define NEED_P_CMD
#include "ecucmd.h"
#include "ecukey.h"
#include "ecuxkey.h"
#include "pc_scr.h"

#define PDAT	"ecuhelp.data"

void help_right_column();

static long start_pos[TOKEN_QUAN];
static int start_pos_has_been_read = 0;
static char ecuhelpdata_name[256] = "";
static FILE *fpdat;			 /* help data file */
static int right_column = 0; /* right column for help_interactively */
static int longest_cmd = 0;
static int longest_descr = 0;

/*+-------------------------------------------------------------------------
	help_search_pcmds(cmd)
--------------------------------------------------------------------------*/
P_CMD *
help_search_pcmds(cmd)
char *cmd;
{
	P_CMD *cmd_list = icmd_cmds;

	while (cmd_list->token != -1)
	{
		if (minunique(cmd_list->cmd, cmd, cmd_list->min_ch))
			break;
		cmd_list++;
	}
	if (cmd_list->token == -1)
		return ((P_CMD *) 0);
	else
		return (cmd_list);

}							 /* end of help_search_pcmds */

/*+-------------------------------------------------------------------------
	help_display_on_stderr(cmd)
--------------------------------------------------------------------------*/
void
help_display_on_stderr(cmd)
char *cmd;
{
	int itmp;
	P_CMD *pcmd;
	char buf[128];

	if (!(pcmd = help_search_pcmds(cmd)))
	{
		ff(se, "'%s' is not a valid command\r\n", cmd);
		return;
	}

	if (!start_pos[pcmd->token])
	{
		ff(se, "no help available for '%s'\r\n", cmd);
		return;
	}

	fseek(fpdat, start_pos[pcmd->token], 0);
	ff(se, "\r\n");
	while (fgets(buf, sizeof(buf), fpdat))
	{
		itmp = strlen(buf);
		buf[--itmp] = 0;
		if (itmp == 0)
			break;
		ff(se, "%s\r\n", buf);
	}

}							 /* end of help_display_on_stderr */

/*+-------------------------------------------------------------------------
	help_right_column()
--------------------------------------------------------------------------*/
void
help_right_column()
{
	int itmp;
	P_CMD *pcmd = icmd_cmds;

	if (right_column)		 /* already bee thru here? */
		return;				 /* ... seems so */

	while (pcmd->token != -1)
	{
		if (!*pcmd->descr)
		{
			pcmd++;
			continue;
		}
		itmp = strlen(pcmd->cmd);
		if (itmp > longest_cmd)
			longest_cmd = itmp;
		itmp = strlen(pcmd->descr);
		if (itmp > longest_descr)
			longest_descr = itmp;
		pcmd++;
	}
	right_column = 1 + longest_cmd + 2 + longest_descr + 3;

}							 /* end of help_right_column */

/*+-------------------------------------------------------------------------
	help_cmd_line_setup(prompt)
--------------------------------------------------------------------------*/
void
help_cmd_line_setup(prompt)
char *prompt;
{
	int icol;
	int y;
	int x;

	touchwin(stdscr);
	wmove(stdscr, LINES - 1, 0);
	wstandout(stdscr);
	waddstr(stdscr, prompt);
	getyx(stdscr, y, x);
	for (icol = x; icol < COLS - 1; icol++)
		waddch(stdscr, ' ');
	wmove(stdscr, y, x);
	wstandend(stdscr);
	wrefresh(stdscr);
}							 /* end of help_cmd_line_setup */

/*+-------------------------------------------------------------------------
	help_choose_cmd() - choose command from category
--------------------------------------------------------------------------*/
char *
help_choose_cmd()
{
	int y;
	int x;
	static char cmd[15];
	UINT delim = 0;
	int wgpos = -1;
	int edit = 0;

	help_cmd_line_setup(
		"Enter command name (F1 for category menu, ESC to exit):  ");
	getyx(stdscr, y, x);
	wstandout(stdscr);
	while ((delim != ESC) && (delim != XF1) && (delim != NL))
	{
		wingets(stdscr, y, x, cmd, sizeof(cmd) - 1, &delim, edit, &wgpos);
		edit = 1;
	}
	wstandend(stdscr);
	if (delim == XF1)
		return ("");
	else if ((delim == ESC) || (!cmd[0]))
		return ((char *)0);
	else
		return (cmd);

}							 /* end of help_choose_cmd */

/*+-------------------------------------------------------------------------
	help_category_menu() - get user command category choice
--------------------------------------------------------------------------*/
int
help_category_menu()
{
	int itmp;
	int y;
	int x;
	char **cpptr;
	static UINT keylist[] =
	{'g', 'c', 't', 'p', ESC, 0};
	static UINT empty_list[] =
	{0};
	static char *list[] =
	{
		"g   - general commands",
		"c   - communications-related commands",
		"t   - transfer-related commands",
		"p   - procedure-related commands",
		"Esc - exit help",
		(char *)0
	};

	tcap_clear_screen();
	wclear(stdscr);
	wmove(stdscr, 0, 0);
	wstandout(stdscr);
	waddstr(stdscr, "-- Interactive Command Help Menu ");
	getyx(stdscr, y, x);
	for (itmp = x; itmp < COLS - 1; itmp++)
		waddch(stdscr, '-');
	wstandend(stdscr);

	itmp = 2;
	cpptr = list;
	while (*cpptr)
	{
		wmove(stdscr, itmp++, 4);
		waddstr(stdscr, *cpptr++);
	}
	wmove(stdscr, 9, 4);
	waddstr(stdscr, "---- choose a category -------");
	return (winget_single(stdscr, empty_list, keylist));

}							 /* end of help_category_menu */

/*+-------------------------------------------------------------------------
	help_show_category(category) - display category and let user choose
--------------------------------------------------------------------------*/
int
help_show_category(category)
int category;
{
	int itmp;
	P_CMD *pcmd;
	int y = 1;
	int x = 0;
	short cmdclass = 0;
	char s80[80];
	char *class_descr = "?";

	switch (category)
	{
		case 'g':
			cmdclass = ccG;
			class_descr = "General";
			break;
		case 'c':
			cmdclass = ccC;
			class_descr = "Communications";
			break;
		case 't':
			cmdclass = ccT;
			class_descr = "File Transfer";
			break;
		case 'p':
			cmdclass = ccP;
			class_descr = "Procedure Related";
			break;
	}

	tcap_clear_screen();
	wclear(stdscr);
	wmove(stdscr, 0, 0);
	wstandout(stdscr);
	waddstr(stdscr, "-- ");
	waddstr(stdscr, class_descr);
	waddstr(stdscr, " Commands ");
	getyx(stdscr, y, x);
	for (itmp = x; itmp < COLS - 1; itmp++)
		waddch(stdscr, '-');
	wstandend(stdscr);

	pcmd = icmd_cmds;
	y = 2;
	x = 0;
	wmove(stdscr, y, x);
	while (pcmd->token != -1)
	{
		if (!*pcmd->descr || (pcmd->cmdclass != cmdclass))
		{
			pcmd++;
			continue;
		}
		wmove(stdscr, y, x);
		strcpy(s80, pcmd->cmd);
		pad_zstr_to_len(s80, longest_cmd + 2);
		for (itmp = 0; itmp < pcmd->min_ch; itmp++)
			s80[itmp] = to_upper(s80[itmp]);
		waddstr(stdscr, s80);

		strcpy(s80, pcmd->descr);
		if (!x)
			pad_zstr_to_len(s80, longest_descr + 1);
		waddstr(stdscr, s80);

		if (!x)
#if defined(SVR4) || defined(SCO32v4) || defined(SCO32v5)
			waddch(stdscr, sVR);
#else
			waddch(stdscr, sVR & 0xFF);
#endif
		y++;
		if (y >= LINES - 3)
		{
			y = 2;
			x = right_column;
		}
		pcmd++;
	}
	wmove(stdscr, LINES - 2, 0);
	wstandout(stdscr);
	waddstr(stdscr,
		"Capitalized portion of listed command sufficient for command recognition");
	getyx(stdscr, y, x);
	for (itmp = x; itmp < COLS - 1; itmp++)
		waddch(stdscr, ' ');
	wstandend(stdscr);
	return (0);

}							 /* end of help_show_category */

/*+-------------------------------------------------------------------------
	help_interactively()
commands with null descriptions are "undocumented"
--------------------------------------------------------------------------*/
void
help_interactively()
{
	char *cp;
	int restart_rcvr = need_rcvr_restart();
	char category;

	kill_rcvr_process(SIGUSR1);

	windows_start();
	help_right_column();

  DISPLAY_MENU:
	if ((category = help_category_menu()) != ESC)
	{
		help_show_category(category);
		while (cp = help_choose_cmd())
		{
			if (!*cp)
				goto DISPLAY_MENU;
			wmove(stdscr, LINES - 1, 0);
			wclrtoeol(stdscr);
			wrefresh(stdscr);
			help_display_on_stderr(cp);
			ff(se, "\r\npress return:  ");
			ttygetc(1);
			help_show_category(category);
		}
	}
	wrefresh(stdscr);
	windows_end(1);
	if (shm->rcvr_ansi_filter)
		redisplay_rcvr_screen();
	else
	{
		tcap_curbotleft();
		ff(se, "\r\n");
	}
	if (restart_rcvr)
		start_rcvr_process(0);

}							 /* end of help_interactively */

/*+-------------------------------------------------------------------------
	icmd_help(narg,arg)
--------------------------------------------------------------------------*/
void
icmd_help(narg, arg)
int narg;
char **arg;
{
	char *cp;
	char *getenv();

	ff(se, "\r\n");
	if (!ecuhelpdata_name[0])
	{
		if (!(cp = getenv("ECUHELP")))
			sprintf(ecuhelpdata_name, "%s/%s", CFG_EcuLibDir, PDAT);
		else
			strcpy(ecuhelpdata_name, cp);
	}

	if (!(fpdat = fopen(ecuhelpdata_name, "r")))
	{
		pperror(ecuhelpdata_name);
		return;
	}

	if (!start_pos_has_been_read)
	{
		fread((char *)start_pos, sizeof(long), TOKEN_QUAN, fpdat);

		start_pos_has_been_read = 1;
	}

	if (narg > 1)
		help_display_on_stderr(arg[1]);
	else
		help_interactively();

	fclose(fpdat);
}							 /* end of icmd_help */

/* vi: set tabstop=4 shiftwidth=4: */
