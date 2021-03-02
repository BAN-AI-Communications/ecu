/*+-------------------------------------------------------------------------
	zcurses.c -- ecu file transfer program curses interface

  000000000011111111112222222222333333333344444444445555555550
  012345678901234567890123456789012345678901234567890123456789
00.-[ prog+rev ]-- <dir> ------------------------------------.
01|  ZMODEM_6____  _40_____________________________________  |
02|  File ### of ###: _38__________________________________  |
03|  File position:  _8______ length: _8______  -rwxrwxrwx   |
04|                                                          |
05|  TX: ____22________________  RX: ____22________________  |
06|  Comm I/O: rx _8______  tx _8______ bytes                |
07|  Bitrate: __7____ BINARY blklen: _____ comm mode: RAW-g  |
08|  Time:    started: __:__:__ this file: __:__:__ window:  |
09|  __:__:__ elapsed: __:__:__            __:__:__ ________ |
10|  Errors: this file: _3_ total: _4__ files skipped: _3_   |
11|  _55____________________________________________________ |  err str
12|  _55____________________________________________________ |  comment str
13|  _55____________________________________________________ |  remote info
14`----------------------------------------------------------'

14|  FE ___ OE ___ rcvd ________ xmtd ________ RTS _ CTS _      FASI
15|  flow xmtr CTS ____ XOFF ____ rcvr RTS ____ XOFF ____
16|  queues: xmtr _____ of _____  rcvr _____ of _____

14|  Output queue depth  ______  RTS _  CTS _                   sun
15|  Input queue depth   ______  Input queue avail ______


  Defined functions:
	clear_area(w, row, col, len)
	clear_area_char(w, row, col, len, fillchar)
	determine_output_mode()
	dumbtty_newline()
	get_elapsed_time(elapsed_secs)
	timeofday_text(type, tod)
	mode_map(file_mode, mode_str)
	report_comm_bitrate(bitrate)
	report_error_count()
	report_file_byte_io(count)
	report_file_close(skipped)
	report_file_open_length(length)
	report_file_open_mode(file_mode)
	report_file_open_tod()
	report_file_rcv_started(filename, length, file_mode)
	report_file_send_open(filename, filestat)
	report_file_xfer_rate(text, count, final)
	report_init(title)
	report_last_rxhdr(rptstr, error_flag)
	report_last_txhdr(rptstr, error_flag)
	report_mode(comm_mode)
	report_protocol_crc_type(str)
	report_protocol_type(str)
	report_rx_ind(status)
	report_rx_tx_count()
	report_rxblklen(blklen)
	report_rxpos(pos)
	report_str(rptstr, error_flag)
	report_top_line(topstr)
	report_transaction(str)
	report_transfer_progress(filepos, initfpos)
	report_tx_ind(status)
	report_txblklen(blklen)
	report_txpos(pos)
	report_uninit()
	report_window()
	report_xfer_mode(str)

------------------------------------------------------------------------*/
/*+:EDITS:*/
/*:03-16-1997-03:28-rll@felton.felton.ca.us-Fix boxes for SCO Products */
/*:01-24-1997-02:38-wht@yuriatin-SOURCE RELEASE 4.00 */
/*:09-11-1996-20:01-wht@yuriatin-3.48-major telnet,curses,structural overhaul */
/*:09-06-1996-03:18-wht@kepler-cleanup */
/*:08-11-1996-02:10-wht@kepler-rename ecu_log_event to logevent */
/*:07-24-1996-21:37-wht@n4hgf-no more wvline/whline */
/*:12-28-1995-12:54-wht@kepler-Andrey Chernov FreeBSD fixes */
/*:11-27-1995-21:56-wht@kepler-remove 120% efficiency hack */
/*:11-27-1995-20:06-wht@kepler-seven character bit rate */
/*:11-14-1995-10:23-wht@kepler-3.37.80-source control point: SOCKETS */
/*:10-19-1995-15:09-wht@kepler-remove SunOS extra data */
/*:06-28-1995-16:29-wht@n4hgf-do not report npats on rx */
/*:02-17-1995-14:21-wht@n4hgf-apply Andrew Chernov to last 3.33 prerelease */
/*:01-15-1995-01:20-wht@n4hgf-clean up creeping port rot */
/*:01-15-1995-00:53-wht@n4hgf-need sys/ioctl.h for FreeBSD */
/*:01-12-1995-15:20-wht@n4hgf-apply Andrew Chernov 8-bit clean+FreeBSD patch */
/*:05-04-1994-04:40-wht@n4hgf-ECU release 3.30 */
/*:01-16-1994-15:46-wht@n4hgf-use ecumachdep.h */
/*:12-02-1993-14:05-Robert_Broughton@mindlink.bc.c-LINUX patches */
/*:09-10-1992-14:00-wht@n4hgf-ECU release 3.20 */
/*:08-22-1992-15:39-wht@n4hgf-ECU release 3.20 BETA */
/*:02-09-1992-16:08-root@n4hgf-ruling characters only on  SCO (tcap curses) */
/*:01-05-1992-17:27-wht@n4hgf-properly display progress for resumed transfers */
/*:09-02-1991-01:12-wht@n4hgf2-show sun driver information */
/*:08-28-1991-14:08-wht@n4hgf2-SVR4 cleanup by aega84!lh */
/*:08-23-1991-18:33-wht@n4hgf2-disable force no curses for tty vs. line speed */
/*:08-21-1991-06:23-wht@n4hgf-sun porting */
/*:07-25-1991-12:59-wht@n4hgf-ECU release 3.10 */
/*:06-15-1991-05:47-root@n4hgf-report per-file xfer rate */
/*:05-25-1991-14:51-wht@n4hgf-FAS/i display */
/*:04-24-1991-01:22-wht@n4hgf-handle dumbtty and single file xfer >= 1 hour */
/*:02-03-1991-17:27-wht@n4hgf-show elapsed time during no curses xfer */
/*:01-04-1991-15:54-wht@n4hgf-dumbtty per-file xfer rate was wrong */
/*:12-18-1990-21:26-wht@n4hgf-better output control */
/*:12-04-1990-04:07-wht@n4hgf-handle slow terminal using faster line */
/*:12-04-1990-03:04-wht@n4hgf-choose ruling chars based on multiscreen or not */
/*:09-19-1990-19:36-wht@n4hgf-logevent now gets pid for log from caller */
/*:08-14-1990-20:41-wht@n4hgf-ecu3.00-flush old edit history */

#include "../ecu_config.h"
#include "../ecucurses.h"
#include "../ecu_types.h"
#include "../ecu_stat.h"
#include "../ecu_time.h"
#include "../pc_scr.h"

#include <ctype.h>
#include <signal.h>

#if defined(__FreeBSD__)
#include <sys/ioctl.h>
#endif

#if defined(FASI)
#include <local/fas.h>
#endif /* FASI */

long time();
void report_error_count();
extern char *tzname[];
struct tm *localtime();

#undef uchar
#define uchar unsigned char

WINCH sTL;
WINCH sTR;
WINCH sBL;
WINCH sBR;
WINCH sLT;
WINCH sRT;
WINCH sVR;
WINCH sHR;

/*
 * define window geometry
 *   varying number of lines depending on sys-dependent driver stats
 *   other values fixed
 */
#if defined(FASI)
#define WIN_LINES	18		 /* FASI stats */
#else
#if defined(__FreeBSD__)
#define WIN_LINES	17
#else
#define WIN_LINES	15
#endif /* FreeBSD */
#endif /* FASI */

#define WIN_COLS	60
#define WIN_TOPY	2
#define WIN_LEFTX	8

extern char curr_dir[];
extern char *bottom_label;
extern int Filcnt;
extern int ecusz_flag;		 /* ecusz == 1, ecurz == 0 */
extern int force_dumbtty;
extern int skip_count;
extern int npats;
extern int iofd;
extern long rxpos;
extern int log_packets;
extern long Txpos;
extern long Rxpos;
extern long initial_filepos;

WINDOW *win;
int report_init_complete;
int report_verbosity;
int dumbtty;
int dumbtty_pos;
int this_file_errors;
long this_file_xfer_count;
int total_errors;
int show_window;
long current_epoch;
long start_seconds;
long this_file_start_epoch;
long elapsed_seconds;
unsigned long total_data_bytes_xfered;
unsigned int zcurses_bitrate;
char s128[128];

#if defined(FASI)
struct fas_info fip_start;

#endif /* FASI */

char *win_template[] =
{
/*  00000000001111111111222222222233333333334444444444555555555 */
/*  01234567890123456789012345678901234567890123456789012345678 */
/*.----------------------------------------------------------. */
	"                                                          ",	/* 1 */
	"  File ### of ###: _____________________________________  ",	/* 2 */
	"  File position:  ________ length: ________               ",	/* 3 */
	"                                                          ",	/* 4 */
	"  TX: ______________________  RX: ______________________  ",	/* 5 */
	"  Comm I/O: rx ________  tx ________ bytes                ",	/* 6 */
	"  Bitrate: _______ ______ blklen: _____ comm mode: ______ ",	/* 7 */
	"  Time:    started: __:__:__ this file: __:__:__          ",	/* 8 */
	"  __:__:__ elapsed: __:__:__            __:__:__          ",	/* 9 */
	"  Errors: this file: ___ total: ____ files skipped: ___   ",	/* 10 */
	"                                                          ",	/* 11 */
	"                                                          ",	/* 12 */
	"                                                          ",	/* 13 */
#if defined(FASI)
	"  FE ___ OE ___ rcvd ________ xmtd ________ RTS _ CTS _   ",	/* 14 */
	"  flow xmtr CTS ____ XOFF ____ rcvr RTS ____ XOFF ____    ",	/* 15 */
	"  Queues: xmtr _____ of _____  rcvr _____ of _____        ",	/* 16 */
#endif /* FASI */
#if defined(__FreeBSD__)
	"  Output queue depth  ______  RTS _  CTS _                ",	/* 14 */
	"  Input queue avail   ______                              ",	/* 1 */
#endif
/*`----------------------------------------------------------' */
	(char *)0
};

/*+-----------------------------------------------------------------------
	char *get_elapsed_time(elapsed_secs)
	hh:mm:ss returned
  static string address is returned
------------------------------------------------------------------------*/
char *
get_elapsed_time(elapsed_secs)
long elapsed_secs;
{
	static char elapsed_time_str[10];
	long hh, mm, ss;

	hh = elapsed_secs / 3600;
	elapsed_secs -= hh * 3600;
	mm = elapsed_secs / 60L;
	elapsed_secs -= mm * 60L;
	ss = elapsed_secs;

	sprintf(elapsed_time_str, "%02ld:%02ld:%02ld", hh, mm, ss);
	return (elapsed_time_str);
}							 /* end of get_elapsed_time */

/*+-----------------------------------------------------------------------
	char *timeofday_text(type,tod)

  time of day types:
	0		hh:mm
	1		hh:mm:ss
	2		mm-dd-yyyy hh:mm

  static string address is returned
  if tod != (char *)0, time is returned there too
------------------------------------------------------------------------*/
char *
timeofday_text(type, tod)
int type;
char *tod;
{
	static char tod_str[32];
	long cur_time = time((long *)0);
	struct tm *lt = localtime(&cur_time);

	switch (type)
	{
		case 0:
			sprintf(tod_str, "%02d:%02d", lt->tm_hour, lt->tm_min);
			break;

		default:
		case 1:
			sprintf(tod_str, "%02d:%02d:%02d",
				lt->tm_hour, lt->tm_min, lt->tm_sec);
			break;

		case 2:
			sprintf(tod_str, "%02d-%02d-%04d %02d:%02d",
				lt->tm_mon + 1, lt->tm_mday, lt->tm_year + 1900,
				lt->tm_hour, lt->tm_min);
			break;
	}

	if (tod != (char *)0)
		strcpy(tod, tod_str);

	return (tod_str);
}							 /* end of timeofday_text */

/*+-----------------------------------------------------------------------
	mode_map(file_mode,mode_str)	build drwxrwxrwx string
------------------------------------------------------------------------*/
char *
mode_map(file_mode, mode_str)
unsigned short file_mode;
char *mode_str;
{
	register unsigned ftype = file_mode & S_IFMT;
	register char *rtn;
	static char result[12];

	rtn = (mode_str == (char *)0) ? result : mode_str;

	/* drwxrwxrwx */
	/* 0123456789 */
	strcpy(rtn, "----------");

	switch (ftype)
	{
		case S_IFIFO:
			*rtn = 'p';
			break;			 /* FIFO (named pipe) */
		case S_IFDIR:
			*rtn = 'd';
			break;			 /* directory */
		case S_IFCHR:
			*rtn = 'c';
			break;			 /* character special */
		case S_IFBLK:
			*rtn = 'b';
			break;			 /* block special */
		case S_IFREG:
			*rtn = '-';
			break;			 /* regular */

#if defined(S_IFLNK)
		case S_IFLNK:
			*rtn = 'l';
			break;			 /* symbolic link */
#endif
#if defined(S_IFSOCK)
		case S_IFSOCK:
			*rtn = 's';
			break;			 /* socket */
#endif

#if defined(S_IFNAM)
		case S_IFNAM:		 /* name space entry */
#if defined(S_IFNAM)
			if (file_mode & S_INSEM)	/* semaphore */
			{
				*rtn = 's';
				break;
			}
#endif
#if defined(S_INSHD)
			if (file_mode & S_INSHD)	/* shared memory */
			{
				*rtn = 'm';
				break;
			}
			break;
#endif
#endif

		default:
			*rtn = '?';
			break;			 /* ??? */
	}

	if (file_mode & 000400)
		*(rtn + 1) = 'r';
	if (file_mode & 000200)
		*(rtn + 2) = 'w';
	if (file_mode & 000100)
		*(rtn + 3) = 'x';
	if (file_mode & 004000)
		*(rtn + 3) = 's';
	if (file_mode & 000040)
		*(rtn + 4) = 'r';
	if (file_mode & 000020)
		*(rtn + 5) = 'w';
	if (file_mode & 000010)
		*(rtn + 6) = 'x';
	if (file_mode & 002000)
		*(rtn + 6) = 's';
	if (file_mode & 000004)
		*(rtn + 7) = 'r';
	if (file_mode & 000002)
		*(rtn + 8) = 'w';
	if (file_mode & 000001)
		*(rtn + 9) = 'x';
	if (file_mode & 001000)
		*(rtn + 9) = 't';

	return (rtn);

}							 /* end of mode_map */

/*+-------------------------------------------------------------------------
	dumbtty_newline()
--------------------------------------------------------------------------*/
void
dumbtty_newline()
{
	if (dumbtty_pos)
		printf("\r\n");
	dumbtty_pos = 0;

}							 /* end of dumbtty_newline */

/*+-------------------------------------------------------------------------
	clear_area(w,row,col,len)
--------------------------------------------------------------------------*/
void
clear_area(w, row, col, len)
WINDOW *w;
int row;
int col;
int len;
{
	if (dumbtty)
		return;
	wmove(w, row, col);
	while (len-- > 0)
		waddch(w, ' ');
	wmove(w, row, col);

}							 /* end of clear_area */

/*+-------------------------------------------------------------------------
	clear_area_char(w,row,col,len,fillchar)
--------------------------------------------------------------------------*/
void
clear_area_char(w, row, col, len, fillchar)
WINDOW *w;
int row;
int col;
int len;
char fillchar;
{
	if (dumbtty)
		return;
	wmove(w, row, col);
	while (len-- > 0)
		waddch(w, fillchar);
	wmove(w, row, col);

}							 /* end of clear_area_char */

/*+-------------------------------------------------------------------------
	report_top_line(topstr)
   top line: row 1 col 17 length 42
--------------------------------------------------------------------------*/
void
report_top_line(topstr)
char *topstr;
{
	char s42[42];

	if (dumbtty)
	{
		dumbtty_newline();
		dumbtty_pos = printf("%s", topstr);
		fflush(stdout);
		return;
	}

	clear_area(win, 1, 17, 42);
	if (strlen(topstr) < 40)
		waddstr(win, topstr);
	else
	{
		strncpy(s42, topstr, 40);
		s42[40] = 0;
		waddstr(win, s42);
	}
}							 /* end of report_top_line */

/*+-------------------------------------------------------------------------
	report_xfer_mode(modestr)  BINARY/ASCII
   protocol xfer type: row 7 col 20 length 6
--------------------------------------------------------------------------*/
void
report_xfer_mode(str)
char *str;
{

	if (dumbtty)
		return;
	clear_area(win, 7, 20, 6);
	waddstr(win, str);
	wrefresh(win);

}							 /* end of report_xfer_mode */

/*+-------------------------------------------------------------------------
	report_protocol_type(str)

  protocol type:  row 1 col 3 length 6 string
--------------------------------------------------------------------------*/
void
report_protocol_type(str)
register char *str;
{
	char s10[10];

	if (dumbtty)
	{
		return;
	}

	if (strlen(str) > 6)
	{
		strncpy(s10, str, 6);
		s10[7] = 0;
		str = s10;
	}
	clear_area(win, 1, 3, 6);
	waddstr(win, str);
	wrefresh(win);

}							 /* end of report_protocol_type */

/*+-------------------------------------------------------------------------
	report_protocol_crc_type(str)

  protocol crc type:  row 1 col 9 length 6
--------------------------------------------------------------------------*/
void
report_protocol_crc_type(str)
register char *str;
{
	char s8[8];

	if (dumbtty)
	{
		return;
	}

	if (strlen(str) > 6)
	{
		strncpy(s8, str, 6);
		s8[7] = 0;
		str = s8;
	}
	clear_area(win, 1, 9, 6);
	waddstr(win, str);
	wrefresh(win);

}							 /* end of report_protocol_crc_type */

/*+-------------------------------------------------------------------------
	report_rx_ind(status)
--------------------------------------------------------------------------*/
void
report_rx_ind(status)
int status;
{
	char *cptr;

	if (dumbtty)
	{
		if (report_verbosity && status)
		{
			printf("R");
			if (++dumbtty_pos > 75)
			{
				dumbtty_newline();
				current_epoch = time((long *)0);
				elapsed_seconds = current_epoch - this_file_start_epoch;
				cptr = get_elapsed_time(elapsed_seconds);
				dumbtty_pos = printf("%s elapsed ", cptr +
					((elapsed_seconds < 3600L) ? 3 : 0));
			}
			fflush(stdout);
		}
		return;
	}
	wmove(win, 1, 56);
	waddch(win, (status) ? 'R' : ' ');
	wmove(win, 1, 56);
	wrefresh(win);
}							 /* end of report_rx_ind */

/*+-------------------------------------------------------------------------
	report_tx_ind(status)
--------------------------------------------------------------------------*/
void
report_tx_ind(status)
int status;
{
	char *cptr;

	if (dumbtty)
	{
		if (report_verbosity && status)
		{
			printf("T");
			if (++dumbtty_pos > 75)
			{
				dumbtty_newline();
				dumbtty_newline();
				current_epoch = time((long *)0);
				elapsed_seconds = current_epoch - this_file_start_epoch;
				cptr = get_elapsed_time(elapsed_seconds);
				dumbtty_pos = printf("%s elapsed ", cptr +
					((elapsed_seconds < 3600L) ? 3 : 0));
			}
			fflush(stdout);
		}
		return;
	}
	wmove(win, 1, 57);
	waddch(win, (status) ? 'T' : ' ');
	wmove(win, 1, 57);
	wrefresh(win);
}							 /* end of report_tx_ind */

/*+-------------------------------------------------------------------------
	report_window() - if enable, show open widow size
--------------------------------------------------------------------------*/
void
report_window()
{
	if (show_window && !dumbtty)
	{
		long ltmp;

		wmove(win, 9, 50);
		if ((ltmp = (Txpos - Rxpos)) > 999999L)
			waddstr(win, ">+999999");
		else if (ltmp < -999999L)
			;
		else
		{
			sprintf(s128, "%+-8ld", ltmp);
			waddstr(win, s128);
			if (log_packets)
			{
				write(log_packets, "window: ", 8);
				write(log_packets, s128, strlen(s128));
				write(log_packets, "\n", 1);
			}
		}
	}
}							 /* end of report_window */

/*+-------------------------------------------------------------------------
	report_rx_tx_count()

  This function may be counted upon to perform wrefresh(win)

  rx char count:          row  6 col 16 len 8 unsigned long
  tx char count:          row  6 col 29 len 8 unsigned long
  session elapsed time:   row  9 col 21 len 8
  this file elapsed time: row  9 col 41 len 8
  current tod:            row  9 col  3 len 8
  window:                 row  9 col 50 len 8

If FASI,
  FE:                     row 14 col  6 len 4
  OE:                     row 14 col 13 len 4
  rcvd count:             row 14 col 22 len 8 unsigned long
  xmtd count:             row 14 col 36 len 8 unsigned long
  RTS status:             row 14 col 48 len 1
  CTS status:             row 14 col 55 len 1
  xmtr CTS count:         row 15 col 18 len 4
  xmtr XOFF count:        row 15 col 28 len 4
  rcvr CTS count:         row 15 col 42 len 4
  rcvr XOFF count:        row 15 col 52 len 4
  xmtr queue depth        row 16 col 16 len 5
  xmtr queue size         row 16 col 25 len 5 (one time)
  rcvr queue depth        row 16 col 37 len 5
  rcvr queue size         row 16 col 46 len 5 (one time)

--------------------------------------------------------------------------*/
void
report_rx_tx_count()
{
	extern unsigned long rx_char_count;
	extern unsigned long tx_char_count;

#if defined(FASI)
	unsigned long ltmp;
	struct fas_info now;

#endif /* FASI */
#if defined(__FreeBSD__)
	UINT output_queue;
	UINT modem_lines;
	UINT input_avail;

#endif

	register char *cptr;

	if (dumbtty)
	{
		return;
	}

	sprintf(s128, "%-8ld", rx_char_count);
	wmove(win, 6, 16);
	waddstr(win, s128);
	sprintf(s128, "%-8ld", tx_char_count);
	wmove(win, 6, 29);
	waddstr(win, s128);

	/* now time of day */
	wmove(win, 9, 3);
	cptr = timeofday_text(1, (char *)0);
	waddstr(win, cptr);
	current_epoch = time((long *)0);
	elapsed_seconds = current_epoch - start_seconds;
	cptr = get_elapsed_time(elapsed_seconds);
	wmove(win, 9, 21);
	waddstr(win, cptr);
	if (this_file_start_epoch)
		elapsed_seconds = current_epoch - this_file_start_epoch;
	else
		elapsed_seconds = 0;
	cptr = get_elapsed_time(elapsed_seconds);
	wmove(win, 9, 41);
	waddstr(win, cptr);

#if defined(FASI)
	if (!ioctl(iofd, FASIC_SIP, &now))
	{
		static int statics = 0;	/* one time display flag */
		if ((ltmp = now.framing_errors - fip_start.framing_errors) > 999L)
			ltmp = 999L;
		sprintf(s128, "%-3lu", ltmp);
		wmove(win, 14, 6);
		waddstr(win, s128);

		if ((ltmp = now.overrun_errors - fip_start.overrun_errors) > 999L)
			ltmp = 999L;
		sprintf(s128, "%-3lu", ltmp);
		wmove(win, 14, 13);
		waddstr(win, s128);

		ltmp = now.characters_received - fip_start.characters_received;
		sprintf(s128, "%-8lu", ltmp);
		wmove(win, 14, 22);
		waddstr(win, s128);

		ltmp = now.characters_transmitted - fip_start.characters_transmitted;
		sprintf(s128, "%-8lu", ltmp);
		wmove(win, 14, 36);
		waddstr(win, s128);

		wmove(win, 14, 49);
		waddch(win, (now.mcr & MC_SET_RTS) ? 'T' : 'F');

		wmove(win, 14, 55);
		waddch(win, (now.msr & MS_CTS_PRESENT) ? 'T' : 'F');

		ltmp = now.xmtr_hw_flow_count - fip_start.xmtr_hw_flow_count;
		if (ltmp > 9999L)
			ltmp = 9999L;
		sprintf(s128, "%-4lu", ltmp);
		wmove(win, 15, 17);
		waddstr(win, s128);

		ltmp = now.xmtr_sw_flow_count - fip_start.xmtr_sw_flow_count;
		if (ltmp > 9999L)
			ltmp = 9999L;
		sprintf(s128, "%-4lu", ltmp);
		wmove(win, 15, 27);
		waddstr(win, s128);

		ltmp = now.rcvr_hw_flow_count - fip_start.rcvr_hw_flow_count;
		if (ltmp > 9999L)
			ltmp = 9999L;
		sprintf(s128, "%-4lu", ltmp);
		wmove(win, 15, 41);
		waddstr(win, s128);

		ltmp = now.rcvr_sw_flow_count - fip_start.rcvr_sw_flow_count;
		if (ltmp > 9999L)
			ltmp = 9999L;
		sprintf(s128, "%-4lu", ltmp);
		wmove(win, 15, 51);
		waddstr(win, s128);

		if (now.xmit_ring_cnt > 99999)
			now.xmit_ring_cnt = 99999;
		sprintf(s128, "%-5u", now.xmit_ring_cnt);
		wmove(win, 16, 16);
		waddstr(win, s128);

		if (now.recv_ring_cnt > 99999)
			now.recv_ring_cnt = 99999;
		sprintf(s128, "%-5u", now.recv_ring_cnt);
		wmove(win, 16, 37);
		waddstr(win, s128);

		if (!statics)
		{
			statics = 1;
			ltmp = XMIT_BUFF_SIZE;
			if (ltmp > 99999)
				ltmp = 99999;
			sprintf(s128, "%-5lu", ltmp);
			wmove(win, 16, 25);
			waddstr(win, s128);

			ltmp = RECV_BUFF_SIZE;
			if (ltmp > 99999)
				ltmp = 99999;
			sprintf(s128, "%-5lu", ltmp);
			wmove(win, 16, 46);
			waddstr(win, s128);
		}
	}
#endif /* FASI */

#if defined(__FreeBSD__)
	output_queue = 0;
	ioctl(iofd, TIOCOUTQ, (int *)&output_queue);
	if (output_queue > 999999)
		output_queue = 999999;
	sprintf(s128, "%6u", output_queue);
	wmove(win, 14, 23);
	waddstr(win, s128);

	modem_lines = 0;
	ioctl(iofd, TIOCMGET, (int *)&modem_lines);
	wmove(win, 14, 35);
	waddch(win, (modem_lines & TIOCM_RTS) ? 'T' : 'F');
	wmove(win, 14, 42);
	waddch(win, (modem_lines & TIOCM_CTS) ? 'T' : 'F');
	input_avail = 0;
	ioctl(iofd, FIONREAD, (int *)&input_avail);
	if (input_avail > 999999)
		input_avail = 999999;
	sprintf(s128, "%6u", input_avail);
	wmove(win, 15, 23);
	waddstr(win, s128);
#endif

	report_window();

	wrefresh(win);			 /* calling procs expect this to occur always */

}							 /* end of report_rx_tx_count */

/*+-------------------------------------------------------------------------
	report_str(rptstr,error_flag) - row 11/12/13

  row 11 col 3 len 55 err str
  row 12 col 3 len 55 comment str
  row 13 col 3 len 55 remote info

  error_flag == 0 for status/progress message
             == 1 for bump error count, unless rptstr is null
                  in which case, merely clear error string area
             == 2 write string on bottom line (not an error)
             == 3 write string on transaction line (not an error)
             == -1 use error line but do not bump error count
--------------------------------------------------------------------------*/
void
report_str(rptstr, error_flag)
register char *rptstr;
int error_flag;
{
	char s60[60];
	extern int log_packets;

	if (log_packets)
	{
		sprintf(s60, "rpt %d:", error_flag);
		write(log_packets, s60, strlen(s60));
		write(log_packets, rptstr, strlen(rptstr));
		write(log_packets, "\n", 1);
	}

	if (dumbtty)
	{
		if (!strlen(rptstr))
			return;
		switch (error_flag)
		{
			case 0:
				break;
			case 1:
				this_file_errors++;
				total_errors++;
			case -1:
			case 2:
			case 3:
				dumbtty_newline();
				dumbtty_pos = printf("%s ", rptstr);
				fflush(stdout);
		}
		return;
	}

	if (strlen(rptstr) > 55)
	{
		strncpy(s60, rptstr, 55);
		s60[55] = 0;
		rptstr = s60;
	}

	switch (error_flag)
	{
		case 0:
			clear_area(win, 12, 3, 55);
			break;
		case 1:
			this_file_errors++;
			total_errors++;
			report_error_count();
		case -1:
			clear_area(win, 11, 3, 55);
			break;
		case 2:
			clear_area(win, 13, 3, 55);
			break;
		case 3:
			clear_area(win, 4, 3, 55);
			break;
	}

	waddstr(win, rptstr);
	wrefresh(win);

}							 /* end of report_str */

/*+-------------------------------------------------------------------------
	report_file_byte_io(count)
--------------------------------------------------------------------------*/
void
report_file_byte_io(count)
long count;
{

	this_file_xfer_count = count;
	total_data_bytes_xfered += count;

	if (dumbtty)
	{
		if (count)
		{
			dumbtty_newline();
			printf("Transferred %ld bytes for this file\n", count);
			dumbtty_newline();
		}
		return;
	}

	if (total_data_bytes_xfered)
	{
		sprintf(s128, "Total file bytes transferred: %ld",
			total_data_bytes_xfered);
		report_str(s128, -1);
	}

}							 /* end of report_file_byte_io */

/*+-------------------------------------------------------------------------
	report_uninit()
--------------------------------------------------------------------------*/
void
report_uninit()
{
	float rate = 0.0;
	float eff = 0.0;

	if (report_init_complete)
	{
		report_tx_ind(0);
		report_rx_ind(0);
		current_epoch = time((long *)0);
		elapsed_seconds = current_epoch - start_seconds;
		if (elapsed_seconds && (zcurses_bitrate > 50))
		{
			rate = (float)total_data_bytes_xfered /
				(float)elapsed_seconds;
			if (zcurses_bitrate)
				eff = 100.0 * (rate / ((float)zcurses_bitrate / 10.0));
		}
		if (rate > 0.01)
		{
			sprintf(s128, "XFERINFO transaction rate ~= %.0f ch/sec (%.0f%%)",
				rate, (eff > 0.5) ? eff : 0.0);
			if (log_packets)
			{
				write(log_packets, "info: ", 6);
				write(log_packets, s128, strlen(s128));
				write(log_packets, "\n", 1);
			}
			report_top_line(s128 + 9);
#if defined(WHT)
			logevent(getppid(), s128);
#endif
		}
		if (dumbtty)
			dumbtty_newline();
		else
		{
			report_file_byte_io(0L);
			report_rx_tx_count();
			wmove(win, WIN_LINES - 1, WIN_COLS - 1);
			wrefresh(win);
#if 0
			endwin();
			fprintf(stderr, "\r\n\r\n\r\n");
			fflush(stderr);
#endif
		}
		report_init_complete = 0;
	}

}							 /* end of report_uninit */

/*+-------------------------------------------------------------------------
	determine_output_mode()
--------------------------------------------------------------------------*/
int
determine_output_mode()
{
	struct stat dn;
	struct stat tty_stat;

#ifdef NO_PTY_CURSES
	struct stat pty_stat;

#endif

	if (force_dumbtty)
	{
		dumbtty = 1;
		report_verbosity = 1;
		report_init_complete = 1;
		return (1);
	}

	/*
	 * if tty (console) is not character special, only report basic
	 * progress
	 */
	memset((char *)&dn, 0, sizeof(dn));
	stat("/dev/null", &dn);
	if (fstat(0, &tty_stat) ||
		((tty_stat.st_mode & S_IFMT) != S_IFCHR) ||
		(dn.st_rdev == tty_stat.st_rdev))
	{
#ifndef LINUX
		dumbtty = 1;
#endif
		report_verbosity = 0;
		report_init_complete = 1;
		return (1);
	}

	/*
	 * if non-multiscreen tty bit rate not at least that of the attached
	 * line, use no curses, but do be a bit more verbose than if tty not
	 * char special
	 */
#ifdef TTY_VS_LINE_SPEED_NO_CURSES
	test_tty_and_line_bitrate();
#endif

	return (dumbtty);

}							 /* end of determine_output_mode */

/*+-------------------------------------------------------------------------
	report_init(title)
  "top line": row 1 col 11 len 21
  file quan:  row 2 col 15 len  3
              row 2 col 12 len  7 clear "of ###"
  start time: row 8 col 21 len  8
  "window:"   row 8 col 50 len  7
--------------------------------------------------------------------------*/
void
report_init(title)
char *title;
{
	int itmp;

#ifdef LINUX
	int x, y;

#endif

	if (report_init_complete)
		return;

	start_seconds = time((long *)0);
	current_epoch = start_seconds;

	if (dumbtty)
		return;

#if defined(FASI)
	(void)ioctl(iofd, FASIC_SIP, &fip_start);
#endif /* FASI */

	if (!initscr())
		exit(254);
	if (!stdscr)
		exit(254);
	crmode();
	noecho();
	nonl();
	clear();
	report_init_complete = 1;
	win = newwin(WIN_LINES, WIN_COLS, WIN_TOPY, WIN_LEFTX);
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
	box(win, sVR, sHR);

#ifdef LINUX
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
#ifndef __FreeBSD__
	wmove(win, 0, 0);
	waddch(win, sTL);                             /* upper left corner */
	wmove(win, win->_maxy - 1, 0);
	waddch(win, sBL);                             /* lower left corner */
	wmove(win, win->_maxy - 1, win->_maxx - 1);
	waddch(win, sBR);                             /* lower right corner */
	wmove(win, 0, win->_maxx - 1);
	waddch(win, sTR);                             /* upper right corner */
#endif
#endif
	wmove(win, 0, 2);
	wstandout(win);
	waddch(win, '[');
	waddch(win, ' ');
	strcpy(s128, title);
	waddstr(win, s128);
	waddch(win, ' ');
	waddch(win, ']');
	wstandend(win);
	waddch(win, sHR);                            /* horizontal line */
	waddch(win, sHR);                            /* horizontal line */
	wmove(win, 0, 8 + strlen(title));
	waddch(win, ' ');
	itmp = WIN_COLS - 2 - 7 - strlen(title);
	curr_dir[itmp] = 0;
	waddstr(win, curr_dir);
	waddch(win, ' ');
	if (bottom_label)
	{
		strncpy(s128, bottom_label, WIN_COLS - 6);
		s128[WIN_COLS - 6] = 0;
		wmove(win, WIN_LINES - 1, 2);
		waddch(win, ' ');
		waddstr(win, s128);
		waddch(win, ' ');
	}

	itmp = 0;
	while (1)
	{
		if (win_template[itmp] == (char *)0)
			break;
		wmove(win, itmp + 1, 1);
		waddstr(win, win_template[itmp++]);
	}
	if (ecusz_flag)
	{
		clear_area(win, 2, 15, 3);
		sprintf(s128, "%-3d", npats);
		waddstr(win, s128);
#if defined(FORK_DEBUG)
		sprintf(s128, "DEBUG ecusz pid %d", getpid());
#endif
	}
	else
		/* ecurz */
	{
		clear_area(win, 2, 11, 8);	/* clear "of ###" */
		waddstr(win, ":");
#if defined(FORK_DEBUG)
		sprintf(s128, "DEBUG ecurz pid %d", getpid());
#endif
	}

#if defined(FORK_DEBUG)
	logevent(getppid(), s128);
#endif

	clear_area(win, 1, 11, 21);
	report_error_count();
	clear_area(win, 8, 21, 8);	/* starting time */
	waddstr(win, timeofday_text(1, (char *)0));

	if (show_window)
	{
		wmove(win, 8, 50);
		waddstr(win, "window:");
		wmove(win, 9, 50);
		waddstr(win, "+0");
	}

	wrefresh(win);

}							 /* end of report_init */

/*+-------------------------------------------------------------------------
	report_mode(comm_mode)

 comm mode row 7 col 52 length 6
   3: save old tty stat, set raw mode with flow control
   2: set XON/XOFF for sb/sz with ZMODEM or YMODEM-g
   1: save old tty stat, set raw mode
   0: restore original tty mode
--------------------------------------------------------------------------*/
void
report_mode(comm_mode)
int comm_mode;
{
	char *cptr;
	char tmp[8];

	if (dumbtty)
	{
		return;
	}

	clear_area(win, 7, 52, 6);
	switch (comm_mode)
	{
		case 0:
			cptr = "NORMAL";
			break;
		case 1:
			cptr = "RAW";
			break;
		case 2:
			cptr = "RAW-g";
			break;
		case 3:
			cptr = "RAW-f";
			break;
		case 4:
			cptr = "TELNET";
			break;
		default:
			sprintf(tmp, "%5u", comm_mode);
			cptr = tmp;
	}
	waddstr(win, cptr);
	wrefresh(win);
	if (log_packets)
	{
		write(log_packets, "comm_mode: ", 6);
		write(log_packets, cptr, strlen(cptr));
		write(log_packets, "\n", 1);
	}

}							 /* end of report_mode */

/*+-------------------------------------------------------------------------
	report_rxblklen(blklen)
row 7 col 35 5 chars
--------------------------------------------------------------------------*/
void
report_rxblklen(blklen)
int blklen;
{
	char tmp[10];

	if (dumbtty)
	{
		return;
	}

	sprintf(tmp, "%-5u", blklen);
	clear_area(win, 7, 35, 5);
	waddstr(win, tmp);
	wrefresh(win);
}							 /* end of report_rxblklen */

/*+-------------------------------------------------------------------------
	report_txblklen(blklen)
row 7 col 35 5 chars
--------------------------------------------------------------------------*/
void
report_txblklen(blklen)
int blklen;
{
	if (dumbtty)
	{
		return;
	}

	report_rxblklen(blklen);
}							 /* end of report_txblklen */

/*+-------------------------------------------------------------------------
	report_file_xfer_rate(text,count,final)
--------------------------------------------------------------------------*/
void
report_file_xfer_rate(text, count, final)
char *text;
long count;
int final;
{
	float rate = 0.0;
	float efficiency = 0.0;
	static long xfer_rate_report_epoch = 0L;

	current_epoch = time((long *)0);

	if (!final && (current_epoch < (xfer_rate_report_epoch + 5L)))
		return;

	xfer_rate_report_epoch = current_epoch;

	elapsed_seconds = current_epoch - this_file_start_epoch;

	if (count && (elapsed_seconds > 0) && zcurses_bitrate)
	{
		rate = (float)count / (float)elapsed_seconds;
		efficiency = 100.0 * (rate / ((float)zcurses_bitrate / 10.0));
	}

#if 0
	if (efficiency > 120.0)	 /* interim hack for ecusz -r */
		return;
#endif

	if (rate > 0.01)
	{
		if (efficiency < 0.5)
			sprintf(s128, "XFERINFO %s rate ~= %.0f ch/sec", text, rate);
		else
		{
			sprintf(s128, "XFERINFO %s rate ~= %.0f ch/sec (%.0f%%)",
				text, rate, efficiency);
		}
		if (!dumbtty)
			report_top_line(s128 + 9);
#if defined(WHT)
		if (final)
			logevent(getppid(), s128);
#endif
	}

}							 /* end of report_file_xfer_rate */

/*+-------------------------------------------------------------------------
	report_transfer_progress(filepos,initfpos)

  file pos:  row 3 col 19 len 8
--------------------------------------------------------------------------*/
void
report_transfer_progress(filepos, initfpos)
long filepos;
long initfpos;
{
	char refr;

	if (dumbtty)
		return;

	if (Rdchk(0))
	{
		read(0, &refr, 1);
		if (refr == 0x0C || refr == 0x012)	/* ^L or ^R */
		{
			touchwin(stdscr);
			wrefresh(stdscr);
			touchwin(win);
			wrefresh(win);
		}
	}

	if (filepos <= 99999999L && filepos >= 0L)
	{
		sprintf(s128, "%-8lu", filepos);
		wmove(win, 3, 19);
		waddstr(win, s128);
		report_file_xfer_rate("data", filepos - initfpos, 0);
	}
	report_rx_tx_count();	 /* which will do a refresh */

}							 /* end of report_transfer_progress */

/*+-------------------------------------------------------------------------
	report_rxpos(pos) - report received file progress
--------------------------------------------------------------------------*/
void
report_rxpos(pos)
long pos;
{

	if (dumbtty)
		return;
	report_transfer_progress(pos, initial_filepos);

}							 /* end of report_rxpos */

/*+-------------------------------------------------------------------------
	report_txpos(pos) - report transmitted file progress
--------------------------------------------------------------------------*/
void
report_txpos(pos)
long pos;
{

	if (dumbtty)
		return;
	report_transfer_progress(pos, initial_filepos);

}							 /* end of report_txpos */

/*+-------------------------------------------------------------------------
	report_error_count()

  this file: row 10 col 22 len 3
  total:     row 10 col 33 len 4
  skipped:   row 10 col 53 len 3
--------------------------------------------------------------------------*/
void
report_error_count()
{
	char tmp[16];

	if (dumbtty)
	{
		return;
	}

	wmove(win, 10, 22);
	sprintf(tmp, "%-3d", this_file_errors);
	if (this_file_errors)
		wstandout(win);
	waddstr(win, tmp);
	if (this_file_errors)
		wstandend(win);

	wmove(win, 10, 33);
	sprintf(tmp, "%-4d", total_errors);
	if (total_errors)
		wstandout(win);
	waddstr(win, tmp);
	if (total_errors)
		wstandend(win);

	wmove(win, 10, 53);
	sprintf(tmp, "%-3d", skip_count);
	waddstr(win, tmp);
	wrefresh(win);

}							 /* end of report_error_count */

/*+-------------------------------------------------------------------------
	report_last_txhdr(rptstr,error_flag)
	5,7,22
--------------------------------------------------------------------------*/
void
report_last_txhdr(rptstr, error_flag)
register char *rptstr;
int error_flag;
{
	char s24[24];

	if (log_packets)
	{
		write(log_packets, "tx:   ", 6);
		write(log_packets, rptstr, strlen(rptstr));
		write(log_packets, "\n", 1);
	}

	if (dumbtty)
	{
		if (error_flag)
		{
			dumbtty_newline();
			dumbtty_pos = printf("%s ", rptstr);
			++this_file_errors;
			++total_errors;
		}
		return;
	}

	if (strlen(rptstr) > 22)
	{
		strncpy(s24, rptstr, 22);
		s24[23] = 0;
		rptstr = s24;
	}
	clear_area(win, 5, 7, 22);
	waddstr(win, rptstr);

	if (error_flag)
	{
		++this_file_errors;
		++total_errors;
		report_error_count();
	}
#if 0
	else
		wrefresh(win);
#endif

}							 /* end of report_last_txhdr */

/*+-------------------------------------------------------------------------
	report_last_rxhdr(rptstr,error_flag)
	5,35,22
--------------------------------------------------------------------------*/
void
report_last_rxhdr(rptstr, error_flag)
register char *rptstr;
int error_flag;
{
	char s24[24];
	extern int log_packets;

	if (log_packets)
	{
		write(log_packets, "rx:   ", 6);
		write(log_packets, rptstr, strlen(rptstr));
		write(log_packets, "\n", 1);
	}

	if (dumbtty)
	{
		if (error_flag)
		{
			dumbtty_newline();
			dumbtty_pos = printf("%s ", rptstr);
			++this_file_errors;
			++total_errors;
		}
		return;
	}

	if (strlen(rptstr) > 22)
	{
		strncpy(s24, rptstr, 22);
		s24[23] = 0;
		rptstr = s24;
	}
	clear_area(win, 5, 35, 22);
	waddstr(win, rptstr);

	report_window();
	if (error_flag)
	{
		++this_file_errors;
		++total_errors;
		report_error_count();
	}
#if 0
	else
		wrefresh(win);
#endif

}							 /* end of report_last_rxhdr */

/*+-------------------------------------------------------------------------
	report_transaction()
--------------------------------------------------------------------------*/
void
report_transaction(str)
char *str;
{
	report_str(str, 3);
}							 /* end of report_transaction */

/*+-------------------------------------------------------------------------
	report_file_open_tod() -- time of start of this file

  this file open time: row 8 col 41 length 8
--------------------------------------------------------------------------*/
void
report_file_open_tod()
{
	if (dumbtty)
		return;
	clear_area(win, 8, 41, 8);
	waddstr(win, timeofday_text(1, (char *)0));
	wrefresh(win);
}							 /* end of report_file_open_tod */

/*+-------------------------------------------------------------------------
	report_file_open_mode(file_mode)
  mode map: row 4 col 46 len 10
--------------------------------------------------------------------------*/
void
report_file_open_mode(file_mode)
unsigned short file_mode;
{
	if (dumbtty)
		return;
	clear_area(win, 3, 46, 10);
	waddstr(win, mode_map(file_mode, (char *)0));
	wrefresh(win);
}							 /* end of report_file_open_mode */

/*+-------------------------------------------------------------------------
	report_file_open_length(long_length)
  length:   row 3 col 36 len  8
--------------------------------------------------------------------------*/
void
report_file_open_length(length)
long length;
{
	if (dumbtty)
		return;
	clear_area(win, 3, 36, 8);
	if (length <= 0)
		waddstr(win, "unknown");
	else
	{
		sprintf(s128, "%-8lu", length);
		waddstr(win, s128);
	}
	wrefresh(win);
}							 /* end of report_file_open_length */

/*+-------------------------------------------------------------------------
	report_file_send_open(filename,filestat)

  filename: row 2 col 20 len 38
  number:   row 2 col 8 len 3
  length:   row 3 col 36 len  8
  mode:     row 3 col 46 len 10
  time of start of this file: row 4 col 47 length 8 hh:mm:ss
--------------------------------------------------------------------------*/
void
report_file_send_open(filename, filestat)
char *filename;
struct stat *filestat;
{
	char s50[50];
	register char *cptr = filename;

	if (log_packets)
	{
		write(log_packets, "file: ", 6);
		write(log_packets, filename, strlen(filename));
		write(log_packets, "\n", 1);
	}

	this_file_start_epoch = time((long *)0);
	this_file_xfer_count = 0;

	if (dumbtty)
	{
		dumbtty_newline();
		dumbtty_pos = printf("Sending '%s' ", filename);
		fflush(stdout);
		return;
	}

	/* number */
	clear_area(win, 2, 8, 3);
	sprintf(s50, "%3d", Filcnt);
	waddstr(win, s50);

	/* filename */
	if (strlen(filename) > 38)
	{
		strncpy(s50, filename, 38);
		s50[39] = 0;
		cptr = s50;
	}
	clear_area(win, 2, 20, 38);
	waddstr(win, cptr);

	/* length */
	report_file_open_length((long)filestat->st_size);

	/* mode */
	report_file_open_mode(filestat->st_mode);

	/* time of start of this file */
	report_file_open_tod();

	this_file_errors = 0;
	report_error_count();
}							 /* end of report_file_send_open */

/*+-------------------------------------------------------------------------
	report_file_rcv_started(filename,length,file_mode)

  filenumber: row 2 col  8 len  3
              row 2 col 12 len  7 clear "of ###"
  filename:   row 2 col 20 len 38
--------------------------------------------------------------------------*/
void
report_file_rcv_started(filename, length, file_mode)
char *filename;
long length;				 /* if < 0, "UNKNOWN" */
unsigned short file_mode;	 /* UNIX file modifier or zero */
{
	register char *cptr;
	char s50[50];

	if (log_packets)
	{
		write(log_packets, "file: ", 6);
		write(log_packets, filename, strlen(filename));
		write(log_packets, "\n", 1);
	}

	this_file_start_epoch = time((long *)0);
	this_file_xfer_count = 0;

	if (dumbtty)
	{
		dumbtty_newline();
		dumbtty_pos = printf("Receiving '%s' ", filename) - 2;
		fflush(stdout);
		return;
	}

	/* filename */
	if (strlen(filename) > 38)
	{
		strncpy(s50, filename, 38);
		s50[39] = 0;
		cptr = s50;
	}
	else
		cptr = filename;

#if 00000000				 /* <> */
	if (log_packets)
	{
		write(log_packets, "file: ", 6);
		write(log_packets, filename, strlen(filename));
		write(log_packets, "\n", 1);
	}
#endif

	clear_area(win, 2, 20, 38);
	waddstr(win, cptr);

	/* file number */
	clear_area(win, 2, 12, 7);	/* clear "of ###" */
	clear_area(win, 2, 8, 3);
#if 0
	Filcnt++;
	sprintf(s50, "%3d", Filcnt);	/* rz uses as file number 1-n */
	waddstr(win, s50);
#endif

/* if remote sender provides a file count, display it */
#if 0
	if (npats)
	{
		clear_area(win, 2, 12, 7);	/* clear "of ###" */
		sprintf(s50, "of %3d:", npats);
		waddstr(win, s50);
	}
#endif

	/* length */
	report_file_open_length(length);

	/* mode */
	report_file_open_mode(file_mode);

	/* time of start of this file */
	report_file_open_tod();

	this_file_errors = 0;
	report_error_count();
}							 /* end of report_file_rcv_started */

/*+-------------------------------------------------------------------------
	report_file_close(skipped)
--------------------------------------------------------------------------*/
void
report_file_close(skipped)
int skipped;
{
	if (dumbtty)
	{
		dumbtty_newline();
		dumbtty_pos +=
			printf("Transfer time was %s", get_elapsed_time(elapsed_seconds));
		dumbtty_newline();
		if (s128[0])
		{
			dumbtty_pos += strlen(s128 + 9);
			fputs(s128 + 9, stdout);
			dumbtty_newline();
		}
		if (this_file_errors)
		{
			dumbtty_pos +=
				printf("Errors for this file were %d", this_file_errors);
			dumbtty_newline();
		}
		return;
	}

	if (show_window)
	{
		clear_area(win, 9, 50, 8);
		waddstr(win, "+0");
		Txpos = 0;
		Rxpos = 0;
	}

	report_str("End of file", 0);
	if (!skipped)
	{
		report_file_xfer_rate("last file",
			this_file_xfer_count - initial_filepos, 1);
	}
	wrefresh(win);
	this_file_start_epoch = 0;

}							 /* end of report_file_close */

/*+-------------------------------------------------------------------------
	report_comm_bitrate(bitrate)

 bit rate: row 7 col 12 length 6
--------------------------------------------------------------------------*/
void
report_comm_bitrate(bitrate)
unsigned int bitrate;
{
	char tstr8[8];

	zcurses_bitrate = bitrate;

	if (dumbtty)
	{
		return;
	}

	clear_area(win, 7, 12, 7);
	if (bitrate == 0)
		waddstr(win, "?");
	else
	{
		sprintf(tstr8, "%-7u", bitrate);
		waddstr(win, tstr8);
	}
	wrefresh(win);

}							 /* end of report_comm_bitrate */

/* end of zcurses.c */
/* vi: set tabstop=4 shiftwidth=4: */
