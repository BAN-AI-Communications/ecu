#if defined(CFG_TelnetOption)

/*+-------------------------------------------------------------------------
	ldtelnet.c

	derived from rterm - teleplay.c - play with telnet by socket
	Copyright (C) 1992,1995, Warren H. Tucker, III
	All Rights Reserved.
	wht@wht.net (770)587-5766

  Defined functions:
	d_hostent(he)
	d_servent(servent)
	d_sockaddr_in(inaddr)
	do_use(topt)
	dont_use(topt)
	family_text(family)
	gethostaddr(namep, inetaddrp)
	inet_atou(addrstr)
	inet_utoa(addr)
	resolve_name(namep, inetaddrp, canonp, canonl)
	tcmd_text(tcmd)
	telnet_cmd(from_rcvr)
	telnet_open()
	telnet_open_failed(sig)
	telnet_option(tcmd, topt)
	telnet_subnegotiate()
	telnet_xmit_AYT()
	telnet_xmit_IP()
	telnet_xmit_opt(tcmd, topt)
	telnet_xmit_ttype()
	topt_text(topt)
	will_use(topt)
	wont_use(topt)

One day we need to handle the SIOCATMARK needs for the DM krock.

This text is from the telnet-5.52: (we just hack at this)
   Three pieces of state need to be kept for each side of each option.
   (You need the localside, sending WILL/WONT & receiving DO/DONT, and
   the remoteside, sending DO/DONT and receiving WILL/WONT)

        MY_STATE:       What state am I in?
        WANT_STATE:     What state do I want?
        WANT_RESP:      How many requests have I initiated?

   Default values:
        MY_STATE = WANT_STATE = DONT
        WANT_RESP = 0

   The local setup will change based on the state of the Telnet
   variables.  When we are the originator, we can either make the
   local setup changes at option request time (in which case if
   the option is denied we need to change things back) or when
   the option is acknowledged.

   To initiate a switch to NEW_STATE:

        if ((WANT_RESP == 0 && NEW_STATE == MY_STATE) ||
                        WANT_STATE == NEW_STATE) {
            do nothing;
        } else {
             * This is where the logic goes to change the local setup
             * if we are doing so at request initiation
            WANT_STATE = NEW_STATE;
            send NEW_STATE;
            WANT_RESP += 1;
        }

   When receiving NEW_STATE:

        if (WANT_RESP) {
            --WANT_RESP;
            if (WANT_RESP && (NEW_STATE == MY_STATE))
                --WANT_RESP;
        }
        if (WANT_RESP == 0) {
            if (NEW_STATE != WANT_STATE) {
                 * This is where the logic goes to decide if it is ok
                 * to switch to NEW_STATE, and if so, do any necessary
                 * local setup changes.
                if (ok_to_switch_to NEW_STATE)
                    WANT_STATE = NEW_STATE;
                else
                    WANT_RESP++;
*               if (MY_STATE != WANT_STATE)
                    reply with WANT_STATE;
            } else {
                 * This is where the logic goes to change the local setup
                 * if we are doing so at request acknowledgment
            }
        }
        MY_STATE = NEW_STATE;

* This if() line is not needed, it should be ok to always do the
  "reply with WANT_STATE".  With the if() line, asking to turn on
  an option that the other side doesn't understand is:
                Send DO option
                Recv WONT option
  Without the if() line, it is:
                Send DO option
                Recv WONT option
                Send DONT option
  If the other side does not expect to receive the latter case,
  but generates the latter case, then there is a potential for
  option negotiation loops.  An implementation that does not expect
  to get the second case should not generate it, an implementation
  that does expect to get it may or may not generate it, and things
  will still work.  Being conservative in what we send, we have the
  if() statement in, but we expect the other side to generate the
  last response.


--------------------------------------------------------------------------*/
/*+:EDITS:*/
/*:04-26-2000-11:16-wht@bob-RELEASE 4.42 */
/*:11-10-1999-00:39-wht@menlo-harden code while stealing for sockserv.c */
/*:06-06-1998-15:29-wht@WIN32-debug code bug */
/*:11-06-1998-14:38-wht@menlo-SO_OOBINLINE */
/*:09-21-1998-15:37-wht@gyro-better inet_atou from other effort */
/*:12-22-1997-03:06-wht@kepler-gethostaddr interruptible */
/*:12-21-1997-14:25-wht@sidonia-do not limit rexmit of TTYPE */
/*:03-03-1997-15:58-wht@yuriatin-getservbyname had wrong protocol */
/*:01-24-1997-02:37-wht@yuriatin-SOURCE RELEASE 4.00 */
/*:10-15-1996-20:36-wht@yuriatin-add timeout/interruptibility to connect */
/*:09-29-1996-14:58-wht@yuriatin-add telnet_xmit_IP */
/*:09-18-1996-06:17-wht@yuriatin-add telnet_xmit_AYT */
/*:09-11-1996-20:00-wht@yuriatin-3.48-major telnet,curses,structural overhaul */
/*:09-06-1996-02:43-wht@kepler-cleanup */
/*:08-20-1996-12:39-wht@kepler-locale/ctype fixes from ache@nagual.ru */
/*:08-11-1996-02:10-wht@kepler-rename ecu_log_event to logevent */
/*:12-06-1995-13:30-wht@n4hgf-termecu w/errno -1 consideration */
/*:11-23-1995-11:20-wht@kepler-source control 3.37 for tsx-11 */
/*:11-20-1995-00:16-wht@n4hgf-superflous include of arpa/telnet.h */
/*:11-19-1995-20:56-wht@gyro-correct ignored command text */
/*:11-14-1995-10:23-wht@kepler-3.37.80-source control point: SOCKETS */
/*:11-12-1995-00:18-wht@gyro-shm->show_telnet_traffic now in shm */
/*:11-10-1995-18:27-wht@gyro-hack becomes user 'shm->show_telnet_traffic' */
/*:11-10-1995-16:14-wht@gyro-use lgetc_rcvr_raw instead of lgetc_rcvr */
/*:11-10-1995-15:53-wht@gyro-do not repeat a DO/DONT in response to WILL */
/*:11-04-1995-21:03-wht@kepler-read from line correctly as rcvr and xmtr */
/*:11-03-1995-16:54-wht@wwtp1-use CFG_TelnetOption */
/*:10-19-1995-02:16-wht@kepler-creation */

#include "ecu.h"
#include "ecukey.h"
#include "esd.h"
#include "var.h"
#include "ecupde.h"
#include "ecuerror.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <setjmp.h>

#include <net/if.h>
#include <netinet/in.h>
#include <netdb.h>

#include "ldtelnet.h"

void telnet_xmit_opt();
void telnet_xmit_ttype();
char *tcmd_text();
void telnet_cmd();

/*
 * these are public notices
 */
UINT32 hostaddr;
int portnum;

uchar Oecho = DO;			 /* echo xmit data? */
char *Ottype = "ANSI";		 /* terminal type */
extern char *ttype;			 /* actual terminal type */
uchar telopt_already[256];
#if defined(SEND_TTYPE_ONCE)
static int _already_sent_ttype;
#endif
static jmp_buf _insurance_jmpbuf;

/*
 * eol_type
 *
 * 0 = strip 0x0D, NL terminator
 * 1 = 0x0D+0x00 == 0x0A
 */
int eol_type = 0;

/*
 * local "get character from line"
 * switch ... see telnet_cmd()
 * at "_lgetc = (from_rcvr) ? lgetc_rcvr_raw : lgetc_xmtr_raw;"
 *
 */
static PFU _lgetc;

/*
 * Internet address
 * a private copy for display use only
 */

struct in_addrP
{
	union
	{
		struct
		{
			uchar s_b1, s_b2, s_b3, s_b4;
		} S_un_b;
		struct
		{
			UINT16 s_w1, s_w2;
		} S_un_w;
		UINT32 S_addr;
	} S_un;
#define s_addrP  S_un.S_addr /* should be used for all code */
};

/*+-------------------------------------------------------------------------
	tcmd_text(tcmd) - telnet cmd to text

		EOR ...........ef
		SE             f0
		NOP ...........f1
		DM/SYNCH       f2
		BREAK .........f3
		IP             f4
		AO ............f5
		AYT            f6
		EC ............f7
		EL             f8
		GA ............f9
		SB             fa
		WILL ..........fb
		WONT           fc
		DO ............fd
		DONT           fe
		IAC ...........ff
--------------------------------------------------------------------------*/
char *
tcmd_text(tcmd)
uchar tcmd;
{
	static char static6[6];

	switch (tcmd)
	{
		case IAC:
			return ("IAC");
		case DONT:
			return ("DONT");
		case DO:
			return ("DO");
		case WONT:
			return ("WONT");
		case WILL:
			return ("WILL");
		case SB:
			return ("SB");
		case GA:
			return ("GA");
		case EL:
			return ("EL");
		case EC:
			return ("EC");
		case AYT:
			return ("AYT");
		case AO:
			return ("AO");
		case IP:
			return ("IP");
		case BREAK:
			return ("BREAK");
		case DM:
			return ("DM/SYNCH");
		case NOP:
			return ("NOP");
		case SE:
			return ("SE");
		case EOR:
			return ("EOR");
		default:
			sprintf(static6, "?%02x?", tcmd);
			return (static6);
	}

}							 /* end of tcmd_text */

/*+-------------------------------------------------------------------------
	topt_text(topt) - TELOPT to text
--------------------------------------------------------------------------*/
char *
topt_text(topt)
uchar topt;
{
	static char static6[6];

	switch (topt)
	{
		case TELOPT_BINARY:
			return ("BINARY");
		case TELOPT_ECHO:
			return ("ECHO");
		case TELOPT_RCP:
			return ("RCP");
		case TELOPT_SGA:
			return ("SGA");
		case TELOPT_NAMS:
			return ("NAMS");
		case TELOPT_STATUS:
			return ("STATUS");
		case TELOPT_TM:
			return ("TM");
		case TELOPT_RCTE:
			return ("RCTE");
		case TELOPT_NAOL:
			return ("NAOL");
		case TELOPT_NAOP:
			return ("NAOP");
		case TELOPT_NAOCRD:
			return ("NAOCRD");
		case TELOPT_NAOHTS:
			return ("NAOHTS");
		case TELOPT_NAOHTD:
			return ("NAOHTD");
		case TELOPT_NAOFFD:
			return ("NAOFFD");
		case TELOPT_NAOVTS:
			return ("NAOVTS");
		case TELOPT_NAOVTD:
			return ("NAOVTD");
		case TELOPT_NAOLFD:
			return ("NAOLFD");
		case TELOPT_XASCII:
			return ("XASCII");
		case TELOPT_LOGOUT:
			return ("LOGOUT");
		case TELOPT_BM:
			return ("BM");
		case TELOPT_DET:
			return ("DET");
		case TELOPT_SUPDUP:
			return ("SUPDUP");
		case TELOPT_SUPDUPOUTPUT:
			return ("SUPDUPOUTPUT");
		case TELOPT_SNDLOC:
			return ("SNDLOC");
		case TELOPT_TTYPE:
			return ("TTYPE");
		case TELOPT_EOR:
			return ("EOR");
		case TELOPT_EXOPL:
			return ("EXOPL");
#if defined(TELOPT_TUID)
		case TELOPT_TUID:
			return ("TUID");
#endif
#if defined(TELOPT_OUTMRK)
		case TELOPT_OUTMRK:
			return ("OUTMRK");
#endif
#if defined(TELOPT_TTYLOC)
		case TELOPT_TTYLOC:
			return ("TTYLOC");
#endif
#if defined(TELOPT_3270REGIME)
		case TELOPT_3270REGIME:
			return ("3270REGIME");
#endif
#if defined(TELOPT_X3PAD)
		case TELOPT_X3PAD:
			return ("X3PAD");
#endif
#if defined(TELOPT_NAWS)
		case TELOPT_NAWS:
			return ("NAWS");
#endif
#if defined(TELOPT_TSPEED)
		case TELOPT_TSPEED:
			return ("TSPEED");
#endif
#if defined(TELOPT_LFLOW)
		case TELOPT_LFLOW:
			return ("LFLOW");
#endif
#if defined(TELOPT_LINEMODE)
		case TELOPT_LINEMODE:
			return ("LINEMODE");
#endif
#if defined(TELOPT_XDISPLOC)
		case TELOPT_XDISPLOC:
			return ("XDISPLOC");
#endif
#if defined(TELOPT_ENVIRON)
		case TELOPT_ENVIRON:
			return ("ENVIRON");
#endif
#if defined(TELOPT_AUTHENTICATION)
		case TELOPT_AUTHENTICATION:
			return ("AUTHENTICATION");
#endif
#if defined(TELOPT_ENCRYPT)
		case TELOPT_ENCRYPT:
			return ("ENCRYPT");
#endif
		default:
			sprintf(static6, "%d?", topt);
			return (static6);
	}
}							 /* end of topt_text */

/*+-------------------------------------------------------------------------
	inet_utoa(addr) - IP address unsigned to ascii
--------------------------------------------------------------------------*/
char *
inet_utoa(addr)
UINT32 addr;
{
	static char txt[40];

	sprintf(txt, "%u.%u.%u.%u",
		(unsigned)(addr >> 24) & 0xFF,
		(unsigned)(addr >> 16) & 0xFF,
		(unsigned)(addr >> 8) & 0xFF,
		(unsigned)(addr) & 0xFF);
	return (txt);

}							 /* end of inet_utoa */

/*+-------------------------------------------------------------------------
	inet_atou(addrstr) - IP address ascii to unsigned

  This routine returns 0  on error, so it may nolt be used to
  decode "0.0.0.0" without ambiguity!
--------------------------------------------------------------------------*/
unsigned long
inet_atou(addrstr)
char *addrstr;
{
	int shift = 24;
	int count;
	char *cp;
	unsigned long ipaddr = 0;
	unsigned int blob;

	for (count = 0; count < 6; ++count)
	{
		if(!isdigit(*addrstr))
			return(0);
		if ((blob = atoi(addrstr)) > 255)
			return (0);
		ipaddr |= blob << shift;
		shift -= 8;
		if (!(cp = strchr(addrstr, '.')))
			break;
		addrstr = cp + 1;
	}

	if (count != 3)
		return (0);

	return (ipaddr);
}							 /* end of inet_atou */

/*+-------------------------------------------------------------------------
	telnet_xmit_AYT() - send Are You There?
--------------------------------------------------------------------------*/
void
telnet_xmit_AYT()
{
	uchar buf[4];
	uchar *ucp;

	if (!shm->Liofd)
	{
		pputs(" < cannot send AYT while unconnected.\n");
		return;
	}

	if (!shm->Ltelnet)
	{
		pputs(" < why send AYT on async connection?\n");
		return;
	}

	if (shm->show_telnet_traffic)
		pputs(" < sending AYT\n");

	ucp = buf;
	*ucp++ = IAC;
	*ucp++ = AYT;
	write(shm->Liofd, buf, (int)(ucp - buf));

}							 /* end of telnet_xmit_AYT */

/*+-------------------------------------------------------------------------
	telnet_xmit_IP() - send Interrupt Process
--------------------------------------------------------------------------*/
void
telnet_xmit_IP()
{
	uchar buf[4];
	uchar *ucp;

	if (!shm->Liofd)
	{
		pputs(" < cannot send IP while unconnected.\n");
		return;
	}

	if (!shm->Ltelnet)
	{
		pputs(" < why send IP on async connection?\n");
		return;
	}

	if (shm->show_telnet_traffic)
		pputs(" < sending IP\n");

	ucp = buf;
	*ucp++ = IAC;
	*ucp++ = IP;
	write(shm->Liofd, buf, (int)(ucp - buf));

}							 /* end of telnet_xmit_IP */

/*+-------------------------------------------------------------------------
	telnet_xmit_ttype() - send terminal type

  but only once per session

--------------------------------------------------------------------------*/
void
telnet_xmit_ttype()
{
	uchar buf[64];
	uchar *ucp;

#if defined(SEND_TTYPE_ONCE)
	if (_already_sent_ttype)
		return;
	_already_sent_ttype = 1;
#endif

	if (shm->show_telnet_traffic)
		pprintf(" < our TTYPE '%s'\n", shm->telnet_ttype);

	ucp = buf;
	*ucp++ = IAC;
	*ucp++ = SB;
	*ucp++ = TELOPT_TTYPE;
	*ucp++ = TELQUAL_IS;
	strcpy((char *)ucp, shm->telnet_ttype);
	ucp += strlen(shm->telnet_ttype) + 1;
	*ucp++ = IAC;
	*ucp++ = SE;
	write(shm->Liofd, buf, (int)(ucp - buf));

}							 /* end of telnet_xmit_ttype */

/*+-------------------------------------------------------------------------
	telnet_xmit_opt(tcmd,topt) - common "send option" routine
--------------------------------------------------------------------------*/
void
telnet_xmit_opt(tcmd, topt)
uchar tcmd;
uchar topt;
{
	uchar optbuf[8];
	uchar *optptr = optbuf;

	if (telopt_already[topt])
	{
		if (shm->show_telnet_traffic)
		{
			pprintf(" > we already said %s %s\n",
				tcmd_text(telopt_already[topt]),
				topt_text(topt));
		}

		return;
	}

	*optptr++ = IAC;
	*optptr++ = tcmd;
	*optptr++ = topt;
	write(shm->Liofd, optbuf, (int)(optptr - optbuf));

	telopt_already[topt] = tcmd;

	if (shm->show_telnet_traffic)
	{
		pprintf(" > we say %s %s\n", tcmd_text(tcmd),
			topt_text(topt));
	}

}							 /* end of telnet_xmit_opt */

/*+-------------------------------------------------------------------------
	telnet_subnegotiate() - SB handler
--------------------------------------------------------------------------*/
void
telnet_subnegotiate()
{
	char s128[128];
	uchar topt_list[128];
	uchar *topt_ptr = topt_list;
	int topt_count = 0;
	int i;
	unsigned char opt;

	if (shm->show_telnet_traffic)
		pprintf(" >< entering subnegotiation\n");

	while (i = (*_lgetc) ())
	{
		if ((*topt_ptr = i) == IAC)
			break;
		topt_ptr++;
		topt_count++;
	}
	if (i == -1)
		return;

	if ((i = (*_lgetc) ()) != SE)
	{
		sprintf(s128, "\nsubnegotiate: expected SE, got 0x%02x\n",
			i & 0xFF);
		pputs(s128);
		errno = -1;
		termecu(TERMECU_UNRECOVERABLE);
	}
	topt_ptr = topt_list;

	if (shm->show_telnet_traffic)
		hex_dump(topt_ptr, topt_count, "subnegotiate str", 1);

	while (topt_count--)
	{
		opt = *topt_ptr++;
		if (shm->show_telnet_traffic)
		{
			pprintf(" > remote %s option %s\n",
				(*topt_ptr == TELQUAL_SEND) ? "requesting" : "in effect:",
				topt_text(opt));
		}

		i = -1;
		switch (opt)
		{
			case TELOPT_BINARY:
				i = DONT;
				break;
			case TELOPT_ECHO:
				i = Oecho;
				break;
			case TELOPT_SGA:
				i = DONT;
				break;
			case TELOPT_XASCII:
				i = DONT;
				break;
			case TELOPT_LOGOUT:
				i = DONT;
				break;
			case TELOPT_EOR:
				i = DONT;
				break;
			case TELOPT_EXOPL:
				i = DONT;
				break;
			case TELOPT_TTYPE:
				if (topt_count && (*topt_ptr == TELQUAL_SEND))
				{
					topt_ptr++, topt_count--;
					telnet_xmit_ttype();
					i = 0;
					continue;
				}
				break;
		}

		if (topt_count && (*topt_ptr == TELQUAL_SEND))
		{
			if (i)		 /* if not handled in switch statement */
			{
				if (i == -1)
				{
					pprintf("dont know how to subnegotiate %s\n",
						topt_text(opt));
					telnet_xmit_opt(WONT, opt);
				}
				else
				{
					telnet_xmit_opt((uchar) i, opt);
					if (shm->show_telnet_traffic)
					{
						pprintf(" > subnegotiate %s %s\n",
							tcmd_text((uchar) i),
							topt_text(opt));
					}
				}
			}
		}
		else if (shm->show_telnet_traffic)
		{
			char buf[64];
			sprintf(buf, " << %s", topt_text(opt));
			hex_dump(topt_ptr + 1, topt_count - 1, buf, 1);
		}
	}

}							 /* end of telnet_subnegotiate */

/*+-------------------------------------------------------------------------
	telnet_option(tcmd,topt) - we received a TELOPT from host
--------------------------------------------------------------------------*/
void
telnet_option(tcmd, topt)
uchar tcmd;
uchar topt;
{
	if (shm->show_telnet_traffic)
	{
		pprintf(" < remote said %s %s\n", tcmd_text(tcmd),
			topt_text(topt));
	}

	switch (tcmd)
	{
		case DO:
			switch (topt)
			{
				case TELOPT_TTYPE:
					telnet_xmit_ttype();
					break;
				case TELOPT_ECHO:
					telnet_xmit_opt(WONT, topt);
					break;
				default:
					telnet_xmit_opt(WONT, topt);
					break;
			}
			break;
		case DONT:
			telnet_xmit_opt(WONT, topt);
			break;
		case WILL:
			if (topt > TELOPT_TTYPE)
				telnet_xmit_opt(DONT, topt);
			else if (topt == TELOPT_ECHO)
				telnet_xmit_opt(DO, topt);
			else if (topt == TELOPT_SGA)
				telnet_xmit_opt(DO, topt);
			else
				telnet_xmit_opt(DONT, topt);
			break;
		case WONT:
			break;
	}
}							 /* end of telnet_option */

/*+-------------------------------------------------------------------------
	telnet_cmd() - IAC received: read and process command

  Alas, ecu has two ways of reading from line, the xmtr
  way and the rcvr way.
--------------------------------------------------------------------------*/
void
telnet_cmd(from_rcvr)
int from_rcvr;
{
	int cmd;
	UINT lgetc_rcvr_raw();
	UINT lgetc_xmtr_raw();

	/*
	 * function to use for getting line character for rest of this telnet
	 * command processing
	 */
	_lgetc = (from_rcvr) ? lgetc_rcvr_raw : lgetc_xmtr_raw;

	switch (cmd = (*_lgetc) ())
	{
		case DONT:
		case DO:
		case WONT:
		case WILL:
			telnet_option(cmd, (*_lgetc) ());
			break;
		case SB:
			telnet_subnegotiate();
			break;
		case GA:
		case EL:
		case EC:
		case AYT:
		case AO:
		case IP:
		case BREAK:
		case DM:
		case NOP:
		case SE:
		case EOR:
			if (shm->show_telnet_traffic)
				pprintf(" < %s (ignored)\n", tcmd_text(cmd));
			break;

	}
}							 /* end of telnet_cmd */

/*+-------------------------------------------------------------------------
	will_use(topt) - send to remote system that we will use an option
--------------------------------------------------------------------------*/
void
will_use(topt)
uchar topt;
{
	telnet_xmit_opt(WILL, topt);
}							 /* end of will_use */

/*+-------------------------------------------------------------------------
	wont_use(topt) - send to remote system that we wont use an option
--------------------------------------------------------------------------*/
void
wont_use(topt)
uchar topt;
{
	telnet_xmit_opt(WONT, topt);
}							 /* end of wont_use */

/*+-------------------------------------------------------------------------
	do_use(topt) - ask remote system to use an option
--------------------------------------------------------------------------*/
void
do_use(topt)
uchar topt;
{
	telnet_xmit_opt(DO, topt);
}							 /* end of do_use */

/*+-------------------------------------------------------------------------
	dont_use(topt) - ask remote system not to use an option
--------------------------------------------------------------------------*/
void
dont_use(topt)
uchar topt;
{
	telnet_xmit_opt(DONT, topt);
}							 /* end of dont_use */

/*+-------------------------------------------------------------------------
	family_text(family) - AF_* to text
--------------------------------------------------------------------------*/
#if defined(DEBUG)
char *
family_text(family)
short family;
{
	static char static32[32];

	switch (family)
	{
#if defined(AF_UNSPEC)
		case AF_UNSPEC:
			return ("Unspecified");
#endif
#if defined(AF_UNIX)
		case AF_UNIX:
			return ("UNIX");
#endif
#if defined(AF_INET)
		case AF_INET:
			return ("INTERNET");
#endif
#if defined(AF_IMPLINK)
		case AF_IMPLINK:
			return ("ARPANET/IMP");
#endif
#if defined(AF_PUP)
		case AF_PUP:
			return ("PUP");
#endif
#if defined(AF_CHAOS)
		case AF_CHAOS:
			return ("MIT CHAOS");
#endif
#if defined(AF_NS)
		case AF_NS:
			return ("XEROX NS");
#endif
#if defined(AF_NBS)
		case AF_NBS:
			return ("NBS");
#endif
#if defined(AF_ECMA)
		case AF_ECMA:
			return ("EUROPEAN");
#endif
#if defined(AF_DATAKIT)
		case AF_DATAKIT:
			return ("DATAKIT");
#endif
#if defined(AF_CCITT)
		case AF_CCITT:
			return ("CCITT");
#endif
#if defined(AF_SNA)
		case AF_SNA:
			return ("SNA");
#endif
#if defined(AF_DECnet)
		case AF_DECnet:
			return ("DECnet");
#endif
#if defined(AF_DLI)
		case AF_DLI:
			return ("DATA LINK INTERFACE");
#endif
#if defined(AF_LAT)
		case AF_LAT:
			return ("LAT");
#endif
#if defined(AF_HYLINK)
		case AF_HYLINK:
			return ("NSC Hyperchannel");
#endif
#if defined(AF_APPLETALK)
		case AF_APPLETALK:
			return ("Apple Talk");
#endif
#if defined(AF_ISO)
		case AF_ISO:
			return ("ISO");
#endif
		default:
			sprintf(static32, "number %d\n", family);
			return (static32);
	}
	/* NOTREACHED */

}							 /* end of family_text */
#endif

/*+-------------------------------------------------------------------------
	d_sockaddr_in(inaddr) - display INTERNET socket address
--------------------------------------------------------------------------*/
#if defined(DEBUG)
void
d_sockaddr_in(inaddr)
struct sockaddr_in *inaddr;
{
	int i;

	pprintf("sockaddr_in @ %08lx family: %s port: 0x%04x\n", inaddr,
		family_text(inaddr->sin_family), inaddr->sin_port);
	pprintf("            port 0x%08lx", ntohl(inaddr->sin_addr.s_addr));
	fputs(" zero:", stdout);
	for (i = 0; i < sizeof(inaddr->sin_zero); i++)
		pprintf(" %02x", inaddr->sin_zero[i]);
	fputs("\n\n", stdout);
}							 /* end of d_sockaddr_in */
#endif

/*+-------------------------------------------------------------------------
	d_hostent(he) - display host entry
--------------------------------------------------------------------------*/
#if defined(DEBUG)
void
d_hostent(he)
struct hostent *he;
{
	int i;
	uchar *haddr;

	pprintf("hostent @ %08lx name: %s family: %s address:", he,
		he->h_name, family_text(he->h_addrtype));
#if defined(BSD43)
	haddr = (uchar *) he->h_addr_list[0];
#else
	haddr = (uchar *) he->h_addr;
#endif
	i = he->h_length;
	while (i--)
		pprintf(" %02x", *haddr++);
	fputs("\n\n", stdout);

}							 /* end of d_hostent */
#endif

/*+-------------------------------------------------------------------------
	d_servent(servent) - display server entry
--------------------------------------------------------------------------*/
#if defined(DEBUG)
void
d_servent(servent)
struct servent *servent;
{
	pprintf("servent @ %08lx name: %s port: 0x%08x protocol: %s\n\n", servent,
		servent->s_name, servent->s_port, servent->s_proto);
}							 /* end of d_servent */

#endif

/*+-------------------------------------------------------------------------
	resolve_name(namep,inetaddrp,canonp,canonl) - host name lookup

  If name argument is dotted-decimal string (first char is
  digit), just convert it without lookup, otherwise resolve host
  name using domain resolver or whatever, and copy canonical host
  name into canonp[canonl].

  Thanks for this from ping.c (but some mods)
--------------------------------------------------------------------------*/
int
resolve_name(namep, inetaddrp, canonp, canonl)
char *namep;
UINT32 *inetaddrp;
char *canonp;
int canonl;
{
	int i;
	struct hostent *hp;

	if (isdigit((uchar) * namep))
	{
		/* Assume dotted-decimal */
		if (canonp)
			*canonp = 0;	 /* No canonical name */
		*inetaddrp = inet_atou(namep);
		return (0);
	}
	if (!(hp = gethostbyname(namep)))
	{
		if (canonp)
			*canonp = '\0';	 /* No canonical name */
		return (-1);
	}
	i = ((i = strlen(hp->h_name)) >= canonl) ? canonl - 1 : i;
	if (canonp)
	{
		(void)memcpy(canonp, hp->h_name, i);
		*(canonp + i) = '\0';
	}
	*inetaddrp = ntohl(*((UINT32 *) hp->h_addr));
	return (0);

}							 /* end of resolve_name */

/*+-------------------------------------------------------------------------
	gethostaddr(namep,inetaddrp) - convenient when only address desired
--------------------------------------------------------------------------*/
int
gethostaddr(namep, inetaddrp)
char *namep;
UINT32 *inetaddrp;
{
	return (resolve_name(namep, inetaddrp, (char *)0, 0));
}							 /* end of gethostaddr */

/*+-------------------------------------------------------------------------
	telnet_open_failed(sig) - see telnet_open() below
--------------------------------------------------------------------------*/
void
telnet_open_failed(sig)
int sig;
{
	if (sig != SIGALRM)
		ff(se, "error %d in telnet_open_failed: tell wht@n4hgf\r\n", sig);
	longjmp(_insurance_jmpbuf, 1);

}							 /* end of telnet_open_failed */

/*+-------------------------------------------------------------------------
	telnet_open() - make telnet connection

returns open, conditioned socket with preliminary exchange started
--------------------------------------------------------------------------*/
enum linst
telnet_open()
{
	int i;
	char *cptr;
	struct servent *servent;
	struct sockaddr_in destaddr;
	struct stat st;
	UINT32 colors_at_entry = colors_current;
	char result[128];
	char hostnm[DESTREF_LEN + 1];
	char *tt;
	char ttype_upper[128];
	int old_ttymode = get_ttymode();	/* save original tty mode */
	extern char kbdintr;

#if defined(SOL_SOCKET)
	int bfsz;
	int intsz;

#endif /* SOL_SOCKET */

	shm->Liofd = -1;		 /* make sure */
	shm->Ltelnet = 0;		 /* make sure */

	/*
	 * all of these meaningless in telnet mode
	 */
	shm->Lparity = 0;
	shm->Lfull_duplex = 1;
	shm->Lxonxoff = 0;
	shm->Ldcdwatch = 0;
	shm->Ladd_nl_incoming = 0;
	shm->Ladd_nl_outgoing = 0;

	/*
	 * choose terminal type to transmit
	 */
	tt = Ottype;
	if (!shm->rcvr_ansi_filter)
	{
		char *cp;

		strcpy(ttype_upper, ttype);
		cp = ttype_upper;
		while (*cp)
		{
			*cp = to_upper(*cp);
			cp++;
		}
		tt = ttype_upper;
	}

	strncpy(shm->telnet_ttype, tt, sizeof(shm->telnet_ttype));
	shm->telnet_ttype[sizeof(shm->telnet_ttype) - 1] = 0;
#if defined(SEND_TTYPE_ONCE)
	_already_sent_ttype = 0;
#endif

	/*
	 * have not transmitted any options
	 */
	memset(telopt_already, 0, sizeof(telopt_already));

	/*
	 * get host name and port number
	 */
	strcpy(hostnm, shm->Ltelno);
	if (!(servent = getservbyname("telnet", "tcp")))
		portnum = 23;
	else
		portnum = ntohs(servent->s_port);
	if (cptr = strchr(hostnm, ':'))
	{
		*cptr = 0;
		portnum = atoi(cptr + 1);
	}
	if ((i = strlen(hostnm)) && (hostnm[i - 1] == '.'))
		hostnm[i - 1] = 0;

	if(!isdigit((uchar)hostnm[0]))
	{
		pprintf("Looking up address for host%s'%.50s'\n",
			(strlen(hostnm) > 40) ? "\n    " : " ",hostnm);
	}

	errno = -1;
	sigint = 0;
	ttymode(2);
	if (gethostaddr(hostnm, &hostaddr))
	{
		if(sigint)
		{
			sigint = 0;
			pprintf("interrupted\n");
			iv[0] = 2;			 /* interrupted */
		}
		else
		{
			pprintf("Cannot get the address for host '%.50s'\n", hostnm);
			iv[0] = 3;			 /* "modem error" */
		}
		ttymode(old_ttymode);
		return (LINST_TELNETFAIL);
	}
	ttymode(old_ttymode);

	if(isdigit((uchar)hostnm[0]))
		pprintf("Calling [%s]:%d ", inet_utoa(hostaddr), portnum);
	else
	{
		pprintf("Calling %.50s [%s]:%d\n",
			hostnm, inet_utoa(hostaddr), portnum);
	}
	if (!tty_not_char_special)
	{
		pprintf("(type %s to abort) ",
			(kbdintr == DEL) ? "DEL" : graphic_char_text(kbdintr, 0));
	}
	pputs("...");

	memset((char *)&destaddr, 0, sizeof(destaddr));
	destaddr.sin_family = AF_INET;
	destaddr.sin_addr.s_addr = htonl(hostaddr);
	destaddr.sin_port = htons(portnum);

	errno = -1;
	shm->Liofd = socket(PF_INET, SOCK_STREAM, 0);
	if (shm->Liofd < 0)
	{
		setcolor(colors_error);
		perror("\nsocket creation");
		setcolor(colors_at_entry);
		iv[0] = 3;			 /* "modem error" */
		return (LINST_TELNETFAIL);
	}

#if defined(SO_OOBINLINE)
	i = 1;
    setsockopt(shm->Liofd, SOL_SOCKET, SO_OOBINLINE,
		(char *)&i, sizeof(i));
#endif

	if (setjmp(_insurance_jmpbuf))
	{
		alarm(0);
		signal(SIGALRM, SIG_IGN);
		setcolor(colors_error);
		pputs("timed out\n");
		setcolor(colors_at_entry);
		goto CONNECT_FAILED;
	}

	ttymode(2);
	alarm(20);
	signal(SIGALRM,telnet_open_failed);
	errno = 0;
	i = connect(shm->Liofd, (struct sockaddr *)&destaddr,
			sizeof(destaddr));
	alarm(0);
	signal(SIGALRM, SIG_IGN);
	ttymode(old_ttymode);
	if(i < 0)
	{
		setcolor(colors_error);
		if(sigint)
			pputs("interrupted\n");
		else
			pperror("\nfailed");
		setcolor(colors_at_entry);
		goto CONNECT_FAILED;
	}

	/*
	 * see if we really have a connection
	 */
	if (fstat(shm->Liofd, &st))
	{
		setcolor(colors_error);
		pputs("\nconnection rejected by remote\n");
		setcolor(colors_at_entry);
CONNECT_FAILED:
		close(shm->Liofd);
		shm->Liofd = -1;
		iv[0] = 1;			 /* connect failed */
		return (LINST_TELNETFAIL);
	}

#if defined(SOL_SOCKET)
#if defined(SO_SNDBUF)
	intsz = sizeof(int);

	if (getsockopt(shm->Liofd, SOL_SOCKET, SO_SNDBUF, (char *)&bfsz, &intsz))
	{
		pperror("getsockopt SO_SNDBUF");
	}
	else if (bfsz < 32768)
	{
		bfsz = 32768;
		if (setsockopt(shm->Liofd, SOL_SOCKET, SO_SNDBUF,
				(char *)&bfsz, sizeof(bfsz)))
		{
			pperror("setsockopt SO_SNDBUF");
		}
	}
#endif /* SO_SNDBUF */

#if defined(SO_RCVBUF)
	intsz = sizeof(int);

	if (getsockopt(shm->Liofd, SOL_SOCKET, SO_RCVBUF,
			(char *)&bfsz, &intsz))
	{
		pperror("getsockopt SO_RCVBUF");
	}
	else if (bfsz < 32768)
	{
		bfsz = 32768;
		if (setsockopt(shm->Liofd, SOL_SOCKET, SO_RCVBUF,
				(char *)&bfsz, sizeof(bfsz)))
		{
			pperror("setsockopt SO_RCVBUF");
		}
	}
#endif /* SO_RCVBUF */

#endif /* SOL_SOCKET */

	/*
	 * well, GLORY!
	 */
	shm->Ltelnet = 1;
	shm->Lconnected = 1;
	sprintf(shm->Lipaddr_str, "%s:%d", inet_utoa(hostaddr), portnum);

	setcolor(colors_success);
	pputs("success\n");
	sprintf(result, "CONNECT %d (guess)", shm->Lbitrate);
	pprintf("%s\n", result);
	setcolor(colors_at_entry);
	pprintf("Ansi filter is %s; sending terminal type %s to remote\n",
		(shm->rcvr_ansi_filter) ? "ON" : "off",
		shm->telnet_ttype);

	vlogevent(getpid(), "CONNECT %.50s (telnet)", shm->Ltelno);
	strncpy(shm->Lrname, hostnm, sizeof(shm->Lrname));
	shm->Lrname[sizeof(shm->Lrname) - 1] = 0;

	if (!keyset_read(shm->Llogical))
		pprintf("[autoloaded fkeys for %s]\n", shm->Llogical);

	shm->xmit_chars_this_connect = 0;
	shm->rcvd_chars_this_connect = 0;
	shm->Lconnected = 1;
	time(&shm->Loff_hook_time);
	iv[0] = 0;
	if (proc_trace)
		pputs("telnet connect() set $i0 = 0\n");
	strcpy(sv[0]->pb, result);
	sv[0]->cb = strlen(result);

	/*
	 * this is a gray area for me
	 */
	do_use(TELOPT_SGA);
	will_use(TELOPT_TTYPE);

	/*
	 * do the _connect.ep
	 */
	if (find_procedure("_connect"))
	{
		int erc;
		char *_doproc_args[3];

		_doproc_args[0] = "_connect";	/* _connect.ep */
		_doproc_args[1] = result;	/* "CONNECT XXXX" */
		if (erc = do_proc(2, _doproc_args))
		{
			DCE_hangup();
			if (erc < 256)
				sprintf(result, "!CONNECT PROCEDURE RETURNED %d", erc);
			else
				strcpy(result, "!CONNECT PROCEDURE ABNORMAL TERMINATION");
			strcpy(sv[0]->pb, result);
			sv[0]->cb = strlen(result);
			setcolor(colors_error);
			pprintf("%s\n", result);
			iv[0] = 1;
			DCE_report_iv_set(0);
		}
	}

	return (LINST_OK);
}							 /* end of telnet_open */

#endif /* CFG_TelnetOption */

/* vi: set tabstop=4 shiftwidth=4: */
/* end of ldtelnet.c */
