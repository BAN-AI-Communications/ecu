/*+-----------------------------------------------------------------------
	ecudump.c  -- very generic hex/graphics dump development aid
	wht@wht.net

  Defined functions:
	dump_putc(ch)
	dump_puts(str)
	hex_dump(str, len, title, terse_flag)
	hex_dump16(int16)
	hex_dump32(int32)
	hex_dump4(int4)
	hex_dump8(int8)
	hex_dump_fp(fp, str, len, title, terse_flag)

------------------------------------------------------------------------*/
/*+:EDITS:*/
/*:04-26-2000-11:15-wht@bob-RELEASE 4.42 */
/*:01-24-1997-02:37-wht@yuriatin-SOURCE RELEASE 4.00 */
/*:09-11-1996-20:00-wht@yuriatin-3.48-major telnet,curses,structural overhaul */
/*:11-28-1995-20:09-wht@kepler-bug in last line space fill */
/*:11-23-1995-11:20-wht@kepler-source control 3.37 for tsx-11 */
/*:11-14-1995-10:23-wht@kepler-3.37.80-source control point: SOCKETS */
/*:11-10-1995-14:56-root@kepler-extend ASCII graphics for len>16 eve if terse */
/*:05-04-1994-04:38-wht@n4hgf-ECU release 3.30 */
/*:08-07-1993-16:09-wht@n4hgf-fix old bug in hex_dump_fp */
/*:09-10-1992-13:58-wht@n4hgf-ECU release 3.20 */
/*:08-22-1992-15:38-wht@n4hgf-ECU release 3.20 BETA */
/*:07-25-1991-12:55-wht@n4hgf-ECU release 3.10 */
/*:08-14-1990-20:40-wht@n4hgf-ecu3.00-flush old edit history */

#include "ecu.h"

FILE *dumpfp;

/*+-------------------------------------------------------------------------
	dump_putc(ch)
--------------------------------------------------------------------------*/
void
dump_putc(ch)
char ch;
{
	if (dumpfp == stderr)
		pputc(ch);
	else
		fputc(ch, dumpfp);
}							 /* end of dump_putc */

/*+-------------------------------------------------------------------------
	dump_puts(str)
--------------------------------------------------------------------------*/
void
dump_puts(str)
char *str;
{
	if (dumpfp == stderr)
		pputs(str);
	else
		fputs(str, dumpfp);
}							 /* end of dump_puts */

/*+-----------------------------------------------------------------------
	hex_dump#... subservient routines
------------------------------------------------------------------------*/
void
hex_dump4(int4)
uchar int4;
{
	int4 &= 15;
	dump_putc((int4 >= 10) ? (int4 + 'A' - 10) : (int4 + '0'));
}

void
hex_dump8(int8)
uchar int8;
{
	hex_dump4(int8 >> 4);
	hex_dump4(int8);
}

void
hex_dump16(int16)
UINT16 int16;
{
	hex_dump8(int16 >> 8);
	hex_dump8(int16);
}

void
hex_dump32(int32)
UINT32 int32;
{
	hex_dump16(int32 >> 16);
	hex_dump16(int32);
}

/*+-----------------------------------------------------------------
	hex_dump_fp(fp,str,len,title,terse_flag)

  if 'title' not NULL, title is printed... 'terse_flag'
  controls whether or not the title is "conspicuous" with
  hyphens before and after it making title line >70 chars long
  If len negative, print no buffer offsets.
------------------------------------------------------------------*/
void
hex_dump_fp(fp, str, len, title, terse_flag)
FILE *fp;
char *str;
int len;
char *title;
int terse_flag;
{
	int ipos = 0;
	int itmp;
	int istr;
	int print_offset = (len > 32);

	dumpfp = fp;

	if (title && (istr = strlen(title)))
	{
		if (!terse_flag)
		{
			ipos = (((print_offset) ? 73 : 67) - istr) / 2;
			itmp = ipos;
			while (itmp--)
				dump_putc('-');
			dump_putc(' ');
			if (istr & 1)
				ipos--;
		}
		dump_puts(title);
		if (!terse_flag)
		{
			dump_putc(' ');
			while (ipos--)
				dump_putc('-');
		}
		if (terse_flag && (len < 12))
			dump_putc(' ');
		else
		{
			if (dumpfp == stderr)
				dump_puts("\r\n");
			else
				dump_puts("\n");
		}
	}

	istr = 0;
	while (istr < len)
	{
		if (print_offset)
		{
			hex_dump16(istr);
			dump_puts("  ");
		}
		for (itmp = 0; itmp < 16; ++itmp)
		{
			ipos = istr + itmp;
			if (ipos >= len)
			{
				if (!terse_flag || (len > 16))
					dump_puts("   ");
				continue;
			}
			if (itmp)
				dump_putc(' ');
			hex_dump8(str[ipos]);
		}
		dump_puts("  | ");
		for (itmp = 0; itmp < 16; ++itmp)
		{
			ipos = istr + itmp;
			if ((ipos) >= len)
			{
				if (!terse_flag)
					dump_putc(' ');
			}
			else
			{
				dump_putc((str[ipos] >= ' ' && str[ipos] < 0x7f)
					? str[ipos] : '.');
			}
		}
		if (dumpfp == stderr)
			dump_puts(" |\r\n");
		else
			dump_puts(" |\n");
		istr += 16;
	}						 /* end of while(istr < len) */

}							 /* end of hex_dump_fp */

/*+-------------------------------------------------------------------------
	hex_dump(str,len,title,terse_flag)
--------------------------------------------------------------------------*/
void
hex_dump(str, len, title, terse_flag)
char *str;
int len;
char *title;
int terse_flag;
{
	hex_dump_fp(stderr, str, len, title, terse_flag);
}							 /* end of hex_dump_fp */
/* end of ecudump.c */
/* vi: set tabstop=4 shiftwidth=4: */
