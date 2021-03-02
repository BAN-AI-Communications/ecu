/*+-----------------------------------------------------------------------
	ecuuclc.c - uuper/lower-case string functions
	wht@wht.net

  Defined functions:
	minunique(str1, str2, minquan)
	ulcmpb(str1, str2)
	ulindex(str1, str2)
	ulrindex(str1, str2)

------------------------------------------------------------------------*/
/*+:EDITS:*/
/*:04-26-2000-11:15-wht@bob-RELEASE 4.42 */
/*:01-24-1997-02:37-wht@yuriatin-SOURCE RELEASE 4.00 */
/*:09-11-1996-20:00-wht@yuriatin-3.48-major telnet,curses,structural overhaul */
/*:11-23-1995-11:20-wht@kepler-source control 3.37 for tsx-11 */
/*:11-14-1995-10:23-wht@kepler-3.37.80-source control point: SOCKETS */
/*:05-04-1994-04:39-wht@n4hgf-ECU release 3.30 */
/*:09-10-1992-13:59-wht@n4hgf-ECU release 3.20 */
/*:08-22-1992-15:38-wht@n4hgf-ECU release 3.20 BETA */
/*:07-25-1991-12:57-wht@n4hgf-ECU release 3.10 */
/*:08-14-1990-20:40-wht@n4hgf-ecu3.00-flush old edit history */

char to_upper();
char to_lower();

/*+----------------------------------------------------------------------------
    ulcmpb(str1,str) -- Upper/Lower [case insensitive] Compare Bytes

 Returns -1 if strings are equal, else failing character position
 If the second strings terminates with a null and both strings have matched
 character for character until that point, then -1 is returned.
 NOTE:  this is not a test for complete equality of two strings, but allows
 discovery of a string as a substring in a larger containing string.
-----------------------------------------------------------------------------*/
int
ulcmpb(str1, str2)
unsigned char *str1;
unsigned char *str2;
{
	int istr;

	for (istr = 0;; ++istr)
	{
		if (str2[istr] == '\0')	/* if second string exhausts, match! */
			return (-1);
		if ((str1[istr] == '\0') ||
			(to_upper(str1[istr]) != to_upper(str2[istr])))
			return (istr);
	}
	/* NOTREACHED */
}							 /* end of ulcmpb */

/*+-------------------------------------------------------------------------
    ulindex:  Upper/Lower [case insensitive] Index function

  Returns position of 'str2' in 'str1' if found
  If 'str2' is null, then 0 is returned (null matches anything)
  Returns -1 if not found

  uses 'ulcmpb'
--------------------------------------------------------------------------*/
int
ulindex(str1, str2)
char *str1;					 /* the (target) string to search */
char *str2;					 /* the (comparand) string to search for */
{
	int istr1 = 0;			 /* moving index into str1 */
	char *mstr = str1;		 /* moving string pointer */

	if (str2[0] == '\0')	 /* null string matches anything */
		return (0);
	if (strlen(str2) > strlen(str1))
		return (-1);
	while (1)
	{
		if (*mstr == '\0')	 /* if we exhaust target string, flunk */
			return (-1);
		/* Can we find either case of first comparand char in target? */
		if (to_upper(*mstr) == to_upper(str2[0]))
		{
			/* we have a first char match... does rest of string match? */
			if (ulcmpb(mstr, str2) == -1)	/* if the rest matches, ... */
				break;
		}

		/*
		 * we did not match this time... increment istr1, mstr and try
		 * again
		 */
		++istr1;
		++mstr;
	}
	return (istr1);			 /* return match position */
}							 /* end of ulindex */

/*+-------------------------------------------------------------------------
    ulrindex:  Upper/Lower [case insensitive] Right Index function

  Returns position of 'str2' in 'str1' if found
  Returns -1 if not found
  If 'str2' is null, then -1 is returned

  uses 'ulcmpb'
--------------------------------------------------------------------------*/
int
ulrindex(str1, str2)
char *str1;					 /* the (target) string to search */
char *str2;					 /* the (comparand) string to search for */
{
	char *mstr;
	int istr1;

	if (!str2[0])			 /* null string matches anything */
		return (-1);
	if (strlen(str2) > strlen(str1))
		return (-1);

	mstr = str1 + strlen(str1) - strlen(str2);	/* moving string pointer */
	istr1 = mstr - str1;	 /* moving index into str1 */

	while (mstr >= str1)
	{
		/* Can we find either case of first comparand char in target? */
		if (to_upper(*mstr) == to_upper(str2[0]))
		{
			/* we have a first char match... does rest of string match? */
			if (ulcmpb(mstr, str2) == -1)	/* if the rest matches, ... */
				return (istr1);	/* ... return match position */
		}

		/*
		 * we did not match this time... increment istr1, mstr and try
		 * again
		 */
		--istr1;
		--mstr;
	}
	return (-1);
}							 /* end of ulrindex */

/*+----------------------------------------------------------------
    minunique(str1,str2,minquan)

  Returns 1 if at least 'minquan' chars of str2 match
  str1 and there are no chars after the minimum unique
  chars which do not match str1.  Returns 0 on failure.
-----------------------------------------------------------------*/
int
minunique(str1, str2, minquan)
char *str1;
char *str2;
int minquan;
{
	int index;

	if (strlen(str2) < minquan)
		return (0);

	index = ulcmpb(str1, str2);
	if (index < 0)
		return (1);

	if (index < minquan)
		return (0);
	if (index < strlen(str2))
		return (0);

	return (1);

}							 /* end of minunique */
/* vi: set tabstop=4 shiftwidth=4: */
