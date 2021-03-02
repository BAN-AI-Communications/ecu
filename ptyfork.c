/*+-------------------------------------------------------------------------
	ptyfork.c

 ptyfork # cmd args .....

 open a pty,  wire fd # so that we pass slave part
 of the pty as fd # to fork # cmd args .....

  Defined functions:

--------------------------------------------------------------------------*/
/*+:EDITS:*/
/*:04-26-2000-11:16-wht@bob-RELEASE 4.42 */
/*:04-06-1998-13:12-wht@gyro-creation */

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <memory.h>
#include <malloc.h>
#include <time.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <signal.h>
#include <termio.h>

/*+-------------------------------------------------------------------------
	usage()
--------------------------------------------------------------------------*/
void
usage()
{
	fprintf(stderr,"ptyfork fd cmd [args ...]\n");
	exit(1);
}	/* end of usage */

/*+-------------------------------------------------------------------------
	open_free_pty()
--------------------------------------------------------------------------*/
int
open_free_pty(master,slave,pmasterfd,pslavefd)
char *master;
char *slave;
int *pmasterfd;
int *pslavefd;
{
	int masterfd = -1;
	int slavefd = -1;
	char c1,c2;
	struct stat st;

	strcpy(master,"/dev/ptyp0");
	strcpy(slave,"/dev/ttyp0");

	printf("1st pty %s %s\n",master,slave);
	for(c1 = 's'; c1 >= 'p'; c1--)
	{
		master[8] = c1;
		master[9] = '0';
		if (!stat(master, &st))
		{
			slave[8] = c1;
			for(c2 = 0; c2 <= 15; c2++)
			{
				master[9] = (c2 < 0x0A) ? (c2 + '0') : (c2 - 10 + 'a');
				if (!stat(master, &st))
				{
					slave[9] = master[9];
					printf("pty try: %s %d %d\n",master,masterfd,slavefd);
					masterfd = open(master, O_RDWR);
					slavefd = open(slave, O_RDWR);
					printf("pty open: %s %d %d\n",master,masterfd,slavefd);
					if ((masterfd >= 0) && (slavefd >= 0))
					{
						*pmasterfd = masterfd;
						*pslavefd = slavefd;
						return(0);
					}
					close(masterfd);
					close(slavefd);
				}
			}
		}
	}
	return(-1);

}	/* end of open_free_pty */

/*+-------------------------------------------------------------------------
	aggressive_fork()
--------------------------------------------------------------------------*/
int
aggressive_fork()
{
	int count = 5;
	int pid;

	while (count--)
	{
		if ((pid = fork()) >= 0)
			return (pid);
	}
	return (-1);
}							 /* end of aggressive_fork */

/*+-------------------------------------------------------------------------
	main(argc,argv)
--------------------------------------------------------------------------*/
main(argc,argv)
int argc;
char **argv;
{
	int itmp;
	char m[128];
	char s[128];
	int masterfd,slavefd,targetfd,fdmax;
	char buf[512];
	int child = 0;
	struct timeval tval,*tv = &tval;
#ifdef CFG_HasFdSet
	CFG_FDSET fdset;
#else
	int fdset;
#endif

	if(argc < 3)
		usage();

	if((targetfd = atoi(argv[1])) <= 0)
		usage();

	if(open_free_pty(m,s,&masterfd,&slavefd))
	{
		printf("ptyfork: cannot open pty\n");
		exit(2);
	}

	if((child = aggressive_fork()) > 0)
	{
		/* parent */
		close(slavefd);
		fdmax = targetfd;
		if(fdmax < masterfd)
			fdmax = masterfd;
		while(1)
		{
#ifdef CFG_HasFdSet
			FD_ZERO(&fdset);
			FD_SET(targetfd, &fdset);
			FD_SET(masterfd, &fdset);
#else
			fdset = 1 << targetfd;
			fdset |= 1 << masterfd;
#endif
			tv->tv_sec = 10;
			tv->tv_usec = 0;
			if ((itmp = select(fdmax, &fdset, 0, 0, tv)) < 1)
			{
				if(itmp == 0)
					continue;
				break;
			}
			if(FD_ISSET(targetfd,&fdset))
			{
				errno = 0;
				if((itmp = read(targetfd,buf,sizeof(buf))) < 0)
					break;
				write(masterfd,buf,itmp);
			}
			if(FD_ISSET(masterfd,&fdset))
			{
				errno = 0;
				if((itmp = read(masterfd,buf,sizeof(buf))) < 0)
					break;
				write(targetfd,buf,itmp);
			}
		}
		kill(child,9);
		exit(1);
	}
	else if(!child)
	{
		/* child */
		close(targetfd);
		dup2(slavefd,targetfd);
		close(slavefd);
		close(masterfd);
		execvp(argv[2],argv + 2);
		perror("child exec failure");
		exit(255);
	}
	perror("fork");
	exit(1);
}	/* end of main */

