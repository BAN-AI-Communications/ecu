/*+-------------------------------------------------------------------------
    feval.c - integer and string function evaluation
	wht@wht.net

    feval_int(param,&int_returned) where 'int' here means long ECU $i int
    feval_str(param,&esd_to_be_plugged)

  These routines are called with param.index as follows:

         !nnnnn       nnn is name of function
          ^
          |

  Defined functions:
	feval_int(param, value)
	feval_str(param, result_esd)
	strfunc_left(param, scratch_esd, result_esd)
	strfunc_right(param, scratch_esd, result_esd)

  Arithmetic is being able to count up to twenty without taking
  off your shoes.  -- Mickey Mouse

--------------------------------------------------------------------------*/
/*+:EDITS:*/
/*:04-26-2000-11:15-wht@bob-RELEASE 4.42 */
/*:01-02-2000-13:53-wht@menlo-%conn returns 0 or 1 only */
/*:04-05-1998-17:33-wht@kepler-move uname code to ecuutil.c get_uname_*name */
/*:01-24-1997-02:37-wht@yuriatin-SOURCE RELEASE 4.00 */
/*:11-01-1996-18:55-wht@yuriatin-add %dates, %timesm and %timesmz */
/*:09-11-1996-20:00-wht@yuriatin-3.48-major telnet,curses,structural overhaul */
/*:08-20-1996-12:39-wht@kepler-locale/ctype fixes from ache@nagual.ru */
/*:12-04-1995-00:02-wht@gyro-add sysname */
/*:12-04-1995-00:01-wht@gyro-uname gets nodename not sysname */
/*:11-23-1995-11:20-wht@kepler-source control 3.37 for tsx-11 */
/*:11-14-1995-10:23-wht@kepler-3.37.80-source control point: SOCKETS */
/*:05-04-1994-04:39-wht@n4hgf-ECU release 3.30 */
/*:01-21-1994-15:42-wht@n4hgf-shmid func missed break if not CFG_MmapSHM */
/*:09-10-1992-13:59-wht@n4hgf-ECU release 3.20 */
/*:08-22-1992-15:38-wht@n4hgf-ECU release 3.20 BETA */
/*:05-07-1992-15:04-wht@n4hgf-bug in FIconn code */
/*:04-25-1992-14:28-wht@n4hgf-%conn returns -1 if no line open */
/*:04-25-1992-13:19-wht@n4hgf-%line succeeds even if line not open */
/*:03-01-1992-13:28-wht@n4hgf-come up to modern times ... enum for FI/FS  */
/*:02-14-1992-16:37-wht@n4hgf-add uname */
/*:07-25-1991-12:58-wht@n4hgf-ECU release 3.10 */
/*:07-14-1991-18:18-wht@n4hgf-new ttygets functions */
/*:06-29-1991-16:33-wht@n4hgf-use cuserid() instead of getlogin() */
/*:03-16-1991-15:23-wht@n4hgf-add %nice */
/*:01-31-1991-16:10-wht@n4hgf-was freeing tesd1 twice in feval_str */
/*:12-24-1990-04:31-wht@n4hgf-experimental fasi driver functions */
/*:08-14-1990-20:40-wht@n4hgf-ecu3.00-flush old edit history */

#include "ecu.h"
#include "ecuerror.h"
#include "esd.h"
#include "procedure.h"
#include "var.h"
#include "ecutty.h"

#if defined(FASI)
char *msr_text();

#endif

enum FI_codes
{
	FIZERO,
	FIargc,
	FIbaud,
	FIbrdet,
	FIcolors,
	FIcols,
	FIconn,
	FIcsec,
	FIctoi,
	FIcurx,
	FIcury,
	FIesecs,
	FIfasi,
	FIfatime,
	FIfmode,
	FIfmtime,
	FIfsize,
	FIftell,
	FIgid,
	FIinstr,
	FIisalnum,
	FIisalpha,
	FIisascii,
	FIischr,
	FIiscntrl,
	FIisdigit,
	FIisdir,
	FIisgraph,
	FIislower,
	FIisprint,
	FIispunct,
	FIisreg,
	FIisspace,
	FIisupper,
	FIisxdigit,
	FIlen,
	FIlgetc,
	FIlines,
	FIlnerr,
	FImatch,
	FImhack,
	FImsr,
	FInice,
	FIpid,
	FIrchr,
	FIrchrc,
	FIridet,
	FIrinstr,
	FIshmid,
	FIstoi,
	FItelnet,
	FIuid,
	FIxchr,
	FIxchrc,
	FI____end
};

KEYTAB feval_int_tbl[] =
{
	{"argc", FIargc},
	{"baud", FIbaud},
#if defined(FASI)
	{"brdet", FIbrdet},
#endif
	{"colors", FIcolors},
	{"cols", FIcols},
	{"conn", FIconn},
	{"csec", FIcsec},
	{"ctoi", FIctoi},
	{"curx", FIcurx},
	{"cury", FIcury},
	{"esecs", FIesecs},
	{"fasi", FIfasi},
	{"fatime", FIfatime},
	{"fmode", FIfmode},
	{"fmtime", FIfmtime},
	{"fsize", FIfsize},
	{"ftell", FIftell},
	{"gid", FIgid},
	{"instr", FIinstr},
	{"isalnum", FIisalnum},
	{"isalpha", FIisalpha},
	{"isalpha", FIisalpha},
	{"isascii", FIisascii},
	{"ischr", FIischr},
	{"iscntrl", FIiscntrl},
	{"isdigit", FIisdigit},
	{"isdir", FIisdir},
	{"isgraph", FIisgraph},
	{"islower", FIislower},
	{"isprint", FIisprint},
	{"ispunct", FIispunct},
	{"isreg", FIisreg},
	{"isspace", FIisspace},
	{"isupper", FIisupper},
	{"isxdigit", FIisxdigit},
	{"len", FIlen},
#if defined(FASI)
	{"lnerr", FIlnerr},
#endif
	{"lgetc", FIlgetc},
	{"lines", FIlines},
	{"match", FImatch},
	{"mhack", FImhack},
#if defined(FASI)
	{"msr", FImsr},
#endif
	{"nice", FInice},
	{"pid", FIpid},
	{"rchr", FIrchr},
	{"rchrc", FIrchrc},
#if defined(FASI)
	{"ridet", FIridet},
#endif
	{"rinstr", FIrinstr},
	{"shmid", FIshmid},
	{"stoi", FIstoi},
	{"telnet", FItelnet},
	{"uid", FIuid},
	{"xchr", FIxchr},
	{"xchrc", FIxchrc},
	{(char *)0, 0}
};

enum FS_codes
{
	FSZERO,
	FSargv,
	FSbasename,
	FScgetc,
	FScgets,
	FSchr,
	FSdate,
	FSdates,
	FSdatez,
	FSday,
	FSdayz,
	FSdir,
	FSdirpart,
	FSedate,
	FSenvvar,
	FSerrstr,
	FSetime,
	FSfilepart,
	FSfmodestr,
	FSitos,
	FSleft,
	FSline,
	FSlogname,
	FSmid,
	FSmonth,
	FSmonthz,
	FSmsrtext,
	FSrdesc,
	FSright,
	FSrname,
	FSrtel,
	FSscreen,
	FSsysname,
	FStime,
	FStimesm,
	FStimesmz,
	FStimes,
	FStimez,
	FStimezs,
	FStty,
	FSuname,
	FS____end
};

KEYTAB feval_str_tbl[] =
{
	{"argv", FSargv},
	{"basename", FSbasename},
	{"cgetc", FScgetc},
	{"cgets", FScgets},
	{"chr", FSchr},
	{"date", FSdate},
	{"dates", FSdates},
	{"datez", FSdatez},
	{"day", FSday},
	{"dir", FSdir},
	{"dirpart", FSdirpart},
	{"edate", FSedate},
	{"envvar", FSenvvar},
	{"errstr", FSerrstr},
	{"etime", FSetime},
	{"filepart", FSfilepart},
	{"fmodestr", FSfmodestr},
	{"getenv", FSenvvar},
	{"itos", FSitos},
	{"left", FSleft},
	{"line", FSline},
	{"logname", FSlogname},
	{"mid", FSmid},
	{"month", FSmonth},
#if defined(FASI)
	{"msrtext", FSmsrtext},
#endif
	{"rdesc", FSrdesc},
	{"right", FSright},
	{"rname", FSrname},
	{"rtelno", FSrtel},
	{"screen", FSscreen},
	{"sysname", FSsysname},
	{"time", FStime},
	{"timesm", FStimesm},
	{"timesmz", FStimesmz},
	{"times", FStimes},
	{"timez", FStimez},
	{"timezs", FStimezs},
	{"tty", FStty},
	{"uname", FSuname},
	{(char *)0, 0}
};

extern char curr_dir[];
extern PCB *pcb_stack[];
extern UINT tcap_LINES;
extern UINT tcap_COLS;
extern struct TIMEB starting_timeb;

char *day_of_week_list = "SunMonTueWedThuFriSat";
char *month_name_list = "JanFebMarAprMayJunJulAugSepOctNovDec";

/*+-------------------------------------------------------------------------
    erc = feval_int(param,&int_returned);
Functions (parameter types are expressed by the usage of variables)
--------------------------------------------------------------------------*/
int
feval_int(param, value)
ESD *param;
long *value;
{
	int erc;
	int keyword_token;
	ESD *tesd1 = (ESD *) 0;
	ESD *tesd2 = (ESD *) 0;
	UINT32 int1;
	char s32[32];
	struct TIMEB timeb;

	if (erc = get_alphanum_zstr(param, s32, sizeof(s32)))
	{
		erc = eInvalidFunction;
		goto FUNC_RETURN;
	}

	keyword_token = keyword_lookup(feval_int_tbl, s32);
	switch (keyword_token)
	{
/* LEN($S0)         length of $S0 */
		case FIlen:
			if (!(tesd1 = esdalloc(ESD_NOMSZ)))
			{
				erc = eNoMemory;
				goto FUNC_RETURN;
			}
			if (erc = skip_paren(param, 1))
				goto FUNC_RETURN;
			if (erc = gstr(param, tesd1, 1))
				goto FUNC_RETURN;
			if (erc = skip_paren(param, 0))
				goto FUNC_RETURN;
			*value = (long)tesd1->cb;
			break;

/* INSTR($S0,$S1)   index of first occurrence of $S1 in $S0, -1 if none */
		case FIinstr:
			if (!(tesd1 = esdalloc(ESD_NOMSZ)))
			{
				erc = eNoMemory;
				goto FUNC_RETURN;
			}
			if (!(tesd2 = esdalloc(ESD_NOMSZ)))
			{
				erc = eNoMemory;
				goto FUNC_RETURN;
			}
			if (erc = skip_paren(param, 1))
				goto FUNC_RETURN;
			if (erc = gstr(param, tesd1, 1))
				goto FUNC_RETURN;
			if (erc = skip_comma(param))
				goto FUNC_RETURN;
			if (erc = gstr(param, tesd2, 1))
				goto FUNC_RETURN;
			if (erc = skip_paren(param, 0))
				goto FUNC_RETURN;

			*value = (long)ulindex(tesd1->pb, tesd2->pb);
			break;

/* RINSTR($S0,$S1)   index of last occurrence of $S1 in $S0, -1 if none */
		case FIrinstr:
			if (!(tesd1 = esdalloc(ESD_NOMSZ)))
			{
				erc = eNoMemory;
				goto FUNC_RETURN;
			}
			if (!(tesd2 = esdalloc(ESD_NOMSZ)))
			{
				erc = eNoMemory;
				goto FUNC_RETURN;
			}
			if (erc = skip_paren(param, 1))
				goto FUNC_RETURN;
			if (erc = gstr(param, tesd1, 1))
				goto FUNC_RETURN;
			if (erc = skip_comma(param))
				goto FUNC_RETURN;
			if (erc = gstr(param, tesd2, 1))
				goto FUNC_RETURN;
			if (erc = skip_paren(param, 0))
				goto FUNC_RETURN;

			*value = (long)ulrindex(tesd1->pb, tesd2->pb);
			break;

		case FImatch:
			if (!(tesd1 = esdalloc(ESD_NOMSZ)))
			{
				erc = eNoMemory;
				goto FUNC_RETURN;
			}
			if (!(tesd2 = esdalloc(ESD_NOMSZ)))
			{
				erc = eNoMemory;
				goto FUNC_RETURN;
			}
			if (erc = skip_paren(param, 1))
				goto FUNC_RETURN;
			if (erc = gstr(param, tesd1, 1))
				goto FUNC_RETURN;
			if (erc = skip_comma(param))
				goto FUNC_RETURN;
			if (erc = gstr(param, tesd2, 1))
				goto FUNC_RETURN;
			if (erc = skip_paren(param, 0))
				goto FUNC_RETURN;

			erc = regexp_operation(tesd1->pb, tesd2->pb, value);
			break;

		case FImhack:
			Ftime(&timeb);
			*value = ((timeb.time - starting_timeb.time) * 1000) +
				(timeb.millitm - starting_timeb.millitm);
			erc = 0;
			break;

		case FIesecs:
			Ftime(&timeb);
			*value = timeb.time;
			erc = 0;
			break;

		case FIargc:
			if (!proc_level)
			{
				pputs("not executing procedure\n");
				erc = eFATAL_ALREADY;
				break;
			}
			*value = (long)pcb_stack[proc_level - 1]->argc;
			break;

		case FIcolors:
			if (erc = ifunc_colors(value))
				goto FUNC_RETURN;
			break;

		case FIftell:
			if (erc = ifunc_ftell(param, value))
				goto FUNC_RETURN;
			break;

		case FIfmode:
			if (erc = ifunc_fmode(param, value))
				goto FUNC_RETURN;
			break;

		case FIfsize:
			if (erc = ifunc_fsize(param, value))
				goto FUNC_RETURN;
			break;

		case FIfmtime:
			if (erc = ifunc_fmtime(param, value))
				goto FUNC_RETURN;
			break;

		case FIfatime:
			if (erc = ifunc_fatime(param, value))
				goto FUNC_RETURN;
			break;

		case FIischr:
			if (erc = ifunc_ischr(param, value))
				goto FUNC_RETURN;
			break;

		case FIisdir:
			if (erc = ifunc_isdir(param, value))
				goto FUNC_RETURN;
			break;

		case FIisreg:
			if (erc = ifunc_isreg(param, value))
				goto FUNC_RETURN;
			break;

		case FIbaud:
			*value = (long)shm->Lbitrate;
			erc = 0;
			break;

		case FIpid:
			*value = (long)getpid();
			erc = 0;
			break;

		case FIcsec:
			*value = (shm->Lconnected) ? shm->Loff_hook_time : -1;
			erc = 0;
			break;

		case FIconn:
			if (shm->Liofd < 0)
				*value = 0;
			else
				*value = (long)(shm->Lconnected) ? (long)shm->Liofd : 0;
			erc = 0;
			break;

		case FIxchr:
			*value = shm->xmit_chars;
			erc = 0;
			break;

		case FIxchrc:
			*value = shm->xmit_chars_this_connect;
			erc = 0;
			break;

		case FIrchr:
			*value = shm->rcvd_chars;
			erc = 0;
			break;

		case FIrchrc:
			*value = shm->rcvd_chars_this_connect;
			erc = 0;
			break;

/* LGETC($I0) get char from line, waiting for $I0 msec
returns  character read or -1 if none read in time */
		case FIlgetc:
			if (erc = skip_paren(param, 1))
				goto FUNC_RETURN;
			if (erc = gint(param, &int1))
				goto FUNC_RETURN;
			if (erc = skip_paren(param, 0))
				goto FUNC_RETURN;
			*value = (long)lgetc_timeout(int1);
			if (zero_length_read_detected)
			{
				zero_length_read_detected = 0;
				erc = eProcAttn_DCDloss;
			}
			break;

		case FIctoi:
			if (!(tesd1 = esdalloc(ESD_NOMSZ)))
			{
				erc = eNoMemory;
				goto FUNC_RETURN;
			}
			if (erc = skip_paren(param, 1))
				goto FUNC_RETURN;
			if (erc = gstr(param, tesd1, 1))
				goto FUNC_RETURN;
			if (erc = skip_paren(param, 0))
				goto FUNC_RETURN;
			if (tesd1->cb == 0)
				*value = -1;
			else
				*value = (long)((unsigned)0xFF & (unsigned)tesd1->pb[0]);
			break;

		case FIstoi:
			if (!(tesd1 = esdalloc(ESD_NOMSZ)))
			{
				erc = eNoMemory;
				goto FUNC_RETURN;
			}
			if (erc = skip_paren(param, 1))
				goto FUNC_RETURN;
			if (erc = gstr(param, tesd1, 1))
				goto FUNC_RETURN;
			if (erc = skip_paren(param, 0))
				goto FUNC_RETURN;

			tesd1->index = 0;
			skip_cmd_break(tesd1);
			*value = 0;
			gint_constant(tesd1, value);
			break;

		case FItelnet:
#ifdef CFG_TelnetOption
			*value = shm->Ltelnet;
#else
			*value = 0;
#endif
			break;

		case FIcurx:
			*value = (long)shm->cursor_x;
			break;

		case FIcury:
			*value = (long)shm->cursor_y;
			break;

		case FIshmid:
#if !defined(CFG_MmapSHM)
			{
				extern int shm_shmid;

				*value = (long)shm_shmid;
			}
#else
			*value = -1L;	 /* not present */
#endif
			break;

		case FIisalpha:
		case FIisupper:
		case FIislower:
		case FIisdigit:
		case FIisxdigit:
		case FIisspace:
		case FIispunct:
		case FIisalnum:
		case FIisprint:
		case FIisgraph:
		case FIiscntrl:
		case FIisascii:
			if (!(tesd1 = esdalloc(ESD_NOMSZ)))
			{
				erc = eNoMemory;
				goto FUNC_RETURN;
			}
			if (erc = skip_paren(param, 1))
				goto FUNC_RETURN;
			if (erc = gstr(param, tesd1, 1))
				goto FUNC_RETURN;
			if (erc = skip_paren(param, 0))
				goto FUNC_RETURN;
			if (!tesd1->cb)
			{
				*value = 0;
				goto FUNC_RETURN;
			}
			switch (keyword_token)
			{
				case FIisalpha:
					*value = !!isalpha((uchar) * tesd1->pb);
					break;
				case FIisupper:
					*value = !!isupper((uchar) * tesd1->pb);
					break;
				case FIislower:
					*value = !!islower((uchar) * tesd1->pb);
					break;
				case FIisdigit:
					*value = !!isdigit((uchar) * tesd1->pb);
					break;
				case FIisxdigit:
					*value = !!isxdigit((uchar) * tesd1->pb);
					break;
				case FIisspace:
					*value = !!isspace((uchar) * tesd1->pb);
					break;
				case FIispunct:
					*value = !!ispunct((uchar) * tesd1->pb);
					break;
				case FIisalnum:
					*value = !!isalnum((uchar) * tesd1->pb);
					break;
				case FIisprint:
					*value = !!isprint((uchar) * tesd1->pb);
					break;
				case FIisgraph:
					*value = !!isgraph((uchar) * tesd1->pb);
					break;
				case FIiscntrl:
					*value = !!iscntrl((uchar) * tesd1->pb);
					break;
				case FIisascii:
					*value = !!isascii((uchar) * tesd1->pb);
					break;
			}
			break;

		case FIlines:
			*value = (long)tcap_LINES;
			break;

		case FIcols:
			*value = (long)tcap_COLS;
			break;

#if defined(FASI)
		case FIfasi:
			*value = 1;
			break;
		case FImsr:
			*value = (long)fasi_msr();
			break;
		case FIlnerr:
			*value = (long)fasi_line_errors();
			break;
		case FIridet:
			*value = (long)fasi_rings_detected();
			break;
		case FIbrdet:
			*value = (long)fasi_breaks_detected();
			break;
#else
		case FIfasi:
			*value = 0;
			break;
#endif

		case FInice:
			*value = (long)nice(0) + 20;
			erc = 0;
			break;

		case FIuid:
			*value = (long)getuid();
			erc = 0;
			break;

		case FIgid:
			*value = (long)getgid();
			erc = 0;
			break;

		default:
			erc = eInvalidFunction;
	}						 /* end of keyword lookup erc switch statement */

  FUNC_RETURN:
	if (tesd1)
		esdfree(tesd1);
	if (tesd2)
		esdfree(tesd2);
	return (erc);

}							 /* end of feval_int() */

/*+------------------------------------------------------------------
    strfunc_left(param,&scratch_esd,&result_esd)
-------------------------------------------------------------------*/
int
strfunc_left(param, scratch_esd, result_esd)
ESD *param;
ESD *scratch_esd;
ESD *result_esd;
{
	int erc;
	int itmp;
	long ltmp;

	if (erc = skip_paren(param, 1))
		return (erc);
	if (erc = gstr(param, scratch_esd, 1))
		return (erc);
	if (erc = skip_comma(param))
		return (erc);
	if (erc = gint(param, &ltmp))
		return (erc);
	itmp = (int)ltmp;
	if (itmp < 0)
		return (eBadParameter);
	if (erc = skip_paren(param, 0))
		return (erc);
	/* take min of param and .cb */
	itmp = (itmp < scratch_esd->cb) ? itmp : scratch_esd->cb;
	if (itmp > (result_esd->maxcb - result_esd->cb))
		return (eBufferTooSmall);
	memcpy(&result_esd->pb[result_esd->cb],
		scratch_esd->pb, itmp);
	result_esd->cb += itmp;
	return (erc);
}							 /* end of strfunc_left() */

/*+-------------------------------------------------------------------------
    erc = strfunc_right(param,&scratch_esd,&result_esd)
--------------------------------------------------------------------------*/
int
strfunc_right(param, scratch_esd, result_esd)
ESD *param;
ESD *scratch_esd;
ESD *result_esd;
{
	int erc;
	int itmp;
	long ltmp;

	if (erc = skip_paren(param, 1))
		return (erc);
	if (erc = gstr(param, scratch_esd, 1))
		return (erc);
	if (erc = skip_comma(param))
		return (erc);
	if (erc = gint(param, &ltmp))
		return (erc);
	itmp = (int)ltmp;
	if (itmp < 0)
		return (eBadParameter);
	if (erc = skip_paren(param, 0))
		return (erc);

/* take min of param and .cb */
	itmp = (itmp < scratch_esd->cb) ? itmp : scratch_esd->cb;
	if (itmp > (result_esd->maxcb - result_esd->cb))
		return (eBufferTooSmall);
	memcpy(&result_esd->pb[result_esd->cb],
		&scratch_esd->pb[scratch_esd->cb - itmp], itmp);
	result_esd->cb += itmp;
	return (0);

}							 /* end of strfunc_right() */

/*+-------------------------------------------------------------------------
    erc = feval_str(param,&esd_to_be_plugged);
    results are APPENDED to 'result_esd'
--------------------------------------------------------------------------*/
feval_str(param, result_esd)
ESD *param;
ESD *result_esd;
{
	int cmd_token;
	int erc;
	int itmp = 0;
	int int1, int2;
	char s64[64];
	char *cp = 0;
	long ltmp;
	long ltmp2;
	long ltmp3;
	ESD *tesd1 = 0;
	ESD *tesd2 = 0;
	struct TIMEB timeb;
	char *get_ttyname();
	char *getenv();
	char *cuserid();
	char *elapsed_time_text();
	char *mode_map();
	char *get_uname_nodename();
	char *get_uname_sysname();

	if (!(tesd1 = esdalloc(128)))
		return (eNoMemory);

	if (erc = get_alphanum_zstr(param, s64, sizeof(s64) - 1))
	{
		esdfree(tesd1);
		return (eInvalidFunction);
	}

	erc = 0;
	cmd_token = keyword_lookup(feval_str_tbl, s64);
	switch (cmd_token)
	{
/* LEFT($S0,$I0)   return leftmost $I0 characters of $S0 */
		case FSleft:
			erc = strfunc_left(param, tesd1, result_esd);
			break;

/* RIGHT($S0,$I0)   return rightmost $I0 characters of $S0 */
		case FSright:
			erc = strfunc_right(param, tesd1, result_esd);
			break;

/* MID($S0,$I0,$I1)   return middle $I1 chars of $S0 starting at $I0 */
		case FSmid:
			if (erc = skip_paren(param, 1))
				break;
			if (erc = gstr(param, tesd1, 1))
				break;
			if (erc = skip_comma(param))
				break;
			if (erc = gint(param, &ltmp))
				break;
			int1 = (int)ltmp;
			if (int1 < 0)
			{
				erc = eBadParameter;
				break;
			}
			if (erc = skip_cmd_break(param))
				break;
			if (param->pb[param->index] == ')')	/* if we find a ')'
												 * instead of ... */
			{				 /* 2nd int param, default to max */
				++param->index;
				int2 = 256;
			}
			else
			{
				if (erc = skip_comma(param))
					break;
				if (erc = gint(param, &ltmp))
					break;
				int2 = (int)ltmp;
				if (int2 < 0)
				{
					erc = eBadParameter;
					break;
				}
				if (erc = skip_paren(param, 0))
					break;
			}

			if (int1 >= tesd1->cb)	/* if initial index past end of string */
				break;
			itmp = tesd1->cb - int1;
			itmp = (int2 < itmp) ? int2 : itmp;
			cp = tesd1->pb + int1;
			goto CPTR_ITMP_COMMON;

/* ARGV($I0) */
		case FSargv:
			if (!proc_level)
			{
				pputs("not executing procedure\n");
				erc = eFATAL_ALREADY;
				break;
			}
			if (erc = skip_paren(param, 1))
				break;
			if (erc = gint(param, &ltmp))
				break;
			if (erc = skip_paren(param, 0))
				break;
			itmp = (long)pcb_stack[proc_level - 1]->argc;	/* arg count */
			if ((int)ltmp > itmp - 1)
			{
				if (proc_trace)
				{
					pprintf("WARNING: %%argc=%d, %%argv(%ld) null\n",
						itmp, ltmp);
				}
				break;
			}
			cp = (pcb_stack[proc_level - 1])->argv[(int)ltmp];
			itmp = strlen(cp);
			goto CPTR_ITMP_COMMON;

		case FSdir:
			cp = curr_dir;
			itmp = strlen(curr_dir);
			goto CPTR_ITMP_COMMON;

		case FSetime:
			if (erc = skip_paren(param, 1))
				break;
			if (erc = gint(param, &ltmp))
				break;
			if (erc = skip_paren(param, 0))
				break;
			cp = elapsed_time_text(ltmp);
			itmp = strlen(cp);
			goto CPTR_ITMP_COMMON;

		case FSerrstr:
			if (erc = skip_paren(param, 1))
				break;
			if (erc = gint(param, &ltmp))
				break;
			if (erc = skip_paren(param, 0))
				break;
			cp = strerror((int)ltmp);
			itmp = strlen(cp);
			goto CPTR_ITMP_COMMON;

		case FSenvvar:
			if (erc = skip_paren(param, 1))
				break;
			if (erc = gstr(param, tesd1, 1))
				break;
			if (erc = skip_paren(param, 0))
				break;
			if (!(cp = getenv(tesd1->pb)))
				break;
			itmp = strlen(cp);
			goto CPTR_ITMP_COMMON;

		case FSlogname:
			if (!(cp = cuserid((char *)0)))
				break;
			itmp = strlen(cp);
			goto CPTR_ITMP_COMMON;

		case FSfmodestr:
			if (erc = skip_paren(param, 1))
				break;
			if (erc = gint(param, &ltmp))
				break;
			if (erc = skip_paren(param, 0))
				break;
			cp = mode_map((UINT16) ltmp, (char *)0);
			itmp = strlen(cp);
			goto CPTR_ITMP_COMMON;

		case FStty:
			cp = get_ttyname();
			itmp = strlen(cp);
			goto CPTR_ITMP_COMMON;

		case FSuname:
			if(!(cp = get_uname_nodename())) /* ecuutil.c */
			{
				erc = eFATAL_ALREADY;
				break;
			}
			itmp = strlen(cp);
			goto CPTR_ITMP_COMMON;

		case FSsysname:
			if(!(cp = get_uname_sysname())) /* ecuutil.c */
			{
				erc = eFATAL_ALREADY;
				break;
			}
			itmp = strlen(cp);
			goto CPTR_ITMP_COMMON;

		case FSrname:
			if (!shm->Lconnected)
				break;
			cp = shm->Lrname;
			itmp = strlen(shm->Lrname);
			goto CPTR_ITMP_COMMON;

		case FSrdesc:
			if (!shm->Lconnected)
				break;
			cp = shm->Ldescr;
			itmp = strlen(shm->Ldescr);
			goto CPTR_ITMP_COMMON;

		case FSrtel:
			if (!shm->Lconnected)
				break;
			cp = shm->Ltelno;
			itmp = strlen(shm->Ltelno);
			goto CPTR_ITMP_COMMON;

		case FSline:
			cp = shm->Lline;
			itmp = strlen(shm->Lline);
			goto CPTR_ITMP_COMMON;

		case FSmonth:
		case FSmonthz:
			cp = &month_name_list[(get_month(itmp == FSmonthz) - 1) * 3];
			itmp = 3;
			goto CPTR_ITMP_COMMON;

		case FSday:
		case FSdayz:
			cp = &day_of_week_list[get_day(itmp == FSdayz) * 3];
			itmp = 3;
			goto CPTR_ITMP_COMMON;

		case FSscreen:
			if (erc = skip_paren(param, 1))
				break;
			if (erc = gint(param, &ltmp))	/* y */
				break;
			if (ltmp > 42)
			{
				erc = eBadParameter;
				break;
			}
			if (erc = skip_comma(param))
				break;
			if (erc = gint(param, &ltmp2))	/* x */
				break;
			if (ltmp2 > 79)
			{
				erc = eBadParameter;
				break;
			}
			if (erc = skip_comma(param))
				break;
			if (erc = gint(param, &ltmp3))	/* len */
				break;
			if (erc = skip_paren(param, 0))
				break;

			int1 = ((int)ltmp * 80) + (int)ltmp2;	/* screen position */
			itmp = (int)ltmp3;	/* length */
			int2 = sizeof(shm->screen) - int1;	/* size from y,x to end */
			if (itmp > int2)
				itmp = int2;
			cp = ((char *)shm->screen) + int1;
			goto CPTR_ITMP_COMMON;

		case FSbasename:
			if (!(tesd2 = esdalloc(32)))
				return (eNoMemory);
			if (erc = skip_paren(param, 1))
				break;
			if (erc = gstr(param, tesd1, 1))
				break;
			if (erc = skip_comma(param))
				break;
			if (erc = gstr(param, tesd2, 1))
				break;
			if (erc = skip_paren(param, 0))
				break;
			cp = tesd1->pb;
			itmp = tesd1->cb;
			if ((tesd1->cb >= tesd2->cb) &&
				!strcmp(cp + tesd1->cb - tesd2->cb, tesd2->pb))
			{
				itmp -= tesd2->cb;
			}
			goto CPTR_ITMP_COMMON;

		case FSdirpart:
			if (erc = skip_paren(param, 1))
				break;
			if (erc = gstr(param, tesd1, 1))
				break;
			if (erc = skip_paren(param, 0))
				break;
			if (cp = strrchr(tesd1->pb, '/'))
				itmp = cp - tesd1->pb;
			else
				itmp = tesd1->cb;
			cp = tesd1->pb;
			goto CPTR_ITMP_COMMON;

		case FSfilepart:
			if (erc = skip_paren(param, 1))
				break;
			if (erc = gstr(param, tesd1, 1))
				break;
			if (erc = skip_paren(param, 0))
				break;
			if (cp = strrchr(tesd1->pb, '/'))
				itmp = strlen(++cp);
			else
			{
				cp = tesd1->pb;
				itmp = tesd1->cb;
			}
			goto CPTR_ITMP_COMMON;

#if defined(FASI)
		case FSmsrtext:
			cp = msr_text(fasi_msr());
			itmp = strlen(cp);
			goto CPTR_ITMP_COMMON;
#endif

		  CPTR_ITMP_COMMON:
			if (itmp > (result_esd->maxcb - result_esd->cb))
			{
				erc = eBufferTooSmall;
				break;
			}
			memcpy(&result_esd->pb[result_esd->cb], cp, itmp);
			result_esd->cb += itmp;
			break;

		case FSedate:
			if (erc = skip_paren(param, 1))
				break;
			if (erc = gint(param, &ltmp))
				break;
			if (erc = skip_paren(param, 0))
				break;
			if (19 > (result_esd->maxcb - result_esd->cb))
			{
				erc = eBufferTooSmall;
				break;
			}
			epoch_secs_to_str(ltmp, 3, &result_esd->pb[result_esd->cb]);
			result_esd->cb += 19;
			break;

		case FStime:
			if (5 > (result_esd->maxcb - result_esd->cb))
			{
				erc = eBufferTooSmall;
				break;
			}
			timeofday_text(0, &result_esd->pb[result_esd->cb]);
			result_esd->cb += 5;
			break;

		case FStimesm:			/* "hh:mm:ss.mmm" */
		case FStimesmz:			/* "hh:mm:ss.mmmZ" */
			if (13 > (result_esd->maxcb - result_esd->cb))
			{
				erc = eBufferTooSmall;
				break;
			}
			Ftime(&timeb);
			epoch_secs_to_str(timeb.time, 
				(cmd_token == FStimesm) ? 1 : 7,
				result_esd->pb + result_esd->cb);
			*(result_esd->pb + result_esd->cb + 8) = '.';
			sprintf(result_esd->pb + result_esd->cb + 9,"%03d", timeb.millitm);
			result_esd->cb += 12;
			if(cmd_token == FStimesmz)
			{
				*(result_esd->pb + result_esd->cb++) = 'Z';
				*(result_esd->pb + result_esd->cb) = 0;
			}
			break;

		case FStimes:
			if (8 > (result_esd->maxcb - result_esd->cb))
			{
				erc = eBufferTooSmall;
				break;
			}
			timeofday_text(1, &result_esd->pb[result_esd->cb]);
			result_esd->cb += 8;
			break;

		case FStimez:
			if (5 > (result_esd->maxcb - result_esd->cb))
			{
				erc = eBufferTooSmall;
				break;
			}
			timeofday_text(6, &result_esd->pb[result_esd->cb]);
			result_esd->cb += 5;
			break;

		case FStimezs:
			if (8 > (result_esd->maxcb - result_esd->cb))
			{
				erc = eBufferTooSmall;
				break;
			}
			timeofday_text(7, &result_esd->pb[result_esd->cb]);
			result_esd->cb += 8;
			break;

		case FSdate:
			if (10 > (result_esd->maxcb - result_esd->cb))
			{
				erc = eBufferTooSmall;
				break;
			}
			timeofday_text(5, &result_esd->pb[result_esd->cb]);
			result_esd->cb += 10;
			break;

		case FSdates:
			if (5 > (result_esd->maxcb - result_esd->cb))
			{
				erc = eBufferTooSmall;
				break;
			}
			timeofday_text(9, result_esd->pb + result_esd->cb);
			result_esd->cb += 5;
			break;

		case FSdatez:
			if (10 > (result_esd->maxcb - result_esd->cb))
			{
				erc = eBufferTooSmall;
				break;
			}
			timeofday_text(8, &result_esd->pb[result_esd->cb]);
			result_esd->cb += 10;
			break;

		case FScgets:
			erc = ttygets_esd(result_esd, TG_CRLF, 1);
			break;

		case FScgetc:
			if (result_esd->cb == result_esd->maxcb)
			{
				erc = eBufferTooSmall;
				break;
			}
			result_esd->pb[result_esd->cb] = ttygetc(0);
			result_esd->cb++;
			break;

		case FSchr:
			if (erc = skip_paren(param, 1))
				break;
			if (erc = gint(param, &ltmp))
				break;
			if (!ltmp)
			{
				pputs("cannot use %chr(0)\n");
				return (eFATAL_ALREADY);
			}
			if (erc = skip_paren(param, 0))
				break;
			if (result_esd->cb == result_esd->maxcb)
			{
				erc = eBufferTooSmall;
				break;
			}
			result_esd->pb[result_esd->cb] = (char)ltmp;
			result_esd->cb++;
			break;

		case FSitos:
			if (erc = skip_paren(param, 1))
				break;
			if (erc = gint(param, &ltmp))
				break;
			s64[0] = 0;
			if (!skip_comma(param))
			{
				if (erc = get_numeric_zstr(param, s64 + 1, sizeof(s64) - 4))
					strcpy(s64 + 1, "1");
				if (((itmp = atoi(s64 + 1)) < 0) ||
					(itmp > (result_esd->maxcb - result_esd->cb)))
				{
					erc = eBufferTooSmall;
					break;
				}
				s64[0] = '%';
				if (ulindex(param->pb + param->index, "x") == 0)
				{
					param->index++;
					strcat(s64, "lx");
				}
				else if (ulindex(param->pb + param->index, "o") == 0)
				{
					param->index++;
					strcat(s64, "lo");
				}
				else if (ulindex(param->pb + param->index, "d") == 0)
				{
					param->index++;
					strcat(s64, "ld");
				}
				else if (erc)
					break;
				else
					strcat(s64, "ld");
			}

			if (erc = skip_paren(param, 0))
				break;
			sprintf(tesd1->pb, s64[0] ? s64 : "%ld", ltmp);
			tesd1->cb = strlen(tesd1->pb);
			if (result_esd->maxcb - result_esd->cb < tesd1->cb)
			{
				erc = eBufferTooSmall;
				break;
			}
			strcpy(&result_esd->pb[result_esd->cb], tesd1->pb);
			result_esd->cb += tesd1->cb;
			break;

		default:
			erc = eInvalidFunction;
			break;
	}						 /* end of keyword lookup erc switch statement */

	esd_null_terminate(result_esd);
	esdfree(tesd1);
	if (tesd2)
		esdfree(tesd2);
	return (erc);

}							 /* end of feval_str() */

/* vi: set tabstop=4 shiftwidth=4: */
/* end of feval.c */
