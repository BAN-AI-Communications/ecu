/* #define NONANSI_DEBUG */
/*+-----------------------------------------------------------------
	funckeymap.c - keyboard function key -> ECU internal
	wht@wht.net

  Defined functions:
	fkmap_command(argc, argv)
	funckeymap(buf, buflen)
	funckeymap_define(bufptr)
	funckeymap_display(fp)
	funckeymap_display_single(ikde, fp)
	funckeymap_init()
	funckeymap_read(name)

------------------------------------------------------------------*/
/*+:EDITS:*/
/*:04-26-2000-11:15-wht@bob-RELEASE 4.42 */
/*:01-24-1997-02:37-wht@yuriatin-SOURCE RELEASE 4.00 */
/*:09-11-1996-20:00-wht@yuriatin-3.48-major telnet,curses,structural overhaul */
/*:12-06-1995-13:30-wht@n4hgf-termecu w/errno -1 consideration */
/*:11-23-1995-11:20-wht@kepler-source control 3.37 for tsx-11 */
/*:11-14-1995-10:23-wht@kepler-3.37.80-source control point: SOCKETS */
/*:11-06-1995-17:27-wht@kepler-entry name comments fell out? */
/*:11-03-1995-18:11-wht@wwtp1-icc found unreported bug in fkmap_command */
/*:10-21-1995-11:36-wht@wwtp1-now entry names can have comments */
/*:01-12-1995-15:20-wht@n4hgf-apply Andrew Chernov 8-bit clean+FreeBSD patch */
/*:05-04-1994-04:39-wht@n4hgf-ECU release 3.30 */
/*:03-27-1993-17:48-wht@n4hgf-SVR4 found mk_char_graphic decl out of scope */
/*:09-16-1992-13:33-wht@n4hgf-add fkmap -l name */
/*:09-10-1992-13:59-wht@n4hgf-ECU release 3.20 */
/*:08-30-1992-23:06-wht@n4hgf-add fkmap_command */
/*:08-22-1992-15:38-wht@n4hgf-ECU release 3.20 BETA */
/*:08-28-1991-14:51-wht@n4hgf2-look correctly for funckeymap in CFG_EcuLibDir */
/*:08-26-1991-05:45-wht@n4hgf2-# got included in key def */
/*:08-06-1991-13:19-wht@n4hgf-allow any code as first in key sequence */
/*:08-03-1991-14:44-wht@n4hgf-look for funckeymap in CFG_EcuLibDir too */
/*:07-25-1991-12:58-wht@n4hgf-ECU release 3.10 */
/*:03-20-1991-03:06-root@n4hgf-no Metro Link problems here */
/*:03-20-1991-01:04-root@n4hgf-diagnose Metro Link xterm differences */
/*:01-10-1991-23:15-wht@n4hgf-string overflow rptd by spooley@compulink.co.uk */
/*:12-01-1990-12:51-wht@n4hgf-creation, borrowing from and using ecufkey.c */

#include "ecu.h"
#include "ecuerror.h"
#include "ecukey.h"
#include "ecufkey.h"
#include "ecuxkey.h"
#include "ecufork.h"

extern int tty_not_char_special;
extern char *dash_f_funckeytype;

KDE funckeymap_table[KDE_COUNT];
char funckeymap_name[32] = "";

#if defined(NONANSI_DEBUG)
static FILE *nadbg = (FILE *) 0;

#endif

/*+-------------------------------------------------------------------------
	funckeymap_init() - initialize function key mapping (recognition)
--------------------------------------------------------------------------*/
void
funckeymap_init()
{
	int itmp;
	KDE *tkde;

	for (itmp = 0; itmp < KDE_COUNT; itmp++)
	{
		tkde = &funckeymap_table[itmp];
		tkde->logical[0] = 0;
		tkde->count = 0;
		tkde->ikde = 0;
	}

	funckeymap_name[0] = 0;

}							 /* end of funckeymap_init */

/*+-------------------------------------------------------------------------
	funckeymap_define(bufptr) - use funckeymap line to define a mapping

  returns 0 good keydef
         -1 if syntax error
--------------------------------------------------------------------------*/
int
funckeymap_define(bufptr)
char *bufptr;
{
	int itmp;
	int token_number = 0;
	KDE *tkde = (KDE *) 0;
	int ikde = 0;
	int octothorpe = 0;
	char *token;
	char *arg_token();
	char *str_token();
	char *skip_ld_break();

	while (!octothorpe &&
		(token = (token_number < 2) ? str_token(bufptr, ":")
			: arg_token(bufptr, " \t")))
	{
		bufptr = (char *)0;	 /* further calls to arg_token need NULL */
		token = skip_ld_break(token);
		strip_trail_break(token);

		switch (token_number)
		{
			case 0:		 /* first field is key identifier */
				if ((ikde = kde_name_to_ikde(token)) < 0)
				{
					pprintf("  %s is not a legal key identifier\r\n", token);
					return (-1);
				}
				tkde = &funckeymap_table[ikde];
				tkde->logical[0] = 0;
				tkde->count = 0;
				tkde->ikde = ikde;
				break;

			case 1:		 /* second field is logical key name */
				if (*token == '#')
					goto MISSING_LABEL;
				strncpy(tkde->logical, token, sizeof(tkde->logical));
				tkde->logical[sizeof(tkde->logical) - 1] = 0;
				break;

			case 2:		 /* third field is first token of sequence */
				if (*token == '#')
					goto MISSING_SEQUENCE;

			default:		 /* third and subsequent to define key */
				if (*token == '#')
				{
					octothorpe = 1;
					break;
				}
				if (tkde->count == sizeof(tkde->str))
				{
					pprintf("  %s: output count too long",
						keyset_idstr(ikde));
					return (-1);
				}
				if ((itmp = ascii_to_hex(token)) < 0)
				{
					pprintf("  %s: '%s' invalid\r\n",
						keyset_idstr(ikde), token);
					return (-1);
				}
				tkde->str[tkde->count] = itmp;
				tkde->count++;
				break;
		}					 /* end of switch(token_number) */

		if (octothorpe)
			break;

		token_number++;

	}						 /* end while not end of record */

	switch (token_number)
	{
		case 0:
			pprintf("funckeymap_define logic error\n");
			errno = -1;
			termecu(TERMECU_LOGIC_ERROR);
			break;

		case 1:
		  MISSING_LABEL:
			pprintf("%s: missing key label\r\n", keyset_idstr(ikde));
			break;

		case 2:
		  MISSING_SEQUENCE:
			pprintf("%s: missing char sequence\r\n", keyset_idstr(ikde));
			break;
		default:
			/* special init string entry */
			if (ikde == IKDE_InitStr)
				fwrite(tkde->str, 1, tkde->count, stderr);
			else if (tkde->count)
			{
				uchar ch = tkde->str[0];
				extern uchar kbdeof;	/* current input EOF */
				extern uchar kbdeol2;	/* current secondary input EOL */
				extern uchar kbdeol;	/* current input EOL */
				extern uchar kbderase;	/* current input ERASE */
				extern uchar kbdintr;	/* current input INTR */
				extern uchar kbdkill;	/* current input KILL */
				extern uchar kbdquit;	/* current input QUIT */

				if ((ch == kbdeof) || (ch == kbdeol2) ||
					(ch == kbdeol) || (ch == kbderase) ||
					(ch == kbdintr) || (ch == kbdkill) ||
					(ch == kbdquit))
				{
					pprintf(
						"%s: 1st char cannot be input control character\r\n",
						keyset_idstr(ikde));
					break;
				}
			}
			return (0);
	}

	return (-1);			 /* error */

}							 /* end of funckeymap_define */

/*+-------------------------------------------------------------------------
	funckeymap_read(name) - read key-sequence-to-fkey map from funckeymap
--------------------------------------------------------------------------*/
void
funckeymap_read(name)
char *name;
{
	int itmp;
	char buf[128];
	FILE *fp_keys;
	int errstat = 0;
	static char ecukeys_name[128];

#if defined(NONANSI_DEBUG)
	if (!nadbg)
	{
		nadbg = fopen("/tmp/nadbg.log", "w");
		setbuf(nadbg, NULL);
	}
#endif

	funckeymap_init();		 /* clear any previous key defns */

	if (!ecukeys_name[0])
	{
		get_home_dir(ecukeys_name);
		strcat(ecukeys_name, "/.ecu/funckeymap");
	}

	if (!(fp_keys = fopen(ecukeys_name, "r")))
	{
		strcpy(ecukeys_name, eculibdir);
		strcat(ecukeys_name, "/funckeymap");
		if (!(fp_keys = fopen(ecukeys_name, "r")))
		{
			ff(stderr, "'funckeymap' not in ~/.ecu or %s; cannot proceed\r\n",
				eculibdir);
			termecu(TERMECU_UNRECOVERABLE);
		}
	}

/* find funckeymap name */
	errstat = 1;
	while ((itmp = kde_fgets(buf, sizeof(buf), fp_keys)) != KDETYPE_EOF)
	{
		char *cptr;

		if ((itmp == KDETYPE_COMMENT) || (itmp == KDETYPE_ENTRY))
			continue;
		if (cptr = strchr(buf, '#'))
			*cptr = 0;
		if (cptr = strchr(buf, ' '))
			*cptr = 0;
		if (cptr = strchr(buf, '\t'))
			*cptr = 0;
		if (!strcmp(buf, name))
		{
			errstat = 0;	 /* indicate success */
			break;
		}
	}
	if (errstat)
	{
		ff(stderr, "terminal type '%s'\r\n", name);
		ff(stderr, "not found in %s; unable to proceed\r\n", ecukeys_name);
		errno = -1;
		termecu(TERMECU_UNRECOVERABLE);
	}

/* read past any other funckeymap names matching this set */
	errstat = 1;
	while ((itmp = kde_fgets(buf, sizeof(buf), fp_keys)) != KDETYPE_EOF)
	{
		if (itmp == KDETYPE_ENTRY)
		{
			errstat = 0;	 /* indicate success */
			break;
		}
	}
	if (errstat)
	{
		ff(stderr,
			"terminal type '%s' has null entry in %s; unable to proceed\r\n",
			name, ecukeys_name);
		errno = -1;
		termecu(TERMECU_UNRECOVERABLE);
	}

/* we found the definition ... process it */
	errstat = 0;
	itmp = KDETYPE_ENTRY;
	do
	{
		if (itmp == KDETYPE_NAME)
			break;
		else if (itmp == KDETYPE_ENTRY)
		{
			if (funckeymap_define(buf))
				errstat = 1;
		}
	}
	while ((itmp = kde_fgets(buf, sizeof(buf), fp_keys)) != KDETYPE_EOF);

/* finish up */
	strncpy(funckeymap_name, name, sizeof(funckeymap_name));
	funckeymap_name[sizeof(funckeymap_name) - 1] = 0;
	fclose(fp_keys);

	if (!funckeymap_table[IKDE_HOME].count)
	{
		ff(stderr, "You MUST have a 'Home' key defined\r\n");
		errstat = 2;
	}
	if (!funckeymap_table[IKDE_END].count)
	{
		ff(stderr, "You MUST have a 'End' key defined\r\n");
		errstat = 2;
	}
	if ((errstat == 2) || (errstat && tty_not_char_special))
	{
		errno = -1;
		termecu(TERMECU_UNRECOVERABLE);
	}

	if (errstat)
	{
		ff(stderr,
			"WARNING: key mapping syntax errors\r\nContinue anyway (y,[n])? ");
		if ((itmp = ttygetc(0)) == 'Y' || (itmp == 'y'))
		{
			ff(stderr, "YES\r\n");
			return;
		}
		ff(stderr, "NO\r\n");
		errno = -1;
		termecu(TERMECU_UNRECOVERABLE);
	}

}							 /* end of funckeymap_read */

/*+-------------------------------------------------------------------------
	funckeymap(buf,buflen) - map char sequence to ikde code

return XF_ code or XF_not_yet if no match yet, XF_no_way if no match possible
--------------------------------------------------------------------------*/
UINT
funckeymap(buf, buflen)
uchar *buf;
int buflen;
{
	int ikde;
	KDE *tkde;
	int err_rtn = XF_no_way;

	if (!buflen)
		return (XF_not_yet);

#if defined(NONANSI_DEBUG)
	if (nadbg)
		hex_dump_fp(nadbg, buf, -buflen, "mapna", 1);
#endif

	for (ikde = 0, tkde = funckeymap_table; ikde <= IKDE_lastKey;
		ikde++, tkde++)
	{
#if defined(NONANSI_DEBUG)
		if (nadbg)
			fprintf(nadbg, "--> %s ", tkde->logical);
#endif
		if ((tkde->count == buflen) && !memcmp(tkde->str, buf, buflen))
		{
#if defined(NONANSI_DEBUG)
			if (nadbg)
				fprintf(nadbg, "yes\n");
#endif
			return (tkde->ikde);
		}
#if defined(NONANSI_DEBUG)
		if (nadbg)
			fprintf(nadbg, "no\n");
#endif
		if ((tkde->count > buflen) &&
			((uchar) * (tkde->str + buflen) == *(buf + buflen)))
		{
			err_rtn = XF_not_yet;
		}
	}
	return (err_rtn);
}							 /* end of funckeymap */

/*+-------------------------------------------------------------------------
	funckeymap_display_single(tkde,fp) - display single mapping on FILE fp
--------------------------------------------------------------------------*/
void
funckeymap_display_single(ikde, fp)
int ikde;
FILE *fp;
{
	int keys_left;
	char *keys;
	char s64[64];
	KDE *tkde;

	if ((unsigned)ikde > IKDE_lastKey)
		return;

	tkde = &funckeymap_table[ikde];
	sprintf(s64, " %s:%s:                   ",
		keyset_idstr(ikde), tkde->logical);
	s64[16] = 0;
	if (fp == stderr)
		pputs(s64);
	else
		fputs(s64, fp);
	keys_left = tkde->count;
	keys = tkde->str;
	while (keys_left--)
	{
		if (fp == stderr)
		{
			pprintf("%s%s",
				graphic_char_text(*keys++, 0),
				(keys_left) ? " " : "");
		}
		else
		{
			fprintf(fp, "%s%s",
				graphic_char_text(*keys++, 0),
				(keys_left) ? " " : "");
		}
	}
	if (fp == stderr)
		pputs("\n");
	else
		fputs("\n", fp);

}							 /* end of funckeymap_display_single */

/*+-------------------------------------------------------------------------
	funckeymap_display(fp) - display function key table of FILE fp
--------------------------------------------------------------------------*/
void
funckeymap_display(fp)
FILE *fp;
{
	int ikde;
	char *ftype = 0;

	if (dash_f_funckeytype)
		ftype = dash_f_funckeytype;
	else
		ftype = getenv("ECUFUNCKEY");

	if (ttype && ftype)
	{
		fprintf(fp, "#$TERM=%s -F/$ECUFUNCKEY=%s", ftype, ttype);
		if (fp == stderr)
			fputs("\r\n", fp);
		else
			fputs("\n", fp);
	}

	if (ttype || ftype)
	{
		fputs((ftype) ? ftype : ttype, fp);
		if (fp == stderr)
			fputs("\r\n", fp);
		else
			fputs("\n", fp);
	}

	for (ikde = 0; ikde <= IKDE_lastKey; ikde++)
		funckeymap_display_single(ikde, fp);

}							 /* end of funckeymap_display */

/*+-------------------------------------------------------------------------
	fkmap_command(argc,argv) - common interactive and procedure 'fkmap' cmd

return procedure error codes
--------------------------------------------------------------------------*/
int
fkmap_command(argc, argv)
int argc;
char **argv;
{
	int itmp;
	int err = 0;
	char *ftype = 0;
	int iargv = 1;
	char *arg;
	char fkcmd[512 + 1];
	char *fkcptr = fkcmd;
	int fkclen = 0;
	int ikde;
	KDE save;
	FILE *fp;

	while ((iargv < argc) && (*(arg = argv[iargv]) == '-'))
	{
		switch (*++arg)
		{
			case 'r':		 /* reset */
				if (err)
					break;
				if (iargv != (argc - 1))
				{
					pputs("no arguments allowed for -r\n");
					err = 1;
					break;
				}
				if (dash_f_funckeytype)
					ftype = dash_f_funckeytype;
				else
					ftype = getenv("ECUFUNCKEY");
				if (ttype || ftype)
					funckeymap_read((ftype) ? ftype : ttype);
				if (!proc_level || proc_trace)
					pputs("funckeymap reset to startup configuration\n");
				return (0);

			case 's':		 /* save in file */
				if (err)
					break;
				if (iargv != (argc - 2))
				{
					pputs("exactly one argument required for -s\n");
					err = 1;
					break;
				}
				iargv++;
				if (!(fp = fopen(argv[iargv], "a")))
				{
					pperror(argv[iargv]);
					return (eFATAL_ALREADY);
				}
				funckeymap_display(fp);
				fclose(fp);
				if (!proc_level || proc_trace)
					pprintf("current mapping saved in %s\n", argv[iargv]);
				return (0);

			case 'l':		 /* load entire */
				if (err)
					break;
				if (iargv != (argc - 2))
				{
					pputs("exactly one argument required for -l\n");
					err = 1;
					break;
				}
				iargv++;
				funckeymap_read(argv[iargv]);
				return (0);

			default:
				pprintf("unknown switch -%c\n", *arg);
				err = 1;
		}
		iargv++;
	}

	if (err)
	{
		fkmap_cmd_usage();
		return (eFATAL_ALREADY);
	}

	if (iargv == argc)
	{
		funckeymap_display(stderr);
		return (0);
	}

	arg = argv[iargv++];
	if ((ikde = kde_name_to_ikde(arg)) < 0)
	{
		pprintf("key name '%s' not recognized\n", arg);
		return (eFATAL_ALREADY);
	}
	sprintf(fkcptr, " %s:%s: ", keyset_idstr(ikde), keyset_idstr(ikde));
	fkcptr += (itmp = strlen(fkcptr));
	fkclen += itmp;

	if (iargv == argc)
	{
		funckeymap_display_single(ikde, stderr);
		return (0);
	}

	while (iargv < argc)
	{
		arg = argv[iargv++];
		itmp = strlen(arg);
		if ((unsigned)(fkclen + itmp + 2) > (unsigned)sizeof(fkcmd))
		{
			pprintf("fkmap command may be no longer than %d characters\n",
				sizeof(fkcmd) - 1);
			return (eFATAL_ALREADY);
		}
		strcpy(fkcptr, arg);
		fkcptr += itmp;
		fkclen += itmp;
		if (iargv != argc)
		{
			*fkcptr++ = ' ';
			*fkcptr = 0;
			fkclen++;
		}
	}

	save = funckeymap_table[ikde];
	if (err = funckeymap_define(fkcmd))
		funckeymap_table[ikde] = save;

	if (!err && (!proc_level || proc_trace))
		funckeymap_display_single(ikde, stderr);

	return ((err) ? eFATAL_ALREADY : 0);

}							 /* end of fkmap_command */

/* end of funckeymap.c */
/* vi: set tabstop=4 shiftwidth=4: */
