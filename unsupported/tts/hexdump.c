/*+-----------------------------------------------------------------------
	hexdump.c  -- very generic hex/graphics dump development aid
	wht@n4hgf.Mt-Park.GA.US

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
/*:01-24-1997-02:38-wht@yuriatin-SOURCE RELEASE 4.00 */
/*:08-21-1993-14:41-wht@gyro-1.22 - fix RNS losing server port numbers */
/*:11-06-1991-15:26-wht@n4hgf-alpha test version finally */
/*:07-25-1991-12:55-wht@n4hgf-ECU release 3.10 */
/*:08-14-1990-20:40-wht@n4hgf-ecu3.00-flush old edit history */

#include <stdio.h>
#include <string.h>

FILE *dumpfp;

/*+-------------------------------------------------------------------------
	dump_putc(ch)
--------------------------------------------------------------------------*/
void
dump_putc(ch)
char ch;
{
	fputc(ch, dumpfp);
}							 /* end of dump_putc */

/*+-------------------------------------------------------------------------
	dump_puts(str)
--------------------------------------------------------------------------*/
void
dump_puts(str)
char *str;
{
	fputs(str, dumpfp);
}							 /* end of dump_puts */

/*+-----------------------------------------------------------------------
	hex_dump#... subservient routines
------------------------------------------------------------------------*/
void
hex_dump4(int4)
unsigned char int4;
{
	int4 &= 15;
	dump_putc((int4 >= 10) ? (int4 + 'A' - 10) : (int4 + '0'));
}

void
hex_dump8(int8)
unsigned char int8;
{
	hex_dump4(int8 >> 4);
	hex_dump4(int8);
}

void
hex_dump16(int16)
unsigned short int16;
{
	hex_dump8(int16 >> 8);
	hex_dump8(int16);
}

void
hex_dump32(int32)
unsigned long int32;
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
	register ipos = 0;
	register itmp;
	int istr;
	int print_offset = (len > 0);

	if (!print_offset)
		len = -len;

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
		dump_puts("\n");

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
				if (!terse_flag)
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
	hex_dump_fp(stdout, str, len, title, terse_flag);
}							 /* end of hex_dump_fp */

/* end of hexdump.c */
/* vi: set tabstop=4 shiftwidth=4: */
