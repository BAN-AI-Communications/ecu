/*+-------------------------------------------------------------------------
	zmodem.c - ZMODEM protocol primitives
    based on code by Chuck Forsberg

  Defined functions:
	noxrd7()
	rclhdr(hdr)
	stohdr(pos)
	zdlread()
	zgeth1()
	zgethdr(hdr)
	zgethex()
	zputc_init()
	zputc_serial(c)
	zputc_telnet(c)
	zputhex(c)
	zrbhdr(hdr)
	zrbhdr32(hdr)
	zrdat32(buf, length)
	zrdata(buf, length)
	zrhhdr(hdr)
	zsbh32(type, hdr)
	zsbhdr(type, hdr)
	zsda32(buf, length, frameend)
	zsdata(buf, length, frameend)
	zshhdr(type, hdr)

--------------------------------------------------------------------------*/
/*+:EDITS:*/
/*:01-24-1997-02:38-wht@yuriatin-SOURCE RELEASE 4.00 */
/*:09-11-1996-20:01-wht@yuriatin-3.48-major telnet,curses,structural overhaul */
/*:09-06-1996-03:18-wht@kepler-cleanup */
/*:11-27-1995-20:41-wht@kepler-two-mode zputc from old zsendline */
/*:11-14-1995-10:23-wht@kepler-3.37.80-source control point: SOCKETS */
/*:05-04-1994-04:40-wht@n4hgf-ECU release 3.30 */
/*:01-25-1994-17:02-wht@n4hgf-robertl corrections + mv extern crctab to hdr */
/*:09-10-1992-14:00-wht@n4hgf-ECU release 3.20 */
/*:08-22-1992-15:39-wht@n4hgf-ECU release 3.20 BETA */
/*:07-25-1991-12:59-wht@n4hgf-ECU release 3.10 */
/*:08-14-1990-20:41-wht@n4hgf-ecu3.00-flush old edit history */

#include "zmodem.h"

extern char s128[];
extern int Zctlesc;
extern int Zmodem;
extern int telnet;
extern unsigned long Bitrate;

int Rxtimeout = 100;		 /* Tenths of seconds to wait for something */

#if !defined(UNSL)
#define UNSL unsigned
#endif

static lastsent;			 /* Last char we sent */
static evenp;				 /* Even parity seen on header */

/* Globals used by ZMODEM functions */
char Attn[ZATTNLEN + 1];	 /* Attention string rx sends to tx on err */
char Rxhdr[4];				 /* Received header */
char Txhdr[4];				 /* Transmitted header */
int Crc32;					 /* Display flag indicating 32 bit CRC being
							  * received */
int Crc32t;					 /* Display flag indicating 32 bit CRC being
							  * sent */
int Rxcount;				 /* Count of data bytes received */
int Rxframeind;				 /* ZBIN ZBIN32,or ZHEX type of frame received */
int Rxtype;					 /* Type of header received */
int Txfcs32;				 /* TURE means send binary frames with 32 bit
							  * FCS */
int Zrwindow;				 /* RX window size (controls garbage count) */
long Rxpos;					 /* Received file position */
long Txpos;					 /* Transmitted file position */

char *frametypes[] =
{
	"Carrier Lost",			 /* -3 */
	"TIMEOUT",				 /* -2 */
	"ERROR",				 /* -1 */
/* #define FTOFFSET 3 moved to zmodem.h */
	"ZRQINIT",
	"ZRINIT",
	"ZSINIT",
	"ZACK ",
	"ZFILE",
	"ZSKIP",
	"ZNAK ",
	"ZABORT",
	"ZFIN ",
	"ZRPOS",
	"ZDATA",
	"ZEOF ",
	"ZFERR",
	"ZCRC ",
	"ZCHALLENGE",
	"ZCOMPL",
	"ZCAN ",
	"ZFREECNT",
	"ZCOMMAND",
	"ZSTDERR",
	"xxxxx"
#define FRTYPES 22			 /* Total number of frame types in this array */
 /* not including psuedo negative entries */
};

static char masked[] = "8 bit transparent path required";
static char badcrc[] = "Bad CRC";

typedef void (*PFV) ();		 /* pointer to function returning nothing */
static PFV zputc;

/*+-------------------------------------------------------------------------
	zputc_telnet(c)
--------------------------------------------------------------------------*/
void
zputc_telnet(c)
unsigned char c;
{

	switch (c)
	{
		case 0377:
			xputc(ZDLE);
			c = ZRUB1;
			break;
		default:
			if (Zctlesc && !(c & 0140))
			{
				xputc(ZDLE);
				c ^= 0100;
			}
	}
	xputc(c);

}							 /* end of zputc_telnet */

/*+-------------------------------------------------------------------------
	zputc_serial(c)
--------------------------------------------------------------------------*/
void
zputc_serial(c)
unsigned char c;
{

	if (c == 0377)
	{
		xputc(ZDLE);
		xputc(lastsent = ZRUB1);
	}
	else if (c & 0140)		 /* Quick check for non control characters */
		xputc(lastsent = c);
	else
	{
		switch (c)
		{
			case ZDLE:
				xputc(ZDLE);
				xputc(lastsent = (c ^= 0100));
				break;
			case 015:
			case 0215:
				if (!Zctlesc && (lastsent & 0177) != '@')
					goto sendit;
				/* **** FALL THRU TO **** */
			case 020:
			case 021:
			case 023:
			case 0220:
			case 0221:
			case 0223:
				xputc(ZDLE);
				c ^= 0100;
			  sendit:
				xputc(lastsent = c);
				break;
			default:
				if (Zctlesc && !(c & 0140))
				{
					xputc(ZDLE);
					c ^= 0100;
				}
				xputc(lastsent = c);
		}
	}
}							 /* end of zputc_serial */

/*+-------------------------------------------------------------------------
	zputc_init()
--------------------------------------------------------------------------*/
void
zputc_init()
{
	extern int telnet;

	if (telnet)
		zputc = zputc_telnet;
	else
		zputc = zputc_serial;
}							 /* end of zputc_init */

/*+-------------------------------------------------------------------------
	zsbh32(type, hdr) - send ZMODEM CRC-32 binary header
--------------------------------------------------------------------------*/
void
zsbh32(type, hdr)
int type;
register unsigned char *hdr;
{
	register int n;
	register UNSL long crc;

	sprintf(s128, "B32 %s %ld",
		frametypes[type + FTOFFSET], rclhdr(hdr));
	report_last_txhdr(s128, 0);
	report_tx_ind(1);
	xputc(ZBIN32);
	(*zputc) ((unsigned char)type);
	crc = (UNSL long)-1;
	crc = UPDC32(type, crc);

	for (n = 4; --n >= 0; ++hdr)
	{
		crc = UPDC32((0377 & *hdr), crc);
		(*zputc) (*hdr);
	}
	crc = ~crc;
	for (n = 4; --n >= 0;)
	{
		(*zputc) ((unsigned char)crc);
		crc >>= 8;
	}
	report_tx_ind(0);
	flushline();
}							 /* end of zsbh32 */

/* Send ZMODEM binary header hdr of type type */
void
zsbhdr(type, hdr)
int type;
register unsigned char *hdr;
{
	register int n;
	register unsigned crc;

	xputc(ZPAD);
	xputc(ZDLE);

	if (Crc32t = Txfcs32)
		zsbh32(type, hdr);
	else
	{
		sprintf(s128, "B16 %s %ld",
			frametypes[type + FTOFFSET], rclhdr(hdr));
		report_last_txhdr(s128, 0);
		report_tx_ind(1);

		xputc(ZBIN);
		(*zputc) ((unsigned char)type);
		crc = updcrc(type, 0);

		for (n = 4; --n >= 0; ++hdr)
		{
			(*zputc) (*hdr);
			crc = updcrc(*hdr, crc);
		}
		crc = updcrc(0, updcrc(0, crc));
		(*zputc) ((unsigned char)(crc >> 8));
		(*zputc) ((unsigned char)crc);
		report_tx_ind(0);
	}
	if (type != ZDATA)
		flushline();
}

/*
 * Send binary array buf of length length,with ending ZDLE sequence frameend
 */
static char *Zendnames[] =
{"ZCRCE", "ZCRCG", "ZCRCQ", "ZCRCW"};

/*+-------------------------------------------------------------------------
	zsda32(buf, length, frameend)
--------------------------------------------------------------------------*/
void
zsda32(buf, length, frameend)
register char *buf;
int length;
int frameend;
{
	register unsigned int c;
	register UNSL long crc;

	sprintf(s128, "D32 %s %d",
		Zendnames[(frameend - ZCRCE) & 3], length);
	report_last_txhdr(s128, 0);
	report_tx_ind(1);

	crc = (UNSL long)-1;
	for (; --length >= 0; ++buf)
	{
		c = *buf & 0377;
		(*zputc) ((unsigned char)c);
		crc = UPDC32(c, crc);
	}
	xputc(ZDLE);
	xputc(frameend);
	crc = UPDC32(frameend, crc);

	crc = ~crc;
	for (length = 4; --length >= 0;)
	{
		(*zputc) ((unsigned char)crc);
		crc >>= 8;
	}
	report_tx_ind(0);
}							 /* end of zsda32 */

/*+-------------------------------------------------------------------------
	zsdata(buf, length, frameend)
--------------------------------------------------------------------------*/
void
zsdata(buf, length, frameend)
register unsigned char *buf;
int length;
int frameend;
{
	register unsigned short crc;

	if (Crc32t)
		zsda32(buf, length, frameend);
	else
	{
		sprintf(s128, "D16 %s %d",
			Zendnames[(frameend - ZCRCE) & 3], length);
		report_last_txhdr(s128, 0);
		report_tx_ind(1);
		crc = 0;
		for (; --length >= 0; ++buf)
		{
			(*zputc) (*buf);
			crc = updcrc(*buf, crc);
		}
		xputc(ZDLE);
		xputc(frameend);
		crc = updcrc(frameend, crc);

		crc = updcrc(0, updcrc(0, crc));
		(*zputc) ((unsigned char)(crc >> 8));
		(*zputc) ((unsigned char)crc);
		report_tx_ind(0);
	}
	if (frameend == ZCRCW)
	{
		xputc(XON);
		flushline();
	}

}							 /* end of zsdata */

/*+-------------------------------------------------------------------------
	zputhex(c) - send a byte as two hex digits
--------------------------------------------------------------------------*/
void
zputhex(c)
register int c;
{
	static char digits[] = "0123456789abcdef";

	sendline(digits[(c & 0xF0) >> 4]);
	sendline(digits[(c) & 0xF]);
}							 /* end of zputhex */

/*+-------------------------------------------------------------------------
	zshhdr(type, hdr)
--------------------------------------------------------------------------*/
void
zshhdr(type, hdr)
int type;
register unsigned char *hdr;
{
	register int n;
	register unsigned short crc;

	sprintf(s128, "HEX %s %ld",
		frametypes[type + FTOFFSET], rclhdr(hdr));
	report_last_txhdr(s128, 0);
	report_tx_ind(1);
	sendline(ZPAD);
	sendline(ZPAD);
	sendline(ZDLE);
	sendline(ZHEX);
	zputhex(type);
	Crc32t = 0;

	crc = updcrc(type, 0);
	for (n = 4; --n >= 0; ++hdr)
	{
		zputhex(*hdr);
		crc = updcrc(*hdr, crc);
/*		crc = updcrc((0377 & *hdr),crc);  original - wht */
	}
	crc = updcrc(0, updcrc(0, crc));
	zputhex(crc >> 8);
	zputhex(crc);

	/* Make it printable on remote machine */
	sendline(015);
	sendline(012);

	/*
	 * Uncork the remote in case a fake XOFF has stopped data flow
	 */
	if (type != ZFIN && type != ZACK)
		sendline(021);
	flushline();
	report_tx_ind(0);
}							 /* end of zshhdr */

/*+-------------------------------------------------------------------------
	zrdat32(buf, length)

  Receive array buf of max length with ending ZDLE sequence and
  CRC.  Returns the ending character or error code.

  NB: On errors may store length+1 bytes!
--------------------------------------------------------------------------*/
int
zrdat32(buf, length)
register char *buf;
int length;
{
	register int c;
	register UNSL long crc;
	register char *end;
	register int d;

	report_rx_ind(1);
	crc = (UNSL long)-1;
	Rxcount = 0;
	end = buf + length;
	while (buf <= end)
	{
		if ((c = zdlread()) & ~0377)
		{
		  crcfoo:
			switch (c)
			{
				case GOTCRCE:
				case GOTCRCG:
				case GOTCRCQ:
				case GOTCRCW:
					d = c;
					c &= 0377;
					crc = UPDC32(c, crc);
					if ((c = zdlread()) & ~0377)
						goto crcfoo;
					crc = UPDC32(c, crc);
					if ((c = zdlread()) & ~0377)
						goto crcfoo;
					crc = UPDC32(c, crc);
					if ((c = zdlread()) & ~0377)
						goto crcfoo;
					crc = UPDC32(c, crc);
					if ((c = zdlread()) & ~0377)
						goto crcfoo;
					crc = UPDC32(c, crc);
					if (crc != (UNSL long)0xDEBB20E3L)
					{
						report_str(badcrc, 0);
						report_rx_ind(0);
						return (ERROR);
					}
					Rxcount = length - (end - buf);
					report_rxblklen(Rxcount);
					sprintf(s128, "D32 %s %d",
						Zendnames[(d - GOTCRCE) & 3], Rxcount);
					report_last_rxhdr(s128, 0);
					report_rx_ind(0);
					return (d);
				case GOTCAN:
					report_str("Sender Canceled", 1);
					report_rx_ind(0);
					return (ZCAN);
				case TIMEOUT:
					report_str("TIMEOUT", 0);
					report_rx_ind(0);
					return (c);
				default:
					report_str("Bad data subpacket", 0);
					report_rx_ind(0);
					return (c);
			}
		}
		*buf++ = c;
		crc = UPDC32(c, crc);
	}
	report_str("Data subpacket too long", 0);
	report_rx_ind(0);
	return (ERROR);
}							 /* end of zrdat32 */

/*+-------------------------------------------------------------------------
	zrdata(buf, length)
--------------------------------------------------------------------------*/
int
zrdata(buf, length)
register char *buf;
int length;
{
	register int c;
	register unsigned short crc;
	register char *end;
	register int d;

	if (Rxframeind == ZBIN32)
		return (zrdat32(buf, length));

	report_rx_ind(1);
	crc = Rxcount = 0;
	end = buf + length;
	while (buf <= end)
	{
		if ((c = zdlread()) & ~0377)
		{
		  crcfoo:
			switch (c)
			{
				case GOTCRCE:
				case GOTCRCG:
				case GOTCRCQ:
				case GOTCRCW:
					crc = updcrc(((d = c) & 0377), crc);
					if ((c = zdlread()) & ~0377)
						goto crcfoo;
					crc = updcrc(c, crc);
					if ((c = zdlread()) & ~0377)
						goto crcfoo;
					crc = updcrc(c, crc);
					if (crc & 0xFFFF)
					{
						report_str(badcrc, 0);
						report_rx_ind(0);
						return (ERROR);
					}
					Rxcount = length - (end - buf);
					report_rxblklen(Rxcount);
					sprintf(s128, "D16 %s %d",
						Zendnames[(d - GOTCRCE) & 3], Rxcount);
					report_last_rxhdr(s128, 0);
					report_rx_ind(0);
					return (d);
				case GOTCAN:
					report_str("Sender Cancelled", 1);
					report_rx_ind(0);
					return (ZCAN);
				case TIMEOUT:
					report_str("TIMEOUT", 0);
					report_rx_ind(0);
					return (c);
				default:
					report_str("Bad data subpacket", 0);
					report_rx_ind(0);
					return (c);
			}
		}
		*buf++ = c;
		crc = updcrc(c, crc);
	}
	report_str("Data subpacket too long", 0);
	report_rx_ind(0);
	return (ERROR);

}							 /* end of zrdata */

/*+-------------------------------------------------------------------------
	zgethdr(hdr) - read a ZMODEM header

  On success,set Zmodem to 1,set Rxpos and return type of header.
   Otherwise return negative on error.
   Return ERROR instantly if ZCRCW sequence,for fast error recovery.
--------------------------------------------------------------------------*/
int
zgethdr(hdr)
char *hdr;
{
	register int c, n, cancount;

	char *hdrtyp = "?";

	report_rx_ind(1);
	n = Zrwindow + Bitrate;	 /* Max bytes before start of frame */
	Rxframeind = Rxtype = 0;

  startover:
	cancount = 5;
  again:
	/* Return immediate ERROR if ZCRCW sequence seen */
	switch (c = readline(Rxtimeout))
	{
		case RCDO:
		case TIMEOUT:
			goto fifi;
		case CAN:
		  gotcan:
			if (--cancount <= 0)
			{
				c = ZCAN;
				goto fifi;
			}
			switch (c = readline(1))
			{
				case TIMEOUT:
					goto again;
				case ZCRCW:
					c = ERROR;
					/* **** FALL THRU TO **** */
				case RCDO:
					goto fifi;
				default:
					break;
				case CAN:
					if (--cancount <= 0)
					{
						c = ZCAN;
						goto fifi;
					}
					goto again;
			}
			/* **** FALL THRU TO **** */
		default:
		  agn2:
			if (--n == 0)
			{
				report_str("Garbage count exceeded", 1);
				report_last_rxhdr("Noise", 0);
				report_rx_ind(0);
				return (ERROR);
			}
			goto startover;
		case ZPAD | 0200:	 /* This is what we want. */
		case ZPAD:			 /* This is what we want. */
			evenp = c & 0200;
			break;
	}
	cancount = 5;
  splat:
	switch (c = noxrd7())
	{
		case ZPAD:
			goto splat;
		case RCDO:
		case TIMEOUT:
			goto fifi;
		default:
			goto agn2;
		case ZDLE:			 /* This is what we want. */
			break;
	}

	switch (c = noxrd7())
	{
		case RCDO:
		case TIMEOUT:
			goto fifi;
		case ZBIN:
			Rxframeind = ZBIN;
			Crc32 = FALSE;
			c = zrbhdr(hdr);
			break;
		case ZBIN32:
			Crc32 = Rxframeind = ZBIN32;
			c = zrbhdr32(hdr);
			break;
		case ZHEX:
			Rxframeind = ZHEX;
			Crc32 = FALSE;
			c = zrhhdr(hdr);
			break;
		case CAN:
			goto gotcan;
		default:
			goto agn2;
	}
	Rxpos = hdr[ZP3] & 0377;
	Rxpos = (Rxpos << 8) + (hdr[ZP2] & 0377);
	Rxpos = (Rxpos << 8) + (hdr[ZP1] & 0377);
	Rxpos = (Rxpos << 8) + (hdr[ZP0] & 0377);
  fifi:
	switch (c)
	{
		case GOTCAN:
			c = ZCAN;
			/* **** FALL THRU TO **** */
		case ZNAK:
		case ZCAN:
		case ERROR:
		case TIMEOUT:
		case RCDO:
			sprintf(s128, "Got %s", frametypes[c + FTOFFSET]);
			report_str(s128, 0);
			/* **** FALL THRU TO **** */
		default:
			switch (Rxframeind)
			{
				case ZBIN:
					hdrtyp = "B16";
					break;
				case ZBIN32:
					hdrtyp = "B32";
					break;
				case ZHEX:
					hdrtyp = "HEX";
					break;
			}
			if (c >= -3 && c <= FRTYPES)
				sprintf(s128, "%s %s %ld", hdrtyp, frametypes[c + FTOFFSET], Rxpos);
			else
				sprintf(s128, "%s 0x%02x? %ld", hdrtyp, c, Rxpos);
			report_last_rxhdr(s128, 0);
	}
	report_rx_ind(0);
	return (c);

}							 /* end of zgethdr */

/*+-------------------------------------------------------------------------
	zrbhdr(hdr) - receive a binary header
--------------------------------------------------------------------------*/
int
zrbhdr(hdr)
register char *hdr;
{
	register int c, n;
	register unsigned short crc;

	if ((c = zdlread()) & ~0377)
		return (c);
	Rxtype = c;
	crc = updcrc(c, 0);

	for (n = 4; --n >= 0; ++hdr)
	{
		if ((c = zdlread()) & ~0377)
			return (c);
		crc = updcrc(c, crc);
		*hdr = c;
	}
	if ((c = zdlread()) & ~0377)
		return (c);
	crc = updcrc(c, crc);
	if ((c = zdlread()) & ~0377)
		return (c);
	crc = updcrc(c, crc);
	if (crc & 0xFFFF)
	{
		if (evenp)
			report_str(masked, 1);
		report_str(badcrc, 0);
		return (ERROR);
	}
#if defined(ZMODEM)
	Protocol = ZMODEM;
#endif
	Zmodem = 1;
	return (Rxtype);
}							 /* end of zrbhdr */

/*+-------------------------------------------------------------------------
	zrbhdr32(hdr) - receive a binary header with 32 bit FCS
--------------------------------------------------------------------------*/
int
zrbhdr32(hdr)
register char *hdr;
{
	register int c, n;
	register UNSL long crc;

	if ((c = zdlread()) & ~0377)
		return (c);
	Rxtype = c;
	crc = (UNSL long)-1;
	crc = UPDC32(c, crc);

	for (n = 4; --n >= 0; ++hdr)
	{
		if ((c = zdlread()) & ~0377)
			return (c);
		crc = UPDC32(c, crc);
		*hdr = c;
	}
	for (n = 4; --n >= 0;)
	{
		if ((c = zdlread()) & ~0377)
			return (c);
		crc = UPDC32(c, crc);
	}
	if (crc != (UNSL long)0xDEBB20E3)
	{
		if (evenp)
			report_str(masked, 1);
		report_str(badcrc, 0);
		return (ERROR);
	}
#if defined(ZMODEM)
	Protocol = ZMODEM;
#endif
	Zmodem = 1;
	return (Rxtype);

}							 /* end of zrbhdr32 */

/*+-------------------------------------------------------------------------
	zrhhdr(hdr) - receive a hex header
--------------------------------------------------------------------------*/
int
zrhhdr(hdr)
char *hdr;
{
	register int c;
	register unsigned short crc;
	register int n;

	if ((c = zgethex()) < 0)
		return (c);
	Rxtype = c;
	crc = updcrc(c, 0);

	for (n = 4; --n >= 0; ++hdr)
	{
		if ((c = zgethex()) < 0)
			return (c);
		crc = updcrc(c, crc);
		*hdr = c;
	}
	if ((c = zgethex()) < 0)
		return (c);
	crc = updcrc(c, crc);
	if ((c = zgethex()) < 0)
		return (c);
	crc = updcrc(c, crc);
	if (crc & 0xFFFF)
	{
		report_str(badcrc, 0);
		return (ERROR);
	}
	if (readline(1) == '\r') /* Throw away possible cr/lf */
		readline(1);
#if defined(ZMODEM)
	Protocol = ZMODEM;
#endif
	Zmodem = 1;
	return (Rxtype);

}							 /* end of zrhhdr */

/*+-------------------------------------------------------------------------
	zgeth1() - decode two lower case hex digits into an 8 bit byte value
--------------------------------------------------------------------------*/
int
zgeth1()
{
	register int c, n;

	if ((c = noxrd7()) < 0)
		return (c);
	n = c - '0';
	if (n > 9)
		n -= ('a' - ':');
	if (n & ~0xF)
		return (ERROR);
	if ((c = noxrd7()) < 0)
		return (c);
	c -= '0';
	if (c > 9)
		c -= ('a' - ':');
	if (c & ~0xF)
		return (ERROR);
	c += (n << 4);
	return (c);
}							 /* end of zgeth1 */

/*+-------------------------------------------------------------------------
	zgethex()
--------------------------------------------------------------------------*/
int
zgethex()
{
	return (zgeth1());
}							 /* end of zgethex */

/*+-------------------------------------------------------------------------
	zdlread() - read a byte checking for ZMODEM escape encoding

  including CAN*5 which represents a quick abort
--------------------------------------------------------------------------*/
int
zdlread()
{
	register int c;

  again:
	/* Quick check for non control characters */
	if ((c = readline(Rxtimeout)) & 0140)
		return (c);
	switch (c)
	{
		case ZDLE:
			break;
		case 023:
		case 0223:
		case 021:
		case 0221:
			goto again;
		default:
			if (Zctlesc && !(c & 0140))
			{
				goto again;
			}
			return (c);
	}
  again2:
	if ((c = readline(Rxtimeout)) < 0)
		return (c);
	if (c == CAN && (c = readline(Rxtimeout)) < 0)
		return (c);
	if (c == CAN && (c = readline(Rxtimeout)) < 0)
		return (c);
	if (c == CAN && (c = readline(Rxtimeout)) < 0)
		return (c);
	switch (c)
	{
		case CAN:
			return (GOTCAN);
		case ZCRCE:
		case ZCRCG:
		case ZCRCQ:
		case ZCRCW:
			return (c | GOTOR);
		case ZRUB0:
			return (0177);
		case ZRUB1:
			return (0377);
		case 023:
		case 0223:
		case 021:
		case 0221:
			goto again2;
		default:
			if (Zctlesc && !(c & 0140))
			{
				goto again2;
			}
			if ((c & 0140) == 0100)
				return (c ^ 0100);
			break;
	}
	sprintf(s128, "Bad escape sequence %x", c);
	report_str(s128, 0);
	return (ERROR);
}							 /* end of zdlread */

/*+-------------------------------------------------------------------------
	noxrd7() - read a character with timeout

  Eat parity,XON and XOFF characters.
--------------------------------------------------------------------------*/
int
noxrd7()
{
	register int c;

	for (;;)
	{
		if ((c = readline(Rxtimeout)) < 0)
			return (c);
		switch (c &= 0177)
		{
			case XON:
			case XOFF:
				continue;
			default:
				if (Zctlesc && !(c & 0140))
					continue;
			case '\r':
			case '\n':
			case ZDLE:
				return (c);
		}
	}
}							 /* end of noxrd7 */

/*+-------------------------------------------------------------------------
	stohdr(pos) - store long integer pos in Txhdr
--------------------------------------------------------------------------*/
void
stohdr(pos)
long pos;
{
	Txhdr[ZP0] = pos;
	Txhdr[ZP1] = pos >> 8;
	Txhdr[ZP2] = pos >> 16;
	Txhdr[ZP3] = pos >> 24;
}							 /* end of stohdr */

/*+-------------------------------------------------------------------------
	rclhdr(hdr) - recover a long integer from a header
--------------------------------------------------------------------------*/
long
rclhdr(hdr)
register char *hdr;
{
	register long l;

	l = (hdr[ZP3] & 0377);
	l = (l << 8) | (hdr[ZP2] & 0377);
	l = (l << 8) | (hdr[ZP1] & 0377);
	l = (l << 8) | (hdr[ZP0] & 0377);
	return (l);
}							 /* end of rclhdr */

/* end of zmodem.c */
/* vi: set tabstop=4 shiftwidth=4: */
