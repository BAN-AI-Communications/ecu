/* CHK=0x5793 */
%{
char *rev = "1.00";
/*+-------------------------------------------------------------------------
	mkoldproto.c.{l,c} - make "old" style prototypes from "new" kind

cc -Zg source.c | mkproto > lint_args.h

takes prototypes of the style
void change_report(char *original);
and emits
void change_report(char *);
The UNIX V compiler makes the new kind, accepts the old and new kind,
but there are a lot of compilers which don't like the new kind.

Warning, this program will make mush out of "old" style prototypes
fed to it.
--------------------------------------------------------------------------*/
/*+:EDITS:*/
/*:04-26-2000-11:16-wht@bob-RELEASE 4.42 */
/*:01-24-1997-02:37-wht@yuriatin-SOURCE RELEASE 4.00 */
/*:09-11-1996-20:00-wht@yuriatin-3.48-major telnet,curses,structural overhaul */
/*:11-23-1995-11:20-wht@kepler-source control 3.37 for tsx-11 */
/*:11-14-1995-10:23-wht@kepler-3.37.80-source control point: SOCKETS */
/*:05-04-1994-04:39-wht@n4hgf-ECU release 3.30 */
/*:09-10-1992-13:59-wht@n4hgf-ECU release 3.20 */
/*:08-22-1992-15:39-wht@n4hgf-ECU release 3.20 BETA */
/*:11-27-1989-18:31-wht-creation */

int verbose = 0;

/*+-------------------------------------------------------------------------
	change_report(original)
--------------------------------------------------------------------------*/
void
change_report(original)
char *original;
{
	fputs("fixing: ",stderr);
	while(*original)
	{
		if((*original >= ' ') && (*original <= '~'))
			fputc(*original,stderr);
		else
			fprintf(stderr,"<%02x>",*original);
		original++;
	}
	fputs("\n",stderr);
}	/* end of change_report */


/*+-------------------------------------------------------------------------
	main(argc,argv,envp)
--------------------------------------------------------------------------*/
main(argc,argv,envp)
int argc;
char **argv;
char **envp;
{
int itmp;
int iargv;
int fileiargv = 0;

	if(argc > 1)
	{
		for(iargv = 1; iargv < argc; iargv++)
		{
			if(argv[iargv][0] == '-')
			{
				switch(itmp = (argv[iargv][1]))
				{
					case 'v':
						verbose = 1;
						break;
					default:
						break;
				}
			}
			else
			{
				if(yyin != stdin)
				{
					fprintf(stderr,"only 1 filename, please\n");
					exit(1);
				}
				if( (yyin = fopen(argv[iargv],"r")) == NULL)
				{
					perror(argv[iargv]);
					exit(1);
				}
				fileiargv = iargv;
			}
		}
	}

	if(verbose && fileiargv)
		fprintf(stderr,"%s:\n",argv[fileiargv]);

	yylex();

	exit(0);
}	/* end of main */

/* vi: set tabstop=4 shiftwidth=4: */
%}
W			[ \t]
STRING		\"([^"\n]|\\\")*\"
OPCHARS		["'+-*/=|!&%^?><:\]]
IDENT		[a-zA-Z0-9_]+
U			"unsigned"
I			"int"
L			"long"
S			"short"
C			"char"
R			"register"
ST			"static"
I8			"INT8"
I16			"INT16"
I32			"INT32"
UN_ST		("union "|"struct "){IDENT}
DECLTAB1	({I}|{C}|{S}|{L}|{U}|{R}|{ST}|{I8}|{I16}|{I32}|{UN_ST})[\t]{IDENT}
DECLTAB2	({I}|{C}|{S}|{L}|{U}|{R}|{ST}|{I8}|{I16}|{I32}|{UN_ST})[\t ][\t ]+{IDENT}
%START	COMMENT PREPROC
%%
<PREPROC>.*\n							{ECHO;
										 BEGIN 0;
										}
^"#"									{
										ECHO;
										BEGIN PREPROC;
										}
<COMMENT>([^*\n]|"*"+[^*/\n])*			ECHO;
<COMMENT>([^*\n]|"*"+[^*/\n])*"*"*"*/"	{ECHO;
										 BEGIN 0;
										}
<COMMENT>.|\n							ECHO;
"/*"				{
						ECHO;
						BEGIN COMMENT;
					}
"extern  "			{
						;
					}
"* "				{
						fprintf(yyout,"*");
					}
{IDENT}","			{
					char *cptr = yytext + yyleng - 1;
						if(verbose)
							change_report(yytext);
						while((*cptr != '\t') && (*cptr != ' ') && (*cptr != '*'))
							*cptr-- = 0;
						fprintf(yyout,",",yytext);
					}
{IDENT}")"			{
					char *cptr = yytext + yyleng - 1;
						if(verbose)
							change_report(yytext);
						while((*cptr != '\t') && (*cptr != ' ') && (*cptr != '*'))
							*cptr-- = 0;
						fprintf(yyout,")",yytext);
					}
{STRING}			ECHO;
"'"."'"				ECHO;
.|\n				ECHO;

