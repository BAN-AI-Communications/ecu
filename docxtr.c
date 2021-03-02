/* CHK=0xE69D */
/*+--------------------------------------------------------------------------
    DOCXTR.c : Documentation Extraction program.

    Copyright (C) 1985, Warren H. Tucker d/b/a TuckerWare
    All Rights Reserved

The program extracts the comments (and possibly other lines) from one or
more source files.  There are several options which select the voluminosity (?)
of the output.  As examples, various  documentation headers in this
program use different options to help produce the documentation.
---------------------------------------------------------------------------*/
/*+:EDITS:*/
/*:04-26-2000-11:15-wht@bob-RELEASE 4.42 */
/*:01-24-1997-02:36-wht@yuriatin-SOURCE RELEASE 4.00 */
/*:01-16-1987-17:40-WHT-do not print EDITS trigger */
/*:03-22-1986-23:20-WHT-Version 1.1/Xenix */
/*:08-04-1985-00:30-WHT-Version 1.0 */
/*:08-03-1985-11:00-WHT-Creation */

#include <stdio.h>
#define TRUE		1
#define FALSE		0

extern char *fgets();

/*+ --- DOCXTR compile-time configuration --- */
#define   S_IN_BUF_MAX          (257)   /* max size of file input line */
#define   N_SFILES_MAX          (32)    /* max number of files to be searched */
#define   S_FNAME_MAX           (41)    /* max size of a file name */
/*} this extract option shows defines even though they are not bounded by slash*+  */

/*+-------------------------------------------------------------------------
    to_upper() / to_lower()
  one would think that these were relatively standard
  types of thing, but MSC/Xenix specifies toupper() to convert to upper
  case if not already and Unix says to adjust without testing,
  so, two stupid little routines here
  ASCII only -- no EBCDIC gradoo here please
--------------------------------------------------------------------------*/
unsigned char to_upper(ch)
register unsigned char    ch;
{ 
	return( ((ch >= 'a') && (ch <= 'z')) ? ch - 0x20 : ch);
}   /* end of to_upper() */

unsigned char to_lower(ch)
register unsigned char    ch;
{ 
	return( ((ch >= 'A') && (ch <= 'Z')) ? ch + 0x20 : ch);
}   /* end of to_lower() */

/*+----------------------------------------------------------------------------
    ulcmpb: Upper/Lower [case insensitive] Compare Bytes
    Returns -1 if strings are equal, else failing character position
    If the second strings terminates with a null and both strings have matched
    character for character until that point, then -1 is returned.
    NOTE:  this is not a test for complete equality of two strings, but allows
    discovery of a string as a substring in a larger containing string.
-----------------------------------------------------------------------------*/
int ulcmpb(str1,str2)
register unsigned char    *str1;
register unsigned char    *str2;
{
	register int istr;

	for( istr=0 ; ;  ++istr )
	{
		if(str2[istr] == '\0')          /* if second string exhausts, match! */
			return(-1);
		if((str1[istr] == '\0' ) ||
		    ( to_upper(str1[istr]) != to_upper(str2[istr]) ))
			return(istr);
	}
	/*NOTREACHED*/
} /* end of ulcmpb */

/*+-------------------------------------------------------------------------
    ulindex:  Upper/Lower [case insensitive] Index functioni

  Returns position of 'str2' in 'str1' if found
  If 'str2' is null, then 0 is returned (null matches anything)
  Returns -1 if not found

  uses 'ulcmpb'
--------------------------------------------------------------------------*/
int ulindex(str1,str2)
register unsigned char	*str1;	/* the (target) string to search */
register unsigned char	*str2;	/* the (comparand) string to search for */
{
	register int istr1 = 0;		/* moving index into str1 */
	register unsigned char	*mstr = str1;	/* moving string pointer */

	if(str2[0] == '\0')             /* null string matches anything */
		return(0);
	while(1)
	{
		if(*mstr == '\0')           /* if we exhaust target string, flunk */
			return(-1);
		/* Can we find either case of first comparand char in target? */
		if( to_upper(*mstr) == to_upper(str2[0]) )
		{
			/* we have a first char match... does rest of string match? */
			if(ulcmpb(mstr,str2) == -1)         /* if the rest matches, ... */
				return(istr1);                  /* ... return match position */
		}
		/* we did not match this time... increment istr1, mstr and try again */
		++istr1;
		++mstr;
	}
}	/* end of ulindex */

/*+-------------------------------------------------------------------------
    main program
--------------------------------------------------------------------------*/
main(argc,argv)
int argc;
char **argv;
{
	int erc;                            /* error code */
	int iFiles;                         /* index into argv array (filenames) */
	int nFiles;                         /* number of input files */

	int fHaveIndFile;                   /* TRUE if input is from "@" file */
	int fIndFileEOF;                    /* TRUE if "@" input file EOF */
	int in_eof;                         /* TRUE if input file eof reached */
	int itmp;                           /* integer/index temp */
	int iargv;                          /* index into argv */
	int fOutputFile = FALSE;            /* if set TRUE, matching lines writen to 'out_fname'*/
	char	*eof_if_zero;                   /* end of file control flag */
	int fListAll;                       /* TRUE if '-a': list all comments */
	int fOutputLines;                   /* if TRUE, output source lines */
	int fCloseBraceStop;                /* if TRUE, end marker is '}' not star-slash */

	long out_line_count;                 /* number of lines output by program */

	char Files[N_SFILES_MAX][S_FNAME_MAX];   /* filenames */
	char in_buf[S_IN_BUF_MAX];               /* line input from @file or file being searched */

	char out_fname[S_FNAME_MAX];         /* "=o"utput file name */

	FILE    *in_fp;                         /* file for reading of search file(s) */
	FILE    *out_fp;                        /* file for output if -o specified */
	FILE    *cmd_fp;                        /* file for search string input if -i and ... */
	/* ... @file read if found */

	puts("... docxtr x1.1/Xenix by TuckerWare ...................");

	strcpy(out_fname,"/dev/tty");    /* default output file is console */
	out_line_count=0L;          /* output line counter to zero */
	fListAll=FALSE;             /* no -a switch yet */

	nFiles=0;

	for( iargv=1; iargv < argc ; ++iargv )  /* walk iargv from 1 to 'argc' */
	{
		if( (itmp=argv[iargv][0]) != '-')  /* if not switch argument */
		{
			if(itmp == '@')                 /* if @file */
			{
				cmd_fp=fopen(&argv[iargv][1],"r");      /* read the '@' file contents */
				if(cmd_fp == 0)
				{
					printf("Error opening @file %s.\n",&argv[iargv][1]);
					exit(1);
				}
				eof_if_zero= in_buf;          /* init error variable to non-error (non-zero) */
				while(eof_if_zero != NULL) /* while not @file input eof or error */
				{
					in_buf[0]='\0';
					eof_if_zero=fgets(in_buf,sizeof(in_buf)-1,cmd_fp); /* read filename */

					if(strlen(in_buf) < 2)  /* just a newline is empty */
						eof_if_zero = NULL;    /* and we simulate eof in such cases */

					if(eof_if_zero == NULL)        /* if eof or error, ... */
					{
						continue;               /* ... continue... while will end loop */
					}

					if(nFiles == N_SFILES_MAX)          /* we have an element... can we use it? */
					{
						printf("...Maximum files (%d) exceeded.\n",N_SFILES_MAX);
						exit(1);
					}

					itmp=strlen(in_buf);        /* strip trailing newline ... */
					in_buf[--itmp]='\0';        /* ... from input record */
					strncpy(Files[nFiles++],in_buf,S_FNAME_MAX-1);      /* copy filename from "stolen" buffer */
					printf("\nFile to be searched:  %s",in_buf);
				}/* end of while not @file eof */

				puts("");
				fclose(cmd_fp);
				continue;
			}/* end of if @file */


			if(nFiles == N_SFILES_MAX)
			{
				printf("...Maximum files (%d) exceeded.\n",N_SFILES_MAX);
				exit(1);
			}

			strncpy(Files[nFiles++],argv[iargv],S_FNAME_MAX-1);
			continue;
		} /* end of if not switch arg */
		else /* switch argument */
		{
			if(to_upper(argv[iargv][1])=='O')    /* output switch directive */
			{
				strncpy(out_fname,&argv[iargv][2],sizeof(out_fname)-1);
				fOutputFile=TRUE;
				continue;
			}

			if(to_upper(argv[iargv][1])=='A')    /* if found, we are to output all comments */
			{
				fListAll=TRUE;
				continue;
			}

			if(to_upper(argv[iargv][1])=='?')    /* input switch directive */
			{
				puts("Copyright (C) 1985 Warren H. Tucker d/b/a TuckerWare");
				continue;
			}

		} /* end of else switch arg */

	} /* end of walk iargv */

	/* convert filenames to all upper case (not nice in some systems, but ok for intended MS-DOS) */
#ifdef MSDOS

	for(iFiles=0; iFiles < nFiles; ++iFiles)
	{
		for(itmp=0 ; Files[nFiles-1][itmp] != '\0' ; ++itmp)
		{
			Files[iFiles][itmp]=to_upper(Files[iFiles][itmp]);
		}
	}
#endif


	if(nFiles == 0)     /* if there are no input files, give help */
	{
		puts("DOCXTR scans a list of search files, extracting documentation enclosed in");
		puts("specially marked comment groups (beginning with '/*+' and ending with '*/'.");
		puts("A line containing a three-char '/*+' substring will be output, as well as");
		puts("all subsequent lines until the substring '*/' is found (or end of file).");
		puts("Similarly, lines containing '/*{' cause lines to be printed until a '}'.");

		puts("\nExample usage:   DOCXTR MODA.H MODB.C MODC.C\n");

		puts("Switches allow you to modify program operation:");
		puts("-a      ALL lines containing comments are extracted.");
		puts("-ofname causes a copy of the output to be routed to 'fname'");
		puts("@fname  causes a list of search files to be read from 'fname'");
		puts("A blank line read by DOCXTR is equivalent to end of file.");

		puts("\nFor example:  DOCXTR @FNAMES.LST ANOTHER.C -okeep.file:\n");

		exit(0);
	}

	/* if outputing to file, open it */
	if(fOutputFile )
	{
		out_fp=fopen(out_fname,"w");
		if(out_fp == 0)
		{
			printf("Error opening output file %s.\n",out_fname);
			exit(1);
		}
	}

	/* walk thru the files one by one */
	for(iFiles=0 ; iFiles < nFiles ; ++iFiles)
	{
		in_eof=FALSE;                           /* reset eof indicator */
		in_fp=fopen(Files[iFiles],"r");         /* open the target file */
		if(in_fp == 0)  /* if fopen returns 0 file number, error occurred */
		{
			printf("Error opening file %d:  %s\n",iFiles,Files[iFiles]);
			goto Next_Input_File;   /* jumping down the page */
		}

		printf(".... Extracting from %s ......................\n",Files[iFiles]);
		if( fOutputFile  )
		{
			fprintf(out_fp,".... Extracting from %s ......................\n",Files[iFiles]);
		}

		fOutputLines = FALSE;       /* do not output lines yet */
		fCloseBraceStop=FALSE;      /* set to star-slash end trigger rather than '}' */

		/************ per file processing goes below here **************/

		while(!in_eof)
		{
			if( (fgets(in_buf,sizeof(in_buf)-1,in_fp)) == 0)
			{
				in_eof=TRUE;
				continue;
			}

			/* if starting trigger on a line, set up to be printed */
			if( ulindex(in_buf,"/*{") == 0 )
			{
				fOutputLines=TRUE;          /* start outputing lines */
				fCloseBraceStop=TRUE;       /* and dont stop until '}' */
			}

			if((ulindex(in_buf,"/*+") == 0) &&
			    (ulindex(in_buf,":EDITS:") == -1))
				fOutputLines = TRUE;

			if( (fListAll )
			    && (ulindex(in_buf,"/*") != -1 ) )
				fOutputLines = TRUE;

			if( fOutputLines && (strncmp(in_buf,"/*+----------",10) != 0)  )
			{
				fputs(in_buf,stdout);
				if(fOutputFile )
				{
					fputs(in_buf,out_fp);
				}
				++out_line_count;
			}

			/* if ending trigger found, even on line just printed, turn off printing */
			if(    (fCloseBraceStop == FALSE)      /* unless looking only for close brace */
			    && (ulindex(in_buf,"*/") != -1)  )
			{
				fOutputLines=FALSE;
			}

			if(    (fCloseBraceStop )
			    && (ulindex(in_buf,"}") != -1)  )
			{
				fOutputLines=FALSE;
				fCloseBraceStop=FALSE;
			}


		} /* end of while !in_eof */

		/************ per file processing goes above here **************/

Close_Input_File:
		if( fOutputFile  )
			fputs("\n",out_fp);
		erc=fclose(in_fp);                  /* close the target file */
		if(erc==-1)
		{
			printf("Error closing file %d:  %s\n",iFiles,Files[iFiles]);
		}

Next_Input_File:
		continue;
	} /* end of for(iFiles=1...nFiles) */

	printf("\nTotal output lines:  %ld\n",out_line_count);
	if( fOutputFile )
	{
		fprintf(out_fp,"\nTotal output lines:  %ld\n",out_line_count);
		fclose(out_fp);
	}
	exit(0);

}

/* vi: set tabstop=4 shiftwidth=4: */
