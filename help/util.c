/*+-------------------------------------------------------------------------
	util.c
	wht@wht.net
--------------------------------------------------------------------------*/
/*+:EDITS:*/
/*:01-24-1997-02:37-wht@yuriatin-SOURCE RELEASE 4.00 */
/*:09-11-1996-20:00-wht@yuriatin-3.48-major telnet,curses,structural overhaul */
/*:11-14-1995-10:23-wht@kepler-3.37.80-source control point: SOCKETS */
/*:05-04-1994-04:39-wht@n4hgf-ECU release 3.30 */
/*:09-10-1992-13:59-wht@n4hgf-ECU release 3.20 */
/*:08-22-1992-15:39-wht@n4hgf-ECU release 3.20 BETA */
/*:07-25-1991-12:58-wht@n4hgf-ECU release 3.10 */
/*:08-14-1990-20:40-wht@n4hgf-ecu3.00-flush old edit history */

/*+-------------------------------------------------------------------------
	all touuper/tolower not created equally, so this works!
--------------------------------------------------------------------------*/
char
to_upper(ch)
register int ch;
{
	return (((ch >= 'a') && (ch <= 'z')) ? ch - 0x20 : ch);
}							 /* end of to_upper() */

char
to_lower(ch)
register int ch;
{
	return (((ch >= 'A') && (ch <= 'Z')) ? ch + 0x20 : ch);
}							 /* end of to_lower() */

/*+-----------------------------------------------------------------------
	pad_zstr_to_len(zstr,len)

  pads with spaces to specified length, unless already longer than
  len in which case the string is truncated to 'len' characters.
------------------------------------------------------------------------*/
void
pad_zstr_to_len(zstr, len)
char *zstr;
int len;
{
	register int izstr;

	izstr = strlen(zstr);
	if (izstr >= len)
		zstr[len] = 0;
	else
	{
		while (izstr < len)
			zstr[izstr++] = 0x20;
		zstr[izstr] = 0;
	}
}							 /* end of pad_zstr_to_len */
