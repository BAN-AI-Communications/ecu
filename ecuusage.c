/*+-----------------------------------------------------------------------
	ecuusage.c - user admonishment
	wht@wht.net

  Defined functions:
	fkmap_cmd_usage()
	general_usage(uptr)
	log_cmd_usage()
	usage()

------------------------------------------------------------------------*/
/*+:EDITS:*/
/*:04-26-2000-11:15-wht@bob-RELEASE 4.42 */
/*:01-24-1997-02:37-wht@yuriatin-SOURCE RELEASE 4.00 */
/*:12-19-1996-17:22-wht@yuriatin-fix bug in <hostname> display */
/*:10-24-1996-03:14-wht@yuriatin-document -T switch */
/*:09-16-1996-04:18-wht@yuriatin-update telnet usage */
/*:09-11-1996-20:00-wht@yuriatin-3.48-major telnet,curses,structural overhaul */
/*:12-06-1995-13:30-wht@n4hgf-termecu w/errno -1 consideration */
/*:11-23-1995-11:20-wht@kepler-source control 3.37 for tsx-11 */
/*:11-14-1995-10:23-wht@kepler-3.37.80-source control point: SOCKETS */
/*:11-12-1995-01:13-wht@gyro-add -n, doc add -z */
/*:10-14-1995-16:17-wht@kepler-use valid_baud_string */
/*:01-12-1995-15:19-wht@n4hgf-apply Andrew Chernov 8-bit clean+FreeBSD patch */
/*:05-04-1994-04:39-wht@n4hgf-ECU release 3.30 */
/*:12-04-1992-21:11-wht@n4hgf-add -l=type info */
/*:09-16-1992-13:54-wht@n4hgf-add fkmap usage */
/*:09-10-1992-13:59-wht@n4hgf-ECU release 3.20 */
/*:08-22-1992-15:38-wht@n4hgf-ECU release 3.20 BETA */
/*:04-24-1992-21:07-wht@n4hgf-put defaults back into cmd line usage */
/*:04-24-1992-06:30-wht@n4hgf-rll usage fixes */
/*:07-25-1991-12:57-wht@n4hgf-ECU release 3.10 */
/*:04-27-1991-01:52-wht@n4hgf-overhaul revision numbers */
/*:08-14-1990-20:40-wht@n4hgf-ecu3.00-flush old edit history */

#include "ecu.h"
#if 0
#include <stdio.h>
#include "termecu.h"
#include "ecutermio.h"		 /* for baudrate */
#define pf printf
#endif

extern char *makedate;		 /* temporary make date */
extern char *revstr;		 /* ecunumrev.c */
extern char *valid_baud_string;

char *usage_text[] =
{
	"usage: ecu [-l <ttyname | =type>] [-b <baud>] [-eon]\n",
	"           [-h] [-t] [-P <phonedir>] [-F name] [-T <tracelevel>]\n",
	"           [-N] [-n] [-z] [-T]\n",
	"           [-p <initial_proc> [-d] [-D] <args...> |\n",
	"           <phone_number> | <logical> ",
#ifdef CFG_TelnetOption
	"| <hostname>",
#endif
	"]\n",
	"-F sets an alternate funckeymap name for the *keyboard*\n",
	"-N disable curses displays - use `scrolling tty' display\n",
	"-P choose alternate phone directory (<phonedir> must be a full path)\n",
	"-T set procedure tracing to level: 0=none, 1=standard, 2=ECU-debugging\n",
	"-b bit rate (",
	"VBS",
	")\n",
	"-e even parity   -o odd parity   -n no parity\n",
	"-h half duplex ... default is full duplex\n",
	"-l <ttyname> chooses /dev/<ttyname>\n",
	"-l =<type> chooses line from Devices with matching type (1st field)\n",
	"-n disables the built-in receiver ANSI filter\n",
	"-t append NL to incoming and outgoing CR characters\n",
#ifdef CFG_TelnetOption
	"-z if telnet connection, show options traffic\n",
#endif
	"\n",
	"<phone_number> is an actual number beginning with a numeric digit.\n",
	"<logical> is a dialing directory name.\n",
#ifdef CFG_TelnetOption
	"<hostname> is an internet hostname or IP address. A hostname\n",
	"is distinguished from a <phone_number> or <logical> by virtue of\n",
	"containing a period; a trailing period will be removed.\n",
#endif
	"\n",
	"-p execute an initial procedure\n",
	"-D unconditionally stop execution when -p initial procedure is done\n",
	"-d stop execution if -p initial procedure fails\n",
	"-T enable procedure tracing prior to first procedure execution\n",
	"\n",
	"For interactive option selection, try executing ecu with no arguments\n",
	"\n",
	"For a list of built in commands, type HOME?<ENTER> once program started\n",
	(char *)0				 /* terminated with null pointer */
};

char *log_cmd_usage_text[] =
{
	"Usage: log [-s] [-r] <filename>\n",
	"       log off   turn logging off\n",
	" -s scratch any previous contents of <filename>, else append\n",
	" -r raw log, else drop non-printable characters\n",
	(char *)0				 /* terminated with null pointer */
};

char *fkmap_cmd_usage_text[] =
{
	"usage: fkmap \n",
	"       fkmap <keyname>\n",
	"       fkmap <keyname> <keylist>\n",
	"       fkmap -l <name>\n",
	"       fkmap -r\n",
	"       fkmap -s <file>\n",
	(char *)0				 /* terminated with null pointer */
};

/*+-----------------------------------------------------------------------
	general_usage(uptr)
------------------------------------------------------------------------*/
void
general_usage(uptr)
char **uptr;
{
	while (*uptr != (char *)0)
		pputs(*(uptr++));
}							 /* end of usage */

/*+-----------------------------------------------------------------------
	usage()

  special case: "VBS" replaced with dynamic-built bit rate string
------------------------------------------------------------------------*/
void
usage()
{
	char **cpp;

	fprintf(stderr, "ecu %s made: %s\n", revstr, makedate);
	cpp = usage_text;
	while (*cpp)
	{
		if (!strcmp(*cpp, "VBS"))
		{
			*cpp = valid_baud_string;
			break;
		}
		cpp++;
	}
	general_usage(usage_text);
	pprintf("\nDefaults: tty=%s baud=%d parity=%c\n",
		CFG_DefaultTty, CFG_DefaultBitRate, CFG_DefaultParity);
	errno = -1;
	termecu(TERMECU_USAGE);
	/* NOTREACHED */
}

/*+-------------------------------------------------------------------------
	log_cmd_usage()
--------------------------------------------------------------------------*/
void
log_cmd_usage()
{
	general_usage(log_cmd_usage_text);
}							 /* end of log_cmd_usage */

/*+-------------------------------------------------------------------------
	fkmap_cmd_usage()
--------------------------------------------------------------------------*/
void
fkmap_cmd_usage()
{
	general_usage(fkmap_cmd_usage_text);
}							 /* end of fkmap_cmd_usage */

/* vi: set tabstop=4 shiftwidth=4: */
