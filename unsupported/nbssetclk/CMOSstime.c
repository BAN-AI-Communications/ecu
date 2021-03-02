/* vi: set tabstop=4 shiftwidth=4: */
/*+-------------------------------------------------------------------------
	CMOSstime.c -- read cmos clock then stime()
	...!gatech!kd4nc!n4hgf!wht

  Defined functions:
	main(argc,argv,envp)

--------------------------------------------------------------------------*/
/*+:EDITS:*/
/*:01-24-1997-02:38-wht@yuriatin-SOURCE RELEASE 4.00 */
/*:01-28-1990-04:55-wht-working on cmostime version 4 */

#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <time.h>

/*+-------------------------------------------------------------------------
	main(argc,argv,envp)
--------------------------------------------------------------------------*/
main(argc,argv,envp)
int argc;
char **argv;
char **envp;
{
	int year,month,day,hour,min,sec;
	long now;

	setbuf(stdout,NULL);
	setbuf(stderr,NULL);

	tzset();
	time(&now);
	fputs("System time before stime(): ",stdout);
	fputs(ctime(&now),stdout);
	get_clock(&year,&month,&day,&hour,&min,&sec);
	now = date_to_epochsecs(year,month,day,hour,min,sec);
	if(stime(&now))
	{
		perror("stime");
		exit(1);
	}
	time(&now);
	fputs("System time after  stime(): ",stdout);
	fputs(ctime(&now),stdout);

	exit(0);
}	/* end of main */
