/*+-------------------------------------------------------------------------
	ecuphdir.c -- visual phone dialer/directory editor
	wht@wht.net

  .---[ title ]------------modified-.<-- dirw "top line"
  |   stand out mode                |<-- dirw "header line"
  |                                 |<-- scrw first line
  |                                 |
  |                                 |
  |                                 |
  |                                 |<-- scrw last line
  +---------------------------------+<-- dirw bottom separator line
  |                                 |<-- dirw extra cmd prompt line
  |   stand out mode                |<-- dirw "cmd line"
  `---------------------------------'<-- dirw bottom line

  000000000011111111112222222222333333333344444444445555555555666666666671
  012345678901234567890123456789012345678901234567890123456789012345678901
 0.--[ entry nnnnn ]-----------------------------------------------------.
 1|                                                                      |
 2| telephone number  ___________________                                |
 3| device            __________                                         |
 4| bit rate         _____                                               |
 5| parity            _                                                  |
 6| description       ________________________________________           |
 7| debug level       _  (dialer -x value 0-9)                           |
 8| DCD watch         _                                                  |
 9| RTS/CTS flow ctl  _  (0=off,7=best,n=no change)                      |
10|                                                                      |
-2| <prompt>                                                             |
-1| <control key description>                                            |
 n`----------------------------------------------------------------------'

  Defined functions:
	check_curr_pde()
	dirw_bot_msg(msg)
	dirw_cmd_line_setup(prompt1, prompt2)
	dirw_display()
	dirw_display_config()
	dirw_display_phonedir_name()
	dirw_get_cmd()
	field_colon_protect(fieldstr)
	field_colon_restore(fieldstr)
	phdir_add_or_edit(tpde, edit)
	phdir_add_or_edit_read(prompt, y, buf, bufmax, delim)
	phdir_cmd_add(tpde)
	phdir_cmd_change_dir()
	phdir_cmd_down()
	phdir_cmd_find()
	phdir_cmd_mark(tpde)
	phdir_cmd_pgdn()
	phdir_cmd_pgup()
	phdir_cmd_remove()
	phdir_cmd_remove_oops()
	phdir_cmd_save()
	phdir_cmd_set_wait()
	phdir_cmd_unmark(tpde)
	phdir_cmd_unmark_all()
	phdir_cmd_up()
	phdir_dial_cycle()
	phdir_display(line, tpde, stand_out)
	phdir_display_logical(line, tpde, stand_out)
	phdir_list_add(tpde)
	phdir_list_erase()
	phdir_list_read()
	phdir_list_remove(tpde)
	phdir_list_save_if_dirty()
	phdir_list_search(logical, exact_flag)
	phdir_list_set_dirty(flag)
	phdir_manager()
	scrw_fill(tpde, curr_pde_line)
	scrw_fill_at(line_num, tpde, curr_pde_line)
	want_pd_create(name)

--------------------------------------------------------------------------*/
/*+:EDITS:*/
/*:04-26-2000-11:15-wht@bob-RELEASE 4.42 */
/*:11-16-1997-22:15-wht@kepler-regexp_compile changed shape */
/*:03-16-1997-03:27-rll@felton.felton.ca.us-Fix boxes for curses under SCO */
/*:02-09-1997-19:37-wht@yuriatin-mv find_procedure defn to ecu.h */
/*:01-24-1997-02:37-wht@yuriatin-SOURCE RELEASE 4.00 */
/*:09-16-1996-05:21-wht@yuriatin-protect colons in phdir entries */
/*:09-11-1996-20:00-wht@yuriatin-3.48-major telnet,curses,structural overhaul */
/*:09-09-1996-03:16-wht@yuriatin-CFG_TelnetOption sizing */
/*:08-22-1996-15:27-wht@fep-better add_or_edit abort on ESC */
/*:08-20-1996-12:39-wht@kepler-locale/ctype fixes from ache@nagual.ru */
/*:07-24-1996-21:37-wht@n4hgf-no more wvline/whline */
/*:12-06-1995-13:30-wht@n4hgf-termecu w/errno -1 consideration */
/*:11-23-1995-11:20-wht@kepler-source control 3.37 for tsx-11 */
/*:11-14-1995-10:23-wht@kepler-3.37.80-source control point: SOCKETS */
/*:10-14-1995-23:17-wht@kepler-widen addw for baud string under Linux/BSD */
/*:10-14-1995-16:29-wht@kepler-use valid_baud_string */
/*:09-17-1995-16:35-wht@kepler-dcdw==1 does not work under Linux */
/*:05-11-1995-15:55-wht@n4hgf-ck_sigint fools optimizing compilers */
/*:05-11-1995-15:00-wht@n4hgf-^F and ^B vi-isms for page down and up */
/*:03-12-1995-02:28-wht@kepler-properly mark dirty on any edited existing pde */
/*:03-12-1995-01:54-wht@kepler-prevent clobber of reentered field during add */
/*:03-12-1995-01:03-wht@kepler-use ECU_MAXPN and clean up get_curr_dir */
/*:01-12-1995-15:19-wht@n4hgf-apply Andrew Chernov 8-bit clean+FreeBSD patch */
/*:05-04-1994-04:38-wht@n4hgf-ECU release 3.30 */
/*:11-17-1993-10:58-wht@n4hgf-ask for 'tty1a' not '1a' in case 1 */
/*:08-17-1993-14:00-wht@n4hgf-with new wingets, skip standout in phdir edit */
/*:05-29-1993-20:21-wht@n4hgf-change linst_err_text to LINST_text */
/*:03-01-1993-03:28-wht@n4hgf-add PgUP and PgDn */
/*:01-11-1993-16:05-wht@n4hgf-improve error checking + bell on error */
/*:12-04-1992-20:43-wht@n4hgf-fix logic error test */
/*:09-10-1992-13:58-wht@n4hgf-ECU release 3.20 */
/*:08-22-1992-15:38-wht@n4hgf-ECU release 3.20 BETA */
/*:05-17-1992-18:29-wht@n4hgf-pde add now cycles thru fields til END */
/*:11-28-1991-14:32-wht@n4hgf-add dcdwatch option */
/*:11-20-1991-05:19-wht@n4hgf-improve "Any" line handling on add or edit */
/*:08-28-1991-15:15-wht@n4hgf2-fix bad structure in phdir_add_or_edit_read */
/*:08-25-1991-14:39-wht@n4hgf-SVR4 port thanks to aega84!lh */
/*:08-15-1991-18:13-wht@n4hgf-do not allow edit of non-existent entry */
/*:08-11-1991-19:56-wht@n4hgf-soup up tty name for ISC vs. SCO */
/*:08-07-1991-13:48-root@n4hgf-w subcommand was not asking both questions */
/*:08-01-1991-03:52-wht@n4hgf-when editing string, set cursor to end */
/*:07-25-1991-12:56-wht@n4hgf-ECU release 3.10 */
/*:07-17-1991-07:04-wht@n4hgf-avoid SCO UNIX nap bug */
/*:07-12-1991-15:37-wht@n4hgf-fix core dump when creating phone from setup */
/*:06-09-1991-16:54-jjb-want_pd_create sneak path when not in curses */
/*:06-02-1991-19:43-wht@n4hgf-add dial debug level */
/*:06-02-1991-17:29-wht@n4hgf-move hdb_choose_Any to hdbintf.c */
/*:06-01-1991-23:53-wht@n4hgf-use PDE_..._LEN identifiers */
/*:04-03-1991-14:47-wht@n4hgf-must refresh both windows in terminfo curses */
/*:03-18-1991-21:39-wht@n4hgf-add wrefresh of scrw in up/down */
/*:02-05-1991-14:51-wht@n4hgf-calloc PDE instead of malloc */
/*:01-09-1991-22:31-wht@n4hgf-ISC port */
/*:08-14-1990-20:40-wht@n4hgf-ecu3.00-flush old edit history */

#include "ecucurses.h"

#define STDIO_H_INCLUDED
#define OMIT_TERMIO_REFERENCES
#include "ecu.h"
#include "pc_scr.h"

#include "ecupde.h"
#include "ecukey.h"
#include "ecuxkey.h"
#include "esd.h"
#include "var.h"

WINDOW *window_create();

void dirw_bot_msg();
void dirw_display_phonedir_name();
void dirw_cmd_line_setup();

/* window definitions */
#define DIRW_LINES		(LINES - 1)
#define DIRW_COLS		80
#define DIRW_TOP_LINES	2
#define DIRW_BOT_LINES	4
#define DIRW_CMD_LINE	(DIRW_LINES - 2)
#define SCRW_LINES		(DIRW_LINES - DIRW_TOP_LINES - DIRW_BOT_LINES)
#define SCRW_COLS		(DIRW_COLS)
#define SCRW_TLY		(DIRW_TOP_LINES)
#define SCRW_TLX		0
#define PDE_ITEM_COUNT	8
#define ADDW_LINES		(PDE_ITEM_COUNT + 6)
#ifdef CFG_TelnetOption
#define ADDW_COLS		75
#else
#define ADDW_COLS		65
#endif
#define ADDW_TLY		(SCRW_TLY + 1)
#define ADDW_TLX		4

extern int errno;

extern int windows_active;
extern char errmsg[];
extern char kbdintr;
extern char phonedir_name[]; /* phone directory name */
extern char *phonedir_trigger;
extern char *valid_baud_string;

UINT ttygetc();

WINDOW *dirw = 0;
WINDOW *scrw = 0;
WINDOW *addw = 0;

PDE *phdir_list_head = 0;	 /* pointer to first pde in linked list */
PDE *curr_pde = 0;			 /* current pde */
PDE *remove_pde = 0;		 /* if non-zero, pde just removed */
int remove_dirty_flag;		 /* phdir_list_dirty at remove time */
int phdir_list_quan = 0;	 /* count of items in list now */
int phdir_list_dirty = 0;	 /* phdir_list modified but not saved */
int pde_marked_for_redial_count = 0;
int scrw_curr_pde_line;		 /* scrw line curr_pde is on */
int phonedir_name_x;		 /* position for phonedir name on screen */

#define NAP_DECISEC_SINGLE_MIN         10
#define NAP_DECISEC_MULTIPLE_MIN       10
int nap_decisec_single = 150;
int nap_decisec_multiple = 50;

#if LOGICAL_LEN > 20
#define LOGICAL_DISP_LEN 20
#else
#define LOGICAL_DISP_LEN LOGICAL_LEN
#endif

/*+-------------------------------------------------------------------------
	phdir_list_erase()
--------------------------------------------------------------------------*/
void
phdir_list_erase()
{
	PDE *pde = phdir_list_head;
	PDE *next;

	while (pde)
	{
		next = pde->next;
		free((char *)pde);
		pde = next;
	}
	phdir_list_head = (PDE *) 0;
	phdir_list_quan = 0;
	curr_pde = (PDE *) 0;
	remove_pde = (PDE *) 0;
	pde_marked_for_redial_count = 0;

}							 /* end of phdir_list_erase */

/*+-------------------------------------------------------------------------
	phdir_list_set_dirty(flag)
0: clean, 1 dirty, -1 do not modify;update screen only
--------------------------------------------------------------------------*/
void
phdir_list_set_dirty(flag)
int flag;
{
	int itmp;
	char *modified = " modified ";

	if (flag != phdir_list_dirty)
	{
		if (flag != -1)
			phdir_list_dirty = flag;
		wmove(dirw, 0, DIRW_COLS - 14);
		if (phdir_list_dirty)
		{
			wstandout(dirw);
			waddstr(dirw, modified);
			wstandend(dirw);
		}
		else
		{
			itmp = strlen(modified);
			while (itmp--)
#if defined(SVR4) || defined(SCO32v4) || defined(SCO32v5)
				waddch(dirw, sHR);
#else
				waddch(dirw, sHR & 0xFF);
#endif
		}
		wrefresh(dirw);
	}
}							 /* end of phdir_list_set_dirty */

/*+-------------------------------------------------------------------------
	phdir_list_add(tpde) -- add to linked list
--------------------------------------------------------------------------*/
void
phdir_list_add(tpde)
PDE *tpde;
{
	PDE *this = tpde;
	PDE *prev;
	PDE *next;

/* if empty, init list with this one and quit */
	if (phdir_list_head == (PDE *) 0)
	{
		phdir_list_head = this;
		this->prev = (PDE *) 0;
		this->next = (PDE *) 0;
		phdir_list_quan++;
		return;
	}

/* list not empty */
	prev = (PDE *) 0;		 /* no previous yet */
	next = phdir_list_head;	 /* init next to top of list */

	while (strcmp(next->logical, this->logical) < 0)
	{
		prev = next;
		next = prev->next;
		if (next == (PDE *) 0)
			break;
	}

	if (prev)				 /* if non-zero, we will not update the list
							  * head */
	{

		/*
		 * 'this' is to be added to a non-empty list
		 */
		this->next = prev->next;
		this->prev = prev;
		prev->next = this;
		if (next)
			next->prev = this;
	}
	else
	{

		/*
		 * 'this' is to become the new list head (1st element)
		 */
		this->next = next;
		this->prev = (PDE *) 0;
		if (next)
			next->prev = this;
		phdir_list_head = this;
	}
	phdir_list_quan++;

}							 /* end of pde_add */

/*+-------------------------------------------------------------------------
	phdir_list_remove(tpde) -- remove from linked list
--------------------------------------------------------------------------*/
void
phdir_list_remove(tpde)
PDE *tpde;
{
	PDE *prev;
	PDE *next;

	prev = (PDE *) 0;		 /* there is no previous now */

	if ((next = phdir_list_head) == (PDE *) 0)	/* if empty list */
		return;

	while (next != tpde)
	{
		prev = next;
		next = prev->next;
		if (next == (PDE *) 0)
			return;
	}

/* take care of "current pde" */
	if (tpde == curr_pde)
	{
		if (tpde->next)
			curr_pde = tpde->next;
		else if (tpde->prev)
			curr_pde = tpde->prev;
		else
			curr_pde = (PDE *) 0;
	}

/* marked? */
	if (tpde->redial)
	{
		tpde->redial = 0;
		pde_marked_for_redial_count--;
	}

/* unlink */

	if (prev)				 /* if non-zero, we will not update the list
							  * head */
	{
		prev->next = tpde->next;
		if (tpde->next)
			(tpde->next)->prev = prev;
	}
	else
	{
		phdir_list_head = tpde->next;
		if (tpde->next)
			(tpde->next)->prev = (PDE *) 0;
	}

	tpde->next = (PDE *) 0;
	tpde->prev = (PDE *) 0;

	phdir_list_quan--;
}							 /* end of phdir_list_remove */

/*+-----------------------------------------------------------------------
	PDE *phdir_list_search(logical,exact_flag)
------------------------------------------------------------------------*/
PDE *
phdir_list_search(logical, exact_flag)
char *logical;
int exact_flag;
{
	PDE *tpde;

	if (!phdir_list_quan)
	{
		if (phdir_list_read())
			return ((PDE *) 0);
	}

	if (!logical || !*logical)
		return ((PDE *) 0);

	tpde = phdir_list_head;
	while (tpde)
	{
		/* only first few chars necessary for match with ulcmpb */
		if (exact_flag)
		{
			if (strcmp(tpde->logical, logical) == 0)
				return (tpde);
		}
		else
		{
			if (ulcmpb(tpde->logical, logical) < 0)
				return (tpde);
		}
		tpde = tpde->next;
	}
	if (!tpde)
		sprintf(errmsg, "'%s' not found", logical);
	return (tpde);

}							 /* end of phdir_list_search */

/*+-------------------------------------------------------------------------
	want_pd_create(name)
--------------------------------------------------------------------------*/
int
want_pd_create(name)
char *name;
{
	UINT uctmp = XF_not_yet;

	if (!dirw)
		return (1);

#ifdef COMPILER_BUG_FIXED
	dirw_bot_msg("type 'y' or 'n'");
	while (uctmp == XF_not_yet)
	{
		ring_bell();
		dirw_cmd_line_setup(name, "does not exist: create?");
		uctmp = to_lower(ttygetc(0));
		switch (uctmp)
		{
			case 'y':
				uctmp = 1;
				break;
			case 'n':
				uctmp = 0;
				break;
			default:
				uctmp = XF_not_yet;
				break;
		}
	}
	dirw_bot_msg("");
	return ((int)uctmp);

#else

  KROCK:
	dirw_bot_msg("type 'y' or 'n'");
	ring_bell();
	dirw_cmd_line_setup(name, "does not exist: create?");
	uctmp = to_lower(ttygetc(0));
	dirw_bot_msg("");
	switch (uctmp)
	{
		case 'y':
			return (1);
		case 'n':
			return (0);
	}
	goto KROCK;
#endif
}							 /* end of want_pd_create */

/*+-------------------------------------------------------------------------
	dirw_display_phonedir_name()
--------------------------------------------------------------------------*/
void
dirw_display_phonedir_name()
{
	int itmp, x;
	char s80[80];

	if (!dirw || !phonedir_name[0])
		return;

	wmove(dirw, 0, phonedir_name_x);
	waddch(dirw, ' ');
	strncpy(s80, phonedir_name, itmp = DIRW_COLS - phonedir_name_x - 5);
	s80[itmp] = 0;
	waddstr(dirw, s80);
	waddch(dirw, ' ');
	getyx(dirw, itmp, x);

	while (x < DIRW_COLS - 1)
	{
#if defined(SVR4) || defined(SCO32v4) || defined(SCO32v5)
		waddch(dirw, sHR);
#else
		waddch(dirw, sHR & 0xFF);
#endif
		x++;
	}
}							 /* end of dirw_display_phonedir_name */

/*+-------------------------------------------------------------------------
	dirw_display_config()
--------------------------------------------------------------------------*/
void
dirw_display_config()
{
	int y, x;

	if (!dirw)
		return;

	getyx(dirw, y, x);
	while (x++ < (DIRW_COLS - 1))
		waddch(dirw, sHR);
	waddch(dirw, sRT);

	if (pde_marked_for_redial_count)
	{
		wmove(dirw, DIRW_LINES - DIRW_BOT_LINES, 2);
		wstandout(dirw);
		wprintw(dirw, " REDIAL CYCLE  wait: single=%d multiple=%d ",
			nap_decisec_single / 10, nap_decisec_multiple / 10);

		wmove(dirw, DIRW_LINES - DIRW_BOT_LINES, 56);
		wprintw(dirw, " %2d marked entr%s ",
			pde_marked_for_redial_count,
			(pde_marked_for_redial_count == 1) ? "y" : "ies");
		wstandend(dirw);
	}
}							 /* end of dirw_display_config */

/*+-----------------------------------------------------------------------
	dirw_display()
00000000001111111111222222222233333333334444444444555555555566666666667777777777
01234567890123456789012345678901234567890123456789012345678901234567890123456789
| entry name | telephone number | tty | baud P | description                   |
| 0123456789 | 0123456789012345 | 01  | baud P | 01234567890123456789012345678 |
------------------------------------------------------------------------*/
void
dirw_display()
{
	if (!dirw)
		return;
	wmove(dirw, 1, 1);
	wstandout(dirw);

#if defined(SVR4) || defined(SCO32v4) || defined(SCO32v5)
	waddstr(dirw,
		" entry name | telephone number | tty | baud  P | description                  "
		);
#else
	waddstr(dirw, " entry name ");
#ifdef __FreeBSD__
	waddch(dirw, sVR);
#else
	waddch(dirw, (unsigned)sVR);
#endif
	waddstr(dirw, " telephone number ");
#ifdef __FreeBSD__
	waddch(dirw, sVR);
#else
	waddch(dirw, (unsigned)sVR);
#endif
	waddstr(dirw, " tty ");
#ifdef __FreeBSD__
	waddch(dirw, sVR);
#else
	waddch(dirw, (unsigned)sVR);
#endif
	waddstr(dirw, " baud  P ");
#ifdef __FreeBSD__
	waddch(dirw, sVR);
#else
	waddch(dirw, (unsigned)sVR);
#endif
	waddstr(dirw, " description                  ");
#endif

	wstandend(dirw);
	dirw_display_phonedir_name();
	dirw_display_config();
	wrefresh(dirw);

}							 /* end of dirw_display */

/*+-------------------------------------------------------------------------
	dirw_bot_msg(msg)
--------------------------------------------------------------------------*/
void
dirw_bot_msg(msg)
char *msg;
{
	int itmp;
	int itmp2;
	static int last_msglen = 0;

#define DIRW_BOT_LINE_TLX 2
#define DIRW_BOT_LINE_MAX_MSGLEN	(DIRW_COLS - DIRW_BOT_LINE_TLX - 8)
	char msg2[80];

	if (!dirw || (!last_msglen && !strlen(msg)))
		return;

	wmove(dirw, DIRW_LINES - 1, DIRW_BOT_LINE_TLX);

	if ((itmp = strlen(msg)) == 0)
	{
		itmp2 = last_msglen + 2;

		for (itmp = 0; itmp < itmp2; itmp++)
#if defined(SVR4) || defined(SCO32v4) || defined(SCO32v5)
			waddch(dirw, sHR);
#else
			waddch(dirw, sHR & 0xFF);
#endif
		last_msglen = 0;
	}
	else
	{
		waddch(dirw, ' ');
		if (itmp > DIRW_BOT_LINE_MAX_MSGLEN)
		{
			strncpy(msg2, msg, DIRW_BOT_LINE_MAX_MSGLEN + 1);
			msg2[DIRW_BOT_LINE_MAX_MSGLEN + 1] = 0;
			waddstr(dirw, msg2);
			itmp = strlen(msg2);
		}
		else
		{
			waddstr(dirw, msg);
			itmp = strlen(msg);
		}
		waddch(dirw, ' ');
		if ((itmp2 = last_msglen - itmp) > 0)
		{
			while (itmp2--)
#if defined(SVR4) || defined(SCO32v4) || defined(SCO32v5)
				waddch(dirw, sHR);
#else
				waddch(dirw, sHR & 0xFF);
#endif
		}
		last_msglen = itmp;	 /* remember last message length */
	}
	wrefresh(dirw);
}							 /* end of dirw_bot_msg */

/*+-------------------------------------------------------------------------
	phdir_display_logical(line,tpde,stand_out)
--------------------------------------------------------------------------*/
void
phdir_display_logical(line, tpde, stand_out)
int line;
PDE *tpde;
int stand_out;
{

	wmove(scrw, line, 0);
#if defined(SVR4) || defined(SCO32v4) || defined(SCO32v5)
	waddch(scrw, sVR);
#else
	waddch(scrw, sVR & 0xFF);
#endif

	if (tpde->redial)
	{
		wstandout(scrw);
		waddch(scrw, '>');
		wstandend(scrw);
	}
	else
		waddch(scrw, ' ');

	if (stand_out)
		wstandout(scrw);
	wprintw(scrw, "%-10.10s", tpde->logical);
	if (stand_out)
		wstandend(scrw);

}							 /* end of phdir_display_logical */

/*+-----------------------------------------------------------------------
	phdir_display(win,line,tpde,stand_out)
00000000001111111111222222222233333333334444444444555555555566666666667777777777
01234567890123456789012345678901234567890123456789012345678901234567890123456789
| entry name | telephone number | tty | baud  P | description                   |
| 0123456789 | 0123456789012345 | 01  | baud  P | 01234567890123456789012345678 |
--------------------------------------------------------------------------*/
phdir_display(line, tpde, stand_out)
int line;
PDE *tpde;
int stand_out;
{

	phdir_display_logical(line, tpde, stand_out);
	waddch(scrw, ' ');
#if defined(SVR4) || defined(SCO32v4) || defined(SCO32v5)
	waddch(scrw, sVR);
	waddch(scrw, ' ');
#else
	waddch(scrw, (unsigned)sVR);
	waddch(scrw, ' ');
#endif
	wprintw(scrw, "%-16.16s ", tpde->telno);
#if defined(SVR4) || defined(SCO32v4) || defined(SCO32v5)
	waddch(scrw, sVR);
#else
	waddch(scrw, (unsigned)sVR);
#endif
	if (tpde->tty[0])
		wprintw(scrw, "%-5.5s", tpde->tty);
	else
		waddstr(scrw, "Any  ");
#if defined(SVR4) || defined(SCO32v4) || defined(SCO32v5)
	waddch(scrw, sVR);
#else
	waddch(scrw, (unsigned)sVR);
#endif
	wprintw(scrw, "%6u %c ", tpde->baud,
		(tpde->parity) ? to_upper(tpde->parity) : 'N');
#if defined(SVR4) || defined(SCO32v4) || defined(SCO32v5)
	waddch(scrw, sVR);
#else
	waddch(scrw, (unsigned)sVR);
#endif
	wprintw(scrw, " %-28.28s ", tpde->descr);
#if defined(SVR4) || defined(SCO32v4) || defined(SCO32v5)
	waddch(scrw, sVR);
#else
	waddch(scrw, (unsigned)sVR);
#endif
	return (0);

}							 /* end of phdir_display */

/*+-----------------------------------------------------------------------
	scrw_fill(first_pde,curr_pde_line)
------------------------------------------------------------------------*/
void
scrw_fill(tpde, curr_pde_line)
PDE *tpde;
int *curr_pde_line;
{
	int line;
	int is_curr_pde;

	*curr_pde_line = -1;
	for (line = 0; line < SCRW_LINES; line++)
	{
		if (tpde)
		{
			if (is_curr_pde = (tpde == curr_pde))
				*curr_pde_line = line;
			phdir_display(line, tpde, is_curr_pde);
			tpde = tpde->next;
		}
		else
		{
			wmove(scrw, line, 0);
#if defined(SVR4) || defined(SCO32v4) || defined(SCO32v5)
			waddch(scrw, sVR);
#else
			waddch(scrw, (unsigned)sVR & 0xFF);
#endif
			wclrtoeol(scrw);
			wmove(scrw, line, SCRW_COLS - 1);
#if defined(SVR4) || defined(SCO32v4) || defined(SCO32v5)
			waddch(scrw, sVR);
#else
			waddch(scrw, sVR & 0xFF);
#endif
		}
	}
	wrefresh(scrw);

}							 /* end of scrw_fill */

/*+-------------------------------------------------------------------------
	scrw_fill_at(line_num,first_pde,curr_pde_line)
--------------------------------------------------------------------------*/
void
scrw_fill_at(line_num, tpde, curr_pde_line)
int line_num;
PDE *tpde;
int *curr_pde_line;
{
	int itmp;

	if (!tpde)
	{
		scrw_fill(tpde, curr_pde_line);
		return;
	}
	for (itmp = 0; itmp < line_num; itmp++)
	{
		if (!tpde->prev)
			break;
		tpde = tpde->prev;
	}

	scrw_fill(tpde, curr_pde_line);

}							 /* end of scrw_fill_at */

/*+-------------------------------------------------------------------------
	dirw_cmd_line_setup(prompt1,prompt2)
--------------------------------------------------------------------------*/
void
dirw_cmd_line_setup(prompt1, prompt2)
char *prompt1;
char *prompt2;
{
	int icol;
	int y;
	int x;
	char *cp;
	int standout_mode;

	wmove(dirw, DIRW_CMD_LINE - 1, 1);
	wstandend(dirw);
	standout_mode = 0;
	waddch(dirw, ' ');
	cp = prompt1;
	while (*cp)
	{
		if (*cp == '~')
		{
			if (standout_mode)
				wstandend(dirw);
			else
				wstandout(dirw);
			standout_mode = !standout_mode;
			cp++;
		}
		else
			waddch(dirw, *cp++);
	}
	wstandend(dirw);
	standout_mode = 0;

	waddch(dirw, ' ');
	getyx(dirw, y, x);
	for (icol = x; icol < DIRW_COLS - 1; icol++)
		waddch(dirw, ' ');

	wmove(dirw, DIRW_CMD_LINE, 1);
	waddch(dirw, ' ');
	cp = prompt2;
	while (*cp)
	{
		if (*cp == '~')
		{
			if (standout_mode)
				wstandend(dirw);
			else
				wstandout(dirw);
			standout_mode = !standout_mode;
			cp++;
		}
		else
			waddch(dirw, *cp++);
	}
	wstandend(dirw);
	waddch(dirw, ' ');
	getyx(dirw, y, x);
	for (icol = x; icol < DIRW_COLS - 1; icol++)
		waddch(dirw, ' ');
	wmove(dirw, y, x);
	wrefresh(scrw);
	wrefresh(dirw);
}							 /* end of dirw_cmd_line_setup */

/*+-------------------------------------------------------------------------
	dirw_get_cmd()
--------------------------------------------------------------------------*/
UINT
dirw_get_cmd()
{
	UINT cmd;
	char setupline1[128];	 /* yetch ... avoid source line > 80 chars */
	char setupline2[128];
	char *setupline1_1 =
	"~d~own ~u~p ~PgDn~ ~PgUp~ ~e~dit ~a~dd ~r~emove ~s~ave ~f~ind ";
	char *setupline1_2 =
	"~c~hange dial dir";
	char *setupline2_1 =
	"redial: ~m~ark un~M~ark ~U~nmark all ~w~ait between dial ";
	char *setupline2_2 =
	"~ENTER~:dial ~ESC,q~uit";

	strcpy(setupline1, setupline1_1);
	strcat(setupline1, setupline1_2);
	strcpy(setupline2, setupline2_1);
	strcat(setupline2, setupline2_2);

	dirw_cmd_line_setup(setupline1, setupline2);

	cmd = ttygetc(1);

	dirw_bot_msg("");
	return (cmd);

}							 /* end of dirw_get_cmd */

/*+-------------------------------------------------------------------------
	field_colon_protect(fieldstr) - change field contents: comma to tilde
--------------------------------------------------------------------------*/
void
field_colon_protect(fieldstr)
char *fieldstr;
{
	while (*fieldstr)		 /* cannot have '~' in description */
	{
		if (*fieldstr == '~')
			*fieldstr = '-';
		else if (*fieldstr == ':')	/* cvt colon to tilde */
			*fieldstr = '~';
		fieldstr++;
	}
}							 /* end of field_colon_protect */

/*+-------------------------------------------------------------------------
	field_colon_restore(fieldstr) - change field contents: comma to tilde
--------------------------------------------------------------------------*/
void
field_colon_restore(fieldstr)
char *fieldstr;
{
	while (*fieldstr)		 /* cannot have ':' in description */
	{						 /* should have picked another separator, ... */
		if (*fieldstr == '~')/* ... but compatibility is important */
			*fieldstr = ':';
		fieldstr++;
	}
}							 /* end of field_colon_restore */

/*+-------------------------------------------------------------------------
	phdir_cmd_save()
--------------------------------------------------------------------------*/
void
phdir_cmd_save()
{
	FILE *fpold;
	FILE *fpnew;
	PDE *tpde;
	char *cp;
	char phonedir_ntmp[256]; /* temp phone directory name */
	char iobuf[128];
	int count = 0;

	if (!phdir_list_dirty)
	{
		dirw_bot_msg("directory has not been modified");
		return;
	}

	strcpy(phonedir_ntmp, phonedir_name);
	strcat(phonedir_ntmp, ".t");

	if (!(fpnew = fopen(phonedir_ntmp, "w")))	/* open old file */
	{
		sprintf(iobuf, "cannot open %s", phonedir_ntmp);
		dirw_bot_msg(iobuf);
		ring_bell();
		return;
	}

/* write trigger */
	fputs(phonedir_trigger, fpnew);

/* retain commented entries */
	if ((fpold = fopen(phonedir_name, "r")))	/* open old file */
	{
		while (fgets(iobuf, sizeof(iobuf), fpold))
		{
			if ((iobuf[0] == '#') && strcmp(iobuf, phonedir_trigger))
				fputs(iobuf, fpnew);
		}
		fclose(fpold);
	}

/* write new entries */
	tpde = phdir_list_head;
	while (tpde)
	{
		cp = tpde->descr;
		while (*cp)			 /* cannot have ':' in description */
		{					 /* should have picked another separator, ... */
			if (*cp == ':')	 /* ... but compatibility is important */
				*cp = '-';
			cp++;
		}
		sprintf(iobuf, "%d", count + 1);
		dirw_bot_msg(iobuf);
		field_colon_protect(tpde->logical);
		field_colon_protect(tpde->telno);
		field_colon_protect(tpde->tty);
		field_colon_protect(tpde->descr);
		sprintf(iobuf, "%s:%s:%s:%u:%c:%s:%d:%c:%c\n",
			tpde->logical, tpde->telno,
			tpde->tty, tpde->baud,
			(tpde->parity) ? to_upper(tpde->parity) : 'N',
			tpde->descr, tpde->debug_level, to_upper(tpde->dcdwatch),
			to_upper(tpde->rtscts_val));
		field_colon_restore(tpde->logical);
		field_colon_restore(tpde->telno);
		field_colon_restore(tpde->tty);
		field_colon_restore(tpde->descr);
		fputs(iobuf, fpnew);
		tpde = tpde->next;
		count++;
	}

	fclose(fpnew);
	if (unlink(phonedir_name))
	{
		sprintf(iobuf, "removing old file: %s", strerror(errno));
		dirw_bot_msg(iobuf);
		ring_bell();
		return;
	}
	if (rename(phonedir_ntmp, phonedir_name))	/* uh oh on old XENIX */
	{
		sprintf(iobuf, "renaming new file: %s", strerror(errno));
		dirw_bot_msg(iobuf);
		ring_bell();
		return;
	}
	sprintf(iobuf, "saved %d entries", count);
	dirw_bot_msg(iobuf);
	phdir_list_set_dirty(0);

}							 /* end of phdir_cmd_save */

/*+-------------------------------------------------------------------------
	phdir_list_save_if_dirty()
--------------------------------------------------------------------------*/
void
phdir_list_save_if_dirty()
{
	UINT cmd = 0;

	if (phdir_list_dirty)
	{
		dirw_bot_msg("type 'y' or 'n'");
		ring_bell();
		while (!cmd)
		{
			ring_bell();
			dirw_cmd_line_setup("", "current directory modified: save?");
			cmd = ttygetc(0);
			if (isupper(cmd))
				cmd = tolower(cmd);
			switch (cmd)
			{
				case 'y':
					phdir_cmd_save();
					break;
				case 'n':
					break;
				default:
					cmd = 0;
					break;
			}
		}
		dirw_bot_msg("");
	}
}							 /* end of phdir_list_save_if_dirty */

/*+-----------------------------------------------------------------------
	phdir_list_read()

return 0 if entire list read, else 1 if error (error msg in errmsg)

if file does not exist, create it, asking confirm only if in
interactive (curses) mode
------------------------------------------------------------------------*/
int
phdir_list_read()
{
	int token_number;
	char *cp;
	char *token;
	int itmp;
	char readpde_buf[128];
	FILE *fp_phone;
	PDE *tpde;
	char *str_token();

	if (!phonedir_name[0])
	{
		get_home_dir(phonedir_name);
		strcat(phonedir_name, "/.ecu/phone");
	}

  TRY_OPEN:
	if (!(fp_phone = fopen(phonedir_name, "r")))
	{
		if (errno == ENOENT)
		{
			if (!want_pd_create(phonedir_name))
			{
				strcpy(errmsg, "non-existent file not created");
				return (1);
			}
			if ((itmp = open(phonedir_name,
						O_RDWR | O_CREAT | O_TRUNC, 0600)) >= 0)
			{
				write(itmp, phonedir_trigger, strlen(phonedir_trigger));
				close(itmp);
				if (windows_active)	/* if called under curses */
				{
					dirw_bot_msg("created new (empty) directory file");
					ring_bell();
					Nap(1000L);
				}
				goto TRY_OPEN;
			}
			if (errno == ENOENT)
			{
				strcpy(errmsg, "~/.ecu directory nonexistent!");
				ring_bell();
				return (1);
			}
		}
		strcpy(errmsg, strerror(errno));
		return (1);
	}

/* we have an open directory file */
	if (!fgets(readpde_buf, sizeof(readpde_buf), fp_phone) ||
		strcmp(readpde_buf, phonedir_trigger))
	{
		fclose(fp_phone);
		strcpy(errmsg, "not an ECU phone directory (or is pre-rev-3)");
		ring_bell();
		return (1);
	}

	dirw_display_phonedir_name();
	phdir_list_erase();		 /* clear any previous directory */
	while (fgets(readpde_buf, sizeof(readpde_buf), fp_phone))
	{
		if (readpde_buf[0] == '#')	/* comment? */
			continue;
		if (itmp = strlen(readpde_buf))	/* itmp = len; if > 0 ... */
		{
			itmp--;
			readpde_buf[itmp] = 0;	/* ... strip trailing NL */
		}
		cp = readpde_buf;	 /* first call to str_token, -> buff */
		while ((*cp == 0x20) || (*cp == 0x09))
			cp++;			 /* strip leading spaces */
		if (*cp == 0)		 /* if line all blank, skip it */
			continue;

		if (!(tpde = (PDE *) malloc(sizeof(PDE))))
		{
			fclose(fp_phone);
			strcpy(errmsg, "Out of memory reading phone list");
			return (1);
		}

		tpde->descr[0] = 0;
		tpde->logical[0] = 0;
		tpde->telno[0] = 0;
		tpde->tty[0] = 0;
		tpde->parity = 0;
		tpde->baud = 2400;
		tpde->redial = 0;
		tpde->prev = (PDE *) 0;
		tpde->next = (PDE *) 0;
		tpde->debug_level = 0;
		tpde->dcdwatch = 'n';/* do not modify shm->Ldcdwatch */
		tpde->rtscts_val = 'n';	/* do not modify shm->Lrtscts_val */

		token_number = 0;
		while ((token = str_token(cp, ":")))
		{
			cp = (char *)0;	 /* further calls to str_token need NULL */
			switch (token_number)
			{
				case 0:	 /* first field is logical name */
					strncpy(tpde->logical, token, sizeof(tpde->logical));
					tpde->logical[sizeof(tpde->logical) - 1] = 0;
					break;
				case 1:	 /* second field is tpde->telno phone number */
					strncpy(tpde->telno, token, sizeof(tpde->telno));
					tpde->telno[sizeof(tpde->telno) - 1] = 0;
					break;
				case 2:	 /* third field is line */
					strncpy(tpde->tty, token, sizeof(tpde->tty));
					tpde->tty[sizeof(tpde->tty) - 1] = 0;
					break;
				case 3:	 /* fourth field is bit rate */
					tpde->baud = atoi(token);
					break;
				case 4:	 /* fifth field is parity */
					switch (itmp = to_lower(token[0]))
					{
						case 'o':
						case 'e':
						case 'm':
						case 's':
							tpde->parity = itmp;
							break;
						default:
						case 'n':
							tpde->parity = 0;
							break;
					}
					break;
				case 5:
					strncpy(tpde->descr, token, sizeof(tpde->descr));
					tpde->descr[sizeof(tpde->descr) - 1] = 0;
					break;
				case 6:
					if ((tpde->debug_level = (uchar) atoi(token)) > 9)
						tpde->debug_level = 9;
					break;
				case 7:
					tpde->dcdwatch = to_lower(token[0]);
#ifdef linux

					/*
					 * alas, Linux does not handle setting DCD watch to
					 * one (catch termination of connection, yet continue
					 * line I/O) ... silently convert to terminate ECU
					 */
					if (tpde->dcdwatch == '1')
						tpde->dcdwatch = 't';
#endif
					break;
				case 8:
					tpde->rtscts_val = to_lower(token[0]);
					break;
			}				 /* end of switch(token_number) */
			token_number++;
		}					 /* end while not end of record */

		field_colon_restore(tpde->logical);
		field_colon_restore(tpde->telno);
		field_colon_restore(tpde->tty);
		field_colon_restore(tpde->descr);
		phdir_list_add(tpde);

	}						 /* while records left to ready */

	fclose(fp_phone);
	return (0);
}							 /* end of phdir_list_read */

/*+-------------------------------------------------------------------------
	phdir_cmd_up()
--------------------------------------------------------------------------*/
void
phdir_cmd_up()
{
	PDE *tpde;

	if ((!curr_pde) || (curr_pde->prev == (PDE *) 0))
	{
		ring_bell();
		return;
	}
	if (scrw_curr_pde_line)
	{
		phdir_display_logical(scrw_curr_pde_line, curr_pde, 0);
		scrw_curr_pde_line--;
		curr_pde = curr_pde->prev;
		phdir_display_logical(scrw_curr_pde_line, curr_pde, 1);
	}
	else
	{
		tpde = curr_pde;
		curr_pde = curr_pde->prev;
		scrw_fill_at(10, tpde, &scrw_curr_pde_line);
	}

}							 /* end of phdir_cmd_up */

/*+-------------------------------------------------------------------------
	phdir_cmd_down()
--------------------------------------------------------------------------*/
void
phdir_cmd_down()
{
	PDE *tpde;

	if ((!curr_pde) || (curr_pde->next == (PDE *) 0))
	{
		ring_bell();
		return;
	}
	if (scrw_curr_pde_line < (SCRW_LINES - 1))
	{
		phdir_display_logical(scrw_curr_pde_line, curr_pde, 0);
		scrw_curr_pde_line++;
		curr_pde = curr_pde->next;
		phdir_display_logical(scrw_curr_pde_line, curr_pde, 1);
	}
	else
	{
		tpde = curr_pde;
		curr_pde = curr_pde->next;
		scrw_fill_at(SCRW_LINES - 10, tpde, &scrw_curr_pde_line);
	}

}							 /* end of phdir_cmd_down */

/*+-------------------------------------------------------------------------
	phdir_cmd_pgup()
--------------------------------------------------------------------------*/
void
phdir_cmd_pgup()
{
	PDE *tpde;
	int itmp;

	if (!curr_pde)
	{
		ring_bell();
		return;
	}
	tpde = curr_pde;
	if (!(itmp = SCRW_LINES * 3 / 4))
		itmp = 1;
	while (itmp-- && tpde->prev)
		tpde = tpde->prev;
	curr_pde = tpde;
	scrw_fill_at(SCRW_LINES / 2, tpde, &scrw_curr_pde_line);

}							 /* end of phdir_cmd_pgup */

/*+-------------------------------------------------------------------------
	phdir_cmd_pgdn()
--------------------------------------------------------------------------*/
void
phdir_cmd_pgdn()
{
	PDE *tpde;
	int itmp;

	if (!curr_pde)
	{
		ring_bell();
		return;
	}
	tpde = curr_pde;
	if (!(itmp = SCRW_LINES * 3 / 4))
		itmp = 1;
	while (itmp-- && tpde->next)
		tpde = tpde->next;
	curr_pde = tpde;
	scrw_fill_at(SCRW_LINES / 2, tpde, &scrw_curr_pde_line);

}							 /* end of phdir_cmd_pgdn */

/*+-------------------------------------------------------------------------
	check_curr_pde() -- return 1 if there is a current pde, else 0
--------------------------------------------------------------------------*/
check_curr_pde()
{
	if (!curr_pde)
	{
		dirw_bot_msg("no directory entry selected");
		ring_bell();
		return (0);
	}
	return (1);
}							 /* end of check_curr_pde */

/*+-------------------------------------------------------------------------
	phdir_add_or_edit_read(prompt,y,buf,bufmax,delim)

There are numerous theoretcally possible string overflow possibilities
in here, but no practical string will be long enough
--------------------------------------------------------------------------*/
void
phdir_add_or_edit_read(prompt, y, buf, bufmax, delim)
char *prompt;
int y;
char *buf;
int bufmax;
UINT *delim;
{
	int wgpos = -1;
	char s80[80];
	int done = 0;

	if (!check_curr_pde())
		return;

	wmove(addw, PDE_ITEM_COUNT + 3, 2);
	waddstr(addw, prompt);

	strcpy(s80, buf);

	do
	{
		(void)wingets(addw, y, 20, s80, bufmax, delim, 1, &wgpos);
		clear_area(addw, y, 20, bufmax);

		switch (*((UINT *) delim))
		{
			case ESC:
				waddstr(addw, buf);
				done = 1;
				break;

			case TAB:
			case NL:
			case XFcurdn:
				*delim = NL;
			case XFend:
				strcpy(buf, s80);
				waddstr(addw, buf);
				done = 1;
				break;

			case CTL_U:
				s80[0] = 0;
				break;

			case CTL_B:
			case XFcurup:
				strcpy(buf, s80);
				waddstr(addw, buf);
				*delim = CTL_B;
				done = 1;
				break;

			case CTL_L:
			case CTL_R:
				done = 1;	 /* we will be right back */
				break;

			default:
				ring_bell();
				break;
		}
	}
	while (!done);

	clear_area(addw, PDE_ITEM_COUNT + 3, 2, strlen(prompt));

}							 /* end of phdir_add_or_edit_read */

/*+-------------------------------------------------------------------------
	phdir_add_or_edit(tpde,edit)
--------------------------------------------------------------------------*/
int
phdir_add_or_edit(tpde, edit)
PDE *tpde;
int edit;
{
	int erc = 0;
	int input_state = 0;
	int changed = 0;
	int done = 0;
	int aborted = 0;
	int have_already_set_dirty = 0;
	int itmp;
	int wgedit = 0;
	int wgpos = -1;
	char s64[64];
	UINT delim = 0;
	int y, x;
	PDE *old_curr_pde = (PDE *) 0;
	UINT baud;
	char cmpbuf[128];

	if (!edit)
	{
		dirw_bot_msg("ESC: abort  ^U: erase input");
		dirw_cmd_line_setup(
			"Only the 1st 10 characters appear in the table",
			"Enter new directory entry name: ");
		getyx(dirw, y, x);
		wstandout(dirw);
		while ((delim != ESC) && (delim != NL))
		{
			(void)wingets(dirw, y, x, tpde->logical, LOGICAL_DISP_LEN + 1, &delim,
				wgedit, &wgpos);
			wgedit = 1;
		}
		wstandend(dirw);
		dirw_bot_msg("");
		if ((!strlen(tpde->logical)) || (delim == ESC))
		{
			dirw_bot_msg("add aborted");
			ring_bell();
			return (0);
		}

		if (!isalpha((uchar) tpde->logical[0]))
		{
			dirw_bot_msg("first character must be alphabetic");
			ring_bell();
			return (0);
		}

		if (phdir_list_search(tpde->logical, 1))
		{
			sprintf(s64, "'%s' is already in the directory", tpde->logical);
			dirw_bot_msg(s64);
			ring_bell();
			return (0);
		}

		tpde->descr[0] = 0;
		tpde->telno[0] = 0;
		tpde->tty[0] = 0;
		tpde->parity = 0;
		tpde->baud = CFG_DefaultBitRate;
		tpde->debug_level = 0;
		tpde->dcdwatch = 'n';/* do not modify shm->Ldcdwatch */
		tpde->rtscts_val = 'n';	/* do not modify shm->Lrtscts_val */

		phdir_list_add(tpde);
		old_curr_pde = curr_pde;
		curr_pde = tpde;
		scrw_fill_at(SCRW_LINES / 2, tpde, &scrw_curr_pde_line);
		tpde = curr_pde;
	}						 /* end of add code */

	dirw_cmd_line_setup("", "");

	/*
	 * get a new window
	 */
	sprintf(s64, "entry: %s", tpde->logical);
	addw = window_create(s64, 3, ADDW_TLY, ADDW_TLX, ADDW_LINES, ADDW_COLS);

	wmove(addw, 2, 2);
	waddstr(addw, "telephone number");
	wmove(addw, 3, 2);
	waddstr(addw, "device");
	wmove(addw, 4, 2);
	waddstr(addw, "bit rate");
	wmove(addw, 5, 2);
	waddstr(addw, "parity");
	wmove(addw, 6, 2);
	waddstr(addw, "description");
	wmove(addw, 7, 2);
	waddstr(addw, "debug level");
	wmove(addw, 7, 23);		 /* extra info */
	waddstr(addw, "(dialer -x value 0-9)");
	wmove(addw, 8, 2);
	waddstr(addw, "DCD watch");
	wmove(addw, 9, 2);
	waddstr(addw, "RTS/CTS flow ctl");
	wmove(addw, 9, 23);		 /* extra info */
	waddstr(addw, "(0=off,7=best,n=no change)");

	wmove(addw, 2, 20);
	waddstr(addw, tpde->telno);
	wmove(addw, 3, 20);
	waddstr(addw, (tpde->tty[0]) ? tpde->tty : "Any");
	sprintf(s64, "%-6u", tpde->baud);
	wmove(addw, 4, 20);
	waddstr(addw, s64);
	s64[0] = (tpde->parity) ? to_upper((char)tpde->parity) : 'N';
	s64[1] = 0;
	wmove(addw, 5, 20);
	waddstr(addw, s64);
	wmove(addw, 6, 20);
	waddstr(addw, tpde->descr);
	sprintf(s64, "%u", tpde->debug_level);
	wmove(addw, 7, 20);
	waddstr(addw, s64);
	wmove(addw, 8, 20);
	waddch(addw, tpde->dcdwatch);
	wmove(addw, 9, 20);
	waddch(addw, tpde->rtscts_val);

	wmove(addw, PDE_ITEM_COUNT + 4, 2);
	if (edit)
		waddstr(addw, "ESC: exit  END: accept  ^U: erase ^B: back  TAB: fwd");
	else
		waddstr(addw, "ESC: cancel  END: accept  ^U: erase field  ^B: back up");
	wrefresh(addw);

/* add/edit common */
	while (!done)
	{
		int screen_y = input_state + 2;	/* leave one blank line in box */
		changed = 0;
		switch (input_state)
		{
			case 0:
				if (edit)
					strcpy(s64, tpde->telno);
				phdir_add_or_edit_read("Enter telephone number",
					screen_y, tpde->telno, DESTREF_LEN + 1, &delim);
				if (delim == ESC)
					break;
				if (edit && strcmp(tpde->telno, s64))
					changed = 1;
				break;

			case 1:
				if (!tpde->tty[0])
					strcpy(tpde->tty, "Any");
				strcpy(s64, tpde->tty);
				erc = 0;
			  CASE_1_AGAIN:
				phdir_add_or_edit_read((erc) ? erc_text(erc) :
					"Enter tty (e.g. tty1a), Any or [=/]Devices-type",
					screen_y, tpde->tty, PDE_TTY_LEN + 1, &delim);
				if (delim == ESC)
					break;
				if ((s64[0] == '/') &&
					(erc = regexp_compile(tpde->tty + 1,
					cmpbuf, sizeof(cmpbuf))))
				{
					ring_bell();
					goto CASE_1_AGAIN;
				}
				erc = 0;
				if (!strlen(tpde->tty) || !strcmpi(tpde->tty, "any"))
				{
					strcpy(tpde->tty, "Any");
					clear_area(addw, screen_y, 20, PDE_TTY_LEN + 1);
					waddstr(addw, tpde->tty);
				}
				changed = !!strcmp(tpde->tty, s64);
				if (!strcmpi(tpde->tty, "any"))
					tpde->tty[0] = 0;
				break;

			case 2:
				sprintf(s64, "%u", tpde->baud);
				phdir_add_or_edit_read(
					valid_baud_string,
					screen_y, s64, 6 + 1, &delim);
				if (delim == ESC)
					break;
				if (valid_baud(baud = atoi(s64)) == -1)
				{
					ring_bell();
					continue;
				}
				if (tpde->baud != baud)
					changed = 1;
				tpde->baud = baud;
				break;

			case 3:
				sprintf(s64, "%c", (tpde->parity) ? tpde->parity : 'N');
				phdir_add_or_edit_read("Enter parity (n,o,e)",
					screen_y, s64, 1 + 1, &delim);
				if (delim == ESC)
					break;
				switch (s64[0] = to_lower(s64[0]))
				{
					case 'n':
						s64[0] = 0;
					case 'o':
					case 'e':
						if (tpde->parity != s64[0])
							changed = 1;
						tpde->parity = s64[0];
						break;
					default:
						ring_bell();
						continue;
				}
				break;

			case 4:
				strcpy(s64, tpde->descr);
				phdir_add_or_edit_read("Enter description",
					screen_y, tpde->descr, PDE_DESCR_LEN + 1, &delim);
				if (delim == ESC)
					break;
				if (strcmp(tpde->descr, s64))
					changed = 1;
				break;

			case 5:
				sprintf(s64, "%u", tpde->debug_level);
				phdir_add_or_edit_read("Enter dialer debug level (0-9)",
					screen_y, s64, 1 + 1, &delim);
				if (delim == ESC)
					break;
				if (!isdigit((uchar) s64[0]))
				{
					ring_bell();
					continue;
				}
				itmp = atoi(s64);
				if (itmp != (int)tpde->debug_level)
					changed = 1;
				tpde->debug_level = itmp;
				break;

			case 6:
				s64[0] = tpde->dcdwatch;
				s64[1] = 0;
				phdir_add_or_edit_read(
#ifdef linux
					"0=off,t=terminate ecu on carrier loss,n=no change",
#else
					"0=off,1=on,t=terminate ecu on carrier loss,n=no change",
#endif
					screen_y, s64, 1 + 1, &delim);
				if (delim == ESC)
					break;
				switch (s64[0] = to_lower(s64[0]))
				{
					case '0':
#ifndef linux
					case '1':	/* does not work under Linux 1.1.59 */
#endif
					case 't':
					case 'n':
						break;
					default:
						ring_bell();
						continue;
				}
				if ((uchar) s64[0] != tpde->dcdwatch)
					changed = 1;
				tpde->dcdwatch = s64[0];
				break;

			case 7:
				s64[0] = tpde->rtscts_val;
				s64[1] = 0;
				phdir_add_or_edit_read(
					"OS/hardware dependent: see doc for other values",
					screen_y, s64, 1 + 1, &delim);
				if (delim == ESC)
					break;
				switch (s64[0] = to_lower(s64[0]))
				{
					case 'n':
					case '0':
					case '1':
					case '2':
					case '3':
					case '7':
						break;
					default:
						ring_bell();
						continue;
				}
				if ((uchar) s64[0] != tpde->rtscts_val)
					changed = 1;
				tpde->rtscts_val = s64[0];
				break;
		}

		switch (delim)		 /* process delimiter */
		{
			case CTL_L:
			case CTL_R:
				touchwin(stdscr);
				wrefresh(stdscr);
				touchwin(dirw);
				wrefresh(dirw);
				touchwin(scrw);
				wrefresh(scrw);
				touchwin(addw);
				wrefresh(addw);
				break;

			case CTL_B:
				if (input_state)
					input_state--;
				else
					input_state = PDE_ITEM_COUNT - 1;
				break;

			case ESC:
				if (edit)
				{
					dirw_bot_msg("edit exit");
					done = 1;
				}
				else
				{
					phdir_list_remove(tpde);
					if (old_curr_pde)
						curr_pde = old_curr_pde;
					else if (phdir_list_quan)
					{
						pputs("\nphdir_add_or_edit logic error\n");
						errno = -1;
						termecu(TERMECU_LOGIC_ERROR);
					}
					dirw_bot_msg("add aborted");
					ring_bell();
					aborted = 1;
					done = 1;
				}
				break;

			case XFend:
				if (edit)
					dirw_bot_msg("edit exit");
				done = 1;
				break;

			case NL:
				input_state++;
				input_state %= PDE_ITEM_COUNT;
				break;

			default:
				ring_bell();
				break;

		}
		if (edit && changed && !have_already_set_dirty)
		{
			phdir_list_set_dirty(1);
			have_already_set_dirty = 1;
		}
	}

	delwin(addw);
	addw = (WINDOW *) 0;
	touchwin(scrw);
	if (aborted)
	{
		scrw_fill_at(scrw_curr_pde_line + 1, curr_pde,
			&scrw_curr_pde_line);
	}
	else
		phdir_display(scrw_curr_pde_line, tpde, 1);
	return (!aborted);

}							 /* end of phdir_add_or_edit */

/*+-------------------------------------------------------------------------
	phdir_cmd_add(tpde)
if tpde != 0, it is an already valid pde that is to be added
else if == 0, interactive add
--------------------------------------------------------------------------*/
void
phdir_cmd_add(tpde)
PDE *tpde;
{

	if (tpde)
	{
		phdir_list_add(tpde);
	}
	else
	{
		if (!(tpde = (PDE *) calloc(1, sizeof(PDE))))
		{
			dirw_bot_msg("Out of memory -- cannot add new entry");
			ring_bell();
			return;
		}
		if (!phdir_add_or_edit(tpde, 0))	/* routine will add to list
											 * ... */
		{					 /* ... if good return */
			free((char *)tpde);
			return;
		}
	}
	phdir_list_set_dirty(1);

	curr_pde = tpde;
	scrw_fill_at(SCRW_LINES / 2, curr_pde, &scrw_curr_pde_line);

}							 /* end of phdir_cmd_add */

/*+-------------------------------------------------------------------------
	phdir_cmd_mark(tpde) - mark for redial
--------------------------------------------------------------------------*/
void
phdir_cmd_mark(tpde)
PDE *tpde;
{

	if (!tpde)
		return;
	if (!tpde->redial)
	{
		tpde->redial = 1;
		if (tpde == curr_pde)
			phdir_display_logical(scrw_curr_pde_line, curr_pde, 1);
		pde_marked_for_redial_count++;
		dirw_display_config();
	}

}							 /* end of phdir_cmd_mark */

/*+-------------------------------------------------------------------------
	phdir_cmd_unmark(tpde) - unmark for redial
--------------------------------------------------------------------------*/
void
phdir_cmd_unmark(tpde)
PDE *tpde;
{

	if (!tpde)
		return;
	if (tpde->redial)
	{
		tpde->redial = 0;
		if (tpde == curr_pde)
			phdir_display_logical(scrw_curr_pde_line, curr_pde, 1);
		pde_marked_for_redial_count--;
		dirw_display_config();
	}

}							 /* end of phdir_cmd_unmark */

/*+-------------------------------------------------------------------------
	phdir_cmd_unmark_all() - unmark for redial all PDEs
--------------------------------------------------------------------------*/
void
phdir_cmd_unmark_all()
{
	PDE *tpde;
	int y;

	tpde = phdir_list_head;
	while (tpde)
	{
		tpde->redial = 0;
		tpde = tpde->next;
	}

	for (y = 0; y < SCRW_LINES; y++)
	{
		wmove(scrw, y, 1);
		waddch(scrw, ' ');
	}
	pde_marked_for_redial_count = 0;
	dirw_display_config();

}							 /* end of phdir_cmd_unmark_all */

/*+-------------------------------------------------------------------------
	phdir_cmd_remove_oops()
--------------------------------------------------------------------------*/
void
phdir_cmd_remove_oops()
{
	if (!remove_pde)
	{
		dirw_bot_msg("no removed entry to restore");
		ring_bell();
		return;
	}
	phdir_cmd_add(remove_pde);
	phdir_list_set_dirty(remove_dirty_flag);
	remove_pde = (PDE *) 0;
}							 /* end of phdir_cmd_remove_oops */

/*+-------------------------------------------------------------------------
	phdir_cmd_remove()
--------------------------------------------------------------------------*/
void
phdir_cmd_remove()
{
	char s80[80];

	if (!check_curr_pde())
		return;

	remove_pde = curr_pde;
	remove_dirty_flag = phdir_list_dirty;
	phdir_list_remove(curr_pde);
	phdir_list_set_dirty(1);

	if (phdir_list_quan)
		scrw_fill_at(scrw_curr_pde_line + 1, curr_pde, &scrw_curr_pde_line);
	else
		scrw_fill((PDE *) 0, &scrw_curr_pde_line);

	ring_bell();
	sprintf(s80, "if you did not mean to to remove '%s', press 'o' (oops) NOW!",
		remove_pde->logical);
	dirw_bot_msg(s80);
	ring_bell();

}							 /* end of phdir_cmd_remove */

/*+-------------------------------------------------------------------------
	phdir_cmd_find()
--------------------------------------------------------------------------*/
void
phdir_cmd_find()
{
	PDE *tpde;
	char findname[LOGICAL_LEN + 1];
	UINT delim = 0;
	int y, x;
	int wgedit = 0;
	int wgpos = -1;

	dirw_bot_msg("ESC: abort  ^U: erase input");
	dirw_cmd_line_setup("", "Directory entry name to find: ");
	getyx(dirw, y, x);
	wstandout(dirw);
	while ((delim != ESC) && (delim != NL))
	{
		(void)wingets(dirw, y, x, findname, LOGICAL_DISP_LEN + 1, &delim,
			wgedit, &wgpos);
		wgedit = 1;
	}
	wstandend(dirw);
	dirw_bot_msg("");
	if ((!strlen(findname)) || (delim == ESC))
		return;

	if (!(tpde = phdir_list_search(findname, 0)))
	{
		dirw_bot_msg(errmsg);
		ring_bell();
		return;
	}
	curr_pde = tpde;
	scrw_fill_at(SCRW_LINES / 2, tpde, &scrw_curr_pde_line);

}							 /* end of phdir_cmd_find */

/*+-------------------------------------------------------------------------
	phdir_cmd_change_dir()
--------------------------------------------------------------------------*/
void
phdir_cmd_change_dir()
{
	int itmp;
	char newdirname[ECU_MAXPN];
	char buf[256];
	UINT delim = 0;
	int y, x;
	int wgedit = 0;
	int wgpos = -1;
	char *expcmd;
	extern char errmsg[];

	phdir_list_save_if_dirty();
	dirw_bot_msg("ESC: abort  ^U: erase input");
	dirw_cmd_line_setup(" Enter new directory name:", "");
	getyx(dirw, y, x);
	wstandout(dirw);
	while ((delim != ESC) && (delim != NL))
	{
		(void)wingets(dirw, y, x, buf, 70 + 1, &delim,
			wgedit, &wgpos);
		wgedit = 1;
	}
	wstandend(dirw);
	dirw_bot_msg("");
	if ((!strlen(buf)) || (delim == ESC))
		return;

	if ((buf[0] == '~') || (buf[0] == '.') || (buf[0] == '/'))
		strcpy(newdirname, buf);
	else
	{
		get_curr_dir(newdirname, sizeof(newdirname) - strlen(buf) - 2);
		strcat(newdirname, "/");
		strcat(newdirname, buf);
	}
	if (find_shell_chars(newdirname))
	{
		if (expand_wildcard_list(newdirname, &expcmd))
		{
			dirw_bot_msg(expcmd);
			ring_bell();
			return;
		}
		strncpy(newdirname, expcmd, sizeof(newdirname) - 1);
		newdirname[sizeof(newdirname) - 1] = 0;
		free(expcmd);
	}

	if (access(newdirname, 4))
	{
		if (errno == ENOENT)
		{
			if (!want_pd_create(newdirname))
			{
				dirw_bot_msg("non-existent file not created");
				ring_bell();
				return;
			}
			if ((itmp = open(newdirname, O_RDWR | O_CREAT | O_TRUNC, 0600)) >= 0)
			{
				write(itmp, phonedir_trigger, strlen(phonedir_trigger));
				close(itmp);
				dirw_bot_msg("created new (empty) directory file");
				ring_bell();
				Nap(1000L);
				goto READ_LIST;
			}
			if (errno == ENOENT)
			{
				dirw_bot_msg("directory does not exist");
				ring_bell();
				return;
			}
		}
		dirw_bot_msg(strerror(errno));
		ring_bell();
		return;
	}

  READ_LIST:
	strcpy(phonedir_name, newdirname);
	if (phdir_list_read())
	{
		dirw_bot_msg(errmsg);
		ring_bell();
		return;
	}
	curr_pde = phdir_list_head;
	scrw_fill(curr_pde, &scrw_curr_pde_line);
	if (!phdir_list_quan)
	{
		dirw_bot_msg("directory empty");
		return;
	}

}							 /* end of phdir_cmd_change_dir */

/*+-------------------------------------------------------------------------
	phdir_dial_cycle() - dial single destination or cycle through list
return 1 if connect occurs, 0 if cycle expires or interrupted
--------------------------------------------------------------------------*/
int
phdir_dial_cycle()
{
	PDE *tpde = phdir_list_head;
	UINT ans;
	int nap_decisec;
	int rtn = 0;
	int restart_rcvr = need_rcvr_restart();

	if (!tpde || !phdir_list_quan)
	{
		ring_bell();
		return (0);
	}

	if (!pde_marked_for_redial_count)
	{
		pde_dial(curr_pde);
		return (!(int)iv[0]);
	}

	kill_rcvr_process(SIGUSR1);

	ff(se, "\r\nbeginning cycle through %d marked redial entr%s\r\n",
		pde_marked_for_redial_count,
		(pde_marked_for_redial_count > 1) ? "ies" : "y");

	while (1)				 /* forever until a connect or interrupt */
	{
		if (tpde->redial)
		{
			pde_dial(tpde);
			switch ((int)iv[0])
			{
				case 0:	 /* CONNECTED */
					tpde->redial = 0;
					pde_marked_for_redial_count--;
					bell_notify(XBELL_C);
					rtn = 1;
					if (restart_rcvr)
						start_rcvr_process(1);
					return (rtn);

				case 2:	 /* INTERRUPTED */
					ff(se, "\r\ndial interrupted: abort cycle (y,n)?  ");
					sigint = 0;
					ans = 0;
					while (ans == 0)
					{
						switch (ans = to_lower(ttygetc(1)))
						{
							case 'y':
								ff(se, "YES\r\n");
								goto ABORT_CYCLE;
							case 'n':
								ff(se, "NO\r\n");
								break;
							default:
								ring_bell();
								ans = 0;
						}
					}
					break;
				case 1:	 /* FAILED TO CONNECT */
				case 3:	 /* MODEM ERROR */
				default:
					if (pde_marked_for_redial_count == 1)
						nap_decisec = nap_decisec_single;
					else
						nap_decisec = nap_decisec_multiple;
					ff(se,
						"waiting %d seconds ... 'c' to cycle, %s to abort\r\n",
						nap_decisec / 10, (kbdintr == DEL)
						? "DEL" : graphic_char_text(kbdintr, 0));
					while (nap_decisec--)
					{
						Nap(100L);
						if (ttyrdchk())
						{
							ans = (UINT) to_lower(ttygetc(1));
							ttyflush(0);
							if (ans == 'c')
								goto CONTINUE_CYCLE;
							else if (ans == (unsigned)kbdintr)
								goto ABORT_CYCLE;
							else
								ring_bell();
						}
						if (ck_sigint())
							goto ABORT_CYCLE;
					}
					break;
			}
		}
	  CONTINUE_CYCLE:
		tpde = tpde->next;
		if (!tpde)
			tpde = phdir_list_head;
	}

  ABORT_CYCLE:
	sigint = 0;
	ff(se, "redial cycle ABORTED\r\n");

	if (restart_rcvr)
		start_rcvr_process(1);
	return (rtn);

}							 /* end of phdir_dial_cycle */

/*+-------------------------------------------------------------------------
	phdir_cmd_set_wait()
--------------------------------------------------------------------------*/
void
phdir_cmd_set_wait()
{
	char buf[64];
	UINT delim = 0;
	int y, x;
	int wgpos = -1;

	dirw_bot_msg("ESC: abort  ^U: erase input");
	sprintf(buf, "(Must be greater than or equal to %d seconds)",
		NAP_DECISEC_SINGLE_MIN / 10);
	dirw_cmd_line_setup(buf, "Wait between dials when one entry marked:");
	getyx(dirw, y, x);
	sprintf(buf, "%d", nap_decisec_single / 10);
	wstandout(dirw);
	wgpos = strlen(buf);
	while ((delim != ESC) && (delim != NL))
		(void)wingets(dirw, y, x, buf, 3 + 1, &delim, 1, &wgpos);
	wstandend(dirw);
	if ((!strlen(buf)) || (delim == ESC))
	{
		dirw_bot_msg("");
		return;
	}
	nap_decisec_single = 0;
	sscanf(buf, "%d", &nap_decisec_single);
	nap_decisec_single *= 10;
	if (nap_decisec_single < NAP_DECISEC_SINGLE_MIN)
		nap_decisec_single = NAP_DECISEC_SINGLE_MIN;
	dirw_display_config();

	sprintf(buf, "(Must be greater than or equal to %d seconds)",
		NAP_DECISEC_MULTIPLE_MIN / 10);
	dirw_cmd_line_setup(buf, "Wait between dials when multiple entries marked:");
	getyx(dirw, y, x);
	sprintf(buf, "%d", nap_decisec_multiple / 10);
	wstandout(dirw);
	wgpos = strlen(buf);
	delim = 0;
	while ((delim != ESC) && (delim != NL))
		(void)wingets(dirw, y, x, buf, 3 + 1, &delim, 1, &wgpos);
	wstandend(dirw);
	if ((!strlen(buf)) || (delim == ESC))
	{
		dirw_bot_msg("");
		return;
	}
	nap_decisec_multiple = 0;
	sscanf(buf, "%d", &nap_decisec_multiple);
	nap_decisec_multiple *= 10;
	if (nap_decisec_multiple < NAP_DECISEC_MULTIPLE_MIN)
		nap_decisec_multiple = NAP_DECISEC_MULTIPLE_MIN;
	dirw_display_config();

	dirw_bot_msg("");

}							 /* end of phdir_cmd_set_wait */

/*+-------------------------------------------------------------------------
	phdir_manager()
--------------------------------------------------------------------------*/
void
phdir_manager()
{
	UINT cmd = 0;
	int done;
	char s80[80];
	int restart_rcvr = need_rcvr_restart();

	kill_rcvr_process(SIGUSR1);

	windows_start();
	dirw = window_create("dialing directory", 3, 0, 0, DIRW_LINES, DIRW_COLS);
	phonedir_name_x = 26;	 /* must be set before calling dirw_display */
	dirw_display();

	scrw = subwin(dirw, SCRW_LINES, SCRW_COLS, SCRW_TLY, SCRW_TLX);
	scrollok(scrw, 0);
	if (!phdir_list_quan)
	{
		if (phdir_list_read())
		{
			dirw_bot_msg(errmsg);
			ring_bell();
		}
		else if (!phdir_list_quan)
		{
			dirw_bot_msg("directory empty");
			ring_bell();
		}
	}

	if (phdir_list_quan)
	{
		if (curr_pde)
			scrw_fill_at(scrw_curr_pde_line, curr_pde, &scrw_curr_pde_line);
		else
		{
			curr_pde = phdir_list_head;
			scrw_fill(curr_pde, &scrw_curr_pde_line);
		}
	}

	phdir_list_set_dirty(-1);

	done = 0;
	while (!done)
	{
		cmd = dirw_get_cmd();
		if ((cmd != 'o') && (remove_pde))
		{
			free((char *)remove_pde);
			remove_pde = (PDE *) 0;
		}

		switch (cmd)
		{
			case XFcurdn:
			case 'd':
			case 'j':
				phdir_cmd_down();
				break;

			case XFcurup:
			case 'u':
			case 'k':
				phdir_cmd_up();
				break;

			case XFpgup:
			case CTL_U:
			case CTL_B:
				phdir_cmd_pgup();
				break;

			case XFpgdn:
			case CTL_F:
			case CTL_D:
				phdir_cmd_pgdn();
				break;

			case 's':
				phdir_cmd_save();
				break;

			case 'm':
				phdir_cmd_mark(curr_pde);
				break;
			case 'M':
				phdir_cmd_unmark(curr_pde);
				break;
			case 'U':
				phdir_cmd_unmark_all();
				break;
			case 'c':
				phdir_cmd_change_dir();
				break;

			case 'w':
				phdir_cmd_set_wait();
				break;

			case CRET:
			case NL:
			case XFend:
				cmd = NL;
				if (!check_curr_pde())
					break;
				phdir_list_save_if_dirty();
				wrefresh(dirw);
				delwin(scrw);
				delwin(dirw);
				windows_end(1);
				dirw = (WINDOW *) 0;
				scrw = (WINDOW *) 0;
				phdir_dial_cycle();
				done = 1;
				if (restart_rcvr)
					start_rcvr_process(1);
				break;

			case ESC:
			case CTL_C:
				cmd = 'q';
			case 'q':
				done = 1;
				continue;

			case '/':
			case 'f':
				phdir_cmd_find();
				break;

			case 'r':
				phdir_cmd_remove();
				break;
			case 'o':
				phdir_cmd_remove_oops();
				break;

			case 'a':
				phdir_cmd_add((PDE *) 0);
				break;

			case 'e':
				phdir_add_or_edit(curr_pde, 1);
				break;

			case CTL_L:
			case CTL_R:
				touchwin(stdscr);
				wrefresh(stdscr);
				touchwin(dirw);
				wrefresh(dirw);
				touchwin(scrw);
				wrefresh(scrw);
				break;

			default:
				sprintf(s80, "invalid command: %s",
					(cmd < 0x80) ? graphic_char_text(cmd, 0) : "?");
				dirw_bot_msg(s80);
				ring_bell();
				break;
		}
	}
	sigint = 0;
	if (cmd == NL)
		return;

	phdir_list_save_if_dirty();
	wrefresh(dirw);
	delwin(scrw);
	delwin(dirw);
	windows_end(0);
	dirw = (WINDOW *) 0;
	scrw = (WINDOW *) 0;
	redisplay_rcvr_screen();

	if (restart_rcvr)
		start_rcvr_process(0);

}							 /* end of phdir_manager */

/* end of ecuphdir.c */
/* vi: set tabstop=4 shiftwidth=4: */
