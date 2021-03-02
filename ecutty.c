/*+-------------------------------------------------------------------------
	ecutty.c - local tty (console) functions
	wht@wht.net

  Defined functions:
	B_to_timeout_msec(termio_tty, st_rdev)
	_setcolor(clrs)
	color_name_to_num(cname)
	console_attribute_test()
	console_xon_status()
	get_initial_colors()
	get_ttymode()
	get_ttyname()
	kbd_test()
	read_colors_file()
	restore_initial_colors()
	ring_bell()
	set_console_xon_xoff_by_arg(arg)
	setcolor(new_colors)
	setcolor_internal(ntokens, tokens)
	termio_to_kbd_chars()
	ttyflush(flush_type)
	ttygetc(xkey_ok)
	ttygets(str, maxsize, flags, delim, pstrpos)
	ttygets_esd(tesd, flags, append_flag)
	ttyinit(param)
	ttymode(arg)
	ttyrdchk()

    In SCO versions, ECU keeps the the state of the normal and
    reverse video foreground and background colors in a 32-bit value:

     00000000001111111111222222222233
     01234567890123456789012345678901
     0000|--|0000|--|0000|--|0000|--|
          fg      bk      fg      bk
           reverse      normal

    The color values are per the SCO extended color definitons:

    black    0     gray         8
    blue     1     lt_blue      9
    green    2     lt_green    10
    cyan     3     lt_cyan     11
    red      4     lt_red      12
    magenta  5     lt_magenta  13
    brown    6     yellow      14
    white    7     hi_white    15

    With Lothar's ISC SVR4, the format is

     00000000001111111111222222222233
     01234567890123456789012345678901
     00000000000000000011????0100????
       0   0   0   0   3   f   4  o b
    where f is the foreground color
    and   b is the background color

    (I dont have one, so I'm guessing these colors are chosen from
    the ISO colors):

     BLACK       0
     RED         1
     GREEN       2
     YELLOW      3
     BLUE        4
     MAGENTA     5
     CYAN        6
     WHITE       7

--------------------------------------------------------------------------*/
/*+:EDITS:*/
/*:04-26-2000-11:15-wht@bob-RELEASE 4.42 */
/*:12-19-1997-20:05-wht@kepler-no more "[stdin is not a tty ...]" */
/*:01-24-1997-02:37-wht@yuriatin-SOURCE RELEASE 4.00 */
/*:10-31-1996-17:49-wht@yuriatin-harden get_ttyname */
/*:10-18-1996-21:12-wht@yuriatin-change text returned by console_xon_status */
/*:09-18-1996-00:29-wht@yuriatin-set USE_COLOR after include of ecu_config.h */
/*:09-16-1996-07:28-wht@yuriatin-use SCO_UNIX to include OS5 */
/*:09-11-1996-20:00-wht@yuriatin-3.48-major telnet,curses,structural overhaul */
/*:07-31-1996-16:32-wht@kepler-clean up style removing ancient debug code */
/*:07-31-1996-16:32-wht@kepler-apply Andrey Chernov FreeBSD color patch */
/*:07-16-1996-16:14-wht@kepler-better doc for screen size error */
/*:07-03-1996-15:09-wht@n4hgf-get rid of unnecessary tty mode 4 */
/*:07-03-1996-14:38-wht@n4hgf-reorder functions */
/*:07-03-1996-14:34-wht@kepler-call tcap_init after tty is ready and raw */
/*:12-09-1995-11:22-wht@n4hgf-print "[stdin not tty ...]" if so */
/*:12-07-1995-15:49-wht@kepler-if stdin not tty, ttyrdchk now returns 0 */
/*:12-06-1995-13:30-wht@n4hgf-termecu w/errno -1 consideration */
/*:11-23-1995-11:20-wht@kepler-source control 3.37 for tsx-11 */
/*:11-14-1995-10:23-wht@kepler-3.37.80-source control point: SOCKETS */
/*:11-11-1995-23:29-wht@gyro-turn off rcvr_ansi_filter if not char special */
/*:10-22-1995-07:56-wht@wwtp1-minimum geometry now 24 lines */
/*:10-14-1995-17:08-wht@kepler-'struct termio' to 'struct TERMIO' */
/*:09-16-1995-14:55-root@kepler-remove rll CONS_GET logevent test hack */
/*:06-15-1995-07:33-wht@kepler-more lines&cols fixing */
/*:05-09-1995-18:32-wht@kepler-support other than main console under linux */
/*:05-09-1995-17:23-wht@kepler-add console_attribute_test for icmd attrtest */
/*:03-29-1995-01:30-wht@n4hgf-use ache internationalization for all versions */
/*:01-12-1995-15:19-wht@n4hgf-apply Andrew Chernov 8-bit clean+FreeBSD patch */
/*:05-04-1994-04:39-wht@n4hgf-ECU release 3.30 */
/*:03-13-1994-19:22-wht@fep-added WHT rterm detection */
/*:01-16-1994-15:46-wht@n4hgf-use ecumachdep.h */
/*:12-12-1993-13:32-wht@n4hgf-support MOTSVR3 */
/*:12-02-1993-14:05-Robert_Broughton@mindlink.bc.c-further LINUX patches */
/*:08-03-1993-12:43-wht@n4hgf-use build_arg_array in read_color_file */
/*:08-03-1993-12:43-wht@n4hgf-global colors file was never read */
/*:08-03-1993-12:41-wht@n4hgf-comment trigger can now be in col > 1 */
/*:07-17-1993-12:36-wht@n4hgf-no more rcvrdisp_actual2_xmtr_buffer junk */
/*:01-30-1993-13:29-wht@n4hgf-get rid of warning about unused variable */
/*:12-20-1992-12:37-wht@n4hgf-WHT experiment with attributes */
/*:10-18-1992-14:26-wht@n4hgf-add console xon/xoff control */
/*:10-08-1992-01:12-wht@n4hgf-no more obsolete Metro Link PTS */
/*:10-08-1992-01:06-wht@n4hgf-SVR4 color work + !use_color normal fix */
/*:09-15-1992-18:52-wht@n4hgf-left some debug code in patch01 dammit */
/*:09-13-1992-12:52-wht@n4hgf-add tty_is_scoterm */
/*:09-10-1992-13:59-wht@n4hgf-ECU release 3.20 */
/*:08-30-1992-07:42-wht@n4hgf-implement USE_COLOR+turn off ESIX color for now */
/*:08-22-1992-15:38-wht@n4hgf-ECU release 3.20 BETA */
/*:08-16-1992-03:43-wht@n4hgf-add -F funckeytype */
/*:06-20-1992-21:13-wht@n4hgf-eculibdir was overwritten if no home dir colors */
/*:06-16-1992-11:20-wht@n4hgf-ECUFUNCKEY */
/*:05-05-1992-17:42-wht@n4hgf-repair underscore always on on sun */
/*:04-24-1992-16:55-wht@n4hgf-dont flunk on >43 lines but use only 43 */
/*:04-20-1992-20:31-wht@n4hgf-ttymode now no-op until ttyinit called */
/*:04-20-1992-19:42-wht@n4hgf-kbdtest code in ttygetc messed up-str too short */
/*:04-19-1992-03:21-jhpb@sarto.budd-lake.nj.us-3.18.37 has ESIX SVR4 */
/*:04-19-1992-02:00-wht@n4hgf-if TERM=ansi with WINDOWID, assume scoterm */
/*:08-31-1991-13:29-wht@n4hgf2-look for colors in CFG_EcuLibDir too */
/*:08-30-1991-04:12-wht@n4hgf2-restore colors wrong to do now if not SCO */
/*:08-30-1991-02:49-aega84!lh-use at_ansi.h/kd.h/CONS_GET under ISC SVR4 */
/*:08-25-1991-14:39-wht@n4hgf-SVR4 port thanks to aega84!lh */
/*:08-17-1991-18:29-wht@n4hgf-add kbdtest command */
/*:07-25-1991-12:57-wht@n4hgf-ECU release 3.10 */
/*:07-14-1991-18:18-wht@n4hgf-new ttygets functions */
/*:07-10-1991-16:19-wht@n4hgf-improve multi-char func key read timeout */
/*:03-20-1991-03:07-root@n4hgf-pts driver returns -1 on Rdchk success! */
/*:03-19-1991-21:24-root@n4hgf-METROLINK_X11R4_PTS mods */
/*:01-29-1991-14:03-wht@n4hgf-more time for ESC vs fkey discrimination */
/*:01-29-1991-13:44-wht@n4hgf-load colors_normal w/ioctl GIO_ATTR if M_UNIX */
/*:12-01-1990-14:33-wht@n4hgf-more non-ansi - fkey mapping with nonansi.c */
/*:11-28-1990-15:56-wht@n4hgf-add non-ansi terminal support */
/*:08-14-1990-20:40-wht@n4hgf-ecu3.00-flush old edit history */

#include "ecu.h"
#include "esd.h"
#include "ecufkey.h"
#include "ecukey.h"
#include "ecuxkey.h"
#include "ecuerror.h"
#include "termecu.h"

#ifndef USE_COLOR
#if defined(SCO_UNIX)			 /* for this module only */
#define USE_COLOR
#endif
#ifdef ISCSVR4				 /* last I heard from lothar, this worked */
#define USE_COLOR
#endif
#ifdef ESIXSVR4				 /* I heard this does not work */
#endif
#ifdef linux
#define USE_COLOR
#endif
#ifdef __FreeBSD__
#define USE_COLOR
#endif
#endif /* USE_COLOR */

#include "ecumachdep.h"

#define DEFINE_TTY_DATA
#include "ecutty.h"

extern UINT tcap_LINES;
extern UINT tcap_COLS;
extern int LINES;
extern int COLS;
extern char screen_dump_file_name[];
extern char *dash_f_funckeytype;

char *kde_text();

UINT LINESxCOLS;
int current_ttymode = 0;
int ttymode_termecu_on_sigint = 0;
int tty_is_pty;
int tty_is_multiscreen;
int tty_is_scoterm;
int tty_not_char_special;
int tty_use_kbd_sw_flow_control = 1;
int use_colors = 0;			 /* set by ttyinit, but default no */
char *ttype;				 /* getenv("TERM") */

static int kbd_test_active = 0;
static int ttyinit_has_been_called = 0;

#if defined(SCO_UNIX) || defined(SVR4) || defined(__FreeBSD__)
static int got_original_colors = 0;

#endif

struct TERMIO tty_termio_at_entry;
struct TERMIO tty_termio_current;
struct stat st_tty;
struct stat st_dn;
struct stat st_tty01;
struct stat st_ttyp0;
struct stat st_console;

uchar kbdeof;				 /* current input EOF */
uchar kbdeol2;				 /* current secondary input EOL */
uchar kbdeol;				 /* current input EOL */
uchar kbderase;				 /* current input ERASE */
uchar kbdintr;				 /* current input INTR */
uchar kbdkill;				 /* current input KILL */
uchar kbdquit;				 /* current input QUIT */
int echo_erase_char;		 /* save users ECHOE bit */
int echo_kill_char;			 /* save users ECHOK bit */
char kbd_is_7bit;			 /* keyboard has parity */
long tty_escape_timeout = 40L;	/* timeout on waiting for char after ESC */

uchar *dole_out_tgc_accum = (uchar *) 0;
int dole_out_tgc_accum_count = 0;

/*
 * color definitions per format described at top of source
 * we handle SCO XENIX and UNIX and ISC SVR4 but not "MYSYS"
 * (search for MYSYS near top of source for clue what MYSYS means)
 *
 * As of this writing, I don't know about colors on ISC 2.2.
 * but they might be the same as for SVR4.
 *
 */
#ifdef SCO_UNIX				 /* SCO */
UINT32 colors_initial = 0x04070A00L;	/* default initial colors */
UINT32 colors_current = 0x04070A00L;	/* colors set during execution */
UINT32 colors_normal = 0x04070A00L;	/* default lt_green/black red/white */
UINT32 colors_success = 0x07000A00L;	/* lt_green/black red/white */
UINT32 colors_alert = 0x0E000E00L;	/* yellow */
UINT32 colors_error = 0x04000400L;	/* red */
UINT32 colors_notify = 0x08000800L;	/* gray */
#define COLORS_DEFINED
#endif /* SCO_UNIX */

#ifdef __FreeBSD__
UINT32 colors_initial = 0x04070700L;	/* default initial colors */
UINT32 colors_current = 0x04070700L;	/* colors set during execution */
UINT32 colors_normal = 0x04070700L;	/* default white/black red/white */
UINT32 colors_success = 0x07000200L;	/* green/black red/white */
UINT32 colors_alert = 0x0E000E00L;	/* yellow */
UINT32 colors_error = 0x04000400L;	/* red */
UINT32 colors_notify = 0x03000300L;	/* cyan */
#define COLORS_DEFINED
#endif

#if defined(ISCSVR4) || defined(linux) || defined(ESIXSVR4)
UINT32 colors_initial = 0x00002528L;
UINT32 colors_current = 0x00002528L;
UINT32 colors_normal = 0x00002528L;	/* white */
UINT32 colors_success = 0x00002028L;	/* green */
UINT32 colors_alert = 0x00002128L;	/* yellow */
UINT32 colors_error = 0x00001F28L;	/* red  */
UINT32 colors_notify = 0x00002428L;	/* cyan */
#define COLORS_DEFINED
#endif /* ISCSVR4 */

#if defined(ESIXSVR4) || defined(linux)
static struct color_remapping
{
	char *name;
	char ecu_val;
	char svr4_val;
}
color_remapping[] =
{
	{ "black", 0, 0x00 },
	{ "blue", 1, 0x04 },
	{ "brown", 6, 0x03 },
	{ "cyan", 3, 0x06 },
	{ "gray", 8, 0x10 },
	{ "green", 2, 0x02 },
	{ "hi_white", 15, 0x17 },
	{ "lt_blue", 9, 0x14 },
	{ "lt_cyan", 11, 0x16 },
	{ "lt_green", 10, 0x12 },
	{ "lt_magenta", 13, 0x15 },
	{ "lt_red", 12, 0x11 },
	{ "magenta", 5, 0x05 },
	{ "red", 4, 0x01 },
	{ "white", 7, 0x07 },
	{ "yellow", 14, 0x13 },
	{ 0, 0, 0 }
};
#endif /* ESIXSVR4 || linux */

#if !defined(COLORS_DEFINED)
/*
 * These "colors" only work for setcolor() when use_colors == 0;
 * They provide unique numbers for termcap mapping
 */
UINT32 colors_current = 1;	 /* dummy */
UINT32 colors_normal = 2;	 /* dummy */
UINT32 colors_initial = 3;	 /* dummy */
UINT32 colors_success = 4;	 /* dummy */
UINT32 colors_alert = 5;		 /* dummy */
UINT32 colors_error = 6;		 /* dummy */
UINT32 colors_notify = 7;	 /* dummy */
#define COLORS_DEFINED
#endif /* !COLORS_DEFINED */

/*+-------------------------------------------------------------------------
	B_to_timeout_msec(c_cflag,st_rdev) - CBAUD code to ESC timeout msec
--------------------------------------------------------------------------*/
/*ARGSUSED*/
UINT32
B_to_timeout_msec(termio_tty, st_rdev)
struct TERMIO *termio_tty;
UINT16 st_rdev;
{
	long ms = 300L;

	/* make network/xterm/pty sweat, but don't make as many mistakes */
	if (tty_is_pty)
		return (ms);

	/* if multiscreen, 3 ticks is pu-lenty */
	if (tty_is_multiscreen)
		return ((long)(1000 / hertz * 3));

	/* bit rate fiddling */
	switch (ecugetspeed(termio_tty))
	{
			/* char times * time/char */
		case B110:
			ms = 10 * 100;
			break;
		case B300:
			ms = 10 * 33;
			break;
		case B600:
			ms = 10 * 16;
			break;
		case B1200:
			ms = 10 * 8;
			break;
		case B2400:
			ms = 10 * 4;
			break;
		default:			 /* many character times for packetized ... */
			ms = 400L;		 /* ... modems used for console */
			break;
	}
	return (ms);

}							 /* end of B_to_timeout_msec */

/*+-------------------------------------------------------------------------
	color_name_to_num(cname)
--------------------------------------------------------------------------*/
int
color_name_to_num(cname)
char *cname;
{
	COLOR *color = colors;
	int itmp;

	while (color->name)
	{
		if ((itmp = strcmp(color->name, cname)) > 0)
			return (-1);
		if (!itmp)
			return (color->num);
		color++;
	}
	return (-1);

}							 /* end of color_name_to_num */

/*+-------------------------------------------------------------------------
	_setcolor(clrs)
--------------------------------------------------------------------------*/
void
_setcolor(clrs)
UINT32 clrs;
{
#if defined(ESIXSVR4) || defined(linux)
	struct color_remapping *cr;

#endif
#if defined(SVR4) || defined(linux)
	char fgcolor;
	char bgcolor;

#endif /* SVR4 || linux */

	if (!use_colors || tty_not_char_special)
		return;

	if(getpid() == shm->rcvr_pid)
		rcvrdisp_actual();

#if defined(SVR4) || defined(linux)
#if defined(ESIXSVR4) || defined(linux)
	/*
	 * set foreground color
	 */
	fgcolor = (clrs >> 8) & 0xff;
	for (cr = color_remapping; cr->name; ++cr)
	{
		if (fgcolor == cr->ecu_val)
		{
			fgcolor = cr->svr4_val;
			break;
		}
	}
	if (!cr->name)
		fgcolor = 0x07;		 /* white */
	if (fgcolor & 0x10)
	{
		fgcolor &= ~0x10;
		ff(se, "\033[1;3%d;m", fgcolor);
	}
	else
		ff(se, "\033[0;3%d;m", fgcolor);

	/*
	 * set background color
	 */
	bgcolor = clrs & 0xff;
	for (cr = color_remapping; cr->name; ++cr)
	{
		if (bgcolor == cr->ecu_val)
		{
			bgcolor = cr->svr4_val;
			break;
		}
	}
	if (!cr->name)
		bgcolor = 0x00;		 /* black */
	if (bgcolor & 0x10)
	{
		bgcolor &= ~0x10;
		ff(se, "\033[5;3%d;m", bgcolor);
	}
	else
		ff(se, "\033[0;3%d;m", bgcolor);
#else /* ISC SVR4 */
	/* normal */
	fgcolor = (clrs >> 8) & 0xff;
	bgcolor = clrs & 0xff;
	ff(se, "\033[%d;%dm", fgcolor, bgcolor);
#endif /* ESIXSVR4 */
#else /* not any SVR4 */
#if defined(SCO_UNIX) || defined(__FreeBSD__)
	/* normal */
	ff(se, "\033[=%ldF\033[=%ldG", (clrs >> 8) & 0xFF, clrs & 0xFF);

	/* reverse */
	ff(se, "\033[=%ldH\033[=%ldI", (clrs >> 24) & 0xFF, (clrs >> 16) & 0xFF);
#endif /* SCO_UNIX */
#endif /* SVR4 */
	colors_current = clrs;
}							 /* end of _setcolor */

/*+-------------------------------------------------------------------------
	setcolor(new_colors)

requires termcap init to have been done
--------------------------------------------------------------------------*/
void
setcolor(new_colors)
UINT32 new_colors;
{
	if (tty_not_char_special)
		return;

	if (!use_colors)
	{
		if (new_colors == colors_notify)
			tcap_underscore_on();
		else if (new_colors == colors_alert)
			tcap_bold_on();
		else if (new_colors == colors_error)
			tcap_stand_out();
		else if (new_colors == colors_success)
		{
			tcap_underscore_on();
			tcap_bold_on();
		}
		else
		{
			tcap_underscore_off();
			tcap_bold_off();
			tcap_stand_end();
		}
		return;
	}
	_setcolor(new_colors);

#if !defined(linux) && !defined(__FreeBSD__)
	tcap_stand_end();
#endif

}							 /* end of setcolor */

/*+-------------------------------------------------------------------------
	setcolor_internal(ntokens,tokens)

returns 0 on success, else token number in error + 1
--------------------------------------------------------------------------*/
int
setcolor_internal(ntokens, tokens)
int ntokens;
char **tokens;
{
	UINT32 fgnd;
	UINT32 bgnd;

	if (tty_not_char_special || !use_colors)
		return (0);

	if (ntokens < 2)
		return (1);
	else if (ntokens == 2)
		tokens[2] = "black";

	if ((fgnd = (UINT32) color_name_to_num(tokens[1])) > 15)
		return (2);
	if ((bgnd = (UINT32) color_name_to_num(tokens[2])) > 15)
		return (3);

	if (!strcmp(tokens[0], "normal"))
	{
		colors_normal &= 0xFFFF0000L;
		colors_normal |= (fgnd << 8) | bgnd;
		setcolor(colors_normal);
	}
	else if (!strcmp(tokens[0], "reverse"))
	{
		colors_normal &= 0x0000FFFFL;
		colors_normal |= (fgnd << 24) | (bgnd << 16);
		setcolor(colors_normal);
	}
	else if (!strcmp(tokens[0], "notify"))
		colors_notify = (fgnd << 24) | (bgnd << 16) | (fgnd << 8) | bgnd;
	else if (!strcmp(tokens[0], "success"))
		colors_success = (fgnd << 24) | (bgnd << 16) | (fgnd << 8) | bgnd;
	else if (!strcmp(tokens[0], "alert"))
		colors_alert = (fgnd << 24) | (bgnd << 16) | (fgnd << 8) | bgnd;
	else if (!strcmp(tokens[0], "error"))
		colors_error = (fgnd << 24) | (bgnd << 16) | (fgnd << 8) | bgnd;
	else
		return (1);

	return (0);

}							 /* end of setcolor_internal */

/*+-------------------------------------------------------------------------
	restore_initial_colors() - make screen safe

On SCO, restore color choices at execution time if we successfully
got them from the driver; in other situations, use tcap to reset.
--------------------------------------------------------------------------*/
void
restore_initial_colors()
{
#if defined(SCO_UNIX) || defined(SVR4) || defined(__FreeBSD__)
	if (use_colors && got_original_colors)
		setcolor(colors_initial);
	else
#endif
	{
		tcap_blink_off();
		tcap_bold_off();
		tcap_underscore_off();
#ifdef __FreeBSD__
		if (use_colors)
			tcap_orig_pair();
#endif
	}
}							 /* end of restore_initial_colors */

/*+-------------------------------------------------------------------------
	get_initial_colors() - read colors at time of execution from driver

     00000000001111111111222222222233
     01234567890123456789012345678901
     0000|--|0000|--|0000|--|0000|--|
          fg      bk      fg      bk
           reverse      normal

--------------------------------------------------------------------------*/
#if defined(SCO_UNIX) || defined(__FreeBSD__)
void
get_initial_colors()
{
	UINT cur_attr;
	UINT32 fgnd;
	UINT32 bgnd;

	colors_initial = colors_normal;	/* scoterm can use color but ... */
	if (ioctl(TTYIN, GIO_ATTR, &cur_attr) == -1)	/* ... GIO_ATTR won't
													 * work */
		return;
	colors_normal = 0L;

/*
 * first, reverse, so we can end up with normal colors selected
 */
	write(1, "\033[7m", 4);	 /* select reverse */
#ifdef __FreeBSD__
	(void)ioctl(TTYIN, GIO_ATTR, &cur_attr);
#else
	cur_attr = (UINT) ioctl(TTYIN, GIO_ATTR, 0);
#endif
	fgnd = (UINT32) cur_attr & 0x0F;
	bgnd = (UINT32) (cur_attr >> 4) & 0x0F;
	colors_normal |= (fgnd << 24) | (bgnd << 16);

/*
 * now, normal
 */
	write(1, "\033[m", 3);	 /* select normal */
#ifdef __FreeBSD__
	(void)ioctl(TTYIN, GIO_ATTR, &cur_attr);
#else
	cur_attr = (UINT) ioctl(TTYIN, GIO_ATTR, 0);
#endif
	fgnd = (UINT32) cur_attr & 0x0F;
	bgnd = (UINT32) (cur_attr >> 4) & 0x0F;
	colors_normal |= (fgnd << 8) | bgnd;

	colors_initial = colors_normal;	/* save for restore_initial_colors */
	got_original_colors = 1;

}							 /* end of get_initial_colors */
#endif

/*+-------------------------------------------------------------------------
	read_colors_file() - read color definition if present
--------------------------------------------------------------------------*/
void
read_colors_file()
{
	FILE *fp;
	char s128[128];

#define MAX_COLOR_TOKENS 6
	char *tokens[MAX_COLOR_TOKENS];
	int ntokens;
	char *cp;
	int itmp;

	if (tty_not_char_special)
		return;

#if defined(SCO_UNIX) || defined(__FreeBSD__)
	get_initial_colors();
#endif

	get_home_dir(s128);
	strcat(s128, "/.ecu/colors");

	if (!(fp = fopen(s128, "r")))
	{
		strcpy(s128, eculibdir);
		strcat(s128, "/colors");
		if (!(fp = fopen(s128, "r")))
			return;
	}

	while (fgets(s128, sizeof(s128), fp))
	{
		if (itmp = strlen(s128))	/* itmp = len; if > 0 ... */
		{
			itmp--;
			s128[itmp] = 0;	 /* ... strip trailing NL */
		}
		if (cp = strchr(s128, '#'))	/* comment? */
			*cp = 0;
		cp = s128;			 /* first call to str_token, -> buff */
		while ((*cp == 0x20) || (*cp == TAB))
			cp++;			 /* strip leading spaces */
		if (*cp == 0)		 /* if line all blank, skip it */
			continue;

		build_arg_array(s128, tokens, MAX_COLOR_TOKENS, &ntokens);
		if (ntokens < 2)
			continue;

		setcolor_internal(ntokens, tokens);

	}						 /* while records left to ready */

#if defined(SCO_UNIX) || defined(__FreeBSD__)
	if (ioctl(TTYIN, GIO_ATTR, &itmp) == -1)
		colors_initial = colors_normal;	/* hack for scoterm */
#endif

	fclose(fp);
}							 /* end of read_colors_file */

/*+-------------------------------------------------------------------------
	ring_bell()
--------------------------------------------------------------------------*/
void
ring_bell()
{
	char b = BEL;

	if (tty_not_char_special)
		return;
	write(TTYOUT, &b, 1);

}							 /* end of ring_bell */

/*+-------------------------------------------------------------------------
	termio_to_kbd_chars()
--------------------------------------------------------------------------*/
void
termio_to_kbd_chars()
{
	kbdintr = (tty_termio_at_entry.c_cc[VINTR])
	? (tty_termio_at_entry.c_cc[VINTR] & 0x7F) : '\377';
	kbdquit = (tty_termio_at_entry.c_cc[VQUIT])
		? (tty_termio_at_entry.c_cc[VQUIT] & 0x7F) : '\377';
	kbderase = (tty_termio_at_entry.c_cc[VERASE])
		? (tty_termio_at_entry.c_cc[VERASE] & 0x7F) : '\377';
	kbdkill = (tty_termio_at_entry.c_cc[VKILL])
		? (tty_termio_at_entry.c_cc[VKILL] & 0x7F) : '\377';
	kbdeof = (tty_termio_at_entry.c_cc[VEOF])
		? (tty_termio_at_entry.c_cc[VEOF] & 0x7F) : '\04';
	kbdeol2 = (tty_termio_at_entry.c_cc[VEOL])
		? (tty_termio_at_entry.c_cc[VEOL] & 0x7F) : '\377';
	kbdeol = (tty_termio_at_entry.c_iflag & ICRNL)
		? '\r' : '\n';

	kbd_is_7bit = ((tty_termio_at_entry.c_cflag & PARENB) != 0);
	echo_erase_char = tty_termio_at_entry.c_lflag & ECHOE;
	echo_kill_char = tty_termio_at_entry.c_lflag & ECHOK;

}							 /* end of termio_to_kbd_chars */

/*+-----------------------------------------------------------------------
	ttymode(arg) -- control user console (kbd/screen)

  Where arg ==
	0 restore attributes saved at start of execution
	1 raw mode (send xon/xoff, but do not respond to it, no ISIG/SIGINT)
	2 raw mode (same as 1 but allow keyboard interrupts)
	3 same as 2 but terminate program on SIGINT

------------------------------------------------------------------------*/
void
ttymode(arg)
int arg;
{

	/*
	 * ignore if no keyboard involved
	 */
	if (tty_not_char_special)
		return;

	/*
	 * usage()->termecu()->ttymode() is possible before ttyinit()
	 */
	if (!ttyinit_has_been_called)
		return;

	switch (arg)
	{
		case 0:
			ecusetattr(TTYIN, TCSETAW, &tty_termio_at_entry);
			tty_termio_current = tty_termio_at_entry;
			current_ttymode = 0;
			ttymode_termecu_on_sigint = 0;
			break;

		case 1:
		case 2:
		case 3:
			tty_termio_current = tty_termio_at_entry;

			tty_termio_current.c_cflag &= ~(PARENB | PARODD);
			tty_termio_current.c_cflag |= CS8;

			/* don't want to honor tty xon/xoff, but pass to other end */
#if defined(CFG_TermiosLineio)
			tty_termio_current.c_iflag &= ~(INLCR | ICRNL | IGNCR | ISTRIP);
			tty_termio_current.c_oflag &= ~(ONLCR);
#else
			tty_termio_current.c_iflag &= ~(INLCR | ICRNL | IGNCR | IUCLC | ISTRIP);
			tty_termio_current.c_oflag &= ~(OLCUC | ONLCR | OCRNL | ONOCR | ONLRET);
#endif
			tty_termio_current.c_oflag |= OPOST;

			if (tty_use_kbd_sw_flow_control)
				tty_termio_current.c_iflag |= IXON | IXOFF;
			else
				tty_termio_current.c_iflag &= ~(IXON | IXOFF);

			tty_termio_current.c_lflag &= ~(ICANON | ISIG | ECHO);
			if (arg > 1)
				tty_termio_current.c_lflag |= ISIG;

			tty_termio_current.c_cc[VMIN] = 1;
			tty_termio_current.c_cc[VTIME] = 0;

			ecusetattr(TTYIN, TCSETAW, &tty_termio_current);
			current_ttymode = arg;

			/*
			 * establish ttymode_termecu_on_sigint indicator for
			 * ecusighdl.c:xmtr_SIGINT_handler()
			 */
			ttymode_termecu_on_sigint = (arg == 3);
			break;

		default:
			ff(se, "\r\nttymode: invalid argument %d\r\n", arg);
			termecu(TERMECU_LOGIC_ERROR);
			break;
	}

}							 /* end of ttymode */

/*+-------------------------------------------------------------------------
	int	get_ttymode()
--------------------------------------------------------------------------*/
int
get_ttymode()
{
	return (current_ttymode);
}							 /* end of get_ttymode */

/*+-----------------------------------------------------------------------
	ttyflush(flush_type) -- flush tty driver input &/or output buffers

  0 == input buffer
  1 == output buffer
  2 == both buffers
------------------------------------------------------------------------*/
void
ttyflush(flush_type)
int flush_type;
{
	if (tty_not_char_special)
		return;

	ecuflow(TTYIN, TCOOFF);
	ecuflush(TTYIN, flush_type);
	ecuflow(TTYIN, TCOON);

	if (flush_type != 1)
	{
		dole_out_tgc_accum = (uchar *) 0;
		dole_out_tgc_accum_count = 0;
	}

}							 /* end of ttyflush */

/*+-------------------------------------------------------------------------
	ttyrdchk() - see if key pressed and not read
--------------------------------------------------------------------------*/
int
ttyrdchk()
{
	if (tty_not_char_special)
		return (0);
	return (Rdchk(TTYIN) || dole_out_tgc_accum_count);
}							 /* end of ttyrdchk */

/*+-------------------------------------------------------------------------
	ttygetc(xkey_ok) -- get a key from the keyboard

  if xkey_ok is 0, disallow extended keys,
  otherwisemap extended keys to sign-bit-set special value
--------------------------------------------------------------------------*/
UINT
ttygetc(xkey_ok)
int xkey_ok;
{
	uchar ctmp;
	UINT utmp;
	extern int errno;
	UINT itmp = 0;
	long timeout_remaining;
	static uchar tgc_accum[16];
	UINT funckeymap();

	if (tty_not_char_special)/* untested */
	{
		if (read(TTYIN, (char *)&ctmp, 1) <= 0)
			return (XF_no_way);
		return ((UINT) ctmp);
	}

	if (dole_out_tgc_accum_count)
	{
		ctmp = *dole_out_tgc_accum++;
		dole_out_tgc_accum_count--;
		if (kbd_is_7bit)
			ctmp &= 0x7F;
		return ((UINT) ctmp);
	}

  GET_KEY:
	errno = 0;
	if (read(TTYIN, (char *)&ctmp, 1) <= 0)
	{
		if (errno == EINTR)
			goto GET_KEY;
		perror_errmsg("keyboard");
		termecu(TERMECU_TTYIN_READ_ERROR);
	}

	if (!isprint(ctmp) &&
		(ctmp != kbderase) && (ctmp != kbdkill) &&
		(ctmp != kbdeol) && (ctmp != kbdeol2) &&
		(ctmp != kbdintr) && (ctmp != kbdeof))
	{
		tgc_accum[0] = ctmp;
		tgc_accum[itmp = 1] = 0;
		timeout_remaining = tty_escape_timeout;
		while (((utmp = funckeymap(tgc_accum, itmp)) >= XF_no_way) &&
			(timeout_remaining > 0))
		{
			timeout_remaining -= Nap(hzmsec);
			if(kbd_test_active > 1)
				ff(se,".");
			if (!Rdchk(TTYIN))
				continue;
			read(TTYIN, (char *)&ctmp, 1);
			if (itmp == (sizeof(tgc_accum) - 1))	/* do not allow overflow */
			{
				utmp = XF_no_way;
				break;
			}
			timeout_remaining = tty_escape_timeout;
			tgc_accum[itmp++] = ctmp;
		}
		tgc_accum[itmp] = 0;
		if ((utmp == XF_not_yet) && (itmp == 1))
		{
			if (kbd_is_7bit)
				tgc_accum[0] &= 0x7F;
			return ((UINT) tgc_accum[0]);
		}
		else if (utmp < XF_no_way)	/* if we got a map */
		{
			if (kbd_test_active)
			{
				char title[128];

				sprintf(title, "--> func key '%s' (%d key codes received)",
					kde_text(utmp), itmp);
				hex_dump(tgc_accum, -itmp, title, 1);
			}
			if (!xkey_ok)
			{
				ring_bell();
				goto GET_KEY;
			}
			switch (utmp)
			{
				case IKDE_CU5:
					screen_dump(screen_dump_file_name);
					goto GET_KEY;
				default:
					return ((UINT) ikde_to_xf(utmp));
			}
			/* NOTREACHED */
		}
		/* not func key -- must be typamatic control key */
		if (kbd_test_active)
		{
			char title[128];

			if (itmp > 1)
			{
				sprintf(title,
					"--> no func key recognized (%d key codes received)",
					itmp);
				hex_dump(tgc_accum, -itmp, title, 1);
			}
		}
		dole_out_tgc_accum_count = itmp - 1;
		dole_out_tgc_accum = tgc_accum + 1;
		if (kbd_is_7bit)
			tgc_accum[0] &= 0x7F;
		return ((UINT) tgc_accum[0]);
	}

	/*
	 * simple key, not special
	 */
	if (kbd_is_7bit)
		ctmp &= 0x7F;

	return ((UINT) ctmp);

}							 /* end if ttygetc */

/*+-----------------------------------------------------------------------
	ttygets(str,maxsize,flags,delim,pstrpos)

flags & TG_CRLF   - echo cr/lf terminator
flags & TG_XDELIM - extended delimiter set
                    (Home, End, PgUp, PgDn, CurUp, CurDn)
flags & TG_EDIT   - redisplay/edit current string
flags & TG_IPOS   - if edit, use initial string pos
------------------------------------------------------------------------*/
void
ttygets(str, maxsize, flags, delim, pstrpos)
char *str;
int maxsize;
int flags;
UINT *delim;
int *pstrpos;
{
	UINT inch;
	char ch;
	int strcount = 0;
	int strpos = 0;
	int insert_mode = 0;
	char *bs_str = "\010 \010";

	--maxsize;				 /* decrement for safety */

	if (flags & TG_EDIT)
	{
		strpos = strcount = strlen(str);
		write(TTYOUT, str, strcount);
		if (pstrpos && (*pstrpos > 0) && (*pstrpos <= strcount))
			strpos = *pstrpos;
		tcap_curleft(strcount - strpos);
	}

	while (1)
	{
		inch = ttygetc(1);
		*delim = inch;		 /* last char will always be the delimiter */
		if ((inch == kbdintr) || (inch == ESC))
		{
			tcap_curright(strcount - strpos);
			while (strcount)
			{
				write(TTYOUT, bs_str, strlen(bs_str));
				strcount--;
			}
			str[strcount] = 0;
			*delim = ESC;
			goto FUNC_RETURN;
		}
		else if (inch == kbdkill)
		{
			tcap_curright(strcount - strpos);
			while (strcount)
			{
				write(TTYOUT, bs_str, strlen(bs_str));
				strcount--;
			}
			strpos = 0;
			*str = 0;
			continue;
		}
		else if (inch == kbderase)
		{
			if (strcount)
			{
				if (strcount == strpos)
				{
					write(TTYOUT, bs_str, strlen(bs_str));
					strcount--, strpos--;
				}
				else
				{
					if (!strpos)
						continue;
					mem_cpy(str + strpos - 1, str + strpos, strcount - strpos);
					write(TTYOUT, "\010", 1);
					str[--strcount] = 0;
					strpos--;
					write(TTYOUT, str + strpos, strlen(str + strpos));
					write(TTYOUT, " ", 1);
					tcap_curleft(strcount - strpos + 1);
				}
			}
			str[strcount] = 0;
			continue;
		}
		else if (inch == XFins)
		{
			insert_mode = !insert_mode;
			continue;
		}
		else if (inch == XFcurlf)
		{
			if (strpos)
			{
				strpos--;
				tcap_curleft(1);
			}
			continue;
		}
		else if (inch == XFcurrt)
		{
			if (strpos < strcount)
			{
				strpos++;
				tcap_curright(1);
			}
			continue;
		}

		if (flags & TG_XDELIM)	/* extended delimiter */
		{
			switch (inch)
			{
				case XFhome:
				case XFend:
				case XFpgup:
				case XFpgdn:
				case XFcurup:
				case XFcurdn:
#ifdef notdef
					tcap_curright(strcount - strpos);
					while (strcount)
					{
						write(TTYOUT, bs_str, strlen(bs_str));
						strcount--;
					}
#endif
					str[strcount] = 0;
					goto FUNC_RETURN;
			}
		}

		switch (inch)
		{
			case CRET:
				*delim = NL;
			case NL:
				str[strcount] = 0;
				tcap_curright(strcount - strpos);
				if ((flags & TG_CRLF))
					ff(se, "\r\n");
				goto FUNC_RETURN;

			case CTL_L:
			case CTL_R:
				tcap_curright(strcount - strpos);
				ff(se, "%s (insert mode %s)\r\n", graphic_char_text(inch, 0),
					(insert_mode) ? "ON" : "OFF");
				tcap_eeol();
				write(TTYOUT, str, strcount);
				tcap_curleft(strcount - strpos);
				break;

			default:
				if (inch > 0xFF || !isprint(inch))
				{
					ring_bell();
					break;
				}
				if (strpos == strcount)
				{
					if (strcount == maxsize)
					{
						ring_bell();
						continue;
					}
					str[strcount++] = inch;
					strpos++;
					ch = (char)inch;
					write(TTYOUT, &ch, 1);
				}
				else
				{
					if (insert_mode)
					{
						if (strcount == maxsize)
						{
							ring_bell();
							continue;
						}
						mem_cpy(str + strpos + 1, str + strpos, strcount - strpos);
						str[strpos] = inch;
						strcount++;
						str[strcount] = 0;
						write(TTYOUT, str + strpos, strcount - strpos);
						strpos++;
						tcap_curleft(strcount - strpos);
					}
					else
					{
						str[strpos++] = inch;
						ch = (char)inch;
						write(TTYOUT, &ch, 1);
					}
				}
				str[strcount] = 0;
		}
	}

  FUNC_RETURN:
	if (pstrpos)
		*pstrpos = strpos;

}							 /* end of ttygets() */

/*+-------------------------------------------------------------------------
	ttygets_esd(tesd,flags,append_flag)
--------------------------------------------------------------------------*/
ttygets_esd(tesd, flags, append_flag)
ESD *tesd;
int flags;
int append_flag;
{
	char *pb = tesd->pb;
	int maxcb = tesd->maxcb;
	UINT delim;

	if (append_flag)
	{
		pb += tesd->cb;
		maxcb -= tesd->cb;
	}
	else
	{
		pb = tesd->pb;
		maxcb = tesd->maxcb;
		tesd->cb = 0;
	}

	ttygets(pb, maxcb, flags, &delim, (int *)0);

	if (delim == ESC)
	{
		if (!append_flag)
			esdzero(tesd);
		return (eProcAttn_ESCAPE);
	}

	tesd->cb = strlen(tesd->pb);
	plogs(pb);
	if (flags & 1)
		plogc(NL);
	return (0);

}							 /* end of ttygets_esd */

/*+-------------------------------------------------------------------------
	TIMEB_diff(t, t0) - msec difference between two TIMEB
--------------------------------------------------------------------------*/
long
TIMEB_diff(t, t0)
struct TIMEB *t;
struct TIMEB *t0;
{ 
	long delta = ((t->time - t0->time) * 1000) + (t->millitm - t0->millitm); 
	return(delta);
}	/* end of TIMEB_diff */

/*+-------------------------------------------------------------------------
	kbd_test() - test keyboard handler
--------------------------------------------------------------------------*/
void
kbd_test()
{
	int i;
	UINT ch;
	long l;
	char *interrupted = "\ninterrupted\n";
	char *nap_text1 = "These characters should appear somewhat slowly.";
	char *nap_text2 = "These appear more slowly.";
	char *cp;
	struct TIMEB timeb0;
	struct TIMEB timeb;
	extern struct TIMEB starting_timeb;

	sigint = 0;
	pprintf("Nap() test (via slow printing): hzmsec = %ld",hzmsec);
	pputs("\n30 ch/sec:  ");
	cp = nap_text1;
	while(*cp)
	{
		Nap(30L);
		if(sigint)
			goto KEY_PRESS_TEST;
		pputc(*cp++);
	}
	pputs("\n10 ch/sec:  ");
	cp = nap_text2;
	while(*cp)
	{
		Nap(100L);
		if(sigint)
			goto KEY_PRESS_TEST;
		pputc(*cp++);
	}
	pputs("\n 5 ch/sec:  ");
	cp = nap_text2;
	while(*cp)
	{
		Nap(200L);
		if(sigint)
			goto KEY_PRESS_TEST;
		pputc(*cp++);
	}

KEY_PRESS_TEST:
	if(!sigint)
		pputs("\n");
	else
	{
		pputs(interrupted);
		sigint = 0;
	}
	dole_out_tgc_accum_count = 0;
	kbd_test_active = 2;
	Ftime(&timeb0);
	l = ((timeb0.time - starting_timeb.time) * 1000) +
		(timeb0.millitm - starting_timeb.millitm);
	pprintf("\nTimed keyboard read test (starting mhack = %ld)\n",l);
	pputs("Press an alphabetic key within 10 seconds\n");
	i = 10;
	while(i--)
	{
		Nap(1000L);
		if(sigint)
		{
			pputs(interrupted);
			sigint = 0;
			break;
		}
		Ftime(&timeb);
		pprintf("[%8ld msec] ",TIMEB_diff(&timeb,&timeb0));
		if(!ttyrdchk())
			pputs("\n");
		else
		{
			pputs("\nGOT IT: ");
			ch = ttygetc(1);
			if (ch >= 0x100)
				pprintf("fkey '%s'\n", xf_text(ch));
			else
				pprintf("key  '%s'\n", hex_to_ascii_name(ch));
			break;
		}
			
	}

	pputs("\nPress the HOME or END key within 10 seconds.\n");
	i = 10;
	Ftime(&timeb0);
	while(i--)
	{
		Nap(1000L);
		if(sigint)
		{
			pputs(interrupted);
			sigint = 0;
			break;
		}
		Ftime(&timeb);
		pprintf("[%8ld msec] ",TIMEB_diff(&timeb,&timeb0));
		if(!ttyrdchk())
			pputs("\n");
		else
		{
			pputs("\nGOT IT: ");
			ch = ttygetc(1);
			if (ch >= 0x100)
				pprintf("fkey '%s'\n", xf_text(ch));
			else
				pprintf("key  '%s'\n", hex_to_ascii_name(ch));
			break;
		}
	}

	kbd_test_active = 1;
	pputs("\nPress various keys to test keyboard (press ESCape to exit)\n");
	ch = 0;
	while (ch != ESC)
	{
		ch = ttygetc(1);

		if ((ch < 0x100) && dole_out_tgc_accum_count)
		{
			pprintf("    got %d key sequence %s ",
				dole_out_tgc_accum_count + 1, hex_to_ascii_name(ch));
			while (dole_out_tgc_accum_count)
			{
				pprintf("%s ", hex_to_ascii_name(*dole_out_tgc_accum++));
				dole_out_tgc_accum_count--;
			}
			pputs("\n");
			ch = 0;
			continue;
		}
		pputs("    got ");
		if (ch >= 0x100)
			pprintf("fkey '%s'\n", xf_text(ch));
		else
			pprintf("key  '%s'\n", hex_to_ascii_name(ch));
	}
	kbd_test_active = 0;
	ttyflush(0);
	pputs("keyboard test complete\n\n");
	dole_out_tgc_accum = (uchar *) 0;
	dole_out_tgc_accum_count = 0;

}							 /* end of kbd_test */

/*+-------------------------------------------------------------------------
	char *get_ttyname() - return pointer to static string

This routine is largely a crock and is likely to explode at any rev or twist
--------------------------------------------------------------------------*/
char *
get_ttyname()
{
	char *ttyname();
	char *tty;

	/*
	 * try to get stat on stdin; check for file
	 */
	memset((char *)&st_tty,0,sizeof(st_tty));
	if(fstat(TTYIN,&st_tty))
		return("none");
    if ((st_tty.st_mode & S_IFMT) == S_IFREG)
		return("file");

	/*
	 * check for /dev/null
	 */
	memset((char *)&st_dn,0,sizeof(st_dn));
	stat("/dev/null",&st_dn);
    if ((st_dn.st_ino == st_tty.st_ino) && (st_dn.st_rdev == st_tty.st_rdev))
		return("/dev/null");

	/*
	 * now check for tty
	 */
	if(tty = ttyname(TTYIN))
		return(tty);

	return("unknown");

}							 /* end of get_ttyname */

/*+-------------------------------------------------------------------------
	set_console_xon_xoff_by_arg(arg)
--------------------------------------------------------------------------*/
int
set_console_xon_xoff_by_arg(arg)
char *arg;
{
	int new_xon_xoff = 0;

	if (ulcmpb(arg, "on") < 0)
	{
		new_xon_xoff = IXON | IXOFF;
		tty_use_kbd_sw_flow_control = 1;
	}
	else if (ulcmpb(arg, "off") < 0)
	{
		new_xon_xoff = 0;
		tty_use_kbd_sw_flow_control = 0;
	}
	else
		return (-1);

	tty_termio_current.c_iflag &= ~(IXON | IXOFF);
	tty_termio_current.c_iflag |= new_xon_xoff;
	ecusetattr(TTYIN, TCSETA, &tty_termio_current);
	return (0);

}							 /* end of set_console_xon_xoff_by_arg */

/*+-------------------------------------------------------------------------
	console_xon_status()
--------------------------------------------------------------------------*/
char *
console_xon_status()
{
	if (tty_use_kbd_sw_flow_control)
		return ("honoring ^S/^Q locally");
	else
		return ("passing ^S/^Q to remote");
}							 /* end of console_xon_status */

/*+-------------------------------------------------------------------------
	console_attribute_test()
--------------------------------------------------------------------------*/
void
console_attribute_test()
{
	int save_use_colors;

	if (save_use_colors = use_colors)
	{
		ff(se, "native ");
		setcolor(colors_notify);
		ff(se, "notify ");
		setcolor(colors_alert);
		ff(se, "alert ");
		setcolor(colors_error);
		ff(se, "error ");
		setcolor(colors_success);
		ff(se, "success ");
		setcolor(colors_normal);
		ff(se, "normal\r\n");
	}
	else
		ff(se, "native color driver not applicable/available\r\n");

	use_colors = 0;
	ff(se, "hack   ");
	setcolor(colors_notify);
	ff(se, "notify ");
	setcolor(colors_alert);
	ff(se, "alert ");
	setcolor(colors_error);
	ff(se, "error ");
	setcolor(colors_success);
	ff(se, "success ");
	setcolor(colors_normal);
	ff(se, "normal\r\n");

	use_colors = save_use_colors;
}							 /* end of console_attribute_test */

/*+-------------------------------------------------------------------------
	ttyinit(param)
--------------------------------------------------------------------------*/
void
ttyinit(param)
uchar param;
{
	int fddevtty;
	int itmp;
	char *ftype;

/* FreeBSD does not have CONS_GET or MONO, but we do want in here */
#if defined(CONS_GET) && defined(MONO) && defined(USE_COLOR) || \
		defined(__FreeBSD__)
	int monitor_type;
	int cons_get_err;

#endif

	ttype = getenv("TERM");	 /* must do this first */

	/*
	 * get control tty control chars in case stdin not tty
	 */
	if ((fddevtty = open("/dev/tty", O_RDONLY, 0)) >= 0)
	{
		ecugetattr(fddevtty, &tty_termio_at_entry);
		close(fddevtty);
		termio_to_kbd_chars();
	}

	sigint = 0;				 /* see xmtr signal handlers */

	memset((char *)&st_tty, 0, sizeof(struct stat));
	memset((char *)&st_ttyp0, 0, sizeof(struct stat));
	memset((char *)&st_console, 0, sizeof(struct stat));

	stat("/dev/console", &st_console);
	stat("/dev/null", &st_dn);
#ifdef __FreeBSD__
	stat("/dev/ttyv0", &st_tty01);
#else
	stat("/dev/tty01", &st_tty01);
#endif
	stat("/dev/ttyp0", &st_ttyp0);

	/*
	 * if stdin not open or is /dev/null or is non-character-device
	 */

	itmp = fstat(TTYIN, &st_tty);
	if (itmp || ((st_tty.st_mode & S_IFMT) != S_IFCHR) ||
		((st_dn.st_ino == st_tty.st_ino) && (st_dn.st_rdev == st_tty.st_rdev)))
	{
		extern char hello_str[];

		tcap_LINES = LINES = 24;	/* fake necessary termcap/curses vars */
		tcap_COLS = COLS = 80;
		LINESxCOLS = tcap_LINES * tcap_COLS;
		shm->scr_lines = tcap_LINES;
		shm->scr_cols = tcap_COLS;
		shm->scr_size = LINESxCOLS;
		tty_not_char_special = 1;
		shm->ttyuse = TTYUSE_FORCE_SIMPLE;
		tty_is_multiscreen = 0;
		shm->rcvr_ansi_filter = 0;
		ff(se, "%s\n", hello_str);
#if 0
		ff(se, "[stdin is not a tty: non-conversational execution]\r\n");
#endif
		return;
	}

	/*
	 * if pty
	 */
	if ((st_tty.st_rdev & 0xFF00) == (st_ttyp0.st_rdev & 0xFF00))
		tty_is_pty = 1;

	/*
	 * use color if we are on a display that supports it and we know how
	 * :-|
	 */
	use_colors = 0;

#ifdef __FreeBSD__
	if ((cons_get_err = ioctl(TTYIN, GIO_COLOR, &monitor_type)) >= 0)
	{
		tty_is_scoterm = 1;
		if ((use_colors = monitor_type))
		{
			read_colors_file();
			setcolor(colors_normal);
		}
	}
#else
#if defined(CONS_GET) && defined(MONO) && defined(USE_COLOR)
	monitor_type = 0;
	if (((cons_get_err = ioctl(TTYIN, CONS_GET, &monitor_type)) >= 0) &&
		(use_colors = (monitor_type != MONO)))
	{
		read_colors_file();
		setcolor(colors_normal);
	}

#endif /* CONS_GET && MONO && USE_COLOR */
#endif /* !__FreeBSD__ */

	/*
	 * remember whether or not we are on a multiscreen
	 */
#ifdef linux
#ifdef KDGETMODE
	tty_is_multiscreen = 0;
	use_colors = 0;
	if (!ioctl(TTYIN, KDGETMODE, &itmp))
	{
		tty_is_multiscreen = 1;
		use_colors = 1;
	}
#else /* must be very very old linux */
	tty_is_multiscreen = 1;
	use_colors = 1;
#endif /* KDGETMODE */
	read_colors_file();
	setcolor(colors_normal);
#endif /* linux */

#if  defined(__FreeBSD__)
	tty_is_multiscreen = !(cons_get_err < 0);

	if (!tty_is_scoterm && ttype && (!strncmp(ttype, "ansi", 4) ||
			!strncmp(ttype, "cons", 4) ||
			!strncmp(ttype, "pc3r", 4) ||
			!strncmp(ttype, "ibmpc3r", 7)
		)
		)
	{
		tty_is_scoterm = 1;
		if (!strstr(ttype, "-m"))
		{
			use_colors = 1;
			read_colors_file();
			setcolor(colors_normal);
		}
	}
#endif /* __FreeBSD__ */

#if defined(SCO_UNIX)
	tty_is_multiscreen = !(cons_get_err < 0);

	/*
	 * a fuzzy heuristic for scoterm:
	 *    1. presence of WINDOWID and
	 *    2. first four characters of $TERM == "ansi"
	 */
	if (getenv("WINDOWID") && ttype && !strncmp(ttype, "ansi", 4))
	{
		use_colors = 1;
		tty_is_scoterm = 1;
		read_colors_file();
		setcolor(colors_normal);
	}
#endif /* SCO_UNIX multiscreen and scoterm */

	/*
	 * save initial tty state
	 */
	ecugetattr(TTYIN, &tty_termio_at_entry);
	tty_escape_timeout =
		B_to_timeout_msec(&tty_termio_at_entry, st_tty.st_rdev);

	termio_to_kbd_chars();

	tty_termio_current = tty_termio_at_entry;
	current_ttymode = 0;

	get_home_dir(screen_dump_file_name);
	strcat(screen_dump_file_name, "/.ecu/screen.dump");

	ttyinit_has_been_called = 1;

	ttymode(3);

	/* initialize termcap */
	tcap_init();			 /* read termcap strings */

	/* read terminal characteristics */
	ftype = 0;
	if (dash_f_funckeytype)
		ftype = dash_f_funckeytype;
	else
		ftype = getenv("ECUFUNCKEY");
#ifdef WHT
	if (!ftype && !strcmp(ttype, "vt100"))
		ftype = "rterm";
#endif
	if (ttype || ftype)
		funckeymap_read((ftype) ? ftype : ttype);

	/* yetch - magic number gretching for lines and columns */
	errno = -1;
	if (tcap_LINES < 24)
	{
		ff(se, "\7screen height must be >= 24 lines\r\n\
(found %d columns, %d rows).\r\n",
			tcap_COLS, tcap_LINES);
		termecu(TERMECU_GEOMETRY);
	}
	if (tcap_LINES > CFG_ScreenLinesMax)
	{
		ff(se, "\7screen height must be <= %d lines\r\n\
(found %d columns, %d rows).\r\n",
			CFG_ScreenLinesMax, tcap_COLS, tcap_LINES);
		termecu(TERMECU_GEOMETRY);
	}
	if (tcap_COLS < 80)
	{
		ff(se, "\7terminal width must be >= 80 columns\r\n\
(found %d columns, %d rows).\r\n",
			tcap_COLS, tcap_LINES);
		termecu(TERMECU_GEOMETRY);
	}
	if (tcap_COLS > CFG_ScreenColsMax)
	{
		ff(se, "\7terminal width must be <= %d columns\r\n\
(found %d columns, %d rows).\r\n",
			CFG_ScreenColsMax, tcap_COLS, tcap_LINES);
		termecu(TERMECU_GEOMETRY);
	}
	LINESxCOLS = tcap_LINES * tcap_COLS;
	shm->scr_lines = tcap_LINES;
	shm->scr_cols = tcap_COLS;
	shm->scr_size = LINESxCOLS;

}							 /* end of ttyinit */

/* end of ecutty.c */
/* vi: set tabstop=4 shiftwidth=4: */
