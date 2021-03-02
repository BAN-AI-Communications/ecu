/*+-----------------------------------------------------------------
	ecuphrases.c - %# phrase management
	wht@wht.net

  Defined functions:
	phrase_help()
	phrases(nargc, nargv)
	read_phrases()

  God made the Idiot for practice, and then He made the School
  Board -- Mark Twain

------------------------------------------------------------------------*/
/*+:EDITS:*/
/*:04-26-2000-11:15-wht@bob-RELEASE 4.42 */
/*:01-24-1997-02:37-wht@yuriatin-SOURCE RELEASE 4.00 */
/*:09-11-1996-20:00-wht@yuriatin-3.48-major telnet,curses,structural overhaul */
/*:07-31-1996-17:12-dgy@rtd.com-fix phrase_help ^p */
/*:11-23-1995-11:20-wht@kepler-source control 3.37 for tsx-11 */
/*:11-14-1995-10:23-wht@kepler-3.37.80-source control point: SOCKETS */
/*:05-11-1995-15:55-wht@n4hgf-ck_sigint fools optimizing compilers */
/*:05-04-1994-04:39-wht@n4hgf-ECU release 3.30 */
/*:09-10-1992-13:58-wht@n4hgf-ECU release 3.20 */
/*:08-22-1992-15:38-wht@n4hgf-ECU release 3.20 BETA */
/*:07-25-1991-12:56-wht@n4hgf-ECU release 3.10 */
/*:07-17-1991-07:04-wht@n4hgf-avoid SCO UNIX nap bug */
/*:08-14-1990-20:40-wht@n4hgf-ecu3.00-flush old edit history */

#include "ecu.h"

#define P_N_QUAN	23
char *phrases_string[P_N_QUAN];
char *phrases_label[P_N_QUAN];
int phrases_count = 0;
int phrases_resident = 0;

/*+-----------------------------------------------------------------------
	read_phrases()
------------------------------------------------------------------------*/
void
read_phrases()
{
	char *phrases_str;
	char phrases_buf[256];
	char phrases_buf_copy[256];
	char *phrases_lbl;
	FILE *fd_phrase;

	if (phrases_resident)
	{
		while (phrases_count)
			free(phrases_string[--phrases_count]);
		phrases_resident = 0;
	}

	get_home_dir(phrases_buf);
	strcat(phrases_buf, "/.ecu/phrases");

	if (!(fd_phrase = fopen(phrases_buf, "r")))
	{
		ff(se, "\r\n");
		perror(phrases_buf);
		ff(se, "\r\n");
		ff(se, "... no phrases resident\r\n");
		return;
	}

/* we have an open .ecu/phrase file */
	phrases_count = 0;
	while (fgets(phrases_buf, sizeof(phrases_buf), fd_phrase))
	{
		phrases_buf[strlen(phrases_buf) - 1] = 0;
		if (strlen(phrases_buf) == 0)
			continue;

		if (phrases_count == P_N_QUAN)
		{
			ff(se, "\r\nMaximum number of phrases %d exceeded\r\n", P_N_QUAN);
			ff(se, "rest of file ignored, starting with the following:\r\n");
			ff(se, "--> %s\r\n\r\n", phrases_buf);
			phrases_resident = 1;
			fclose(fd_phrase);
			return;
		}
		strcpy(phrases_buf_copy, phrases_buf);
		phrases_lbl = phrases_buf_copy;
		for (phrases_str = phrases_buf_copy; *phrases_str; phrases_str++)
		{
			if (*phrases_str == ':')
			{
				*phrases_str++ = 0;
				break;
			}
			if (*phrases_str == 0)
			{
				ff(se, "invalid entry `%s'\n", phrases_buf);
				continue;
			}
		}

		if (!(phrases_string[phrases_count] =
				malloc(strlen(phrases_str) + 2)) ||
			!(phrases_label[phrases_count] =
				malloc(strlen(phrases_lbl) + 2)))
		{
			ff(se, "\r\nNo more memory for phrases\r\n");
			ff(se, "rest of file ignored, starting with the following:\r\n");
			ff(se, "--> %s\r\n\r\n", phrases_buf);
			phrases_resident = 1;
			fclose(fd_phrase);
			if (phrases_string[phrases_count])
				free(phrases_string[phrases_count]);
			return;
		}
		strcpy(phrases_string[phrases_count], phrases_str);
		strcpy(phrases_label[phrases_count], phrases_lbl);
		phrases_count++;
	}						 /* while records left to read */

	fclose(fd_phrase);
	phrases_resident = 1;
}							 /* end of read_phrases */

/*+-------------------------------------------------------------------------
	phrases(nargc,nargv)
--------------------------------------------------------------------------*/
phrases(nargc, nargv)
int nargc;
char **nargv;
{
	int itmp;
	int ichar;
	char *cp;
	int old_ttymode = get_ttymode();
	extern int icmd_prompt_len;

	for (itmp = icmd_prompt_len + strlen(nargv[0]); itmp; itmp--)
		fputs("\b \b", se);

	itmp = atoi(nargv[0]);

	if (itmp == 0)
	{
		ff(se, "\r\n");
		read_phrases();
		if (!phrases_count)
			return (0);
		tcap_stand_out();
		ff(se,
			" # |  mnemonic    |     phrase                                              ");
		tcap_stand_end();
		ff(se, "\r\n");
		for (itmp = 0; itmp < phrases_count; itmp++)
		{
			ff(se, "%2d | %12s |  %s\r\n", itmp + 1, phrases_label[itmp],
				phrases_string[itmp]);
		}
		return (0);
	}
	else if (phrases_resident == 0)
		read_phrases();

	if (itmp > phrases_count)
	{
		ff(se, "  unknown: %d\r\n", itmp);
		return (-1);
	}
	else
	{
		cp = phrases_string[itmp - 1];
		ttymode(2);
		while (*cp)
		{
			if (ck_sigint())
				break;

			switch (ichar = *cp++)
			{
				case '^':
					ichar = *cp++;
					if ((ichar >= '@') && (ichar <= '_'))
						lputc_paced(0, ichar & 0x1F);
					else if (ichar == '?')
						lputc_paced(0, 0x7F);
					else
					{
						switch (ichar)
						{
							case 0:
								goto NUL_FOUND;
							case 'r':
								lputc_paced(0, '\r');
								break;
							case 'n':
								lputc_paced(0, '\n');
								break;
							case 't':
								lputc_paced(0, '\t');
								break;
							case '^':
								lputc_paced(0, '^');
								break;
							case 'p':
								itmp = atoi(cp);
								while ((*cp >= '0') && (*cp <= '9'))
									cp++;
								if (*cp == '.')
									cp++;
								if (!itmp)
									itmp = 1;
								Nap((long)itmp * 100L);
								break;
							case 'a':
								itmp = atoi(cp);
								while ((*cp >= '0') && (*cp <= '9'))
									cp++;
								if (*cp == '.')
									cp++;
								if (itmp < nargc)
								{
									lputs_paced(0, nargv[itmp]);
									itmp = strlen(nargv[itmp]);
								}
								break;
						}
					}
					break;
				default:
					lputc_paced(0, ichar);
			}
		}

	  NUL_FOUND:
		if (ck_sigint())
		{
			sigint = 0;
			ff(se, "\r\n--> interrupted\r\n");
		}

	}

	ttymode(old_ttymode);
	return (0);

}							 /* end of phrases */

/*+-------------------------------------------------------------------------
	phrase_help()
--------------------------------------------------------------------------*/
void
phrase_help()
{
	ff(se, "^r == \\r    ^n == \\n   ^t == \\t  ^^ == '^'\r\n");
	ff(se, "^p#.  pause # decisecs\r\n");
	ff(se, "^a#.  arg number # of %%# invocation\r\n");
}							 /* end of phrase_help */
/* vi: set tabstop=4 shiftwidth=4: */
