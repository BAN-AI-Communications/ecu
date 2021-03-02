/* vi: set tabstop=4 shiftwidth=4: */
/*+-------------------------------------------------------------------------
	dev_rtc.c -- UNIX /dev/rtc routines
	...!gatech!kd4nc!n4hgf!wht

  Defined functions:
	get_clock(year,month,day,hour,min,sec)
	get_rtc(fdrtc,rtcbuf)
	set_clock(year,month,day,hour,min,sec)

--------------------------------------------------------------------------*/
/*+:EDITS:*/
/*:01-24-1997-02:38-wht@yuriatin-SOURCE RELEASE 4.00 */
/*:01-28-1990-04:55-wht-working on cmostime version 4 */

#include "dev_rtc.h"

unsigned char bcdch_to_uchar();
unsigned char uchar_to_bcdch();

char *rtc = "/dev/rtc";

/*+-------------------------------------------------------------------------
	get_rtc(fdrtc,rtcbuf)
--------------------------------------------------------------------------*/
void
get_rtc(fdrtc,rtcbuf)
int fdrtc;
RTCU_T *rtcbuf;
{

	(void)ioctl(fdrtc,RTCRTIME,(char *)&rtcbuf.s);

}	/* end of get_rtc */

/*+-------------------------------------------------------------------------
	get_clock(year,month,day,hour,min,sec) -- read /dev/rtc clock

return values:
year is full year (e.g., 1988)
month is 0-11
day is 0-30
hour is 0-23
min is 0-59
--------------------------------------------------------------------------*/
void
get_clock(year,month,day,hour,min,sec)
int *year;
int *month;
int *day;
int *hour;
int *min;
int *sec;
{
RTCU_T ru;
int fdrtc;

	if((fdrtc = open(rtc,O_RDWR,0)) < 0)
	{
		perror(rtc);
		exit(251);
	}

	get_rtc(fdrtc,&ru);

	*year  = (int)bcdch_to_uchar(ru.s.year) + 1900;
	*month = (int)bcdch_to_uchar(ru.s.month) - 1;
	*day   = (int)bcdch_to_uchar(ru.s.date) - 1;
	*hour  = (int)bcdch_to_uchar(ru.s.hour);
	*min   = (int)bcdch_to_uchar(ru.s.min);
	*sec   = (int)bcdch_to_uchar(ru.s.sec);
	close(fdrtc);

}	/* end of get_clock */

/*+-------------------------------------------------------------------------
	set_clock(year,month,day,hour,min,sec) -- write /dev/rtc clock
--------------------------------------------------------------------------*/
void
set_clock(year,month,day,hour,min,sec)
int year;
int month;
int day;
int hour;
int min;
{
RTC_T ru;
int fdrtc;

	if((fdrtc = open(rtc,O_RDWR,0)) < 0)
	{
		perror(rtc);
		exit(251);
	}

	month++;			/* month to 1-n */
	year %= 100;		/* think ahead (ha) */

	ru.s.year  = uchar_to_bcdch(uchar)year);
	ru.s.month = uchar_to_bcdch(uchar)month);
	ru.s.day   = uchar_to_bcdch(uchar)date);
	ru.s.hour  = uchar_to_bcdch(uchar)hour);
	ru.s.min   = uchar_to_bcdch(uchar)min);
	ru.s.sec   = uchar_to_bcdch(uchar)sec);
	ru.s.amin  = 1;
	ru.s.asec  = 1;
	(void)ioctl(fdrtc,RTCSTIME,(char *)&rtcbuf.s);
	close(fdrtc);

}	/* end of set_clock */

