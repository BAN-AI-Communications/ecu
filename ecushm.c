/*+-------------------------------------------------------------------------
	ecushm.c - shared memory handler for ecu xmtr/rcvr comm
	wht@wht.net

  Signal handler purists will surely get aneurisms from looking
  at what we do in here, but any OS that doesn't properly push
  a stack frame for a signal event just won't play here.  We even
  fork() in a signal handler.  That ought to make some of the
  brethren gag.

  Defined functions:
	shm_done()
	shm_init()
	shmr_notify_termecu()
	shmr_notify_xmtr_of_DCD_loss()
	shmr_notify_xmtr_of_telnet_close()
	shmr_notify_zmodem_frame()
	shmr_process_rcvr_SIGUSR2()
	shmx_make_rcvr_sleep(seconds)
	shmx_process_xmtr_SIGUSR2()
	shmx_set_rcvr_log(logfilename, append_flag, raw_flag, flush_each)
	shmx_unpause_rcvr()

  Sforzando (It., sfohr-tsahn'-doh).  A direction to perform the
  tone or chord with special stress, or marked and sudden emphasis.
  -- Schirmer Pronouncing POCKET-MANUAL of Musical Terms, 1936.

--------------------------------------------------------------------------*/
/*+:EDITS:*/
/*:04-26-2000-11:15-wht@bob-RELEASE 4.42 */
/*:01-24-1997-02:37-wht@yuriatin-SOURCE RELEASE 4.00 */
/*:09-28-1996-23:57-wht@yuriatin-SHMR_NOTIFY_TELNET */
/*:09-28-1996-23:52-wht@yuriatin-add shmr_notify_xmtr_of_telnet_close */
/*:09-11-1996-20:00-wht@yuriatin-3.48-major telnet,curses,structural overhaul */
/*:12-06-1995-13:30-wht@n4hgf-termecu w/errno -1 consideration */
/*:11-23-1995-11:20-wht@kepler-source control 3.37 for tsx-11 */
/*:11-14-1995-10:23-wht@kepler-3.37.80-source control point: SOCKETS */
/*:11-12-1995-01:57-wht@gyro-eliminate peurile shmx_connect */
/*:11-12-1995-01:49-wht@gyro-add SHMR_NOTIFY_TERMECU code */
/*:11-12-1995-00:33-wht@gyro-init new rcvr_ansi_filter */
/*:10-14-1995-17:08-wht@kepler-'struct termio' to 'struct TERMIO' */
/*:08-27-1995-06:35-wht@n4hgf-init shm->Lrtscts_val */
/*:05-04-1994-04:39-wht@n4hgf-ECU release 3.30 */
/*:11-25-1993-14:43-wht@n4hgf-3.281-fix typo in shm_done shmctl statement */
/*:09-10-1992-13:58-wht@n4hgf-ECU release 3.20 */
/*:09-10-1992-04:34-wht@n4hgf-add rcvrdisp semaphore */
/*:08-22-1992-15:38-wht@n4hgf-ECU release 3.20 BETA */
/*:12-15-1991-14:22-wht@n4hgf-autorz initialized */
/*:12-13-1991-04:16-wht@n4hgf-move bell_notify_state to shm */
/*:11-12-1991-18:02-wht@n4hgf-remove obsolete shmx_rc_report */
/*:11-11-1991-14:59-wht@n4hgf-shmr_notify_xmtr_of_DCD_loss */
/*:07-25-1991-12:56-wht@n4hgf-ECU release 3.10 */
/*:08-14-1990-20:40-wht@n4hgf-ecu3.00-flush old edit history */

#include "ecu.h"

#if defined(CFG_MmapSHM)
#include <sys/mman.h>
#else
#include <sys/ipc.h>
#include <sys/shm.h>
#ifdef CFG_SemWithShm
#include <sys/sem.h>

union semun
{
	int val;
	struct semid_ds *buf;
	UINT16 *array;
} semctl_arg;

#endif /* CFG_SemWithShm */
#endif /* CFG_MmapSHM */

/* xmtr to rcvr cmds */
#define SHMX_MAKE_RCVR_SLEEP	1	/* sleep xi1=#seconds */
#define SHMX_UNPAUSE			2	/* no-op to un-pause() rcvr */
#define SHMX_SET_RCVR_LOG		3	/* log file manipulations
									 * xi1=append,xi2=raw,xs1=n ame */

/* rcvr to xmtr cmds */
#define SHMR_NOTIFY_DCD_LOSS	1	/* rcvr detected DCD loss */
#define SHMR_NOTIFY_ZMODEM		2	/* rcvr detected ZMODEM frame */
#define SHMR_NOTIFY_TERMECU		3	/* rcvr go bye-bye */
#define SHMR_NOTIFY_TELNET		4	/* rcvr detected telnet close */

extern char rcvr_log_file[]; /* if rcvr_log!= 0,log filename */
extern int rcvr_log;		 /* rcvr log active if != 0 */
extern FILE *rcvr_log_fp;	 /* rcvr log file */
extern int rcvr_log_raw;	 /* if true, log all, else filter ctl chrs */
extern int rcvr_log_flusheach;
extern int rcvr_log_append;	 /* if true, append, else scratch */
extern int rcvr_log_gen_title;

ECU_SDS FAR *shm;			 /* shared segment pointer */
#if !defined(CFG_MmapSHM)
key_t shm_key;
int shm_shmid;

#endif

/*+-------------------------------------------------------------------------
	shm_init()
  Called by parent process (xmtr) to initialize environment
--------------------------------------------------------------------------*/
void
shm_init()
{
	UINT save_LINESxCOLS;
	extern UINT LINESxCOLS;

	/*
	 * see the comments in ecu.h titled "Communication line variables" for
	 * an explanation of the following
	 */
	if (sizeof(shm->Ltiobuf) < sizeof(struct TERMIO))
	{
		pprintf("ecushm.h LTIOBUF_SIZE needs to be increased to %d.\n",
			sizeof(struct TERMIO));

		pputs("Please change it, remake and notify wht@wht.net.\n");
		errno = -1;
		termecu(TERMECU_CONFIG_ERROR);
	}

#if defined(CFG_MmapSHM)
	shm = (ECU_SDS *) mmap(0, sizeof(ECU_SDS), PROT_READ | PROT_WRITE,
		MAP_ANON | MAP_INHERIT | MAP_SHARED, -1, 0);
	if (shm == (ECU_SDS *) - 1)
	{
		perror("mmap");
		termecu(TERMECU_IPC_ERROR);
	}
#else
	shm_key = 0xEC000000L | getpid();

	if ((shm_shmid = shmget(shm_key, sizeof(ECU_SDS), IPC_CREAT | 0600)) < 0)
	{
		perror("shmget");
		termecu(TERMECU_IPC_ERROR);
	}

	if ((shm = (ECU_SDS FAR *) shmat(shm_shmid, (char FAR *)0, 0)) ==
		(ECU_SDS FAR *) - 1)
	{
		perror("shmat");
		termecu(TERMECU_IPC_ERROR);
	}

#ifdef CFG_SemWithShm
	if ((shm->rcvrdisp_semid = semget(shm_key, 1, IPC_CREAT | 0600)) < 0)
	{
		perror("semget");
		termecu(TERMECU_IPC_ERROR);
	}
	rcvrdisp_v();			 /* first unlock */
#endif /* CFG_SemWithShm */
#endif /* CFG_MmapSHM */

	/*
	 * see the comments in ecu.h titled "Communication line variables" for
	 * an explanation of the following
	 */
	Ltermio = (struct TERMIO *)shm->Ltiobuf;	/* cover termio buffer */

	shm->xcmd = 0;			 /* signal from xmtr to rcvr SIGUSR2 */
	shm->xi1 = 0;			 /* parameters */
	shm->xi2 = 0;
	shm->xs1[0] = 0;
	shm->rcmd = 0;			 /* signal from rcvr to xmtr SIGUSR2 */
	shm->ri1 = 0;			 /* parameters */
	shm->ri2 = 0;
	shm->rs1[0] = 0;
	shm->rcvd_chars = 0L;
	shm->rcvd_chars_this_connect = 0L;
	shm->bell_notify_state = 1;	/* default to want bell notify */
	shm->Ldcdwatch = 0;		 /* default DCD watcher to off */
	shm->Lrtscts_val = 0;	 /* default no hw flow control */
	shm->autorz = 1;		 /* default automatic rz to on */
	shm->cursor_y = 0;
	shm->cursor_x = 0;
	shm->friend_space[0] = 0;
	save_LINESxCOLS = LINESxCOLS;
	LINESxCOLS = sizeof(shm->screen);	/* avoid trap */
	spaces((char *)shm->screen, sizeof(shm->screen));
	LINESxCOLS = save_LINESxCOLS;
	shm->rcvrdisp_ptr = shm->rcvrdisp_buffer;
	shm->rcvrdisp_count = 0;
#if defined(CFG_NoAnsiEmulation)
	shm->rcvr_ansi_filter = 0;
#else
	shm->rcvr_ansi_filter = 1;
#endif

}							 /* end of shm_init */

/*+-------------------------------------------------------------------------
	shm_done() -- finished with shm/sem -- clean up

We might make a trip thru here with the xmtr and rcvr, so
ignore EINVAL
--------------------------------------------------------------------------*/
void
shm_done()
{

#if defined(CFG_MmapSHM)
	munmap(shm, sizeof(ECU_SDS));
#else
#ifdef CFG_SemWithShm
	union semun semctl_arg;

	if (semctl(shm->rcvrdisp_semid, 0, IPC_RMID, &semctl_arg) &&
		(errno != EINVAL))
	{
		pperror("semctl IPC_RMID");
	}
#endif /* CFG_SemWithShm */

	if (shmctl(shm_shmid, IPC_RMID, (struct shmid_ds *)0) && (errno != EINVAL))
		pperror("shmctl IPC_RMID");
#endif

}							 /* end of shm_done */

/*+-------------------------------------------------------------------------
	shmx_make_rcvr_sleep(seconds)
--------------------------------------------------------------------------*/
void
shmx_make_rcvr_sleep(seconds)
int seconds;
{
	shm->xcmd = SHMX_MAKE_RCVR_SLEEP;
	shm->xi1 = seconds;
	kill_rcvr_process(SIGUSR2);

}							 /* end of shmx_make_rcvr_sleep */

/*+-------------------------------------------------------------------------
	shmx_unpause_rcvr() - no-op SIGUSR2 to unpause receiver
--------------------------------------------------------------------------*/
void
shmx_unpause_rcvr()
{
	shm->xcmd = SHMX_UNPAUSE;
	kill_rcvr_process(SIGUSR2);
}							 /* end of shmx_unpause_rcvr */

/*+-------------------------------------------------------------------------
	shmx_set_rcvr_log(logfilename,append_flag,raw_flag,flush_each)

null logfilename stops logging
append_flag says whether to open for write or append
raw_flag says whether or not to filter non-printable chars or not
(NL not filtered)
--------------------------------------------------------------------------*/
void
shmx_set_rcvr_log(logfilename, append_flag, raw_flag, flush_each)
char *logfilename;
int append_flag;
int raw_flag;
int flush_each;
{
	shm->xcmd = SHMX_SET_RCVR_LOG;
	shm->xi1 = append_flag;
	shm->xi2 = raw_flag;
	shm->xi3 = flush_each;
	strcpy(shm->xs1, logfilename);
	kill_rcvr_process(SIGUSR2);
}							 /* end of shmx_set_rcvr_log */

/*+-------------------------------------------------------------------------
	shmr_notify_xmtr_of_telnet_close()
--------------------------------------------------------------------------*/
void
shmr_notify_xmtr_of_telnet_close()
{
	shm->rcmd = SHMR_NOTIFY_TELNET;
	kill(shm->xmtr_pid, SIGUSR2);
	while (1)
		pause();			 /* wait for xmtr to kill */
}							 /* end of shmr_notify_xmtr_of_telnet_close */

/*+-------------------------------------------------------------------------
	shmr_notify_xmtr_of_DCD_loss()
--------------------------------------------------------------------------*/
void
shmr_notify_xmtr_of_DCD_loss()
{
	shm->rcmd = SHMR_NOTIFY_DCD_LOSS;
	kill(shm->xmtr_pid, SIGUSR2);
}							 /* end of shmr_notify_xmtr_of_DCD_loss */

/*+-------------------------------------------------------------------------
	shmr_notify_zmodem_frame()
--------------------------------------------------------------------------*/
void
shmr_notify_zmodem_frame()
{
	shm->rcmd = SHMR_NOTIFY_ZMODEM;	/* rcvr detected ZMODEM frame */
	kill(shm->xmtr_pid, SIGUSR2);
}							 /* end of shmr_notify_zmodem_frame */

/*+-------------------------------------------------------------------------
	shmr_notify_termecu()
--------------------------------------------------------------------------*/
void
shmr_notify_termecu()
{
	shm->rcmd = SHMR_NOTIFY_TERMECU;	/* rcvr go bye-bye */
	kill(shm->xmtr_pid, SIGUSR2);
}							 /* end of shmr_notify_termecu */

/*+-------------------------------------------------------------------------
	shmx_process_xmtr_SIGUSR2()
--------------------------------------------------------------------------*/
void
shmx_process_xmtr_SIGUSR2()
{
	int rcmd;
	int ri1, ri2;
	int argc;
	char *cp;
	char **argv;
	UINT32 colors_save;
	int lchar;
	int success_flag;

	rcmd = shm->rcmd;
	shm->rcmd = 0;
	ri1 = shm->ri1;
	ri2 = shm->ri2;

	switch (rcmd)
	{
		case SHMR_NOTIFY_DCD_LOSS:
			lzero_length_read_detected();
			break;

		case SHMR_NOTIFY_TELNET:
			{
				extern UINT32 colors_current;
				UINT32 colors_at_entry = colors_current;

				kill_rcvr_process(SIGKILL);
				fputs("\r\n", se);
				setcolor(colors_notify);
				fputs("[session terminated by remote host]", se);
				setcolor(colors_at_entry);
				fputs("\r\n", se);
				DCE_now_on_hook();	/* does a lclose */
			}
			break;

		case SHMR_NOTIFY_ZMODEM:
			kill_rcvr_process(SIGUSR1);
			success_flag = 0;
			while ((lchar = lgetc_timeout(100L)) >= 0)
			{
				fputc(lchar, se);
				if (lchar == '\n')
				{
					success_flag = 1;
					break;
				}
			}
			if (!success_flag)
				fputs("\r\n", se);
			colors_save = colors_current;
			setcolor(colors_notify);
			fputs("[automatic rz]", se);
			setcolor(colors_save);
			fputs("\r\n", se);
			argc = 1;
			cp = "rz";
			argv = &cp;
			receive_files_from_remote(argc, argv);
			start_rcvr_process(1);
			break;
		case SHMR_NOTIFY_TERMECU:
			termecu(TERMECU_RCVR_FATAL_ERROR);
			break;
	}

}							 /* end of shmx_process_xmtr_SIGUSR2 */

/*+-------------------------------------------------------------------------
	shmr_process_rcvr_SIGUSR2()
--------------------------------------------------------------------------*/
void
shmr_process_rcvr_SIGUSR2()
{
	int xcmd;
	int xi1, xi2, xi3;
	char xs1[SHM_STRLEN];

	xcmd = shm->xcmd;
	shm->xcmd = 0;
	xi1 = shm->xi1;
	xi2 = shm->xi2;
	xi3 = shm->xi3;
	strcpy(xs1, shm->xs1);

	switch (xcmd)
	{
		case SHMX_MAKE_RCVR_SLEEP:
			sleep(xi1);
			break;

		case SHMX_SET_RCVR_LOG:
			if (rcvr_log)	 /* if already logging */
			{
				if (!rcvr_log_raw)
					LOGPUTC('\n', rcvr_log_fp);
				fclose(rcvr_log_fp);
				rcvr_log = 0;
			}
			if (strlen(xs1) == 0)	/* if all we wanted was to stop log
									 * ... */
				break;		 /* ... then quit */
			rcvr_log_gen_title = 1;
			rcvr_log = 1;
			rcvr_log_append = xi1;
			rcvr_log_raw = xi2;
			rcvr_log_flusheach = xi3;
			strcpy(rcvr_log_file, xs1);
			rcvr_log_open();
			break;

		case SHMX_UNPAUSE:
			break;
	}

}							 /* end of shmr_process_rcvr_SIGUSR2 */

/* end of ecushm.c */
/* vi: set tabstop=4 shiftwidth=4: */
