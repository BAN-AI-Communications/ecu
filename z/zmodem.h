/*+-------------------------------------------------------------------------
	zmodem.h -- common include filefor ecurz/ecusz
--------------------------------------------------------------------------*/
/*+:EDITS:*/
/*:01-24-1997-02:38-wht@yuriatin-SOURCE RELEASE 4.00 */
/*:09-11-1996-20:01-wht@yuriatin-3.48-major telnet,curses,structural overhaul */
/*:11-14-1995-10:23-wht@kepler-3.37.80-source control point: SOCKETS */
/*:05-09-1995-17:08-wht@kepler-no more use of VOLATILE */
/*:03-12-1995-02:55-wht@kepler-Linux clean-up */
/*:05-04-1994-04:40-wht@n4hgf-ECU release 3.30 */
/*:01-25-1994-17:02-wht@n4hgf-robertl corrections + mv extern crctab to hdr */
/*:01-30-1993-11:53-wht@n4hgf-test __STDC==1 for volatile */
/*:09-10-1992-14:00-wht@n4hgf-ECU release 3.20 */
/*:08-22-1992-15:39-wht@n4hgf-ECU release 3.20 BETA */
/*:08-28-1991-14:08-wht@n4hgf2-SVR4 cleanup by aega84!lh */
/*:08-21-1991-06:23-wht@n4hgf-sun porting */
/*:07-25-1991-12:59-wht@n4hgf-ECU release 3.10 */
/*:08-14-1990-20:41-wht@n4hgf-ecu3.00-flush old edit history */

#if defined(__GNUC__) || defined(GNUC) || (__STDC__==1)
#define VOLATILE volatile
#else
#define VOLATILE
#endif

#include "../ecu_types.h"
#include "../ecu_stat.h"
#include "../ecutermio.h"
#include <string.h>
#define MODE2OK

#if defined(M_UNIX)
#undef M_XENIX
#endif

#define ACK 6
#define CAN ('X'&037)
#define CPMEOF 032
#define ENQ 005
#define EOT 4
#define ERROR (-1)
#define ERRORMAX 5
#define FALSE 0
#define NAK 025
#define OK 0
#define PATHLEN 257			 /* ready for 4.2 bsd ? */
#define RCDO (-3)
#define SOH 1
#define STX 2
#define TIMEOUT (-2)
#define TRUE 1
#define UNIXFILE 0xF000		 /* The S_IFMT file mask bit for stat */
#define WANTCRC 0103		 /* send C not NAK to get crc not checksum */

#define WANTG 0107			 /* Send G not NAK to get nonstop batch xmsn */
#define WCEOT (-10)
#define XOFF ('s'&037)
#define XON ('q'&037)

/*
 * updcrc macro derived from article Copyright (C) 1986 Stephen Satchell.
 *  NOTE: First argument must be in range 0 to 255.
 *        Second argument is referenced twice.
 *
 * Programmers may incorporate any or all code into their programs,
 * giving proper credit within the source. Publication of the
 * source routines is permitted so long as proper credit is given
 * to Stephen Satchell, Satchell Evaluations and Chuck Forsberg,
 * Omen Technology.
 */

#define updcrc(cp, crc) ( crctab[(((unsigned)crc >> 8) & 255)] ^ (crc << 8) ^ (cp))
#define UPDC32(b, c) (cr3tab[((int)c ^ b) & 0xff] ^ ((c >> 8) & 0x00FFFFFF))

extern unsigned long cr3tab[];
extern unsigned short crctab[];

#define ZPAD '*'			 /* 052 Padding character begins frames */
#define ZDLE 030			 /* Ctrl-X Zmodem escape - `ala BISYNC DLE */
#define ZDLEE (ZDLE^0100)	 /* Escaped ZDLE as transmitted */
#define ZBIN 'A'			 /* Binary frame indicator */
#define ZHEX 'B'			 /* HEX frame indicator */
#define ZBIN32 'C'			 /* Binary frame with 32 bit FCS */

/* Frame types (see array "frametypes" in zm.c) */
#define ZRQINIT	0			 /* Request receive init */
#define ZRINIT	1			 /* Receive init */
#define ZSINIT 2			 /* Send init sequence (optional) */
#define ZACK 3				 /* ACK to above */
#define ZFILE 4				 /* File name from sender */
#define ZSKIP 5				 /* To sender: skip this file */
#define ZNAK 6				 /* Last packet was garbled */
#define ZABORT 7			 /* Abort batch transfers */
#define ZFIN 8				 /* Finish session */
#define ZRPOS 9				 /* Resume data trans at this position */
#define ZDATA 10			 /* Data packet(s) follow */
#define ZEOF 11				 /* End of file */
#define ZFERR 12			 /* Fatal Read or Write error Detected */
#define ZCRC 13				 /* Request for file CRC and response */
#define ZCHALLENGE 14		 /* Receiver's Challenge */
#define ZCOMPL 15			 /* Request is complete */
#define ZCAN 16				 /* Other end canned session with CAN*5 */
#define ZFREECNT 17			 /* Request for free bytes on filesystem */
#define ZCOMMAND 18			 /* Command from sending program */
#define ZSTDERR 19			 /* Output to standard error, data follows */

/* ZDLE sequences */
#define ZCRCE 'h'			 /* CRC next, frame ends, header packet
							  * follows */
#define ZCRCG 'i'			 /* CRC next, frame continues nonstop */
#define ZCRCQ 'j'			 /* CRC next, frame continues, ZACK expected */
#define ZCRCW 'k'			 /* CRC next, ZACK expected, end of frame */
#define ZRUB0 'l'			 /* Translate to rubout 0177 */
#define ZRUB1 'm'			 /* Translate to rubout 0377 */

/* zdlread return values (internal) */
/* -1 is general error, -2 is timeout */
#define GOTOR 0400
#define GOTCRCE (ZCRCE|GOTOR)/* ZDLE-ZCRCE received */
#define GOTCRCG (ZCRCG|GOTOR)/* ZDLE-ZCRCG received */
#define GOTCRCQ (ZCRCQ|GOTOR)/* ZDLE-ZCRCQ received */
#define GOTCRCW (ZCRCW|GOTOR)/* ZDLE-ZCRCW received */
#define GOTCAN	(GOTOR|030)	 /* CAN*5 seen */

/* Byte positions within header array */
#define ZF0	3				 /* First flags byte */
#define ZF1	2
#define ZF2	1
#define ZF3	0
#define ZP0	0				 /* Low order 8 bits of position */
#define ZP1	1
#define ZP2	2
#define ZP3	3				 /* High order 8 bits of file position */

/* Bit Masks for ZRINIT flags byte ZF0 */
#define CANFDX	01			 /* Rx can send and receive true FDX */
#define CANOVIO	02			 /* Rx can receive data during disk I/O */
#define CANBRK	04			 /* Rx can send a break signal */
#define CANCRY	010			 /* Receiver can decrypt */
#define CANLZW	020			 /* Receiver can uncompress */
#define CANFC32	040			 /* Receiver can use 32 bit Frame Check */
#define ESCCTL 0100			 /* Receiver expects ctl chars to be escaped */
#define ESC8   0200			 /* Receiver expects 8th bit to be escaped */

/* Parameters for ZSINIT frame */
#define ZATTNLEN 32			 /* Max length of attention string */
/* Bit Masks for ZSINIT flags byte ZF0 */
#define TESCCTL 0100		 /* Transmitter expects ctl chars to be
							  * escaped */
#define TESC8   0200		 /* Transmitter expects 8th bit to be escaped */

/* Parameters for ZFILE frame */
/* Conversion options one of these in ZF0 */
#define ZCBIN	1			 /* Binary transfer - inhibit conversion */
#define ZCNL	2			 /* Convert NL to local end of line convention */
#define ZCRESUM	3			 /* Resume interrupted file transfer */
/* Management include options, one of these ored in ZF1 */
#define ZMSKNOLOC	0200	 /* Skip file if not present at rx */
/* Management options, one of these ored in ZF1 */
#define ZMMASK	037			 /* Mask for the choices below */
#define ZMNEWL	1			 /* Transfer if source newer or longer */
#define ZMCRC	2			 /* Transfer if different file CRC or length */
#define ZMAPND	3			 /* Append contents to existing file (if any) */
#define ZMCLOB	4			 /* Replace existing file */
#define ZMNEW	5			 /* Transfer if source newer */
 /* Number 5 is alive ... */
#define ZMDIFF	6			 /* Transfer if dates or lengths different */
#define ZMPROT	7			 /* Protect destination file */
/* Transport options, one of these in ZF2 */
#define ZTLZW	1			 /* Lempel-Ziv compression */
#define ZTCRYPT	2			 /* Encryption */
#define ZTRLE	3			 /* Run Length encoding */
/* Extended options for ZF3, bit encoded */
#define ZXSPARS	64			 /* Encoding for sparse file operations */

/* Parameters for ZCOMMAND frame ZF0 (otherwise 0) */
#define ZCACK1	1			 /* Acknowledge, then do command */

/* FTOFFSET is offset for frametypes array in ecuzm.c */
#define FTOFFSET 3

long rclhdr();

#ifndef CFG_HasStrerror
char *strerror(); /* zcommon.c */
#endif

/* vi: set tabstop=4 shiftwidth=4: */
