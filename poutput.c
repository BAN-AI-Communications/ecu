/*+-------------------------------------------------------------------------
	poutput.c - ecu procedure output routines
	wht@wht.net

  Defined functions:
	pcmd_plog(param)
	pflush()
	plog_control(fname)
	plogc(ch)
	plogs(str)
	poutput_init()
	pperror(str)
	pputc(ch)
	pputs(str)

--------------------------------------------------------------------------*/
/*+:EDITS:*/
/*:04-26-2000-11:16-wht@bob-RELEASE 4.42 */
/*:01-24-1997-02:38-wht@yuriatin-SOURCE RELEASE 4.00 */
/*:09-11-1996-20:01-wht@yuriatin-3.48-major telnet,curses,structural overhaul */
/*:12-06-1995-13:30-wht@n4hgf-termecu w/errno -1 consideration */
/*:11-23-1995-11:20-wht@kepler-source control 3.37 for tsx-11 */
/*:11-14-1995-10:23-wht@kepler-3.37.80-source control point: SOCKETS */
/*:05-04-1994-04:39-wht@n4hgf-ECU release 3.30 */
/*:09-10-1992-14:00-wht@n4hgf-ECU release 3.20 */
/*:08-22-1992-15:39-wht@n4hgf-ECU release 3.20 BETA */
/*:07-25-1991-12:59-wht@n4hgf-ECU release 3.10 */
/*:08-14-1990-20:40-wht@n4hgf-ecu3.00-flush old edit history */

#include "ecu.h"
#include "ecukey.h"
#include "ecuerror.h"
#include "esd.h"

extern FILE *rcvr_log_fp;

FILE *plog_fp = (FILE *) 0;
ESD *plog_name = (ESD *) 0;

/*+-------------------------------------------------------------------------
	plogs(str)
--------------------------------------------------------------------------*/
void
plogs(str)
char *str;
{
	if (plog_fp)
		fputs(str, plog_fp);
}							 /* end of plogs */

/*+-------------------------------------------------------------------------
	plogc(ch)
--------------------------------------------------------------------------*/
void
plogc(ch)
char ch;
{
	if (plog_fp)
		fputc(ch, plog_fp);
}							 /* end of plogc */

/*+-------------------------------------------------------------------------
	pputc(ch) - put procedure output character to stderr and log
--------------------------------------------------------------------------*/
void
pputc(ch)
char ch;
{
	if (ch == NL)
		fputc(CRET, se);
	fputc(ch, se);
	if (plog_fp && (ch != CRET))
		fputc(ch, plog_fp);
	if (rcvr_log_fp && (ch != CRET))
		fputc(ch, rcvr_log_fp);
}							 /* end of pputc */

/*+-------------------------------------------------------------------------
	pputs(str) - put procedure output string to stderr and log
--------------------------------------------------------------------------*/
void
pputs(str)
char *str;
{
	while (*str)
	{
		if (*str == NL)
			fputc(CRET, se);
		fputc(*str, se);
		if (plog_fp && (*str != CRET))
			fputc(*str, plog_fp);
		if (rcvr_log_fp && (*str != CRET))
			fputc(*str, rcvr_log_fp);
		str++;
	}
}							 /* end of pputs */

/*+-------------------------------------------------------------------------
	pflush()
--------------------------------------------------------------------------*/
void
pflush()
{
	if (plog_fp)
		fflush(plog_fp);
}							 /* end of pflush */

/*+-------------------------------------------------------------------------
	pperror(str)
--------------------------------------------------------------------------*/
void
pperror(str)
char *str;
{
	int save_errno = errno;

	if (str && *str)
	{
		pputs(str);
		pputs(": ");
	}
	pputs(strerror(save_errno));
	pputs("\n");
}							 /* end of pperror */

/*+-------------------------------------------------------------------------
	plog_control(fname)
 fname == 0, close
 fname == 1, plog_name already plugged
--------------------------------------------------------------------------*/
int
plog_control(fname)
char *fname;
{
	if (!fname)				 /* close */
	{
		if (plog_fp)
			fclose(plog_fp);
		plog_fp = (FILE *) 0;
		return (0);
	}

	if (plog_fp)
		plog_control((char *)0);

	if (fname != (char *)1)
	{
		strcpy(plog_name->pb, fname);
		plog_name->cb = strlen(fname);
	}

	if ((plog_fp = fopen(plog_name->pb, "a")) == NULL)
	{
		pperror(plog_name->pb);
		return (eFATAL_ALREADY);
	}
	return (0);
}							 /* end of plog_control */

/*+-------------------------------------------------------------------------
	pcmd_plog(param)

plog $s0     log to file
plog off     stop logging
plog         show status
--------------------------------------------------------------------------*/
int
pcmd_plog(param)
ESD *param;
{
	int erc = eSyntaxError;
	char off_str[8];

	if (!skip_cmd_break(param))	/* if arguments */
	{
		if (!get_alpha_zstr(param, off_str, sizeof(off_str)))
		{
			if (strcmp(off_str, "off"))
				return (eBadParameter);
			erc = plog_control((char *)0);
		}
		else
		{
			if (erc = gstr(param, plog_name, 1))
				return (erc);
			erc = plog_control((char *)1);
		}
	}
	return (erc);

}							 /* end of pcmd_plog */

/*+-------------------------------------------------------------------------
	poutput_init()
--------------------------------------------------------------------------*/
void
poutput_init()
{
	if (!(plog_name = esdalloc(ESD_NOMSZ)))
	{
		ff(se, "poutput_init: Out of memory\r\n");
		errno = -1;
		termecu(TERMECU_MALLOC);
	}
}							 /* end of poutput_init */

/* vi: set tabstop=4 shiftwidth=4: */
/* end of poutput.c */
