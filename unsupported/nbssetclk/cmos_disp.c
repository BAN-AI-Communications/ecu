/* vi: set tabstop=4 shiftwidth=4: */
/*+-------------------------------------------------------------------------
	cmos_disp.c -- interprets /dev/cmos 286/386 AT CMOS ram
	...!gatech!kd4nc!n4hgf!wht

It is beyond me how the checksumis actually calculated.  I had only
some pooooooor clues from an old IBM AT Technical reference to go
on.  Also, I get zero floppy drives when I run this, but I'm looking
at one as I write this and it works :-).  Oh well, at least the time-
related stuff jives.
--------------------------------------------------------------------------*/
/*+:EDITS:*/
/*:01-24-1997-02:38-wht@yuriatin-SOURCE RELEASE 4.00 */
/*:01-28-1990-04:55-wht-working on cmostime version 4 */

#include <stdio.h>
#include <fcntl.h>
#include "dev_cmos.h"

union cmos_union	cmos;

char	*day_of_week[] =
{
	"Sun","Mon","Tue","Wed","Thu","Fri","Sat"
};

char	*cmosname = "/dev/cmos";

main(argc,argv)
int argc;
char	**argv;
{
register int icmos;
register int itmp;
char	*format;
int fdcmos;
int clock_bcd_mode;			/* true if clock in bcd mode */
unsigned short calc_cksum;		/* calculated checksum */
unsigned short cmos_cksum;		/* checksum in cmos */

	if((fdcmos = open(cmosname,O_RDONLY)) < 0)
	{
		perror(cmosname);
		exit(1);
	}
	get_cmos(fdcmos,&cmos);
	close(fdcmos);

	for(icmos = 0; icmos < 64; icmos++)
	{
		if(icmos % 16 == 0)
			printf("\n %02x:  ",icmos);
		printf("%02x ",cmos.a[icmos]);
		if(icmos % 4 == 3)
			printf(" ");
	}
	printf("\n\n");

	clock_bcd_mode = (cmos.s.srB & SRB_DM) ? 0 : 1;

	itmp = cmos.s.base_high << 8 | cmos.s.base_low;
	printf("Base memory:       %6dk  (%8lx bytes hex)\n",
					itmp,(long)itmp * (long)1024);

	itmp = cmos.s.expand_high << 8 | cmos.s.expand_low;
	printf("Expansion memory:  %6dk  (%8lx bytes hex)\n",
					itmp,(long)itmp * (long)1024);

	itmp = cmos.s.exp2_high << 8 | cmos.s.exp2_low;
	printf("Memory above 1MB:  %6dk  (%8lx bytes hex)\n",
					itmp,(long)itmp * (long)1024);

	printf("\n");

	printf("Diag status byte: %02x  Shutdown status byte: %02x\n",
					cmos.s.dsb,cmos.s.ssb);

	if(cmos.s.dsb)
	{
		itmp = cmos.s.dsb;
		printf("  Diagnostic Status Byte (DSB) says:\n");
		if(itmp & 0x80)
			printf("      Real-time clock has lost power.\n");
		if(itmp & 0x40)
			printf("      Bad configuration record checksum.\n");
		if(itmp & 0x20)
			printf("      Incorrect configuration information.\n");
		if(itmp & 0x10)
			printf("      Memory size miscompare.\n");
		if(itmp & 0x08)
			printf("      Fixed disk adapter failed init.\n");
		if(itmp & 0x04)
			printf("      Time is invalid.\n");
		printf("\n");
	}

	printf("Clock: %d-hour mode, %s time, alarm %s, %s mode\n",
					(cmos.s.srB & SRB_24HR) ? 24 : 12,
					(cmos.s.srB & SRB_DSE) ? "daylight" : "standard",
					(cmos.s.srB & SRB_AIE) ? "enabled" : "disabled",
					(clock_bcd_mode) ? "bcd" : "hex");

	if(clock_bcd_mode)
		format = "Date: %02x-%02x-%02x%02x  ";
	else
		format = "Date: %02d-%02d-%02d%02d  ";
	printf(format,cmos.s.month,cmos.s.date,cmos.s.century,cmos.s.year);

	if(clock_bcd_mode)
		format = "Time: %02x:%02x:%02x  Alarm: %02x:%02x:%02x\n";
	else
		format = "Time: %02d:%02d:%02d  Alarm: %02d:%02d:%02d\n";
	printf(format,cmos.s.hour,cmos.s.min,cmos.s.sec,
		cmos.s.hour_alarm,cmos.s.min_alarm,cmos.s.sec_alarm);

	printf("Number of diskette drives: %d   Math coprocessor %s\n",
					(int)(cmos.s.equip & 0xC0) >> 6,
					(cmos.s.equip & 0x02) ? "present" : "not present"
					);

	printf("Primary display:  ");
	switch( itmp = (cmos.s.equip & 0x30) >> 4)
	{
		case 1:
			format = "Color/Graphics 40 column\n";
			break;
		case 2:
			format = "Color/Graphics 80 column\n";
			break;
		case 3:
			format = "Monochrome display\n";
			break;
		default:
			format = "unknown type: %d\n";
			break;
	}
	printf(format,itmp);

/* tech ref doesn't say: empirically determined checksum algorithm (correct?) */
	calc_cksum = 0;
	for(itmp = 0x10; itmp < 0x21; itmp++)
		calc_cksum += cmos.a[itmp];
	calc_cksum++;

	cmos_cksum = (cmos.s.cksum_high << 8) | cmos.s.cksum_low;
	printf("Checksum: in ram: %04x  calculated: %04x\n",cmos_cksum,calc_cksum);

	exit(0);	/* for now */
	
}	/* end of main() */

