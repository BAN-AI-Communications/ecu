/* CHK=0xD3B5 */
/*+-------------------------------------------------------------------------

	utc2.c - modified version of c.s.m utc.c

derived from:
> Newsgroups: comp.sources.misc
> Subject: v03i079: N.B.S. Time Service program
> Posting-number: Volume 3, Issue 79
> Submitted-by: "BALDWIN" <mike@whutt.UUCP>
> Archive-name: utc

Revised comments by the original author:

The Naval Observatory clock (+1 202 653 0351) prints this every second:

jjjjj ddd hhmmss UTC

jjjjj	Julian date modulo 2400000
ddd		days since beginning of year
hhmmss	time of day in Universal Time Coordinated

This program's switches:

    -s          read time from stdin and set the system time via stime(2)
                (NOTE: NBSsetclk.c replaces this function to a large degree)

    -p	        prints the time via ctime(3C)

    no switch   mimic the Naval Observatory for about one minute

When invoked without options, it prints the time in Naval clock format
for about a minute.  Thus, it can be installed as a login shell to provide
time service for your other systems without having them all call DC.
Just make this program the shell of a no-password username 'utc'.

As a test, "utc | utc -p" should print the current time.  You can pipe
cu right into it, so set up a crontab entry to execute

	cu 1-201-653-0351 | utc -s

You may have to fix your cu to die properly when it receives a SIGPIPE.
I have my crontab entry run once a day, but it only calls DC if the
time hasn't been set in over a week.  A simple shell file accomplishes
this:

	LAST=/etc/.lastutc
	[ -z "`find $LAST -mtime -7 -print`" ] &&
	cu 1-202-653-0351 | utc -s && >$LAST

Alternatively, you may use NBSsetclk and ecu with a ~/.ecu/phone entry:

nbssetclk:12026530351::1200:N:Naval Observatory Time:6:1

and the nbssetclk.ep provided with this distribution.

Then the script becomes:

	LAST=/etc/.lastutc
	[ -z "`find $LAST -mtime -7 -print`" ] &&
	ecu -D -p nbssetclk >$LAST

--------------------------------------------------------------------------*/
/*+:EDITS:*/
/*:01-24-1997-02:38-wht@yuriatin-SOURCE RELEASE 4.00 */
/*:12-29-1992-11:49-wht@n4hgf-work around funky julian date calculation */


#define FIX_JDATE /* author's yday calculation did not work */

#include <stdio.h>
#include <time.h>
#include <sys/types.h>

#define	EPOCH		40587			/* UNIX starts JD 2440587, */
#define	leap(y, m)	((y+m-1 - 70%m) / m)	/* also known as 1/1/70 */
#define	TONE		'*'
#define	TIME		"\n%05ld %03d %02d%02d%02d UTC"

main(argc, argv)
int	argc;
char	*argv[];
{
	int	setflg = 0, prtflg = 0;
	int	y, d, h, m, s;
	long	j;
	time_t	now;
	int	c;
#ifdef FIX_JDATE
	struct tm *ut;
#endif /* FIX_JDATE */

	while ((c = getopt(argc, argv, "sp")) != EOF)
		switch (c) {
		case 's': setflg++; break;
		case 'p': prtflg++; break;
		default:
			fprintf(stderr, "usage: %s [-s] [-p]\n", argv[0]);
			return 1;
		}
	if (setflg || prtflg) {
		while ((c = getchar()) != TONE)
			if (c == EOF)
				return 1;
		if (scanf(TIME, &j, &d, &h, &m, &s) != 5)
			return 1;
		now = (((j - EPOCH) * 24 + h) * 60 + m) * 60 + s;
		if (setflg && stime(&now) == -1)
			perror(argv[0]);
		if (prtflg)
			fputs(ctime(&now), stdout);
	} else {
		for (c = 0; c < 60; c++) {
			time(&now);
#ifdef FIX_JDATE
			ut = gmtime(&now);
			s = ut->tm_sec;
			m = ut->tm_min;
			h = ut->tm_hour;
			d = ut->tm_yday + 1;
			now /= 60 * 60 * 24;
			j = now + EPOCH;
			y = (now /= 365);
#else
			s = (now % 60);
			m = (now /= 60) % 60;
			h = (now /= 60) % 24;
			d = (now /= 24) % 365; /* I get wrong results -- wht */
			j = now + EPOCH;
			y = (now /= 365);
			d += 1 - leap(y, 4) + leap(y, 100) - leap(y, 400);
#endif /* FIX_JDATE */
			putchar(TONE);
			printf(TIME, j, d, h, m, s);
			putchar('\n');
			fflush(stdout);
#if defined(M_SYSV) || defined(i386)
			nap(1000);
#else
#if defined(sun)
			usleep(1000 * 1000);
#else
			sleep(1);	/* less accurate interval */
#endif /* sun */
#endif /* M_SYSV || i386 */
		}
	}
	exit(0);
}
