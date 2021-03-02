/*+-------------------------------------------------------------------------
	ecuwinutil.c - curses window utilities
	wht@wht.net

  Defined functions:
	clear_area(win, y, x, len)
	clear_area_char(win, y, x, len, fillchar)
	winbox(win)
	window_create(title, title_x, tly, tlx, lines, cols)
	window_setup(win, title, title_x)
	windows_end(botleft_flag)
	windows_end_signal()
	windows_start()
	winget_single(win, nondelim_list, delim_list)
	wingets(win, y, x, buf, bufsize, delim, edit, pwgpos)

  It's hard to get ivory in Africa, but in Alabama the
  Tuscaloosa.  -- Groucho

--------------------------------------------------------------------------*/
/*+:EDITS:*/
/*:04-26-2000-11:15-wht@bob-RELEASE 4.42 */
/*:05-12-1997-17:38-wht@kepler-xclnt intl port still screwed winget_single */
/*:01-24-1997-02:37-wht@yuriatin-SOURCE RELEASE 4.00 */
/*:09-11-1996-20:00-wht@yuriatin-3.48-major telnet,curses,structural overhaul */
/*:09-07-1996-16:18-wht@kepler-CFG_UseACS coined for use outside of config */
/*:09-05-1996-17:33-wht@kepler-try retiring use of typeahead(-1) */
/*:08-22-1996-14:13-wht@fep-decommitted AT chars + fix SVR4 */
/*:12-06-1995-13:30-wht@n4hgf-termecu w/errno -1 consideration */
/*:11-23-1995-11:20-wht@kepler-source control 3.37 for tsx-11 */
/*:11-14-1995-10:23-wht@kepler-3.37.80-source control point: SOCKETS */
/*:03-12-1995-01:29-wht@kepler-Linux winbox was inaccurate */
/*:01-15-1995-01:52-wht@n4hgf-clean up creeping port rot */
/*:01-12-1995-15:19-wht@n4hgf-apply Andrew Chernov 8-bit clean+FreeBSD patch */
/*:05-17-1994-20:49-wht@n4hgf-winget_single ichar now unsigned char */
/*:05-04-1994-04:39-wht@n4hgf-ECU release 3.30 */
/*:11-12-1993-11:00-wht@n4hgf-Linux changes by bob@vancouver.zadall.com */
/*:09-15-1993-11:31-wht@n4hgf-endwin on non-SCO per bob@vancouver.zadall.com */
/*:08-17-1993-14:04-wht@n4hgf-add OLD_WINGETS for compatibility */
/*:08-13-1993-04:04-wht@n4hgf-add highlight_delete function to wingets */
/*:09-10-1992-13:59-wht@n4hgf-ECU release 3.20 */
/*:08-22-1992-15:38-wht@n4hgf-ECU release 3.20 BETA */
/*:02-09-1992-16:08-root@n4hgf-ruling characters only on  SCO (tcap curses) */
/*:08-25-1991-14:39-wht@n4hgf-SVR4 port thanks to aega84!lh */
/*:08-01-1991-03:52-wht@n4hgf-when editing string, set cursor to end */
/*:07-25-1991-12:57-wht@n4hgf-ECU release 3.10 */
/*:08-14-1990-20:40-wht@n4hgf-ecu3.00-flush old edit history */

#include "ecucurses.h"
#include <errno.h>
#include <ctype.h>
#include "ecukey.h"
#include "ecuxkey.h"
#include "termecu.h"
#include "pc_scr.h"

#if !defined(UINT16)
#define UINT16 unsigned short
#endif
#if !defined(uchar)
#define uchar unsigned char
#endif
#if !defined(UINT)
#define UINT unsigned int
#endif
#if !defined(UINT32)
#define UINT32 unsigned long
#endif

extern int tty_is_multiscreen;

WINCH sTL;
WINCH sTR;
WINCH sBL;
WINCH sBR;
WINCH sLT;
WINCH sRT;
WINCH sVR;
WINCH sHR;

int windows_active = 0;

int ttymode_before_window_start;

/*+-------------------------------------------------------------------------
	clear_area_char(win,y,x,len,fillchar)
--------------------------------------------------------------------------*/
void
clear_area_char(win, y, x, len, fillchar)
WINDOW *win;
int y;
int x;
int len;
char fillchar;
{
	wmove(win, y, x);
#if 0
	wprintw(win, "%02x %d,%d,%d", fillchar, y, x, len);
#endif
	while (len-- > 0)
		waddch(win, fillchar);
	wmove(win, y, x);

}							 /* end of clear_area_char */

/*+-------------------------------------------------------------------------
	clear_area(win,y,x,len)
--------------------------------------------------------------------------*/
void
clear_area(win, y, x, len)
WINDOW *win;
int y;
int x;
int len;
{
	clear_area_char(win, y, x, len, ' ');
}							 /* end of clear_area_char */

/*+-------------------------------------------------------------------------
	windows_start()
--------------------------------------------------------------------------*/
void
windows_start()
{
	extern int tty_not_char_special;
	static int initscr_already_performed = 0;

	if (tty_not_char_special)
	{
		fprintf(stderr, "curses features unavailable when stdin not tty\r\n");
		errno = -1;
		termecu(TERMECU_CURSES_ERROR);
	}

	ttymode_before_window_start = get_ttymode();
	ttymode(0);
	if (!initscr_already_performed && !initscr())
	{
		fprintf(stderr, "curses init failure ... check terminal type\r\n");
		errno = -1;
		termecu(TERMECU_CURSES_ERROR);
	}
	initscr_already_performed = 1;
	scrollok(stdscr, 0);
	savetty();
	raw();
	noecho();
	nonl();
	clear();

	windows_active = 1;

#if defined(CFG_UseACS)
	sTL = ACS_ULCORNER;
	sTR = ACS_URCORNER;
	sBL = ACS_LLCORNER;
	sBR = ACS_LRCORNER;
	sLT = ACS_LTEE;
	sRT = ACS_RTEE;
	sVR = ACS_VLINE;
	sHR = ACS_HLINE;
	if ((sTL < 127) && strchr("+-|", (uchar) sTL))
		sTL = '.';
	if ((sTR < 127) && strchr("+-|", (uchar) sTR))
		sTR = '.';
	if ((sBR < 127) && strchr("+-|", (uchar) sBR))
		sBL = '`';
	if ((sBR < 127) && strchr("+-|", (uchar) sBR))
		sBR = '\'';
#else
	sTL = vanilla_TL;
	sTR = vanilla_TR;
	sBL = vanilla_BL;
	sBR = vanilla_BR;
	sLT = vanilla_LT;
	sRT = vanilla_RT;
	sVR = vanilla_VR;
	sHR = vanilla_HR;
#endif /* defined(CFG_UseACS) */

	wclear(stdscr);
	touchwin(stdscr);
	wrefresh(stdscr);

}							 /* end of windows_start */

/*+-------------------------------------------------------------------------
	windows_end(botleft_flag)
--------------------------------------------------------------------------*/
void
windows_end(botleft_flag)
int botleft_flag;
{
	if (!windows_active)
		return;
#if !defined(M_UNIX) && !defined(M_XENIX) && !defined(SCO32v5)
	endwin();
#else
	refresh();
#endif
	if (botleft_flag)
		tcap_cursor(LINES - 1, 0);
	ttymode(ttymode_before_window_start);
	windows_active = 0;
}							 /* end of windows_end */

/*+-------------------------------------------------------------------------
	windows_end_signal() -- called by termecu()
--------------------------------------------------------------------------*/
void
windows_end_signal()
{
	windows_end(0);
}							 /* end of windows_end_signal */

/*+-------------------------------------------------------------------------
	winbox(win)
--------------------------------------------------------------------------*/
void
winbox(win)
WINDOW *win;
{

#ifdef __FreeBSD__			 /* ache */
	box(win, sVR, sHR);
#else
#if defined(linux) || defined(SVR4)	/* wht */
	int x, y;

	box(win, sVR, sHR);
	getmaxyx(win, y, x);
	wmove(win, 0, 0);
	waddch(win, sTL);
	wmove(win, y - 1, 0);
	waddch(win, sBL);
	wmove(win, y - 1, x - 1);
	waddch(win, sBR);
	wmove(win, 0, x - 1);
	waddch(win, sTR);
#else

	/*
	 * default is not very portable, but has survived well ... too bad
	 * getmaxyx() is not standard
	 */
	box(win, sVR, sHR);
	wmove(win, 0, 0);
	waddch(win, sTL);
	wmove(win, win->_maxy - 1, 0);
	waddch(win, sBL);
	wmove(win, win->_maxy - 1, win->_maxx - 1);
	waddch(win, sBR);
	wmove(win, 0, win->_maxx - 1);
	waddch(win, sTR);
#endif
#endif

}							 /* end of winbox */

/*+-------------------------------------------------------------------------
	window_setup(win,title,title_x)
--------------------------------------------------------------------------*/
void
window_setup(win, title, title_x)
WINDOW *win;
char *title;
int title_x;
{
	int stand = (title_x < 0);

	if (stand)
		title_x = -title_x;

	touchwin(win);
	scrollok(win, 0);		 /* do not scroll */
	winbox(win);
	wmove(win, 0, title_x);
	if (stand)
		wstandout(win);
	waddch(win, '[');
	wprintw(win, " %s ", title);
	waddch(win, ']');
	if (stand)
		wstandend(win);
}							 /* end of window_setup */

/*+-------------------------------------------------------------------------
	window_create(title,title_x,tly,tlx,lines,cols)
if title_x negative, make title "stand" out
--------------------------------------------------------------------------*/
WINDOW *
window_create(title, title_x, tly, tlx, lines, cols)
char *title;
int title_x;
int tly;
int tlx;
int lines;
int cols;
{
	WINDOW *nwin = newwin(lines, cols, tly, tlx);

	errno = -1;
	if (nwin)
		window_setup(nwin, title, title_x);
	else
	{
		fprintf(stderr, "\r\ncurses error: cannot create new window\r\n");
		termecu(TERMECU_CURSES_ERROR);
	}
	return (nwin);
}							 /* end of window_create */

/*+-------------------------------------------------------------------------
	wingets(win,y,x,buf,bufsize,delim,edit,pwgpos)

This procedure reads a string from win and returns the number
of characters read.

If edit is non-zero and pwgpos is not null, the inital string
position is set by dereferencing the pointer.

The terminating delim is returned in 'delim'.

If pwgpos is not null, the ending string position is returned in
the integer pointed to.

-1 is returned if an ESCape is typed by the keyboard user,
otherwise the count of characters in the string.

The entire line must be contained on one line (no line wrap supported).
--------------------------------------------------------------------------*/
int
wingets(win, y, x, buf, bufsize, delim, edit, pwgpos)
WINDOW *win;
int y;
int x;
char *buf;
int bufsize;				 /* includes room for null..field is 1 less */
UINT *delim;
int edit;
int *pwgpos;
{
	int count = 0;
	int pos = 0;
	int insert_mode = 0;

#ifndef OLD_WINGETS
	int highlight_delete = 0;

#endif
	int rtn_val = 0;

	bufsize--;
	if (edit && strlen(buf))
	{
#ifndef OLD_WINGETS
		highlight_delete = 1;
#endif
		clear_area_char(win, y, x, bufsize, ' ');
		wstandout(win);
		waddstr(win, buf);
		wstandend(win);
		count = pos = strlen(buf);
		if (pwgpos)
		{
			pos = *pwgpos;
			if ((pos < 0) || (pos > count))
				pos = count;
		}
	}
	else
	{
		clear_area_char(win, y, x, bufsize, '_');
		*buf = 0;
	}

	wmove(win, y, x + pos);

	while (1)
	{
		wrefresh(win);
		*delim = ttygetc(1);
		if (*delim > 0xFF || !isprint(*delim))
		{
#ifndef OLD_WINGETS
			if (highlight_delete)
			{
				clear_area_char(win, y, x, bufsize, '_');
				waddstr(win, buf);
				wmove(win, y, x + pos);
				highlight_delete = 0;
			}
#endif
			switch (*delim)
			{
				case CRET:
					*delim = NL;
				case NL:
					wrefresh(win);
					rtn_val = count;
					goto FUNC_RETURN;

				case BS:
				case DEL:
					if (count)
					{
						if (count == pos)
						{
							*(buf + --count) = 0;
							wmove(win, y, x + count);
							waddch(win, '_');
							wmove(win, y, x + count);
							pos--;
						}
						else
						{
							if (!pos)
								continue;
							mem_cpy(buf + pos - 1, buf + pos, count - pos);
							*(buf + --count) = 0;
							wmove(win, y, x + --pos);
							waddstr(win, buf + pos);
							waddch(win, '_');
							wmove(win, y, x + pos);
						}
					}
					continue;

				case XFcurlf:
					if (pos)
						wmove(win, y, x + --pos);
					continue;

				case XFcurrt:
					if (pos < count)
						wmove(win, y, x + ++pos);
					continue;

				case XFins:
					insert_mode = !insert_mode;
					continue;

				case ESC:
					rtn_val = -1;
					goto FUNC_RETURN;

				case CTL_U:
					clear_area_char(win, y, x, bufsize, '_');
					count = 0;
					pos = 0;
					*buf = 0;
					continue;

				default:
					*(buf + count) = 0;
					rtn_val = count;
					goto FUNC_RETURN;

			}				 /* end of switch(*delim) */
			/* NOTREACHED */
		}					 /* end of if read delimiter */

#ifndef OLD_WINGETS
		if (highlight_delete)
		{
			clear_area_char(win, y, x, bufsize, '_');
			count = 0;
			pos = 0;
			highlight_delete = 0;
		}
#endif

		if (count == bufsize)
		{
			ring_bell();
			continue;
		}

		if (insert_mode && (pos != count))
		{
			waddch(win, *delim);
			waddstr(win, buf + pos);
			mem_cpy(buf + pos + 1, buf + pos, count - pos);
			*(buf + pos++) = *delim;
			*(buf + ++count) = 0;
			wmove(win, y, x + pos);
		}
		else
		{
			waddch(win, *delim);
			*(buf + pos) = *delim;
			if (pos == count)
				*(buf + ++count) = 0;
			pos++;
		}
	}						 /* end of while can get character */

  FUNC_RETURN:
	if (pwgpos)
		*pwgpos = pos;
	return (rtn_val);

}							 /* end of wingets */

/*+-------------------------------------------------------------------------
	winget_single(win,nondelim_list,delim_list)

This procedure assumes cursor is positioned, repeats reading a non-echoing
character from the keyboard until it matches a character in nondelim_list
or delim_list.  delim_list is expected to contain printable characters
and no upper-case characters.

If no match occurs, the bell is rung and the keyboard is read again.

If the input character matches a character in delim_list, the index (0-n)
of the character in delim_list is returned.  If a match occurs, an
upper-case version of the matching character is placed in the window.

If the input character matches a character in nondelim_list, the character
is returned or'ed with 0x1000

--------------------------------------------------------------------------*/
int
winget_single(win, nondelim_list, delim_list)
WINDOW *win;
UINT *nondelim_list;
UINT *delim_list;
{
	int itmp;
	UINT ichar;

	wrefresh(win);

	while (1)
	{
		ichar = ttygetc(1);
		for (itmp = 0; delim_list[itmp]; itmp++)
		{
			if (ichar == delim_list[itmp])
				return (ichar | 0x1000);
		}
		ichar = to_lower(ichar);
		for (itmp = 0; nondelim_list[itmp]; itmp++)
		{
			if (ichar == nondelim_list[itmp])
			{
				if (ichar <= 0xFF && isprint(ichar))
					waddch(win, to_upper(ichar));
				wrefresh(win);
				return (itmp);
			}
		}
		ring_bell();
	}

}							 /* end of winget_single */

/* end of ecuwinutil.c */
/* vi: set tabstop=4 shiftwidth=4: */
