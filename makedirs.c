/*+-------------------------------------------------------------------------
	makedirs.c
--------------------------------------------------------------------------*/
/*+:EDITS:*/
/*:04-26-2000-11:16-wht@bob-RELEASE 4.42 */
/*:12-18-1997-16:18-wht@kepler-nap chged to sleep for portability */
/*:01-24-1997-02:37-wht@yuriatin-SOURCE RELEASE 4.00 */
/*:09-11-1996-20:00-wht@yuriatin-3.48-major telnet,curses,structural overhaul */
/*:11-23-1995-11:20-wht@kepler-source control 3.37 for tsx-11 */
/*:11-14-1995-10:23-wht@kepler-3.37.80-source control point: SOCKETS */
/*:05-04-1994-04:39-wht@n4hgf-ECU release 3.30 */
/*:09-10-1992-13:59-wht@n4hgf-ECU release 3.20 */
/*:08-22-1992-15:39-wht@n4hgf-ECU release 3.20 BETA */
/*:08-30-1991-00:37-wht@n4hgf2-force umask to 022 for installation */
/*:08-23-1991-14:38-wht@n4hgf-was not making last subdirectory in list */
/*:08-09-1991-02:13-root@n4hgf-need smart_fork for XENIX */
/*:07-25-1991-12:58-wht@n4hgf-ECU release 3.10 */
/*:07-15-1991-14:24-wht@n4hgf-creation */

#include <stdio.h>
#include <sys/errno.h>
#include "ecu_types.h"
#include "ecu_stat.h"

extern int errno;

/*+-------------------------------------------------------------------------
	main(argc,argv)
--------------------------------------------------------------------------*/
void
main(argc, argv)
int argc;
char **argv;
{
	int itmp;
	int errflg = 0;
	int dmode = 0755;
	char *dname;
	struct stat stat_buf, *st = &stat_buf;
	char s512[512];
	extern char *optarg;
	extern int optind;

	umask(022);

	while ((itmp = getopt(argc, argv, "m:")) != -1)
	{
		switch (itmp)
		{
			case 'm':
				sscanf(optarg, "%o", &dmode);
				dmode &= 0777;
				if (!dmode)
					dmode = 0755;
		}
	}
	if (errflg || (optind == argc))
	{
		(void)fprintf(stderr, "usage: makedirs [-m mode] dir ...\n");
		exit(1);
	}

	for (; optind < argc; optind++)
	{
		if (!stat(dname = argv[optind], st))
		{
			if ((st->st_mode & S_IFMT) != S_IFDIR)
			{
				fprintf(stderr, "%s exists and is not a directory\n", dname);
				exit(1);
			}
			chmod(dname, (unsigned short)dmode);
		}
		else
		{
			strcpy(s512, dname);
			strcat(s512, "/dummy");
			errno = ENOENT;	 /* fake make_dirs() into always trying */
			if (!make_dirs(s512, dmode))
			{
				perror(dname);
				exit(1);
			}
			else
				printf("Made directory %s\n", dname);
		}
	}
	exit(0);
}							 /* end of main */

/*+-------------------------------------------------------------------------
	smart_fork() - needed for mkdirs.c under XENIX
--------------------------------------------------------------------------*/
#if defined(M_XENIX)
int
smart_fork()
{
	int count = 5;
	int pid;

	while (count--)
	{
		if ((pid = fork()) >= 0)
			return (pid);
		if (count)
			nap(40L);
	}
	return (-1);
}							 /* end of smart_fork */
#endif

/* vi: set tabstop=4 shiftwidth=4: */
/* end of makedirs.c */
