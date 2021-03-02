/* #define ALPHA */
/* #define BETA */

char *numeric_revision = "4";

#if defined(WHT) || defined(ALPHA) || defined(BETA)
char *numeric_devrev = "32";

#else
char *numeric_devrev = "";

#endif

/*+-----------------------------------------------------------------------
	ecunumrev.c - revision numbers
	wht@wht.net

  Defined functions:
	build_revision_string()

------------------------------------------------------------------------*/
/*+:EDITS:*/
/*:04-26-2000-11:15-wht@bob-RELEASE 4.42 */
/*:04-05-1998-16:54-wht@kepler-4.21.32-flush tty at servewire boj */
/*:03-25-1998-20:16-wht@kepler-4.21.30-get one out for Bob Balin */
/*:02-26-1998-18:18-wht@menlo-4.20.25-back to the original world */
/*:02-26-1998-01:45-wht@menlo-4.20.24-linux2/redhat*/
/*:02-02-1998-17:37-wht@kepler-4.14.23 */
/*:12-22-1997-15:54-wht@kepler-4.13.22-to SCO skunkworks */
/*:12-21-1997-13:53-wht@sidonia-4.13.21-more portability */
/*:12-18-1997-06:06-wht@sidonia-4.12.18-port to aix */
/*:12-15-1997-20:46-wht@fep-4.11.17-OS7+enhance portability */
/*:12-08-1997-17:08-wht@kepler-4.10.16-releaseable again */
/*:11-03-1997-02:13-wht@kepler-4.08a-identified as 4.08.10 */
/*:06-06-1997-15:26-wht@gyro-4.07.06-open new rev for routine maintenance */
/*:03-21-1997-01:58-wht@varykino-4.05 */
/*:03-03-1997-16:00-wht@yuriatin-4.04 */
/*:02-25-1997-14:40-wht@yuriatin-4.03 */
/*:02-04-1997-15:31-wht@yuriatin-4.02 */
/*:01-24-1997-02:37-wht@yuriatin-SOURCE RELEASE 4.00 */
/*:11-04-1996-19:46-wht@yuriatin-BETA-3.56.41T */
/*:11-04-1996-03:47-wht@yuriatin-BETA-3.56.40T */
/*:11-01-1996-18:24-wht@yuriatin-BETA-3.56.39T */
/*:10-31-1996-17:49-wht@yuriatin-BETA-3.55.38T */
/*:10-28-1996-14:05-wht@yuriatin-BETA-3.55.37T */
/*:10-25-1996-17:28-wht@yuriatin-BETA-3.55.36T */
/*:10-20-1996-19:49-wht@yuriatin-BETA-3.54.35T */
/*:10-18-1996-21:14-wht@yuriatin-BETA-3.54.34T */
/*:10-18-1996-01:28-wht@yuriatin-BETA-3.54.33T */
/*:10-16-1996-20:25-wht@yuriatin-BETA-3.54.32T */
/*:10-16-1996-04:00-wht@yuriatin-BETA-3.54.31T */
/*:10-16-1996-02:12-wht@yuriatin-BETA-3.54.30T */
/*:10-15-1996-20:12-wht@yuriatin-BETA-3.54.29T */
/*:10-09-1996-20:45-wht@yuriatin-BETA-3.54.28T */
/*:09-24-1996-06:19-wht@yuriatin-BETA-3.53.27T */
/*:09-19-1996-02:29-wht@yuriatin-BETA 3.52.25 */
/*:09-18-1996-17:21-wht@yuriatin-BETA-3.51.24 */
/*:09-17-1996-05:09-wht@yuriatin-tag CFG_TelnetOption use */
/*:09-13-1996-17:50-wht@kepler-3.49-cleanup 3.48 details */
/*:09-11-1996-20:00-wht@yuriatin-3.48-major telnet,curses,structural overhaul */
/*:09-08-1996-15:15-wht@kepler-3.48-radicial rewrite/simplify/update/harden */
/*:07-31-1996-17:19-wht@kepler-3-43-3.44 rapid fire Solaris/FreeBSD fixes */
/*:06-13-1996-13:25-wht@kepler-3.42-fix GCC/CC discrep in config */
/*:02-28-1996-21:31-wht@kepler-3.39.01-fix ">" shell cmd to use /bin/sh */
/*:01-27-1996-21:28-wht@kepler-bump for primary release - not BETA */
/*:12-12-1995-14:22-wht@kepler-3.38.05-notify [no line attached] on keystroke */
/*:12-10-1995-09:55-wht@kepler-3.38.04-tty1A is default on SCO */
/*:12-09-1995-12:00-wht@kepler-3.38.03-fix stdin /dev/null operation */
/*:12-03-1995-19:19-wht@kepler-3.38.02-work on SunOS version */
/*:11-24-1995-14:18-wht@kepler-3.37.90-zmodem xfer fixed */
/*:11-23-1995-11:20-wht@kepler-source control 3.37 for tsx-11 */
/*:11-20-1995-12:24-wht@n4hgf-3.37.82-misc cleanup for SCO 32v4 */
/*:11-14-1995-17:44-wht@kepler-3.37.81-more cleanup */
/*:11-14-1995-10:23-wht@kepler-3.37.80-source control point: SOCKETS */
/*:11-04-1995-15:10-wht@wwtp1-3.37.78-working MUCH better */
/*:11-03-1995-18:05-wht@kepler-3.37.77-clean up OS5 port */
/*:11-03-1995-16:08-wht@wwtp1-3.37.76-refine port to OS5 */
/*:10-19-1995-14:41-wht@kepler-3.37.75-telnet almost working */
/*:10-15-1995-15:22-wht@calvin-3.37.74-SVR4 and SunOS cleanup */
/*:10-14-1995-14:38-wht@kepler-3.37.73-baud values extension */
/*:10-09-1995-16:42-wht@kepler-3.37-72-fix SU */
/*:09-16-1995-17:01-wht@kepler-3.37.71-tcap execution debug/investigation */
/*:09-04-1995-19:03-wht@n4hgf-3.37.70-fix 32v5 lock file naming */
/*:09-01-1995-17:37-wht@n4hgf-3.37.69-32v5 lock file naming */
/*:06-05-1995-02:23-wht@n4hgf-moving toward Everest and ECU 3.40 */
/*:01-15-1995-01:44-wht@n4hgf-clean up port rot */
/*:01-12-1995-15:19-wht@n4hgf-apply Andrew Chernov 8-bit clean+FreeBSD patch */
/*:05-04-1994-04:38-wht@n4hgf-ECU release 3.30 */
/*:12-12-1993-13:28-wht@n4hgf-support MOTSVR3 */
/*:11-14-1993-12:33-wht@n4hgf-HP-UX port by Carl Wuebker at HP */
/*:09-16-1992-14:13-wht@n4hgf-add M and F version qualifiers */
/*:09-10-1992-13:58-wht@n4hgf-ECU release 3.20 */
/*:08-22-1992-15:38-wht@n4hgf-ECU release 3.20 BETA */
/*:11-16-1991-17:05-wht@n4hgf-better development "x" rev numbering */
/*:08-28-1991-14:07-wht@n4hgf2-SVR4 cleanup by aega84!lh */
/*:08-25-1991-14:39-wht@n4hgf-SVR4 port thanks to aega84!lh */
/*:07-25-1991-12:56-wht@n4hgf-ECU release 3.10 */
/*:07-12-1991-14:14-wht@n4hgf-GCC140 differentiation */
/*:04-27-1991-01:52-wht@n4hgf-overhaul revision numbers */
/*:08-14-1990-20:40-wht@n4hgf-ecu3.00-flush old edit history */

#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include "ecu_config.h"
#include "patchlevel.h"

#if defined(M_UNIX)
#undef M_XENIX
#endif

char *revstr = "";

/*+-------------------------------------------------------------------------
	build_revision_string()
--------------------------------------------------------------------------*/
void
build_revision_string()
{
	int itmp;
	char s128[128];

	sprintf(s128, "%s%s.%02d", (itmp = strlen(numeric_devrev)) ?
#ifdef ALPHA
		"ALPHA-"
#else
#ifdef BETA
		"BETA-"
#else
		"x"
#endif
#endif
		: "",
		numeric_revision, PATCHLEVEL);

	if (itmp)
	{
		strcat(s128, ".");
		strcat(s128, numeric_devrev);
	}

#ifdef CFG_TelnetOption
	strcat(s128, "T");
#endif

#ifdef WHT
	strcat(s128, "*");
#endif

	if (!(revstr = malloc(strlen(s128) + 1)))
	{
		fprintf(stderr, "out of memory so early!?\n");
		exit(255);
	}
	strcpy(revstr, s128);

}							 /* end of build_revision_string */

/* vi: set tabstop=4 shiftwidth=4: */
