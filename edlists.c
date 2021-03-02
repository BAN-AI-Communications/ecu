/* CHK=0x5A71 */
/*+--------------------------------------------------------------------------
	edlists.c - print edit history
a crocko early effort - no apologies
---------------------------------------------------------------------------*/
/*+:EDITS:*/
/*:04-26-2000-11:15-wht@bob-RELEASE 4.42 */
/*:01-24-1997-02:37-wht@yuriatin-SOURCE RELEASE 4.00 */
/*:08-12-1990-01:10-wht@n4hgf-handle no edit note after trigger */
/*:11-07-1989-03:27-wht-attempt to bring up to date in function if not style */
/*:08-11-1986-11:00-WHT-plus before colon in trigger */
/*:08-06-1986-16:30-WHT-creation */

#ifdef MSDOS
#define LINT_ARGS /* for microsoft C */
#endif

#include "stdio.h"
#include "ctype.h"

#ifndef TRUE
#define TRUE        1
#define FALSE       0
#endif

#ifdef M_I86
#define N_SFILES_MAX    (128)
#else
#define N_SFILES_MAX    (16384)  /* max number of files scanned for match */
#endif

#define	S_IN_BUF_MAX	(256)

#ifdef MSDOS
#define S_FNAME_MAX     (65)
#else
#define S_FNAME_MAX     (129)   /* max size of a file name */
#endif

/* ASCII */
#define TAB             (0x09)  /* tab character */
#define CR              (0x0D)  /* carriage return */
#define LF              (0x0A)  /* line feed */
#define DEFTABWIDTH     (4)     /* default tab width */

int tabwidth = DEFTABWIDTH;     /* tab width (defaulted) */
int iFiles;                     /* index into argv array (filenames) */
int nFiles;                     /* number of input files */
int line_number = 0;            /* line number in file */
int cbSearch;                   /* search string length */
int iargv;                      /* index into argv */
char *eof_if_zero;               /* end of file control flag */
int fInEOF;                     /* TRUE if input file eof reached */
char Files[N_SFILES_MAX][S_FNAME_MAX];   /* filenames */
char inbuf[S_IN_BUF_MAX];
char cmd_fname[S_FNAME_MAX];
FILE    *in_fp;
FILE	*cmd_fp;

char *argv0;

/*+---------------------------------------------
    help_and_return()
    print help text and return to caller
----------------------------------------------*/
help_and_return()
{
	fprintf(stderr,"%s : print edit history (trigger =/*+:EDITS:*/)\n",argv0);
	fprintf(stderr,"usage:  edlists [-a] file1 [ ...]\n",argv0);
	fprintf(stderr,"-a list all edit history, otherwise only latest\n");
	fprintf(stderr,"switch placement on command line not critical\n");
}   /* end of help_and_return() */

/*+---------------------------------------------
    help_and_exit()
    print help text and exit normally
----------------------------------------------*/
help_and_exit()
{
	help_and_return();
	exit(1);
}   /* end of help_and_exit() */

/*+-------------------------------------------------------------------------
    to_upper()  one would think that this was a relatively standard
  type of thing, but MSC specifies toupper() to convert to upper
  case if not already and Unix says to subtract 'A'-'a' regardless,
  so, a stupid little routine here
--------------------------------------------------------------------------*/
char
to_upper(ch)
char ch;
{
	if((ch >= 'a') && (ch <= 'z'))
		return(ch - 0x20);
	else
		return(ch);

}   /* end of to_upper() */

/*+-------------------------------------------------------------------------
    to_lower() ditto comments of previous procedure
--------------------------------------------------------------------------*/
char
to_lower(ch)
char ch;
{

	if( (ch >= 'A') && (ch <= 'Z') )
		return(ch + 0x20);
	else
		return(ch);

}   /* end of to_lower() */

/*+----------------------------------------------------------------------------
    ulcmpb: Upper/Lower [case insensitive] Compare Bytes
    Returns -1 if strings are equal, else failing character position

    If the second strings terminates with a null and both strings have matched
    character for character until that point, then -1 is returned.

    NOTE:  this is not a test for complete equality of two strings, but allows
    discovery of a string as a substring in a larger containing string.
-----------------------------------------------------------------------------*/
ulcmpb(str1,str2)

char *str1;         /* the two strings (terminated with nulls) */
char *str2;
{
	register int istr;

	for( istr=0 ; ;  ++istr )
	{
		if(str2[istr] == '\0')          /* if second string exhausts, match! */
			return(-1);

		if(     ( str1[istr] == '\0' )
		    ||  ( to_upper(str1[istr]) != to_upper(str2[istr]) )
		    )
			return(istr);
	}

} /* end of ulcmpb() */

/*+-------------------------------------------------------------------------
    ulindex:  Upper/Lower [case insensitive] Index functioni

  Returns position of 'str2' in 'str1' if found
  If 'str2' is null, then 0 is returned (null matches anything)
  Returns -1 if not found

  uses 'ulcmpb'
--------------------------------------------------------------------------*/
ulindex(str1,str2)

char *str1;          /* the (target) string to search */
char *str2;          /* the (comparand) string to search for */
{
	register int istr1;         /* moving index into str1 */
	register char first_str2;    /* first character of str2 as upper case */
	register char *mstr;         /* moving string pointer */

	if(*str2 == '\0')               /* null string matches anything */
		return(0);

	istr1=0;                        /* first char position */
	mstr=str1;                      /* address of string 1 */

	for(;;)                         /* well, maybe forever; hopefully not */
	{

		if(*mstr == '\0')           /* if we exhaust target string, flunk */
			return(-1);

		first_str2 = to_upper(*str2);    /* 1st str2 character as upper case */

		/* Can we find either case of first comparand char in target? */
		if( to_upper(*mstr) == first_str2 )
		{
			/* we have a first char match... does rest of string match? */
			if(ulcmpb(mstr,str2) == -1)         /* if the rest matches, ... */
				return(istr1);                  /* ... return match position */
		}

		/* we did not match this time... increment istr1, mstr and try again */
		++istr1;
		++mstr;
	}

}   /* end of ulindex() */

/*+-------------------------------------------------------------------------
	bcat(target,source,source_count)
--------------------------------------------------------------------------*/
char *
bcat(target,source,source_count)
char *target;
char *source;
int source_count;
{
register int pos;
register char *msrc = source;

	if(source_count == 0)
		return(target);
	while(source_count-- && *msrc)
		*target++ =	*msrc++;
	*target = 0;
	return(target);

}	/* end of bcat */

/*+-------------------------------------------------------------------------
	print_edit(fname,buffer)
  000000000011111111112222222222333333333...
  012345678901234567890123456789012345678...
  :08-06-1986-16:30-wht-add -a switch *|		/ replaced with | here

  roaches input buffer (must be writable)
--------------------------------------------------------------------------*/
void
print_edit(fname,inbuf)
char *fname;
char *inbuf;
{
register int itmp;
char out[128];
char *cptr;

	if(inbuf[0] != ':')
		return;
	if((itmp = ulindex(inbuf,"*/")) >= 0)
	{
		inbuf[itmp++] = '\n';
		inbuf[itmp] = 0;
	}
	cptr = out;
	cptr = bcat(cptr,&inbuf[7],5);
	cptr = bcat(cptr,&inbuf[1],6);
	cptr = bcat(cptr,&inbuf[12],6);
	strcat(cptr,fname);
	strcat(cptr,&inbuf[17]);
	fputs(out,stdout);

}	/* end of print_edit */

/*+-------------------------------------------------------------------------
    main program
--------------------------------------------------------------------------*/
main(argc,argv)
int argc;
char **argv;
{
int iSearch; /* index into search strings */
int nSearch; /* number of search strings */
int matchpos; /* ulindex return param */
int fLineNumber = FALSE; /* if TRUE, prefix line number of matching line to output */
int fMatchPos = FALSE; /* if TRUE, print match position */
int itmp;
int all_flag = 0;
int asm_code = 0;
int c_code = 0;
int hash_flag = 0;
char *indicator;

	cmd_fname[0]='\0';                  /* no aux cmd file name yet */
	nFiles=0;                   /* no search files yet */
	argv0 = argv[0];

	/* if there are no arguments, give help */

	if( argc < 2 )
		help_and_exit();    /* and exit program */

	/* parse command line */
	for( iargv=1; iargv < argc ; ++iargv )  /* walk iargv from 1 to 'argc' */
	{
		if((itmp=argv[iargv][0]) != '-')    /* if not switch argument */
		{
			if(itmp == '@')             /* if @file */
			{
				cmd_fp=fopen(&argv[iargv][1],"r");  /* read the '@' file contents */
				if(cmd_fp == NULL)
				{
					fprintf(stderr,"Error opening @file %s.\n",&argv[iargv][1]);
					exit(1);
				}
				do  
				{
					inbuf[0]='\0';
					eof_if_zero=fgets(inbuf,sizeof(inbuf)-1,cmd_fp); /* read filename */

					if(strlen(inbuf) < 2)      /* just a newline is empty */
						eof_if_zero = NULL;    /* simulate eof in such cases */

					if(eof_if_zero == NULL)    /* if eof or error, ... */
						continue;

					if(nFiles == N_SFILES_MAX)  /* can we use it? */
					{
						fprintf(stderr,"\nError: Max files (%d) exceeded.\n",
						    N_SFILES_MAX);
						exit(1);
					}

					itmp=strlen(inbuf);        /* strip trailing newline ... */
					inbuf[--itmp]='\0';        /* ... from input record */
					strncpy(Files[nFiles++],inbuf,S_FNAME_MAX-1);
					printf("\nFile to be searched:  %s",inbuf);
				}/* end of while not @file eof */            while(eof_if_zero != NULL); /* while not @file input eof or error */

				fclose(cmd_fp);
				fputs("\n",stdout);
				continue;
			}/* end of if @file */


			if(nFiles == N_SFILES_MAX)
			{
				fprintf(stderr,"...Max files (%d) exceeded.\n",N_SFILES_MAX);
				exit(1);
			}

			strncpy(Files[nFiles++],argv[iargv],S_FNAME_MAX-1);
			continue;
		} /* end of if not switch arg */
		else /* switch argument */
		{
			switch(to_lower(argv[iargv][1]))
			{
			case 'a':
				all_flag = 1;
				break;
			default:
				fprintf(stderr,"WARNING: unrecognized switch: %s\n",
				    argv[iargv]);
				break;
			}
		} /* end of else switch arg */
	} /* end of walk iargv */

	/* if no files, terminate now */
	if(nFiles == 0)
	{
		printf("No files to search. Program terminated.\n");
		exit(1);
	}

	for(iFiles=0 ; iFiles < nFiles ; ++iFiles)
	{
		fInEOF=FALSE;                       /* reset eof indicator */
		in_fp=fopen(Files[iFiles],"r");     /* open the target file */
		if(in_fp == 0)  /* if fopen returns 0 file number, error occurred */
		{
			printf("\nError opening file %d:  %s\n",iFiles,Files[iFiles]);
			continue;           /* next file */
		}

		line_number = 0;

		/************ per file processing goes below here **************/

		while(!fInEOF)
		{
			if( fgets(inbuf,sizeof(inbuf)-1,in_fp) == 0)
			{
				/*fprintf(stderr,"%s: no EDITS trigger\n",Files[iFiles]);*/
				fInEOF = TRUE;
				continue;
			}
			if( (ulindex(inbuf,"EDITS:") >= 0) &&
					((c_code = !strncmp(inbuf,"/*+:",4)) ||
					 (asm_code = !strncmp(inbuf,"*+:",3)) ||
					 (hash_flag = !strncmp(inbuf,"#+:",3))))
			{
				if(c_code)
					indicator = "/*:";
				else if(asm_code)
					indicator = "*:";
				else
					indicator = "#:";

				if(fgets(inbuf,sizeof(inbuf)-1,in_fp) == 0)
				{
					printf("EOF after EDITS trigger\n");
					break;
				}
				if(strlen(inbuf) > 1)
					print_edit(Files[iFiles],&inbuf[(c_code) ? 2 : 1]);
				if(all_flag == 0)
					break;
				while((fgets(inbuf,sizeof(inbuf)-1,in_fp) != 0) &&
				    (ulindex(inbuf,indicator) == 0)	)
				{
					print_edit(Files[iFiles],&inbuf[(c_code) ? 2 : 1]);
				}
				break;
			}
		} /* end of while !fInEOF */

		/************ per file processing goes above here **************/

		fclose(in_fp);              /* close the target file */
		continue;
	} /* end of for(iFiles=1...nFiles) */
	exit(0);
}	/* end of main() */
/* end of edlist.c */

/* vi: set tabstop=4 shiftwidth=4: */
