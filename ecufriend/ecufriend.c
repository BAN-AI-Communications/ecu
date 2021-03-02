#define DUMP_SCREEN
/*+-------------------------------------------------------------------------
	ecufriend.c -- example of using ecu shared memory access
	wht@wht.net

Execute by ecu procedure command:  system -s 'ecufriend '+%itos(%shmid)
To manually execute for perusal, do an ipcs command to find the
shared memory id or HOME pc echo 'shmid='+%itos(%shmid)
Then HOME >ecufriend # where # is replaced by the shmid
--------------------------------------------------------------------------*/
/*+:EDITS:*/
/*:01-24-1997-02:37-wht@yuriatin-SOURCE RELEASE 4.00 */
/*:09-11-1996-20:00-wht@yuriatin-3.48-major telnet,curses,structural overhaul */
/*:11-23-1995-11:20-wht@kepler-source control 3.37 for tsx-11 */
/*:11-14-1995-10:23-wht@kepler-3.37.80-source control point: SOCKETS */
/*:05-04-1994-04:38-wht@n4hgf-ECU release 3.30 */
/*:09-10-1992-13:58-wht@n4hgf-ECU release 3.20 */
/*:08-22-1992-15:38-wht@n4hgf-ECU release 3.20 BETA */
/*:07-25-1991-12:56-wht@n4hgf-ECU release 3.10 */
/*:08-14-1990-20:40-wht@n4hgf-ecu3.00-flush old edit history */

#include <stdio.h>
#include <signal.h>
#include <termio.h>
#include <sys/errno.h>
#include "../ecu_types.h"
#include "../ecu_stat.h"
#include "../ecu_time.h"
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include "../ecushm.h"

ECU_SDS FAR *shm;			 /* shared segment pointer */
int shmid;					 /* sharged segment id */

/*+-------------------------------------------------------------------------
	main(argc,argv,envp)
--------------------------------------------------------------------------*/
main(argc, argv, envp)
int argc;
char **argv;
char **envp;
{
	register itmp;

#ifdef DUMP_SCREEN
	int row, col;
	unsigned char sch;

#endif

	if (argc < 2)
	{
		printf("usage: ecufriend <ecu-shmid>\n");
		exit(255);
	}
	shmid = atoi(argv[1]);

	if ((shm = (ECU_SDS FAR *) shmat(shmid, (char FAR *)0, SHM_RDONLY))
		== (ECU_SDS FAR *) - 1)
	{
		perror("shmat");
		exit(1);
	}

	printf("ecu shm address = %08lx shmid=%d revision=%08lx\n",
		shm, shmid, shm->shm_revision);

	if (shm->shm_revision != SHM_REV)
	{
		printf("incompatible shared memory revision (compiled with %08lx)\n",
			SHM_REV);
		exit(1);
	}

	printf("xmit chars=%lu rcvd chars=%lu\n",
		shm->xmit_chars, shm->rcvd_chars);
	printf("The receive cursor position is at column %d row %d\n",
		shm->cursor_y, shm->cursor_x);

	printf("If this program was not executed by a shell that closed it,\n");
	printf("then fd %d is available to us to write and read from the line.\n",
		shm->Liofd);

	if (isatty(shm->Liofd))
		printf("It is available.\n");
	else
		printf("It is not available. Even so, we could re-open %s here\n",
			shm->Lline);

	if (shm->Lmodem_off_hook)
		printf("Hmmm.. we seem to be connected to %s at %s\n",
			shm->Ldescr, shm->Ltelno);

#ifdef DUMP_SCREEN
	printf("screen dump\n");
	printf("-----------\n");
	for (row = 0; row < 43; row++)
	{
		for (col = 0; col < 79; col++)
		{
			sch = shm->screen[row][col];
			if ((sch < 0x20) && (sch > 0x7E))
				putc('.', stdout);
			else
				putc(sch, stdout);
		}
		putc('\n', stdout);
	}
#endif

	exit(0);
}							 /* end of main */

/* vi: set tabstop=4 shiftwidth=4: */
/* end of ecufriend.c */
