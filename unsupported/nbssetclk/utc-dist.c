/* CHK=0xB6F9 */
/*
Path: devon!vu-vlsi!cbmvax!rutgers!ucsd!ames!necntc!ncoast!allbery
From: mike@whutt.UUCP (BALDWIN)
Newsgroups: comp.sources.misc
Subject: v03i079: N.B.S. Time Service program
Summary: I've got (a small) one
Keywords: naval observatory
Message-ID: <3506@whutt.UUCP>
Date: 12 Jul 88 14:48:27 GMT
Sender: allbery@ncoast.UUCP
Reply-To: mike@whutt.UUCP (BALDWIN)
Distribution: na
Organization: AT&T Bell Laboratories
Lines: 107
Approved: allbery@ncoast.UUCP

Posting-number: Volume 3, Issue 79
Submitted-by: "BALDWIN" <mike@whutt.UUCP>
Archive-name: utc

I've been running such a program, which I wrote, at home for over six
months now.  It's written in C, and runs under System V (or any UNIX
system with an stime(2) system call).  It consists of a single program
called "utc" (universal time coordinated).  When invoked with options,
it reads the Naval clock and does one or both of these things:

	-s	sets the time via stime(2)
	-p	prints the time via ctime(3C)

If it can't read the time from the standard input, it exits non-zero.
When invoked without options, it prints the time in Naval clock format
for about a minute.  Thus, it can be installed as a login shell to provide
time service for your other systems without having them all call DC.
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
*/
/* comp.sources.misc v03i079
 * The Naval Observatory clock (+1 202 653 0351) prints this every second:
 *
 *	*
 *	jjjjj ddd hhmmss UTC
 *
 * jjjjj	Julian date modulo 2400000
 * ddd		days since beginning of year
 * hhmmss	time of day in Universal Time Coordinated
 */

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
			s = (now % 60);
			m = (now /= 60) % 60;
			h = (now /= 60) % 24;
			d = (now /= 24) % 365;
			j = now + EPOCH;
			y = (now /= 365);
			d += 1 - leap(y, 4) + leap(y, 100) - leap(y, 400);
			putchar(TONE);
			printf(TIME, j, d, h, m, s);
			putchar('\n');
			fflush(stdout);
			sleep(1);
		}
	}
	return 0;
}
