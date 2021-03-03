/*+-------------------------------------------------------------------------
	logevent.c - log ecu event
	wht@wht.net

  Defined functions:
	logevent(pid, event_note)
	vlogevent(pid, format, va_alist)

--------------------------------------------------------------------------*/
/*+:EDITS:*/
/*:04-26-2000-11:16-wht@bob-RELEASE 4.42 */
/*:01-24-1997-02:37-wht@yuriatin-SOURCE RELEASE 4.00 */
/*:09-11-1996-20:00-wht@yuriatin-3.48-major telnet,curses,structural overhaul */
/*:08-11-1996-02:14-wht@kepler-add vlogevent */
/*:08-11-1996-02:10-wht@kepler-rename ecu_log_event to logevent */
/*:11-23-1995-11:20-wht@kepler-source control 3.37 for tsx-11 */
/*:11-14-1995-10:23-wht@kepler-3.37.80-source control point: SOCKETS */
/*:05-04-1994-04:39-wht@n4hgf-ECU release 3.30 */
/*:09-10-1992-13:59-wht@n4hgf-ECU release 3.20 */
/*:08-22-1992-15:39-wht@n4hgf-ECU release 3.20 BETA */
/*:08-21-1991-02:00-wht@n4hgf-sun does not have xenix locking - fix later */
/*:08-07-1991-14:23-wht@n4hgf-use static logname */
/*:07-25-1991-12:58-wht@n4hgf-ECU release 3.10 */
/*:09-19-1990-19:36-wht@n4hgf-logevent now gets pid for log from caller */
/*:08-14-1990-20:40-wht@n4hgf-ecu3.00-flush old edit history */

#include <stdio.h>
#if defined(USE_LOCKING)
#include <sys/locking.h>
#endif

#include <string.h>
#include <stdarg.h>

/* This must be a typedef not a #define! */
typedef char *pointer;
typedef int *intp;

/*+-------------------------------------------------------------------------
	logevent(pid,event_note)
--------------------------------------------------------------------------*/
void
logevent(pid, event_note)
int pid;
char *event_note;
{
	char s32[32];
	FILE *ecu_log_fp;
	static char logname[256] = "";

	if (!logname[0])
	{
		get_home_dir(logname);
		strcat(logname, "/.ecu/log");
	}
	if (ecu_log_fp = fopen(logname, "a"))
	{
#if defined(USE_LOCKING)
		locking(fileno(ecu_log_fp), LK_LOCK, 0L);
#endif
		timeofday_text(4, s32);
		s32[10] = '-';
		fputs(s32, ecu_log_fp);
		fprintf(ecu_log_fp, "-%05d-", pid);
		fputs(event_note, ecu_log_fp);
		fputs("\n", ecu_log_fp);
#if defined(USE_LOCKING)
		fflush(ecu_log_fp);
		locking(fileno(ecu_log_fp), LK_UNLCK, 0L);
#endif
		fclose(ecu_log_fp);
	}
}							 /* end of logevent */

void vlogevent(pid, format, va_alist) { } 
/*+-------------------------------------------------------------------------
	vlogevent(pid,format,va_alist)
--------------------------------------------------------------------------*/
/*void
vlogevent(pid, format, va_alist)
int pid;
char *format;
va_dcl
{
	va_list args;
	char c;
	char *tp;
	char tempfmt[64];
	char accum_string[256];
	char *dp = accum_string;

	va_start(args);

	tempfmt[0] = '%';
	while (c = *format++)
	{
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
	logevent(pid, accum_string);

}*/							 /* end of vlogevent */

/* vi: set tabstop=4 shiftwidth=4: */
/* end of logevent.c */
