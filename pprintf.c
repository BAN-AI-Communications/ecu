/*+-------------------------------------------------------------------------
	pprintf.c - procedure printf

  Defined functions:
	pprintf(format, va_alist)

  This module has been hacked a bit to work for ECU applications

--------------------------------------------------------------------------*/
/*+:EDITS:*/
/*:04-26-2000-11:16-wht@bob-RELEASE 4.42 */
/*:12-10-1997-02:42-wht@fep-ACCUM_MAX_ALLOWABLE practically fail-safe */
/*:01-24-1997-02:38-wht@yuriatin-SOURCE RELEASE 4.00 */
/*:09-11-1996-20:01-wht@yuriatin-3.48-major telnet,curses,structural overhaul */
/*:08-11-1996-02:11-wht@kepler-simplify */
/*:11-23-1995-11:20-wht@kepler-source control 3.37 for tsx-11 */
/*:11-14-1995-10:23-wht@kepler-3.37.80-source control point: SOCKETS */
/*:05-04-1994-04:39-wht@n4hgf-ECU release 3.30 */
/*:09-10-1992-14:00-wht@n4hgf-ECU release 3.20 */
/*:08-22-1992-15:39-wht@n4hgf-ECU release 3.20 BETA */
/*:08-21-1991-02:12-wht@n4hgf-handle char *sprintf */
/*:07-25-1991-12:59-wht@n4hgf-ECU release 3.10 */
/*:01-09-1991-22:31-wht@n4hgf-ISC port */
/*:08-14-1990-20:40-wht@n4hgf-ecu3.00-flush old edit history */

/* based on Portable vsprintf  by Robert A. Larson <blarson@skat.usc.edu> */

/* Copyright 1989 Robert A. Larson.
 * Distribution in any form is allowed as long as the author
 * retains credit, changes are noted by their author and the
 * copyright message remains intact.  This program comes as-is
 * with no warentee of fitness for any purpouse.
 *
 * Thanks to Doug Gwen, Chris Torek, and others who helped clarify
 * the ansi printf specs.
 *
 * Please send any bug fixes and improvments to blarson@skat.usc.edu .
 * The use of goto is NOT a bug.
 */

#if !defined(BUILDING_PROTOTYPES)

#include <stdio.h>
#include <varargs.h>

/* This must be a typedef not a #define! */
typedef char *pointer;
typedef int *intp;

/*+-------------------------------------------------------------------------
	pprintf(format,va_alist)
--------------------------------------------------------------------------*/
void
pprintf(format, va_alist)
char *format;
va_dcl
{
	va_list args;
	char c;
	char *tp;
	char tempfmt[64];
#define ACCUM_MAX_ALLOWABLE 1024
    char accum_string[ACCUM_MAX_ALLOWABLE + 1024]; /* fairly fail-safe */
	char *dp = accum_string;

	va_start(args);

	tempfmt[0] = '%';
	while (c = *format++)
	{
        if(((int)(accum_string - dp)) > ACCUM_MAX_ALLOWABLE)
            break;

		if (c == '%')
		{
			tp = &tempfmt[1];
		  continue_format:
			switch (c = *format++)
			{
				case 's':
					*tp++ = c;
					*tp = '\0';
					sprintf(dp, tempfmt, va_arg(args, char *));

					dp += strlen(dp);
					break;
				case 'u':
				case 'x':
				case 'o':
				case 'X':
#if defined(UNSIGNEDSPECIAL)
					*tp++ = c;
					*tp = '\0';
					sprintf(dp, tempfmt, va_arg(args, unsigned));

					dp += strlen(dp);
					break;
#endif
				case 'd':
				case 'c':
				case 'i':
					*tp++ = c;
					*tp = '\0';
					sprintf(dp, tempfmt, va_arg(args, int));

					dp += strlen(dp);
					break;
				case 'f':
				case 'e':
				case 'E':
				case 'g':
				case 'G':
					*tp++ = c;
					*tp = '\0';
					sprintf(dp, tempfmt, va_arg(args, double));

					dp += strlen(dp);
					break;
				case '-':
				case '+':
				case '0':
				case '1':
				case '2':
				case '3':
				case '4':
				case '5':
				case '6':
				case '7':
				case '8':
				case '9':
				case '.':
				case ' ':
				case '#':
				case 'h':
					*tp++ = c;
					goto continue_format;
				case 'l':
					goto continue_format;
				case '*':
					sprintf(tp, "%d", va_arg(args, int));

					tp += strlen(tp);
					goto continue_format;
				case '%':
				default:
					*dp++ = c;
					break;
			}
		}
		else
			*dp++ = c;
	}
	*dp = '\0';
	va_end(args);
	pputs(accum_string);

}							 /* end of pprintf */

#endif /* !defined(BUILDING_PROTOTYPES) */

/* vi: set tabstop=4 shiftwidth=4: */
/* end of pprintf.c */
