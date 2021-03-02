/* vi: set tabstop=4 shiftwidth=4: */
/*+-------------------------------------------------------------------------
	dev_cmos.h -- AT 286/386 cmos ram definitions (some problems in here)
	...!gatech!kd4nc!n4hgf!wht

Some fields don't jive with reality; but there again, manufacturers
differ in how they use some of the ram...  This is good enough for
time-related stuff.
--------------------------------------------------------------------------*/
/*+:EDITS:*/
/*:01-24-1997-02:38-wht@yuriatin-SOURCE RELEASE 4.00 */
/*:01-28-1990-04:55-wht-working on cmostime version 4 */

#ifndef uchar
#define uchar	unsigned char
#endif

typedef struct at_cmos
{
	uchar	sec;
	uchar	sec_alarm;
	uchar	min;
	uchar	min_alarm;
	uchar	hour;
	uchar	hour_alarm;
	uchar	day;			/* 0-6 */
	uchar	date;			/* 1-31 */
	uchar	month;
	uchar	year;
	uchar	srA;			/* status register A */
	uchar	srB;			/* status register B */
	uchar	srC;			/* status register C */
	uchar	srD;			/* status register D */

	uchar	dsb;			/* diagnostic status byte */
	uchar	ssb;			/* shutdown status byte */
	uchar	ddtb;			/* diskette drive type byte */
	uchar	rsvd11;			/* byte at address 0x11 reserved */
	uchar	fdtb;			/* fixed disk type byte */
	uchar	rsvd13;			/* byte at address 0x13 reserved */
	uchar	equip;			/* equipment byte */

/* base size of memory = (contents(byte 0x16) << 8) + contents(byte 0x15) */
	uchar	base_low;		/* low base memory byte */
	uchar	base_high;		/* hi base memory byte */
	
/* memory expansion size similar calculation (0x17 low order byte, 0x18 hi) */
	uchar	expand_low;
	uchar	expand_high;

/* reserved bytes */
	uchar	rsvd19_2D[ 0x2D - 0x19 + 1 ];

/* checksum (on bytes in addresses 0x10 - 0x20) */
	uchar	cksum_high;		/* tech ref sez these bytes in other order */
	uchar	cksum_low;

/* > 1 mb expansion size (in cmos ram addresses 0x30-31) */
	uchar	exp2_low;
	uchar	exp2_high;

	uchar	century;		/* date century byte */
	uchar	info_flag;		/* see IF_ bits below */

	uchar	rsvd34_3F[ 0x3F - 0x34 + 1];

} AT_CMOS;

union cmos_union
{
	uchar		a[64];
	AT_CMOS		s;
};

/*+-----------------------------------------------------------------------
	AT Real Time Clock Status Register bit assignments
------------------------------------------------------------------------*/
#define SRA_UIP		0x80	/* update in progress */
#define SRA_DV22	0x70	/* 3-bit time-base divider */
#define SRA_DVOUT	0x0F	/* 4-bit time-base divider output freq */

#define SRB_SET		0x80	/* when 1, no tick advance (set mode) */
#define SRB_PIE		0x40	/* when 1, periodic interrupt */
#define SRB_AIE		0x20	/* when 1, alarm enabled */
#define SRB_UIE		0x10	/* enabled update-ended interrupt */
#define SRB_SQWE	0x08	/* square wave enabled */
#define SRB_DM		0x04	/* data mode 1==binary, 0==BCD */
#define SRB_24HR	0x02	/* 1==24 hour mode, 0==12 hour mode */
#define SRB_DSE		0x01	/* daylight savings time enabled */

#define SRC_IRQF	0x80
#define SRC_PF		0x40
#define SRC_AF		0x20
#define SRC_UF		0x10
							/* remaining bits in status register C reserved */

#define SRD_VRB		0x80	/* valid ram (==0 when power has been lost)
							 * remaining bits in status register D reserved */

/*+-----------------------------------------------------------------------
	Diagnostic Status Byte
------------------------------------------------------------------------*/
#define DSB_RTLOST	0x80	/* ==1 if real time clock chip lost power */
#define DSB_CSI		0x40	/* checksum status indicator == 1 if bad */
#define DSB_ICI		0x20	/* invalid configuration information (if ==1) */
#define DSB_MSM		0x10	/* memory size miscompare (==1 if different) */
#define DSB_HDBAD	0x08	/* fixed disk bad if == 1 */
#define DSB_TSI		0x04	/* time status indicator (==1 if time bad) */

#ifdef NEEDED	/* not under XENIX! */
#define		cmos_ctrl_dev		0x70		/* cmos address control port */
#define		cmos_data_dev		0x71		/* cmos data i/o port */
#endif

