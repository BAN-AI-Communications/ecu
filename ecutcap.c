/*+-------------------------------------------------------------------------
	ecutcap.c -- termcap stuff
	wht@wht.net

  Defined functions:
	tcap_blink_off()
	tcap_blink_on()
	tcap_bold_off()
	tcap_bold_on()
	tcap_clear_area_char(count, clrch)
	tcap_clear_screen()
	tcap_curbotleft()
	tcap_curleft(count)
	tcap_curright(count)
	tcap_cursor(y, x)
	tcap_delete_chars(count)
	tcap_delete_lines(count)
	tcap_display()
	tcap_draw_box(y, x, height, width, title, title_x)
	tcap_draw_box_primitive(y, x, height, width)
	tcap_eeod()
	tcap_eeol()
	tcap_horiz_rule(count)
	tcap_init()
	tcap_insert_chars(count)
	tcap_insert_lines(count)
	tcap_orig_pair()
	tcap_putc(character)
	tcap_stand_end()
	tcap_stand_out()
	tcap_underscore_off()
	tcap_underscore_on()
	tcap_vbell()
	tcap_vertical_rule(y, x, count)

--------------------------------------------------------------------------*/
/*+:EDITS:*/
/*:04-26-2000-11:15-wht@bob-RELEASE 4.42 */
/*:12-21-1997-13:50-wht@sidonia-add CFG_UseSetupterm and CFG_UseStructWinsize *//*:12-19-1997-21:44-wht@kepler-add MUCH more to kbd_test */
/*:03-16-1997-03:37-rll@felton.felton.ca.us-fix boxes for SCO Products */
/*:01-24-1997-02:37-wht@yuriatin-SOURCE RELEASE 4.00 */
/*:10-20-1996-20:33-wht@yuriatin-move LINES/COLUMNS override to common code */
/*:09-24-1996-14:26-wht@yuriatin-tcap_putc needs pid test for output dest */
/*:09-12-1996-05:24-wht@kepler-TIOCGWINSZ for linux */
/*:09-11-1996-20:00-wht@yuriatin-3.48-major telnet,curses,structural overhaul */
/*:09-06-1996-20:15-wht@kepler-exhaustive tcap tests */
/*:09-06-1996-02:42-wht@kepler-linux tputc takes int not char */
/*:09-05-1996-16:56-wht@kepler-CFG_UseSetupterm optimization */
/*:07-03-1996-14:27-wht@kepler-tcap_curbotleft emits CR/LF if not initialized */
/*:07-03-1996-13:56-wht@kepler-better doc of terminal db shortcomings */
/*:06-02-1996-17:19-wht@kepler-check all tcap funcs */
/*:05-19-1996-16:07-wht@gyro-protect all entries against tcap_initialized */
/*:12-06-1995-13:30-wht@n4hgf-termecu w/errno -1 consideration */
/*:11-23-1995-11:20-wht@kepler-source control 3.37 for tsx-11 */
/*:11-14-1995-10:23-wht@kepler-3.37.80-source control point: SOCKETS */
/*:10-15-1995-15:20-wht@calvin-logic error in tcap_init */
/*:10-14-1995-23:03-wht@kepler-termecu may call tcap_curbotleft before init */
/*:09-16-1995-16:31-root@kepler-add tcap_display */
/*:03-11-1995-17:46-wht@kepler-Linux wants different 3rd arg to tputs */
/*:01-12-1995-15:19-wht@n4hgf-apply Andrew Chernov 8-bit clean+FreeBSD patch */
/*:05-04-1994-04:39-wht@n4hgf-ECU release 3.30 */
/*:01-06-1994-04:21-wht@n4hgf-clean up LINUX tcap_putc */
/*:01-04-1994-05:32-wht@n4hgf-modify Linux port to balance braces */
/*:01-04-1994-01:43-wht@n4hgf-protect against NULL tc_blink_on/off */
/*:11-12-1993-11:00-wht@n4hgf-Linux changes by bob@vancouver.zadall.com */
/*:07-23-1993-13:43-wht@n4hgf-tcap_clear_area_char had same buggy tputs call */
/*:07-17-1993-12:19-wht@n4hgf-tcap_curleft and right had buggy tputs calls */
/*:01-11-1993-16:04-wht@n4hgf-vbell defaults to bell */
/*:09-10-1992-13:59-wht@n4hgf-ECU release 3.20 */
/*:08-22-1992-15:38-wht@n4hgf-ECU release 3.20 BETA */
/*:03-10-1992-13:25-wht@n4hgf2-quick sanity check on ttype features */
/*:02-24-1992-06:50-root@n4hgf-getenv COLUMNS not COLS */
/*:07-25-1991-12:56-wht@n4hgf-ECU release 3.10 */
/*:05-01-1991-04:05-wht@n4hgf-try to catch tbetz tc= infinite loop early */
/*:03-20-1991-16:25-root@n4hgf-environment LINES/COLS overrides termcap li/co */
/*:11-28-1990-14:52-wht@n4hgf-tcap support for non-ansi console */
/*:08-14-1990-20:40-wht@n4hgf-ecu3.00-flush old edit history */

#if defined(linux) || defined(__FreeBSD__)
#include <ncurses.h>
#include <term.h>
#endif
#if defined(SCO32v4) || defined(SVR4)
#include <curses.h>
#endif
#include "ecu.h"
#include "ecukey.h"
#include "pc_scr.h"

UINT tcap_LINES;
UINT tcap_COLS;

static char *tc_blink_on = "";
static char *tc_blink_off = "";
static char *tc_bold_on = "";
static char *tc_bold_off = "";
static char *tc_clear = "";
static char *tc_curleft = "";
static char *tc_curright = "";
static char *tc_delchar = "";
static char *tc_delline = "";
static char *tc_eeod = "";
static char *tc_eeol = "";
static char *tc_inschar = "";
static char *tc_insline = "";
static char *tc_move = "";
static char *tc_standout = "";
static char *tc_standend = "";
static char *tc_underscore_on = "";
static char *tc_underscore_off = "";
static char *tc_vbell = "";

static int tcap_initialized = 0;

#if !defined(linux) && !defined(__FreeBSD__)
static char tc_strbuf[768];	 /* absolutely blunderous overkill */
#endif
static int tc_standout_width;

char *tgetstr();
char *tgoto();
char *getenv();

void tcap_cursor();
void tcap_stand_out();
void tcap_stand_end();

#if defined(linux) || defined(__FreeBSD__)
#define TCAP_PUTC_TYPE_IS_INT
#define TCAP_PUTC_TYPE int
#define TCAP_PUTC_ARG_TYPE int
TCAP_PUTC_TYPE tcap_putc(TCAP_PUTC_ARG_TYPE);

#else
#define TCAP_PUTC_TYPE void
#define TCAP_PUTC_ARG_TYPE int
TCAP_PUTC_TYPE tcap_putc();

#endif

/*+-------------------------------------------------------------------------
	get_winsize(prows,pcols)
--------------------------------------------------------------------------*/
#if defined(CFG_UseStructWinsize)
get_winsize(prows,pcols)
int *prows;
int *pcols;
{

		struct winsize winsz;

#if !defined(CFG_WinsizeIoctl) && defined(TXGETWIN)
#define CFG_WinsizeIoctl TXGETWIN
#endif
#if !defined(CFG_WinsizeIoctl) && defined(TIOCGWINSZ)
#define CFG_WinsizeIoctl TIOCGWINSZ
#endif
#if !defined(CFG_WinsizeIoctl)
#	include "porting.attention.needed.here"
#endif

	if(!ioctl (1, CFG_WinsizeIoctl, (char *)&winsz))
	{
		*prows = winsz.ws_row;
		*pcols = winsz.ws_col;
		return(0);
	}

	return(-1);

}	/* end of get_winsize */
#endif /* CFG_UseStructWinsize */

/*+-------------------------------------------------------------------------
	tcap_init() - get termcap variables
--------------------------------------------------------------------------*/
void
tcap_init()
{
	int itmp;
	int initialized = 0;
	char *cp;

#ifdef CFG_UseSetupterm
	int result;

#else
	char termbuf[1024];

#endif /* CFG_UseSetupterm */

	if (!ttype || !*ttype)
	{
		ff(se, "invalid or missing TERM environment variable\r\n");
		errno = -1;
		termecu(TERMECU_CURSES_ERROR);
	}

#ifdef CFG_UseSetupterm
	setupterm(ttype, 1, &result);
	if (result == 1)
	{
#ifdef CFG_UseStructWinsize
		if(get_winsize(&tcap_LINES,&tcap_COLS))
		{
			tcap_LINES = lines;
			tcap_COLS = columns;
		}
#else
		tcap_LINES = lines;
		tcap_COLS = columns;
#endif /* CFG_UseStructWinsize */

		tc_standout_width = label_width;
		tc_standout = enter_standout_mode;
		tc_standend = exit_standout_mode;
		tc_clear = clear_screen;
		tc_curleft = cursor_left;
		tc_curright = cursor_right;
		tc_delchar = delete_character;
		tc_delline = delete_line;
		tc_eeod = clr_eos;
		tc_eeol = clr_eol;
		tc_inschar = insert_character;
		tc_insline = insert_line;
		tc_move = cursor_address;
		tc_vbell = bell;
		tc_underscore_on = enter_underline_mode;
		tc_underscore_off = exit_underline_mode;
		if (!tc_underscore_on || !tc_underscore_off)
		{
			tc_underscore_on = tc_standout;
			tc_underscore_off = tc_standend;
		}
		tc_bold_on = enter_bold_mode;	/* for now */
		tc_bold_off = exit_attribute_mode;	/* for now */
		if (!tc_bold_on || !tc_bold_off)
		{
			tc_bold_on = tc_standout;
			tc_bold_off = tc_standend;
		}
		tc_blink_on = enter_blink_mode;
		tc_blink_off = exit_attribute_mode;
		if (!tc_blink_on || !tc_blink_off)
		{
			tc_blink_on = tc_standout;
			tc_blink_off = tc_standend;
		}
		initialized = 1;
	}
#else /* CFG_UseSetupterm */
	if (tgetent(termbuf, ttype) > 0)
	{
		char *cp2;

#ifdef CFG_UseStructWinsize
		if(get_winsize(&tcap_LINES,&tcap_COLS))
		{
			tcap_LINES = tgetnum("li");
			tcap_COLS = tgetnum("co");
		}
#else
		tcap_LINES = tgetnum("li");
		tcap_COLS = tgetnum("co");
#endif /* CFG_UseStructWinsize */

		if ((tc_standout_width = tgetnum("sg")) < 0)
			tc_standout_width = 0;
		cp = tc_strbuf;
		tc_standout = tgetstr("so", &cp);
		tc_standend = tgetstr("se", &cp);
		if (!tc_standout || !tc_standend)
		{
			tc_standout = "";
			tc_standend = "";
		}
		tc_clear = (cp2 = tgetstr("cl", &cp)) ? cp2 : "";
		tc_curleft = tgetstr("kl", &cp);
		if (!tc_curleft)
			tc_curleft = "\10";
		tc_curright = tgetstr("kr", &cp);
		if (!tc_curright)
			tc_curright = " ";
		tc_delchar = (cp2 = tgetstr("dc", &cp)) ? cp2 : "";
		tc_delline = (cp2 = tgetstr("dl", &cp)) ? cp2 : "";
		tc_eeod = (cp2 = tgetstr("cd", &cp)) ? cp2 : "";
		tc_eeol = (cp2 = tgetstr("ce", &cp)) ? cp2 : "";
		tc_inschar = (cp2 = tgetstr("ic", &cp)) ? cp2 : "";
		tc_insline = (cp2 = tgetstr("al", &cp)) ? cp2 : "";
		tc_move = (cp2 = tgetstr("cm", &cp)) ? cp2 : "";
		tc_vbell = tgetstr("vb", &cp);
		if (!tc_vbell)
			tc_vbell = "\7";
		tc_underscore_on = tgetstr("us", &cp);
		tc_underscore_off = tgetstr("ue", &cp);
		if (!tc_underscore_on || !tc_underscore_off)
		{
			tc_underscore_on = tc_standout;
			tc_underscore_off = tc_standend;
		}
		tc_bold_on = tc_standout;	/* for now */
		tc_bold_off = tc_standend;	/* for now */
		if (!tc_bold_on || !tc_bold_off)
		{
			tc_bold_on = tc_standout;
			tc_bold_off = tc_standend;
		}
		tc_blink_on = tgetstr("mb", &cp);	/* "XENIX extension" */
		tc_blink_off = tgetstr("me", &cp);	/* "XENIX extension" */
		if (!tc_blink_on || !tc_blink_off)
		{
			tc_blink_on = tc_standout;
			tc_blink_off = tc_standend;
		}
		initialized = 1;
	}
#endif

	if (!initialized)
	{
		ff(se, "Cannot find terminal type '%s' or entry in error\r\n",
			ttype);
		errno = -1;
		termecu(TERMECU_CURSES_ERROR);
	}

	/*
	 * let LINES/COLUMNS environment variables override tcap
	 */
	if (cp = getenv("LINES"))	/* environment override ... */
		tcap_LINES = atoi(cp);	/* ... for termcap systems */
	if (cp = getenv("COLUMNS"))
		tcap_COLS = atoi(cp);

	if (!tc_clear || !*tc_clear || !tc_move || !*tc_move)
	{
		pputs("Terminal type '%s' does not have the beef.\n",
			ttype);
		if (!tc_clear || !*tc_clear)
			pputs("  No clear screen (`cl' or `ed')\n");
		if (!tc_move || !*tc_move)
			pputs("  No cursor positioning (`cm or `cup'')\n");
		pputs("The first `code' is the TERMCAP identifier for the feature\n");
		pputs("while the second `code' is the TERMINFO identifier.\n");
		pputs("Use a screen-oriented terminal (or fix your\n");
		pputs("terminal database entry) and try again.\n");
		errno = -1;
		termecu(TERMECU_CURSES_ERROR);
	}
	if (!tc_standout || !tc_standend || !*tc_standout || !*tc_standend ||
		!tc_insline || !*tc_insline || !tc_delline || !*tc_delline ||
		!tc_inschar || !*tc_inschar || !tc_delchar || !*tc_delchar ||
		!tc_curleft || !*tc_curleft ||
		!tc_curright || !*tc_curright ||
		!tc_eeod || !*tc_eeod ||
		!tc_eeol || !*tc_eeol ||
		!tc_move || !*tc_move ||
		!tc_underscore_on || !*tc_underscore_on ||
		!tc_underscore_off || !*tc_underscore_off)
	{
		pprintf("Warning: Terminal type '%s' has\n", ttype);
		if (!tc_standout || !tc_standend || !*tc_standout || !*tc_standend)
			pputs("  No standout/standend (`so')\n");
		if (!tc_insline || !*tc_insline)
			pputs("  No insert line (`al' or `il1')\n");
		if (!tc_inschar || !*tc_inschar)
			pputs("  No insert character (`ic' or `ich1')\n");
		if (!tc_delline || !*tc_delline)
			pputs("  No delete line (`dl' or `dl1')\n");
		if (!tc_delchar || !*tc_delchar)
			pputs("  No delete character (`dc' or `dch1')\n");
		if (!tc_curleft || !*tc_curleft)
			pputs("  No cursor left (`kl' or `')\n");
		if (!tc_curright || !*tc_curright)
			pputs("  No cursor right (`nd' or `')\n");
		if (!tc_eeod || !*tc_eeod)
			pputs("  No erase to end of display (`cd' or `ed')\n");
		if (!tc_eeol || !*tc_eeol)
			pputs("  No erase to end of line (`ce' or `el')\n");

		pputs("The first `code' is the TERMCAP identifier for the feature\n");
		pputs("while the second `code' is the TERMINFO identifier.\n");
		pputs("We will continue, though your screen may be badly painted.\7\n");
		pputs("Press RETURN to continue, ESCape to exit: ");
		itmp = ttygetc(0);
		pputs("\n");
		if (itmp != 0x0D)
			termecu(TERMECU_CURSES_ERROR);
	}
#ifdef CFG_UseSetupterm
	if (enter_ca_mode)
		ff(se, "%s", enter_ca_mode);
#endif /* CFG_UseSetupterm */
	tcap_initialized = 1;

}							 /* end of tcap_init */

/*+-------------------------------------------------------------------------
	tcap_display()
--------------------------------------------------------------------------*/
void
tcap_display()
{
	hex_dump(tc_blink_on, strlen(tc_blink_on), "blink_on", 1);
	hex_dump(tc_blink_off, strlen(tc_blink_off), "blink_off", 1);
	hex_dump(tc_bold_on, strlen(tc_bold_on), "bold_on", 1);
	hex_dump(tc_bold_off, strlen(tc_bold_off), "bold_off", 1);
	hex_dump(tc_clear, strlen(tc_clear), "clear", 1);
	hex_dump(tc_curleft, strlen(tc_curleft), "curleft", 1);
	hex_dump(tc_curright, strlen(tc_curright), "curright", 1);
	hex_dump(tc_delchar, strlen(tc_delchar), "delchar", 1);
	hex_dump(tc_delline, strlen(tc_delline), "delline", 1);
	hex_dump(tc_eeod, strlen(tc_eeod), "eeod", 1);
	hex_dump(tc_eeol, strlen(tc_eeol), "eeol", 1);
	hex_dump(tc_inschar, strlen(tc_inschar), "inschar", 1);
	hex_dump(tc_insline, strlen(tc_insline), "insline", 1);
	hex_dump(tc_move, strlen(tc_move), "move", 1);
	hex_dump(tc_standout, strlen(tc_standout), "standout", 1);
	hex_dump(tc_standend, strlen(tc_standend), "standend", 1);
	hex_dump(tc_underscore_on, strlen(tc_underscore_on), "underscore_on", 1);
	hex_dump(tc_underscore_off, strlen(tc_underscore_off), "underscore_off", 1);
	hex_dump(tc_vbell, strlen(tc_vbell), "vbell", 1);

	if (tc_blink_on)
		hex_dump(tc_blink_on, strlen(tc_blink_on), "tc_blink_on", 1);
	if (tc_blink_off)
		hex_dump(tc_blink_off, strlen(tc_blink_off), "tc_blink_off", 1);
	if (tc_bold_on)
		hex_dump(tc_bold_on, strlen(tc_bold_on), "tc_bold_on", 1);
	if (tc_bold_off)
		hex_dump(tc_bold_off, strlen(tc_bold_off), "tc_bold_off", 1);
	if (tc_clear)
		hex_dump(tc_clear, strlen(tc_clear), "tc_clear", 1);
	if (tc_curleft)
		hex_dump(tc_curleft, strlen(tc_curleft), "tc_curleft", 1);
	if (tc_curright)
		hex_dump(tc_curright, strlen(tc_curright), "tc_curright", 1);
	if (tc_delchar)
		hex_dump(tc_delchar, strlen(tc_delchar), "tc_delchar", 1);
	if (tc_delline)
		hex_dump(tc_delline, strlen(tc_delline), "tc_delline", 1);
	if (tc_eeod)
		hex_dump(tc_eeod, strlen(tc_eeod), "tc_eeod", 1);
	if (tc_eeol)
		hex_dump(tc_eeol, strlen(tc_eeol), "tc_eeol", 1);
	if (tc_inschar)
		hex_dump(tc_inschar, strlen(tc_inschar), "tc_inschar", 1);
	if (tc_insline)
		hex_dump(tc_insline, strlen(tc_insline), "tc_insline", 1);
	if (tc_move)
		hex_dump(tc_move, strlen(tc_move), "tc_move", 1);
	if (tc_standout)
		hex_dump(tc_standout, strlen(tc_standout), "tc_standout", 1);
	if (tc_standend)
		hex_dump(tc_standend, strlen(tc_standend), "tc_standend", 1);
	if (tc_underscore_on)
	{
		hex_dump(tc_underscore_on, strlen(tc_underscore_on),
			"tc_underscore_on", 1);
	}
	if (tc_underscore_off)
	{
		hex_dump(tc_underscore_off, strlen(tc_underscore_off),
			"tc_underscore_off", 1);
	}
	if (tc_vbell)
		hex_dump(tc_vbell, strlen(tc_vbell), "tc_vbell", 1);
}							 /* end of tcap_display */

/*+-------------------------------------------------------------------------
	tcap_putc(character) - utility routine for tputs
--------------------------------------------------------------------------*/
TCAP_PUTC_TYPE
tcap_putc(character)
TCAP_PUTC_ARG_TYPE character;
{
	char ch = character;

	if(getpid() == shm->rcvr_pid)
		rcvrdisp(&ch, 1);
	else
		fputc(ch,se);
#ifdef TCAP_PUTC_TYPE_IS_INT
	return (0);
#endif
}							 /* end of tcap_putc */

/*+-------------------------------------------------------------------------
	tcap_horiz_rule(count) - horizontal rule starting at current position
--------------------------------------------------------------------------*/
void
tcap_horiz_rule(count)
int count;
{
	if (!tcap_initialized)
		return;
	while (count--)
		tcap_putc(sHR);
	rcvrdisp_actual2();
}							 /* end of tcap_horiz_rule */

/*+-------------------------------------------------------------------------
	tcap_vertical_rule(y,x,count) - vertical rule starting at y,x
--------------------------------------------------------------------------*/
void
tcap_vertical_rule(y, x, count)
int y;
int x;
int count;
{

	if (!tcap_initialized)
		return;
	while (count--)
	{
		tcap_cursor(y++, x);
		tcap_putc(sVR);
	}
	rcvrdisp_actual2();

}							 /* end of tcap_vertical_rule */

/*+-------------------------------------------------------------------------
	tcap_draw_box_primitive(y,x,height,width) - ruled box
--------------------------------------------------------------------------*/
void
tcap_draw_box_primitive(y, x, height, width)
int y;
int x;
int height;
int width;
{
	int i;

	if (!tcap_initialized)
		return;
	tcap_cursor(y, x);
	tcap_putc(sTL);
	if ((i = width - 2) > 0)
		tcap_horiz_rule(i);
	tcap_putc(sTR);
	if ((i = height - 2) > 0)
	{
		tcap_vertical_rule(y + 1, x + width - 1, i);
		tcap_vertical_rule(y + 1, x, i);
	}
	tcap_cursor(y + height - 1, x);
	tcap_putc(sBL);
	if ((i = width - 2) > 0)
		tcap_horiz_rule(i);
	tcap_putc(sBR);
	rcvrdisp_actual2();

}							 /* end of tcap_draw_box_primitive */

/*+-------------------------------------------------------------------------
	tcap_draw_box(y,x,height,width,title,title_x)
--------------------------------------------------------------------------*/
void
tcap_draw_box(y, x, height, width, title, title_x)
int y;
int x;
int height;
int width;
char *title;
int title_x;
{
	int stand = (title_x < 0);

	if (!tcap_initialized)
		return;
	if (stand)
		title_x = -title_x;

	tcap_draw_box_primitive(y, x, height, width);
	tcap_cursor(y, x + title_x);
	tcap_putc('[');
	if (stand)
		tcap_stand_out();
	ff(se, " %s ", title);
	if (stand)
		tcap_stand_end();
	tcap_putc(']');
	rcvrdisp_actual2();

}							 /* end of tcap_draw_box */

/*+-------------------------------------------------------------------------
	tcap_cursor(y,x)
--------------------------------------------------------------------------*/
void
tcap_cursor(y, x)
UINT y;
UINT x;
{
	if (!tcap_initialized)
		return;
	if (y >= tcap_LINES)
		y = tcap_LINES - 1;
	if (x >= tcap_COLS)
		x = tcap_COLS - 1;
	tputs(tgoto(tc_move, x, y), 1, tcap_putc);
	rcvrdisp_actual2();

}							 /* end of tcap_cursor */

/*+-------------------------------------------------------------------------
	tcap_curleft(count) - move cursor left
--------------------------------------------------------------------------*/
void
tcap_curleft(count)
int count;
{
	if (!tcap_initialized)
		return;
	while (count--)
		tputs(tc_curleft, 1, tcap_putc);
	rcvrdisp_actual2();
}							 /* end of tcap_curleft */

/*+-------------------------------------------------------------------------
	tcap_curright(count) - move cursor right
--------------------------------------------------------------------------*/
void
tcap_curright(count)
int count;
{
	if (!tcap_initialized)
		return;
	while (count--)
		tputs(tc_curright, 1, tcap_putc);
	rcvrdisp_actual2();
}							 /* end of tcap_curright */

/*+-------------------------------------------------------------------------
	tcap_curbotleft()
--------------------------------------------------------------------------*/
void
tcap_curbotleft()
{

	if (!tcap_initialized)
	{
		tcap_putc('\r');
		tcap_putc('\n');
		return;
	}
	tcap_cursor(tcap_LINES - 1, 0);
}							 /* end of tcap_curbotleft */

/*+-------------------------------------------------------------------------
	tcap_insert_lines(count)
--------------------------------------------------------------------------*/
void
tcap_insert_lines(count)
int count;
{
	if (!tcap_initialized)
		return;
	if (count && *tc_insline)
	{
		while (count--)
			tputs(tc_insline, 1, tcap_putc);
		rcvrdisp_actual2();
	}
}							 /* end of tcap_insert_lines */

/*+-------------------------------------------------------------------------
	tcap_delete_lines(count)
--------------------------------------------------------------------------*/
void
tcap_delete_lines(count)
int count;
{
	if (!tcap_initialized)
		return;
	if (count && *tc_delline)
	{
		while (count--)
			tputs(tc_delline, 1, tcap_putc);
		rcvrdisp_actual2();
	}
}							 /* end of tcap_delete_lines */

/*+-------------------------------------------------------------------------
	tcap_insert_chars(count)
--------------------------------------------------------------------------*/
void
tcap_insert_chars(count)
int count;
{
	if (!tcap_initialized)
		return;
	if (count && *tc_inschar)
	{
		while (count--)
			tputs(tc_inschar, 1, tcap_putc);
		rcvrdisp_actual2();
	}
}							 /* end of tcap_insert_chars */

/*+-------------------------------------------------------------------------
	tcap_delete_chars(count)
--------------------------------------------------------------------------*/
void
tcap_delete_chars(count)
int count;
{
	if (!tcap_initialized)
		return;
	if (count && *tc_delchar)
	{
		while (count--)
			tputs(tc_delchar, 1, tcap_putc);
		rcvrdisp_actual2();
	}
}							 /* end of tcap_delete_chars */

/*+-------------------------------------------------------------------------
	tcap_vbell() - output visual bell
--------------------------------------------------------------------------*/
void
tcap_vbell()
{
	if (!tcap_initialized)
		return;
	if (*tc_vbell)
	{
		tputs(tc_vbell, 1, tcap_putc);
		rcvrdisp_actual2();
	}
}							 /* end of tcap_vbell */

/*+-------------------------------------------------------------------------
	tcap_clear_screen()
--------------------------------------------------------------------------*/
void
tcap_clear_screen()
{
	if (!tcap_initialized)
		return;
	if (*tc_clear)
	{
		tputs(tc_clear, 1, tcap_putc);
		rcvrdisp_actual2();
	}
}							 /* end of tcap_clear_screen */

/*+-------------------------------------------------------------------------
	tcap_eeol() - erase to end of line
--------------------------------------------------------------------------*/
void
tcap_eeol()
{
	if (!tcap_initialized)
		return;
	if (*tc_eeol)
	{
		tputs(tc_eeol, 1, tcap_putc);
		rcvrdisp_actual2();
	}
}							 /* end of tcap_eeol */

/*+-------------------------------------------------------------------------
	tcap_eeod() - erase to end of display
--------------------------------------------------------------------------*/
void
tcap_eeod()
{
	if (!tcap_initialized)
		return;
	if (*tc_eeod)
	{
		tputs(tc_eeod, 1, tcap_putc);
		rcvrdisp_actual2();
	}
}							 /* end of tcap_eeod */

/*+-------------------------------------------------------------------------
	tcap_stand_out()
--------------------------------------------------------------------------*/
void
tcap_stand_out()
{
	if (!tcap_initialized)
		return;
	if (*tc_standout)		 /* && (tc_standout_width == 0)) */
	{
		tputs(tc_standout, 1, tcap_putc);
		rcvrdisp_actual2();
	}
}							 /* end of tcap_stand_out */

/*+-------------------------------------------------------------------------
	tcap_stand_end()
--------------------------------------------------------------------------*/
void
tcap_stand_end()
{
	if (!tcap_initialized)
		return;
	if (*tc_standend)		 /* && (tc_standout_width == 0)) */
	{
		tputs(tc_standend, 1, tcap_putc);
		rcvrdisp_actual2();
	}
}							 /* end of tcap_stand_end */

/*+-------------------------------------------------------------------------
	tcap_bold_on()
--------------------------------------------------------------------------*/
void
tcap_bold_on()
{
	if (!tcap_initialized)
		return;
	if (*tc_bold_on)
		tputs(tc_bold_on, 1, tcap_putc);
	else if (*tc_standout)	 /* && (tc_standout_width == 0)) */
		tputs(tc_standout, 1, tcap_putc);
	rcvrdisp_actual2();
}							 /* end of tcap_bold_on */

/*+-------------------------------------------------------------------------
	tcap_bold_off()
--------------------------------------------------------------------------*/
void
tcap_bold_off()
{
	if (!tcap_initialized)
		return;
	if (*tc_bold_off)
		tputs(tc_bold_off, 1, tcap_putc);
	else if (*tc_standend)	 /* && (tc_standout_width == 0)) */
		tputs(tc_standend, 1, tcap_putc);
	rcvrdisp_actual2();
}							 /* end of tcap_bold_off */

/*+-------------------------------------------------------------------------
	tcap_underscore_on()
--------------------------------------------------------------------------*/
void
tcap_underscore_on()
{
	if (!tcap_initialized)
		return;
	if (*tc_underscore_on)
		tputs(tc_underscore_on, 1, tcap_putc);
	else if (*tc_standout)	 /* && (tc_standout_width == 0)) */
		tputs(tc_standout, 1, tcap_putc);
	rcvrdisp_actual2();
}							 /* end of tcap_underscore_on */

/*+-------------------------------------------------------------------------
	tcap_underscore_off()
--------------------------------------------------------------------------*/
void
tcap_underscore_off()
{
	if (!tcap_initialized)
		return;
	if (*tc_underscore_off)
		tputs(tc_underscore_off, 1, tcap_putc);
	else if (*tc_standend)	 /* && (tc_standout_width == 0)) */
		tputs(tc_standend, 1, tcap_putc);
	rcvrdisp_actual2();
}							 /* end of tcap_underscore_off */

/*+-------------------------------------------------------------------------
	tcap_blink_on()
--------------------------------------------------------------------------*/
void
tcap_blink_on()
{
	if (!tcap_initialized)
		return;
	if (*tc_blink_on)
		tputs(tc_blink_on, 1, tcap_putc);
	else if (*tc_standout)	 /* && (tc_standout_width == 0)) */
		tputs(tc_standout, 1, tcap_putc);
	rcvrdisp_actual2();
}							 /* end of tcap_blink_on */

/*+-------------------------------------------------------------------------
	tcap_blink_off()
--------------------------------------------------------------------------*/
void
tcap_blink_off()
{
	if (!tcap_initialized)
		return;
	if (*tc_blink_off)
		tputs(tc_blink_off, 1, tcap_putc);
	else if (*tc_standend)	 /* && (tc_standout_width == 0)) */
		tputs(tc_standend, 1, tcap_putc);
	rcvrdisp_actual2();
}							 /* end of tcap_blink_off */

/*+-------------------------------------------------------------------------
	tcap_clear_area_char(count,clrch)
--------------------------------------------------------------------------*/
void
tcap_clear_area_char(count, clrch)
int count;
int clrch;
{
	int itmp = count;

	if (!tcap_initialized)
		return;
	while (itmp--)
		tcap_putc(clrch);
	itmp = count;
	while (itmp--)
		tputs(tc_curleft, 1, tcap_putc);
	rcvrdisp_actual2();

}							 /* end of tcap_clear_area_char */

/*+-------------------------------------------------------------------------
	tcap_orig_pair() - output initial color pair
--------------------------------------------------------------------------*/
#ifdef __FreeBSD__
void
tcap_orig_pair()
{
	if (!tcap_initialized)
		return;
	if (orig_pair && *orig_pair)
	{
		tputs(orig_pair, 1, tcap_putc);
		rcvrdisp_actual2();
	}
}							 /* end of tcap_orig_pair */
#endif

/* end of ecutcap.c */
/* vi: set tabstop=4 shiftwidth=4: */
