char *rev = "1.2";
/*+-------------------------------------------------------------------------
	bperr.c - build proc_error .c from ecuerror.h
	wht@wht.net
--------------------------------------------------------------------------*/
/*+:EDITS:*/
/*:02-01-1998-18:53-wht@kepler-make erc_text and have proc_error call it */
/*:01-24-1997-02:36-wht@yuriatin-SOURCE RELEASE 4.00 */
/*:09-11-1996-19:59-wht@yuriatin-3.48-major telnet,curses,structural overhaul */
/*:11-23-1995-11:20-wht@kepler-source control 3.37 for tsx-11 */
/*:11-14-1995-10:23-wht@kepler-3.37.80-source control point: SOCKETS */
/*:05-04-1994-04:38-wht@n4hgf-ECU release 3.30 */
/*:09-10-1992-13:58-wht@n4hgf-ECU release 3.20 */
/*:08-22-1992-15:38-wht@n4hgf-ECU release 3.20 BETA */
/*:07-25-1991-12:55-wht@n4hgf-ECU release 3.10 */
/*:08-14-1990-20:39-wht@n4hgf-ecu3.00-flush old edit history */

#include <stdio.h>
#include <time.h>

char *strchr();

#define MAXLINE 256
#define MAXFLDS 50

char line[MAXLINE];
char copy[MAXLINE];
char *fields[MAXFLDS + 1];

char *bc =
"/*+-------------------------------------------------------------------------";
char *ec =
"--------------------------------------------------------------------------*/";

/*+-------------------------------------------------------------------------
	splitter(sep)
--------------------------------------------------------------------------*/
int
splitter(sep)
char *sep;
{
	char *cptr;
	register fld;

	for (fld = 1; fld <= MAXFLDS; fld++)
		fields[fld] = 0;
	if (!strlen(sep) || !strlen(line))
		return (0);
	fld = 1;
	strcpy(copy, line);
	cptr = copy;
	while (fld < MAXFLDS)
	{
		while (strchr(sep, *cptr))
		{
			if (!*++cptr)
				return (fld);
		}
		fields[fld++] = cptr++;
		while (!strchr(sep, *cptr))
		{
			if (!*++cptr)
				return (fld);
		}
		*cptr++ = 0;
	}
	return (fld);
}							 /* end of splitter */

/*+-------------------------------------------------------------------------
	main(argc,argv)
--------------------------------------------------------------------------*/
main(argc, argv)
int argc;
char **argv;
{
	register field_count;
	register itmp;
	long cur_time;
	struct tm *ltime;
	FILE *fp;

#if defined(USE_FCRC)
	char cmd[256];

#endif /* defined(USE_FCRC) */

	argv = 0; /* stop saying unused */
	argc = 0;

	freopen("proc_error.c", "w", stdout);

	puts(bc);
	puts("\tproc_error.c - ecu error code handler");
	puts("");
	puts("  D O   N O T   E D I T   B Y   H A N D");
	puts("  Created automagically by bperr/bperr.c");
	puts("");
	puts("  Defined functions:");
	puts("\terc_text(erc)");
	puts("\tproc_error(erc)");
	puts("");
	puts(ec);
	puts("/*+:EDITS:*/");

	cur_time = time((long *)0);
	ltime = localtime(&cur_time);
	printf("/*\
:%02d-%02d-%04d-%02d:%02d-auto-creation from ecuerror.h by build_err %s */\n",
		ltime->tm_mon + 1, ltime->tm_mday, ltime->tm_year + 1900,
		ltime->tm_hour, ltime->tm_min,
		rev);
	puts("");
	puts("#include \"ecu.h\"");
	puts("#include \"ecuerror.h\"");
	puts("");
	puts(bc);
	puts("\terc_text(erc) - error code to text");
	puts("");
	puts(" e.*ALREADY and eProcAttn.* excluded because they are never printed");
	puts(ec);
	puts("char *");
	puts("erc_text(erc)");
	puts("int erc;");
	puts("{");
	puts("\tstatic char errant[54];");
	puts("");
	puts("\tswitch(erc)");
	puts("\t{");

	fp = fopen("ecuerror.h", "r");

	while (fgets(line, sizeof(line), fp))
	{
		if(itmp = strlen(line))
			line[itmp - 1] = 0;
		if(!itmp)
			continue;

		memset((char *)fields,0,sizeof(fields));

		fields[0] = line;
		field_count = splitter(" \t");
		if (!field_count || (strcmp(fields[1], "#define")))
			continue;
		if ((!strcmp(fields[2], "eFATAL_ALREADY")) ||
			(!strcmp(fields[2], "eWARNING_ALREADY")) ||
			(!strncmp(fields[2], "eProcAttn",9)) ||
			(!strncmp(fields[2], "_e", 2)) ||
			(!strncmp(fields[2], "e_", 2)))
		{
			continue;
		}
		printf("\t\tcase %s:\n", fields[2]);
		fputs("\t\t\treturn(\"", stdout);

		for (itmp = 1; itmp < field_count - 1; itmp++)
		{
			if (!strcmp(fields[itmp], "/*"))
				break;
		}
		itmp++;

		for (; itmp < field_count - 1; itmp++)
		{
			fputs(fields[itmp], stdout);
			if (itmp != (field_count - 2))
				fputc(' ', stdout);
		}
		puts("\");");
	}
	puts("");
	puts("\t\tdefault:");
	puts("\t\t\tsprintf(errant,\"unknown error %04X\",erc);");
	puts("\t\t\treturn(errant);");

	puts("\t}");
	puts("} /* end of erc_text */\n");

	puts("");
	puts(bc);
	puts("\tproc_error(erc) - print error text with newline appended");
	puts(ec);
	puts("void");
	puts("proc_error(erc)");
	puts("int erc;");
	puts("{");
	puts("\tpputs(erc_text(erc));");
	puts("\tpputs(\"\\n\");\n");
	puts("} /* end of proc_error */\n");

	puts("");
	puts("/* vi: set tabstop=4 shiftwidth=4: */");
	puts("/* end of proc_error.c */");
	freopen("/dev/tty", "a", stdout);
#if defined(USE_FCRC)
	sprintf(cmd, "fcrc -u proc_error.c");
	system(cmd);
#endif /* defined(USE_FCRC) */
	exit(0);
}							 /* end of main */

/* vi: set tabstop=4 shiftwidth=4: */
/* end of bperr.c */
