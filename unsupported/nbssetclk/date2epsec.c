
/*+-------------------------------------------------------------------------
	date2epsec.c - convert date to standard UNIX "seconds since epoch"
	...!gatech!kd4nc!n4hgf!wht

Use -DUTC_CLK if date supplied is UTC, otherwise date supplied is
treated as expected to be local time
--------------------------------------------------------------------------*/
/*+:EDITS:*/
/*:01-24-1997-02:38-wht@yuriatin-SOURCE RELEASE 4.00 */
/*:01-28-1990-04:55-wht-working on cmostime version 4 */

#include <sys/types.h>
#include <time.h>

/*+-------------------------------------------------------------------------
	leap_year(yr) -- returns 1 if leap year, 0 otherwise 
--------------------------------------------------------------------------*/
int
leap_year(yr)
int yr;
{
	return((yr % 4 == 0) && (yr % 100 != 0) || (yr % 400 == 0));
}	/* end of leap_year */

/*+-------------------------------------------------------------------------
	days_since_epoch(year,month,day)

Return the number of days between Jan 1, 1970 and year,month,day
year is full year (e.g., 1988)
month is 0-11
day is 0-30 (must be legal day in month)
--------------------------------------------------------------------------*/
int
days_since_epoch(year,month,day)
int year;
int month;
register int day;
{
register int mm;
register int yy;
static char days_per_month[] = {31,28,31,30,31,30,31,31,30,31,30,31};

	day++;			/* adjust for day 0-n instead of 1-n */

	for(yy = 1970; yy < year; ++yy) 
	{
		day += 365;
		if(leap_year(yy))
			day++;
	}
	for(mm = 0; mm < month; ++mm)
		day += days_per_month[mm] + (mm == 1 && leap_year(yy));

	return(day);

}	/* end of days_since_epoch */

/*+-------------------------------------------------------------------------
	date_to_epochsecs(year,month,day,hour,min,sec)

Convert arguments into long seconds since 1/1/1970
year is full year (e.g., 1988)
month is 0-11
day is 0-30 (must be legal day in month)
hour is 0-23, min and sec 0-59
--------------------------------------------------------------------------*/
long
date_to_epochsecs(year,month,day,hour,min,sec)
int year;
int month;
int day;
int hour;
int min;
int sec;
{
register int m1;
register int m2;
long t;
struct tm *tp;
struct tm *localtime();

	t = (days_since_epoch(year,month,day) - 1) * 86400L
		+ (hour * 3600L) + (min * 60L) + sec;

#if !defined(UTC_CLK)
	/* correct for the time zone */
	tp = localtime(&t);
	m1 = tp->tm_hour * 60 + tp->tm_min;
	m2 = (hour * 60) + min;
	t -= ((m1 - m2 + 720 + 1440) % 1440 - 720) * 60L;
#endif

	return(t);

}	/* end of date_to_epochsecs */

/* vi: set tabstop=4 shiftwidth=4: */
/* end of date2epsec.c */
