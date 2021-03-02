/* vi: set tabstop=4 shiftwidth=4: */
/*+-------------------------------------------------------------------------
	dev_rtc.h
	...!gatech!kd4nc!n4hgf!wht
--------------------------------------------------------------------------*/
/*+:EDITS:*/
/*:01-24-1997-02:38-wht@yuriatin-SOURCE RELEASE 4.00 */
/*:01-28-1990-04:55-wht-working on cmostime version 4 */

#include <sys/rtc.h>

#define RTCNAME "/dev/rtc"

typedef union rtcu_t
{
	unsigned char b[RTC_NREG];
	struct rtc_t s;
} RTCU_T;

/* end of dev_rtc.h */
