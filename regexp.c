/*+-------------------------------------------------------------------------
	regexp.c -- regular expression functions made sane
	wht@wht.net

  Defined functions:
	advance(lp, compile_ptr)
	compile(pattern, compile_ptr, endbuf)
	ecmp(a, b, count)
	get_count_range(regexp)
	regexp_compile(regexp, cmpbuf, cmpbuf_size)
	regexp_operation(match_str, regexp_str, rtn_value)
	regexp_scan(cmpbuf, str_to_search, match, matchlen)
	step(p1, p2)

--------------------------------------------------------------------------*/
/*+:EDITS:*/
/*:04-26-2000-11:16-wht@bob-RELEASE 4.42 */
/*:02-01-1998-18:32-wht@fep-overhaul completely */
/*:01-27-1998-12:15-wht@apollo-no more use of setjmp */
/*:11-16-1997-20:45-wht@fep-adapt error handling */
/*:11-09-1997-19:40-wht@gyro-x0.50--- CONTROL POINT proc language integrated */
/*:11-09-1997-19:40-wht@gyro-edit notes have been sparse to get here */
/*:11-07-1997-20:04-wht@fep-successful meld of proc language */
/*:01-24-1997-02:38-wht@varykino-ECU SOURCE RELEASE 4.00 */

#include "ecuerror.h"
#include "esd.h"
#include "var.h"

#undef uchar
#define uchar unsigned char

#if 1						 /* NO PARENTHESIS-BOUNDED GROUPS */
#define	PAREN_GROUPS_MAX	0
#else
#define	PAREN_GROUPS_MAX	9
#endif

#define	RTK_CHARACTER			0x01
#define	RTK_ANY_CHAR			0x02
#define	RTK_CHAR_SET			0x03
#define	RTK_DOLLAR				0x04
#define	RTK_EXPR_END			0x05
#define	RTK_PAREN_GROUP_START	0x06
#define	RTK_PAREN_GROUP_END		0x07

#define	RTK_STAR_MODIFIER		0x40
#define	RTK_RANGE_MODIFIER		0x20

#if (PAREN_GROUPS_MAX)
/*
 * parenthesis-bound group definitions
 */
char *paren_group_starts[PAREN_GROUPS_MAX];
char *paren_group_ends[PAREN_GROUPS_MAX];
int paren_group_start_count;
int paren_group_end_count;

#endif

extern int proc_trace;
extern int proc_level;

static char *match_start;
static char *match_end;
static char *locs;
static int circumflex;
static int count_range_low;
static int count_range_size;

static unsigned char bittab[] =
{1, 2, 4, 8, 16, 32, 64, 128};

#define	PLACE(c)	compile_ptr[c >> 3] |= bittab[c & 07]
#define	ISTHERE(c)	(compile_ptr[c >> 3] & bittab[c & 07])

/*+-------------------------------------------------------------------------
	compile(pattern,ep,endbuf)
--------------------------------------------------------------------------*/
static int
compile(pattern, compile_ptr, endbuf)
char *pattern;
char *compile_ptr;
char *endbuf;
{
	int c;
	char *pattern_ptr = pattern;
	char *last_compile_ptr = 0;
	char neg;
	int lc;
	int itmp;
	int comma_count;

#if (PAREN_GROUPS_MAX > 0)
	char paren_group[PAREN_GROUPS_MAX];
	char *paren_group_ptr = paren_group;
	int paren_group_closes = 0;

	paren_group_start_count = 0;	/* index to paren_group_starts */
	paren_group_end_count = 0;	/* index to paren_group_ends */

#endif

	if (!(c = *pattern_ptr++))
		return (eRegNullRE);

	/*
	 * is there a circumflex at the beginning?
	 */
	circumflex = 0;			 /* no '^' encountered */
	if (c == '^')
		circumflex++;
	else
		--pattern_ptr;

	/*
	 * walk through each RE character through the end
	 */
	while (1)
	{
		if (compile_ptr >= endbuf)
			return (eReg2Complex);
		if (!(c = *pattern_ptr++))
		{
			*compile_ptr++ = RTK_EXPR_END;
			return (0);
		}
		if ((c != '*') && ((c != '\\') || (*pattern_ptr != 0x7B)))
			last_compile_ptr = compile_ptr;
		switch (c)
		{

			case '.':
				*compile_ptr++ = RTK_ANY_CHAR;
				continue;

			case '*':
				if (!last_compile_ptr ||
					(*last_compile_ptr == RTK_PAREN_GROUP_START) ||
					(*last_compile_ptr == RTK_PAREN_GROUP_END))
				{
					goto DEFAULT_HANDLER;
				}
				*last_compile_ptr |= RTK_STAR_MODIFIER;
				continue;

			case '$':
				if (*pattern_ptr)
					goto DEFAULT_HANDLER;
				*compile_ptr++ = RTK_DOLLAR;
				continue;

			case '[':
				if ((compile_ptr + 16 + 1) >= endbuf)
					return (eReg2Complex);

				*compile_ptr++ = RTK_CHAR_SET;
				lc = 0;
				for (itmp = 0; itmp < 16; itmp++)
					compile_ptr[itmp] = 0;

				neg = 0;
				if ((c = *pattern_ptr++) == '^')
				{
					neg = 1;
					c = *pattern_ptr++;
				}

				do
				{
					if (!c)
						return (eRegBracketImb);
					if ((c == '-') && (lc != 0))
					{
						if ((c = *pattern_ptr++) == ']')
						{
							PLACE('-');
							break;
						}
						while (lc < c)
						{
							PLACE(lc);
							lc++;
						}
					}
					if (c == '\\')
					{
						switch (c = *pattern_ptr++)
						{
							case 'n':
								c = '\n';
								break;
						}
					}
					lc = c;
					PLACE(c);
				}
				while ((c = *pattern_ptr++) != ']')
				;

				if (neg)
				{
					for (itmp = 0; itmp < 16; itmp++)
						compile_ptr[itmp] ^= -1;
					compile_ptr[0] &= 0376;
				}

				compile_ptr += 16;

				continue;

			case '\\':
				switch (c = *pattern_ptr++)
				{

#if (!PAREN_GROUPS_MAX)		 /* no need to support parentheses */
					case 0x28:	/* open paren */
					case 0x29:	/* close paren */
						return (eRegNoParen);
#else
					case 0x28:	/* open paren */
						if (paren_group_start_count >= PAREN_GROUPS_MAX)
							return (eReg2ManyOpens);
						*paren_group_ptr++ = paren_group_start_count;
						*compile_ptr++ = RTK_PAREN_GROUP_START;
						*compile_ptr++ = paren_group_start_count++;
						continue;

					case 0x29:	/* close paren */
						if ((paren_group_ptr <= paren_group) ||
							(++paren_group_end_count != paren_group_start_count))
						{
							return (eReg2FewOpens);
						}
						*compile_ptr++ = RTK_PAREN_GROUP_END;
						*compile_ptr++ = *--paren_group_ptr;
						paren_group_closes++;
						continue;
#endif

					case 0x7B:	/* open brace */
						if (!last_compile_ptr)
							goto DEFAULT_HANDLER;
						*last_compile_ptr |= RTK_RANGE_MODIFIER;
						comma_count = 0;	/* no comma yet */
					  GET_NUMERIC:
						c = *pattern_ptr++;
						itmp = 0;	/* accumulator for numeric value */
						do
						{
							if ('0' <= c && c <= '9')
								itmp = 10 * itmp + c - '0';
							else
								return (eRegBadNum);
						}
						while (((c = *pattern_ptr++) != '\\') && (c != ','))
						;
						if (itmp >= 255)
							return (eRegRange);
						*compile_ptr++ = itmp;
						if (c == ',')
						{
							if (comma_count++)
								return (eReg2ManyArgs);
							if ((c = *pattern_ptr++) == '\\')
								*compile_ptr++ = (char)255;
							else
							{
								--pattern_ptr;
								goto GET_NUMERIC;	/* get 2nd number */
							}
						}
						if (*pattern_ptr++ != 0x7D)	/* close brace */
							return (eRegBraceExp);
						if (!comma_count)	/* one number */
							*compile_ptr++ = itmp;
						else if ((compile_ptr[-1] & 255) <
							(compile_ptr[-2] & 255))
						{
							return (eRegBraceRange);
						}
						continue;

					default:
						break;
				}

				/*
				 * Drop through to default to use \ to turn off special
				 * chars
				 */

			  DEFAULT_HANDLER:
			default:
				last_compile_ptr = compile_ptr;
				*compile_ptr++ = RTK_CHARACTER;
				*compile_ptr++ = c;
		}
	}
	/* NOTREACHED */
}							 /* end of compile */

/*+-------------------------------------------------------------------------
	get_count_range(regexp)
--------------------------------------------------------------------------*/
static void
get_count_range(regexp)
char *regexp;
{
	count_range_low = *regexp++ & 255;
	count_range_size = (*((uchar *) regexp) == 255)
		? 20000 :
		((*(uchar *) regexp) - count_range_low);
}							 /* end of get_count_range */

/*+-------------------------------------------------------------------------
	advance(lp,compile_ptr)
--------------------------------------------------------------------------*/
static int
advance(lp, compile_ptr)
char *lp, *compile_ptr;
{
	char *curlp;
	char c;

	while (1)
	{
		switch (*compile_ptr++)
		{

			case RTK_CHARACTER:
				if (*compile_ptr++ == *lp++)
					continue;
				return (0);

			case RTK_ANY_CHAR:
				if (*lp++)
					continue;
				return (0);

			case RTK_DOLLAR:
				if (*lp == 0)
					continue;
				return (0);

			case RTK_EXPR_END:
				match_end = lp;
				return (1);

			case RTK_CHAR_SET:
				c = *lp++ & 0177;
				if (ISTHERE(c))
				{
					compile_ptr += 16;
					continue;
				}
				return (0);

			case RTK_CHAR_SET | RTK_RANGE_MODIFIER:
				get_count_range(compile_ptr + 16);
				while (count_range_low--)
				{
					c = *lp++ & 0177;
					if (!ISTHERE(c))
						return (0);
				}
				curlp = lp;
				while (count_range_size--)
				{
					c = *lp++ & 0177;
					if (!ISTHERE(c))
						break;
				}
				if (count_range_size < 0)
					lp++;
				compile_ptr += 18;	/* 16 + 2 */
				goto STAR_REPEATER;

#if (PAREN_GROUPS_MAX > 0)
			case RTK_PAREN_GROUP_START:
				paren_group_starts[(unsigned)*compile_ptr++] = lp;
				continue;

			case RTK_PAREN_GROUP_END:
				paren_group_ends[(unsigned)*compile_ptr++] = lp;
				continue;
#endif

			case RTK_CHARACTER | RTK_RANGE_MODIFIER:
				c = *compile_ptr++;
				get_count_range(compile_ptr);
				while (count_range_low--)
				{
					if (*lp++ != c)
						return (0);
				}
				curlp = lp;
				while (count_range_size--)
					if (*lp++ != c)
						break;
				if (count_range_size < 0)
					lp++;
				compile_ptr += 2;
				goto STAR_REPEATER;

			case RTK_ANY_CHAR | RTK_RANGE_MODIFIER:
				get_count_range(compile_ptr);
				while (count_range_low--)
				{
					if (*lp++ == '\0')
						return (0);
				}
				curlp = lp;
				while (count_range_size--)
				{
					if (*lp++ == '\0')
						break;
				}
				if (count_range_size < 0)
					lp++;
				compile_ptr += 2;
				goto STAR_REPEATER;

			case RTK_ANY_CHAR | RTK_STAR_MODIFIER:
				curlp = lp;
				while (*lp++)
					;
				goto STAR_REPEATER;

			case RTK_CHARACTER | RTK_STAR_MODIFIER:
				curlp = lp;
				while (*lp++ == *compile_ptr)
					;
				compile_ptr++;
				goto STAR_REPEATER;

			case RTK_CHAR_SET | RTK_STAR_MODIFIER:
				curlp = lp;
				do
				{
					c = *lp++ & 0177;
				}
				while (ISTHERE(c))
				;
				compile_ptr += 16;
				goto STAR_REPEATER;

			  STAR_REPEATER:
				do
				{
					if (!--lp)
						break;
					if (advance(lp, compile_ptr))
						return (1);
				}
				while (lp > curlp);
				return (0);
		}
	}
}							 /* end of advance */

/*+-------------------------------------------------------------------------
	step(p1,p2)
--------------------------------------------------------------------------*/
static int
step(p1, p2)
char *p1, *p2;
{
	int c;

	if (circumflex)
	{
		match_start = p1;
		return (advance(p1, p2));
	}
	/* fast check for first character */
	if (*p2 == RTK_CHARACTER)
	{
		c = p2[1];
		do
		{
			if (*p1 != c)
				continue;
			if (advance(p1, p2))
			{
				match_start = p1;
				return (1);
			}
		}
		while (*p1++)
		;
		return (0);
	}
	/* regular algorithm */
	do
	{
		if (advance(p1, p2))
		{
			match_start = p1;
			return (1);
		}
	}
	while (*p1++)
	;
	return (0);
}							 /* end of step */

/*+-------------------------------------------------------------------------
	ecmp(a,b,count)
--------------------------------------------------------------------------*/
static int
ecmp(a, b, count)
char *a, *b;
int count;
{
	while (count--)
		if (*a++ != *b++)
			return (0);
	return (1);
}							 /* end of ecmp */

/*+-------------------------------------------------------------------------
	regexp_compile(regexp,cmpbuf,cmpbuf_size)

returns 0 if no compile error,
--------------------------------------------------------------------------*/
int
regexp_compile(regexp, cmpbuf, cmpbuf_size)
char *regexp;
char *cmpbuf;
int cmpbuf_size;
{
	return (compile(regexp, cmpbuf, cmpbuf + cmpbuf_size));
}							 /* end of regexp_compile */

/*+-------------------------------------------------------------------------
	regexp_scan(cmpbuf,str_to_search,&match,&matchlen)
return 1 if string match found, else 0
if string matches, match receives pointer to first byte, matchlen = length
of matching string

--------------------------------------------------------------------------*/
int
regexp_scan(cmpbuf, str_to_search, match, matchlen)
char *cmpbuf;
char *str_to_search;
char **match;
int *matchlen;
{
	int itmp = step(str_to_search, cmpbuf);

	if (itmp)
	{
		*match = match_start;
		*matchlen = (int)(match_end - match_start);
	}
	return (itmp);
}							 /* end of regexp_scan */

/*+-------------------------------------------------------------------------
	regexp_operation(match_str,regexp_str,rtn_value)

one stop operation used by procedure language:
determine if 'match_str' matches 'regexp_str', returning the index of
the match in *rtn_value and setting #I0 to the length of the match
--------------------------------------------------------------------------*/
int
regexp_operation(match_str, regexp_str, rtn_value)
char *match_str;
char *regexp_str;
long *rtn_value;
{
	int erc;

#define CMPBUF_SIZE	512
	char cmpbuf[CMPBUF_SIZE];
	char *match;
	int matchlen;

	if (erc = regexp_compile(regexp_str, cmpbuf, sizeof(cmpbuf)))
		return (erc);

	if (regexp_scan(cmpbuf, match_str, &match, &matchlen))
	{
		*rtn_value = (long)(match - match_str);
#if !defined(TEST)
		iv[0] = (long)matchlen;
		if (proc_level && proc_trace)
			pprintf("%match set $i00 = %ld\n", iv[0]);
#endif
	}
	else
		*rtn_value = -1L;

	return (0);
}							 /* end of regexp_operation */

/*+-------------------------------------------------------------------------
	main(argc,argv)
--------------------------------------------------------------------------*/
#ifdef TEST
main(argc,argv)
int argc;
char **argv;
{
	int erc;
	long ltmp;

	if( erc = regexp_operation("abcd","a.*d",&ltmp))
	{
		printf("error %04X\n",erc);
		exit(1);
	}
	printf("1  0==%d\n",ltmp);
	if( erc = regexp_operation("abcd","abc",&ltmp))
	{
		printf("error %04X\n",erc);
		exit(1);
	}
	printf("2  0==%d\n",ltmp);
	if( erc = regexp_operation("abcd","bc",&ltmp))
	{
		printf("error %04X\n",erc);
		exit(1);
	}
	printf("3  1==%d\n",ltmp);
	if( erc = regexp_operation("abcd","^abcd$",&ltmp))
	{
		printf("error %04X\n",erc);
		exit(1);
	}
	printf("4  0==%d\n",ltmp);
	if( erc = regexp_operation("Xabcd","^abcd$",&ltmp))
	{
		printf("error %04X\n",erc);
		exit(1);
	}
	printf("5  -1==%d\n",ltmp);
	if( erc = regexp_operation("abcdX","^abcd$",&ltmp))
	{
		printf("error %04X\n",erc);
		exit(1);
	}
	printf("6  -1==%d\n",ltmp);
	exit(0);
}	/* end of main */
#endif

/* vi: set tabstop=4 shiftwidth=4: */
/* end of regexp.c */
