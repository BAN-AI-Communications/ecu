/*+-------------------------------------------------------------------------
	fasiintf.c - FAS/i interface
	wht@wht.net

  Defined functions:
	display_fasi(fip)
	fasi_breaks_detected()
	fasi_line_errors()
	fasi_msr()
	fasi_rings_detected()
	icmd_fasi(narg, arg)
	ier_text(ier)
	lcr_text(lcr)
	mcr_text(mcr)
	msr_text(msr)
	pcmd_fasi(param)

--------------------------------------------------------------------------*/
/*+:EDITS:*/
/*:04-26-2000-11:15-wht@bob-RELEASE 4.42 */
/*:01-24-1997-02:37-wht@yuriatin-SOURCE RELEASE 4.00 */
/*:09-11-1996-20:00-wht@yuriatin-3.48-major telnet,curses,structural overhaul */
/*:11-23-1995-11:20-wht@kepler-source control 3.37 for tsx-11 */
/*:11-14-1995-10:23-wht@kepler-3.37.80-source control point: SOCKETS */
/*:05-04-1994-04:39-wht@n4hgf-ECU release 3.30 */
/*:09-10-1992-13:59-wht@n4hgf-ECU release 3.20 */
/*:08-22-1992-15:38-wht@n4hgf-ECU release 3.20 BETA */
/*:07-25-1991-12:57-wht@n4hgf-ECU release 3.10 */
/*:04-29-1991-18:52-wht@n4hgf-FAS/i 2.08.0 official release */
/*:12-24-1990-00:51-wht-creation */

#include "ecu.h"

#if	defined(FASI)
#include "ecuerror.h"
#include "esd.h"
#if defined(NULL)
#undef NULL
#endif
#include <local/fas.h>


/*+-------------------------------------------------------------------------
	msr_text(msr)
--------------------------------------------------------------------------*/
char *
msr_text(msr)
uchar msr;
{
	static char txt[50];

	txt[0] = '*';
	txt[1] = 0;
	if (!msr)
	{
		strcat(txt, "NULL*");
		return (txt);
	}
	if (msr & MS_CTS_DELTA)
		strcat(txt, "dCTS*");
	if (msr & MS_DSR_DELTA)
		strcat(txt, "dDSR*");
	if (msr & MS_RING_TEDGE)
		strcat(txt, "dRI*");
	if (msr & MS_DCD_DELTA)
		strcat(txt, "dDCD*");
	if (msr & MS_CTS_PRESENT)
		strcat(txt, "CTS*");
	if (msr & MS_DSR_PRESENT)
		strcat(txt, "DSR*");
	if (msr & MS_RING_PRESENT)
		strcat(txt, "RING*");
	if (msr & MS_DCD_PRESENT)
		strcat(txt, "DCD*");
	return (txt);

}							 /* end of msr_text */

/*+-------------------------------------------------------------------------
	mcr_text(mcr)
--------------------------------------------------------------------------*/
char *
mcr_text(mcr)
uchar mcr;
{
	static char txt[32];

	txt[0] = '*';
	txt[1] = 0;
	if (!mcr)
	{
		strcat(txt, "NULL*");
		return (txt);
	}
	if (mcr & MC_SET_DTR)
		strcat(txt, "DTR*");
	if (mcr & MC_SET_RTS)
		strcat(txt, "RTS*");
	if (mcr & MC_SET_OUT1)
		strcat(txt, "OUT1*");
	if (mcr & MC_SET_OUT2)
		strcat(txt, "OUT2*");
	if (mcr & MC_SET_LOOPBACK)
		strcat(txt, "LOOPBACK*");
	return (txt);

}							 /* end of mcr_text */

/*+-------------------------------------------------------------------------
	lcr_text(lcr)
--------------------------------------------------------------------------*/
char *
lcr_text(lcr)
uchar lcr;
{
	static char txt[64];

	sprintf(txt, "*%ddb*", (lcr & LC_WORDLEN_MASK) + 5);
	strcat(txt, (lcr & LC_STOPBITS_LONG) ? "2" : "1");
	strcat(txt, "sb*");
	if (lcr & LC_ENABLE_PARITY)
	{
		strcat(txt, "PARITY*");
		if (lcr & LC_STICK_PARITY)
			strcat(txt, (lcr & LC_EVEN_PARITY) ? "MARK*" : "SPACE*");
		else
			strcat(txt, (lcr & LC_EVEN_PARITY) ? "EVEN*" : "ODD*");
	}
	else
		strcat(txt, "NOPAR*");
	if (lcr & LC_SET_BREAK_LEVEL)
		strcat(txt, "SETBREAK*");
	if (lcr & LC_ENABLE_DIVISOR)
		strcat(txt, "ENABDIV*");
	return (txt);

}							 /* end of lcr_text */

/*+-------------------------------------------------------------------------
	ier_text(ier)
--------------------------------------------------------------------------*/
char *
ier_text(ier)
uchar ier;
{
	static char txt[32];

	txt[0] = '*';
	txt[1] = 0;
	if (!ier)
	{
		strcat(txt, "NULL*");
		return (txt);
	}
	if (ier & IE_RECV_DATA_AVAILABLE)
		strcat(txt, "RDAV*");
	if (ier & IE_XMIT_HOLDING_BUFFER_EMPTY)
		strcat(txt, "TBMT*");
	if (ier & IE_LINE_STATUS)
		strcat(txt, "LS*");
	if (ier & IE_MODEM_STATUS)
		strcat(txt, "MS*");
	return (txt);

}							 /* end of ier_text */

/*+-------------------------------------------------------------------------
	display_fasi(fip)
--------------------------------------------------------------------------*/
void
display_fasi(fip)
struct fas_info *fip;
{

	pprintf("base address: %04x irq=%u ", fip->port_0.addr, fip->vec);
	pputs("device is ");
	if (fip->device_flags.s & DF_DEVICE_IS_NS16550A)
		pputs("16550\n");
	else if (fip->device_flags.s & DF_DEVICE_IS_I82510)
		pputs("82510\n");
	else
		pputs("16450\n");
	pprintf("MSR=%s   ", msr_text(fip->msr));
	pprintf("MCR=%s\n", mcr_text(fip->mcr));
	pprintf("LCR=%s   ", lcr_text(fip->lcr));
	pprintf("IER=%s\n", ier_text(fip->ier));
	pprintf("recv ring cnt=%u  ", fip->recv_ring_cnt);
	pprintf("xmit ring cnt=%u  ", fip->xmit_ring_cnt);
	pprintf("xmit fifo size=%u\n", fip->xmit_fifo_size);
	pprintf("characters received    =%12lu\n", fip->characters_received);
	pprintf("characters transmitted =%12lu\n", fip->characters_transmitted);
	pprintf("modem status events    =%12lu\n", fip->modem_status_events);
	pprintf("overrun errors=%lu  ", fip->overrun_errors);
	pprintf("framing errors=%lu  ", fip->framing_errors);
	pprintf("parity errors=%lu\n", fip->parity_errors);
	pprintf("rings detected=%lu  ", fip->rings_detected);
	pprintf("breaks detected=%lu\n", fip->breaks_detected);
	pprintf("xmtr flow off XON/XOFF=%lu  RTS/CTS=%lu\n",
		fip->xmtr_sw_flow_count, fip->xmtr_hw_flow_count);
	pprintf("rcvr flow off XON/XOFF=%lu  RTS/CTS=%lu\n",
		fip->rcvr_sw_flow_count, fip->rcvr_hw_flow_count);

}							 /* end of display_fasi */

/*+-------------------------------------------------------------------------
	fasi_msr() - return modem status register contents
--------------------------------------------------------------------------*/
uchar
fasi_msr()
{
	UINT32 ltmp = 0;

	return ((uchar) ioctl(shm->Liofd, FASIC_MCR, (char *)&ltmp));
	return (ltmp);
}							 /* end of fasi_msr */

/*+-------------------------------------------------------------------------
	fasi_line_errors() - return UART error count
--------------------------------------------------------------------------*/
UINT32
fasi_line_errors()
{
	struct fas_info finfo, *fip = &finfo;

	memset((char *)fip, 0, sizeof(*fip));

	if ((ioctl(shm->Liofd, FASIC_SIP, (char *)fip)) < 0)
		return (0);
	return (fip->parity_errors + fip->framing_errors + fip->overrun_errors);

}							 /* end of fasi_line_errors */

/*+-------------------------------------------------------------------------
	fasi_rings_detected() - return number of RI trailing edges
--------------------------------------------------------------------------*/
UINT32
fasi_rings_detected()
{
	struct fas_info finfo, *fip = &finfo;

	memset((char *)fip, 0, sizeof(*fip));

	if ((ioctl(shm->Liofd, FASIC_SIP, (char *)fip)) < 0)
		return (0);
	return (fip->rings_detected);

}							 /* end of fasi_rings_detected */

/*+-------------------------------------------------------------------------
	fasi_breaks_detected() - return number of BREAKs detected
--------------------------------------------------------------------------*/
UINT32
fasi_breaks_detected()
{
	struct fas_info finfo, *fip = &finfo;

	memset((char *)fip, 0, sizeof(*fip));

	if ((ioctl(shm->Liofd, FASIC_SIP, (char *)fip)) < 0)
		return (0);
	return (fip->breaks_detected);

}							 /* end of fasi_breaks_detected */

/*+-------------------------------------------------------------------------
	pcmd_fasi(param)
fasi [-switches] <str-cmd>>

where <str-cmd> is 'd[isplay]'
                or 'r[eset]'

fasi 'd'
fasi 'r'
--------------------------------------------------------------------------*/
int
pcmd_fasi(param)
ESD *param;
{
	int erc;
	char switches[8];
	ESD *tesd = (ESD *) 0;
	struct fas_info finfo, *fip = &finfo;
	char ident_str[128];

	memset((char *)fip, 0, sizeof(*fip));

	get_switches(param, switches, sizeof(switches));
	if (!(tesd = esdalloc(64)))
		return (eNoMemory);
	if (!(erc = gstr(param, tesd, 1)))
	{
		skip_cmd_break(tesd);
		switch (to_lower(*(tesd->pb + tesd->index)))
		{
			case 'd':		 /* display */
				if ((ioctl(shm->Liofd, FASIC_SIP, (char *)fip)) < 0)
				{
					pperror("ioctl FASIC_SIP");
					erc = eFATAL_ALREADY;
				}
				else
					display_fasi(fip);
				if ((ioctl(shm->Liofd, FASIC_DVR_IDENT, ident_str)) < 0)
				{
					pperror("ioctl FASIC_DVR_IDENT");
					erc = eFATAL_ALREADY;
				}
				else
					pprintf("driver:  '%s'\n", ident_str);
				if ((ioctl(shm->Liofd, FASIC_SPACE_IDENT, ident_str)) < 0)
				{
					pperror("ioctl FASIC_SPACE_IDENT");
					erc = eFATAL_ALREADY;
				}
				else
					pprintf("space.c: '%s'\n", ident_str);
				break;

			case 'r':		 /* reset */
				if ((ioctl(shm->Liofd, FASIC_RESET_STAT, (char *)0)) < 0)
				{
					pperror("ioctl FASIC_RESET_STAT");
					erc = eFATAL_ALREADY;
				}
				else if (proc_trace)
					pputs("statistics reset\n");
				break;

			default:
				pputs("invalid subcommand '");
				pputs(tesd->pb);
				pputs("'\n");
				erc = eFATAL_ALREADY;
				break;
		}
	}

	if (tesd)
		esdfree(tesd);
	return (erc);

}							 /* end of pcmd_fasi */

/*+-------------------------------------------------------------------------
	icmd_fasi(narg,arg)
--------------------------------------------------------------------------*/
void
icmd_fasi(narg, arg)
int narg;
char **arg;
{
	struct fas_info finfo, *fip = &finfo;
	char ident_str[128];

	memset((char *)fip, 0, sizeof(*fip));

	if ((narg > 1) && (to_lower(*arg[1]) == 'r'))
	{
		if ((ioctl(shm->Liofd, FASIC_RESET_STAT, (char *)0)) < 0)
		{
			pperror("   ioctl FASIC_RESET_STAT");
			return;
		}
		ff(se, "  fasi statistics reset\r\n");
	}
	else
	{
		if ((ioctl(shm->Liofd, FASIC_SIP, (char *)fip)) < 0)
		{
			pperror("   ioctl FASIC_SIP");
			return;
		}
		ff(se, "\r\n");
		display_fasi(fip);
		if ((ioctl(shm->Liofd, FASIC_DVR_IDENT, ident_str)) < 0)
			pperror("ioctl FASIC_DVR_IDENT");
		else
			pprintf("driver:  '%s'\n", ident_str);
		if ((ioctl(shm->Liofd, FASIC_SPACE_IDENT, ident_str)) < 0)
			pperror("ioctl FASIC_SPACE_IDENT");
		else
			pprintf("space.c: '%s'\n", ident_str);
	}

}							 /* end of icmd_fasi */

#endif /* FASI */

/* vi: set tabstop=4 shiftwidth=4: */
/* end of fasiintf.c */
