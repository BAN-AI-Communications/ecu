/* CHK=0xD757 */
char *revision = "1.2";

/*+-----------------------------------------------------------------------
	dutmp.c -- dump /etc/utmp
	wht@wht.net
------------------------------------------------------------------------*/
/*+:EDITS:*/
/*:04-26-2000-11:15-wht@bob-RELEASE 4.42 */
/*:01-24-1997-02:36-wht@yuriatin-SOURCE RELEASE 4.00 */
/*:09-11-1996-19:59-wht@yuriatin-3.48-major telnet,curses,structural overhaul */
/*:11-23-1995-11:20-wht@kepler-source control 3.37 for tsx-11 */
/*:11-14-1995-10:23-wht@kepler-3.37.80-source control point: SOCKETS */
/*:05-04-1994-04:38-wht@n4hgf-ECU release 3.30 */
/*:09-10-1992-13:58-wht@n4hgf-ECU release 3.20 */
/*:08-22-1992-15:38-wht@n4hgf-ECU release 3.20 BETA */
/*:04-28-1992-13:05-wht@n4hgf-clean up for inclusion in ecu */
/*:11-19-1989-16:34-wht-creation */

#include <stdio.h>
#include <fcntl.h>
#include <ctype.h>
#include <sys/types.h>
#include <utmp.h>

char *utmp_file = "/etc/utmp";

extern char *ctime();

/*+-------------------------------------------------------------------------
	main(argc,argv)
--------------------------------------------------------------------------*/
main(argc, argv)
int argc;
char **argv;
{
	struct utmp ut;
	int ufd;

	printf("dutmp %s\n", revision);

	if ((ufd = open(utmp_file, O_RDONLY, 755)) < 0)
	{
		perror(utmp_file);
		exit(1);
	}

	while (read(ufd, &ut, sizeof(ut)) > 0)
	{
#if defined(sun)
		if (!*ut.ut_name || (ut.ut_time < 0))
			continue;
		printf("%-10.10s %-14.14s %-15.15s %s",
			ut.ut_name,
			ut.ut_line,
			ut.ut_host,
			ctime(&ut.ut_time));
#else
		printf("%-10.10s %-7.7s %-14.14s %6u %s",
			ut.ut_user,
			ut.ut_id,
			ut.ut_line,
			ut.ut_pid,
			ctime(&ut.ut_time));
#endif
	}
	close(ufd);
	exit(0);
}							 /* end of main */

/* vi: set tabstop=4 shiftwidth=4: */
/* end of dutmp.c */
