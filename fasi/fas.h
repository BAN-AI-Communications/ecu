/* This file contains various defines for the FAS async driver.
   If you change anything here you have to recompile the driver module.
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
/*:01-20-1991-16:17-wht@n4hgf-add fas_names */
/*:01-20-1991-05:01-wht@n4hgf-changed buffer sizes */

/* Alas, SCO idinstall has no -z (Define) option like ISC does */
#if !defined(FASI)
#define FASI
#endif
#if !defined(SCO)
#define SCO
#endif

#if !defined (M_I286) && !defined(__STDC__)
#ident	"@(#)fas.h	2.08"
#endif

/* Uncomment the following line if you need asyputchar and asygetchar.
   This is only required if you link the kernel without the original
   asy driver and these functions aren't provided by any other kernel
   module.
*/
#if 0
#define NEED_PUT_GETCHAR
#endif

/* Uncomment the following line if you have VP/ix support in the
   kernel.
*/
#if 0
#define HAVE_VPIX
#endif

/* Uncomment the following line if you need init8250. DosMerge needs
   this function, but only if you link the kernel without the original
   asy driver.
*/
#if 0
#define NEED_INIT8250
#endif

#if defined (VPIX)
#undef VPIX
#endif

#if defined (HAVE_VPIX)
#define VPIX
#endif

#if defined (XENIX)
typedef unsigned char unchar;
typedef unsigned long UINT32;

/*
**	Union for use by all device handler ioctl routines.
*/
union ioctl_arg
{
	struct termio *stparg;	 /* ptr to termio struct */
	char *cparg;			 /* ptr to character */
	char carg;				 /* character */
	int *iparg;				 /* ptr to integer */
	int iarg;				 /* integer */
	long *lparg;			 /* ptr to long */
	long larg;				 /* long */
};

#endif

#if defined (TRUE)
#undef TRUE
#endif
#define	TRUE	(1)

#if defined (FALSE)
#undef FALSE
#endif
#define FALSE	(0)

/* Initial line control register.  Value will only be meaningful for
   asyputchar and asygetchar and they are only meaningful if
   NEED_PUT_GETCHAR is defined.
*/
#define	INITIAL_LINE_CONTROL	LC_WORDLEN_8

/* Initial baud rate.  Value will only be meaningful for
   asyputchar and asygetchar and they are only meaningful if
   NEED_PUT_GETCHAR is defined.
*/
#define INITIAL_BAUD_RATE	(BAUD_BASE/9600)

/* Initial modem control register.  This should probably not have to
   be touched.  It is here because some terminals used as the console
   require one or more of the modem signals set. It is only meaningful
   for asyputchar and asygetchar and they are only meaningful if
   NEED_PUT_GETCHAR is defined.
*/
#define INITIAL_MDM_CONTROL	0

/****************************************************/
/* Nothing past this line should have to be changed */
/****************************************************/

#define NUM_INT_VECTORS	32	 /* number of possible int vectors, but only
							  * the first eight are normally used */

#define MAX_UNITS	16		 /* we will only use that many units */

/* Miscellaneous Constants */

#define BAUD_BASE	(1843200 / 16)	/* 115200 bps */
#define HANGUP_DELAY	500	 /* in milli-seconds */
#define HANGUP_TIME	1000	 /* in milli-seconds */
#define RECOVER_TIME	30	 /* in seconds */
#define BREAK_TIME	250		 /* in milli-seconds */
#define EVENT_TIME	20		 /* in milli-seconds */
#if defined (M_I286)
#define	RECV_BUFF_SIZE	1000 /* receiver ring buffer size (MAX) */
#define XMIT_BUFF_SIZE	500	 /* transmitter ring buffer size */
#else
#if defined(FASI)			 /* we'll make do with less */
#define	RECV_BUFF_SIZE	3500 /* receiver ring buffer size (MAX) */
#define XMIT_BUFF_SIZE	500	 /* transmitter ring buffer size */
#else /* FASI */
#define	RECV_BUFF_SIZE	5000 /* receiver ring buffer size (MAX) */
#define XMIT_BUFF_SIZE	2500 /* transmitter ring buffer size */
#endif /* FASI */
#endif /* M_I286 */

#define RBS RECV_BUFF_SIZE

#define SW_LOW_WATER	((int)(RBS*0.5))	/* 50% MAX	sw flow control */
#define SW_HIGH_WATER	((int)(RBS*0.8))	/* 80% MAX	 trigger levels */
#if defined(FASI)			 /* experiment */
#define HW_LOW_WATER	(RBS-300)	/* MAX - 300	hw flow control */
#define HW_HIGH_WATER	(RBS-100)	/* MAX - 100	 trigger levels */
#else
#define HW_LOW_WATER	(RBS-500)	/* MAX - 500	hw flow control */
#define HW_HIGH_WATER	(RBS-300)	/* MAX - 300	 trigger levels */
#endif
#define MAX_UNIX_FILL	(TTYHOG)	/* read buffer max UNIX fill level */
#define MAX_VPIX_FILL	64	 /* read buffer max VP/ix fill level */
#define MIN_READ_CHUNK	32	 /* must be <= MAX_????_FILL/2 */
#define MAX_MSI_CNT	1000	 /* max modem status ints per second */
#define READ_PORT	0x0100	 /* read command for fas_init_seq */
#define NO_FIFO		0x10000	 /* force FIFOs off */
#define SOFT_INIT	0		 /* init registers if cflag changed */
#define HARD_INIT	1		 /* init registers w/o checking cflag */
#if defined (XENIX)
#define SPLWRK		spl5	 /* SPL for character processing */
#define SPLINT		spl7	 /* SPL to disable FAS interrupts */
#else
#define SPLWRK		spl6	 /* SPL for character processing */
#define SPLINT		spltty	 /* SPL to disable FAS interrupts */
#endif

#if ((EVENT_TIME) * (HZ) / 1000) == 0
#undef EVENT_TIME
#define EVENT_TIME	(1000 / (HZ))
#endif

#if (MAX_UNIX_FILL) > (TTYHOG)
#undef MAX_UNIX_FILL
#define MAX_UNIX_FILL	(TTYHOG)
#endif

#if (MAX_VPIX_FILL) > (TTYHOG)
#undef MAX_VPIX_FILL
#define MAX_VPIX_FILL	(TTYHOG)
#endif

#if (MIN_READ_CHUNK) > ((MAX_UNIX_FILL) / 2)
#undef MIN_READ_CHUNK
#define MIN_READ_CHUNK	((MAX_UNIX_FILL) / 2)
#endif

#if (MIN_READ_CHUNK) > ((MAX_VPIX_FILL) / 2)
#undef MIN_READ_CHUNK
#define MIN_READ_CHUNK	((MAX_VPIX_FILL) / 2)
#endif

#define MAX_INPUT_FIFO_SIZE	INPUT_NS_FIFO_SIZE
#define MAX_OUTPUT_FIFO_SIZE	OUTPUT_NS_FIFO_SIZE

/* Here are the modem control flags for the fas_modem array in space.c.
   They are arranged in three 8-bit masks which are combined to a 32-bit
   word. Each of these 32-bit words represents one entry in the fas_modem
   array.

   The lowest byte is used as a mask to manipulate the modem control
   register for modem disable. Use the MC_* macros to build the mask.

   The second lowest byte is used as a mask to manipulate the modem control
   register for modem enable during dialout. Use the MC_* macros to build
   the mask and shift them 8 bits to the left.

   The second highest byte is used as a mask to manipulate the modem control
   register for modem enable during dialin. Use the MC_* macros to build
   the mask and shift them 16 bits to the left.

   The highest byte is used to mask signals from the modem status
   register that will be used as the carrier detect signal. Use the MS_*
   macros to build the mask and shift them 24 bits to the left. If you use
   more than one signal, carrier is considered on only when all signals
   are on.

   Here are some useful macros for the space.c file. You may create your
   own macros if you have some special requirements not met by the
   predefined ones.
*/

/* modem disable (choose one) */
#define DI_RTS			MC_SET_RTS	/* RTS disables modem */
#define DI_DTR			MC_SET_DTR	/* DTR disables modem */
#define DI_RTS_AND_DTR		(MC_SET_RTS | MC_SET_DTR)

/* modem enable for dialout (choose one) */
#define EO_RTS			(MC_SET_RTS << 8)	/* RTS enables modem */
#define EO_DTR			(MC_SET_DTR << 8)	/* DTR enables modem */
#define EO_RTS_AND_DTR		((MC_SET_RTS | MC_SET_DTR) << 8)

/* modem enable for dialin (choose one) */
#define EI_RTS			(MC_SET_RTS << 16)	/* RTS enables modem */
#define EI_DTR			(MC_SET_DTR << 16)	/* DTR enables modem */
#define EI_RTS_AND_DTR		((MC_SET_RTS | MC_SET_DTR) << 16)

/* carrier detect signal (choose one) */
#define CA_DCD			(MS_DCD_PRESENT << 24)	/* DCD is carr. detect */
#define CA_CTS			(MS_CTS_PRESENT << 24)	/* CTS is carr. detect */
#define CA_DSR			(MS_DSR_PRESENT << 24)	/* DSR is carr. detect */

/* Here are the hardware handshake flags for the fas_flow array in space.c.
   They are arranged in three 8-bit masks which are combined to a 32-bit
   word. Each of these 32-bit words represents one entry in the fas_flow
   array.

   The lowest byte is used as a mask to manipulate the modem control
   register for input flow control. Use the MC_* macros to build the mask.

   The second lowest byte is used to mask signals from the modem status
   register that will be used for output flow control. Use the MS_* macros
   to build the mask and shift them 8 bits to the left. If you use more
   than one signal, output is allowed only when all signals are on.

   The second highest byte is used to mask signals from the modem status
   register that will be used to enable the output flow control selected
   by the second lowest byte. Use the MS_* macros to build the mask and
   shift them 16 bits to the left. If you use more than one signal, output
   flow control is enabled only when all signals are on.

   The highest byte is used as a mask to manipulate the modem control
   register for output half duplex flow control. Use the MC_* macros to
   build the mask and shift them 24 bits to the left.

   Here are some useful macros for the space.c file. You may create your
   own macros if you have some special requirements not met by the
   predefined ones.
*/

/* input flow control (choose one) */
#define HI_RTS			MC_SET_RTS	/* RTS input flow ctrl */
#define HI_DTR			MC_SET_DTR	/* DTR input flow ctrl */
#define HI_RTS_AND_DTR		(MC_SET_RTS | MC_SET_DTR)

/* output flow control (choose one) */
#define HO_CTS			(MS_CTS_PRESENT << 8)	/* CTS output flow ctrl */
#define HO_DSR			(MS_DSR_PRESENT << 8)	/* DSR output flow ctrl */
#define HO_CTS_AND_DSR		((MS_CTS_PRESENT | MS_DSR_PRESENT) << 8)
#define HO_CTS_ON_DSR		((MS_CTS_PRESENT << 8) | (MS_DSR_PRESENT << 16))
#define HO_CTS_ON_DSR_AND_DCD	((MS_CTS_PRESENT << 8) \
				| ((MS_DSR_PRESENT | MS_DCD_PRESENT) << 16))

/* output hdx flow control (choose one) */
#define HX_RTS			(MC_SET_RTS << 24)	/* RTS hdx flow ctrl */
#define HX_DTR			(MC_SET_DTR << 24)	/* DTR hdx flow ctrl */
#define HX_RTS_AND_DTR		((MC_SET_RTS | MC_SET_DTR) << 24)

/* define the local open flags */

#define OS_DEVICE_CLOSED	0x0000
#define OS_OPEN_FOR_DIALOUT	0x0001
#define OS_OPEN_FOR_GETTY	0x0002
#define OS_WAIT_OPEN		0x0004
#define OS_NO_DIALOUT		0x0008
#define OS_FAKE_CARR_ON		0x0010
#define OS_CLOCAL		0x0020
#define OS_HWO_HANDSHAKE	0x0040
#define OS_HWI_HANDSHAKE	0x0080
#define OS_HDX_HANDSHAKE	0x0100
#define OS_EXCLUSIVE_OPEN_1	0x0200
#define OS_EXCLUSIVE_OPEN_2	0x0400	/* SYSV 3.2 Xenix compatibility */

#define OS_OPEN_STATES		(OS_OPEN_FOR_DIALOUT | OS_OPEN_FOR_GETTY)
#define OS_TEST_MASK		(OS_OPEN_FOR_DIALOUT | OS_NO_DIALOUT \
				| OS_FAKE_CARR_ON | OS_CLOCAL \
				| OS_HWO_HANDSHAKE | OS_HWI_HANDSHAKE \
				| OS_HDX_HANDSHAKE | OS_EXCLUSIVE_OPEN_1 \
				| OS_EXCLUSIVE_OPEN_2)
#define OS_SU_TEST_MASK		(OS_OPEN_FOR_DIALOUT | OS_NO_DIALOUT \
				| OS_FAKE_CARR_ON | OS_CLOCAL \
				| OS_HWO_HANDSHAKE | OS_HWI_HANDSHAKE \
				| OS_HDX_HANDSHAKE | OS_EXCLUSIVE_OPEN_1)

/* define the device status flags */

#define DF_DEVICE_CONFIGURED	0x0001	/* device is configured */
#define DF_DEVICE_IS_NS16550A	0x0002	/* it's an NS16550A */
#define DF_DEVICE_IS_I82510	0x0004	/* it's an I82510 */
#define DF_CTL_FIRST		0x0008	/* write ctl port at first access */
#define DF_CTL_EVERY		0x0010	/* write ctl port at every access */
#define DF_DEVICE_OPEN		0x0020	/* physical device is open */
#define DF_DEVICE_LOCKED	0x0040	/* physical device locked */
#define DF_MODEM_ENABLED	0x0080	/* modem enabled */
#define DF_XMIT_BUSY		0x0100	/* transmitter busy */
#define DF_XMIT_BREAK		0x0200	/* transmitter sends break */
#define DF_XMIT_LOCKED		0x0400	/* transmitter locked against output */
#define DF_DO_HANGUP		0x0800	/* delayed hangup request */
#define DF_DO_BREAK		0x1000	/* delayed break request */
#define DF_GUARD_TIMEOUT	0x2000	/* protect last char from corruption */
#define DF_NS16550A_DROP_MODE	0x4000	/* receiver trigger level is
										 * dropped */

/* define the flow control status flags */

#define FF_HWO_HANDSHAKE	0x0001	/* output hw handshake enabled */
#define FF_HWI_HANDSHAKE	0x0002	/* input hw handshake enabled */
#define FF_HDX_HANDSHAKE	0x0004	/* output hdx hw handshake enabled */
#define	FF_HWO_STOPPED		0x0008	/* output stopped by hw handshake */
#define FF_HWI_STOPPED		0x0010	/* input stopped by hw handshake */
#define FF_HDX_STARTED		0x0020	/* output buffer contains characters */
#define FF_SWO_STOPPED		0x0040	/* output stopped by sw flow control */
#define FF_SWI_STOPPED		0x0080	/* input stopped by sw flow control */
#define FF_SW_FC_REQ		0x0100	/* sw input flow control request */
#define FF_RXFER_STOPPED	0x0200	/* rxfer function stopped */

/* define the scheduled events flags */

#define EF_DO_RXFER		0x0001	/* rxfer function request */
#define EF_DO_XXFER		0x0002	/* xxfer function request */
#define EF_DO_BRKINT		0x0004	/* break int request */
#define EF_DO_MPROC		0x0008	/* mproc function request */
#define EF_SIGNAL_VPIX		0x0010	/* send pseudorupt to VP/ix */

/* define an easy way to reference the port structures */

#define RCV_DATA_PORT		(fip->port_0)
#define XMT_DATA_PORT		(fip->port_0)
#define INT_ENABLE_PORT		(fip->port_1)
#define INT_ID_PORT		(fip->port_2)
#define NS_FIFO_CTL_PORT	(fip->port_2)
#define I_BANK_PORT		(fip->port_2)
#define LINE_CTL_PORT		(fip->port_3)
#define MDM_CTL_PORT		(fip->port_4)
#define I_IDM_PORT		(fip->port_4)
#define LINE_STATUS_PORT	(fip->port_5)
#define I_RCM_PORT		(fip->port_5)
#define MDM_STATUS_PORT		(fip->port_6)
#define I_TCM_PORT		(fip->port_6)
#define DIVISOR_LSB_PORT	(fip->port_0)
#define DIVISOR_MSB_PORT	(fip->port_1)
#define CTL_PORT		(fip->ctl_port)

/* modem control port */

#define MC_SET_DTR		0x01
#define MC_SET_RTS		0x02
#define MC_SET_OUT1		0x04
#define MC_SET_OUT2		0x08 /* tristates int line when false */
#define MC_SET_LOOPBACK		0x10

#define MC_ANY_CONTROL	(MC_SET_DTR | MC_SET_RTS)

/* modem status port */

#define MS_CTS_DELTA		0x01
#define MS_DSR_DELTA		0x02
#define MS_RING_TEDGE		0x04
#define MS_DCD_DELTA		0x08
#define MS_CTS_PRESENT		0x10
#define MS_DSR_PRESENT		0x20
#define MS_RING_PRESENT		0x40
#define MS_DCD_PRESENT		0x80

#define MS_ANY_DELTA	(MS_CTS_DELTA | MS_DSR_DELTA | MS_RING_TEDGE \
				| MS_DCD_DELTA)
#define MS_ANY_PRESENT	(MS_CTS_PRESENT | MS_DSR_PRESENT | MS_RING_PRESENT \
				| MS_DCD_PRESENT)

/* interrupt enable port */

#define IE_NONE				0x00
#define	IE_RECV_DATA_AVAILABLE		0x01
#define	IE_XMIT_HOLDING_BUFFER_EMPTY	0x02
#define IE_LINE_STATUS			0x04
#define IE_MODEM_STATUS			0x08

#define IE_INIT_MODE	(IE_RECV_DATA_AVAILABLE | IE_XMIT_HOLDING_BUFFER_EMPTY \
			| IE_LINE_STATUS | IE_MODEM_STATUS)

/* interrupt id port */

#define II_NO_INTS_PENDING	0x01
#define II_CODE_MASK		0x07
#define II_MODEM_STATE		0x00
#define II_XMTD_CHAR		0x02
#define II_RCVD_CHAR		0x04
#define II_RCV_ERROR		0x06
#define II_NS_FIFO_TIMEOUT	0x08
#define II_NS_FIFO_ENABLED	0xC0

/* line control port */

#define	LC_WORDLEN_MASK		0x03
#define	LC_WORDLEN_5		0x00
#define	LC_WORDLEN_6		0x01
#define	LC_WORDLEN_7		0x02
#define	LC_WORDLEN_8		0x03
#define LC_STOPBITS_LONG	0x04
#define LC_ENABLE_PARITY	0x08
#define LC_EVEN_PARITY		0x10
#define LC_STICK_PARITY		0x20
#define LC_SET_BREAK_LEVEL	0x40
#define LC_ENABLE_DIVISOR	0x80

/* line status port */

#define LS_RCV_AVAIL		0x01
#define LS_OVERRUN		0x02
#define LS_PARITY_ERROR		0x04
#define LS_FRAMING_ERROR	0x08
#define LS_BREAK_DETECTED	0x10
#define LS_XMIT_AVAIL		0x20
#define LS_XMIT_COMPLETE	0x40
#define LS_ERROR_IN_NS_FIFO	0x80	/* NS16550A only */

#define LS_RCV_INT	(LS_RCV_AVAIL | LS_OVERRUN | LS_PARITY_ERROR \
			| LS_FRAMING_ERROR | LS_BREAK_DETECTED)

/* fifo control port (NS16550A only) */

#define	NS_FIFO_ENABLE		0x01
#define	NS_FIFO_CLR_RECV	0x02
#define	NS_FIFO_CLR_XMIT	0x04
#define	NS_FIFO_START_DMA	0x08
#define NS_FIFO_SIZE_1		0x00
#define NS_FIFO_SIZE_4		0x40
#define NS_FIFO_SIZE_8		0x80
#define NS_FIFO_SIZE_14		0xC0
#define NS_FIFO_SIZE_MASK	0xC0

#define NS_FIFO_CLEAR_CMD	0
#define NS_FIFO_DROP_CMD	(NS_FIFO_SIZE_1 | NS_FIFO_ENABLE)
#define NS_FIFO_SETUP_CMD	(NS_FIFO_SIZE_4 | NS_FIFO_ENABLE)
#define NS_FIFO_INIT_CMD	(NS_FIFO_SETUP_CMD | NS_FIFO_CLR_RECV \
				| NS_FIFO_CLR_XMIT)

#define INPUT_NS_FIFO_SIZE	16
#define OUTPUT_NS_FIFO_SIZE	16

/* fifo control ports (i82510 only) */

#define I_BANK_0		0x00
#define I_BANK_1		0x20
#define I_BANK_2		0x40
#define I_BANK_3		0x60
#define I_FIFO_ENABLE		0x08
#define I_FIFO_CLR_RECV		0x30
#define I_FIFO_CLR_XMIT		0x0c

#define I_FIFO_CLEAR_CMD	0
#define I_FIFO_SETUP_CMD	I_FIFO_ENABLE

#define INPUT_I_FIFO_SIZE	4
#define OUTPUT_I_FIFO_SIZE	4

/* defines for ioctl calls (VP/ix) */

#define AIOC			('A'<<8)
#define AIOCINTTYPE		(AIOC|60)	/* set interrupt type */
#define AIOCDOSMODE		(AIOC|61)	/* set DOS mode */
#define AIOCNONDOSMODE		(AIOC|62)	/* reset DOS mode */
#define AIOCSERIALOUT		(AIOC|63)	/* serial device data write */
#define AIOCSERIALIN		(AIOC|64)	/* serial device data read */
#define AIOCSETSS		(AIOC|65)	/* set start/stop chars */
#define AIOCINFO		(AIOC|66)	/* tell us what device we are */

/* ioctl alternate names used by VP/ix */

#define VPC_SERIAL_DOS		AIOCDOSMODE
#define VPC_SERIAL_NONDOS	AIOCNONDOSMODE
#define VPC_SERIAL_INFO		AIOCINFO
#define VPC_SERIAL_OUT		AIOCSERIALOUT
#define VPC_SERIAL_IN		AIOCSERIALIN

#if defined(FASI)
#define FASIC			('~' << 8)
#define FASIC_SIP		(FASIC | 16)	/* get entire fas_info struct */
#define FASIC_SIP_CHANGE	(FASIC | 17)	/* get entire fas_info struct
											 * after wait for change */
#define FASIC_MSR		(FASIC | 18)	/* get various registers */
#define FASIC_LCR		(FASIC | 19)
#define FASIC_IER		(FASIC | 20)
#define FASIC_MCR		(FASIC | 21)
#define FASIC_DVR_IDENT		(FASIC | 22)	/* get driver revision */
#define FASIC_SPACE_IDENT	(FASIC | 23)	/* get space.c revision */
#define FASIC_RESET_STAT	(FASIC | 24)	/* reset statistics */
#endif /* FASI */

/* serial in/out requests */

#define SO_DIVLLSB		1
#define SO_DIVLMSB		2
#define SO_LCR			3
#define SO_MCR			4
#define SI_MSR			1
#define SIO_MASK(x)		(1<<((x)-1))

/* This structure contains everything one would like to know about
   an open device.  There is one of it for each physical unit.

   We use several unions to eliminate most integer type conversions
   at run-time. The standard UNIX V 3.X/386 C compiler forces all
   operands in expressions and all function parameters to type int.
   To save some time, with the means of unions we deliver type int
   at the proper locations while dealing with the original type
   wherever int would be slower.

   This is highly compiler implementation specific. But for the sake
   of speed the end justifies the means.

   Take care that the size of the area that contains the various
   structure fields (up to, but excluding the ring buffers)
   is <= 128 bytes. Otherwise a 4-byte offset is used to access
   some of the structure fields. For the first 128 bytes a 1-byte
   offset is used, which is faster.
*/

struct fas_info
{
	struct tty *tty;		 /* the tty structure */
	struct fas_info *prev_int_user;	/* link to previous fas_info struct */
	struct fas_info *next_int_user;	/* link to next fas_info struct */
	int timeout_idx;		 /* timeout index for untimeout () */
	UINT iflag;				 /* current terminal input flags */
	UINT cflag;				 /* current terminal hardware control flags */
	union
	{						 /* flags about the device state */
		UINT16 s;
		UINT i;
	} device_flags;
	union
	{						 /* flags about the flow control state */
		UINT16 s;
		UINT i;
	} flow_flags;
	union
	{						 /* flags about the scheduled events */
		UINT16 s;
		UINT i;
	} event_flags;
	UINT o_state;			 /* current open state */
	UINT po_state;			 /* previous open state */
	union
	{						 /* modem control masks */
		struct
		{
			unchar di;		 /* mask for modem disable */
			unchar eo;		 /* mask for modem enable (dialout) */
			unchar ei;		 /* mask for modem enable (dialin) */
			unchar ca;		 /* mask for carrier detect */
		} m;
		UINT32 l;
	} modem;
	union
	{						 /* hardware flow control masks */
		struct
		{
			unchar ic;		 /* control mask for inp. flow ctrl */
			unchar oc;		 /* control mask for outp. flow ctrl */
			unchar oe;		 /* enable mask for outp. flow ctrl */
			unchar hc;		 /* control mask for hdx flow ctrl */
		} m;
		UINT32 l;
	} flow;
	unchar msr;				 /* modem status register value */
	unchar new_msr;			 /* new modem status register value */
	unchar mcr;				 /* modem control register value */
	unchar lcr;				 /* line control register value */
	unchar ier;				 /* interrupt enable register value */
	unchar vec;				 /* interrupt vector for this struct */
	unchar msi_cnt;			 /* modem status interrupt counter */
#if defined (HAVE_VPIX)
	unchar v86_intmask;		 /* VP/ix pseudorupt mask */
	v86_t *v86_proc;		 /* VP/ix v86proc pointer for pseudorupts */
	struct termss v86_ss;	 /* VP/ix start/stop characters */
#endif
	UINT ctl_port;			 /* muliplexer control port */
	union
	{						 /* uart port addresses and control values */
		UINT addr;
		struct
		{
			UINT16 addr;
			unchar ctl;
		} p;
	} port_0, port_1, port_2, port_3, port_4, port_5, port_6;
	UINT recv_ring_cnt;		 /* receiver ring buffer counter */
	unchar *recv_ring_put_ptr;	/* recv ring buf put ptr */
	unchar *recv_ring_take_ptr;	/* recv ring buf take ptr */
	UINT16 xmit_fifo_size;	 /* transmitter FIFO size */
	UINT16 xmit_ring_size;	 /* transmitter ring buffer size */
	UINT xmit_ring_cnt;		 /* transmitter ring buffer counter */
	unchar *xmit_ring_put_ptr;	/* xmit ring buf put ptr */
	unchar *xmit_ring_take_ptr;	/* xmit ring buf take ptr */
#if defined(FASI)
	unsigned long characters_received;
	unsigned long characters_transmitted;
	unsigned long modem_status_events;
	unsigned long overrun_errors;
	unsigned long framing_errors;
	unsigned long parity_errors;
	unsigned long rings_detected;
	unsigned long breaks_detected;
	unsigned long xmtr_hw_flow_count;
	unsigned long xmtr_sw_flow_count;
	unsigned long rcvr_hw_flow_count;
	unsigned long rcvr_sw_flow_count;
#endif /* FASI */
	unchar recv_buffer[RECV_BUFF_SIZE];	/* recv ring buf */
	unchar xmit_buffer[XMIT_BUFF_SIZE];	/* xmit ring buf */
};

#if defined(FASI)
struct fas_name
{
	char name[8];
};

#endif
