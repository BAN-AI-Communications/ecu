
/* FAS Final Async Solution driver for 286/386 versions of system V UNIX */

/* FAS was developed by
Uwe Doering             INET : gemini@geminix.in-berlin.de
Billstedter Pfad 17 b   UUCP : ...!unido!fub!geminix.in-berlin.de!gemini
1000 Berlin 20
Germany
*/
/*+:EDITS:*/
/*:01-24-1997-02:37-wht@yuriatin-SOURCE RELEASE 4.00 */
/*:09-11-1996-20:00-wht@yuriatin-3.48-major telnet,curses,structural overhaul */
/*:11-23-1995-11:20-wht@kepler-source control 3.37 for tsx-11 */
/*:11-14-1995-10:23-wht@kepler-3.37.80-source control point: SOCKETS */
/*:05-04-1994-04:39-wht@n4hgf-ECU release 3.30 */
/*:09-10-1992-13:59-wht@n4hgf-ECU release 3.20 */
/*:08-22-1992-15:38-wht@n4hgf-ECU release 3.20 BETA */
/*:07-25-1991-12:57-wht@n4hgf-ECU release 3.10 */
/*:06-04-1991-19:41-wht@n4hgf-add FASIC_SIP_CHANGE */
/*:02-05-1991-12:13-wht@n4hgf-apply 2.08b2->2.08.0 diffs */
/*:01-20-1991-05:01-wht@n4hgf-changed buffer sizes */

#if defined(FASI)
char *fasi_driver_ident = "FAS/i 2.08.01";

#endif /* FASI */

#if !defined (M_I286) && !defined(__STDC__)
#ident	"@(#)fas.c	2.08"
#endif

/* Note: This source code was quite heavily optimized for speed. You
         may wonder that register variables aren't used everywhere.
         This is because there is an overhead in memory accesses
         when using register variables. As you may know data accesses
         usually need much more wait states on the memory bus than
         code accesses (because of page or cache misses). Therefor,
         saving some data accesses has higher priority than saving
         code accesses.

         You may also note some not very elegant constructions that
         may be intentional because they are faster. If you want to
         make style improvements you should check the assembler output
         whether this wouldn't slow things down.

         Decisions for speed optimization were based on assembler
         listings produced by the standard UNIX V 3.X/386 C compiler.
*/

#include <sys/param.h>
#include <sys/types.h>
#include <sys/signal.h>
#include <sys/buf.h>
#include <sys/dir.h>
#if defined (XENIX)
#include <sys/page.h>
#include <sys/seg.h>
#endif
#include <sys/user.h>
#include <sys/errno.h>
#include <sys/tty.h>
#include <sys/conf.h>
#include <sys/sysinfo.h>
#include <sys/file.h>
#if !defined (XENIX) && !defined(CBAUD)
#include <sys/termio.h>
#endif
#include <sys/ioctl.h>
#if !defined(FASI)
#include <macros.h>
#endif
#if defined (HAVE_VPIX)
#if !defined (XENIX)
#include <sys/tss.h>
#include <sys/immu.h>
#include <sys/region.h>
#endif
#include <sys/proc.h>
#include <sys/v86.h>
#endif

#if defined (XENIX)
#include "fas.h"
#else
#include <local/fas.h>
#if !defined (NO_ASM)
#include <sys/inline.h>
#endif
#endif

#if defined (SCO) || defined (XENIX)
#define asyputchar sioputchar
#define asygetchar siogetchar
#endif

#if defined (XENIX) || defined (NO_ASM)
#define intr_disable()	old_level = SPLINT ()
#define intr_restore()	(void) splx (old_level)
#define REGVAR
#else
/* This is a terrible ugly kludge to speed up the `inb' and `outb'
   functions. I.e., originally, the `outb' inline function had an
   overhead of four data memory accesses for parameter passing. This
   parameter passing actually consumed more clock cycles than the
   assembler `outb' command itself. Although this solution can't
   prevent unnessessary register moves it limits them at least to
   register to register moves that are much faster. You need a
   line like the following in the declaration part of every
   function that uses `inb' or `outb' calls:

	REGVAR;

   This hack should work with every compiler that knows about the
   UNIX V 3.X/386 standard compiler's inline assembler directives.
*/

asm void
loadal(val)
{
	%reg val;
	movl val, %eax
	%   mem val;
	movb val, %al
}

asm void
loaddx(val)
{
	%reg val;
	movl val, %edx
	%   mem val;
	movw val, %dx
}

asm int
outbyte()
{
	outb(%dx)
}

asm int
inbyte()
{
	xorl % eax, %eax
	inb(%dx)
}

/* The port parameter of the `outb' macro must be one of the predefined
   port macros from `fas.h' or a simple UINT variable (no indirection
   is allowed). Additionally, `fip' must be a register variable in the
   functions where `outb' is used. This prevents the destruction of the
   `eax' CPU register while loading the `edx' register with the port
   address. This is highly compiler implementation specific.
*/
#define outb(port,val) (regvar = (val), loadal (regvar), regvar = (port), loaddx (regvar), outbyte ())

#define inb(port) (regvar = (port), loaddx (regvar), inbyte ())

#define REGVAR register UINT	regvar

/* This function inserts the address optimization assembler pseudo-op
   wherever called.
*/

asm void
optim()
{
	.optim
}

/* This dummy function has nothing to do but to call optim so that
   the `.optim' assembler pseudo-op will be included in the assembler
   file. This must be the first of all functions.
*/

#if defined (OPTIM)			 /* Define for uPort, ISC doesn't know about */
static void					 /* `.optim', but has turned on optimization
							  * by */
dummy()						 /* default, so we don't need it there anyway. */
{
	optim();
}
#endif
#endif /* XENIX || NO_ASM */

/* functions provided by this driver */
int fasinit();
int fasopen();
int fasclose();
int fasread();
int faswrite();
int fasioctl();
int fasintr();

#if defined (NEED_PUT_GETCHAR)
int asyputchar();
int asygetchar();

#endif
#if defined (NEED_INIT8250)
int init8250();

#endif
static int fas_proc();
static void fas_param();
static void fas_fproc();
static void fas_mproc();
static UINT fas_rproc();
static void fas_xproc();
static void fas_event();

#if defined (HAVE_VPIX)
static int fas_vpix_sr();

#endif
static void fas_rxfer();
static void fas_xxfer();
static void fas_ihlw_check();
static void fas_hdx_check();
static void fas_hangup();
static void fas_timeout();
static void fas_cmd();
static void fas_open_device();
static void fas_close_device();
static UINT fas_make_ctl_val();
static int fas_test_device();

/* external functions used by this driver */
extern int ttinit();
extern int ttiocom();
extern int ttyflush();
extern int SPLINT();
extern int SPLWRK();
extern int splx();
extern int sleep();
extern int wakeup();
extern void longjmp();
extern int signal();
extern int timeout();
extern int untimeout();
extern int printf();

#if defined (SCO) || defined (XENIX)
extern int printcfg();

#endif
#if defined (HAVE_VPIX)
extern int fubyte();
extern int subyte();
extern int v86setint();

#endif
#if defined (XENIX)
extern int inb();
extern int outb();

#endif

/* external data objects used by this driver */
extern int tthiwat[];

/* the following stuff is defined in space.c */
extern UINT fas_physical_units;
extern UINT32 fas_port[];
extern UINT fas_vec[];
extern UINT fas_init_seq[];
extern UINT fas_mcb[];
extern UINT32 fas_modem[];
extern UINT32 fas_flow[];
extern UINT fas_ctl_port[];
extern UINT fas_ctl_val[];
extern UINT fas_int_ack_port[];
extern UINT fas_int_ack[];
extern struct fas_info fas_info[];
extern struct tty fas_tty[];
extern struct fas_info *fas_info_ptr[];
extern struct tty *fas_tty_ptr[];

/* end of space.c references */

#if defined(FASI)
int fasiintr_entries = 0;
extern char *fasi_space_ident;

#endif /* FASI */

/* fas_is_initted
   Flag to indicate that we have been thru init.
   This is realy only necessary for systems that use asyputchar
   and asygetchar but it doesn't hurt to have it anyway.
*/
static int fas_is_initted = FALSE;

/* event_scheduled
   Flag to indicate that the event handler has been scheduled
   via the timeout() function.
*/
static int event_scheduled = FALSE;

/* array of pointers to the first fas_info structure for each
   interrupt vector
*/
static struct fas_info *fas_first_int_user[NUM_INT_VECTORS];

/* the values for the various baud rates */
static UINT fas_speeds[CBAUD + 1] =
{1, BAUD_BASE / 50,
	BAUD_BASE / 75, BAUD_BASE / 110,
	(2 * BAUD_BASE + 134) / 269, BAUD_BASE / 150,
	BAUD_BASE / 200, BAUD_BASE / 300,
	BAUD_BASE / 600, BAUD_BASE / 1200,
	BAUD_BASE / 1800, BAUD_BASE / 2400,
	BAUD_BASE / 4800, BAUD_BASE / 9600,
	BAUD_BASE / 19200, BAUD_BASE / 38400
};

/* time for one character to completely leave the transmitter shift register */
static UINT fas_ctimes[CBAUD + 1] =
{1, HZ * 15 / 50 + 2,
	HZ * 15 / 75 + 2, HZ * 15 / 110 + 2,
	HZ * 30 / 269 + 2, HZ * 15 / 150 + 2,
	HZ * 15 / 200 + 2, HZ * 15 / 300 + 2,
	HZ * 15 / 600 + 2, HZ * 15 / 1200 + 2,
	HZ * 15 / 1800 + 2, HZ * 15 / 2400 + 2,
	HZ * 15 / 4800 + 2, HZ * 15 / 9600 + 2,
	HZ * 15 / 19200 + 2, HZ * 15 / 38400 + 2
};

/* dynamically adapt xmit buffer size to baud rate to prevent long buffer
   drains at low speeds
   These values are checked against boundaries and will be modified if
   necessary before use. Checking is done in fas_param (). Drain time
   is about 5 seconds with continuous character flow.
*/
static UINT fas_xbuf_size[CBAUD + 1] =
{1, 50 / 2,
	75 / 2, 110 / 2,
	269 / 4, 150 / 2,
	200 / 2, 300 / 2,
	600 / 2, 1200 / 2,
	1800 / 2, 2400 / 2,
	4800 / 2, 9600 / 2,
	19200 / 2, 38400 / 2
};

/* lookup table for minor device number -> open mode flags translation */
static UINT fas_open_modes[16] =
{
	OS_OPEN_FOR_DIALOUT | OS_FAKE_CARR_ON | OS_CLOCAL,
	OS_OPEN_FOR_DIALOUT | OS_FAKE_CARR_ON | OS_CLOCAL | OS_HWO_HANDSHAKE
	| OS_HWI_HANDSHAKE,
	OS_OPEN_FOR_DIALOUT | OS_FAKE_CARR_ON | OS_CLOCAL | OS_HWO_HANDSHAKE,
	OS_OPEN_FOR_DIALOUT | OS_FAKE_CARR_ON | OS_CLOCAL | OS_HWO_HANDSHAKE
	| OS_HDX_HANDSHAKE,
	OS_OPEN_FOR_DIALOUT | OS_FAKE_CARR_ON,
	OS_OPEN_FOR_DIALOUT | OS_FAKE_CARR_ON | OS_HWO_HANDSHAKE
	| OS_HWI_HANDSHAKE,
	OS_OPEN_FOR_DIALOUT | OS_FAKE_CARR_ON | OS_HWO_HANDSHAKE,
	OS_OPEN_FOR_DIALOUT | OS_FAKE_CARR_ON | OS_HWO_HANDSHAKE
	| OS_HDX_HANDSHAKE,
	OS_OPEN_FOR_GETTY | OS_WAIT_OPEN | OS_NO_DIALOUT,
	OS_OPEN_FOR_GETTY | OS_WAIT_OPEN | OS_NO_DIALOUT | OS_HWO_HANDSHAKE
	| OS_HWI_HANDSHAKE,
	OS_OPEN_FOR_GETTY | OS_WAIT_OPEN | OS_NO_DIALOUT | OS_HWO_HANDSHAKE,
	OS_OPEN_FOR_GETTY | OS_WAIT_OPEN | OS_NO_DIALOUT | OS_HWO_HANDSHAKE
	| OS_HDX_HANDSHAKE,
	OS_OPEN_FOR_GETTY | OS_WAIT_OPEN,
	OS_OPEN_FOR_GETTY | OS_WAIT_OPEN | OS_HWO_HANDSHAKE
	| OS_HWI_HANDSHAKE,
	OS_OPEN_FOR_GETTY | OS_WAIT_OPEN | OS_HWO_HANDSHAKE,
	OS_OPEN_FOR_GETTY | OS_WAIT_OPEN | OS_HWO_HANDSHAKE
	| OS_HDX_HANDSHAKE
};

/* The following defines are used to access multiplexed ports. */
#define GET_PORT(port,num) \
	((fip->device_flags.i & DF_CTL_EVERY)\
			? (port)\
			: (port) + (num))

#define fas_first_ctl(fip,port) \
	((void) (((fip)->device_flags.i & DF_CTL_FIRST)\
			? outb (CTL_PORT, (port).p.ctl)\
			: 0))

#define fas_ctl(fip,port) \
	((void) (((fip)->device_flags.i & (DF_CTL_FIRST | DF_CTL_EVERY))\
			? outb (CTL_PORT, (port).p.ctl)\
			: 0))

#define fas_first_outb(fip,port,val) \
	((void) (((fip)->device_flags.i & (DF_CTL_FIRST | DF_CTL_EVERY))\
			? outb (CTL_PORT, (port).p.ctl)\
			: 0),\
		(void) outb ((port).addr, (val)))

#define fas_outb(fip,port,val) \
	((void) (((fip)->device_flags.i & DF_CTL_EVERY)\
			? outb (CTL_PORT, (port).p.ctl)\
			: 0),\
		(void) outb ((port).addr, (val)))

#define fas_first_inb(fip,port) \
	((void) (((fip)->device_flags.i & (DF_CTL_FIRST | DF_CTL_EVERY))\
			? outb (CTL_PORT, (port).p.ctl)\
			: 0),\
		inb ((port).addr))

#define fas_inb(fip,port) \
	((void) (((fip)->device_flags.i & DF_CTL_EVERY)\
			? outb (CTL_PORT, (port).p.ctl)\
			: 0),\
		inb ((port).addr))

/* The following defines are used to take apart the minor device numbers. */
#define GET_UNIT(dev)		((dev) & 0x0f)
#define GET_OPEN_MODE(dev)	(fas_open_modes [((dev) >> 4) & 0x0f])

/* lock device against concurrent use */
#define get_device_lock(fip,prio) \
{\
	/* sleep while device is used by an other process */\
	while ((fip)->device_flags.i & DF_DEVICE_LOCKED)\
		(void) sleep ((caddr_t) &(fip)->device_flags.i, (prio));\
	(fip)->device_flags.s |= DF_DEVICE_LOCKED;\
}

/* release device */
#define release_device_lock(fip) \
{\
	(fip)->device_flags.s &= ~DF_DEVICE_LOCKED;\
	/* wakeup the process that may wait for this device */\
	(void) wakeup ((caddr_t) &(fip)->device_flags.i);\
}

/* schedule event */
#define event_sched(fip,event) \
{\
	(fip)->event_flags.s |= (event);\
	if (!event_scheduled)\
	{\
		event_scheduled = TRUE;\
		(void) timeout (fas_event, (void *) NULL,\
				(EVENT_TIME) * (HZ) / 1000);\
	}\
}

/* fasinit
   This routine checks for the presense of the devices in the fas_port
   array and if the device is present tests and initializes it.
   During the initialization if the device is determined to be an
   NS16550A chip the DF_DEVICE_IS_NS16550A flag is set and the FIFOs will
   be used. If the device is an i82510 chip the DF_DEVICE_IS_I82510 flag
   is set and the device will be handled accordingly.
*/

int
fasinit()
{
	register struct fas_info *fip;
	register UINT unit;
	UINT logical_units, port, *seq_ptr;
	char port_stat[MAX_UNITS + 1];

	REGVAR;

	if (fas_is_initted)
		return (0);

	fas_is_initted = TRUE;

	/* execute the init sequence for the serial card */
	for (seq_ptr = fas_init_seq; *seq_ptr; seq_ptr++)
	{
		port = *seq_ptr;
		seq_ptr++;
		if (*seq_ptr & READ_PORT)
			(void)inb(port);
		else
			(void)outb(port, *seq_ptr);
	}

	/* setup the list of pointers to the tty structures */
	for (unit = 0, logical_units = fas_physical_units * 2;
		unit < logical_units; unit++)
		fas_tty_ptr[unit] = &fas_tty[unit];

	/* setup and initialize all serial ports */
	for (unit = 0; unit < fas_physical_units; unit++)
	{
		fas_info_ptr[unit] = fip = &fas_info[unit];
		port_stat[unit] = '-';
		if (port = (UINT) ((UINT16) (fas_port[unit])))
		{
			/* check the int vector */
			if (fas_vec[unit] >= NUM_INT_VECTORS)
			{
				port_stat[unit] = '>';
				continue;
			}

			/* init all of its ports */
			if (fas_ctl_port[unit])
			{
				fip->ctl_port = fas_ctl_port[unit];

				if (fas_ctl_val[unit] & 0xff00)
					fip->device_flags.s |= DF_CTL_EVERY;
				else
					fip->device_flags.s |= DF_CTL_FIRST;
			}

			fip->port_0.p.addr = GET_PORT(port, 0);
			fip->port_1.p.addr = GET_PORT(port, 1);
			fip->port_2.p.addr = GET_PORT(port, 2);
			fip->port_3.p.addr = GET_PORT(port, 3);
			fip->port_4.p.addr = GET_PORT(port, 4);
			fip->port_5.p.addr = GET_PORT(port, 5);
			fip->port_6.p.addr = GET_PORT(port, 6);
			fip->port_0.p.ctl = fas_make_ctl_val(fip, unit, 0);
			fip->port_1.p.ctl = fas_make_ctl_val(fip, unit, 1);
			fip->port_2.p.ctl = fas_make_ctl_val(fip, unit, 2);
			fip->port_3.p.ctl = fas_make_ctl_val(fip, unit, 3);
			fip->port_4.p.ctl = fas_make_ctl_val(fip, unit, 4);
			fip->port_5.p.ctl = fas_make_ctl_val(fip, unit, 5);
			fip->port_6.p.ctl = fas_make_ctl_val(fip, unit, 6);
			fip->vec = fas_vec[unit];
			fip->modem.l = fas_modem[unit];
			fip->flow.l = fas_flow[unit];

			/* mask off invalid bits */
			fip->modem.m.di &= MC_ANY_CONTROL;
			fip->modem.m.eo &= MC_ANY_CONTROL;
			fip->modem.m.ei &= MC_ANY_CONTROL;
			fip->modem.m.ca &= MS_ANY_PRESENT;
			fip->flow.m.ic &= MC_ANY_CONTROL;
			fip->flow.m.oc &= MS_ANY_PRESENT;
			fip->flow.m.oe &= MS_ANY_PRESENT;
			fip->flow.m.hc &= MC_ANY_CONTROL;

			fip->recv_ring_put_ptr = fip->recv_buffer;
			fip->recv_ring_take_ptr = fip->recv_buffer;
			fip->xmit_ring_put_ptr = fip->xmit_buffer;
			fip->xmit_ring_take_ptr = fip->xmit_buffer;
			fip->xmit_fifo_size = 1;

			fip->ier = IE_NONE;	/* disable all ints */
			fas_first_outb(fip, INT_ENABLE_PORT, fip->ier);

			/* is there a serial chip ? */
			if (fas_inb(fip, INT_ENABLE_PORT) != fip->ier)
			{
				port_stat[unit] = '?';
				continue;	 /* a hardware error */
			}

			/* test the chip thoroughly */
			if ((port_stat[unit] = (fas_test_device(fip) + '0'))
				!= '0')
			{
				continue;	 /* a hardware error */
			}

			fip->lcr = 0;
			fas_outb(fip, LINE_CTL_PORT, fip->lcr);
			fip->mcr = fas_mcb[unit] | fip->modem.m.di;
			fas_outb(fip, MDM_CTL_PORT, fip->mcr);

			port_stat[unit] = '*';

			/* let's see if it's an NS16550A */
			fas_outb(fip, NS_FIFO_CTL_PORT, NS_FIFO_INIT_CMD);
			if (!(~fas_inb(fip, INT_ID_PORT) & II_NS_FIFO_ENABLED))
			{
				fip->device_flags.s |= DF_DEVICE_IS_NS16550A;
				fip->xmit_fifo_size = OUTPUT_NS_FIFO_SIZE;
				port_stat[unit] = 'F';
				fas_outb(fip, NS_FIFO_CTL_PORT, NS_FIFO_CLEAR_CMD);
			}
			else
			{
				fas_outb(fip, NS_FIFO_CTL_PORT, NS_FIFO_CLEAR_CMD);
				/* or is it an i82510 ? */
				fas_outb(fip, I_BANK_PORT, I_BANK_2);
				if (!(~fas_inb(fip, I_BANK_PORT) & I_BANK_2))
				{
					fip->device_flags.s |= DF_DEVICE_IS_I82510;
					fip->xmit_fifo_size = OUTPUT_I_FIFO_SIZE;
					port_stat[unit] = 'f';
					fas_outb(fip, I_BANK_PORT, I_BANK_1);
					fas_outb(fip, I_TCM_PORT, I_FIFO_CLR_XMIT);
					fas_outb(fip, I_RCM_PORT, I_FIFO_CLR_RECV);
				}
				fas_outb(fip, I_BANK_PORT, I_BANK_0);
			}

			/* disable FIFOs if requested in space.c */
			if ((fas_port[unit] & NO_FIFO) && (fip->device_flags.i
					& (DF_DEVICE_IS_NS16550A
						| DF_DEVICE_IS_I82510)))
			{
				fip->device_flags.s &= ~(DF_DEVICE_IS_NS16550A
					| DF_DEVICE_IS_I82510);
				fip->xmit_fifo_size = 1;
				port_stat[unit] = '+';
			}

			/* clear potential interrupts */
			(void)fas_inb(fip, MDM_STATUS_PORT);
			(void)fas_inb(fip, RCV_DATA_PORT);
			(void)fas_inb(fip, RCV_DATA_PORT);
			(void)fas_inb(fip, LINE_STATUS_PORT);
			(void)fas_inb(fip, INT_ID_PORT);
			if (port = fas_int_ack_port[fip->vec])
				(void)outb(port, fas_int_ack[fip->vec]);

			/* show that it is present and configured */
			fip->device_flags.s |= DF_DEVICE_CONFIGURED;
		}
	}

#if defined (NEED_PUT_GETCHAR)
	fip = &fas_info[0];
	fip->mcr &= ~fip->modem.m.di;
	fip->mcr |= INITIAL_MDM_CONTROL;
	fas_first_outb(fip, MDM_CTL_PORT, fip->mcr);

	fip->lcr = INITIAL_LINE_CONTROL;
	fas_outb(fip, LINE_CTL_PORT, fip->lcr | LC_ENABLE_DIVISOR);
	fas_outb(fip, DIVISOR_LSB_PORT, INITIAL_BAUD_RATE);
	fas_outb(fip, DIVISOR_MSB_PORT, (INITIAL_BAUD_RATE) >> 8);
	fas_outb(fip, LINE_CTL_PORT, fip->lcr);
#endif

#if defined (SCO) || defined (XENIX)
	for (unit = 0; unit < fas_physical_units; unit++)
		(void)printcfg("fas", (UINT) ((UINT16) (fas_port[unit])), 7,
			fas_vec[unit], -1,
#if defined (FASI)
			"unit=%d type=%c FAS/i 2.08.01",
#else
			"unit=%d type=%c release=2.08.0",
#endif /* FASI */
			unit, port_stat[unit]);
#else
	port_stat[unit] = '\0';
	(void)printf("\nFAS 2.08.0 async driver: Unit 0-%d init state is [%s]\n\n",
		unit - 1,
		port_stat);
#endif
	return (0);
}

/* Open a tty line. This function is called for every open, as opposed
   to the fasclose function which is called only with the last close.
*/
int
fasopen(dev, flag)
int dev;
int flag;
{
	register struct fas_info *fip;
	register struct tty *ttyp;
	register UINT open_mode;
	UINT physical_unit;
	int old_level;

	physical_unit = GET_UNIT(dev);

	/* check for valid port number */
	if (physical_unit >= fas_physical_units)
	{
		u.u_error = ENXIO;
		return (-1);
	}

	fip = fas_info_ptr[physical_unit];

	/* was the port present at init time ? */
	if (!(fip->device_flags.i & DF_DEVICE_CONFIGURED))
	{
		u.u_error = ENXIO;
		return (-1);
	}

	open_mode = GET_OPEN_MODE(dev);

	old_level = SPLINT();
	get_device_lock(fip, TTIPRI);

	/*
	 * If this is a getty open, the device is already open for dialout and
	 * the FNDELAY flag is not set, wait until device is closed.
	 */
	while ((open_mode & OS_OPEN_FOR_GETTY)
		&& (fip->o_state & OS_OPEN_FOR_DIALOUT)
		&& !(flag & FNDELAY))
	{
		release_device_lock(fip);
		(void)sleep((caddr_t) & fip->o_state, TTIPRI);
		get_device_lock(fip, TTIPRI);
	}

	/*
	 * If the device is already open and another open uses a different
	 * open mode or if a getty open waits for carrier and doesn't allow
	 * parallel dialout opens, return with EBUSY error.
	 */
	if ((fip->o_state & ((open_mode & OS_OPEN_FOR_GETTY)
				? (OS_OPEN_STATES | OS_WAIT_OPEN)
				: (OS_OPEN_STATES | OS_NO_DIALOUT)))
		&& ((flag & FEXCL)
			|| ((open_mode ^ fip->o_state) & (u.u_uid
					? OS_TEST_MASK
					: OS_SU_TEST_MASK))))
	{
		u.u_error = EBUSY;
		release_device_lock(fip);
		(void)splx(old_level);
		return (-1);
	}

	/* disable subsequent opens */
	if (flag & FEXCL)
		open_mode |= OS_EXCLUSIVE_OPEN_1;

	/* set up pointer to tty structure */
	ttyp = (open_mode & OS_OPEN_FOR_GETTY)
		? fas_tty_ptr[physical_unit + fas_physical_units]
		: fas_tty_ptr[physical_unit];

	/* things to do on first open only */
	if (!(fip->o_state & ((open_mode & OS_OPEN_FOR_GETTY)
				? (OS_OPEN_STATES | OS_WAIT_OPEN)
				: OS_OPEN_STATES)))
	{
		/* init data structures */
		fip->tty = ttyp;
		(void)ttinit(ttyp);
		ttyp->t_proc = fas_proc;
		fip->po_state = fip->o_state;
		fip->o_state = open_mode & ~OS_OPEN_STATES;
#if defined (HAVE_VPIX)
		/* initialize VP/ix related variables */
		fip->v86_proc = (v86_t *) NULL;
		fip->v86_intmask = 0;
		fip->v86_ss.ss_start = CSTART;
		fip->v86_ss.ss_stop = CSTOP;
#endif
		fas_open_device(fip);/* open physical device */
		fas_param(fip, HARD_INIT);	/* set up port registers */

		/* allow pending tty interrupts */
		(void)SPLWRK();
		(void)SPLINT();
	}

	/*
	 * If getty open and the FNDELAY flag is not set, block and wait for
	 * carrier if device not yet open.
	 */
	if ((open_mode & OS_OPEN_FOR_GETTY) && !(flag & FNDELAY))
	{
		/* sleep while open for dialout or no carrier */
		while ((fip->o_state & OS_OPEN_FOR_DIALOUT)
			|| !(ttyp->t_state & (ISOPEN | CARR_ON)))
		{
			ttyp->t_state |= WOPEN;
			release_device_lock(fip);
			(void)sleep((caddr_t) & ttyp->t_canq, TTIPRI);
			get_device_lock(fip, TTIPRI);
		}
		ttyp->t_state &= ~WOPEN;
	}

	/* wakeup processes that are still sleeping in getty open */
	if (ttyp->t_state & WOPEN)
#if defined(FASI)
	{
#endif
		(void)wakeup((caddr_t) & ttyp->t_canq);
#if defined(FASI)
		(void)wakeup((caddr_t) & fip->device_flags.i);
	}
#endif

	/* we need to flush the receiver with the first open */
	if (!(fip->o_state & OS_OPEN_STATES))
		fas_cmd(fip, ttyp, T_RFLUSH);

	(*linesw[ttyp->t_line].l_open) (ttyp);

	/* set open type flags */
	fip->o_state = open_mode;

	release_device_lock(fip);
	(void)splx(old_level);
	return (0);
}

/* Close a tty line. This is only called if there is no other
   concurrent open left. A blocked getty open is not counted as
   a concurrent open because in this state it isn't really open.
*/
int
fasclose(dev)
int dev;
{
	register struct fas_info *fip;
	register struct tty *ttyp;
	UINT physical_unit;
	UINT open_mode;
	int old_level;
	void (*old_sigkill) ();

	physical_unit = GET_UNIT(dev);

	fip = fas_info_ptr[physical_unit];

	open_mode = GET_OPEN_MODE(dev);

	/* set up pointer to tty structure */
	ttyp = (open_mode & OS_OPEN_FOR_GETTY)
		? fas_tty_ptr[physical_unit + fas_physical_units]
		: fas_tty_ptr[physical_unit];

	old_level = SPLINT();
	get_device_lock(fip, TTIPRI);

	/* wait for output buffer drain only if device was open */
	if (ttyp->t_state & ISOPEN)
	{

		/*
		 * flush the output buffer immediately if the device has been shut
		 * down because of an error
		 */
		if (!(fip->device_flags.i & DF_DEVICE_CONFIGURED))
		{
			(void)ttyflush(ttyp, FWRITE);
		}
		/* wait for buffer drain and catch interrupts */
		while (ttyp->t_outq.c_cc || (ttyp->t_state & (BUSY | TIMEOUT)))
		{
			old_sigkill = u.u_signal[SIGKILL - 1];
			/* allow kill signal if close on exit */
			if (old_sigkill == SIG_IGN)
				u.u_signal[SIGKILL - 1] = SIG_DFL;
			ttyp->t_state |= TTIOW;
			if (sleep((caddr_t) & ttyp->t_oflag, TTOPRI | PCATCH))
			{
				/* caught signal */
				ttyp->t_state &= ~TTIOW;

				/*
				 * If close on exit, flush output buffer to allow
				 * completion of the fasclose() function. Otherwise, do
				 * the normal signal handling.
				 */
				if (old_sigkill == SIG_IGN)
					(void)ttyflush(ttyp, FWRITE);
				else
				{
					release_device_lock(fip);
					(void)splx(old_level);
					longjmp(u.u_qsav);
				}
			}
			if (old_sigkill == SIG_IGN)
				u.u_signal[SIGKILL - 1] = old_sigkill;
		}
	}

	(*linesw[ttyp->t_line].l_close) (ttyp);

	/* allow pending tty interrupts */
	(void)SPLWRK();
	(void)SPLINT();

	if (open_mode & OS_OPEN_FOR_GETTY)
	{
		/* not waiting any more */
		ttyp->t_state &= ~WOPEN;
		if (!(fip->o_state & OS_OPEN_FOR_DIALOUT))
		{
			fas_close_device(fip);
			fip->o_state = OS_DEVICE_CLOSED;
		}
		else
			fip->po_state = OS_DEVICE_CLOSED;
	}
	else
	{
		fas_close_device(fip);
		fip->o_state = OS_DEVICE_CLOSED;

		/*
		 * If there is a waiting getty open on this port, reopen the
		 * physical device.
		 */
		if (fip->po_state & OS_WAIT_OPEN)
		{

			/*
			 * get the getty version of the tty structure
			 */
			fip->tty = fas_tty_ptr[physical_unit
				+ fas_physical_units];
			fip->o_state = fip->po_state;
			fip->po_state = OS_DEVICE_CLOSED;
#if defined (HAVE_VPIX)
			/* initialize VP/ix related variables */
			fip->v86_proc = (v86_t *) NULL;
			fip->v86_intmask = 0;
			fip->v86_ss.ss_start = CSTART;
			fip->v86_ss.ss_stop = CSTOP;
#endif
			if (!(fip->device_flags.i & DF_DO_HANGUP))
			{
				fas_open_device(fip);
				/* set up port registers */
				fas_param(fip, HARD_INIT);
			}
		}
		(void)wakeup((caddr_t) & fip->o_state);
	}

	if (!(fip->device_flags.i & DF_DO_HANGUP))
		release_device_lock(fip);

#if defined(FASI)
	(void)wakeup((caddr_t) & fip->device_flags.i);
#endif
	(void)splx(old_level);
	return (0);
}

/* read characters from the input buffer */
int
fasread(dev)
int dev;
{
	register struct fas_info *fip;
	register struct tty *ttyp;
	int old_level;

	fip = fas_info_ptr[GET_UNIT(dev)];

	/* was the port present at init time ? */
	if (!(fip->device_flags.i & DF_DEVICE_CONFIGURED))
	{
		u.u_error = ENXIO;
		return (-1);
	}

	ttyp = fip->tty;

	(*linesw[ttyp->t_line].l_read) (ttyp);

	old_level = SPLINT();

	/* schedule character transfer to UNIX buffer */
	if (fip->recv_ring_cnt
#if defined (HAVE_VPIX)
		&& (((fip->iflag & DOSMODE)
				? MAX_VPIX_FILL - MIN_READ_CHUNK
				: MAX_UNIX_FILL - MIN_READ_CHUNK)
			>= ttyp->t_rawq.c_cc)
#else
		&& ((MAX_UNIX_FILL - MIN_READ_CHUNK) >= ttyp->t_rawq.c_cc)
#endif
		&& !(fip->flow_flags.i & FF_RXFER_STOPPED))
	{
		event_sched(fip, EF_DO_RXFER);
	}

#if defined(FASI)
	(void)wakeup((caddr_t) & fip->device_flags.i);
#endif
	(void)splx(old_level);
	return (0);
}

/* write characters to the output buffer */
int
faswrite(dev)
int dev;
{
	register struct fas_info *fip;
	register struct tty *ttyp;

	fip = fas_info_ptr[GET_UNIT(dev)];

	/* was the port present at init time ? */
	if (!(fip->device_flags.i & DF_DEVICE_CONFIGURED))
	{
		u.u_error = ENXIO;
		return (-1);
	}

	ttyp = fip->tty;

	(*linesw[ttyp->t_line].l_write) (ttyp);
	return (0);
}

/*+-------------------------------------------------------------------------
	strlen(str)
--------------------------------------------------------------------------*/
#if defined(FASI)
static int
strlen(str)
register char *str;
{
	register len = 0;

	while (*str++)
		len++;
	return (len);
}							 /* end of strlen */
#endif /* FASI */

/* process ioctl calls */
int
fasioctl(dev, cmd, arg3, arg4)
int dev;
int cmd;
union ioctl_arg arg3;
int arg4;
{
	register struct fas_info *fip;
	register struct tty *ttyp;
	int v86_cmd, v86_data;
	int old_level;

	REGVAR;

	fip = fas_info_ptr[GET_UNIT(dev)];

	/* was the port present at init time ? */
	if (!(fip->device_flags.i & DF_DEVICE_CONFIGURED))
	{
		u.u_error = ENXIO;
		return (-1);
	}

	ttyp = fip->tty;

	/* process ioctl commands */
	switch (cmd)
	{
#if defined (FASI)
		case FASIC_SIP_CHANGE:
			(void)sleep((caddr_t) & fip->device_flags.i, PZERO + 1);
		case FASIC_SIP:
			if (copyout((char *)fip, arg3.cparg, sizeof(*fip)))
			{
				u.u_error = EFAULT;
				return (-1);
			}
			return (fasiintr_entries);
		case FASIC_DVR_IDENT:
			if (copyout(fasi_driver_ident, arg3.cparg,
					strlen(fasi_driver_ident) + 1))
			{
				u.u_error = EFAULT;
				return (-1);
			}
			break;
		case FASIC_SPACE_IDENT:
			if (copyout(fasi_space_ident, arg3.cparg,
					strlen(fasi_space_ident) + 1))
			{
				u.u_error = EFAULT;
				return (-1);
			}
			break;
		case FASIC_MSR:
			return ((unsigned int)fip->msr);
		case FASIC_LCR:
			return ((unsigned int)fip->lcr);
		case FASIC_IER:
			return ((unsigned int)fip->ier);
		case FASIC_MCR:
			return ((unsigned int)fip->mcr);
		case FASIC_RESET_STAT:
			old_level = SPLINT();
			fip->characters_received = 0;
			fip->characters_transmitted = 0;
			fip->modem_status_events = 0;
			fip->overrun_errors = 0;
			fip->framing_errors = 0;
			fip->parity_errors = 0;
			fip->rings_detected = 0;
			fip->breaks_detected = 0;
			fip->xmtr_hw_flow_count = 0;
			fip->xmtr_sw_flow_count = 0;
			fip->rcvr_hw_flow_count = 0;
			fip->rcvr_sw_flow_count = 0;
			(void)splx(old_level);
			break;
#endif /* FASI */
#if defined (HAVE_VPIX)
		case AIOCINTTYPE:	 /* set pseudorupt type */
			switch (arg3.iarg)
			{
				case V86VI_KBD:
				case V86VI_SERIAL0:
				case V86VI_SERIAL1:
					intr_disable();
					fip->v86_intmask = arg3.iarg;
					intr_restore();
					break;

				default:
					intr_disable();
					fip->v86_intmask = V86VI_SERIAL0;
					intr_restore();
					break;
			}
			break;

		case AIOCDOSMODE:	 /* enable dos mode */
			if (!(fip->iflag & DOSMODE))
			{
				old_level = SPLINT();
				fip->v86_proc = u.u_procp->p_v86;
				if (!(fip->v86_intmask))
					fip->v86_intmask = V86VI_SERIAL0;
				ttyp->t_iflag |= DOSMODE;
				if (fip->v86_intmask != V86VI_KBD)
					ttyp->t_cflag |= CLOCAL;
				fas_param(fip, SOFT_INIT);
				(void)splx(old_level);
			}
			u.u_r.r_reg.r_val1 = 0;
			break;

		case AIOCNONDOSMODE:/* disable dos mode */
			if (fip->iflag & DOSMODE)
			{
				old_level = SPLINT();
				fip->v86_proc = (v86_t *) NULL;
				fip->v86_intmask = 0;
				ttyp->t_iflag &= ~DOSMODE;
				if (fip->flow_flags.i & FF_RXFER_STOPPED)
				{
					fip->flow_flags.s &= ~FF_RXFER_STOPPED;

					/*
					 * schedule character transfer to UNIX buffer
					 */
					if (fip->recv_ring_cnt)
						event_sched(fip, EF_DO_RXFER);
				}
				fip->lcr &= ~LC_SET_BREAK_LEVEL;
				fas_param(fip, HARD_INIT);
				(void)splx(old_level);
			}
			u.u_r.r_reg.r_val1 = 0;
			break;

		case AIOCSERIALOUT: /* setup port registers for dos */
			if ((fip->iflag & DOSMODE) && fip->v86_proc)
			{
				/* wait until output is done */
				old_level = SPLINT();
				while (ttyp->t_outq.c_cc
					|| (ttyp->t_state & (BUSY | TIMEOUT)))
				{
					ttyp->t_state |= TTIOW;
					(void)sleep((caddr_t) & ttyp->t_oflag,
						TTOPRI);
				}

				/*
				 * block transmitter and wait until it is empty
				 */
				fip->device_flags.s |= DF_XMIT_LOCKED;
				while (fip->device_flags.i & (DF_XMIT_BUSY
						| DF_XMIT_BREAK
						| DF_GUARD_TIMEOUT))
					(void)sleep((caddr_t) & fip->
						device_flags.i,
						PZERO - 1);
				(void)splx(old_level);

				/* get port write command */
				v86_cmd = fubyte(arg3.cparg);
				/* set divisor lsb requested */
				if (v86_cmd & SIO_MASK(SO_DIVLLSB))
				{
					v86_data = fubyte(arg3.cparg
						+ SO_DIVLLSB);
					intr_disable();
					fas_first_outb(fip, LINE_CTL_PORT, fip->lcr
						| LC_ENABLE_DIVISOR);
					fas_outb(fip, DIVISOR_LSB_PORT, v86_data);
					fas_outb(fip, LINE_CTL_PORT, fip->lcr
						& ~LC_ENABLE_DIVISOR);
					intr_restore();
				}
				/* set divisor msb requested */
				if (v86_cmd & SIO_MASK(SO_DIVLMSB))
				{
					v86_data = fubyte(arg3.cparg
						+ SO_DIVLMSB);
					intr_disable();
					fas_first_outb(fip, LINE_CTL_PORT, fip->lcr
						| LC_ENABLE_DIVISOR);
					fas_outb(fip, DIVISOR_MSB_PORT, v86_data);
					fas_outb(fip, LINE_CTL_PORT, fip->lcr
						& ~LC_ENABLE_DIVISOR);
					intr_restore();
				}
				/* set lcr requested */
				if (v86_cmd & SIO_MASK(SO_LCR))
				{
					v86_data = fubyte(arg3.cparg + SO_LCR);
					intr_disable();
					fip->lcr = v86_data
						& ~LC_ENABLE_DIVISOR;
					fas_first_outb(fip, LINE_CTL_PORT, fip->lcr);
					intr_restore();
				}
				/* set mcr requested */
				if (v86_cmd & SIO_MASK(SO_MCR))
				{
					v86_data = fubyte(arg3.cparg + SO_MCR);
					old_level = SPLINT();
					/* virtual dtr processing */
					if (v86_data & MC_SET_DTR)
					{
						fip->device_flags.s
							|= DF_MODEM_ENABLED;
						fip->mcr |= (fip->o_state
							& OS_WAIT_OPEN)
							? fip->modem.m.ei
							: fip->modem.m.eo;
					}
					else
					{
						fip->device_flags.s
							&= ~DF_MODEM_ENABLED;
						fip->mcr &= (fip->o_state
							& OS_WAIT_OPEN)
							? ~fip->modem.m.ei
							: ~fip->modem.m.eo;
					}
					/* virtual rts processing */
					if (fip->flow_flags.i
						& FF_HWI_HANDSHAKE)
					{
						if (v86_data & MC_SET_RTS)
						{
							if (fip->flow_flags.i
								& FF_RXFER_STOPPED)
							{
								fip->flow_flags.s
									&= ~FF_RXFER_STOPPED;

								/*
								 * schedule character transfer to UNIX
								 * buffer
								 */
								if (fip->recv_ring_cnt)
									event_sched(fip,
										EF_DO_RXFER);
							}
						}
						else
							fip->flow_flags.s
								|= FF_RXFER_STOPPED;
					}
					else if (!(fip->flow_flags.i
							& FF_HDX_HANDSHAKE))
					{
						if (v86_data & MC_SET_RTS)
						{
							fip->flow_flags.s
								|= FF_HDX_STARTED;
							fip->mcr
								|= fip->flow.m.hc;
						}
						else
						{
							fip->flow_flags.s
								&= ~FF_HDX_STARTED;
							fip->mcr
								&= ~fip->flow.m.hc;
						}
					}
					fas_first_outb(fip, MDM_CTL_PORT, fip->mcr);
					(void)splx(old_level);
				}

				old_level = SPLINT();
				/* enable transmitter and restart output */
				fip->device_flags.s &= ~DF_XMIT_LOCKED;
				fas_xproc(fip);
				(void)splx(old_level);
			}
			break;

		case AIOCSERIALIN:	 /* read port registers for dos */
			if ((fip->iflag & DOSMODE) && fip->v86_proc)
			{
				v86_cmd = fubyte(arg3.cparg);
				if (v86_cmd & SIO_MASK(SI_MSR))
				{
					(void)subyte(arg3.cparg + SI_MSR,
						((fip->flow_flags.i
								& FF_HWO_HANDSHAKE)
							? fip->msr
							| fip->flow.m.oc
							| fip->flow.m.oe
							: fip->msr)
						& MS_ANY_PRESENT);
				}
			}
			break;

		case AIOCSETSS:	 /* set start/stop characters */
			intr_disable();
			*((short *)(&fip->v86_ss)) = arg3.iarg;
			intr_restore();
			break;

		case AIOCINFO:		 /* show what type of device we are */
			u.u_r.r_reg.r_val1 = ('a' << 8) | (UINT) ((unchar) dev);
			break;
#endif
		default:			 /* default ioctl processing */
			/* if it is a TCSETA* command, call fas_param () */
			if (ttiocom(ttyp, cmd, arg3, arg4))
			{
				old_level = SPLINT();
				fas_param(fip, SOFT_INIT);
				(void)splx(old_level);
			}
			break;
	}
	return (0);
}

/* pass fas commands to the fas multi-function procedure */
static int
fas_proc(ttyp, arg2)
struct tty *ttyp;
int arg2;
{
	register UINT physical_unit;
	int old_level;

	physical_unit = ttyp - &fas_tty[0];
	if (physical_unit >= fas_physical_units)
		physical_unit -= fas_physical_units;

	old_level = SPLINT();
	fas_cmd(fas_info_ptr[physical_unit], ttyp, arg2);
	(void)splx(old_level);
	return (0);
}

/* set up a port according to the given termio structure */
static void
fas_param(fip, init_type)
register struct fas_info *fip;
int init_type;
{
	register UINT cflag;
	UINT divisor;
	int xmit_ring_size;

	REGVAR;

	cflag = fip->tty->t_cflag;

#if defined (HAVE_VPIX)
	/* we don't set port registers if we are in dos mode */
	if (fip->tty->t_iflag & DOSMODE)
		goto setflags2;
#endif
	/* if soft init mode: don't set port registers if cflag didn't change */
	if ((init_type == SOFT_INIT) && !((cflag ^ fip->cflag)
			& (CBAUD | CSIZE | CSTOPB
				| PARENB | PARODD)))
		goto setflags;

	/* lock transmitter and wait until it is empty */
	fip->device_flags.s |= DF_XMIT_LOCKED;
	while (fip->device_flags.i & (DF_XMIT_BUSY | DF_XMIT_BREAK
			| DF_GUARD_TIMEOUT))
		(void)sleep((caddr_t) & fip->device_flags.i, PZERO - 1);

	/* hangup line if it is baud rate 0, else enable line */
	if ((cflag & CBAUD) == B0)
	{
		fip->mcr &= (fip->o_state & OS_WAIT_OPEN)
			? ~fip->modem.m.ei
			: ~fip->modem.m.eo;
		fas_first_outb(fip, MDM_CTL_PORT, fip->mcr);
		fip->device_flags.s &= ~DF_MODEM_ENABLED;
	}
	else
	{
		if (!(fip->device_flags.i & DF_MODEM_ENABLED))
		{
			fip->mcr |= (fip->o_state & OS_WAIT_OPEN)
				? fip->modem.m.ei
				: fip->modem.m.eo;
			fas_first_outb(fip, MDM_CTL_PORT, fip->mcr);
			fip->device_flags.s |= DF_MODEM_ENABLED;
		}
	}

	/* don't change break flag */
	fip->lcr &= LC_SET_BREAK_LEVEL;

	/* set character size */
	switch (cflag & CSIZE)
	{
		case CS5:
			fip->lcr |= LC_WORDLEN_5;
			break;

		case CS6:
			fip->lcr |= LC_WORDLEN_6;
			break;

		case CS7:
			fip->lcr |= LC_WORDLEN_7;
			break;

		default:
			fip->lcr |= LC_WORDLEN_8;
			break;
	}

	/* set # of stop bits */
	if (cflag & CSTOPB)
		fip->lcr |= LC_STOPBITS_LONG;

	/* set parity */
	if (cflag & PARENB)
	{
		fip->lcr |= LC_ENABLE_PARITY;

		if (!(cflag & PARODD))
			fip->lcr |= LC_EVEN_PARITY;
	}

	/* set divisor registers only if baud rate is valid */
	if ((cflag & CBAUD) != B0)
	{
		/* get counter divisor for selected baud rate */
		divisor = fas_speeds[cflag & CBAUD];
		/* set LCR and baud rate */
		fas_first_outb(fip, LINE_CTL_PORT, fip->lcr
			| LC_ENABLE_DIVISOR);
		fas_outb(fip, DIVISOR_LSB_PORT, divisor);
		fas_outb(fip, DIVISOR_MSB_PORT, divisor >> 8);
	}

	fas_first_outb(fip, LINE_CTL_PORT, fip->lcr);

  setflags:

	/*
	 * check dynamic xmit ring buffer size against boundaries, modify it
	 * if necessary and update the fas_info structure
	 */
	if ((cflag & CBAUD) != B0)
	{
		xmit_ring_size = fas_xbuf_size[cflag & CBAUD]
			- tthiwat[cflag & CBAUD];
		if (xmit_ring_size < MAX_OUTPUT_FIFO_SIZE * 2)
		{
		  setflags2:
			xmit_ring_size = MAX_OUTPUT_FIFO_SIZE * 2;
		}
		if (xmit_ring_size > XMIT_BUFF_SIZE)
			xmit_ring_size = XMIT_BUFF_SIZE;
		fip->xmit_ring_size = xmit_ring_size;
	}

	/* setup character time for B0 mode */
	fas_ctimes[B0] = fas_ctimes[cflag & CBAUD];

	/* disable modem control signals if required by open mode */
	if (fip->o_state & OS_CLOCAL)
		cflag |= CLOCAL;

	/*
	 * Select hardware handshake depending on the minor device number and
	 * the CTSFLOW and RTSFLOW flags (if they are available).
	 */
	fip->flow_flags.s &= ~(FF_HWO_HANDSHAKE
		| FF_HWI_HANDSHAKE
		| FF_HDX_HANDSHAKE);
	if (fip->o_state & (OS_HWO_HANDSHAKE | OS_HWI_HANDSHAKE
			| OS_HDX_HANDSHAKE))
	{
		if (fip->o_state & OS_HWO_HANDSHAKE)
			fip->flow_flags.s |= FF_HWO_HANDSHAKE;
		if (fip->o_state & OS_HWI_HANDSHAKE)
			fip->flow_flags.s |= FF_HWI_HANDSHAKE;
		if (fip->o_state & OS_HDX_HANDSHAKE)
			fip->flow_flags.s |= FF_HDX_HANDSHAKE;
	}
	else
	{
#if defined (CTSFLOW)		 /* SYSV 3.2 Xenix compatibility */
		if ((cflag & (CTSFLOW | CLOCAL)) == CTSFLOW)
			fip->flow_flags.s |= FF_HWO_HANDSHAKE;
#endif
#if defined (RTSFLOW)		 /* SYSV 3.2 Xenix compatibility */
		if ((cflag & (RTSFLOW | CLOCAL)) == RTSFLOW)
			fip->flow_flags.s |= FF_HDX_HANDSHAKE;
#endif
	}

	/*
	 * Fake the carrier detect state flag if CLOCAL mode or if requested
	 * by open mode.
	 */
	if (!(~fip->msr & fip->modem.m.ca)
		|| (fip->o_state & OS_FAKE_CARR_ON)
		|| (cflag & CLOCAL))
		fip->tty->t_state |= CARR_ON;
	else
		fip->tty->t_state &= ~CARR_ON;

#if defined (XCLUDE)		 /* SYSV 3.2 Xenix compatibility */
	/* Permit exclusive use of this device. */
	if (cflag & XCLUDE)
		fip->o_state |= OS_EXCLUSIVE_OPEN_2;
	else
		fip->o_state &= ~OS_EXCLUSIVE_OPEN_2;
#endif

	fip->cflag = cflag;
	fip->iflag = fip->tty->t_iflag;

	/* enable transmitter */
	fip->device_flags.s &= ~DF_XMIT_LOCKED;

	/* setup handshake flags */
	fas_hdx_check(fip);
	fas_ihlw_check(fip);
	fas_fproc(fip, fip->new_msr);

	/* restart output */
	fas_xproc(fip);
}

/* Main fas interrupt handler. Actual character processing is splitted
   into sub-functions.
*/
int
fasintr(vect)
int vect;
{
	register struct fas_info *fip;
	register UINT status;
	struct fas_info *old_fip;
	int done, drop_mode;
	UINT port, old_recv_count;

	REGVAR;

#if defined(FASI)
	fasiintr_entries++;
#endif /* FASI */

	drop_mode = FALSE;

	/*
	 * The 8259 interrupt controller is set up for edge trigger. Therefor,
	 * we must loop until we make a complete pass without getting any
	 * UARTs that are interrupting.
	 */
	do
	{
		done = TRUE;
		fip = fas_first_int_user[vect];

		/* loop through all users of this interrupt vector */
		for (;; fip = fip->next_int_user)
		{
			if (!fip)
				break;		 /* all users done */

			/*
			 * process only ports that we expect ints from and that
			 * actually need to be serviced
			 */
		  fastloop:
			if (fas_first_inb(fip, INT_ID_PORT)
				& II_NO_INTS_PENDING)
			{
				/* restore the normal receiver trigger level */
				if (fip->device_flags.i & DF_NS16550A_DROP_MODE)
				{
					fip->device_flags.s &=
						~DF_NS16550A_DROP_MODE;
					fas_outb(fip, NS_FIFO_CTL_PORT,
						NS_FIFO_SETUP_CMD);
				}
				/* speed beats beauty */
				fip = fip->next_int_user;
				if (fip)
					goto fastloop;
				break;
			}

			/* restore the normal receiver trigger level */
			if (fip->device_flags.i & DF_NS16550A_DROP_MODE)
			{
				fip->device_flags.s &= ~DF_NS16550A_DROP_MODE;
				fas_outb(fip, NS_FIFO_CTL_PORT,
					NS_FIFO_SETUP_CMD);
			}

			done = FALSE;	 /* not done if we got an int */
			old_recv_count = fip->recv_ring_cnt;

			do
			{
				/* read in all the characters from the FIFO */
				if ((status = fas_inb(fip, LINE_STATUS_PORT))
					& LS_RCV_INT)
				{
					if (!drop_mode && (fip->device_flags.i
							& DF_DEVICE_IS_NS16550A))
					{

						/*
						 * Drop receiver trigger levels to make sure that
						 * we will see all received characters in all
						 * NS16550A. This prevents multiple interrupts if
						 * we receive characters on more than one unit.
						 */
						old_fip = fip;
						for (fip = fas_first_int_user[vect];
							fip; fip = fip->next_int_user)
						{
							if ((fip->device_flags.i
									& DF_DEVICE_IS_NS16550A)
								&& (fip != old_fip))
							{
								fip->device_flags.s |=
									DF_NS16550A_DROP_MODE;
								fas_first_outb(fip,
									NS_FIFO_CTL_PORT,
									NS_FIFO_DROP_CMD);
							}
						}
						fip = old_fip;
						drop_mode = TRUE;
					}
					status = fas_rproc(fip, status);
					sysinfo.rcvint++;
				}

				/* Is it a transmitter empty int ? */
				if ((status & LS_XMIT_AVAIL)
					&& (fip->device_flags.i & DF_XMIT_BUSY))
				{
					fip->device_flags.s &= ~DF_XMIT_BUSY;
					fas_xproc(fip);
					if (!(fip->device_flags.i
							& DF_XMIT_BUSY))
					{
						fip->device_flags.s |=
							DF_GUARD_TIMEOUT;
						fip->tty->t_state |=
							TIMEOUT;
						fip->timeout_idx =
							timeout(
							fas_timeout, fip,
							fas_ctimes[fip->cflag
								& CBAUD]);
					}
					sysinfo.xmtint++;
				}

				/*
				 * Has there been a polarity change on some of the modem
				 * lines ?
				 */
				if ((status = fas_inb(fip, MDM_STATUS_PORT))
					& MS_ANY_DELTA)
				{

					/*
					 * Do special RING line handling. RING generates an
					 * int only on the trailing edge.
					 */
					status = (status & ~MS_RING_PRESENT)
						| (fip->new_msr
						& MS_RING_PRESENT);
					if (status & MS_RING_TEDGE)
#if defined(FASI)
						fip->rings_detected++,
#endif /* FASI */
							status |= MS_RING_PRESENT;
					if ((status ^ fip->new_msr)
						& MS_ANY_PRESENT)
					{

						/*
						 * check for excessive modem status interrupts
						 */
						if (++fip->msi_cnt >
							(MAX_MSI_CNT / HZ)
							* (EVENT_TIME * HZ
								/ 1000))
						{
							fip->ier = IE_NONE;
							fas_outb(fip,
								INT_ENABLE_PORT,
								fip->ier);
						}
						/* check hw flow flags */
						fas_fproc(fip, status);
						fip->new_msr = status;
						event_sched(fip, EF_DO_MPROC);
					}
					sysinfo.mdmint++;
#if defined(FASI)
					fip->modem_status_events++;
#endif /* FASI */
				}
			}
			while (!(fas_inb(fip, INT_ID_PORT)
					& II_NO_INTS_PENDING));

			/* schedule character transfer to UNIX buffer */
			if (fip->recv_ring_cnt
#if defined (HAVE_VPIX)
				&& (((fip->iflag & DOSMODE)
						? MAX_VPIX_FILL - MIN_READ_CHUNK
						: MAX_UNIX_FILL - MIN_READ_CHUNK)
					>= fip->tty->t_rawq.c_cc)
#else
				&& ((MAX_UNIX_FILL - MIN_READ_CHUNK)
					>= fip->tty->t_rawq.c_cc)
#endif
				&& !(fip->flow_flags.i & FF_RXFER_STOPPED))
			{
				event_sched(fip, EF_DO_RXFER);
			}

			/* check input buffer high/low water marks */
			if (fip->recv_ring_cnt != old_recv_count)
				fas_ihlw_check(fip);
		}
	}
	while (!done);

	/*
	 * clear the shared interrupt since we have scanned all of the ports
	 * that share this interrupt vector
	 */
	if (port = fas_int_ack_port[vect])
		(void)outb(port, fas_int_ack[vect]);

	return (0);
}

/* hardware flow control interrupt handler */
static void
fas_fproc(fip, mdm_status)
register struct fas_info *fip;
register UINT mdm_status;
{

	/*
	 * Check the output flow control signals and set the state flag
	 * accordingly.
	 */
	if (!(~mdm_status & fip->flow.m.oc)
		|| (~mdm_status & fip->flow.m.oe)
		|| !(fip->flow_flags.i & FF_HWO_HANDSHAKE))
	{
		if (fip->flow_flags.i & FF_HWO_STOPPED)
		{
			fip->flow_flags.s &= ~FF_HWO_STOPPED;
			fas_xproc(fip);
		}
	}
	else
#if defined(FASI)
		fip->xmtr_hw_flow_count++,
			wakeup((caddr_t) & fip->device_flags.i),
#endif /* FASI */
			fip->flow_flags.s |= FF_HWO_STOPPED;
}

/* modem status handler */
static void
fas_mproc(fip)
register struct fas_info *fip;
{
	register struct tty *ttyp;
	register UINT mdm_status;
	UINT vpix_status;
	int old_level;

	ttyp = fip->tty;
	mdm_status = fip->new_msr;
	fip->new_msr &= ~MS_RING_PRESENT;

	/*
	 * Check the carrier detect signal and set the state flags
	 * accordingly. Also, if not in clocal mode, send SIGHUP on carrier
	 * loss and flush the buffers.
	 */
	if (!(fip->cflag & CLOCAL))
	{
		if (!(~mdm_status & fip->modem.m.ca))
		{
			ttyp->t_state |= CARR_ON;
			/* Unblock getty open only if it is ready to run. */
			if ((ttyp->t_state & WOPEN)
				&& (~fip->msr & fip->modem.m.ca))
#if defined(FASI)
			{
#endif
				(void)wakeup((caddr_t) & ttyp->t_canq);
#if defined(FASI)
				(void)wakeup((caddr_t) & fip->device_flags.i);
			}
#endif
		}
		else
		{
			if (!(~fip->msr & fip->modem.m.ca))
			{
				ttyp->t_state &= ~CARR_ON;
				old_level = SPLWRK();
				if (ttyp->t_state & ISOPEN)
					(void)signal(ttyp->t_pgrp, SIGHUP);
				(void)ttyflush(ttyp, FREAD | FWRITE);
				(void)splx(old_level);
			}
		}
	}

#if defined (HAVE_VPIX)
	if (((fip->iflag & (DOSMODE | PARMRK))
			== (DOSMODE | PARMRK))
		&& (fip->v86_intmask != V86VI_KBD))
	{
		/* prepare status bits for VP/ix */
		vpix_status = (((mdm_status ^ fip->msr) >> 4) & MS_ANY_DELTA)
			| (mdm_status & (MS_CTS_PRESENT
				| MS_DSR_PRESENT
				| MS_DCD_PRESENT));
		if (fip->flow_flags.i & FF_HWO_HANDSHAKE)
		{
			vpix_status &= ~((fip->flow.m.oc | fip->flow.m.oe)
				>> 4);
			vpix_status |= fip->flow.m.oc | fip->flow.m.oe;
		}
		/* send status bits to VP/ix */
		if ((vpix_status & MS_ANY_DELTA)
			&& fas_vpix_sr(fip, 2, vpix_status))
			event_sched(fip, EF_DO_RXFER);
	}
#endif
	fip->msr = mdm_status & ~MS_RING_PRESENT;

	/* re-schedule if modem status flags have changed in the mean time */
	if ((fip->new_msr ^ fip->msr) & MS_ANY_PRESENT)
	{
		event_sched(fip, EF_DO_MPROC)
	}
#if defined(FASI)
	else
		(void)wakeup((caddr_t) & fip->device_flags.i);
#endif /* FASI */
}

/* Receiver interrupt handler. Translates input characters to character
   sequences as described in TERMIO(7) man page.
*/
static UINT
fas_rproc(fip, line_status)
register struct fas_info *fip;
UINT line_status;
{
	struct tty *ttyp;
	UINT charac;
	register UINT csize;
	unchar metta[4];

	REGVAR;

	ttyp = fip->tty;

	fas_first_ctl(fip, RCV_DATA_PORT);

	/*
	 * Translate characters from FIFO according to the TERMIO(7) man page.
	 */
	do
	{
		charac = (line_status & LS_RCV_AVAIL)
#if defined(FASI)
			? (fip->characters_received++, fas_inb(fip, RCV_DATA_PORT))
#else
			? fas_inb(fip, RCV_DATA_PORT)
#endif /* FASI */
			: 0;			 /* was line status int only */

		/* do we have to junk the character ? */
		if (!(fip->cflag & CREAD)
			|| ((ttyp->t_state & (ISOPEN | CARR_ON)) !=
				(ISOPEN | CARR_ON)))
		{
			/* if there are FIFOs we take a short cut */
			if (fip->device_flags.i & DF_DEVICE_IS_NS16550A)
				fas_outb(fip, NS_FIFO_CTL_PORT, NS_FIFO_SETUP_CMD
					| NS_FIFO_CLR_RECV);
			else if (fip->device_flags.i & DF_DEVICE_IS_I82510)
			{
				fas_outb(fip, I_BANK_PORT, I_BANK_1);
				fas_outb(fip, I_RCM_PORT, I_FIFO_CLR_RECV);
				fas_outb(fip, I_BANK_PORT, I_BANK_0);
			}
			continue;
		}

		csize = 0;

		/* strip off 8th bit ? */
		if (fip->iflag & ISTRIP)
			charac &= 0x7f;

		/* ignore parity errors ? */
		if ((line_status & LS_PARITY_ERROR)
			&& !(fip->iflag & INPCK))
			line_status &= ~LS_PARITY_ERROR;

#if defined(FASI)
		if (line_status & LS_OVERRUN)
			fip->overrun_errors++;
#endif /* FASI */

		/* do we have some kind of character error ? */
		if (line_status & (LS_PARITY_ERROR
				| LS_FRAMING_ERROR
				| LS_BREAK_DETECTED))
		{
#if defined(FASI)
			if (line_status & LS_PARITY_ERROR)
				fip->parity_errors++;
			if (line_status & LS_FRAMING_ERROR)
				fip->framing_errors++;
			if (line_status & LS_BREAK_DETECTED)
				fip->breaks_detected++;
#endif /* FASI */
#if defined (HAVE_VPIX)
			if ((fip->iflag & (DOSMODE | PARMRK))
				== (DOSMODE | PARMRK))
			{
				/* send status bits to VP/ix */
				(void)fas_vpix_sr(fip, 1,
					(line_status & (LS_PARITY_ERROR
							| LS_FRAMING_ERROR
							| LS_BREAK_DETECTED))
					| LS_RCV_AVAIL
					| LS_XMIT_AVAIL
					| LS_XMIT_COMPLETE);
				goto valid_char;
			}
#endif
			/* is it a BREAK ? */
			if (line_status & LS_BREAK_DETECTED)
			{
				if (!(fip->iflag & IGNBRK))
					if (fip->iflag & BRKINT)
					{
						/* do BREAK interrupt */
						event_sched(fip, EF_DO_BRKINT);
					}
					else
					{
						metta[csize] = 0;
						csize++;
						if (fip->iflag & PARMRK)
						{
							metta[csize] = 0;
							csize++;
							metta[csize] = 0xff;
							csize++;
						}
					}
			}
			else if (!(fip->iflag & IGNPAR))
				if (fip->iflag & PARMRK)
				{
					metta[csize] = charac;
					csize++;
					metta[csize] = 0;
					csize++;
					metta[csize] = 0xff;
					csize++;
				}
				else
				{
					metta[csize] = 0;
					csize++;
				}
		}
		else
			/* is there a character to process ? */
		if (line_status & LS_RCV_AVAIL)
		{
			if (fip->iflag & IXON)
			{
				/* do output start/stop handling */
				if (fip->flow_flags.i & FF_SWO_STOPPED)
				{
#if defined (HAVE_VPIX)
					if ((charac == fip->v86_ss.ss_start)
#else
					if ((charac == CSTART)
#endif
						|| (fip->iflag & IXANY))
					{
						fip->flow_flags.s &=
							~FF_SWO_STOPPED;
						ttyp->t_state &= ~TTSTOP;
						/* restart output */
						fas_xproc(fip);
					}
				}
				else
				{
#if defined (HAVE_VPIX)
					if (charac == fip->v86_ss.ss_stop)
#else
					if (charac == CSTOP)
#endif
					{
						fip->flow_flags.s |=
							FF_SWO_STOPPED;
						ttyp->t_state |= TTSTOP;
					}
				}

				/*
				 * we don't put start/stop characters into the receiver
				 * buffer
				 */
#if defined (HAVE_VPIX)
				if ((charac == fip->v86_ss.ss_start)
					|| (charac == fip->v86_ss.ss_stop))
#else
				if ((charac == CSTART)
					|| (charac == CSTOP))
#endif
					continue;
			}
		  valid_char:
			if ((charac == 0xff) && (fip->iflag & PARMRK))
			{
				metta[csize] = 0xff;
				csize++;
				metta[csize] = 0xff;
				csize++;
			}
			else
			{

				/*
				 * we take a short-cut if only one character has to be put
				 * into the receiver buffer
				 */
				if (fip->recv_ring_cnt < RECV_BUFF_SIZE)
				{
					fip->recv_ring_cnt++;
					*fip->recv_ring_put_ptr = charac;
					if (++fip->recv_ring_put_ptr
						!= &fip->recv_buffer
						[RECV_BUFF_SIZE])
						continue;
					fip->recv_ring_put_ptr =
						&fip->recv_buffer[0];
				}
				continue;
			}
		}

		if (!(csize) || (fip->recv_ring_cnt + csize > RECV_BUFF_SIZE))
			continue;		 /* nothing to put into recv buffer */

		fip->recv_ring_cnt += csize;

		/* store translation in ring buffer */
		do
		{
			do
			{
				*fip->recv_ring_put_ptr = (metta - 1)[csize];
				if (++fip->recv_ring_put_ptr
					== &fip->recv_buffer[RECV_BUFF_SIZE])
					break;
			}
			while (--csize);
			if (!csize)
				break;
			fip->recv_ring_put_ptr = &fip->recv_buffer[0];
		}
		while (--csize);
	}
	while ((line_status = fas_inb(fip, LINE_STATUS_PORT)) & LS_RCV_INT);

	return (line_status);
}

/* Output characters to the transmitter register. */
static void
fas_xproc(fip)
register struct fas_info *fip;
{
	register UINT num_to_output;

	REGVAR;

	/* proceed only if transmitter is available */
	if ((fip->device_flags.i & (DF_XMIT_BUSY | DF_XMIT_BREAK
				| DF_XMIT_LOCKED))
		|| (fip->flow_flags.i & FF_HWO_STOPPED))
		goto sched;

	num_to_output = fip->xmit_fifo_size;

	/* handle XON/XOFF input flow control requests */
	if (fip->flow_flags.i & FF_SW_FC_REQ)
	{
#if defined (HAVE_VPIX)
		fas_first_outb(fip, XMT_DATA_PORT, (fip->flow_flags.i & FF_SWI_STOPPED)
			? fip->v86_ss.ss_stop
			: fip->v86_ss.ss_start);
#else
		fas_first_outb(fip, XMT_DATA_PORT, (fip->flow_flags.i & FF_SWI_STOPPED)
			? CSTOP
			: CSTART);
#endif
		fip->tty->t_state &= ~(TTXON | TTXOFF);
		fip->device_flags.s |= DF_XMIT_BUSY;
		fip->flow_flags.s &= ~FF_SW_FC_REQ;
		/* disable guard timeout */
		if (fip->device_flags.i & DF_GUARD_TIMEOUT)
		{
			fip->device_flags.s &= ~DF_GUARD_TIMEOUT;
			fip->tty->t_state &= ~TIMEOUT;
			(void)untimeout(fip->timeout_idx);
		}
		num_to_output--;
	}

	/* bail out if output is suspended by XOFF */
	if (fip->flow_flags.i & FF_SWO_STOPPED)
		goto sched;

	/*
	 * Determine how many chars to put into the transmitter register.
	 */
	if (fip->xmit_ring_cnt < num_to_output)
		num_to_output = fip->xmit_ring_cnt;

	/* no characters available ? */
	if (!num_to_output)
		goto sched;

	/* output characters */
	fip->xmit_ring_cnt -= num_to_output;

	fas_ctl(fip, XMT_DATA_PORT);

#if defined(FASI)
	fip->characters_transmitted += num_to_output;
#endif /* FASI */

	do
	{
		do
		{
			(void)outb(XMT_DATA_PORT.addr,
				*fip->xmit_ring_take_ptr);
			if (++fip->xmit_ring_take_ptr
				== &fip->xmit_buffer[XMIT_BUFF_SIZE])
				break;
		}
		while (--num_to_output);
		if (!num_to_output)
			break;
		fip->xmit_ring_take_ptr = &fip->xmit_buffer[0];
	}
	while (--num_to_output);

	/* signal that transmitter is busy now */
	fip->device_flags.s |= DF_XMIT_BUSY;
	/* disable guard timeout */
	if (fip->device_flags.i & DF_GUARD_TIMEOUT)
	{
		fip->device_flags.s &= ~DF_GUARD_TIMEOUT;
		fip->tty->t_state &= ~TIMEOUT;
		(void)untimeout(fip->timeout_idx);
	}

	/*
	 * schedule fas_xxfer () if there are more characters to transfer into
	 * the transmitter ring buffer
	 */
  sched:
	if ((fip->xmit_ring_size > fip->xmit_ring_cnt)
		&& (fip->tty->t_outq.c_cc || fip->tty->t_tbuf.c_count))
	{
		event_sched(fip, EF_DO_XXFER);
	}
}

/* Asynchronous event handler. Scheduled by functions that can't do the
   processing themselves because of execution time restrictions.
*/
static void
fas_event(dummy)
void *dummy;
{
	register struct fas_info *fip;
	register UINT unit;
	int old_level;

	old_level = SPLINT();

	unit = 0;
	fip = &fas_info[0];

	/* loop through all fas_info structures */
	for (;; fip++, unit++)
	{
		if (unit >= fas_physical_units)
			break;			 /* all structures done */

		/*
		 * process only structures that actually need to be serviced
		 */
	  fastloop2:
		if (!fip->event_flags.i)
		{
			/* speed beats beauty */
			fip++;
			if (++unit < fas_physical_units)
				goto fastloop2;
			break;
		}

		do
		{
			/* check the modem signals */
			if (fip->event_flags.i & EF_DO_MPROC)
			{
				fip->event_flags.s &= ~EF_DO_MPROC;
				fas_mproc(fip);

				/*
				 * disable the device if there were too many modem status
				 * interrupts
				 */
				if (fip->msi_cnt > (MAX_MSI_CNT / HZ)
					* (EVENT_TIME * HZ / 1000))
				{
					fip->device_flags.s &= ~(DF_DEVICE_CONFIGURED
						| DF_XMIT_BUSY
						| DF_XMIT_BREAK);
					fip->device_flags.s |= DF_XMIT_LOCKED;
					if (fip->device_flags.i & DF_GUARD_TIMEOUT)
					{
						fip->device_flags.s &=
							~DF_GUARD_TIMEOUT;
						fip->tty->t_state &= ~TIMEOUT;
						(void)untimeout(fip->timeout_idx);
						(void)wakeup((caddr_t) & (fip)->
							device_flags.i);
					}
					fip->tty->t_state &= ~CARR_ON;
					(void)SPLWRK();
					if (!(fip->cflag & CLOCAL)
						&& (fip->tty->t_state & ISOPEN))
						(void)signal(fip->tty->t_pgrp,
							SIGHUP);
					(void)ttyflush(fip->tty, FREAD | FWRITE);
					(void)printf("\nWARNING: Excessive modem status interrupts on FAS unit %d (check the cabling).\n",
						fip - &fas_info[0]);
					(void)SPLINT();
				}
				fip->msi_cnt = 0;
			}

			/* do the break interrupt */
			if (fip->event_flags.i & EF_DO_BRKINT)
			{
				fip->event_flags.s &= ~EF_DO_BRKINT;
				if (fip->tty->t_state & ISOPEN)
				{
					(void)SPLWRK();
					(*linesw[fip->tty->t_line].l_input)
						(fip->tty, L_BREAK);
					(void)SPLINT();
				}
			}

			/* transfer characters to the UNIX input buffer */
			if (fip->event_flags.i & EF_DO_RXFER)
			{
				fip->event_flags.s &= ~EF_DO_RXFER;
				if (!(fip->flow_flags.i & FF_RXFER_STOPPED))
				{
					(void)SPLWRK();
					fas_rxfer(fip);
					(void)SPLINT();
					/* check input buffer high/low water marks */
					fas_ihlw_check(fip);
				}
			}

			/* transfer characters to the output ring buffer */
			if (fip->event_flags.i & EF_DO_XXFER)
			{
				fip->event_flags.s &= ~EF_DO_XXFER;
				(void)SPLWRK();
				fas_xxfer(fip);
				(void)SPLINT();
				fas_hdx_check(fip);
				/* output characters */
				fas_xproc(fip);
			}

#if defined (HAVE_VPIX)
			/* send pseudorupt to VP/ix */
			if (fip->event_flags.i & EF_SIGNAL_VPIX)
			{
				fip->event_flags.s &= ~EF_SIGNAL_VPIX;
				if ((fip->iflag & DOSMODE) && fip->v86_proc)
				{
					(void)SPLWRK();
					(void)v86setint(fip->v86_proc,
						fip->v86_intmask);
					(void)SPLINT();
				}
			}
#endif
		}
		while (fip->event_flags.i);

		/* allow pending tty interrupts */
		(void)SPLWRK();
		(void)SPLINT();
	}

	event_scheduled = FALSE;

	/* check whether there have been new requests in the mean time */
	for (unit = 0, fip = &fas_info[0]; unit < fas_physical_units;
		fip++, unit++)
		if (fip->event_flags.i)
		{

			/*
			 * there is at least one new request, so schedule the next
			 * event processing
			 */
			event_scheduled = TRUE;
			(void)timeout(fas_event, (void *)NULL,
				(EVENT_TIME) * (HZ) / 1000);
			break;
		}

#if defined(FASI)
	(void)wakeup((caddr_t) & fip->device_flags.i);
#endif /* FASI */
	(void)splx(old_level);
}

#if defined (HAVE_VPIX)
/* Send port status register to VP/ix */
static int
fas_vpix_sr(fip, token, status)
register struct fas_info *fip;
UINT token;
UINT status;
{
	if ((fip->recv_ring_cnt <= RECV_BUFF_SIZE - 3)
		&& ((fip->tty->t_state & (ISOPEN | CARR_ON)) ==
			(ISOPEN | CARR_ON)))
	{

		/*
		 * sent the character sequence 0xff, <token>, <status> to VP/ix
		 */
		fip->recv_ring_cnt += 3;

		*fip->recv_ring_put_ptr = 0xff;
		if (++fip->recv_ring_put_ptr
			== &fip->recv_buffer[RECV_BUFF_SIZE])
			fip->recv_ring_put_ptr
				= &fip->recv_buffer[0];
		*fip->recv_ring_put_ptr = token;
		if (++fip->recv_ring_put_ptr
			== &fip->recv_buffer[RECV_BUFF_SIZE])
			fip->recv_ring_put_ptr
				= &fip->recv_buffer[0];
		*fip->recv_ring_put_ptr = status;
		if (++fip->recv_ring_put_ptr
			== &fip->recv_buffer[RECV_BUFF_SIZE])
			fip->recv_ring_put_ptr
				= &fip->recv_buffer[0];
		return (TRUE);
	}
	return (FALSE);
}
#endif

/* Receiver ring buffer -> UNIX buffer transfer function. */
static void
fas_rxfer(fip)
register struct fas_info *fip;
{
	register struct tty *ttyp;
	register int num_to_xfer;
	int num_save;
	int old_level;

	ttyp = fip->tty;

	for (;;)
	{
		if (!fip->recv_ring_cnt || !ttyp->t_rbuf.c_ptr)
			break;			 /* no characters to transfer */

		/* determine how many characters to transfer */
#if defined (HAVE_VPIX)
		num_to_xfer = ((fip->iflag & DOSMODE)
			? MAX_VPIX_FILL
			: MAX_UNIX_FILL) - ttyp->t_rawq.c_cc;
#else
		num_to_xfer = MAX_UNIX_FILL - ttyp->t_rawq.c_cc;
#endif

		if (num_to_xfer < MIN_READ_CHUNK)
			break;			 /* input buffer full */

#if defined (HAVE_VPIX)
		/* wakeup VP/ix */
		if ((fip->iflag & DOSMODE) && !ttyp->t_rawq.c_cc)
			event_sched(fip, EF_SIGNAL_VPIX);
#endif

		/* determine how many characters are in one contigous block */
		if (fip->recv_ring_cnt < num_to_xfer)
			num_to_xfer = fip->recv_ring_cnt;
		if (&fip->recv_buffer[RECV_BUFF_SIZE] - fip->recv_ring_take_ptr
			< num_to_xfer)
			num_to_xfer = &fip->recv_buffer[RECV_BUFF_SIZE]
				- fip->recv_ring_take_ptr;
		if (ttyp->t_rbuf.c_count < num_to_xfer)
			num_to_xfer = ttyp->t_rbuf.c_count;

		num_save = num_to_xfer;
		ttyp->t_rbuf.c_count -= num_to_xfer;

		/* do the transfer */
		do
		{
			*ttyp->t_rbuf.c_ptr = *fip->recv_ring_take_ptr;
			ttyp->t_rbuf.c_ptr++;
			fip->recv_ring_take_ptr++;
		}
		while (--num_to_xfer);

		if (fip->recv_ring_take_ptr == &fip->recv_buffer[RECV_BUFF_SIZE])
			fip->recv_ring_take_ptr = &fip->recv_buffer[0];

		intr_disable();
		fip->recv_ring_cnt -= num_save;
		intr_restore();

		ttyp->t_rbuf.c_ptr -= ttyp->t_rbuf.c_size
			- ttyp->t_rbuf.c_count;
		(*linesw[ttyp->t_line].l_input) (ttyp, L_BUF);
	}
}

/* UNIX buffer -> transmitter ring buffer transfer function. */
static void
fas_xxfer(fip)
register struct fas_info *fip;
{
	register struct tty *ttyp;
	register int num_to_xfer;
	int num_save;
	int old_level;

	ttyp = fip->tty;

	for (;;)
	{

		/*
		 * Check if tbuf is empty. If it is empty, reset buffer pointer
		 * and counter and get the next chunk of output characters.
		 */
		if (!ttyp->t_tbuf.c_ptr || !ttyp->t_tbuf.c_count)
		{
			if (ttyp->t_tbuf.c_ptr)
				ttyp->t_tbuf.c_ptr -= ttyp->t_tbuf.c_size;
			if (!((*linesw[ttyp->t_line].l_output) (ttyp)
					& CPRES))
				break;
		}

		/* set the maximum character limit */
		num_to_xfer = fip->xmit_ring_size - fip->xmit_ring_cnt;

		/* Return if transmitter ring buffer is full. */
		if (num_to_xfer < 1)
			break;

		/* Determine how many chars to transfer this time. */
		if (&fip->xmit_buffer[XMIT_BUFF_SIZE] - fip->xmit_ring_put_ptr
			< num_to_xfer)
			num_to_xfer = &fip->xmit_buffer[XMIT_BUFF_SIZE]
				- fip->xmit_ring_put_ptr;
		if (ttyp->t_tbuf.c_count < num_to_xfer)
			num_to_xfer = ttyp->t_tbuf.c_count;

		num_save = num_to_xfer;
		ttyp->t_tbuf.c_count -= num_to_xfer;
		ttyp->t_state |= BUSY;

		/* do the transfer */
		do
		{
			*fip->xmit_ring_put_ptr = *ttyp->t_tbuf.c_ptr;
			ttyp->t_tbuf.c_ptr++;
			fip->xmit_ring_put_ptr++;
		}
		while (--num_to_xfer);

		if (fip->xmit_ring_put_ptr == &fip->xmit_buffer[XMIT_BUFF_SIZE])
			fip->xmit_ring_put_ptr = &fip->xmit_buffer[0];

		intr_disable();
		fip->xmit_ring_cnt += num_save;
		intr_restore();
	}
}

/* Input buffer high/low water mark check. */
static void
fas_ihlw_check(fip)
register struct fas_info *fip;
{
	REGVAR;

	if (fip->flow_flags.i & FF_HWI_STOPPED)
	{

		/*
		 * If input buffer level has dropped below the low water mark and
		 * input was stopped by hardware handshake, restart input.
		 */
		if (fip->recv_ring_cnt < HW_LOW_WATER)
		{
			fip->mcr |= fip->flow.m.ic;
			fas_first_outb(fip, MDM_CTL_PORT, fip->mcr);
			fip->flow_flags.s &= ~FF_HWI_STOPPED;
		}
	}
	else
	{

		/*
		 * If input buffer level has risen above the high water mark and
		 * input is not yet stopped, stop input by hardware handshake.
		 */
		if ((fip->flow_flags.i & FF_HWI_HANDSHAKE)
			&& (fip->recv_ring_cnt > HW_HIGH_WATER))
		{
			fip->mcr &= ~fip->flow.m.ic;
			fas_first_outb(fip, MDM_CTL_PORT, fip->mcr);
			fip->flow_flags.s |= FF_HWI_STOPPED;
#if defined(FASI)
			fip->rcvr_hw_flow_count++;
			(void)wakeup((caddr_t) & fip->device_flags.i);
#endif /* FASI */
		}
	}

	if (fip->flow_flags.i & FF_SWI_STOPPED)
	{

		/*
		 * If input buffer level has dropped below the low water mark and
		 * input was stopped by XOFF, send XON to restart input.
		 */
		if (!(fip->iflag & IXOFF)
			|| (fip->recv_ring_cnt < SW_LOW_WATER))
		{
			fip->flow_flags.s &= ~FF_SWI_STOPPED;
			fip->flow_flags.s ^= FF_SW_FC_REQ;
			if (fip->flow_flags.i & FF_SW_FC_REQ)
			{
				fip->tty->t_state |= TTXON;
				fas_xproc(fip);
			}
			else
				fip->tty->t_state &= ~TTXOFF;
		}
	}
	else
	{

		/*
		 * If input buffer level has risen above the high water mark and
		 * input is not yet stopped, send XOFF to stop input.
		 */
		if ((fip->iflag & IXOFF)
			&& (fip->recv_ring_cnt > SW_HIGH_WATER))
		{
			fip->flow_flags.s |= FF_SWI_STOPPED;
			fip->flow_flags.s ^= FF_SW_FC_REQ;
			if (fip->flow_flags.i & FF_SW_FC_REQ)
			{
				fip->tty->t_state |= TTXOFF;
				fas_xproc(fip);
#if defined(FASI)
				fip->rcvr_sw_flow_count++;
				(void)wakeup((caddr_t) & fip->device_flags.i);
#endif /* FASI */
			}
			else
				fip->tty->t_state &= ~TTXON;
		}
	}
}

/* Half-duplex hardware flow control check. */
static void
fas_hdx_check(fip)
register struct fas_info *fip;
{
	REGVAR;

	/* don't interfere with hardware input handshake */
	if (fip->flow_flags.i & FF_HWI_HANDSHAKE)
		return;

#if defined (HAVE_VPIX)

	/*
	 * don't touch the mcr if we are in dos mode and hdx hardware
	 * handshake is disabled (dos handles the handshake line(s) on its own
	 * in this mode)
	 */
	if ((fip->iflag & DOSMODE) && !(fip->flow_flags.i & FF_HDX_HANDSHAKE))
		return;
#endif
	if (fip->flow_flags.i & FF_HDX_STARTED)
	{

		/*
		 * If output buffer is empty signal the connected device that all
		 * output is done.
		 */
		if ((fip->flow_flags.i & FF_HDX_HANDSHAKE)
			&& !(fip->tty->t_state & BUSY))
		{
			fip->mcr &= ~fip->flow.m.hc;
			fas_first_outb(fip, MDM_CTL_PORT, fip->mcr);
			fip->flow_flags.s &= ~FF_HDX_STARTED;
		}
	}
	else
	{

		/*
		 * If the output ring buffer contains characters and was
		 * previously empty signal the connected device that output is
		 * resumed.
		 */
		if (!(fip->flow_flags.i & FF_HDX_HANDSHAKE)
			|| (fip->tty->t_state & BUSY))
		{
			fip->mcr |= fip->flow.m.hc;
			fas_first_outb(fip, MDM_CTL_PORT, fip->mcr);
			fip->flow_flags.s |= FF_HDX_STARTED;
		}
	}
}

/* Handle hangup after last close */
static void
fas_hangup(fip)
register struct fas_info *fip;
{
	int old_level;

	REGVAR;

	old_level = SPLINT();

	if (fip->device_flags.i & DF_DO_HANGUP)
	{
		/* do the hangup */
		fip->mcr &= ~(fip->modem.m.ei
			| fip->modem.m.eo);
		fip->mcr |= fip->modem.m.di;
		fas_first_outb(fip, MDM_CTL_PORT, fip->mcr);
		fip->device_flags.s &= ~(DF_MODEM_ENABLED | DF_DO_HANGUP);
		(void)timeout(fas_hangup, fip, (fip->device_flags.i
				& DF_DEVICE_CONFIGURED)
			? (HANGUP_TIME) * (HZ) / 1000
			: (RECOVER_TIME) * (HZ));
	}
	else
	{
		/* unlock the device */
		fip->device_flags.s |= DF_DEVICE_CONFIGURED;

		/*
		 * If there was a waiting getty open on this port, reopen the
		 * physical device.
		 */
		if (fip->o_state & OS_WAIT_OPEN)
		{
			fas_open_device(fip);
			fas_param(fip, HARD_INIT);	/* set up port regs */
		}
		release_device_lock(fip);
	}
	(void)splx(old_level);
}

/* main timeout function */
static void
fas_timeout(fip)
register struct fas_info *fip;
{
	int old_level;

	REGVAR;

	old_level = SPLINT();

	/* handle break request */
	if (fip->device_flags.i & DF_DO_BREAK)
	{
		/* set up break request flags */
		fip->lcr |= LC_SET_BREAK_LEVEL;
		fas_first_outb(fip, LINE_CTL_PORT, fip->lcr);
		fip->device_flags.s &= ~(DF_DO_BREAK | DF_GUARD_TIMEOUT);
		(void)timeout(fas_timeout, fip, (BREAK_TIME) * (HZ) / 1000);
		(void)splx(old_level);
		return;
	}

	/* reset break state */
	if (fip->device_flags.i & DF_XMIT_BREAK)
	{
		if (fip->lcr & LC_SET_BREAK_LEVEL)
		{
			fip->lcr &= ~LC_SET_BREAK_LEVEL;
			fas_first_outb(fip, LINE_CTL_PORT, fip->lcr);
			fip->device_flags.s |= DF_GUARD_TIMEOUT;
			fip->timeout_idx = timeout(fas_timeout, fip,
				fas_ctimes[fip->cflag & CBAUD]);
			(void)splx(old_level);
			return;
		}
		fip->device_flags.s &= ~DF_XMIT_BREAK;
		/* restart output after BREAK */
		fas_xproc(fip);
	}

	/* handle character guard timeout */
	if (fip->device_flags.i & DF_GUARD_TIMEOUT)
	{
		fip->device_flags.s &= ~DF_GUARD_TIMEOUT;
		if (!fip->xmit_ring_cnt)
		{
			fip->tty->t_state &= ~BUSY;
			fas_hdx_check(fip);
		}
	}

	fip->tty->t_state &= ~TIMEOUT;

	event_sched(fip, EF_DO_XXFER);

	(void)wakeup((caddr_t) & (fip)->device_flags.i);
	(void)splx(old_level);
}

/* Several functions for flow control, character output and special event
   requests and handling.
*/
static void
fas_cmd(fip, ttyp, arg2)
register struct fas_info *fip;
register struct tty *ttyp;
int arg2;
{
	REGVAR;

	switch (arg2)
	{
		case T_TIME:		 /* timeout */
			goto start_output;

		case T_OUTPUT:		 /* output characters to the transmitter */
			if (fip->xmit_ring_size > fip->xmit_ring_cnt)
			{
			  start_output:
				event_sched(fip, EF_DO_XXFER);
			}
			break;

		case T_SUSPEND:	 /* suspend character output */
			fip->flow_flags.s |= FF_SWO_STOPPED;
#if defined(FASI)
			fip->xmtr_sw_flow_count++;
			(void)wakeup((caddr_t) & fip->device_flags.i);
#endif /* FASI */
			ttyp->t_state |= TTSTOP;
			break;

		case T_RESUME:		 /* restart character output */
			fip->flow_flags.s &= ~FF_SWO_STOPPED;
			ttyp->t_state &= ~TTSTOP;
			fas_xproc(fip);
			break;

		case T_BLOCK:		 /* stop character input, request XOFF */
			ttyp->t_state |= TBLOCK;
			break;			 /* note: we do our own XON/XOFF */

		case T_UNBLOCK:	 /* restart character input, request XON */
			ttyp->t_state &= ~TBLOCK;
			break;			 /* note: we do our own XON/XOFF */

		case T_RFLUSH:		 /* flush input buffers and restart input */
			if (fip->device_flags.i & DF_DEVICE_IS_NS16550A)
				fas_first_outb(fip, NS_FIFO_CTL_PORT, NS_FIFO_SETUP_CMD
					| NS_FIFO_CLR_RECV);
			else if (fip->device_flags.i & DF_DEVICE_IS_I82510)
			{
				fas_first_outb(fip, I_BANK_PORT, I_BANK_1);
				fas_outb(fip, I_RCM_PORT, I_FIFO_CLR_RECV);
				fas_outb(fip, I_BANK_PORT, I_BANK_0);
			}

			fip->recv_ring_take_ptr = fip->recv_ring_put_ptr;
			fip->recv_ring_cnt = 0;
			ttyp->t_state &= ~TBLOCK;

			fas_ihlw_check(fip);
			break;

		case T_WFLUSH:		 /* flush output buffer and restart output */
			if (fip->device_flags.i & DF_DEVICE_IS_NS16550A)
				fas_first_outb(fip, NS_FIFO_CTL_PORT, NS_FIFO_SETUP_CMD
					| NS_FIFO_CLR_XMIT);
			else if (fip->device_flags.i & DF_DEVICE_IS_I82510)
			{
				fas_first_outb(fip, I_BANK_PORT, I_BANK_1);
				fas_outb(fip, I_TCM_PORT, I_FIFO_CLR_XMIT);
				fas_outb(fip, I_BANK_PORT, I_BANK_0);
			}

			fip->xmit_ring_take_ptr = fip->xmit_ring_put_ptr;
			fip->xmit_ring_cnt = 0;

			fip->flow_flags.s &= ~FF_SWO_STOPPED;
			ttyp->t_state &= ~TTSTOP;

			if (ttyp->t_tbuf.c_ptr)
				ttyp->t_tbuf.c_ptr += ttyp->t_tbuf.c_count;
			ttyp->t_tbuf.c_count = 0;

			if (!(fip->device_flags.i & (DF_XMIT_BUSY | DF_GUARD_TIMEOUT)))
			{
				ttyp->t_state &= ~BUSY;
				fas_hdx_check(fip);
				goto start_output;
			}
			break;

		case T_BREAK:		 /* do a break on the transmitter line */
			fip->device_flags.s |= DF_XMIT_BREAK;
			ttyp->t_state |= TIMEOUT;
			if (fip->device_flags.i & (DF_XMIT_BUSY | DF_GUARD_TIMEOUT))
			{
				fip->device_flags.s |= DF_DO_BREAK;
			}
			else
			{
				/* set up break request flags */
				fip->lcr |= LC_SET_BREAK_LEVEL;
				fas_first_outb(fip, LINE_CTL_PORT, fip->lcr);
				(void)timeout(fas_timeout, fip, (BREAK_TIME) * (HZ)
					/ 1000);
			}
			break;

		case T_PARM:		 /* set up the port according to the termio
							  * structure */
			fas_param(fip, SOFT_INIT);
			break;

		case T_SWTCH:		 /* handle layer switch request */
			break;
	}
}

/* open device physically */
static void
fas_open_device(fip)
register struct fas_info *fip;
{
	REGVAR;

	/* if already open, set up the mcr register only */
	if (fip->device_flags.i & DF_DEVICE_OPEN)
		goto setmcr;

	/* init some variables */
	fip->device_flags.s &= DF_DEVICE_CONFIGURED | DF_DEVICE_IS_NS16550A
		| DF_DEVICE_IS_I82510 | DF_DEVICE_LOCKED
		| DF_CTL_FIRST | DF_CTL_EVERY;
	fip->flow_flags.s = 0;
	fip->cflag = 0;
	fip->iflag = 0;
	fip->recv_ring_take_ptr = fip->recv_ring_put_ptr;
	fip->recv_ring_cnt = 0;
	fip->xmit_ring_take_ptr = fip->xmit_ring_put_ptr;
	fip->xmit_ring_cnt = 0;

	/* hook into the interrupt users chain */
	fip->next_int_user = fas_first_int_user[fip->vec];
	if (fip->next_int_user)
		fip->next_int_user->prev_int_user = fip;
	fas_first_int_user[fip->vec] = fip;
	fip->prev_int_user = (struct fas_info *)NULL;

	fip->lcr = 0;
	fas_first_outb(fip, LINE_CTL_PORT, fip->lcr);

	/* clear and disable FIFOs */
	if (fip->device_flags.i & DF_DEVICE_IS_NS16550A)
		fas_outb(fip, NS_FIFO_CTL_PORT, NS_FIFO_CLEAR_CMD);
	else if (fip->device_flags.i & DF_DEVICE_IS_I82510)
	{
		fas_outb(fip, I_BANK_PORT, I_BANK_1);
		fas_outb(fip, I_TCM_PORT, I_FIFO_CLR_XMIT);
		fas_outb(fip, I_RCM_PORT, I_FIFO_CLR_RECV);
		fas_outb(fip, I_BANK_PORT, I_BANK_2);
		fas_outb(fip, I_IDM_PORT, I_FIFO_CLEAR_CMD);
		fas_outb(fip, I_BANK_PORT, I_BANK_0);
	}

	/* clear interrupts */
	(void)fas_inb(fip, MDM_STATUS_PORT);
	(void)fas_inb(fip, RCV_DATA_PORT);
	(void)fas_inb(fip, RCV_DATA_PORT);
	(void)fas_inb(fip, LINE_STATUS_PORT);
	(void)fas_inb(fip, INT_ID_PORT);

	/* enable FIFOs */
	if (fip->device_flags.i & DF_DEVICE_IS_NS16550A)
		fas_outb(fip, NS_FIFO_CTL_PORT, NS_FIFO_SETUP_CMD);
	else if (fip->device_flags.i & DF_DEVICE_IS_I82510)
	{
		fas_outb(fip, I_BANK_PORT, I_BANK_2);
		fas_outb(fip, I_IDM_PORT, I_FIFO_SETUP_CMD);
		fas_outb(fip, I_BANK_PORT, I_BANK_0);
	}

	fip->msi_cnt = 0;
	fip->msr = fip->new_msr = fas_inb(fip, MDM_STATUS_PORT)
		& (MS_CTS_PRESENT
		| MS_DSR_PRESENT
		| MS_DCD_PRESENT);

	fip->ier = IE_INIT_MODE; /* enable UART interrupts */
	fas_outb(fip, INT_ENABLE_PORT, fip->ier);

  setmcr:
	/* set up modem and flow control lines */
	fip->mcr &= ~(fip->modem.m.di
		| fip->modem.m.ei
		| fip->modem.m.eo
		| fip->flow.m.ic
		| fip->flow.m.hc);

	fip->mcr |= (fip->o_state & OS_WAIT_OPEN)
		? fip->modem.m.ei
		: fip->modem.m.eo;

	if (fip->o_state & OS_HWI_HANDSHAKE)
		fip->mcr |= fip->flow.m.ic;
	else if (!(fip->o_state & OS_HDX_HANDSHAKE))
	{
		fip->flow_flags.s |= FF_HDX_STARTED;
		fip->mcr |= fip->flow.m.hc;
	}

	fas_outb(fip, MDM_CTL_PORT, fip->mcr);

	fip->device_flags.s |= DF_DEVICE_OPEN | DF_MODEM_ENABLED;
}

/* close device physically */
static void
fas_close_device(fip)
register struct fas_info *fip;
{
	REGVAR;

	fip->device_flags.s &= ~DF_DEVICE_OPEN;

	fip->ier = IE_NONE;		 /* disable UART interrupts */
	fas_first_outb(fip, INT_ENABLE_PORT, fip->ier);

	/* drop flow control lines */
	fip->mcr &= (fip->o_state & OS_HWI_HANDSHAKE)
		? ~fip->flow.m.ic
		: ~fip->flow.m.hc;
	fas_outb(fip, MDM_CTL_PORT, fip->mcr);

	/* clear and disable FIFOs */
	if (fip->device_flags.i & DF_DEVICE_IS_NS16550A)
		fas_outb(fip, NS_FIFO_CTL_PORT, NS_FIFO_CLEAR_CMD);
	else if (fip->device_flags.i & DF_DEVICE_IS_I82510)
	{
		fas_outb(fip, I_BANK_PORT, I_BANK_1);
		fas_outb(fip, I_TCM_PORT, I_FIFO_CLR_XMIT);
		fas_outb(fip, I_RCM_PORT, I_FIFO_CLR_RECV);
		fas_outb(fip, I_BANK_PORT, I_BANK_2);
		fas_outb(fip, I_IDM_PORT, I_FIFO_CLEAR_CMD);
		fas_outb(fip, I_BANK_PORT, I_BANK_0);
	}

	/* reset break level */
	fip->lcr &= ~LC_SET_BREAK_LEVEL;
	fas_outb(fip, LINE_CTL_PORT, fip->lcr);

	/* unhook from interrupt users chain */
	if (fip->prev_int_user)
		fip->prev_int_user->next_int_user = fip->next_int_user;
	else
		fas_first_int_user[fip->vec] = fip->next_int_user;
	if (fip->next_int_user)
		fip->next_int_user->prev_int_user = fip->prev_int_user;

	if ((fip->cflag & HUPCL)
		|| !(fip->device_flags.i & DF_DEVICE_CONFIGURED))
	{
		/* request hangup */
		fip->device_flags.s |= DF_DO_HANGUP;
		(void)timeout(fas_hangup, fip, (HANGUP_DELAY) * (HZ) / 1000);
	}
}

/* compute the port access control value */
static UINT
fas_make_ctl_val(fip, unit, num)
register struct fas_info *fip;
UINT unit;
UINT num;
{
	register UINT mask, val;
	UINT i;

	if (fip->device_flags.i & DF_CTL_FIRST)
		return (fas_ctl_val[unit]);

	if (fip->device_flags.i & DF_CTL_EVERY)
	{
		for (i = 0, mask = fas_ctl_val[unit],
			val = fas_ctl_val[unit] << 8; i < 8; i++)
		{
			if (mask & 0x100)
			{
				if (num & 0x01)
					val ^= 0x100;
				num >>= 1;
			}
			mask >>= 1;
			val >>= 1;
		}
		return (val);
	}
	return (0);
}

/* test device thoroughly */
static int
fas_test_device(fip)
register struct fas_info *fip;
{
	register unchar *cptr;
	int done;
	UINT delay_count, i;
	static UINT lcrval[3] =
	{
		LC_WORDLEN_8,
		LC_WORDLEN_8 | LC_ENABLE_PARITY,
		LC_WORDLEN_8 | LC_ENABLE_PARITY | LC_EVEN_PARITY
	};

	REGVAR;

	/* make sure FIFO is off */
	fas_first_outb(fip, NS_FIFO_CTL_PORT, NS_FIFO_CLEAR_CMD);
	fas_outb(fip, I_BANK_PORT, I_BANK_2);
	fas_outb(fip, I_IDM_PORT, I_FIFO_CLEAR_CMD);
	fas_outb(fip, I_BANK_PORT, I_BANK_0);

	/* set counter divisor */
	fas_outb(fip, LINE_CTL_PORT, LC_ENABLE_DIVISOR);
	fas_outb(fip, DIVISOR_LSB_PORT, fas_speeds[B38400]);
	fas_outb(fip, DIVISOR_MSB_PORT, fas_speeds[B38400] >> 8);
	fas_outb(fip, LINE_CTL_PORT, 0);

	/* switch to local loopback */
	fas_outb(fip, MDM_CTL_PORT, MC_SET_LOOPBACK);

	done = 0;

	/* wait until the transmitter register is empty */
	for (delay_count = 20000;
		delay_count && (~fas_inb(fip, LINE_STATUS_PORT)
			& (LS_XMIT_AVAIL | LS_XMIT_COMPLETE));
		delay_count--)
		;

	if (!delay_count)
		done = 1;

	if (!done)
	{
		/* clear flags */
		(void)fas_inb(fip, RCV_DATA_PORT);
		(void)fas_inb(fip, RCV_DATA_PORT);
		(void)fas_inb(fip, LINE_STATUS_PORT);

		/*
		 * make sure there are no more characters in the receiver register
		 */
		for (delay_count = 20000;
			delay_count && !(fas_inb(fip, LINE_STATUS_PORT) & LS_RCV_AVAIL);
			delay_count--)
			;

		if (delay_count)
			(void)fas_inb(fip, RCV_DATA_PORT);

		/* test pattern */
		cptr = (unchar *) "\
\377\125\252\045\244\0\
\377\125\252\045\244\0\
\377\125\252\045\244\0\
\377\125\252\045\244\0\
\377\125\252\045\244\0\0";

		do
		{
			for (i = 0; i < 3; i++)
			{

				/*
				 * test transmitter and receiver with different line
				 * settings
				 */
				fas_outb(fip, LINE_CTL_PORT, lcrval[i]);

				/*
				 * wait until the transmitter register is empty
				 */
				for (delay_count = 20000;
					delay_count && (~fas_inb(fip, LINE_STATUS_PORT)
						& (LS_XMIT_AVAIL
							| LS_XMIT_COMPLETE));
					delay_count--)
					;

				if (!delay_count)
				{
					done = 2;
					break;
				}

				/* send test pattern */
				fas_outb(fip, XMT_DATA_PORT, *cptr);

				/* wait until the test pattern is received */
				for (delay_count = 20000;
					delay_count && ((fas_inb(fip, LINE_STATUS_PORT)
							& LS_RCV_INT)
						!= LS_RCV_AVAIL);
					delay_count--)
					;

				if (!delay_count)
				{
					done = 3;
					break;
				}

				/* check test pattern */
				if (fas_inb(fip, RCV_DATA_PORT) != *cptr)
				{
					done = 4;
					break;
				}
			}

			if (done)
				break;
		}
		while (*((UINT16 *) cptr++));
	}

	if (!done)
	{
		/* wait until the transmitter register is empty */
		for (delay_count = 20000;
			delay_count && (~fas_inb(fip, LINE_STATUS_PORT)
				& (LS_XMIT_AVAIL | LS_XMIT_COMPLETE));
			delay_count--)
			;

		if (!delay_count)
			done = 5;
	}

	if (!done)
	{
		/* test pattern */
		cptr = (unchar *) "\
\005\142\012\237\006\130\011\257\017\361\0\017\
\005\142\012\237\006\130\011\257\017\361\0\017\
\005\142\012\237\006\130\011\257\017\361\0\017\
\005\142\012\237\006\130\011\257\017\361\0\017\
\005\142\012\237\006\130\011\257\017\361\0\017\0\0";

		/* clear delta bits */
		(void)fas_inb(fip, MDM_STATUS_PORT);

		do
		{
			/* test modem control and status lines */
			fas_outb(fip, MDM_CTL_PORT, *cptr | MC_SET_LOOPBACK);
			if (fas_inb(fip, MDM_STATUS_PORT) != *(cptr + 1))
			{
				done = 6;
				break;
			}
		}
		while (*((UINT16 *) cptr)++);
	}

	/* switch back to normal operation */
	fas_outb(fip, MDM_CTL_PORT, 0);

	return (done);
}

#if defined (NEED_PUT_GETCHAR)

int
asyputchar(arg1)
unchar arg1;
{
	register struct fas_info *fip;

	REGVAR;

	if (!fas_is_initted)
		(void)fasinit();

	fip = &fas_info[0];
	if (fip->device_flags.i & DF_DEVICE_CONFIGURED)
	{
		fas_ctl(fip, LINE_STATUS_PORT);
		while (!(inb(LINE_STATUS_PORT.addr) & LS_XMIT_AVAIL))
			;
		fas_outb(fip, XMT_DATA_PORT, arg1);
		if (arg1 == 10)
			(void)asyputchar(13);
	}
	return (0);
}

int
asygetchar()
{
	register struct fas_info *fip;

	REGVAR;

	if (!fas_is_initted)
		(void)fasinit();

	fip = &fas_info[0];
	if ((fip->device_flags.i & DF_DEVICE_CONFIGURED)
		&& (fas_first_inb(fip, LINE_STATUS_PORT) & LS_RCV_AVAIL))
		return (fas_inb(fip, RCV_DATA_PORT));
	else
		return (-1);
}
#endif

#if defined (NEED_INIT8250)

/* reset the requested port to be used directly by a DOS process */
int
init8250(port, ier)
UINT16 port, ier;			 /* ier not used in this stub */
{
	register struct fas_info *fip;
	register UINT physical_unit;
	int old_level;

	REGVAR;

	/*
	 * See if the port address matches a port that is used by the fas
	 * driver.
	 */
	for (physical_unit = 0; physical_unit < fas_physical_units;
		physical_unit++)
		if (port == (UINT16) (fas_port[physical_unit]))
			break;

	if (physical_unit >= fas_physical_units)
		return (-1);		 /* port didn't match */

	fip = fas_info_ptr[physical_unit];

	old_level = SPLINT();

	fip->ier = IE_NONE;
	fas_first_outb(fip, INT_ENABLE_PORT, fip->ier);

	fip->mcr &= ~(fip->flow.m.ic | fip->flow.m.hc);
	fas_outb(fip, MDM_CTL_PORT, fip->mcr);

	if (fip->device_flags.i & DF_DEVICE_IS_NS16550A)
		fas_outb(fip, NS_FIFO_CTL_PORT, NS_FIFO_CLEAR_CMD);
	else if (fip->device_flags.i & DF_DEVICE_IS_I82510)
	{
		fas_outb(fip, I_BANK_PORT, I_BANK_1);
		fas_outb(fip, I_TCM_PORT, I_FIFO_CLR_XMIT);
		fas_outb(fip, I_RCM_PORT, I_FIFO_CLR_RECV);
		fas_outb(fip, I_BANK_PORT, I_BANK_2);
		fas_outb(fip, I_IDM_PORT, I_FIFO_CLEAR_CMD);
		fas_outb(fip, I_BANK_PORT, I_BANK_0);
	}

	(void)fas_inb(fip, MDM_STATUS_PORT);
	(void)fas_inb(fip, RCV_DATA_PORT);
	(void)fas_inb(fip, RCV_DATA_PORT);
	(void)fas_inb(fip, LINE_STATUS_PORT);
	(void)fas_inb(fip, INT_ID_PORT);
	(void)splx(old_level);
	return (0);
}
#endif
