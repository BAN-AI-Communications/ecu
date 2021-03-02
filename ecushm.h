#define SHM_REV	0x0EC0000EL	 /* high 16-bits unique, low=revision */
/*+-------------------------------------------------------------------------
	ecushm.h -- ecu shared data segment
	wht@wht.net
--------------------------------------------------------------------------*/
/*+:EDITS:*/
/*:04-26-2000-11:15-wht@bob-RELEASE 4.42 */
/*:01-08-2000-12:59-wht@menlo-add Lsockserve */
/*:01-24-1997-02:37-wht@yuriatin-SOURCE RELEASE 4.00 */
/*:09-11-1996-20:00-wht@yuriatin-3.48-major telnet,curses,structural overhaul */
/*:09-09-1996-03:16-wht@yuriatin-CFG_TelnetOption sizing */
/*:11-23-1995-11:20-wht@kepler-source control 3.37 for tsx-11 */
/*:11-14-1995-10:23-wht@kepler-3.37.80-source control point: SOCKETS */
/*:11-12-1995-00:18-wht@gyro-show_telnet_traffic now in shm */
/*:11-11-1995-23:57-wht@gyro-add rcvr_ansi_filter */
/*:11-03-1995-16:54-wht@wwtp1-use CFG_TelnetOption */
/*:10-20-1995-02:44-wht@kepler-length line logical, descr, rname strings */
/*:10-19-1995-01:01-wht@kepler-add Ltelnet */
/*:08-27-1995-06:31-wht@n4hgf-add Lrtscts_val */
/*:07-25-1994-00:39-wht@n4hgf-allow for predef of max lines and columns */
/*:05-04-1994-04:39-wht@n4hgf-ECU release 3.30 */
/*:09-10-1992-13:59-wht@n4hgf-ECU release 3.20 */
/*:09-10-1992-04:34-wht@n4hgf-add rcvrdisp semaphore */
/*:09-06-1992-13:29-wht@n4hgf-add receiver process buffered screen write */
/*:08-22-1992-15:38-wht@n4hgf-ECU release 3.20 BETA */
/*:08-17-1992-04:55-wht@n4hgf-keep rcvr pid in shm for friend code */
/*:07-19-1992-07:44-wht@n4hgf-85 lines too expensive to keep updated */
/*:07-19-1992-07:42-wht@n4hgf-ttyinit_param -> ttyuse */
/*:05-08-1992-03:36-wht@n4hgf-bumped rev: max screen geometry now 85x80 */
/*:03-27-1992-16:21-wht@n4hgf-re-include protection for all .h files */
/*:12-15-1991-14:22-wht@n4hgf-autorz and zmodem_asterisk_count added */
/*:12-13-1991-04:16-wht@n4hgf-move bell_notify_state to shm */
/*:11-11-1991-22:25-wht@n4hgf-add Ldcdwatch and Ltiobuf */
/*:08-21-1991-01:34-wht@n4hgf-FAR depends only on M_I286 */
/*:07-25-1991-12:56-wht@n4hgf-ECU release 3.10 */
/*:12-19-1990-17:09-wht@n4hgf-make cursor variables unsigned */
/*:11-30-1990-19:01-wht@n4hgf-add ttyinit_param */
/*:11-28-1990-17:43-wht@n4hgf-move cursor_y, cursor_x to right after revision */
/*:08-14-1990-20:40-wht@n4hgf-ecu3.00-flush old edit history */

#ifndef _ecushm_h
#define _ecushm_h

#if defined(M_I286)
#define FAR far
#else
#define FAR
#endif

#if !defined(UINT16)
#define UINT16 unsigned short
#endif
#if !defined(uchar)
#define uchar unsigned char
#endif
#if !defined(UINT)
#define UINT unsigned int
#endif
#if !defined(UINT32)
#define UINT32 unsigned long
#endif

/* tty usage parameter (shm->ttyuse) */
#define TTYUSE_NORMAL			0	/* must be zero */
#define TTYUSE_FORCE_SIMPLE		1

/*
 * max length of a hostname or telephone number
 */
/*
 * max length of a hostname or telephone number
 * string this the actual number of characters: arrays are
 * defined DESTREF_LEN + 1 in length to provide for null
 * (if telnet enabled, 10 more spaces for longest hostnames
 * we can manage in 80 col display)
 */
#ifdef CFG_TelnetOption
#define LOGICAL_LEN			50
#define DESTREF_LEN			50
#else
#define LOGICAL_LEN			40
#define DESTREF_LEN			40
#endif

#ifndef CFG_ScreenLinesMax
#define CFG_ScreenLinesMax	50
#endif

#ifndef CFG_ScreenColsMax
#define CFG_ScreenColsMax	80
#endif

#define SHM_STRLEN			256
#define TO_SCREEN_BUFSZ		128

typedef struct ecu_sds
{
	uchar screen[CFG_ScreenLinesMax][CFG_ScreenColsMax];
	UINT32 shm_revision;
	UINT cursor_y;			 /* program-maintained receive cursor */
	UINT cursor_x;			 /* program-maintained receive cursor */
	UINT16 scr_lines;		 /* lines in use */
	UINT16 scr_cols;		 /* columns in use */
	UINT16 scr_size;		 /* screen size (lines * cols) */
	UINT16 terminating;		 /* made one when ECU terminating */
	/* xmtr to rcvr communication area */
	int xcmd;				 /* signal from xmtr to rcvr SIGUSR2 */
	int xi1;
	int xi2;
	int xi3;
	char xs1[SHM_STRLEN];
	/* rcvr to xmtr communication area */
	int rcmd;				 /* signal from rcvr to xmtr SIGUSR2 */
	int ri1;
	int ri2;
	char rs1[SHM_STRLEN];
	UINT32 rcvd_chars;		 /* rcvr char count */
	UINT32 rcvd_chars_this_connect;	/* count since last connect */
	UINT32 xmit_chars;		 /* xmit char count */
	UINT32 xmit_chars_this_connect;	/* count since last connect */
	int Ladd_nl_incoming;	 /* when in ksr mode, add nl to cr on receive */
	int Ladd_nl_outgoing;	 /* when in ksr mode, add nl to cr on xmit */
	int Lfull_duplex;		 /* if non-zero, full duplex else half */
	int Liofd;				 /* file descriptor for line */
	int Lmodem_already_init; /* true if modem already initialized */
	int Lconnected;			 /* we try to keep accurate */
	int Lparity;			 /* 0==NONE, 'e' == even, 'o' == odd */
	UINT Lbitrate;			 /* bit rate */
	char Ldescr[70];		 /* description of remote */
	char Lline[64];			 /* /dev/ttyname for outgoing line */
	char Llogical[LOGICAL_LEN + 1];	/* logical name of remote (from dial
									 * dir) */
	char Lrname[70];		 /* logical name of remote (settable) */
	long Loff_hook_time;	 /* time() at connect */
	char Ltelno[DESTREF_LEN + 1];	/* telephone number for remote or null */
	UINT32 Lxonxoff;			 /* status of line IXON and IXOFF */
	uchar Ldcdwatch;		 /* state of line DCD watcher */
	uchar Ltelnet;			 /* true if connection is telnet, false if
							  * serial */
	uchar Lsockserve;		 /* if Ltelnet true: true if connection is 
							  * under sockserve, false if not */
#ifdef CFG_TelnetOption
	uchar Ltelnet_raw;		 /* true if telnet IAC + CR/0 suppression */
	char Lipaddr_str[32];	 /* "nnn.nnn.nnn.nnn:nnnnn" */
#endif
	uchar Lrtscts_val;		 /* RTS/CTS state */

	/*
	 * this is a projectile vomit hack, but you don't need termio.h to use
	 * ecushm.h this way
	 */
#define TIOBUF_SIZE		40	 /* big enough for all systems I know about */
	long Ltiobuf[(TIOBUF_SIZE / sizeof(long)) + 1];	/* buffer for termio */
	CFG_PidType xmtr_pid;	 /* transmitter process pid */
	CFG_PidType xmtr_ppid;	 /* transmitter process' parent's pid */
	CFG_PidType xmtr_pgrp;	 /* transmitter process group */
	CFG_PidType rcvr_pid;	 /* receiver pid (or -1 if inactive) */
	char tty_name[64];		 /* comm line name (not console) */
	uchar ttyuse;			 /* see TTYUSE_... above and ecuxfer.c */
	uchar bell_notify_state; /* bell/text-event to annunciator mapping
							  * state */
	uchar autorz;			 /* if true, automatic rz on rcvd zmodem
							  * prefix */
	uchar autorz_pos;		 /* position in autorz match sequence */
	uchar rcvr_ansi_filter;	 /* true if ANSI filter desired */
	uchar show_telnet_traffic;	/* if true, show telnet options traffic */
	uchar telnet_ttype[64];	 /* transmitted telnet term name */

	/*
	 * receiver process buffered screen write
	 */
	char rcvrdisp_buffer[TO_SCREEN_BUFSZ];
	char *rcvrdisp_ptr;
	int rcvrdisp_count;
	int rcvrdisp_semid;

	UINT32 friend_space[128]; /* space for friend programs */
}
ECU_SDS;

extern ECU_SDS FAR *shm;	 /* shared segment pointer */

#endif /* _ecushm_h */

/* vi: set tabstop=4 shiftwidth=4: */
/* end of ecushm.h */
