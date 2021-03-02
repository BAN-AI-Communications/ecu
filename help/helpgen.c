char *rev = "4.00";
/*+-------------------------------------------------------------------------
	helpgen.c -- ecu command help file maker
	wht@wht.net

  Defined functions:
	build_ecudoc()
	build_ecuhelp()
	build_ecunroff()
	main(argc, argv)
	search_cmd_list(cmd)
	show_cmds()
	test_help()
	usage()

--------------------------------------------------------------------------*/
/*+:EDITS:*/
/*:01-24-1997-02:37-wht@yuriatin-SOURCE RELEASE 4.00 */
/*:09-16-1996-19:03-wht@yuriatin-add nroff output */
/*:09-11-1996-20:00-wht@yuriatin-3.48-major telnet,curses,structural overhaul */
/*:09-06-1996-02:47-wht@kepler-cleanup */
/*:11-23-1995-11:20-wht@kepler-source control 3.37 for tsx-11 */
/*:11-14-1995-10:23-wht@kepler-3.37.80-source control point: SOCKETS */
/*:05-04-1994-04:39-wht@n4hgf-ECU release 3.30 */
/*:09-10-1992-13:59-wht@n4hgf-ECU release 3.20 */
/*:08-22-1992-15:39-wht@n4hgf-ECU release 3.20 BETA */
/*:07-25-1991-12:58-wht@n4hgf-ECU release 3.10 */
/*:07-12-1991-14:50-wht@n4hgf-remove obsolete ecuhelp.txt generator */
/*:08-14-1990-20:40-wht@n4hgf-ecu3.00-flush old edit history */

#include <stdio.h>
#include <ctype.h>

#include "../ecu_types.h"
#include "../ecutermio.h"

#define DECLARE_P_CMD
#define HELPGEN
typedef int (*PFI) ();		 /* pointer to function returning integer */
#include "../ecucmd.h"

#include "../esd.h"

#define PSRC	"ecuhelp.src"
#define PDAT	"ecuhelp.data"
#define PDOC	"ecuhelp.doc"
#define PNROFF	"ecuhelp.nroff"

long start_pos[TOKEN_QUAN];
int token_line[TOKEN_QUAN];
FILE *fpsrc;				 /* help source file */
FILE *fpdat;				 /* help data file */
FILE *fpdoc;				 /* help doc file */
FILE *fptxt;				 /* help nroff file */
FILE *fpnroff;				 /* help nroff file */
P_CMD *pcmd;
int src_line = 0;
char buf[128];

/*+-------------------------------------------------------------------------
	usage()
--------------------------------------------------------------------------*/
usage()
{
	fprintf(stderr, "usage: helpgen [-b] [-d] [-s] [-t]\n");
	fprintf(stderr, " -b build %s from %s\n", PDAT, PSRC);
	fprintf(stderr, " -d build %s from %s\n", PDOC, PDAT);
	fprintf(stderr, " -n build %s from %s\n",PNROFF,PSRC);
	fprintf(stderr, " -s show list of commands\n");
	fprintf(stderr, " -t test help\n");
	fprintf(stderr, "At least one switch must be issued.  They are executed\n");
	fprintf(stderr, "in the order shown on the usage line.\n");
	exit(1);
}							 /* end of usage */

/*+-------------------------------------------------------------------------
	search_cmd_list(cmd)
--------------------------------------------------------------------------*/
P_CMD *
search_cmd_list(cmd)
register char *cmd;
{
	register P_CMD *cmd_list = icmd_cmds;

	while (cmd_list->token != -1)
	{
		if (strcmp(cmd_list->cmd, cmd) == 0)
			break;
		cmd_list++;
	}
	if (cmd_list->token == -1)
		return ((P_CMD *) 0);
	else
		return (cmd_list);

}							 /* end of search_cmd_list */

/*+-------------------------------------------------------------------------
	show_cmds()
commands with null descriptions are "undocumented"
--------------------------------------------------------------------------*/
void
show_cmds()
{
	register int itmp;
	register P_CMD *this = icmd_cmds;
	register int longest_cmd = 0;
	register int longest_descr = 0;
	register int nl_flag = 0;
	char s80[80];
	P_CMD *longest_cmd_p = 0;
	P_CMD *longest_descr_p = 0;

	while (this->token != -1)
	{
		if (!*this->descr)
		{
			this++;
			continue;
		}
		itmp = strlen(this->cmd);
		if (itmp > longest_cmd)
		{
			longest_cmd = itmp;
			longest_cmd_p = this;
		}
		itmp = strlen(this->descr);
		if (itmp > longest_descr)
		{
			longest_descr = itmp;
			longest_descr_p = this;
		}
		this++;
	}
	this = icmd_cmds;
	while (this->token != -1)
	{
		if ((!this->min_ch) || (!*this->descr))
		{
			this++;
			continue;
		}
		strcpy(s80, this->cmd);
		pad_zstr_to_len(s80, longest_cmd + 2);
		for (itmp = 0; itmp < this->min_ch; itmp++)
			s80[itmp] = to_upper(s80[itmp]);
		fputs(s80, stderr);

		strcpy(s80, this->descr);
		pad_zstr_to_len(s80, longest_descr + 1);
		fputs(s80, stderr);

		if (nl_flag)
			fputs("\r\n", stderr);
		else
			fputs("| ", stderr);
		nl_flag = (nl_flag) ? 0 : 1;

		this++;
	}
	if (nl_flag)
		fputs("\r\n", stderr);

	itmp = longest_cmd + longest_descr + 5;
	sprintf(s80, "recwidth = %d\r\n", itmp);
	fprintf(stderr, s80);
	this = longest_cmd_p;
	sprintf(s80, "longest cmd: %s: %s\r\n", this->cmd, this->descr);
	fprintf(stderr, s80);
	this = longest_descr_p;
	sprintf(s80, "longest dsc: %s: %s\r\n", this->cmd, this->descr);
	fprintf(stderr, s80);

}							 /* end of show_cmds */

/*+-------------------------------------------------------------------------
	build_ecuhelp()
--------------------------------------------------------------------------*/
void
build_ecuhelp()
{
	register int itmp;
	register char *cptr;
	P_CMD *this;

	printf("\nBuilding %s\n", PDAT);

/* use proc cmd entry for flag */
	this = icmd_cmds;
	while (this->token != -1)
	{
		this->proc = (PFI) 0;
		this++;
	}

	for (itmp = 0; itmp < TOKEN_QUAN; itmp++)
	{
		start_pos[itmp] = 0L;
		token_line[itmp] = 0;
	}

	if ((fpsrc = fopen(PSRC, "r")) == NULL)
	{
		perror(PSRC);
		exit(1);
	}

	if ((fpdat = fopen(PDAT, "w")) == NULL)
	{
		perror(PDAT);
		exit(1);
	}

	fwrite((char *)start_pos, sizeof(long),	/* write null table */
		TOKEN_QUAN, fpdat);

	while (fgets(buf, sizeof(buf), fpsrc) != NULL)
	{
		src_line++;
		itmp = strlen(buf);
		buf[--itmp] = 0;	 /* kill trailing nl */
		if (buf[0] == '#')	 /* ignore comments */
			continue;
		if(buf[0] == '.')	 /* nroff directive */
			continue;
		if (buf[0] == '%')	 /* command indication */
		{
		  SEARCH_CMD_LIST:
			if (!(this = search_cmd_list(&buf[1])))
			{
#if 0						 /* primarily because of 'eto' and 'fasi' */
				printf("line %d: '%s' not in command table\n",
					src_line, &buf[1]);
#endif
				while (fgets(buf, sizeof(buf), fpsrc) != NULL)
				{
					src_line++;
					if(buf[0] == '.')	 /* nroff directive */
						continue;
					itmp = strlen(buf);
					buf[--itmp] = 0;	/* kill trailing nl */
					if (buf[0] == '%')	/* command indication */
						goto SEARCH_CMD_LIST;
				}
				break;
			}
			if (start_pos[this->token])
			{
				printf("line %d: '%s' already found on line %d\n",
					src_line, &buf[1], token_line[this->token]);
				exit(1);
			}
			fputs("\n", fpdat);	/* terminate previous command description */
			start_pos[this->token] = ftell(fpdat);
			token_line[this->token] = src_line;
			fputs("   ", fpdat);
			cptr = &buf[1];	 /* command text */
			itmp = 0;
			this->proc = (PFI) 1;	/* indicate we save command info */
			while (*cptr)	 /* show cmd and min chars required */
			{
				if (itmp < this->min_ch)
					fputc(to_upper(*cptr++), fpdat);
				else
					fputc(to_lower(*cptr++), fpdat);
				itmp++;
			}
			if (*this->descr)/* if description present */
				fprintf(fpdat, " : %s\n \n", this->descr);
			else
				fputs("\n \n", fpdat);
			continue;
		}
		fprintf(fpdat, " %s\n", buf);
	}

	fseek(fpdat, 0L, 0);	 /* back to position table */
	fwrite((char *)start_pos, sizeof(long),	/* write actual table */
		TOKEN_QUAN, fpdat);
	fclose(fpsrc);
	fputs("\n", fpdat);		 /* terminate last command */
	fclose(fpdat);

/* say which commands weren't in the help source */
	this = icmd_cmds;
	while (this->token != -1)
	{
		if (this->min_ch && !this->proc)
			fprintf(stderr, "'%s' not in help source\n", this->cmd);
		this++;
	}

}							 /* end of build_ecuhelp */

/*+-------------------------------------------------------------------------
	build_ecunroff()
--------------------------------------------------------------------------*/
void
build_ecunroff()
{
	register int itmp;
	register char *cptr;
	P_CMD *this;

	printf("\nBuilding %s\n", PNROFF);

/* use proc cmd entry for flag */
	this = icmd_cmds;
	while (this->token != -1)
	{
		this->proc = (PFI) 0;
		this++;
	}

	for (itmp = 0; itmp < TOKEN_QUAN; itmp++)
	{
		start_pos[itmp] = 0L;
		token_line[itmp] = 0;
	}

	if ((fpsrc = fopen(PSRC, "r")) == NULL)
	{
		perror(PSRC);
		exit(1);
	}

	if ((fpnroff = fopen(PNROFF, "w")) == NULL)
	{
		perror(PDAT);
		exit(1);
	}


	while (fgets(buf, sizeof(buf), fpsrc) != NULL)
	{
		src_line++;
		itmp = strlen(buf);
		buf[--itmp] = 0;	 /* kill trailing nl */
		if (buf[0] == '#')	 /* ignore comments */
			continue;
		if (buf[0] == '%')	 /* command indication */
		{
		  SEARCH_CMD_LIST:
			if (!(this = search_cmd_list(&buf[1])))
			{
#if 0						 /* primarily because of 'eto' and 'fasi' */
				printf("line %d: '%s' not in command table\n",
					src_line, &buf[1]);
#endif
				while (fgets(buf, sizeof(buf), fpsrc) != NULL)
				{
					src_line++;
					itmp = strlen(buf);
					buf[--itmp] = 0;	/* kill trailing nl */
					if (buf[0] == '%')	/* command indication */
						goto SEARCH_CMD_LIST;
				}
				break;
			}
			fputs("\n", fpnroff);	/* terminate previous command description */
			start_pos[this->token] = ftell(fpnroff);
			token_line[this->token] = src_line;
			cptr = &buf[1];	 /* command text */
			itmp = 0;
			this->proc = (PFI) 1;	/* indicate we save command info */
			fputs(".SH ",fpnroff);
			while (*cptr)	 /* show cmd and min chars required */
			{
				if (itmp < this->min_ch)
					fputc(to_upper(*cptr++), fpnroff);
				else
					fputc(to_lower(*cptr++), fpnroff);
				itmp++;
			}
			if (*this->descr)/* if description present */
				fprintf(fpnroff, " : %s\n \n", this->descr);
			else
				fputs("\n \n", fpnroff);
			continue;
		}
		fprintf(fpnroff, "%s\n", buf);
	}

	fclose(fpsrc);
	fclose(fpnroff);

/* say which commands weren't in the nroff source */
	this = icmd_cmds;
	while (this->token != -1)
	{
		if (this->min_ch && !this->proc)
			fprintf(stderr, "'%s' not in help source\n", this->cmd);
		this++;
	}

}							 /* end of build_ecunroff */

/*+-------------------------------------------------------------------------
	build_ecudoc()
--------------------------------------------------------------------------*/
void
build_ecudoc()
{
	register int itmp;

	printf("\nBuilding %s\n", PDOC);
	if ((fpdat = fopen(PDAT, "r")) == NULL)
	{
		perror(PDAT);
		exit(1);
	}
	if ((fpdoc = fopen(PDOC, "w")) == NULL)
	{
		perror(PDOC);
		exit(1);
	}
	fprintf(fpdoc,
		"\n     ECU  Command  Help  Documentation  (PRELIMINARY)\n\n");
	fprintf(fpdoc,
		"Commands are accessed by pressing the HOME key followed by one\n");
	fprintf(fpdoc,
		"of the following commands (capitalized portions are sufficient\n");
	fprintf(fpdoc,
		"to invoke the command):\n");
	fprintf(fpdoc, "\n");
	fprintf(fpdoc,
		"---------------------------------------------------------------------\n");
	fread((char *)start_pos, sizeof(long), TOKEN_QUAN, fpdat);

	pcmd = icmd_cmds;
	while (pcmd->token != -1)
	{
		if (!pcmd->token)
		{
			pcmd++;
			continue;
		}
		if (pcmd->min_ch && !start_pos[pcmd->token])
		{
			printf("no help available for '%s'\n", pcmd->cmd);
			pcmd++;
			continue;
		}
		fseek(fpdat, start_pos[pcmd->token], 0);
		while (fgets(buf, sizeof(buf), fpdat) != NULL)
		{
			itmp = strlen(buf);
			buf[--itmp] = 0;
			if (itmp == 0)
				break;
			fprintf(fpdoc, "%s\n", buf);
		}
		fprintf(fpdoc,
			"---------------------------------------------------------------------\n");
		pcmd++;
	}
	fclose(fpdat);
	fclose(fpdoc);
}							 /* end of build_ecudoc */

/*+-------------------------------------------------------------------------
	test_help()
--------------------------------------------------------------------------*/
void
test_help()
{
	register int itmp;

/* test code */
	printf("\nNow to test\n");
	if ((fpdat = fopen(PDAT, "r")) == NULL)
	{
		perror(PDAT);
		exit(1);
	}
	fread((char *)start_pos, sizeof(long), TOKEN_QUAN, fpdat);

	while (1)
	{
		printf("\ncommand: ");
		fgets(buf, sizeof(buf), stdin);
		itmp = strlen(buf);
		buf[--itmp] = 0;
		if (itmp == 0)
			break;
		if (!(pcmd = search_cmd_list(buf)))
		{
			printf("'%s' not found in ecu cmd table\n", buf);
			continue;
		}
		if (pcmd->min_ch && !start_pos[pcmd->token])
		{
			printf("no help available for '%s'\n", buf);
			continue;
		}
		fseek(fpdat, start_pos[pcmd->token], 0);
		while (fgets(buf, sizeof(buf), fpdat) != NULL)
		{
			itmp = strlen(buf);
			buf[--itmp] = 0;
			if (itmp == 0)
				break;
			printf("%s\n", buf);
		}
	}
}							 /* end of test_help */

/*+-------------------------------------------------------------------------
	main(argc,argv)
--------------------------------------------------------------------------*/
main(argc, argv)
int argc;
char **argv;
{
	register int itmp;
	int iargv;
	int b_flag = 0;
	int d_flag = 0;
	int n_flag = 0;
	int s_flag = 0;
	int t_flag = 0;

	setbuf(stdout, NULL);
	setbuf(stderr, NULL);
	fprintf(stderr,"helpgen %s\n",rev);

	if (argc < 1)
		usage();
	for (iargv = 1; iargv < argc; iargv++)
	{
		if (argv[iargv][0] == '-')
		{
			switch (itmp = (argv[iargv][1]))
			{
				case 'b':
					b_flag = 1;
					break;
				case 's':
					s_flag = 1;
					break;
				case 'n':
					n_flag = 1;
					break;
				case 't':
					t_flag = 1;
					break;
				case 'd':
					d_flag = 1;
					break;
				default:
					usage();
					break;
			}
		}
		else
			usage();
	}
	if (!b_flag && !s_flag && !t_flag && !d_flag && !n_flag)
		usage();

	if (b_flag)
		build_ecuhelp();
	if (d_flag)
		build_ecudoc();
	if (n_flag)
		build_ecunroff();
	if (s_flag)
		show_cmds();
	if (t_flag)
		test_help();

	exit(0);
}							 /* end of main */
/* end of helpgen.c */
/* vi: set tabstop=4 shiftwidth=4: */
