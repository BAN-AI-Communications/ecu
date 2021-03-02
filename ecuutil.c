/*+-----------------------------------------------------------------------
	ecuutil.c -- utility routines for extended calling unit
	wht@wht.net

  Defined functions:
	arg_token(parsestr, termchars)
	ascii_name_to_hex(name)
	ascii_to_hex(ascii)
	base_name(fullname)
	build_arg_array(cmd, arg, arg_max_quan, narg_rtn)
	build_str_array(str, arg, arg_max_quan, narg_rtn)
	build_valid_baud_string()
	cfree(p, num, size)
	disp_line_termio(fd, text)
	disp_stat(st)
	disp_termio(ttt, text)
	find_shell_chars(command)
	get_curr_dir(cdir, cdir_max)
	get_home_dir(home_dir)
	graphic_char_text(ch, incl_3char)
	hex_to_ascii_name(char_val)
	make_ecu_subdir()
	mem_cpy(dest, src, len)
	mkdir_auto(dirfn)
	mode_map(mode, mode_str)
	pad_zstr_to_len(zstr, len)
	perror_errmsg(str)
	Rdchk(fd)
	skip_ld_break(zstr)
	str_classify(sc, str)
	str_token(parsestr, termchars)
	strerror(err_no)
	strip_trail_break(zstr)
	yes_or_no(strarg)

  Some of these routines are the first coding I did in C
  and they just aren't worth improving ... like pad_zstr_to_len().
  Don't be too harsh on me.

------------------------------------------------------------------------*/
/*+:EDITS:*/
/*:04-26-2000-11:15-wht@bob-RELEASE 4.42 */
/*:04-05-1998-17:32-wht@kepler-move feval code to get_uname_sys/nodename */
/*:11-21-1997-05:18-wht@kepler-remove parity bit in graphic_char_text */
/*:11-20-1997-15:35-wht@kepler-add more shell chars to find_shell_chars() */
/*:02-09-1997-20:20-wht@yuriatin-add base_name */
/*:01-24-1997-02:37-wht@yuriatin-SOURCE RELEASE 4.00 */
/*:10-11-1996-02:35-wht@gyro-sunos can live off of CFG_FionreadInFilioH */
/*:10-11-1996-02:09-wht@yuriatin-add CFG_FionreadInFilioH */
/*:09-11-1996-20:00-wht@yuriatin-3.48-major telnet,curses,structural overhaul */
/*:08-20-1996-12:39-wht@kepler-locale/ctype fixes from ache@nagual.ru */
/*:07-31-1996-16:33-wht@kepler-apply Andrey Chernov FreeBSD getwd() patch */
/*:01-01-1996-18:34-wht@kepler-massage disp_termio for CSIZE */
/*:12-28-1995-12:54-wht@kepler-Andrey Chernov FreeBSD fixes */
/*:12-06-1995-13:30-wht@n4hgf-termecu w/errno -1 consideration */
/*:11-23-1995-11:20-wht@kepler-source control 3.37 for tsx-11 */
/*:11-14-1995-10:23-wht@kepler-3.37.80-source control point: SOCKETS */
/*:10-14-1995-17:08-wht@kepler-'struct termio' to 'struct TERMIO' */
/*:10-14-1995-16:55-wht@kepler-except FreeBSD, print max of 8 c_cc */
/*:10-14-1995-16:17-wht@kepler-add build_valid_baud_string */
/*:09-17-1995-16:28-wht@kepler-remove obsolete #if 0 code */
/*:06-15-1995-07:09-wht@kepler-dynamic CFG_HasStrerror */
/*:03-12-1995-01:03-wht@kepler-use ECU_MAXPN and clean up get_curr_dir */
/*:03-12-1995-00:57-wht@kepler-get_home_dir now a void function */
/*:03-11-1995-17:51-wht@kepler-type for getwd for Linux */
/*:01-12-1995-15:19-wht@n4hgf-apply Andrew Chernov 8-bit clean+FreeBSD patch */
/*:06-03-1994-16:42-wht@kepler-non-termios disp_termio handled iflag badly */
/*:05-29-1994-05:06-wht@n4hgf-2-char input to ascii_name_to_hex now working */
/*:05-04-1994-04:39-wht@n4hgf-ECU release 3.30 */
/*:04-07-1994-13:41-wht@n4hgf-fix LINUX get_curr_dir */
/*:12-12-1993-14:22-wht@fep-MOTSVR3 support */
/*:11-12-1993-11:00-wht@n4hgf-Linux changes by bob@vancouver.zadall.com */
/*:10-02-1993-22:03-wht@n4hgf-some nitpicking cleanup */
/*:07-24-1993-14:12-wht@n4hgf-reorganize ascii_ctlstr */
/*:01-30-1993-12:17-wht@n4hgf-remove gcc < 1.40 bug workaround */
/*:09-10-1992-13:59-wht@n4hgf-ECU release 3.20 */
/*:08-22-1992-15:38-wht@n4hgf-ECU release 3.20 BETA */
/*:05-08-1992-02:42-wht@n4hgf-select-based Nap was buggy on EINTR */
/*:04-05-1992-15:31-wht@n4hgf-no more use of memmove in any environment */
/*:02-22-1992-16:19-wht@n4hgf-build arg/str array now handles zero tokens */
/*:11-26-1991-19:36-wht@n4hgf-add str_classify and yes_or_no uses it */
/*:09-01-1991-12:46-wht@n4hgf2-show sun flow control bit */
/*:08-30-1991-20:09-wht@n4hgf2-sun Nap was not returning a value */
/*:08-25-1991-14:39-wht@n4hgf-SVR4 port thanks to aega84!lh */
/*:08-17-1991-14:11-root@n4hgf-ascii_to_hex supports "csi" */
/*:08-13-1991-13:53-wht@n4hgf-UNIX and ISC nap() broken; XENIX still wins */
/*:07-25-1991-12:57-wht@n4hgf-ECU release 3.10 */
/*:04-16-1991-15:45-wht@n4hgf-gcc cannot use memmove */
/*:03-18-1991-22:31-wht@n4hgf-ISC 2.2 has mkdir() */
/*:02-03-1991-14:23-wht@n4hgf-hack workaround for get_home_dir under x286 */
/*:01-25-1991-16:23-wht@n4hgf-source name wrong in headers */
/*:12-26-1990-14:32-wht@n4hgf-use memmove or Duff's Device in mem_cpy() */
/*:12-04-1990-00:58-wht@n4hgf-allow alternating between str/arg_token */
/*:08-14-1990-20:40-wht@n4hgf-ecu3.00-flush old edit history */

#include "ecu.h"
#include "termecu.h"
#include "ecufork.h"
#include "ecukey.h"
#include "ecu_pwd.h"
#include <sys/utsname.h>

#if defined(SVR4)
#include <sys/termiox.h>
extern int hx_flag;
#endif

/*
 * system independence for strerror functionality
 */
#if !defined(CFG_HasStrerror)
extern char *sys_errlist[];
extern int sys_nerr;

#endif

char curr_dir[ECU_MAXPN];	 /* current working directory of process */

char *str_token_static = (char *)0;

static char *ascii_ctlstr[] =
{
	"NUL", "SOH", "STX", "ETX", "EOT", "ENQ", "ACK", "BEL",
	"BS ", "HT ", "NL ", "VT ", "FF ", "CR ", "SO ", "SI ",
	"DLE", "DC1", "DC2", "DC3", "DC4", "NAK", "SYN", "ETB",
	"CAN", "EM ", "SUB", "ESC", "FS ", "GS ", "RS ", "US ", "SP "
};

char *valid_baud_string = "";
static char uname_rtnstring[128];

/*+-------------------------------------------------------------------------
	mem_cpy(dest,src,len) - memcpy() with non-destructive overlapping copy

  use Duff's device for speed
--------------------------------------------------------------------------*/
void
mem_cpy(dest, src, len)
char *dest;
char *src;
int len;
{
	int itmp = (len + 15) / 16;

	if ((unsigned long)dest > (unsigned long)src)
	{
		dest += len;
		src += len;
		switch (len % 16)
		{
			case 0:
				do
				{
					*--dest = *--src;
			case 15: *--dest = *--src;
			case 14: *--dest = *--src;
			case 13: *--dest = *--src;
			case 12: *--dest = *--src;
			case 11: *--dest = *--src;
			case 10: *--dest = *--src;
			case 9: *--dest = *--src;
			case 8: *--dest = *--src;
			case 7: *--dest = *--src;
			case 6: *--dest = *--src;
			case 5: *--dest = *--src;
			case 4: *--dest = *--src;
			case 3: *--dest = *--src;
			case 2: *--dest = *--src;
			case 1: *--dest = *--src;
				}
				while (--itmp > 0);
		}
	}
	else
	{
		switch (len % 16)
		{
			case 0:
				do
				{
					*dest++ = *src++;
			case 15: *dest++ = *src++;
			case 14: *dest++ = *src++;
			case 13: *dest++ = *src++;
			case 12: *dest++ = *src++;
			case 11: *dest++ = *src++;
			case 10: *dest++ = *src++;
			case 9: *dest++ = *src++;
			case 8: *dest++ = *src++;
			case 7: *dest++ = *src++;
			case 6: *dest++ = *src++;
			case 5: *dest++ = *src++;
			case 4: *dest++ = *src++;
			case 3: *dest++ = *src++;
			case 2: *dest++ = *src++;
			case 1: *dest++ = *src++;
				}
				while (--itmp > 0);
		}
	}
}							 /* end of mem_cpy */

/*+-------------------------------------------------------------------------
	skip_ld_break(zstr) - skip leading spaces and tabs
--------------------------------------------------------------------------*/
char *
skip_ld_break(zstr)
char *zstr;
{
	while (isspace((uchar) * zstr))
		zstr++;
	return (zstr);
}							 /* end of skip_ld_break */

/*+-------------------------------------------------------------------------
	strip_trail_break(zstr) - strip trailing spaces and tabs
--------------------------------------------------------------------------*/
void
strip_trail_break(zstr)
char *zstr;
{
	int itmp = strlen(zstr);
	char *zptr = zstr + itmp - 1;

	while (itmp && isspace((uchar) * zptr))
	{
		*zptr-- = 0;
		itmp--;
	}
}							 /* end of strip_trail_break */

/*+-----------------------------------------------------------------------
	pad_zstr_to_len(zstr,len)

  pads with spaces to specified length, unless already longer than
  len in which case the string is truncated to 'len' characters.
------------------------------------------------------------------------*/
void
pad_zstr_to_len(zstr, len)
char *zstr;
int len;
{
	int izstr;

	izstr = strlen(zstr);
	if (izstr >= len)
		zstr[len] = 0;
	else
	{
		while (izstr < len)
			zstr[izstr++] = 0x20;
		zstr[izstr] = 0;
	}
}							 /* end of pad_zstr_to_len */

/*+-----------------------------------------------------------------------
	arg_token(parsestr,termchars)

Get next token from string parsestr ((char *)0 on 2nd, 3rd, etc.
calls), where tokens are nonempty strings separated by runs of chars
from termchars.  Writes nulls into parsestr to end tokens.
termchars need not remain constant from call to call.

Treats multiple occurrences of a termchar as one delimiter (does not
allow null fields).
------------------------------------------------------------------------*/
char *
arg_token(parsestr, termchars)
char *parsestr;
char *termchars;
{
	char *parseptr;
	char *token;

	if (!parsestr && !str_token_static)
		return ((char *)0);

	if (parsestr)
	{
		str_token_static = (char *)0;
		parseptr = parsestr;
	}
	else
		parseptr = str_token_static;

	while (*parseptr)
	{
		if (!strchr(termchars, *parseptr))
			break;
		parseptr++;
	}

	if (!*parseptr)
	{
		str_token_static = (char *)0;
		return ((char *)0);
	}

	token = parseptr;

	/*
	 * tokens beginning with apostrophe or quotes kept together
	 */
	if (*token == '\'')
	{
		token++;
		parseptr++;
		while (*parseptr)
		{
			if (*parseptr == '\'')
			{
				str_token_static = parseptr + 1;
				*parseptr = 0;
				return (token);
			}
			parseptr++;
		}
		str_token_static = (char *)0;
		return (token);
	}
	else if (*token == '"')
	{
		token++;
		parseptr++;
		while (*parseptr)
		{
			if (*parseptr == '"')
			{
				str_token_static = parseptr + 1;
				*parseptr = 0;
				return (token);
			}
			parseptr++;
		}
		str_token_static = (char *)0;
		return (token);
	}

	while (*parseptr)
	{
		if (strchr(termchars, *parseptr))
		{
			*parseptr = 0;
			str_token_static = parseptr + 1;
			while (*str_token_static)
			{
				if (!strchr(termchars, *str_token_static))
					break;
				str_token_static++;
			}
			return (token);
		}
		parseptr++;
	}
	str_token_static = (char *)0;
	return (token);
}							 /* end of arg_token */

/*+-------------------------------------------------------------------------
	build_arg_array(cmd,arg,arg_max_quan,&narg)
--------------------------------------------------------------------------*/
void
build_arg_array(cmd, arg, arg_max_quan, narg_rtn)
char *cmd;
char **arg;
int arg_max_quan;
int *narg_rtn;
{
	int narg;

	str_token_static = (char *)0;
	memset((char *)arg, 0, sizeof(char *) * arg_max_quan);

	if (!(arg[0] = arg_token(cmd, " \t\r\n")))
	{
		*narg_rtn = 0;
		return;
	}

	for (narg = 1; narg < arg_max_quan; ++narg)
	{
		if (!(arg[narg] = arg_token((char *)0, " \t\r\n")))
			break;
	}

	*narg_rtn = narg;

}							 /* end of build_arg_array */

/*+-----------------------------------------------------------------------
	str_token(parsestr,termchars)

Get next token from string parsestr ((char *)0 on 2nd, 3rd, etc.
calls), where tokens are nonempty strings separated by runs of chars
from termchars.  Writes nulls into parsestr to end tokens.
termchars need not remain constant from call to call.

Treats each occurrence of a termchar as delimiter (allows null
fields).
------------------------------------------------------------------------*/
char *
str_token(parsestr, termchars)
char *parsestr;
char *termchars;
{
	char *termptr;
	char *parseptr;
	char *token;

	if (!parsestr && !str_token_static)
		return ((char *)0);

	if (parsestr)
	{
		str_token_static = (char *)0;
		parseptr = parsestr;
	}
	else
		parseptr = str_token_static;

	while (*parseptr)
	{
		for (termptr = termchars; *termptr != 0; termptr++)
		{
			if (*parseptr == *termptr)
				goto FOUND_TERM;
		}
		if (!*termptr)
			break;
		parseptr++;
	}

	if (!*parseptr)
	{
		str_token_static = (char *)0;
		return ((char *)0);
	}

  FOUND_TERM:
	token = parseptr;
	while (*parseptr)
	{
		for (termptr = termchars; *termptr;)
		{
			if (*parseptr == *termptr++)
			{
				str_token_static = parseptr + 1;
				*parseptr = 0;
				return (token);
			}
		}
		parseptr++;
	}
	str_token_static = (char *)0;
	return (token);
}							 /* end of str_token */

/*+-------------------------------------------------------------------------
	build_str_array(str,arg,arg_max_quan,&narg)
--------------------------------------------------------------------------*/
void
build_str_array(str, arg, arg_max_quan, narg_rtn)
char *str;
char **arg;
int arg_max_quan;
int *narg_rtn;
{
	int narg;

	str_token_static = (char *)0;
	memset((char *)arg, 0, sizeof(char *) * arg_max_quan);

	if (!(arg[0] = str_token(str, " \t\r\n")))
	{
		*narg_rtn = 0;
		return;
	}

	for (narg = 1; narg < arg_max_quan; ++narg)
	{
		if (!(arg[narg] = str_token((char *)0, " \t\r\n")))
			break;
	}

	*narg_rtn = narg;

}							 /* end of build_str_array */

/*+-----------------------------------------------------------------------
	graphic_char_text(character,incl_3char) - Make all chars "printable"

  returns pointer to a static string containing printable version
  of a character.  If control char, printed as "^A", etc.
  if incl_3char set true, then space + ASCII assignment (e.g. "NUL") is
  appended to the string for non-printable graphics
------------------------------------------------------------------------*/
char *
graphic_char_text(ch, incl_3char)
uchar ch;
int incl_3char;
{
	static char gg[16];

	ch &= 0x7F;
	if (isprint(ch))
	{
		gg[0] = ch;
		gg[1] = 0;
	}
	else
	{
		gg[0] = '^';
		if (ch == 0x7F)
		{
			gg[1] = '?';
			if (incl_3char)
				strcpy(&gg[2], " DEL");
			else
				gg[2] = 0;
		}
		else
		{
			gg[1] = ch + 0x40;
			if (incl_3char)
			{
				gg[2] = 0x20;
				strcpy(gg + 3, ascii_ctlstr[ch]);
				gg[7] = 0;
			}
			else
				gg[2] = 0;
		}
	}
	return (gg);
}							 /* end of graphic_char_text */

/*+-----------------------------------------------------------------------
	mode_map(mode,mode_str)	build drwxrwxrwx string
------------------------------------------------------------------------*/
char *
mode_map(mode, mode_str)
unsigned short mode;
char *mode_str;
{
	unsigned ftype = mode & S_IFMT;
	char *rtn;
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
			if (mode & S_INSEM)	/* semaphore */
			{
				*rtn = 's';
				break;
			}
#endif
#if defined(S_INSHD)
			if (mode & S_INSHD)	/* shared memory */
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

	if (mode & 000400)
		*(rtn + 1) = 'r';
	if (mode & 000200)
		*(rtn + 2) = 'w';
	if (mode & 000100)
		*(rtn + 3) = 'x';
	if (mode & 004000)
		*(rtn + 3) = 's';
	if (mode & 000040)
		*(rtn + 4) = 'r';
	if (mode & 000020)
		*(rtn + 5) = 'w';
	if (mode & 000010)
		*(rtn + 6) = 'x';
	if (mode & 002000)
		*(rtn + 6) = 's';
	if (mode & 000004)
		*(rtn + 7) = 'r';
	if (mode & 000002)
		*(rtn + 8) = 'w';
	if (mode & 000001)
		*(rtn + 9) = 'x';
	if (mode & 001000)
		*(rtn + 9) = 't';

	return (rtn);

}							 /* end of mode_map */

/*+-----------------------------------------------------------------------
	disp_termio(ttt,text) - display termio 'ttt' on console/plog
------------------------------------------------------------------------*/
void
disp_termio(ttt, text)
struct TERMIO *ttt;
char *text;
{
	int flag;
	int i_cc;
	char *bitrate;
	char dbits;
	char parity;

	pprintf("---------> %s\n", text);

#if defined(CFG_TermiosLineio)
	flag = ttt->c_iflag;
	pprintf("iflag: %07o IGNBRK:%d BRKINT:%d IGNPAR:%d ",
		flag,
		(flag & IGNBRK) ? 1 : 0,
		(flag & BRKINT) ? 1 : 0,
		(flag & IGNPAR) ? 1 : 0);
	pprintf("PARENB:%d PARMRK:%d INPCK:%d ISTRIP:%d\n",
		(flag & PARENB) ? 1 : 0,
		(flag & PARMRK) ? 1 : 0,
		(flag & INPCK) ? 1 : 0,
		(flag & ISTRIP) ? 1 : 0);
	pprintf("               INLCR:%d  IGNCR:%d  ICRNL:%d  ",
		(flag & INLCR) ? 1 : 0,
		(flag & IGNCR) ? 1 : 0,
		(flag & ICRNL) ? 1 : 0);
	pprintf("IXON:%d  IXANY:%d  IXOFF:%d\n",
		(flag & IXON) ? 1 : 0,
		(flag & IXANY) ? 1 : 0,
		(flag & IXOFF) ? 1 : 0);

	flag = ttt->c_oflag;
	pprintf("oflag: %07o OPOST:%d  ONLCR:%d\n",
		flag,
		(flag & OPOST) ? 1 : 0,
		(flag & ONLCR) ? 1 : 0);

	flag = ttt->c_lflag;
	pprintf("lflag: %07o ISIG:%d   ICANON:%d  ECHO:%d   ECHOE:%d\n",
		flag,
		(flag & ISIG) ? 1 : 0,
		(flag & ICANON) ? 1 : 0,
		(flag & ECHO) ? 1 : 0,
		(flag & ECHOE) ? 1 : 0);
	pprintf("               ECHOK:%d  ECHONL:%d NOFLSH:%d",
		(flag & ECHOK) ? 1 : 0,
		(flag & ECHONL) ? 1 : 0,
		(flag & NOFLSH) ? 1 : 0);
#else
	flag = ttt->c_iflag;
	pprintf("iflag: %07o IGNBRK:%d BRKINT:%d IGNPAR:%d ",
		flag,
		(flag & IGNBRK) ? 1 : 0,
		(flag & BRKINT) ? 1 : 0,
		(flag & IGNPAR) ? 1 : 0);
	pprintf("PARENB:%d PARMRK:%d INPCK:%d ISTRIP:%d\n",
		(flag & PARENB) ? 1 : 0,
		(flag & PARMRK) ? 1 : 0,
		(flag & INPCK) ? 1 : 0,
		(flag & ISTRIP) ? 1 : 0);
	pprintf("               INLCR:%d  IGNCR:%d  ICRNL:%d  IUCLC:%d  ",
		(flag & INLCR) ? 1 : 0,
		(flag & IGNCR) ? 1 : 0,
		(flag & ICRNL) ? 1 : 0,
		(flag & IUCLC) ? 1 : 0);
	pprintf("IXON:%d  IXANY:%d  IXOFF:%d\n",
		(flag & IXON) ? 1 : 0,
		(flag & IXANY) ? 1 : 0,
		(flag & IXOFF) ? 1 : 0);

	flag = ttt->c_oflag;
	pprintf("oflag: %07o OPOST:%d  OLCUC:%d  ONLCR:%d  OCRNL:%d  ",
		flag,
		(flag & OPOST) ? 1 : 0,
		(flag & OLCUC) ? 1 : 0,
		(flag & ONLCR) ? 1 : 0,
		(flag & OCRNL) ? 1 : 0);
	pprintf("ONOCR:%d ONLRET:%d OFDEL:%d\n",
		(flag & ONOCR) ? 1 : 0,
		(flag & ONLRET) ? 1 : 0,
		(flag & OFDEL) ? 1 : 0);

	flag = ttt->c_lflag;
	pprintf("lflag: %07o ISIG:%d   ICANON:%d XCASE:%d  ECHO:%d   ECHOE:%d\n",
		flag,
		(flag & ISIG) ? 1 : 0,
		(flag & ICANON) ? 1 : 0,
		(flag & XCASE) ? 1 : 0,
		(flag & ECHO) ? 1 : 0,
		(flag & ECHOE) ? 1 : 0);
	pprintf("               ECHOK:%d  ECHONL:%d NOFLSH:%d",
		(flag & ECHOK) ? 1 : 0,
		(flag & ECHONL) ? 1 : 0,
		(flag & NOFLSH) ? 1 : 0);
#if defined(XCLUDE)
	pprintf(" XCLUDE:%d", (flag & XCLUDE) ? 1 : 0);
#endif
#endif

	pprintf("\n");

	flag = ttt->c_cflag;
	pprintf("cflag: %07o ", ttt->c_cflag);
	switch (ecugetspeed(ttt))
	{
		case B0:
			bitrate = "HUP";
			break;
		case B50:
			bitrate = "50";
			break;
		case B75:
			bitrate = "75";
			break;
		case B110:
			bitrate = "110";
			break;
		case B134:
			bitrate = "134.5";
			break;
		case B150:
			bitrate = "150";
			break;
		case B200:
			bitrate = "200";
			break;
		case B300:
			bitrate = "300";
			break;
		case B600:
			bitrate = "600";
			break;
		case B1200:
			bitrate = "1200";
			break;
		case B1800:
			bitrate = "1800";
			break;
		case B2400:
			bitrate = "2400";
			break;
		case B4800:
			bitrate = "4800";
			break;
		case B9600:
			bitrate = "9600";
			break;
#if defined(B19200)
		case B19200:
			bitrate = "19200";
			break;
#endif
#if defined(B38400)
		case B38400:
			bitrate = "38400";
			break;
#endif
#if defined(B57600)
		case B57600:
			bitrate = "57600";
			break;
#endif
#if defined(B115200)
		case B115200:
			bitrate = "115200";
			break;
#endif
		default:
			switch (ecugetspeed(ttt))
			{
				case EXTA:
					bitrate = "EXTA";
					break;
				case EXTB:
					bitrate = "EXTB";
					break;
				default:
					bitrate = "????";
					break;
			}
	}
#ifndef CSIZE
#define CSIZE CS8
#endif
	switch (flag & CSIZE)
	{
		case CS8:
			dbits = '8';
			break;
		case CS7:
			dbits = '7';
			break;
		case CS6:
			dbits = '6';
			break;
		case CS5:
			dbits = '5';
			break;
		default:
			dbits = '?';
			break;
	}
	parity = (flag & PARENB) ? ((flag & PARODD) ? 'O' : 'E') : 'N';
	pprintf("%s-%c-%c-%d ", bitrate, dbits, parity, (flag & CSTOPB) ? 2 : 1);
	pprintf("CREAD:%d  HUPCL:%d  CLOCAL:%d",
		(flag & CREAD) ? 1 : 0,
		(flag & HUPCL) ? 1 : 0,
		(flag & CLOCAL) ? 1 : 0);
#if defined(RTSFLOW)		 /* SCO */
	pprintf(" RTSFLOW:%d CTSFLOW:%d",
		(flag & RTSFLOW) ? 1 : 0,
		(flag & CTSFLOW) ? 1 : 0);
#endif
#if defined(CRTSFL)			 /* SCO 3.2v4 */
	pprintf("\n               CRTSFL:%d",
		(flag & CRTSFL) ? 1 : 0);
#endif
#if defined(RTSXOFF)		 /* SVR4 */
	pprintf(" RTSXOFF:%d  CTSXON:%d",
		(hx_flag & RTSXOFF) ? 1 : 0,
		(hx_flag & CTSXON) ? 1 : 0);
#endif
#ifdef CRTSCTS				 /* sun */
	pprintf(" CRTSCTS:%d", (flag & CRTSCTS) ? 1 : 0);
#endif
	pputs("\n");

#ifdef __FreeBSD__
	pputs("ctls: ");
	for (i_cc = 0; i_cc < NCC; i_cc++)
	{
		if (i_cc == 7 || i_cc == 19)
			continue;
		pprintf("%02x  ", ttt->c_cc[i_cc]);
	}
	pputs("\n");
	pputs("(hex) EOF EOL EL2 ERS WRS KIL ");
	pputs("RPR INT QUI SSP DSP STA STP LNX DIS MIN TIM STA\n");
#else
	pprintf("ctl chars: ");
	for (i_cc = 0; i_cc < ((NCC > 8) ? 8 : NCC); i_cc++)
		pprintf("%02x   ", ttt->c_cc[i_cc]);
	pputs("  (hex)\n");
	pputs("           INTR QUIT ERAS KILL EOF  EOL  ");
	pputs("EOL2 SWTCH  VMIN-EOF VTIME-EOL\n");
#endif
}							 /* end of disp_termio */

/*+-------------------------------------------------------------------------
	disp_stat(st)
--------------------------------------------------------------------------*/
void
disp_stat(st)
struct stat *st;
{
	char mdmap[32];
	struct passwd *pw;

	if (pw = getpwuid(st->st_uid))
		pprintf("owner: %s ", pw->pw_name);
	else
		pprintf("uid: %d ", st->st_uid);
	endpwent();
	mode_map(st->st_mode, mdmap);
	pprintf("mode: %s ", mdmap);
	pprintf("inode: %5u  dev: %3u rdev: %u,%u (0x%04x)\n",
		(UINT) st->st_ino, (UINT) st->st_dev,
		(UINT16) st->st_rdev >> 8, (UINT16) st->st_rdev & 0xFF,
		(UINT16) st->st_rdev);

}							 /* end of disp_stat */

/*+-----------------------------------------------------------------------
	disp_line_termio(fd)

Get current termio structure for file descriptor fd
and display on stderr
------------------------------------------------------------------------*/
void
disp_line_termio(fd, text)
int fd;						 /* file descriptor */
char *text;
{
	struct TERMIO fd_termio;
	struct stat fd_stat;
	char text2[128];

	ecugetattr(fd, &fd_termio);
	sprintf(text2, "fd: %d  %s", fd, text);
	disp_termio(&fd_termio, text2);
	fstat(fd, &fd_stat);
	disp_stat(&fd_stat);

}							 /* end of disp_line_termio */

/*+-----------------------------------------------------------------------
	ascii_name_to_hex(name)

  return value of ascii ctl char name (e.g., "NUL") 0 - 0x1F
  support CSI "ascii" 0x9B (for ESC + '[' ANSI)
  returns -1 if input not valid
------------------------------------------------------------------------*/
ascii_name_to_hex(name)
char *name;
{
	char **cpptr;
	int itmp;
	char s4[4];

	if (!strcmpi(name, "del"))
		return (0x7F);
	else if (!strcmpi(name, "csi"))
		return (0x9B);
	else if ((itmp = strlen(name)) < 3)
	{
		if (!itmp)
			return (-1);
		memcpy(s4, "   ", 4);
		memcpy(s4, name, itmp);
		name = s4;
	}

	cpptr = ascii_ctlstr;
	for (itmp = 0; itmp <= SPACE; itmp++)
	{
		if (!strcmpi(name, *cpptr))
			return (itmp);
		cpptr++;
	}

	return (-1);

}							 /* end of ascii_name_to_hex */

/*+-------------------------------------------------------------------------
	ascii_to_hex(ascii)
--------------------------------------------------------------------------*/
int
ascii_to_hex(ascii)
char *ascii;
{
	int hexval;

	if (strlen(ascii) == 1)
		return (*ascii);
	else if (!strncmp(ascii, "0x", 2))
	{
		sscanf(ascii + 2, "%x", &hexval);
		return (hexval & 0xFF);
	}
	else if (*ascii == '^')
		return (*(ascii + 1) & 0x1F);
	else
		return (ascii_name_to_hex(ascii));
}							 /* end of ascii_to_hex */

/*+-------------------------------------------------------------------------
	hex_to_ascii_name(char_val)

  Returns pointer to static string containing three character ASCII
  name for control character followed by a null.
--------------------------------------------------------------------------*/
char *
hex_to_ascii_name(char_val)
char char_val;
{
	static char ascii_name[4];

	char_val &= 0x7F;

	if (char_val == 0x7F)
		strcpy(ascii_name, "DEL");
	else if (char_val <= SPACE)
		strcpy(ascii_name, ascii_ctlstr[char_val]);
	else
	{
		ascii_name[0] = char_val;
		ascii_name[1] = 0;
	}

	return (ascii_name);

}							 /* end of hex_to_ascii_name */

/*+-------------------------------------------------------------------------
	get_curr_dir(cdir,cdir_max) - get current directory into 'cdir'

  getcwd()/getwd() not available everywhere nor consistent;
  get_curr_dir() has the same shape as getcwd(), but the toil of
  keeping up with who supports what is not worth the hassle since
  ECU does not use this function very much
--------------------------------------------------------------------------*/
int
get_curr_dir(cdir, cdir_max)
char *cdir;
int cdir_max;
{
#if defined(linux)
	char *temp;
	char parm[ECU_MAXPN];
	char *getcwd();

	strcpy(parm, ".");
	temp = getcwd(parm,sizeof(parm));
	if (!temp)
		return (-1);
	strncpy(cdir, temp, cdir_max);
	cdir[cdir_max] = 0;
	return (0);
#else
#if defined(__FreeBSD__)
	char *temp;
	char parm[ECU_MAXPN];
	char *getwd();

	strcpy(parm, ".");
	temp = getwd(parm);
	if (!temp)
		return (-1);
	strncpy(cdir, temp, cdir_max);
	cdir[cdir_max] = 0;
	return (0);
#else
	FILE *popen();
	FILE *pipefp = popen("/bin/pwd", "r");
	int itmp;

	strcpy(cdir, ".");
	if (!pipefp)
		return (-1);
	fgets(cdir, cdir_max, pipefp);
	cdir[cdir_max - 1] = 0;
	if ((itmp = strlen(cdir)) && (*(cdir + itmp - 1) == 0x0A))
		*(cdir + itmp - 1) = 0;
	fclose(pipefp);
	return (0);
#endif
#endif
}							 /* end of get_curr_dir */

/*+-----------------------------------------------------------------------
	get_home_dir(home_dir) - get user home directory
------------------------------------------------------------------------*/
void
get_home_dir(home_dir)
char *home_dir;
{
	static char *home_directory = 0;
	struct passwd *pw;
	char *cp;

	*home_dir = 0;
	if (home_directory)
	{
		strcpy(home_dir, home_directory);
		return;
	}

	if (cp = getenv("HOME"))
	{
		if (home_directory = malloc(strlen(cp) + 1))
			strcpy(home_directory, cp);
		strcpy(home_dir, cp);
		return;
	}

	if (pw = getpwuid(getuid()))
	{
		endpwent();
		if (home_directory = malloc(strlen(pw->pw_dir) + 1))
			strcpy(home_directory, pw->pw_dir);
		strcpy(home_dir, pw->pw_dir);
		return;
	}
	endpwent();

	pputs("Cannot find your home directory in\n");
	pperror("$HOME or /etc/passwd");
	termecu(TERMECU_PWENT_ERROR);

}							 /* end of get_home_dir */

/*+-------------------------------------------------------------------------
	make_ecu_subdir()
must be called early in execution before wierd tty states set, etc.
--------------------------------------------------------------------------*/
void
make_ecu_subdir()
{
	int itmp = 0;
	struct stat fst;
	char subdir_pathn[ECU_MAXPN];

	get_home_dir(subdir_pathn);
	strcat(subdir_pathn, "/.ecu");
	itmp = stat(subdir_pathn, &fst);

	if (!itmp && ((fst.st_mode & S_IFMT) != S_IFDIR))
	{
		ff(se, "~/.ecu is not a directory. Rename the file and try again.\n\n");
		exit(1);
	}

	if (itmp)				 /* if stat failed, try to make the directory */
	{
		char pseudo_file_in_subdir[ECU_MAXPN + 2];

		strcpy(pseudo_file_in_subdir, subdir_pathn);
		strcat(pseudo_file_in_subdir, "/x");
		errno = ENOENT;
		if (make_dirs(pseudo_file_in_subdir))
		{
			if (stat(subdir_pathn, &fst))
			{
				pputs("cannot make ~/.ecu subdirectory.\n");
				pperror(subdir_pathn);
				exit(1);
			}
		}
	}

	/*
	 * do this on every call whether we make the dir this time or not
	 */
	chmod(subdir_pathn, 0700);
}							 /* end of make_ecu_subdir */

/*+-------------------------------------------------------------------------
	mkdir_auto(dirfn)

  returns -1 if error,
      else # levels required to get target

--------------------------------------------------------------------------*/
int
mkdir_auto(dirfn)
char *dirfn;
{

	int itmp = strlen(dirfn);
	char *buf;

	if (!(itmp = mkdir(dirfn, 0755)))
		return (1);			 /* one level */

	if (itmp != ENOENT)
		return (-1);

	errno = 0;
	if (!(buf = malloc(itmp + 4)))
	{
		pputs("mkdir_auto malloc failed\n");
		termecu(TERMECU_MALLOC);
	}

	strcpy(buf, dirfn);
	strcat(buf + itmp, "/a");/* make_dirs() wants dummy fn */

	errno = ENOENT;
	itmp = make_dirs(buf, 0755);
	free(buf);

	return (itmp);

}							 /* end of mkdir_auto */

/*+-------------------------------------------------------------------------
	str_classify(sc,str) - classify a string and return value

Use the STR_CLASSIFY structure to classify a string (convert str to
lexical token or error code) and return the value;  use last token
in table if no string matches
--------------------------------------------------------------------------*/
str_classify(sc, str)
STR_CLASSIFY *sc;
char *str;
{
	while (sc->str)
	{
		if (minunique(sc->str, str, sc->min_ch))
			return (sc->token);
		sc++;
	}
	return (sc->token);
}							 /* end of str_classify */

/*+-------------------------------------------------------------------------
	yes_or_no(strarg) - lenient yes/no, on/off

  Returns 1 if first char is 'Y' or 'y'
	or if strarg is numeric returns the numeric value
	or if strarg is alpha == "on" returns 1
  Returns 0 otherwise
--------------------------------------------------------------------------*/
int
yes_or_no(strarg)
char *strarg;
{
	static STR_CLASSIFY sc[] =
	{
		{"yes", 1, 1},
		{"on", 2, 1},
		{"no", 1, 0},
		{"off", 3, 0},
		{(char *)0, 0, 0},
	};

	if (isdigit((uchar) * strarg))
		return (atoi(strarg));
	else
		return (str_classify(sc, strarg));

}							 /* end of yes_or_no */

/*+-------------------------------------------------------------------------
	find_shell_chars(command) - search for shell metacharacters

returns 1 if found

 The characters ";|&><" are more to promote security than to
 aid in filename expansion
--------------------------------------------------------------------------*/
int
find_shell_chars(command)
char *command;
{
	int schar;
	int cchar;
	char *scptr;
	static char shell_chars[] = "\\\"~*?'`{}[];$|&><";

	while (cchar = *command++)
	{
		scptr = shell_chars;
		while (schar = *scptr++)
		{
			if (schar == cchar)
				return (1);
		}
	}
	return (0);
}							 /* end of find_shell_chars */

/*+-------------------------------------------------------------------------
	strerror(err_no) - safe sys_errlist lookup
--------------------------------------------------------------------------*/
#ifndef CFG_HasStrerror
char *
strerror(err_no)
int err_no;
{
	static char errant[32];

	if ((unsigned)err_no <= (unsigned)sys_nerr)
		return (sys_errlist[errno]);
	sprintf(errant, "Error %d", errno);
	return (errant);

}							 /* end of strerror */
#endif

/*+-------------------------------------------------------------------------
	perror_errmsg(str)
--------------------------------------------------------------------------*/
void
perror_errmsg(str)
char *str;
{
	extern char errmsg[];

	sprintf(errmsg, "%s: %s", str, strerror(errno));
}							 /* end of perror_errmsg */

/*+-------------------------------------------------------------------------
	cfree(p,num,size) - fix bug in XENIX -lmalloc
--------------------------------------------------------------------------*/
#if defined(M_XENIX) && defined(XENIX_MALLOC_LIB_BUG)
cfree(p, num, size)
char *p;
int num;
int size;
{
	free(p);
}							 /* end of cfree */
#endif

/*+-------------------------------------------------------------------------
	Rdchk(fd) - for systems without it but with FIONREAD
--------------------------------------------------------------------------*/
#if defined(CFG_FionreadRdchk)
int
Rdchk(fd)
int fd;
{
	int chars_waiting;

	if (ioctl(fd, FIONREAD, &chars_waiting))
		return (0);
	else
		return (!!chars_waiting);
}							 /* end of Rdchk */
#endif

/*+-------------------------------------------------------------------------
	build_valid_baud_string()
--------------------------------------------------------------------------*/
void
build_valid_baud_string()
{
	char s2048[2048];
	static int bauds[] =
	{
		110, 300, 600, 1200, 2400, 4800, 9600, 19200, 38400,
		57600, 115200, 0
	};
	int *b = bauds;

	valid_baud_string = "???";

	s2048[0] = 0;
	while (*b)
	{
		if (valid_baud(*b) != -1)
			sprintf(s2048 + strlen(s2048), ",%u", *b);
		b++;
	}
	valid_baud_string = strdup((s2048[0] == ',') ? s2048 + 1 : s2048);

}							 /* end of build_valid_baud_string */

/*+-------------------------------------------------------------------------
	base_name(fullname) - strip directory name from file_name
--------------------------------------------------------------------------*/
char *
base_name(fullname)
char *fullname;
{
	char *start = strrchr(fullname,'/'); /* find last slash */

	return((start) ? start + 1 : fullname);

}	/* end of base_name */

/*+-------------------------------------------------------------------------
	get_uname_nodename()
--------------------------------------------------------------------------*/
char *
get_uname_nodename()
{
	struct utsname utsn;

#ifdef M_SYSV				 /* SCO before 3.2v5 */

	/*
	 * before UNIX, had to use systemid first; 3.2.0, uname
	 * might have been bad, but systemid ok; in 3.2v5,
	 * /etc/systemid is backward compatibility maintained in
	 * /etc/rc2.d by `uname -n > /etc/systemid'
	 */
	FILE *fp = fopen("/etc/systemid", "r");

	if (fp)
	{
		uname_rtnstring[0] = 0;
		fgets(uname_rtnstring, sizeof(uname_rtnstring), fp);
		fclose(fp);
		if (itmp = strlen(uname_rtnstring))
		{
			if (*(cp = uname_rtnstring + itmp - 1) == '\n')
				*cp = 0, itmp--;
			strip_trail_break(uname_rtnstring);
			return(uname_rtnstring);
		}
	}
#endif

	if (uname(&utsn))
	{
		pperror("uname");
		return(0);
	}
	strncpy(uname_rtnstring,utsn.nodename,sizeof(uname_rtnstring));
	uname_rtnstring[sizeof(uname_rtnstring) - 1] = 0;
	return(uname_rtnstring);

}	/* end of get_uname_nodename */

/*+-------------------------------------------------------------------------
	get_uname_sysname()
--------------------------------------------------------------------------*/
char *
get_uname_sysname()
{
	struct utsname utsn;

	if (uname(&utsn))
	{
		pperror("uname");
		return(0);
	}
	strncpy(uname_rtnstring,utsn.nodename,sizeof(uname_rtnstring));
	uname_rtnstring[sizeof(uname_rtnstring) - 1] = 0;
	return(uname_rtnstring);
}	/* end of get_uname_sysname */


/* end of ecuutil.c */
/* vi: set tabstop=4 shiftwidth=4: */
