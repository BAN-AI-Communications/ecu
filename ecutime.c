/*+-------------------------------------------------------------------------
	ecutime.c -- ecu time-related functions
	wht@wht.net

  Defined functions:
	elapsed_time_text(elapsed_seconds)
	epoch_secs_to_str(epoch_secs, type, buf)
	get_day(zflag)
	get_month(zflag)
	timeofday_text(type, buf)
	tod_plus_msec_text()

  Hofstadter's Law: It will always take longer, even if you take
  into consideration Hofstadter's Law.  "Yeah, but July only
  seemed to last thirty minutes."

--------------------------------------------------------------------------*/
/*+:EDITS:*/
/*:04-26-2000-11:15-wht@bob-RELEASE 4.42 */
/*:12-12-1997-21:11-wht@kepler-complete isolation of substituted ftime */
/*:01-24-1997-02:37-wht@yuriatin-SOURCE RELEASE 4.00 */
/*:11-01-1996-18:30-wht@yuriatin-add type 11 to epoch_secs_to_str */
/*:09-11-1996-20:00-wht@yuriatin-3.48-major telnet,curses,structural overhaul */
/*:11-23-1995-11:20-wht@kepler-source control 3.37 for tsx-11 */
/*:11-14-1995-10:23-wht@kepler-3.37.80-source control point: SOCKETS */
/*:05-04-1994-04:39-wht@n4hgf-ECU release 3.30 */
/*:03-02-1994-05:27-wht@n4hgf-add tod_plus_msec_text */
/*:01-12-1994-07:17-wht@fep-move Ftime() to nap.c */
/*:12-12-1993-13:11-wht@fep-use ecu_time.h + gettimeofday-based Ftime clone */
/*:09-10-1992-13:59-wht@n4hgf-ECU release 3.20 */
/*:08-22-1992-15:38-wht@n4hgf-ECU release 3.20 BETA */
/*:07-25-1991-12:56-wht@n4hgf-ECU release 3.10 */
/*:08-14-1990-20:40-wht@n4hgf-ecu3.00-flush old edit history */

#include "ecu.h"

struct tm *gmtime();
struct tm *localtime();

/*+-------------------------------------------------------------------------
	get_month(zflag) - month 1-12 - zflag true for UTC (Z)), else local time
--------------------------------------------------------------------------*/
int
get_month(zflag)
int zflag;
{
	long epoch_secs = time((long *)0);
	struct tm *tod = (zflag) ? gmtime(&epoch_secs) : localtime(&epoch_secs);

	return (tod->tm_mon + 1);

}							 /* end of get_month */

/*+-------------------------------------------------------------------------
	get_day(zflag) - day 0-6 - zflag true for UTC (Z)), else local time
--------------------------------------------------------------------------*/
int
get_day(zflag)
int zflag;
{
	long epoch_secs = time((long *)0);
	struct tm *tod = (zflag) ? gmtime(&epoch_secs) : localtime(&epoch_secs);

	return (tod->tm_wday);

}							 /* end of get_day */

/*+-----------------------------------------------------------------------
	char *epoch_secs_to_str(epoch_secs,type,buf)

  time of day types:
	0		hh:mm
	1		hh:mm:ss
	2		mm-dd-yyyy hh:mm
	3		mm-dd-yyyy hh:mm:ss
	4		mm-dd-yyyy hh:mm:ss (UTC hh:mm)
	5		mm-dd-yyyy
	6		hh:mmZ
	7		hh:mm:ssZ
	8		mm-dd-yyyy          in UTC
    9		mm-dd

	returns 'buf' address

------------------------------------------------------------------------*/
char *
epoch_secs_to_str(epoch_secs, type, buf)
long epoch_secs;
int type;
char *buf;
{
	struct tm *tod = 0;

	switch(type)
	{
		case 0:
		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
		case 9:
			tod = localtime(&epoch_secs);
			break;
		case 6:
		case 7:
		case 8:
			tod = gmtime(&epoch_secs);
			break;
		default:
			pprintf("logic error in epoch_secs_to_str: %d\n",type);
			termecu(TERMECU_LOGIC_ERROR);
	}

	switch (type)
	{
		default:
		case 0:
		case 6:
			sprintf(buf, "%02d:%02d", tod->tm_hour, tod->tm_min);
			if (type == 6)
				strcat(buf, "Z");
			break;

		case 1:
		case 7:
			sprintf(buf, "%02d:%02d:%02d", tod->tm_hour,
				tod->tm_min, tod->tm_sec);
			if (type == 7)
				strcat(buf, "Z");
			break;

		case 2:
			sprintf(buf, "%02d-%02d-%04d %02d:%02d",
				tod->tm_mon + 1, tod->tm_mday, tod->tm_year + 1900,
				tod->tm_hour, tod->tm_min);
			break;

		case 3:
			sprintf(buf, "%02d-%02d-%04d %02d:%02d:%02d",
				tod->tm_mon + 1, tod->tm_mday, tod->tm_year + 1900,
				tod->tm_hour, tod->tm_min, tod->tm_sec);
			break;

		case 4:
			sprintf(buf, "%02d-%02d-%04d %02d:%02d:%02d",
				tod->tm_mon + 1, tod->tm_mday, tod->tm_year + 1900,
				tod->tm_hour, tod->tm_min, tod->tm_sec);
			tod = gmtime(&epoch_secs);
			sprintf(&buf[strlen(buf)], " (UTC %02d:%02d)",
				tod->tm_hour, tod->tm_min);
			break;

		case 5:
		case 8:
			sprintf(buf, "%02d-%02d-%04d",
				tod->tm_mon + 1, tod->tm_mday, tod->tm_year + 1900);
			break;

		case 9:
			sprintf(buf, "%02d-%02d", tod->tm_mon + 1, tod->tm_mday);
			break;


	}

	return (buf);
}							 /* end of epoch_secs_to_str */

/*+-----------------------------------------------------------------------
	char *timeofday_text(type,buf)

  time of day types: (for refernce only; see epoch_secs_to_str())
	0		hh:mm
	1		hh:mm:ss
	2		mm-dd-yyyy hh:mm
	3		mm-dd-yyyy hh:mm:ss
	4		mm-dd-yyyy hh:mm:ss (UTC hh:mm)
	5		mm-dd-yyyy
	6		hh:mmZ
	7		hh:mm:ssZ
	8		mm-dd-yyyy (UTC date)
    9		mm-dd

  returns 'buf' address

------------------------------------------------------------------------*/
char *
timeofday_text(type, buf)
int type;
char *buf;
{
	static char s128[128];
	if(!buf)
		buf = s128;
	return (epoch_secs_to_str(time((long *)0), type, buf));
}							 /* end of timeofday_text */

/*+-----------------------------------------------------------------------
	char *elapsed_time_text(elapsed_seconds)
	"hh:mm:ss" returned
  static string address is returned
------------------------------------------------------------------------*/
char *
elapsed_time_text(elapsed_seconds)
long elapsed_seconds;
{
	static char elapsed_time_str[40];
	long hh, mm, ss;

	hh = elapsed_seconds / 3600;
	elapsed_seconds -= hh * 3600;
	mm = elapsed_seconds / 60L;
	elapsed_seconds -= mm * 60L;
	ss = elapsed_seconds;

	sprintf(elapsed_time_str, "%02ld:%02ld:%02ld", hh, mm, ss);
	return (elapsed_time_str);
}							 /* end of elapsed_time_text */

/*+-------------------------------------------------------------------------
	tod_plus_msec_text()
--------------------------------------------------------------------------*/
char *
tod_plus_msec_text()
{
	static char buf[40];
	struct TIMEB tb;
	struct tm *tod = 0;

	Ftime(&tb);
	tod = localtime(&tb.time);
	sprintf(buf, "%02d:%02d:%02d.%03d",
		tod->tm_hour, tod->tm_min, tod->tm_sec, tb.millitm);
	return (buf);

}							 /* end of tod_plus_msec_text */

/* end of ecutime.c */
/* vi: set tabstop=4 shiftwidth=4: */
