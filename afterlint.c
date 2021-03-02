/* CHK=0xC2C5 */
char *rev = "1.20";

/*+-------------------------------------------------------------------------
	afterlint.c -- process MSC -Zg output (for *some* MSC versions)

This works for ecu but maybe not other code collections;
returning a typedefed but unnamed structure (or pointer)
will, for instance, produce bad output.

Use with zgcc.
--------------------------------------------------------------------------*/
/*+:EDITS:*/
/*:04-26-2000-11:15-wht@bob-RELEASE 4.42 */
/*:01-24-1997-02:36-wht@yuriatin-SOURCE RELEASE 4.00 */
/*:09-11-1996-19:59-wht@yuriatin-3.48-major telnet,curses,structural overhaul */
/*:11-23-1995-11:20-wht@kepler-source control 3.37 for tsx-11 */
/*:11-14-1995-10:23-wht@kepler-3.37.80-source control point: SOCKETS */
/*:05-04-1994-04:38-wht@n4hgf-ECU release 3.30 */
/*:09-10-1992-13:58-wht@n4hgf-ECU release 3.20 */
/*:08-22-1992-15:38-wht@n4hgf-ECU release 3.20 BETA */
/*:01-21-1992-03:03-wht@n4hgf-better handling of commented items with UNNAMED */
/*:07-25-1991-12:55-wht@n4hgf-ECU release 3.10 */
/*:01-09-1991-22:31-wht@n4hgf-ISC port */
/*:11-18-1990-21:15-wht@n4hgf-clobber 'extern  ' in prototypes */
/*:07-13-1988-19:50-wht-creation */

#include <stdio.h>
#include <sys/types.h>

#ifdef BSD
#include <sys/time.h>
#define strchr index
#define strrchr rindex
char *index();
char *rindex();

/*************************/
#else /* assuming SYSV */
/*************************/
#include <time.h>
char *strchr();
char *strrchr();

#endif /* system dependencies */

struct tm *localtime();

/*+-------------------------------------------------------------------------
	emit_editnote(fp)
--------------------------------------------------------------------------*/
void
emit_editnote(fp)
FILE *fp;
{
	struct tm *ltime;
	long cur_time;

	time(&cur_time);
	ltime = localtime(&cur_time);

	fputs("/*+:EDITS:*/\n", fp);
	fprintf(fp, "/*:%02d-%02d-%04d-%02d:%02d-afterlint %s-creation */\n",
		ltime->tm_mon + 1, ltime->tm_mday, ltime->tm_year + 1900,
		ltime->tm_hour, ltime->tm_min, rev);

}							 /* end of emit_editnote */

/*+-------------------------------------------------------------------------
	instr(str1,str2) - is str2 contained in str1?

return 1 if so, else 0
--------------------------------------------------------------------------*/
int
instr(str1, str2)
char *str1;					 /* the (target) string to search */
char *str2;					 /* the (comparand) string to search for */
{
	int lstr2 = strlen(str2);

	if (lstr2 > strlen(str1))
		return (0);
	while (*str1)
	{
		if (*str1 == *str2)
		{
			if (!strncmp(str1, str2, lstr2))
				return (1);
		}
		++str1;
	}
	return (0);

}							 /* end of instr */

/*+-------------------------------------------------------------------------
	main(argc,argv,envp)
--------------------------------------------------------------------------*/
main(argc, argv, envp)
int argc;
char **argv;
char **envp;
{
	char *cp;
	FILE *fpin;
	FILE *fpout;
	char buf[256];
	char *basename;

	if (argc < 2)
	{
		fprintf(stderr, "usage: afterlint <infile> [<outfile>]\n");
		fprintf(stderr, "if outfile not supplied, output is to stdout\n");
		exit(1);
	}

	if (!(fpin = fopen(argv[1], "r")))
	{
		perror(argv[1]);
		exit(1);
	}

	if (argc > 2)
	{
		if (!(fpout = fopen(argv[2], "w")))
		{
			perror(argv[2]);
			exit(1);
		}
		basename = argv[2];
	}
	else
		fpout = stdout;

	fprintf(fpout,
		"/*+-----------------------------------------------------------------------\n");
	if (argc > 2)
		fprintf(fpout, "\t%s\n", basename);
	else
		fprintf(fpout, "\tfunction declarations\n", basename);
	fprintf(fpout,
		"------------------------------------------------------------------------*/\n");

	emit_editnote(fpout);
	fprintf(fpout, "\n");
	fprintf(fpout, "#ifndef BUILDING_PROTOTYPES\n");
	fprintf(fpout, "\n/* the following should catch only SCO MSC */\n");
	fprintf(fpout,
		"#if defined(M_SYSV) && !defined(__GNUC__) && !defined(GNUC)\n\n");

	while (fgets(buf, sizeof(buf), fpin))
	{
		cp = buf;
		if (instr(buf, "UNNAMED"))
		{
			cp += 3;
			if (strchr(cp, '('))
			{
				while (*cp != '(')
					fputc(*cp++, fpout);
				fputs("(); /* no proto (struct UNNAMED param) */\n", fpout);
			}
			continue;
		}
		if (!strncmp(buf, "/*global*/  ", 12))
			cp += 12;
		else if (!strncmp(buf, "extern  ", 8))
			cp += 8;
		else if (!strncmp(buf, "static  ", 8))
			cp += 8;
		fputs(cp, fpout);
	}

	fprintf(fpout, "\n#else\t\t/* compiler doesn't know prototyping */\n\n");

	fclose(fpin);
	fpin = fopen(argv[1], "r");

	while (fgets(buf, sizeof(buf), fpin))
	{
		cp = buf;
		if (instr(buf, "UNNAMED"))
			cp += 3;
		else if (!strncmp(buf, "/*global*/  ", 12))
			cp += 12;
		else if (!strncmp(buf, "extern  ", 8))
			cp += 8;
		else if (!strncmp(buf, "static  ", 8))
			cp += 8;
		if (strchr(cp, '('))
		{
			while (*cp != '(')
				fputc(*cp++, fpout);
			fputs("();\n", fpout);
		}
	}

	fprintf(fpout, "\n#endif /* MSC considerations */\n");
	fprintf(fpout, "#endif /* BUILDING_PROTOTYPES */\n");
	fprintf(fpout, "\n/* end of %s */\n",
		(argc > 2) ? basename : "function declarations");

	fclose(fpin);
	fclose(fpout);
	exit(0);
}							 /* end of main */

/* vi: set tabstop=4 shiftwidth=4: */
