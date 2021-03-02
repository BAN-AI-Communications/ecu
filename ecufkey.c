/*+-----------------------------------------------------------------
	ecufkey.c -- function key definition
	wht@wht.net

  Defined functions:
	ffso(str)
	ikde_to_xf(ikde)
	kde_fgets(buf, bufsize, fp)
	kde_name_to_ikde(keystr)
	kde_text(ikde)
	keyset_define_key(bufptr)
	keyset_display()
	keyset_idstr(ikde)
	keyset_init()
	keyset_read(name)
	xf_text(xf)
	xf_to_ikde(xf)

------------------------------------------------------------------*/
/*+:EDITS:*/
/*:04-26-2000-11:15-wht@bob-RELEASE 4.42 */
/*:01-24-1997-02:37-wht@yuriatin-SOURCE RELEASE 4.00 */
/*:09-11-1996-20:00-wht@yuriatin-3.48-major telnet,curses,structural overhaul */
/*:11-23-1995-11:20-wht@kepler-source control 3.37 for tsx-11 */
/*:11-14-1995-10:23-wht@kepler-3.37.80-source control point: SOCKETS */
/*:03-12-1995-03:27-wht@kepler-use ECU_MAXPN */
/*:01-12-1995-15:19-wht@n4hgf-apply Andrew Chernov 8-bit clean+FreeBSD patch */
/*:05-04-1994-04:38-wht@n4hgf-ECU release 3.30 */
/*:01-30-1993-12:17-wht@n4hgf-remove gcc < 1.40 bug workaround */
/*:01-11-1993-15:42-wht@n4hgf-declare skip_ld_break */
/*:01-01-1993-12:52-wht@n4hgf-add procedure binding for function keys */
/*:09-10-1992-13:58-wht@n4hgf-ECU release 3.20 */
/*:08-22-1992-15:38-wht@n4hgf-ECU release 3.20 BETA */
/*:04-17-1992-16:29-wht@n4hgf-initialize keyset to SCO "ANSI" send strings */
/*:09-01-1991-05:16-wht@n4hgf2-allow comments and whitespace after names */
/*:08-31-1991-13:21-wht@n4hgf2-look for keys file in CFG_EcuLibDir */
/*:08-16-1991-00:11-wht@n4hgf-keyset_init loads default keyset if found */
/*:07-25-1991-12:55-wht@n4hgf-ECU release 3.10 */
/*:07-12-1991-13:57-wht@n4hgf-GCC140 fix update */
/*:05-21-1991-00:45-wht@n4hgf-added -3 error code to keyset_read */
/*:05-21-1991-00:37-wht@n4hgf-improve fkey load error detection */
/*:05-16-1991-15:05-wht@n4hgf-gcc binary exploded in keyset_display */
/*:08-14-1990-20:40-wht@n4hgf-ecu3.00-flush old edit history */

#include "ecu.h"
#include "ecukey.h"
#include "ecufkey.h"
#include "ecuxkey.h"
#include "ecufork.h"

char *str_token();
char *skip_ld_break();

extern char kbdintr;		 /* current input INTR */
extern char curr_dir[ECU_MAXPN];	/* current working key defns */

KDE keyset_table[KDE_COUNT];
char keyset_name[256] = "";

KDEMAP kdemap[] =
{
	{XFcurup, IKDE_CUU, "CUU", "\033[A"},
	{XFcurdn, IKDE_CUD, "CUD", "\033[B"},
	{XFcurrt, IKDE_CUR, "CUR", "\033[C"},
	{XFcurlf, IKDE_CUL, "CUL", "\033[D"},
	{XFcur5, IKDE_CU5, "CU5", "\033[E"},
	{XFend, IKDE_END, "End", "\033[F"},
	{XFpgdn, IKDE_PGDN, "PgDn", "\033[G"},
	{XFhome, IKDE_HOME, "Home", "\033[H"},
	{XFpgup, IKDE_PGUP, "PgUp", "\033[I"},
	{XFins, IKDE_INS, "Ins", "\033[L"},
	{XF1, IKDE_F1, "F1", "\033[M"},
	{XF2, IKDE_F2, "F2", "\033[N"},
	{XF3, IKDE_F3, "F3", "\033[O"},
	{XF4, IKDE_F4, "F4", "\033[P"},
	{XF5, IKDE_F5, "F5", "\033[Q"},
	{XF6, IKDE_F6, "F6", "\033[R"},
	{XF7, IKDE_F7, "F7", "\033[S"},
	{XF8, IKDE_F8, "F8", "\033[T"},
	{XF9, IKDE_F9, "F9", "\033[U"},
	{XF10, IKDE_F10, "F10", "\033[V"},
	{XF11, IKDE_F11, "F11", "\033[W"},
	{XF12, IKDE_F12, "F12", "\033[X"},
	{XFbktab, IKDE_BKTAB, "BkTab", "\033[Z"},
	{XF_not_yet, IKDE_InitStr, "IS", ""},
	{0, 0, "", ""}
};

/*+-------------------------------------------------------------------------
	keyset_init()
--------------------------------------------------------------------------*/
void
keyset_init()
{
	int itmp;
	KDE *tkde;
	KDEMAP *kmap;

	for (itmp = 0; itmp < KDE_COUNT; itmp++)
	{
		tkde = &keyset_table[itmp];
		tkde->logical[0] = 0;
		tkde->ikde = (uchar) itmp;
	}

	kmap = kdemap;
	while (kmap->xf)
	{
		if (kmap->ikde <= IKDE_lastKey)
		{
			tkde = &keyset_table[kmap->ikde];
			sprintf(tkde->logical, "SCO %s", kmap->name);
			strcpy(tkde->str, kmap->init);
			tkde->count = strlen(tkde->str);
		}
		kmap++;
	}

	keyset_name[0] = 0;

	tkde = &keyset_table[IKDE_BKTAB];
	strcpy(tkde->logical, "redisplay");
	tkde->count = KACT_REDISPLAY;

	tkde = &keyset_table[IKDE_HOME];
	strcpy(tkde->logical, "ecu cmd");
	tkde->count = KACT_COMMAND;

	tkde = &keyset_table[IKDE_INS];
	strcpy(tkde->logical, "local shell");
	tkde->count = KACT_LOCAL_SHELL;

	tkde = &keyset_table[IKDE_CU5];
	strcpy(tkde->logical, "screen dump");
	tkde->str[0] = 0x7F;	 /* this key is intercepted by kbd read
							  * routine */
	tkde->count = 0;

}							 /* end of keyset_init */

/*+-------------------------------------------------------------------------
	kde_fgets(buf,bufsize,fp) - read and evaluate key file line

Returns:
    KDETYPE_COMMENT     comment or blank line (null, all blank or "#"
                        as non-blank)
    KDETYPE_NAME        "name" (non "#\t " in column 1)
    KDETYPE_ENTRY       "entry" ("\t " in column 1)
    KDETYPE_EOF         end of file
--------------------------------------------------------------------------*/
int
kde_fgets(buf, bufsize, fp)
char *buf;
int bufsize;
FILE *fp;
{
	int itmp;
	char *cp;

	if (!fgets(buf, bufsize, fp))
	{
		return (KDETYPE_EOF);
	}

	if (!(itmp = strlen(buf)))
		return (KDETYPE_COMMENT);
	if (buf[itmp - 1] == NL)
	{
		buf[itmp - 1] = 0;
		itmp--;
	}
	if (!itmp)
		return (KDETYPE_COMMENT);

	cp = buf;
	itmp = (strchr(" \t", *cp)) ? KDETYPE_ENTRY : KDETYPE_NAME;
	while (*cp && ((*cp == SPACE) || (*cp == TAB)))
		cp++;

	if (!*cp || (*cp == '#'))
		return (KDETYPE_COMMENT);

	return (itmp);

}							 /* end of kde_fgets */

/*+-------------------------------------------------------------------------
	kde_name_to_ikde(keystr)
--------------------------------------------------------------------------*/
int
kde_name_to_ikde(keystr)
char *keystr;
{
	KDEMAP *kmap = kdemap;

	while (kmap->xf != 0)
	{
		if (!strcmpi(kmap->name, keystr))
			return ((int)kmap->ikde);
		kmap++;
	}
	return (-1);
}							 /* end of kde_name_to_ikde */

/*+-------------------------------------------------------------------------
	keyset_idstr(ikde)
--------------------------------------------------------------------------*/
char *
keyset_idstr(ikde)
int ikde;
{
	KDEMAP *kmap = kdemap;

	while (kmap->xf)
	{
		if ((int)kmap->ikde == ikde)
			return (kmap->name);
		kmap++;
	}
	return ((char *)0);
}							 /* end of keyset_idstr */

/*+-------------------------------------------------------------------------
	xf_to_ikde(xf)
--------------------------------------------------------------------------*/
int
xf_to_ikde(xf)
UINT xf;
{
	KDEMAP *kmap = kdemap;

	while (kmap->xf)
	{
		if (kmap->xf == xf)
			return ((int)kmap->ikde & 0xFF);
		kmap++;
	}
	return (-1);
}							 /* end of xf_to_ikde */

/*+-------------------------------------------------------------------------
	ikde_to_xf(ikde)
--------------------------------------------------------------------------*/
int
ikde_to_xf(ikde)
char ikde;
{
	KDEMAP *kmap = kdemap;

	while (kmap->xf)
	{
		if (kmap->ikde == ikde)
			return (kmap->xf);
		kmap++;
	}
	return (-1);
}							 /* end of ikde_to_xf */

/*+-------------------------------------------------------------------------
	kde_text(ikde) - ikde value to text
--------------------------------------------------------------------------*/
char *
kde_text(ikde)
int ikde;
{
	KDEMAP *kmap = kdemap;

	while (kmap->xf)
	{
		if (kmap->ikde == (char)ikde)
			return (kmap->name);
		kmap++;
	}
	return ("??");
}							 /* end of xf_to_ikde */

/*+-------------------------------------------------------------------------
	xf_text(xf) - XFkey to text
--------------------------------------------------------------------------*/
char *
xf_text(xf)
UINT xf;
{
	static char sc8[8];

	switch (xf)
	{
		case XFcurup:
			return ("CUU");
		case XFcurdn:
			return ("CUD");
		case XFcurrt:
			return ("CUR");
		case XFcurlf:
			return ("CUL");
		case XFcur5:
			return ("CU5");
		case XFend:
			return ("End");
		case XFpgdn:
			return ("PgDn");
		case XFhome:
			return ("Home");
		case XFpgup:
			return ("PgUp");
		case XFins:
			return ("Ins");
		case XF1:
			return ("F1");
		case XF2:
			return ("F2");
		case XF3:
			return ("F3");
		case XF4:
			return ("F4");
		case XF5:
			return ("F5");
		case XF6:
			return ("F6");
		case XF7:
			return ("F7");
		case XF8:
			return ("F8");
		case XF9:
			return ("F9");
		case XF10:
			return ("F10");
		case XF11:
			return ("F11");
		case XF12:
			return ("F12");
		case XFbktab:
			return ("BkTab");
	}

	if ((xf >= XF_ALTA) && (xf <= XF_ALTZ))
	{
		sprintf(sc8, "Alt-%c", 'a' + xf - XF_ALTA);
		return (sc8);
	}
	sprintf(sc8, "XF_%02x\n", xf);
	return (sc8);
}							 /* end of xf_text */

/*+-------------------------------------------------------------------------
	keyset_define_key(bufptr)

return 0 if no error, -1 if error
--------------------------------------------------------------------------*/
int
keyset_define_key(bufptr)
char *bufptr;
{
	int itmp;
	int token_number;
	KDE *tkde = (KDE *) 0;
	int ikde = 0;
	char token_separator[8];
	char *token;
	char *syntax = "syntax error in key definition: %s\n";

	if ((itmp = strlen(bufptr)) && (bufptr[itmp - 1] == NL))
		bufptr[--itmp] = 0;	 /* strip trailing NL */
	if (!itmp)
		return (0);

	if ((*bufptr != SPACE) && (*bufptr != TAB))	/* if no leading space */
		return (0);

	while ((*bufptr == SPACE) || (*bufptr == TAB))	/* strip lding sp/tab */
		bufptr++;

	token_number = 0;
	strcpy(token_separator, ":");
	while (token = str_token(bufptr, token_separator))
	{
		bufptr = (char *)0;	 /* further calls to str_token need NULL */
		switch (token_number)
		{
			case 0:		 /* first field is key identifier */
				if ((ikde = kde_name_to_ikde(token)) < 0)
				{
					pprintf(syntax, keyset_name);
					pprintf("  %s is not a legal key identifier\n", token);
					return (-1);
				}
				if (ikde == IKDE_HOME)
				{
					pprintf(syntax, keyset_name);
					pprintf("  HOME cannot be redefined!\n");
					return (-1);
				}
				if (ikde == IKDE_CU5)
				{
					pprintf(syntax, keyset_name);
					pprintf("  CUR5 cannot be redefined!\n");
					return (-1);
				}
				if (ikde == IKDE_BKTAB)
				{
					pprintf(syntax, keyset_name);
					pprintf("  BkTab cannot be redefined!\n");
					return (-1);
				}
				tkde = &keyset_table[ikde];
				tkde->logical[0] = 0;
				tkde->count = 0;
				break;

			case 1:		 /* second field is logical key name */
				strncpy(tkde->logical, token, sizeof(tkde->logical));
				tkde->logical[sizeof(tkde->logical) - 1] = 0;
				strcpy(token_separator, " \t");	/* whitespace is tok sep
												 * now */
				break;

			case 2:
				if (!strcmp(token, "proc"))
				{
					token = skip_ld_break(token + 5);
					strncpy(tkde->str, token, sizeof(tkde->str));
					tkde->str[sizeof(tkde->str) - 1] = 0;
					tkde->count = KACT_PROC;
					return (0);	/* <<=========================== */
				}
			default:		 /* third and subsequent to define key */
				if (!strlen(token))
					continue;
				if (tkde->count == sizeof(tkde->str))
				{
					pprintf(syntax, keyset_name);
					pprintf("  %s: output count too long",
						keyset_idstr(ikde));
					return (-1);
				}
				if ((itmp = ascii_to_hex(token)) < 0)
				{
					pprintf(syntax, keyset_name);
					pprintf("  %s: '%s' invalid code\n",
						keyset_idstr(ikde), token);
					return (0);
				}
				tkde->str[tkde->count] = itmp;
				tkde->count++;
				break;
		}					 /* end of switch(token_number) */
		token_number++;
	}						 /* end while not end of record */

	return (0);

}							 /* end of keyset_define_key */

/*+-------------------------------------------------------------------------
	keyset_read(name)
returns 0 on success, -1 if no .ecu/keys, -2 if no 'name', -3 if error
--------------------------------------------------------------------------*/
int
keyset_read(name)
char *name;
{
	int itmp;
	int ikde;
	char ecukeys_name[128];
	char s128[128];
	FILE *fp_keys;

	get_home_dir(ecukeys_name);
	strcat(ecukeys_name, "/.ecu/keys");	/* someone may core dump here one
										 * day */

	if (!(fp_keys = fopen(ecukeys_name, "r")))
	{
		strcpy(ecukeys_name, eculibdir);
		strcat(ecukeys_name, "/keys");
		if (!(fp_keys = fopen(ecukeys_name, "r")))
			return (-1);
	}

/* find keyset name */
	itmp = 0;
	while ((ikde = kde_fgets(s128, sizeof(s128), fp_keys)) != KDETYPE_EOF)
	{
		if ((ikde == KDETYPE_NAME) && !strcmp(s128, name))
		{
			itmp = 1;		 /* success */
			break;
		}
	}
	if (!itmp)				 /* find match? */
	{
		fclose(fp_keys);	 /* nope */
		return (-2);
	}

/*
 * read past any other keyset names matching this set
 * process 1st line of definition when found
 */
	memset(s128, 0, sizeof(s128));
	while ((ikde = kde_fgets(s128, sizeof(s128), fp_keys)) != KDETYPE_EOF)
	{
		if (ikde == KDETYPE_ENTRY)
		{
			if (keyset_define_key(s128) < 0)
			{
				fclose(fp_keys);
				keyset_init();
				return (-3);
			}
			break;
		}
		memset(s128, 0, sizeof(s128));
	}

/*
 * read rest of definition
 */
	while ((ikde = kde_fgets(s128, sizeof(s128), fp_keys)) != KDETYPE_EOF)
	{
		switch (ikde)
		{
			case KDETYPE_COMMENT:
				continue;
			case KDETYPE_ENTRY:
				if (keyset_define_key(s128) < 0)
				{
					fclose(fp_keys);
					keyset_init();
					return (-3);
				}
				break;
			default:
				goto DONE;
		}
	}

  DONE:
	strncpy(keyset_name, name, sizeof(keyset_name));
	keyset_name[sizeof(keyset_name) - 1] = 0;
	fclose(fp_keys);
	return (0);
}							 /* end of keyset_read */

/*+-------------------------------------------------------------------------
	ffso(str)
--------------------------------------------------------------------------*/
void
ffso(str)
char *str;
{
	tcap_stand_out();
	fputs(str, se);
	tcap_stand_end();
}							 /* end of ffso */

/*+-------------------------------------------------------------------------
	keyset_display()

 F1  xxxxx  F2   xxxxx   HOME xxxxx  PGUP xxxxx
 F3  xxxxx  F4   xxxxx   END  xxxxx  PGDN xxxxx
 F5  xxxxx  F6   xxxxx   INS  xxxxx  CUR5 xxxxx
 F7  xxxxx  F8   xxxxx   BkTab xxxxx
 F9  xxxxx  F10  xxxxx   CUR^ xxxxx  CUR> xxxxx
 F11 xxxxx  F12  xxxxx   CUR< xxxxx  CURv xxxxx
--------------------------------------------------------------------------*/
void
keyset_display()
{
	int itmp;
	int itmp2;
	int clen1 = 0;
	char cfmt1[32];
	int clen2 = 0;
	char cfmt2[32];
	int clen3 = 0;
	char cfmt3[32];
	char cfmt4[32];
	KDE *tkde;

	if (!keyset_name[0])
		keyset_init();

	for (itmp = 0; itmp < KDE_COUNT; itmp++)
	{
		tkde = &keyset_table[itmp];
		itmp2 = strlen(tkde->logical);
		switch (itmp)
		{
			case IKDE_F1:
			case IKDE_F3:
			case IKDE_F5:
			case IKDE_F7:
			case IKDE_F9:
			case IKDE_F11:
				if (clen1 < itmp2)
					clen1 = itmp2;
				break;

			case IKDE_F2:
			case IKDE_F4:
			case IKDE_F6:
			case IKDE_F8:
			case IKDE_F10:
			case IKDE_F12:
				if (clen2 < itmp2)
					clen2 = itmp2;
				break;

			case IKDE_HOME:
			case IKDE_END:
			case IKDE_INS:
			case IKDE_CUU:
			case IKDE_CUL:
				if (clen3 < itmp2)
					clen3 = itmp2;
				break;

			case IKDE_InitStr:	/* initialization string excluded */
				break;
		}
	}
	sprintf(cfmt1, " %%-%d.%ds", clen1, clen1);
	sprintf(cfmt2, " %%-%d.%ds", clen2, clen2);
	sprintf(cfmt3, " %%-%d.%ds", clen3, clen3);
	strcpy(cfmt4, " %s");
	ff(se, "   key definition: %s\r\n\r\n", keyset_name);

	ffso(" F1  ");
	ff(se, cfmt1, keyset_table[IKDE_F1].logical);
	fputs("  ", se);
	ffso(" F2  ");
	ff(se, cfmt2, keyset_table[IKDE_F2].logical);
	fputs("  ", se);
	ffso(" Home ");
	ff(se, cfmt3, keyset_table[IKDE_HOME].logical);
	fputs("  ", se);
	ffso(" PgUp ");
	ff(se, cfmt4, keyset_table[IKDE_PGUP].logical);
	fputs("\r\n", se);

	ffso(" F3  ");
	ff(se, cfmt1, keyset_table[IKDE_F3].logical);
	fputs("  ", se);
	ffso(" F4  ");
	ff(se, cfmt2, keyset_table[IKDE_F4].logical);
	fputs("  ", se);
	ffso(" End  ");
	ff(se, cfmt3, keyset_table[IKDE_END].logical);
	fputs("  ", se);
	ffso(" PgDn ");
	ff(se, cfmt4, keyset_table[IKDE_PGDN].logical);
	fputs("\r\n", se);

	ffso(" F5  ");
	ff(se, cfmt1, keyset_table[IKDE_F5].logical);
	fputs("  ", se);
	ffso(" F6  ");
	ff(se, cfmt2, keyset_table[IKDE_F6].logical);
	fputs("  ", se);
	ffso(" Ins  ");
	ff(se, cfmt3, keyset_table[IKDE_INS].logical);
	fputs("  ", se);
	ffso(" CUR5 ");
	ff(se, cfmt4, keyset_table[IKDE_CU5].logical);
	fputs("\r\n", se);

	ffso(" F7  ");
	ff(se, cfmt1, keyset_table[IKDE_F7].logical);
	fputs("  ", se);
	ffso(" F8  ");
	ff(se, cfmt2, keyset_table[IKDE_F8].logical);
	fputs("  ", se);
	ffso(" BkTab");
	ff(se, cfmt3, keyset_table[IKDE_BKTAB].logical);
	fputs("\r\n", se);

	ffso(" F9  ");
	ff(se, cfmt1, keyset_table[IKDE_F9].logical);
	fputs("  ", se);
	ffso(" F10 ");
	ff(se, cfmt2, keyset_table[IKDE_F10].logical);
	fputs("  ", se);
	ffso(" CUR^ ");
	ff(se, cfmt3, keyset_table[IKDE_CUU].logical);
	fputs("  ", se);
	ffso(" CUR> ");
	ff(se, cfmt4, keyset_table[IKDE_CUR].logical);
	fputs("\r\n", se);

	ffso(" F11 ");
	ff(se, cfmt1, keyset_table[IKDE_F11].logical);
	fputs("  ", se);
	ffso(" F12 ");
	ff(se, cfmt2, keyset_table[IKDE_F12].logical);
	fputs("  ", se);
	ffso(" CUR< ");
	ff(se, cfmt3, keyset_table[IKDE_CUL].logical);
	fputs("  ", se);
	ffso(" CURv ");
	ff(se, cfmt4, keyset_table[IKDE_CUD].logical);
	fputs("\r\n\r\n", se);

}							 /* end of keyset_display */

/* end of ecufkey.c */
/* vi: set tabstop=4 shiftwidth=4: */
