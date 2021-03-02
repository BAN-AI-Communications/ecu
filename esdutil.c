/*+----------------------------------------------------------------
	esdutil.c - ecu extended string descriptor manipulation
	wht@wht.net

  Defined functions:
	end_of_cmd(tesd)
	esd_null_terminate(tesd)
	esd_strip_trail_break(ztext)
	esdalloc(maxcb)
	esdcat(dest, suffix, realloc_ok)
	esdfgets(tesd, fileptr)
	esdfputs(tesd, fileptr, index_flag, nl_flag)
	esdfree(tesd)
	esdinit(tesd, cp, maxcb)
	esdprefix(tesd, zstr)
	esdrealloc(tesd, maxcb)
	esdshow(tesd, title)
	esdstrcat(tesd, zstr)
	esdstrindex(esd1, esd2, index1_flag, index2_flag)
	esdzero(tesd)
	get_alpha_zstr(tesd, strbuf, strbuf_maxcb)
	get_alphanum_zstr(tesd, strbuf, strbuf_maxcb)
	get_cmd_char(tesd, pchar)
	get_numeric_value(tesd, value)
	get_numeric_zstr(tesd, strbuf, strbuf_maxcb)
	get_switches(tesd, switches, switches_max)
	get_word_zstr(tesd, strbuf, strbuf_maxcb)
	keyword_lookup(ktable, keywd)
	skip_cmd_break(tesd)
	skip_cmd_char(tesd, skipchar)
	skip_colon(tesd)
	skip_comma(tesd)
	skip_paren(tesd, fLeft)
	strindex(str1, str2)

  This is old code; give me a break

-----------------------------------------------------------------*/
/*+:EDITS:*/
/*:04-26-2000-11:15-wht@bob-RELEASE 4.42 */
/*:03-22-2000-18:47-wht@blue-4.41-IMPORTANT esdrealloc fix */
/*:12-28-1999-12:00-wht@menlo-4.40-borrow back get_ipaddr_zstr and esdclone */
/*:01-24-1997-02:37-wht@yuriatin-SOURCE RELEASE 4.00 */
/*:09-11-1996-20:00-wht@yuriatin-3.48-major telnet,curses,structural overhaul */
/*:08-20-1996-12:39-wht@kepler-locale/ctype fixes from ache@nagual.ru */
/*:11-24-1995-10:58-wht@kepler-add esdprefix */
/*:11-23-1995-11:20-wht@kepler-source control 3.37 for tsx-11 */
/*:11-14-1995-10:23-wht@kepler-3.37.80-source control point: SOCKETS */
/*:11-04-1995-13:57-wht@wwtp1-esdfree now ignores zero arg */
/*:09-17-1995-16:28-wht@kepler-remove obsolete #if 0 code */
/*:05-04-1994-04:39-wht@n4hgf-ECU release 3.30 */
/*:12-19-1992-16:30-wht@n4hgf-rectification of names in keyword_lookup */
/*:09-10-1992-13:59-wht@n4hgf-ECU release 3.20 */
/*:08-22-1992-15:38-wht@n4hgf-ECU release 3.20 BETA */
/*:03-20-1992-06:26-wht@n4hgf-esdstrcat will grow an esd */
/*:08-25-1991-23:20-root@n4hgf2-get_switches could overflow result string */
/*:07-25-1991-12:57-wht@n4hgf-ECU release 3.10 */
/*:05-02-1991-04:12-wht@n4hgf-how did esdrealloc ever work? */
/*:04-23-1991-04:33-wht@n4hgf-function name reorganization */
/*:04-23-1991-04:33-wht@n4hgf-add esdcat */
/*:01-31-1991-14:49-wht@n4hgf-rework esdrealloc for speed */
/*:08-14-1990-20:40-wht@n4hgf-ecu3.00-flush old edit history */

#include "ecu.h"
#include "ecuerror.h"
#include "esd.h"

extern int errno;

/*+-------------------------------------------------------------------------
    esd_null_terminate(&esd)
    puts null at 'cb' position of string (standard esd always
    has one more byte in buffer than maxcb says)
--------------------------------------------------------------------------*/
void
esd_null_terminate(tesd)
ESD *tesd;
{
	tesd->pb[tesd->cb] = 0;
}							 /* end of esd_null_terminate */

/*+-----------------------------------------------------------------------
	esdzero(tesd) - zero an esd
------------------------------------------------------------------------*/
void
esdzero(tesd)
ESD *tesd;
{
	tesd->cb = 0;			 /* current count == 0 */
	tesd->index = 0;		 /* parse index to first position */
	tesd->old_index = 0;	 /* parse index to first position */
	*tesd->pb = 0;			 /* start with null terminated string */

}							 /* end of esdzero */

/*+-----------------------------------------------------------------------
	esdinit(tesd,cp,maxcb) - init an esd
------------------------------------------------------------------------*/
void
esdinit(tesd, cp, maxcb)
ESD *tesd;
char *cp;
int maxcb;
{
	tesd->pb = cp;			 /* pointer to string */
	tesd->maxcb = maxcb;	 /* max characters in buffer */
	esdzero(tesd);

}							 /* end of esdinit */

/*+-----------------------------------------------------------------------
	esdptr = esdalloc(maxcb) - allocate an esd and buffer
------------------------------------------------------------------------*/
ESD *
esdalloc(maxcb)
int maxcb;					 /* desired maxcb */
{
	ESD *tesd;
	int actual_cb;

	/* we get an extra character to ensure room for null past maxcb */
	actual_cb = maxcb + 1;
	if (actual_cb & 1)		 /* even allocation */
		++actual_cb;

	if (!(tesd = (ESD *) malloc(sizeof(ESD))))
		return ((ESD *) 0);	 /* return NULL if failure */

	if (!(tesd->pb = malloc(actual_cb)))
	{
		free((char *)tesd);
		return ((ESD *) 0);	 /* return NULL if failure */
	}

	esdinit(tesd, tesd->pb, maxcb);
	return (tesd);

}							 /* end of esdalloc */

/*+-----------------------------------------------------------------------
	esdptr = esdrealloc(maxcb)	- realloc an esd buffer

may only be used to enlarge an esd buffer
this used to use realloc(), which did a lot of unnecessary copying
also no more abnormal program termination on memory failure
------------------------------------------------------------------------*/
int
esdrealloc(tesd, maxcb)
ESD *tesd;
int maxcb;					 /* desired maxcb */
{
	int actual_cb;
	char *newpb;

	if (!tesd || (tesd->maxcb > maxcb))
		return (eInternalLogicError);

	/* enforce our limit */
	if (maxcb > ESD_MAXSZ)
		return (eBufferTooSmall);

	/* we get an extra character to ensure room for null past maxcb */
	actual_cb = maxcb + 1;
	if (actual_cb & 1)		 /* even allocation */
		++actual_cb;

	if (!(newpb = malloc(actual_cb)))
		return (eNoMemory);

	if (tesd->cb)
		memcpy(newpb, tesd->pb, tesd->cb);

	free(tesd->pb);
	tesd->pb = newpb;
	tesd->maxcb = maxcb;
	esd_null_terminate(tesd);
	return (0);

}							 /* end of esdrealloc */

/*+-----------------------------------------------------------------------
	esdfree(esdptr) - free an allocated esd
------------------------------------------------------------------------*/
void
esdfree(tesd)
ESD *tesd;
{
	if (tesd)
	{
		free(tesd->pb);
		free((char *)tesd);
	}
}

/*+-------------------------------------------------------------------------
    esdcat(dest,suffix,realloc_ok) - "strcat" for ESDs

  Append 'suffix' contents to 'dest'
  if realloc_ok true, expand 'dest' as necessary

  Returns: 0 - success
           eNoMemory
           eBufferTooSmall
--------------------------------------------------------------------------*/
int
esdcat(dest, suffix, realloc_ok)
ESD *dest;
ESD *suffix;
int realloc_ok;
{
	int erc = 0;
	int new_maxcb = dest->cb + suffix->cb;

	if (dest->maxcb < new_maxcb)
	{
		if (!realloc_ok)
			return (eBufferTooSmall);
		if (erc = esdrealloc(dest, new_maxcb))
			return (erc);
	}

	memcpy(dest->pb + dest->cb, suffix->pb, suffix->cb + 1); /* null too */
	dest->cb += suffix->cb; /* but do not include null in count */

	return (0);

}							 /* end of esdcat */

/*+-------------------------------------------------------------------------
    esdstrcat(tesd,zstr) - "strcat" for ESDs

similar to esdcat(), but with automatic esd growth
--------------------------------------------------------------------------*/
int
esdstrcat(tesd, zstr)
ESD *tesd;
char *zstr;
{
	int zstrlen = strlen(zstr);
	int erc = 0;

	if (zstrlen > (tesd->maxcb - tesd->cb))
	{
		if (erc = esdrealloc(tesd, tesd->cb + zstrlen))
			return (erc);
	}

	if (zstrlen)
	{
		strncpy(tesd->pb + tesd->cb, zstr, zstrlen);
		tesd->cb += zstrlen;
		esd_null_terminate(tesd);
	}

	return (erc);

}							 /* end of esdstrcat */

/*+-------------------------------------------------------------------------
    esdprefix(tesd,zstr) - "prefix" for ESDs

similar to esdcat(), but with automatic esd growth
--------------------------------------------------------------------------*/
int
esdprefix(tesd, zstr)
ESD *tesd;
char *zstr;
{
	int zstrlen = strlen(zstr);
	int erc = 0;

	if (zstrlen > (tesd->maxcb - tesd->cb))
	{
		if (erc = esdrealloc(tesd, tesd->cb + zstrlen))
			return (erc);
	}

	if (zstrlen)
	{
		mem_cpy(tesd->pb + zstrlen, tesd->pb, tesd->cb);
		memcpy(tesd->pb, zstr, zstrlen);
		tesd->cb += zstrlen;
		esd_null_terminate(tesd);
	}

	return (erc);

}							 /* end of esdprefix */

/*+-------------------------------------------------------------------------
	esdshow(tesd,title) - display an ESD's contents and indices
--------------------------------------------------------------------------*/
void
esdshow(tesd, title)
ESD *tesd;
char *title;
{
	int itmp;

	if (title && *title)
	{
		pputs(title);
		pputs("\n");
	}
	esd_null_terminate(tesd);
	pputs(tesd->pb);
	pputs("\n");
	for (itmp = 0; itmp <= tesd->cb; itmp++)
	{
		if (itmp == tesd->old_index)
			pputc('^');
		else if (itmp == tesd->index)
			pputc('^');
		else
			pputc(' ');
		if ((itmp > tesd->old_index) && (itmp > tesd->index))
			break;
	}
	pputs("\n");

}							 /* end of esdshow */

/*+-------------------------------------------------------------------------
	esddump(tesd,title) - display an ESD's contents and indices
--------------------------------------------------------------------------*/
void
esddump(tesd, title)
ESD *tesd;
char *title;
{
	pprintf("esd@%08X: ",tesd);
	if (title && *title)
	{
		pputs(title);
		pputs(": ");
	}

	pprintf("pb=%08X ",tesd->pb);
	pprintf("maxcb=%d ",tesd->maxcb);
	pprintf("cb=%d ",tesd->cb);
	pprintf("i=%d ",tesd->index);
	pprintf("oi=%d\n",tesd->old_index);
	hex_dump(tesd, sizeof *tesd, "", 1);
	hex_dump(tesd->pb, tesd->cb + 1, "", 1);

}							 /* end of esddump */

/*+----------------------------------------------------------------
	strindex(str1, str2) - string index function

  Returns position of 'str2' in 'str1' if found
  If 'str2' is null, then 0 is returned (null matches anything)
  Returns -1 if not found
-----------------------------------------------------------------*/
int
strindex(str1, str2)
char *str1;					 /* the (target) string to search */
char *str2;					 /* the (comparand) string to search for */
{
	int istr1 = 0;
	int lstr2 = strlen(str2);
	char *mstr = str1;		 /* the (target) string to search */

	if (*str2 == 0)			 /* null string matches anything */
		return (0);

	while (*mstr)
	{
		if (*mstr == *str2)
		{					 /* we have a first char match... does rest of
							  * string match? */
			if (!strncmp(mstr, str2, lstr2))
				return (istr1);	/* if so, return match position */
		}
		mstr++;
		istr1++;
	}

	return (-1);			 /* if we exhaust target string, flunk */

}							 /* end of strindex */

/*+-------------------------------------------------------------------------
	esdstrindex(esd1,esd2,index1_flag,index2_flag)

  Call strindex with esd1->pb and esd2->pb.
  If index1_flag != 0, esd1->pb + esd1->index passed
  If index2_flag != 0, esd2->pb + esd2->index passed
--------------------------------------------------------------------------*/
esdstrindex(esd1, esd2, index1_flag, index2_flag)
ESD *esd1;
ESD *esd2;
int index1_flag;
int index2_flag;
{
	return (strindex(
		(index1_flag) ? esd1->pb : esd1->pb + esd1->index,
		(index2_flag) ? esd2->pb : esd2->pb + esd2->index));

}							 /* end of esdstrindex */

/*+----------------------------------------------------------------
    keyword_lookup(ktable,keywd)

  Lookup string in keyword_table struct array
  Returns table->key_token if 'keywd' found in
  'table', else -1

  Beware substrings.  "type","typedef" will both match "type"
  This procedure fell in here and it too late to move it.
-----------------------------------------------------------------*/
keyword_lookup(ktable, keywd)
KEYTAB *ktable;
char *keywd;
{
/* int plen = strlen(keywd); */

	while (ktable->key_word)
	{
/*		if(!strncmp(ktable->key_word,keywd,plen)) */
		if (!strcmp(ktable->key_word, keywd))
			return (ktable->key_token);
		++ktable;
	}						 /* end of while */

	return (-1);			 /* search failed */

}							 /* end of keyword_lookup */

/*+----------------------------------------------------------------
    skip_cmd_break(tesd) - finds next non-break

  'tesd' is an esd with valid 'index' field
  Returns  0             index field points to non-break character
           eNoParameter  end of command found
-----------------------------------------------------------------*/
int
skip_cmd_break(tesd)
ESD *tesd;
{
	int cb = tesd->cb;
	int idx = tesd->index;
	char *pb = tesd->pb;

	while (idx < cb)
	{
		if (!isspace((uchar) * (pb + idx)))
			break;
		idx++;
	}
	tesd->old_index = tesd->index = idx;
	if (idx >= cb)
		return (eNoParameter);
	return (0);

}							 /* end of skip_cmd_break */

/*+-------------------------------------------------------------------------
	end_of_cmd(tesd) - return 1 if at end of command
--------------------------------------------------------------------------*/
int
end_of_cmd(tesd)
ESD *tesd;
{
	if (skip_cmd_break(tesd) || (*(tesd->pb + tesd->index) == ';') ||
		(*(tesd->pb + tesd->index) == '#'))
		return (1);
	return (0);
}							 /* end of end_of_cmd */

/*+-------------------------------------------------------------------------
    erc = skip_cmd_char(tesd,skipchar)
--------------------------------------------------------------------------*/
int
skip_cmd_char(tesd, skipchar)
ESD *tesd;
char skipchar;
{
	int erc;

	if (erc = skip_cmd_break(tesd))
		return (erc);

	if (tesd->pb[tesd->index] == skipchar)
	{
		++tesd->index;
		return (0);
	}

	return (eSyntaxError);

}							 /* end of skip_cmd_char */

/*+-------------------------------------------------------------------------
    erc = skip_colon(tesd)
--------------------------------------------------------------------------*/
int
skip_colon(tesd)
ESD *tesd;
{
	int erc;

	if (erc = skip_cmd_break(tesd))
		return (erc);

	if (tesd->pb[tesd->index] == ':')
	{
		++tesd->index;
		return (0);
	}

	return (eCommaExpected);

}							 /* end of skip_colon */

/*+-------------------------------------------------------------------------
    erc = skip_comma(tesd)
--------------------------------------------------------------------------*/
int
skip_comma(tesd)
ESD *tesd;
{
	int erc;

	if (erc = skip_cmd_break(tesd))
		return (erc);

	if (tesd->pb[tesd->index] == ',')
	{
		++tesd->index;
		return (0);
	}

	return (eCommaExpected);

}							 /* end of skip_comma */

/*+-------------------------------------------------------------------------
    erc = skip_paren(fparam,LEFT or RIGHT)
--------------------------------------------------------------------------*/
int
skip_paren(tesd, fLeft)
ESD *tesd;
int fLeft;
{
	int erc;
	char search = (fLeft) ? '(' : ')';

	if (erc = skip_cmd_break(tesd))
		return (erc);

	if (tesd->pb[tesd->index] == search)
	{
		tesd->index++;
		return (0);
	}
	return ((fLeft) ? eMissingLeftParen : eMissingRightParen);

}							 /* end of skip_paren */

/*+-------------------------------------------------------------------------
	get_cmd_char(tesd,pchar)
--------------------------------------------------------------------------*/
int
get_cmd_char(tesd, pchar)
ESD *tesd;
char *pchar;
{
	int erc;

	if (erc = skip_cmd_break(tesd))
		return (erc);
	*pchar = tesd->pb[tesd->index++];
	return (0);

}							 /* end of get_cmd_char */

/*+----------------------------------------------------------------
    get_alpha_zstr(&esd,&strbuf,strbuf_maxcb)

  places next alphabetic string token [A-Za-z_] into
  the null-terminated 'strbuf' string.  returns 0 or -1
  or skip_cmd_break error codes
-----------------------------------------------------------------*/
int
get_alpha_zstr(tesd, strbuf, strbuf_maxcb)
ESD *tesd;
char *strbuf;
int strbuf_maxcb;
{
	int izstr;
	int schar;
	char *pb = tesd->pb;

	if (izstr = skip_cmd_break(tesd))
		return (izstr);
	izstr = 0;
	while ((izstr < strbuf_maxcb - 1) && (tesd->index < tesd->cb))
	{
		schar = pb[tesd->index] & 0xFF;
		if ((!isalpha(schar)) && (schar != '_'))
			break;
		strbuf[izstr++] = schar;
		tesd->index++;
	}

	strbuf[izstr] = 0;
	return (izstr ? 0 : eBadParameter);

}							 /* end of get_alpha_zstr */

/*+----------------------------------------------------------------
    get_alphanum_zstr(&esd,&strbuf,strbuf_maxcb)

  places next alphanumeric string token [A-Za-z0-9_]
  into the null-terminated 'strbuf' string.  returns 0
  or -1 or skip_cmd_break error codes
-----------------------------------------------------------------*/
int
get_alphanum_zstr(tesd, strbuf, strbuf_maxcb)
ESD *tesd;
char *strbuf;
int strbuf_maxcb;
{
	int izstr = 0;
	int schar;
	int cb = tesd->cb;
	int idx;

	if (izstr = skip_cmd_break(tesd))
		return (izstr);

	idx = tesd->index;
	while ((izstr < strbuf_maxcb - 1) && (idx < cb))
	{
		schar = tesd->pb[idx++] & 0xFF;
		if (isalnum(schar) || (schar == '_'))
			strbuf[izstr++] = schar;
		else
		{
			--idx;
			break;
		}
	}

	tesd->index = idx;
	strbuf[izstr] = 0;
	return (izstr ? 0 : eBadParameter);

}							 /* end of get_alphanum_zstr */

/*+----------------------------------------------------------------
    get_numeric_zstr(&esd,&strbuf,strbuf_maxcb)
    gets next numeric string token places it
    into the null-terminated 'strbuf' string.  returns 0 or -1
    or skip_cmd_break error codes
-----------------------------------------------------------------*/
int
get_numeric_zstr(tesd, strbuf, strbuf_maxcb)
ESD *tesd;
char *strbuf;
int strbuf_maxcb;
{
	int izstr;
	int schar;

	if (izstr = skip_cmd_break(tesd))
		return (izstr);

	while ((izstr < strbuf_maxcb - 1) && (tesd->index < tesd->cb))
	{
		schar = tesd->pb[tesd->index++] & 0xFF;
		if (isdigit(schar))
			strbuf[izstr++] = schar;
		else
		{
			--tesd->index;
			break;
		}
	}

	strbuf[izstr] = 0;
	return (izstr ? 0 : eBadParameter);

}							 /* end of get_numeric_zstr */

/*+-----------------------------------------------------------------------
	get_numeric_value(tesd,&long_var)
------------------------------------------------------------------------*/
get_numeric_value(tesd, value)
ESD *tesd;
long *value;
{
	int erc;
	char buf[32];

	if (erc = get_numeric_zstr(tesd, buf, sizeof(buf)))
		return (erc);
	sscanf(buf, "%ld", value);
	return (0);

}							 /* end of get_numeric_value */

/*+----------------------------------------------------------------
    get_ipaddr_zstr(&esd,&strbuf,strbuf_maxcb)

  places next ip address string token [A-Za-z0-9_.]
  into the null-terminated 'strbuf' string.  returns 0
  or -1 or skip_cmd_break error codes
-----------------------------------------------------------------*/
int
get_ipaddr_zstr(e, strbuf, strbuf_maxcb)
ESD *e;
char *strbuf;
int strbuf_maxcb;
{
	int izstr = 0;
	int schar;
	int erc;
	int cb = e->cb;
	int entry_idx;
	int idx;
	int dot_count = 0;
	u_long ipaddr;

	if (erc = skip_cmd_break(e))
		return (end_of_cmd(e) ? eMissingIpAddress : eBadIpAddress);
	entry_idx = e->index;

	idx = e->index;
	while ((izstr < strbuf_maxcb - 1) && (idx < cb))
	{
		schar = e->pb[idx++] & 0xFF;
		if (isdigit(schar))
			strbuf[izstr++] = schar;
		else if (schar == '.')
		{
			dot_count++;
			strbuf[izstr++] = schar;
		}
		else
		{
			--idx;
			break;
		}
	}

	e->index = idx;
	e->old_index = e->index;
	strbuf[izstr] = 0;
	ipaddr = inet_atou(strbuf);
	erc = ((izstr && (dot_count == 3) && ipaddr) ? 0 : eBadIpAddress);
	if (erc)
		e->index = entry_idx;
	return (erc);

}								/* end of get_ipaddr_zstr */

/*+----------------------------------------------------------------
    get_word_zstr(&esd,&strbuf,strbuf_maxcb)

  gets next word (continuous string of characters without spaces
  or tabs) returns 0 or -1 or skip_cmd_break error codes
-----------------------------------------------------------------*/
int
get_word_zstr(tesd, strbuf, strbuf_maxcb)
ESD *tesd;
char *strbuf;
int strbuf_maxcb;
{
	int izstr;
	int schar;

	if (izstr = skip_cmd_break(tesd))
		return (izstr);

	strbuf_maxcb--;
	while ((izstr < strbuf_maxcb) && (tesd->index < tesd->cb))
	{
		schar = tesd->pb[tesd->index++];
		if ((schar > 0x20) && (schar <= 0x7e))
			strbuf[izstr++] = schar;
		else
		{
			--tesd->index;
			break;
		}
	}

	strbuf[izstr] = 0;
	return (izstr ? 0 : eBadParameter);

}							 /* end of get_word_zstr */

/*+-------------------------------------------------------------------------
    esd_strip_trail_break(tesd)
--------------------------------------------------------------------------*/
void
esd_strip_trail_break(ztext)
ESD *ztext;
{
	while (ztext->cb &&
		((ztext->pb[ztext->cb - 1] == 0x20) || (ztext->pb[ztext->cb - 1] == 0x20)))
	{
		ztext->cb--;
	}
}							 /* end of esd_strip_trail_break */

/*+-------------------------------------------------------------------------
	esdfgets(&esd,fileptr)

  stdio read from FILE *fileptr into esd
  returns tesd->cb set up not including trailing nl, tesd->index == 0
--------------------------------------------------------------------------*/
int
esdfgets(tesd, fileptr)
ESD *tesd;
FILE *fileptr;
{
	char *cp;

	tesd->cb = 0;
	if (!fgets(tesd->pb, tesd->maxcb + 1, fileptr))
		return (eEOF);
	if (!(cp = strchr(tesd->pb, 0x0A)))
		return (eBufferTooSmall);
	tesd->cb = (int)(cp - tesd->pb);
	esd_null_terminate(tesd);
	tesd->index = 0;
	tesd->old_index = 0;
	return (0);

}							 /* end of esdfgets */

/*+-------------------------------------------------------------------------
	esdfputs(&esd,fileptr,index_flag,nl_flag)

  write esd contents to stdio FILE *fileptr
  if index_flag is true, write from tesd->index thru end of esd
  otherwise, from start of esd
  if nl_flag is true, append nl to write, else just esd contents
--------------------------------------------------------------------------*/
int
esdfputs(tesd, fileptr, index_flag, nl_flag)
ESD *tesd;
FILE *fileptr;
int index_flag;
int nl_flag;
{
	char *cp;
	int write_length;

	if (index_flag)
	{
		cp = &tesd->pb[tesd->index];
		write_length = tesd->cb - tesd->index;
	}
	else
	{
		cp = tesd->pb;
		write_length = tesd->cb;
	}

	if (write_length)
		fwrite(cp, write_length, 1, fileptr);

	if (nl_flag)
		fputc(0x0A, fileptr);

	return (0);
}							 /* end of esdfputs */

/*+-------------------------------------------------------------------------
    get_switches(tesd,switches,switches_max)
--------------------------------------------------------------------------*/
int
get_switches(tesd, switches, switches_max)
ESD *tesd;
char *switches;
int switches_max;
{
	int erc;
	int idx;
	char *pb = tesd->pb;
	int cb = tesd->cb;
	char schar;

	*switches = 0;

	if (erc = skip_cmd_break(tesd))
		return (erc);

	idx = tesd->index;
	if (*(pb + idx) != '-')
		return (eNoSwitches);

	if (switches_max < 3)
		return (eSwitchesTooLong);

	switches_max--;			 /* for trailing null */
	*switches++ = '-';
	switches_max--;
	idx++;
	while (idx < cb)
	{
		schar = *(pb + idx++);
		if (switches_max > 0)
			*switches++ = schar;
		switches_max--;
		if (isspace((uchar) schar))
			break;
	}

	tesd->index = idx;
	*switches = 0;
	return ((switches_max < 0) ? eSwitchesTooLong : 0);

}							 /* end of get_switches() */

/*+-------------------------------------------------------------------------
    esdclone(e) - clone an esd
--------------------------------------------------------------------------*/
ESD *
esdclone(e)
ESD *e;
{
	ESD *e2 = esdalloc(e->maxcb);
	esdcat(e2, e, 1);
	return(e2);
}								/* end of esdcopy */

/* vi: set tabstop=4 shiftwidth=4: */
/* end of esdutil.c */
