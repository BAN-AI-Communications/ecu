/*+-------------------------------------------------------------------------
	termecu.h -- termecu (exit()) codes
	wht@wht.net

  1 - 64    reserved for signals
  193 - 223 reserved for procedure 'exit' codes
--------------------------------------------------------------------------*/
/*+:EDITS:*/
/*:04-26-2000-11:16-wht@bob-RELEASE 4.42 */
/*:01-24-1997-02:38-wht@yuriatin-SOURCE RELEASE 4.00 */
/*:09-11-1996-20:01-wht@yuriatin-3.48-major telnet,curses,structural overhaul */
/*:11-23-1995-11:20-wht@kepler-source control 3.37 for tsx-11 */
/*:11-14-1995-10:23-wht@kepler-3.37.80-source control point: SOCKETS */
/*:05-04-1994-04:40-wht@n4hgf-ECU release 3.30 */
/*:12-20-1992-12:02-wht@n4hgf-add TERMECU_SVC_NOT_AVAIL */
/*:09-16-1992-03:29-wht@n4hgf-add TERMECU_UNRECOVERABLE */
/*:09-10-1992-14:00-wht@n4hgf-ECU release 3.20 */
/*:08-22-1992-15:39-wht@n4hgf-ECU release 3.20 BETA */
/*:03-27-1992-16:21-wht@n4hgf-re-include protection for all .h files */
/*:07-25-1991-12:59-wht@n4hgf-ECU release 3.10 */
/*:08-14-1990-20:40-wht@n4hgf-ecu3.00-flush old edit history */

#ifndef _termecu_h
#define _termecu_h

#define TERMECU_OK					0
#define TERMECU_SIG1				1
#define TERMECU_SIGN				64
#define TERMECU_LINE_READ_ERROR		129
#define TERMECU_XMTR_WRITE_ERROR	130
#define TERMECU_XMTR_FATAL_ERROR	131
#define TERMECU_BSD4_IOCTL			132
#define TERMECU_SHM_ABL				133
#define TERMECU_SHM_RTL				134
#define TERMECU_NO_FORK_FOR_RCVR	135
#define TERMECU_TTYIN_READ_ERROR	136
#define TERMECU_LINE_OPEN_ERROR		137
#define TERMECU_PWENT_ERROR			138
#define TERMECU_USAGE				139
#define TERMECU_CONFIG_ERROR		140
#define TERMECU_CURSES_ERROR		141
#define TERMECU_RCVR_FATAL_ERROR	142
#define TERMECU_MALLOC				143
#define TERMECU_LOGIC_ERROR			144
#define TERMECU_GEOMETRY			145
#define TERMECU_IPC_ERROR			146
#define TERMECU_UNRECOVERABLE		147
#define TERMECU_SVC_NOT_AVAIL		148

#define TERMECU_INIT_PROC_ERROR		192

#define TERMECU_USER1				193
#define TERMECU_USERN				223

#endif /* _termecu_h */

/* vi: set tabstop=4 shiftwidth=4: */
/* end of termecu.h */
