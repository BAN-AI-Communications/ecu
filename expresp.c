/*+-------------------------------------------------------------------------
	expresp.c - HDB expect/respond per SCO Devices file
	wht@wht.net

 Meaning of some of the escape characters:
 \p - pause (approximately 1/4-1/2 second delay)
 \d - delay (2 seconds)
 \D - phone number/token
 \T - phone number with Dialcodes and character translation
 \N - null byte
 \K - insert a BREAK
 \E - turn on echo checking (for slow devices)
 \e - turn off echo checking
 \r - carriage return
 \c - no new-line
 \n - send new-line
 \nnn - send octal number
 \\ - send backslash
 \m### - sleep ### (decimal) milliseconds (non-standard)
 Speed - Hayes-specific "CONNECT"  handler

  Defined functions:
	execute_expresp(expresp_script)
	expect(str)
	pcmd_expresp(param)
	respond(str)

  "Why should we live with such hurry and waste of life?  We are
  determined to be starved before we are hungry?  Men say that a
  stich in time saves nine, and so they take a thousand stiches
  today to save nine thousand tommorrow." -- Henry David Thoreau

--------------------------------------------------------------------------*/
/*+:EDITS:*/
/*:04-26-2000-11:15-wht@bob-RELEASE 4.42 */
/*:12-12-1997-21:11-wht@kepler-complete isolation of substituted ftime */
/*:06-25-1997-00:16-wht@kepler-messed up a simple fix */
/*:06-24-1997-20:24-wht@kepler-strdup clone allocated 1 too few bytes */
/*:01-24-1997-02:37-wht@yuriatin-SOURCE RELEASE 4.00 */
/*:09-11-1996-20:00-wht@yuriatin-3.48-major telnet,curses,structural overhaul */
/*:08-20-1996-12:39-wht@kepler-locale/ctype fixes from ache@nagual.ru */
/*:11-23-1995-11:20-wht@kepler-source control 3.37 for tsx-11 */
/*:11-14-1995-10:23-wht@kepler-3.37.80-source control point: SOCKETS */
/*:05-11-1995-15:55-wht@n4hgf-ck_sigint fools optimizing compilers */
/*:03-21-1995-15:12-wht@n4hgf-ERDEBUG now essentially binary */
/*:05-04-1994-04:39-wht@n4hgf-ECU release 3.30 */
/*:01-04-1994-06:34-wht@n4hgf-CFG_DialTimeout + last_Speed_result fix */
/*:09-10-1992-13:59-wht@n4hgf-ECU release 3.20 */
/*:09-04-1992-19:07-wht@n4hgf-new msec delay syntax + harden */
/*:08-22-1992-15:38-wht@n4hgf-ECU release 3.20 BETA */
/*:12-16-1991-15:25-wht@n4hgf-allow for backslash in expect and respond */
/*:10-09-1991-20:21-wht@n4hgf-bad llookfor echo argument */
/*:08-01-1991-05:00-wht@n4hgf-\n sent CR not NL */
/*:08-01-1991-04:31-wht@n4hgf-nap min of hzmsec if \m */
/*:08-01-1991-04:22-wht@n4hgf-detect NULL expect string */
/*:07-25-1991-12:57-wht@n4hgf-ECU release 3.10 */
/*:07-17-1991-07:04-wht@n4hgf-avoid SCO UNIX nap bug */
/*:08-14-1990-20:40-wht@n4hgf-ecu3.00-flush old edit history */

#include "ecu.h"
#include "ecuerror.h"
#include "esd.h"
#include "var.h"
#include "procedure.h"

#define MAX_FIELDS	50		 /* max fields in a chat script */
#define MAX_EXPECT	63		 /* max length of expect string */
#define DEFAULT_TIMEOUT_MSECS (10*1000L)

#define ERDEBUG(verb,str,arg) if(expresp_verbosity > (verb)) \
	pprintf(str,arg)

long atol();
char *strip_phone_num();
char *dialcodes_translate();


int expresp_verbosity = 0;
UINT32 expect_timeout_msecs = DEFAULT_TIMEOUT_MSECS;
int expresp_echo_check = 0;

char last_Speed_result[32];

/*+-------------------------------------------------------------------------
	expect(str) - expect (read) string from remote
return code on failure, 0 on success
--------------------------------------------------------------------------*/
int
expect(str)
char *str;
{
	int erc;
	int itmp;
	char op;
	char s8[8];
	char parsebuf[MAX_EXPECT + 1];
	int remaining = MAX_EXPECT;
	long atol();
	char *cp;
	char *parsed = parsebuf;
	int old_ttymode = get_ttymode();

	if (!str)
	{
		ERDEBUG(0, "\nexpect string cannot be NULL\n", 0);
		return (eExpectRespondFail);
	}

	if (old_ttymode != 2)
		ttymode(2);

	/*
	 * ~[?]
	 */
	if ((*str == '~') && *(str + 1) && (*(str + 2) == '['))
	{
		op = *(str + 1);
		str += 3;
		switch (op)
		{
			case 'm':		 /* msec expect timeout */
			case 't':		 /* sec expect timeout */
				expect_timeout_msecs = atol(str);
				if (op == 't')
					expect_timeout_msecs *= 1000L;
				ERDEBUG(1, "expect timeout = %lu msec\n", expect_timeout_msecs);
				break;

			default:
				ERDEBUG(0, "\nexpect: invalid subop: ~%c[]\n", op);
				break;
		}
		if (cp = strchr(str, ']'))
			str = cp + 1;
		else
		{
			ERDEBUG(0, "\nexpect: missing ] after ~[%c\n", op);
			erc = eExpectRespondFail;
			goto DID_NOT_GET_EXPECTED;
		}
	}

	ERDEBUG(1, "expect: <<%s>>\n", str);
	if (!strlen(str) || !strcmp(str, "\"\""))
		goto GOT_EXPECTED;

	if (!strcmp(str, "Speed"))
	{
		LRWT lr;
		long ms_start;
		long ms_now;
		struct TIMEB now_timeb;

		Ftime(&now_timeb);
		ms_start = (now_timeb.time * 1000) + now_timeb.millitm;
		do
		{
			last_Speed_result[0] = 0;
			lr.to1 = CFG_DialTimeout * 1000L;
			lr.to2 = 300L;
			/* allow interrupts + cooked read */
			lr.raw_flag = 0x80;
			lr.buffer = last_Speed_result;
			lr.bufsize = sizeof(last_Speed_result);
			lr.delim = "\n";
			lr.echo_flag = !!expresp_verbosity;
			lgets_timeout(&lr);
			Ftime(&now_timeb);
			ms_now = (now_timeb.time * 1000) + now_timeb.millitm;
		}
		while (!ck_sigint() && !lr.count &&
			((ms_now - ms_start) < CFG_DialTimeout * 1000L));

		if (ck_sigint() || strncmp(lr.buffer, "CONNECT", 7))
			goto DID_NOT_GET_EXPECTED;
		else
			goto GOT_EXPECTED;
	}

	cp = str;
	while (remaining && *cp)
	{
		if (*cp == '\\')
		{
			if (!*(++cp))	 /* if no character after escape, ... */
			{
				ERDEBUG(1, " error: str ended with '\\'\n", 0);
				goto DID_NOT_GET_EXPECTED;
			}

			if (isdigit((uchar) * cp))	/* handle \ooo */
			{
				strncpy(s8, cp, 3);
				s8[3] = 0;
				sscanf(s8, "%o", &itmp);
				cp += strspn(s8, "01234567");
				*parsed++ = (char)itmp;
				remaining--;
				continue;
			}

			switch (*cp)
			{
				case 'n':
					*parsed++ = 0x0A;
					remaining--;
					break;
				case 'r':
					*parsed++ = 0x0D;
					remaining--;
					break;
				case '\\':
					*parsed++ = '\\';
					remaining--;
					break;
				case '~':
					*parsed++ = '~';
					remaining--;
					break;
				default:
					ERDEBUG(0, " meaningless here: \\%c\n", *cp);
					break;
			}
			cp++;
		}
		else
		{
			*parsed++ = *cp++;
			remaining--;
		}
	}
	*parsed = 0;

	if (!remaining)
		ERDEBUG(0, " expect string too long\n", 0);

	if (expresp_verbosity >= 1)
		hex_dump(parsebuf, strlen(parsebuf), "expecting", 1);

	if (llookfor(parsebuf, expect_timeout_msecs, !!expresp_verbosity))
	{
	  GOT_EXPECTED:
		ERDEBUG(1, "[EXPECT SUCCEEDED]\n", 0);
		erc = 0;
		goto RESTORE_TTYMODE_AND_RETURN_ERC;

	}

  DID_NOT_GET_EXPECTED:
	ERDEBUG(1, "[EXPECT FAILED%s]\n", (ck_sigint())? " (interrupted)" : "");
	if (ck_sigint())
	{
		sigint = 0;
		erc = eCONINT;
	}
	else
		erc = eExpectRespondFail;
	goto RESTORE_TTYMODE_AND_RETURN_ERC;

  RESTORE_TTYMODE_AND_RETURN_ERC:
	if (old_ttymode != 2)
		ttymode(old_ttymode);
	return (erc);

}							 /* end of expect */

/*+-------------------------------------------------------------------------
	respond(str) - send to remote

we enable SIGINT processing in here and return if 'sigint'
detected, but here, unlike many other places, we do *not* reset
sigint (since we do not really "handle" it)
--------------------------------------------------------------------------*/
void
respond(str)
char *str;
{
	int itmp;
	long nap_msec;
	char s8[8];
	char *cp;
	char *phnum;
	char op;
	int send_no_cr = 0;
	int old_ttymode = get_ttymode();

	if (ck_sigint())
		return;

	ttymode(2);				 /* enable SIGINT/sigint */

	ERDEBUG(1, "respond: <<%s>>\n", str);
	while (*str)
	{
		if (*str == '\\')
		{
			if (isdigit((uchar) * ++str))	/* handle \ooo */
			{
				strncpy(s8, str, 3);
				s8[3] = 0;
				sscanf(s8, "%o", &itmp);
				str += strspn(s8, "01234567") - 1;	/* -1 because str++
													 * later */
				lputc((char)itmp);
			}
			else
				switch (*str)
				{
					case 'p':	/* pause (approximately 1/4-1/2 second
								 * delay) */
						ldraino(0);	/* wait for output to drain */
						if (Nap(400L) < 0)
							goto FUNC_RETURN;
						break;
					case 'M':	/* CLOCAL on */
					case 'm':	/* CLOCAL off */
						itmp = (*str == 'M');
						lCLOCAL(itmp);
						ERDEBUG(1, "CLOCAL set %s\n", (itmp) ? "ON" : "OFF");
						break;
					case 'd':	/* delay (2 seconds) */
						ldraino(0);	/* wait for output to drain */
						if (Nap(2000L) < 0)
							goto FUNC_RETURN;
						break;
					case 'D':	/* phone number/token */
						cp = strip_phone_num(shm->Ltelno);
						if (expresp_echo_check)
							lputs_paced(40, cp);
						else
							lputs(cp);
						break;
					case 'T':	/* phnum with Dialcodes and char
								 * translation */
						phnum = strip_phone_num(shm->Ltelno);
						cp = dialcodes_translate(&phnum);
						if (expresp_echo_check)
						{
							lputs_paced(40, cp);
							lputs_paced(40, phnum);
						}
						else
						{
							lputs(cp);
							lputs(phnum);
						}
						break;
					case 'N':	/* null byte */
						lputc(0);
						break;
					case 'K':	/* insert a BREAK */
						lbreak();
						break;
					case 'E':	/* turn on echo checking (for slow
								 * devices) */
						expresp_echo_check = 1;
						break;
					case 'e':	/* turn off echo checking */
						expresp_echo_check = 0;
						break;
					case 'r':	/* carriage return */
						lputc(0x0D);
						break;
					case 'c':	/* no new-line */
						send_no_cr = 1;
						break;
					case 'n':	/* send new-line */
						lputc(0x0A);
						break;
					case '\\':	/* send backslash */
						lputc('\\');
						break;
					case '~':	/* send tilde */
						lputc('~');
						break;
				}

		}
		else if ((*str == '~') && *(str + 1) && (*(str + 2) == '['))
		{
			op = *(str + 1);
			str += 3;
			switch (op)
			{

				case 'n':	 /* nap for milliseconds */
					nap_msec = atol(str);
					if (nap_msec < 0L)
						nap_msec = 0;
					if (nap_msec >= 500)
						ERDEBUG(1, "nap for %lu msec\n", nap_msec);
					Nap(nap_msec);
					break;

				default:
					ERDEBUG(0, "\nrespond: invalid subop: ~%c[]\n", op);
					break;

			}
			if (cp = strchr(str, ']'))
				str = cp + 1;
			else
			{
				ERDEBUG(0, "\nrespond: missing ] after ~[%c\n", op);
				goto FUNC_RETURN;
			}
		}
		else
			lputc(*str);

		if (expresp_echo_check)
		{
			ldraino(1);		 /* wait for output to drain, then flush input */
			Nap(40L);		 /* fake it */
		}
		str++;
	}

	if (!send_no_cr)
		lputc(0x0D);

  FUNC_RETURN:
	ttymode(old_ttymode);

}							 /* end of respond */

/*+-------------------------------------------------------------------------
	execute_expresp(expresp_script)

return 0 on success, else error code
--------------------------------------------------------------------------*/
int
execute_expresp(expresp_script)
char *expresp_script;
{
	char *fields[MAX_FIELDS + 1];
	int ifields;
	int nfields;
	int erc;
	char *expresp_copy = 0;
	char *expect_this;
	char *send_on_fail;

#define EXPECT_STATE (!(ifields & 1))	/* even numbered fields are expect */

	expresp_echo_check = 0;
	last_Speed_result[0] = 0;

	ERDEBUG(1, "[EXPECT/RESPOND INITIAL TIMEOUT %ld MSEC]\n",
		expect_timeout_msecs);

	/*
	 * dup script so we can poke holes in it with strtok()
	 * 
	 * should be expresp_copy = strdup(expresp_script), but this was added
	 * late in beta and I didn't know if NetBSD had strdup(); BSD 4.1 did
	 * not; but, the following hack is mea culpa -- wht w/no remorse
	 */
	if (!(expresp_copy = malloc(strlen(expresp_script) + 1)))
		return (eNoMemory);
	strcpy(expresp_copy, expresp_script);

	/*
	 * tokenize the script
	 */
	build_arg_array(expresp_copy, fields, MAX_FIELDS, &nfields);
	if (!nfields)			 /* if no script, assume success */
	{
		ERDEBUG(1, "[EMPTY SCRIPT - EXPECT/RESPOND SUCCEEDED]\n", 0);
		erc = 0;
		goto FUNC_RETURN;
	}

	/*
	 * work the tokens, starting with expect
	 */
	for (ifields = 0; ifields < nfields; ifields++)
	{
		if (ck_sigint())
			break;
		if (EXPECT_STATE)
		{
			expect_this = fields[ifields];
			while (1)		 /* until break or return(error) */
			{
				if (send_on_fail = strchr(expect_this, '-'))
					*send_on_fail++ = 0;
				if (!(erc = expect(expect_this)))
					goto OUT_OF_HERE; /* double break */
				if ((erc != eExpectRespondFail) || !send_on_fail)
				{
					ERDEBUG(1, "[EXPECT/RESPOND FAILED]\n", 0);
					erc = eExpectRespondFail;
					goto FUNC_RETURN;
				}
				if (expect_this = strchr(send_on_fail, '-'))
					*expect_this++ = 0;
				if (ck_sigint())
					break;
				respond(send_on_fail);
			}
		}
		else
			respond(fields[ifields]);
	}
OUT_OF_HERE: ;

	/*
	 * if console interrupt
	 */
	if (ck_sigint())
	{
		sigint = 0;
		ERDEBUG(1, "[CONSOLE INTERRUPT]\n", 0);
		erc = eCONINT;
		goto FUNC_RETURN;
	}

	/*
	 * wow
	 */
	ERDEBUG(1, "[EXPECT/RESPOND SUCCEEDED]\n", 0);
	erc = 0;

  FUNC_RETURN:
	if (expresp_copy)
		free(expresp_copy);
	return (erc);

}							 /* end of execute_expresp */

/*+-------------------------------------------------------------------------
	pcmd_expresp(param)
expresp [-v[v...]] <exp-resp-str> [<timeout_msecs>]
--------------------------------------------------------------------------*/
int
pcmd_expresp(param)
ESD *param;
{
	int erc;
	int itmp;
	char *cp;
	ESD *tesd;
	char switches[8];

	if (!(tesd = esdalloc(ESD_NOMSZ)))
		return (eNoMemory);

	get_switches(param, switches, sizeof(switches));

	if (erc = gstr(param, tesd, 0))
	{
		esdfree(tesd);
		return (erc);
	}

#define DEFAULT_TIMEOUT_MSECS (10*1000L)
	expect_timeout_msecs = DEFAULT_TIMEOUT_MSECS;
	if (!expresp_verbosity)
		expresp_verbosity = (!!strchr(switches, 'v')) || proc_trace;
	if (expresp_verbosity)
	{
		cp = switches;
		itmp = 0;
		while (*cp)
			itmp += (*cp++ == 'v');
		if (itmp > 1)
			expresp_verbosity = itmp;
	}

	if (erc = gint(param, &expect_timeout_msecs))
	{
		/* if something there non-integer */
		if (!end_of_cmd(param))
		{
			erc = eSyntaxError;
			goto FUNC_RETURN;
		}
	}

	erc = execute_expresp(tesd->pb);
  FUNC_RETURN:
	esdfree(tesd);
	iv[0] = !!erc;
	if (proc_trace)
		pprintf("$i00 = %7ld (0x%08lx,0%lo)\n", iv[0], iv[0], iv[0]);
	if (erc == eExpectRespondFail)
		erc = 0;
	return (erc);

}							 /* end of pcmd_expresp */

/* vi: set tabstop=4 shiftwidth=4: */
/* end of expresp.c */
