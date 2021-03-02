/*+-------------------------------------------------------------------------
	rtc_disp.c - display /dev/rtc under SCO or ISC UNIX V 3.2
	...!gatech!kd4nc!n4hgf!wht
--------------------------------------------------------------------------*/
/*+:EDITS:*/
/*:01-24-1997-02:38-wht@yuriatin-SOURCE RELEASE 4.00 */
/*:01-28-1990-04:55-wht-working on cmostime version 4 */

#include <stdio.h>
#include <fcntl.h>
#include <time.h>
#include "dev_rtc.h"

char *rtc = RTCNAME;

RTCU_T rtcbuf;

/*+-------------------------------------------------------------------------
	main(argc,argv)
--------------------------------------------------------------------------*/
main(argc,argv)
int argc;
char **argv;
{
register itmp;
int fdrtc = open(rtc,O_RDONLY,0);
char *e;
char *d;
char *binf;
char *bcdf;

	e = "enabled\n";
	d = "disabled\n";
	binf = "date: %02d/%02d/%02d time: %02d:%02d:%02d alarm: %02d:%02d\n";
	bcdf = "date: %02x/%02x/%02x time: %02x:%02x:%02x alarm: %02x:%02x\n";

	if(fdrtc < 0)
	{
		perror(rtc);
		exit(1);
	}

	ioctl(fdrtc,RTCRTIME,&rtcbuf.s);

	printf("\n%s ",rtc);
	for(itmp = 0; itmp < RTC_NREG; itmp++)
		printf("%02x ",rtcbuf.b[itmp]);
	printf("\n\n");

	printf((rtcbuf.b[RTC_B] & RTC_DM) ? binf : bcdf,
		(unsigned)rtcbuf.s.rtc_mon & 0xFF,
		(unsigned)rtcbuf.s.rtc_dom & 0xFF,
		(unsigned)rtcbuf.s.rtc_yr & 0xFF,
		(unsigned)rtcbuf.s.rtc_hr & 0xFF,
		(unsigned)rtcbuf.s.rtc_min & 0xFF,
		(unsigned)rtcbuf.s.rtc_sec & 0xFF,
		(unsigned)rtcbuf.s.rtc_ahr & 0xFF,
		(unsigned)rtcbuf.s.rtc_amin & 0xFF);

	if(rtcbuf.b[RTC_A] & RTC_UIP)
		fputs("Update in progress, ",stdout);
	if(rtcbuf.b[RTC_A] & RTC_DIV0)
		fputs("Time base 4.194304 MHz, ",stdout);
	if(rtcbuf.b[RTC_A] & RTC_DIV1)
		fputs("Time base 1.048576 MHz, ",stdout);
	if(rtcbuf.b[RTC_A] & RTC_DIV2)
		fputs("Time base 32.768 KHz, ",stdout);
	if(rtcbuf.b[RTC_A] & RTC_RATE6)
		fputs("interrupt rate 976.562\n",stdout);
	else
		printf("interrupt rate code %x\n",rtcbuf.b[RTC_A] & 0x0F);

	if(rtcbuf.b[RTC_B] & RTC_SET)
		fputs("Updates stopped for time set\n",stdout);

	fputs("Periodic interrupt ",stdout);
	if(rtcbuf.b[RTC_B] & RTC_PIE)
		fputs(e,stdout);
	else
		fputs(d,stdout);

	fputs("Alarm interrupt ",stdout);
	if(rtcbuf.b[RTC_B] & RTC_AIE)
		fputs(e,stdout);
	else
		fputs(d,stdout);

	fputs("Update ended interrupt ",stdout);
	if(rtcbuf.b[RTC_B] & RTC_UIE)
		fputs(e,stdout);
	else
		fputs(d,stdout);

	fputs("Square wave ",stdout);
	if(rtcbuf.b[RTC_B] & RTC_SQWE)
		fputs(e,stdout);
	else
		fputs(d,stdout);
	if(rtcbuf.b[RTC_B] & RTC_DM)
		fputs("binary, ",stdout);
	else
		fputs("BCD, ",stdout);
	if(rtcbuf.b[RTC_B] & RTC_HM)
		fputs("24 hour, ",stdout);
	else
		fputs("12 hour, ",stdout);
	fputs("daylight savings ",stdout);
	if(rtcbuf.b[RTC_B] & RTC_DSE)
		fputs(e,stdout);
	else
		fputs(d,stdout);

	fputs("\n",stdout);
	exit(0);
}	/* end of main */

/* vi: set tabstop=4 shiftwidth=4: */
/* end of rtc_disp.c */
