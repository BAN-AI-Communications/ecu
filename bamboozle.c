/*+-------------------------------------------------------------------------
	bamboozle.c - ecu/ecuungetty protection scheme
	wht@wht.net

  When bad men combine, the good must associate; else they will
  fall one by one, an unpitied sacrifice in a contemptible
  struggle.  -Edmund Burke

--------------------------------------------------------------------------*/
/*+:EDITS:*/
/*:04-26-2000-11:15-wht@bob-RELEASE 4.42 */
/*:01-24-1997-02:36-wht@yuriatin-SOURCE RELEASE 4.00 */
/*:09-11-1996-19:59-wht@yuriatin-3.48-major telnet,curses,structural overhaul */
/*:09-06-1996-02:11-wht@kepler-bit fiddling fixes/cleanup */
/*:11-23-1995-11:20-wht@kepler-source control 3.37 for tsx-11 */
/*:11-14-1995-10:23-wht@kepler-3.37.80-source control point: SOCKETS */
/*:05-04-1994-04:38-wht@n4hgf-ECU release 3.30 */
/*:09-10-1992-13:58-wht@n4hgf-ECU release 3.20 */
/*:08-22-1992-15:38-wht@n4hgf-ECU release 3.20 BETA */
/*:07-25-1991-12:55-wht@n4hgf-ECU release 3.10 */
/*:01-09-1991-22:31-wht@n4hgf-ISC port */
/*:01-09-1991-21:05-wht@n4hgf-no prototype for crypt if not __STDC__ */
/*:08-14-1990-20:39-wht@n4hgf-ecu3.00-flush old edit history */

#include <stdio.h>

#if __STDC__ == 1
char *crypt(char *, char *);

#else
char *crypt();

#endif

/*+-------------------------------------------------------------------------
	bamboozle(pid) - build encrypted string based on 'pid'

If crypt not used, do something pretty basic  (probably enough)
If you are paranoid, don't use _ANY_ of these algorithms exactly
--------------------------------------------------------------------------*/
char *
bamboozle(pid)
int pid;
{
#if defined(CRYPT)
	char pidstr[16];
	char *cp;

	sprintf(pidstr, "z%08d", pid);
	pidstr[0] = 'G';		 /* fool strings searchers */
	cp = crypt(pidstr, "ba");
	return (cp);
#else /* probably enough */
	static char pidstr[16];

	sprintf(pidstr, "b%09da", (int)(((long)pid * 21) / 5));
	return (pidstr);
#endif

#ifdef VARIANT_1			 /* very paranoid */
	char pidstr[16];
	char *cp;

	sprintf(pidstr, "z%08d", pid);
	pidstr[0] = 0xFF;		 /* fool strings searchers */
	cp = crypt(pidstr, pidstr);
	return (cp);
#endif
#ifdef VARIANT_2			 /* not paranoid at all */
	char pidstr[16];

	sprintf(pidstr, "z%08d", pid - 2);
#endif
#ifdef VARIANT_3			 /* invite trouble :-) */
	return ("I_am_easy");
#endif

}							 /* end of bamboozle */

/* vi: set tabstop=4 shiftwidth=4: */
/* end of bamboozle.c */
