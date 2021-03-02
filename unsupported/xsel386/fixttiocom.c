/*+-------------------------------------------------------------------------
	fixttiocom.c - change ttiocom to Ttiocom calls in /usr/sys/sys/libsys.a
This program patched my Xenix 386 2.3.1 system library (a copy of which
had been named libfix.a, fixed, verified, then moved to libsys.a)
--------------------------------------------------------------------------*/
/*+:EDITS:*/
/*:01-24-1997-02:38-wht@yuriatin-SOURCE RELEASE 4.00 */
/*:09-11-1996-20:01-wht@yuriatin-3.48-major telnet,curses,structural overhaul */
/*:11-23-1995-11:20-wht@kepler-source control 3.37 for tsx-11 */
/*:11-14-1995-10:23-wht@kepler-3.37.80-source control point: SOCKETS */
/*:05-04-1994-04:40-wht@n4hgf-ECU release 3.30 */
/*:09-10-1992-14:00-wht@n4hgf-ECU release 3.20 */
/*:08-22-1992-15:39-wht@n4hgf-ECU release 3.20 BETA */
/*:07-25-1991-12:59-wht@n4hgf-ECU release 3.10 */
/*:08-14-1990-20:40-wht@n4hgf-ecu3.00-flush old edit history */

#include <stdio.h>
#include <sys/types.h>
#include <fcntl.h>

char *lib = "libfix.a";

/* file positions where names needed changing */
#define PLACE1 0xdcaL
#define PLACE2 0x1b1cbL
#define PLACE3 0x1c41bL

/*+-------------------------------------------------------------------------
	main(argc,argv,envp)
--------------------------------------------------------------------------*/
main(argc, argv, envp)
int argc;
char **argv;
char **envp;
{
	int fd = open(lib, O_RDWR, 0);
	unsigned char ch1, ch2, ch3;
	long lseek();

	if (fd < 0)
	{
		perror(lib);
		exit(9);
	}

	if (lseek(fd, PLACE1, 0) != PLACE1)
	{
		perror("seek1");
		exit(1);
	}
	if (read(fd, &ch1, 1) != 1)
	{
		perror("read1");
		exit(1);
	}
	printf("char 1 = %02x\n", ch1);

	if (lseek(fd, PLACE2, 0) != PLACE2)
	{
		perror("seek2");
		exit(2);
	}
	if (read(fd, &ch2, 2) != 2)
	{
		perror("read2");
		exit(2);
	}
	printf("char 2 = %02x\n", ch2);

	if (lseek(fd, PLACE3, 0) != PLACE3)
	{
		perror("seek3");
		exit(3);
	}
	if (read(fd, &ch3, 1) != 1)
	{
		perror("read3");
		exit(3);
	}
	printf("char 3 = %02x\n", ch3);

	if ((ch1 != 't') || (ch2 != 't') || (ch3 != 't'))
		exit(8);
	if (lseek(fd, PLACE1, 0) != PLACE1)
	{
		perror("seek1");
		exit(1);
	}
	write(fd, "T", 1);
	if (lseek(fd, PLACE2, 0) != PLACE2)
	{
		perror("seek1");
		exit(1);
	}
	write(fd, "T", 1);
	if (lseek(fd, PLACE3, 0) != PLACE3)
	{
		perror("seek1");
		exit(1);
	}
	write(fd, "T", 1);
	printf("done\n");
	close(fd);

	exit(0);
}							 /* end of main */

/* vi: set tabstop=4 shiftwidth=4: */
/* end of fixttiocom.c */
