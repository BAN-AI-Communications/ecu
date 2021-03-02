/* CHK=0x2A4F */
/*+-------------------------------------------------------------------------
	mors.c - SCO UNIX/Xenix morse code driver

  Defined functions:
	morsclose(dev)
	morse_gen(mchar)
	morse_load_timer2()
	morse_tone(ticks)
	morsioctl(dev, cmd, arg, mode)
	morsopen(dev, flag)
	morsread(dev)
	morswrite(dev)

--------------------------------------------------------------------------*/
/*+:EDITS:*/
/*:01-24-1997-02:38-wht@yuriatin-SOURCE RELEASE 4.00 */
/*:11-08-1990-13:26-wht@n4hgf-add morse_silent and friends */
/*:11-27-1989-20:15-wht-now working on UNIX/386 too */
/*:12-31-1988-15:13-wht-start work for 386 support */
/*:11-06-1988-12:57-wht-creation (inspired by Marc de Groot speaker driver) */

#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/param.h>
#if defined(M_I386)
#include <sys/page.h>
#include <sys/seg.h>
#endif
#include <sys/signal.h>
#include <sys/dir.h>
#include <sys/user.h>
#include <sys/errno.h>

#include "morse_dvr.h"

#define SP_TIMER		0x40
#define SP_PORTB		0x61
#define XTAL_FREQ		1193180L

typedef union morse_arg
{
	unsigned int iarg;
}
IOCTL_ARG;

int morse_in_use = 0;
int morse_silent = 0;
unsigned int morse_freq_div;
unsigned int morse_ticks;

unsigned char morse_qualify[] =
{
	0xFF,					 /* nul */
	37,						 /* soh */
	38,						 /* stx */
	0xFF,					 /* etx */
	0xFF,					 /* eot */
	0xFF,					 /* enq */
	0xFF,					 /* ack */
	40,						 /* bel */
	0xFF,					 /* bs  */
	36,						 /* ht  */
	36,						 /* nl  */
	0xFF,					 /* vt  */
	0xFF,					 /* np  */
	36,						 /* cr  */
	0xFF,					 /* so  */
	0xFF,					 /* si  */
	0xFF,					 /* dle */
	0xFF,					 /* dc1 */
	0xFF,					 /* dc2 */
	0xFF,					 /* dc3 */
	0xFF,					 /* dc4 */
	0xFF,					 /* nak */
	0xFF,					 /* syn */
	0xFF,					 /* etb */
	0xFF,					 /* can */
	0xFF,					 /* em  */
	0xFF,					 /* sub */
	39,						 /* esc */
	0xFF,					 /* fs  */
	0xFF,					 /* gs  */
	0xFF,					 /* rs  */
	0xFF,					 /* us  */
	36,						 /* sp  */
	0xFF,					 /* !  */
	0xFF,					 /* "  */
	0xFF,					 /* #  */
	0xFF,					 /* $  */
	0xFF,					 /* %  */
	0xFF,					 /* &  */
	0xFF,					 /* '  */
	0xFF,					 /* (  */
	0xFF,					 /* )  */
	0xFF,					 /* *  */
	0xFF,					 /* +  */
	43,						 /* , */
	0xFF,					 /* -  */
	40,						 /* .  */
	42,						 /* /  */
	26,						 /* 0  */
	27,						 /* 1  */
	28,						 /* 2  */
	29,						 /* 3  */
	30,						 /* 4  */
	31,						 /* 5  */
	32,						 /* 6  */
	33,						 /* 7  */
	34,						 /* 8  */
	35,						 /* 9  */
	0xFF,					 /* :  */
	0xFF,					 /* ;  */
	0xFF,					 /* <  */
	0xFF,					 /* =  */
	0xFF,					 /* >  */
	41,						 /* ?  */
	0xFF,					 /* @  */
	0,						 /* A  */
	1,						 /* B  */
	2,						 /* C  */
	3,						 /* D  */
	4,						 /* E  */
	5,						 /* F  */
	6,						 /* G  */
	7,						 /* H  */
	8,						 /* I  */
	9,						 /* J  */
	10,						 /* K  */
	11,						 /* L  */
	12,						 /* M  */
	13,						 /* N  */
	14,						 /* O  */
	15,						 /* P  */
	16,						 /* Q  */
	17,						 /* R  */
	18,						 /* S  */
	19,						 /* T  */
	20,						 /* U  */
	21,						 /* V  */
	22,						 /* W  */
	23,						 /* X  */
	24,						 /* Y  */
	25,						 /* Z  */
	0xFF,					 /* [  */
	0xFF,					 /* \  */
	0xFF,					 /* ]  */
	0xFF,					 /* ^  */
	0xFF,					 /* _  */
	0xFF,					 /* `  */
	0,						 /* a  */
	1,						 /* b  */
	2,						 /* c  */
	3,						 /* d  */
	4,						 /* e  */
	5,						 /* f  */
	6,						 /* g  */
	7,						 /* h  */
	8,						 /* i  */
	9,						 /* j  */
	10,						 /* k  */
	11,						 /* l  */
	12,						 /* m  */
	13,						 /* n  */
	14,						 /* o  */
	15,						 /* p  */
	16,						 /* q  */
	17,						 /* r  */
	18,						 /* s  */
	19,						 /* t  */
	20,						 /* u  */
	21,						 /* v  */
	22,						 /* w  */
	23,						 /* x  */
	24,						 /* y  */
	25,						 /* z  */
	0xFF,					 /* {  */
	0xFF,					 /* |  */
	0xFF,					 /* }  */
	0xFF,					 /* ~  */
	0xFF,					 /* del */
};

unsigned char *tone_list[] =
{
	"\1\3",					 /* 0 A */
	"\3\1\1\1",
	"\3\1\3\1",
	"\3\1\1",
	"\1",
	"\1\1\3\1",
	"\3\3\1",
	"\1\1\1\1",
	"\1\1",
	"\1\3\3\3",
	"\3\1\3",
	"\1\3\1\1",
	"\3\3",
	"\3\1",
	"\3\3\3",
	"\1\3\3\1",
	"\3\3\1\3",
	"\1\3\1",
	"\1\1\1",
	"\3",
	"\1\1\3",
	"\1\1\1\3",
	"\1\3\3",
	"\3\1\1\3",
	"\3\1\3\3",
	"\3\3\1\1",				 /* 25 Z */

	"\3\3\3\3\3",			 /* 26 0 */
	"\1\3\3\3\3",
	"\1\1\3\3\3",
	"\1\1\1\3\3",
	"\1\1\1\1\3",
	"\1\1\1\1\1",
	"\3\1\1\1\1",
	"\3\3\1\1\1",
	"\3\3\3\1\1",
	"\3\3\3\3\1",			 /* 35 9 */

	"",						 /* 36 space */

	"\1\3\1\3",				 /* 37 AA */
	"\3\1\1\1\3",			 /* 38 BT */
	"\1\1\1\3\1\3",			 /* 39 SK */

	"\1\3\1\3\1\3",			 /* 40 . AAA */
	"\1\1\3\3\1\1",			 /* 41 ? */
	"\3\1\1\3\1",			 /* 42 / */
	"\3\3\1\1\3\3",			 /* 43 , */

};

/*+-------------------------------------------------------------------------
	morse_load_timer2()
--------------------------------------------------------------------------*/
void
morse_load_timer2()
{
	register unsigned int div = morse_freq_div;

	outb(SP_TIMER + 3, 0xB6);/* timer 2, lsb, msb, binary mode */
	outb(SP_TIMER + 2, div & 0xFF);
	outb(SP_TIMER + 2, (div >> 8) & 0xFF);
}							 /* end of morse_load_timer2 */

/*+-------------------------------------------------------------------------
	morse_tone(ticks)
--------------------------------------------------------------------------*/
void
morse_tone(ticks)
int ticks;
{
	morse_load_timer2();	 /* reload each time in case XENIX driver
							  * intrudes */
	outb(SP_PORTB, inb(SP_PORTB) | 3);
	delay(ticks);
	outb(SP_PORTB, inb(SP_PORTB) & ~3);
}							 /* end of morse_tone */

/*+-------------------------------------------------------------------------
	morse_gen(mchar)
--------------------------------------------------------------------------*/
void
morse_gen(mchar)
unsigned int mchar;
{
	register unsigned char *tone = tone_list[mchar];

	if (!*tone)				 /* handle 'space' to get three ticks */
		delay(morse_ticks);
	else
	{
		while (*tone)
		{
			morse_tone(*tone++ * morse_ticks);
			delay(morse_ticks);
		}
	}
	delay(morse_ticks << 1);

}							 /* end of morse_gen */

/*+-------------------------------------------------------------------------
	morswrite(dev)
--------------------------------------------------------------------------*/
void
morswrite(dev)
int dev;
{
	register unsigned int itmp;
	register unsigned int qual;
	register unsigned char *tone;
	unsigned char ch;

	if (morse_silent)
	{
		u.u_count = 0;
		return;
	}

	for (itmp = u.u_count; itmp--; u.u_base++)
	{
		copyin(u.u_base, &ch, 1);
		if ((qual = morse_qualify[ch & 0x7F]) == 0xFF)
			continue;
		morse_gen(qual);
	}
	u.u_count = 0;
}							 /* end of morswrite */

/*+-------------------------------------------------------------------------
	morsioctl(dev,cmd,arg,mode)
--------------------------------------------------------------------------*/
void
morsioctl(dev, cmd, arg, mode)
int dev;
int cmd;
faddr_t arg;
int mode;
{
	IOCTL_ARG morse_arg;

	copyin(arg, (char *)&morse_arg, sizeof(morse_arg.iarg));
	switch (cmd)
	{
		case MORSE_SET_SPEED:
			if ((morse_arg.iarg > 0) && (morse_arg.iarg <= HZ / 5))
				morse_ticks = morse_arg.iarg;
			else
				u.u_error = EINVAL;
			break;
		case MORSE_SET_FREQUENCY:
			if (morse_arg.iarg >= 20)	/* freqs lower won't fit divisor */
				morse_freq_div = XTAL_FREQ / (long)morse_arg.iarg;
			else
				u.u_error = EINVAL;
			break;
		case MORSE_ENABLE:
			morse_silent = 0;
			break;
		case MORSE_DISABLE:
			morse_silent = 1;
			break;
		default:
			u.u_error = EINVAL;
	}
}							 /* end of morsioctl */

/*+-------------------------------------------------------------------------
	morsopen(dev,flag)
--------------------------------------------------------------------------*/
void
morsopen(dev, flag)
int dev;
int flag;
{
	if (morse_in_use)
	{
		u.u_error = EACCES;
		return;
	}

	morse_in_use = 1;
	morse_freq_div = XTAL_FREQ / 500L;	/* 500 Hz default */
	morse_ticks = HZ / 20;

}							 /* end of morsopen */

/*+-------------------------------------------------------------------------
	morsclose(dev)
--------------------------------------------------------------------------*/
void
morsclose(dev)
int dev;
{
	if (morse_in_use)
		morse_in_use = 0;
	else
		u.u_error = EBADF;
}							 /* end of morsclose */

/*+-------------------------------------------------------------------------
	morsread(dev)
--------------------------------------------------------------------------*/
void
morsread(dev)
int dev;
{
	u.u_error = EACCES;
}							 /* end of morsread */
