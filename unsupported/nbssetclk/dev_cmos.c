/* vi: set tabstop=4 shiftwidth=4: */
/*+-------------------------------------------------------------------------
	dev_cmos.c -- XENIX /dev/cmos routines
	...!gatech!kd4nc!n4hgf!wht

  Defined functions:
	get_clock(year,month,day,hour,min,sec)
	get_cmos(fdcmos,cmosbuf)
	set_clock(year,month,day,hour,min,sec)

--------------------------------------------------------------------------*/
/*+:EDITS:*/
/*:01-24-1997-02:38-wht@yuriatin-SOURCE RELEASE 4.00 */
/*:01-28-1990-04:55-wht-working on cmostime version 4 */

#include <fcntl.h>
#include "dev_cmos.h"

#define SEEKPOS(elem)	((long)((char *)&cu.s.elem - (char *)&cu))

unsigned char bcdch_to_uchar();
unsigned char uchar_to_bcdch();

char *cmos = "/dev/cmos";
union cmos_union cu;

/*+-------------------------------------------------------------------------
	get_cmos(fdcmos,cmosbuf)
--------------------------------------------------------------------------*/
void
get_cmos(fdcmos,cmosbuf)
int fdcmos;
union cmos_union *cmosbuf;
{
unsigned int wait_counter;
unsigned char stat_reg_A;

/* check for clock update in progress and wait til done */
	wait_counter = 32768;
	while(1)
	{
		if(--wait_counter == 0)
		{
			puts("/dev/cmos: clock update in progress for too long");
			exit(200);
		}
		
		lseek(fdcmos,SEEKPOS(srA),0);		/* position to status register A */
		read(fdcmos,&stat_reg_A,1);
		if(!(stat_reg_A & SRA_UIP))			/* check update bit */
			break;							/* and break out of loop if not */
	}

/* read the cmos ram */
	lseek(fdcmos,0L,0);
	read(fdcmos,(char *)cmosbuf,sizeof(AT_CMOS));

}	/* end of get_cmos */

/*+-------------------------------------------------------------------------
	get_clock(year,month,day,hour,min,sec) -- read /dev/cmos clock

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
int fdcmos;

	if((fdcmos = open(cmos,O_RDWR,0)) < 0)
	{
		perror(cmos);
		exit(251);
	}

	get_cmos(fdcmos,&cu);

	*year  = (int)bcdch_to_uchar(cu.s.year) + 1900;
	*month = (int)bcdch_to_uchar(cu.s.month) - 1;
	*day   = (int)bcdch_to_uchar(cu.s.date) - 1;
	*hour  = (int)bcdch_to_uchar(cu.s.hour);
	*min   = (int)bcdch_to_uchar(cu.s.min);
	*sec   = (int)bcdch_to_uchar(cu.s.sec);
	close(fdcmos);

}	/* end of get_clock */

/*+-------------------------------------------------------------------------
	set_clock(year,month,day,hour,min,sec) -- write /dev/cmos clock
--------------------------------------------------------------------------*/
void
set_clock(year,month,day,hour,min,sec)
int year;
int month;
int day;
int hour;
int min;
{
unsigned char stat_reg_B;
unsigned char wchar;
int fdcmos;

	if((fdcmos = open(cmos,O_RDWR,0)) < 0)
	{
		perror(cmos);
		exit(251);
	}

	month++;			/* month to 1-n */
	year %= 100;		/* think ahead (ha) */

/* set no tick advance */
	lseek(fdcmos,SEEKPOS(srB),0);
	read(fdcmos,&stat_reg_B,1);		/* get current value */
	stat_reg_B |= SRB_SET;
	lseek(fdcmos,SEEKPOS(srB),0);
	write(fdcmos,&stat_reg_B,1);	/* plug new value */

/* set clock */
	wchar = uchar_to_bcdch((uchar)year);
	lseek(fdcmos,SEEKPOS(year),0);
	write(fdcmos,&wchar,1);

	wchar = uchar_to_bcdch((uchar)month);
	lseek(fdcmos,SEEKPOS(month),0);
	write(fdcmos,&wchar,1);

	wchar = uchar_to_bcdch((uchar)day);
	lseek(fdcmos,SEEKPOS(date),0);
	write(fdcmos,&wchar,1);

	wchar = uchar_to_bcdch((uchar)hour);
	lseek(fdcmos,SEEKPOS(hour),0);
	write(fdcmos,&wchar,1);

	wchar = uchar_to_bcdch((uchar)min);
	lseek(fdcmos,SEEKPOS(min),0);
	write(fdcmos,&wchar,1);

	wchar = uchar_to_bcdch((uchar)sec);
	lseek(fdcmos,SEEKPOS(sec),0);
	write(fdcmos,&wchar,1);
	
/* set no tick advance */
	lseek(fdcmos,SEEKPOS(srB),0);
	stat_reg_B &= ~SRB_SET;
	write(fdcmos,&stat_reg_B,1);	/* plug new value */

/* update 'time is invalid' is diag status byte */
	lseek(fdcmos,SEEKPOS(dsb),0);
	read(fdcmos,&wchar,1);
	wchar &= ~DSB_TSI;
	lseek(fdcmos,SEEKPOS(dsb),0);
	write(fdcmos,&wchar,1);
	if(lseek(fdcmos,SEEKPOS(srB),0) != SEEKPOS(srB))
	{
		perror("/dev/cmos seek 1");
		exit(1);
	}

	if(read(fdcmos,&wchar,1) != 1)
	{
		perror("/dev/cmos read");
		exit(1);
	}

	wchar |= SRB_24HR;

	if(lseek(fdcmos,SEEKPOS(srB),0) != SEEKPOS(srB))
	{
		perror("/dev/cmos seek 2");
		exit(1);
	}

	if(write(fdcmos,&wchar,1) != 1)
	{
		perror("/dev/cmos write");
		exit(1);
	}
	close(fdcmos);


}	/* end of set_clock */

