
/* CHK=0x3B2F */
/*+-------------------------------------------------------------------------
	testmorse.c
--------------------------------------------------------------------------*/
/*+:EDITS:*/
/*:01-24-1997-02:38-wht@yuriatin-SOURCE RELEASE 4.00 */
/*:11-08-1990-13:52-wht@n4hgf-convert hack to decent test */
/*:11-06-1988-21:17-wht-creation */

#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/errno.h>
#include "morse_dvr.h"

/*+-------------------------------------------------------------------------
	send_morse(fd,str)
--------------------------------------------------------------------------*/
void
send_morse(fd, str)
int fd;
char *str;
{
	while (*str)
	{
		write(1, str, 1);
		write(fd, str++, 1);
	}
}							 /* end of send_morse */

/*+-------------------------------------------------------------------------
	main(argc,argv,envp)
--------------------------------------------------------------------------*/
main(argc, argv, envp)
int argc;
char **argv;
char **envp;
{
	int fdm = open("/dev/morse", O_WRONLY, 0);
	int dummy = 0;
	int ticks;
	int freq;

	setbuf(stdout, NULL);
	setbuf(stderr, NULL);

	if (fdm < 0)
	{
		perror("/dev/morse open error");
		exit(1);
	}

	if (ioctl(fdm, MORSE_ENABLE, &dummy))
	{
		fprintf(stderr,
			"I think you have an old version of the driver installed.\n");
	}

	for (freq = 400; freq < 800; freq += 400)
	{
		printf("frequency = %d ioctl status = %d\n",
			freq, ioctl(fdm, MORSE_SET_FREQUENCY, &freq));
		for (ticks = 2; ticks < 9; ticks++)
		{
			printf("ticks = %d ioctl status = %d:       ",
				ticks, ioctl(fdm, MORSE_SET_SPEED, &ticks));
			send_morse(fdm, "test ");
			fputs("\n", stdout);
		}
		fputs("\n", stdout);
	}
	for (freq = 800; freq < 3300; freq += 200)
	{
		printf("frequency = %d ioctl status = %d\n",
			freq, ioctl(fdm, MORSE_SET_FREQUENCY, &freq));
		ticks = 3;
		printf("ticks = %d ioctl status = %d:       ",
			ticks, ioctl(fdm, MORSE_SET_SPEED, &ticks));
		send_morse(fdm, "test ");
		fputs("\n", stdout);
	}
	fputs("\n", stdout);
	exit(0);
}							 /* end of main */

/* end of user-morse.c */
/* vi: set tabstop=4 shiftwidth=4: */
