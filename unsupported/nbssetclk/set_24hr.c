/*+-------------------------------------------------------------------------
	set_24hr.c
	...!gatech!kd4nc!n4hgf!wht
--------------------------------------------------------------------------*/
/*+:EDITS:*/
/*:01-24-1997-02:38-wht@yuriatin-SOURCE RELEASE 4.00 */
/*:01-28-1990-04:55-wht-working on cmostime version 4 */

#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>

#include "dev_cmos.h"

long lseek();

char	*cmos = "/dev/cmos";

/*+-------------------------------------------------------------------------
	main(argc,argv,envp)
--------------------------------------------------------------------------*/
main(argc,argv,envp)
int argc;
char **argv;
char **envp;
{
int fdcmos;
AT_CMOS *base;
long srB_pos = (long)((char *)&base->srB - (char *)base);
uchar srB;

	setbuf(stdout,NULL);
	setbuf(stderr,NULL);

	if((fdcmos = open(cmos,O_RDWR,0)) < 0)
	{
		perror("/dev/cmos open");
		exit(1);
	}

	if(lseek(fdcmos,srB_pos,0) != srB_pos)
	{
		perror("/dev/cmos seek 1");
		exit(1);
	}

	if(read(fdcmos,&srB,1) != 1)
	{
		perror("/dev/cmos read");
		exit(1);
	}

	srB |= SRB_24HR;

	if(lseek(fdcmos,srB_pos,0) != srB_pos)
	{
		perror("/dev/cmos seek 2");
		exit(1);
	}

	if(write(fdcmos,&srB,1) != 1)
	{
		perror("/dev/cmos write");
		exit(1);
	}

	system("cmos_disp");
	exit(0);
}	/* end of main */
