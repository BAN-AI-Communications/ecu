#define USE_XON_XOFF
/*+-------------------------------------------------------------------------
	ecufinsert.c -- insert file onto comm line
	wht@wht.net

  Defined functions:
	expand_filename(fname, maxlen)
	file_insert_clear_xoff()
	file_insert_to_line(narg, arg)

--------------------------------------------------------------------------*/
/*+:EDITS:*/
/*:04-26-2000-11:15-wht@bob-RELEASE 4.42 */
/*:01-24-1997-02:37-wht@yuriatin-SOURCE RELEASE 4.00 */
/*:09-11-1996-20:00-wht@yuriatin-3.48-major telnet,curses,structural overhaul */
/*:11-23-1995-11:20-wht@kepler-source control 3.37 for tsx-11 */
/*:11-14-1995-10:23-wht@kepler-3.37.80-source control point: SOCKETS */
/*:05-11-1995-15:55-wht@n4hgf-ck_sigint fools optimizing compilers */
/*:01-12-1995-15:19-wht@n4hgf-apply Andrew Chernov 8-bit clean+FreeBSD patch */
/*:05-04-1994-04:38-wht@n4hgf-ECU release 3.30 */
/*:09-10-1992-13:58-wht@n4hgf-ECU release 3.20 */
/*:08-22-1992-15:38-wht@n4hgf-ECU release 3.20 BETA */
/*:07-25-1991-12:55-wht@n4hgf-ECU release 3.10 */
/*:07-17-1991-07:04-wht@n4hgf-avoid SCO UNIX nap bug */
/*:07-14-1991-18:18-wht@n4hgf-new ttygets functions */
/*:04-27-1991-01:24-wht@n4hgf-expand_filename was NFG */
/*:03-30-1991-12:40-wht@n4hgf-redi!donovan found q does not restart receiver */
/*:08-14-1990-20:40-wht@n4hgf-ecu3.00-flush old edit history */

#include "ecu.h"
#include "ecukey.h"
#include "ecutty.h"

extern char kbdintr;		 /* current input INTR */
extern UINT32 colors_current;
extern UINT32 colors_alert;
extern UINT32 colors_errors;

/*+-------------------------------------------------------------------------
	expand_filename(fname) - convert fnames with shell chars

return 0 if no shell characters found
       -1 if shell expansion match found
       1 if shell expansion found
--------------------------------------------------------------------------*/
int
expand_filename(fname, maxlen)
char *fname;
int maxlen;
{
	char *expcmd;

	if (!find_shell_chars(fname))
		return (0);

	if (expand_wildcard_list(fname, &expcmd))
	{
		fputs("\r\n", se);
		fputs(expcmd, se);
		fputs("\r\n", se);
		return (-1);
	}
	strncpy(fname, expcmd, maxlen);
	fname[maxlen - 1] = 0;
	if (strchr(expcmd, ' '))
	{
		pputs("\nToo many files:\n");
		pputs(expcmd);
		pputs("\n");
		free(expcmd);
		return (-1);
	}
	strncpy(fname, expcmd, maxlen);
	*(fname + maxlen - 1) = 0;
	free(expcmd);
	return (0);

}							 /* end of expand_filename */

/*+-------------------------------------------------------------------------
	file_insert_clear_xoff()
--------------------------------------------------------------------------*/
void
file_insert_clear_xoff()
{
#ifdef USE_XON_XOFF
#ifdef SAY_CLEARED_XOFF
	UINT32 colors_at_entry = colors_current;

	setcolor(colors_alert);
	fputs("--> local XOFF cleared\r", se);
	setcolor(colors_at_entry);
#endif
	lclear_xmtr_xoff();
#endif
}							 /* end of file_insert_clear_xoff */

/*+-------------------------------------------------------------------------
	file_insert_to_line(narg,arg)
--------------------------------------------------------------------------*/
file_insert_to_line(narg, arg)
int narg;
char **arg;
{
	int itmp;
	int rchar;
	int old_ttymode = get_ttymode();
	long total_chars = 0L;
	long total_lines = 0L;
	UINT32 colors_at_entry = colors_current;
	long timeout_msecs;
	FILE *fp;
	char file_string[256];
	char s256[256];
	UINT xmit_mode;
	UINT delim;

#ifdef USE_XON_XOFF
	int ixon;
	int ixoff;

#endif

	sigint = 0;

	if (narg > 1)
	{
		strncpy(s256, arg[1], sizeof(s256));
		s256[sizeof(s256) - 1] = 0;
	}
	else
	{
		ff(se, "\r\n--> File to insert on comm line: ");
		ttygets(s256, sizeof(s256), TG_CRLF, &delim, (int *)0);
		if ((delim == ESC) || !strlen(s256))
		{
			ff(se, " --> transmission aborted\r\n");
			return (0);
		}
	}
	if ((itmp = expand_filename(s256, sizeof(s256))) < 0)
		return (-1);
	else if (itmp)
		ff(se, "\r\n--> wild card match: %s", s256);

	if ((fp = fopen(s256, "r")) == (FILE *) 0)
	{
		ff(se, "\r\n--> ");
		pperror(s256);		 /* print error if we get one */
		return (-1);
	}

	if (narg > 1)
		ff(se, "\r\n");

	if (narg > 2)
		xmit_mode = *arg[2];
	else
	{
	  ASK_OPTION:
		ff(se, "(S)ingle line at a time\r\n");
		ff(se, "(E)cho pacing\r\n");
		ff(se, "(F)ull speed transmission\r\n");
		ff(se, "(P)aced transmission (20 msec/char)\r\n");
		ff(se, "(Q)uit (or ESC)          press a key:   ");
		xmit_mode = ttygetc(0);
		if (xmit_mode > 0x20)
			fputs(graphic_char_text(xmit_mode, 0), se);
		fputs("\r\n", se);
	}

	kill_rcvr_process(SIGUSR1);

	switch (xmit_mode = to_lower(xmit_mode))
	{
		case 's':
			setcolor(colors_alert);
			fputs("--> press SPACE to continue or ESC/'s' to stop\r\n", se);
			setcolor(colors_at_entry);
			break;

		case 'e':
			/* fall through */

		case 'f':
		case 'p':
			setcolor(colors_alert);
			ff(se, "--> press %s to abort\r\n", graphic_char_text(kbdintr, 0));
			setcolor(colors_at_entry);
			ttymode(2);
			break;

		case 'q':
		case ESC:
			goto INSERT_DONE2;

		default:
			ring_bell();
			fputs("\r\n", se);
			goto ASK_OPTION;
	}

#ifdef USE_XON_XOFF
	lget_xon_xoff(&ixon, &ixoff);	/* get current line xon/xoff status */
	lxon_xoff(IXON);		 /* set it for us */
#endif

	while (fgets(file_string, sizeof(file_string), fp))
	{
		int xmit_len = strlen(file_string);
		int xmit_cr = xmit_len && (file_string[xmit_len - 1] == NL);

		if (xmit_cr)
		{
			xmit_len--;
			file_string[xmit_len] = 0;
		}
		total_chars += xmit_len;
		total_lines++;

/* some destinations, like BBS msg entry, take a blank line to mean
end of message, so do not send completely blank lines */
		if (!xmit_len && xmit_cr)
		{
			lputc(' ');
			xmit_len = 1;
		}
		else if (xmit_mode == 'p')
		{
			char *cp = file_string;

			while (*cp)
			{
				lputc(*cp++);
				Nap(20L);
				while (Rdchk(shm->Liofd))
				{
					rchar = lgetc_xmtr();
					process_xmtr_rcvd_char((char)rchar, 1);
				}
			}
		}
		else
			lputs(file_string);
		if (xmit_cr)
		{
			if (xmit_mode == 'p')
				Nap(20L);
			lputc('\r');
			xmit_len++;
		}

		if (ck_sigint())
			break;

		switch (xmit_mode)
		{
			case 's':
				while (1)
				{
					if (ttyrdchk())
						break;
					rchar = lgetc_timeout(5 * 1000L);
					if (rchar < 0)
						file_insert_clear_xoff();
					else
						process_xmtr_rcvd_char((char)rchar, 1);
					if (rchar == 0x0A)
						break;
				}
				rchar = to_lower(ttygetc(1));
				if ((rchar == 's') || (rchar == ESC))
					goto INSERT_DONE;
				break;

			case 'e':
				timeout_msecs = 5 * 1000L;
				while (1)
				{
					if (ck_sigint())
						break;
					rchar = lgetc_timeout(timeout_msecs);
					if (rchar < 0)
					{
						if (!xmit_len)
							break;
						file_insert_clear_xoff();
						timeout_msecs = 1 * 1000L;
					}
					else
					{
						process_xmtr_rcvd_char((char)rchar, 1);
						timeout_msecs = 100L;
						if (xmit_len)
							xmit_len--;
					}
					if (rchar == 0x0A)
						break;
				}
				break;

			case 'f':
			case 'p':
				while (Rdchk(shm->Liofd))
				{
					rchar = lgetc_xmtr();
					process_xmtr_rcvd_char((char)rchar, 1);
				}
				break;
		}
		if (ck_sigint())
			break;
	}

  INSERT_DONE:

	if (ck_sigint())
	{
		sigint = 0;
		setcolor(colors_error);
		ff(se, "--> Interrupted\r\n");
		setcolor(colors_at_entry);
	}

  INSERT_DONE2:

	fclose(fp);

	ttymode(old_ttymode);	 /* restore old console mode */
	sigint = 0;				 /* reset SIGINT flag */

	while (((rchar = lgetc_timeout(200L)) >= 0) && !ck_sigint())
		process_xmtr_rcvd_char((char)rchar, 1);

	setcolor(colors_success);
	ff(se, "\r\n-->  done ... sent %ld lines, %ld characters\r\n",
		total_lines, total_chars);
	setcolor(colors_at_entry);
	lclear_xmtr_xoff();
#ifdef USE_XON_XOFF
	lxon_xoff(ixon | ixoff); /* restore old line xon/xoff status */
#endif
	start_rcvr_process(1);
	return (0);

}							 /* end of file_insert_to_line */

/* vi: set tabstop=4 shiftwidth=4: */
/* end of ecufinsert.c */
