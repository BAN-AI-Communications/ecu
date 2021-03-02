
/*+-------------------------------------------------------------------------
	bcdcvt.c - BCD to binary and vice versa
	...!gatech!kd4nc!n4hgf!wht
--------------------------------------------------------------------------*/
/*+:EDITS:*/
/*:01-24-1997-02:38-wht@yuriatin-SOURCE RELEASE 4.00 */
/*:01-28-1990-04:55-wht-working on cmostime version 4 */

/*+-------------------------------------------------------------------------
	bcdch_to_uchar(bcdch) -- convert two-nibble bcd to binary
--------------------------------------------------------------------------*/
unsigned char
bcdch_to_uchar(bcdch)
unsigned char bcdch;
{
	return((((bcdch & 0xF0) >> 4) * 10) + (bcdch & 0x0F));
}	/* end of bcdch_to_uchar */

/*+-------------------------------------------------------------------------
	uchar_to_bcdch(uch) -- convert binary to two-nibble bcd
--------------------------------------------------------------------------*/
unsigned char
uchar_to_bcdch(uch)
unsigned char uch;
{
	return(((uch / 10) << 4) | (uch % 10));
}	/* end of uchar_to_bcdch */

