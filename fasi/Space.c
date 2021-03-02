/* Async device configuration file for the FAS async driver. */

/*
 * COM1(STD) + COM2(DIGIBOARD PC-8)
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
/*:01-20-1991-16:17-wht@n4hgf-add fas_names */
/*:01-20-1991-05:01-wht@n4hgf-changed buffer sizes */
/*:01-16-1991-22:13-wht@n4hgf-creation */

/* FAS was developed by ( ==> BUT DO NOT CONTACT HIM ABOUT THIS HACK )
Uwe Doering             INET : gemini@geminix.in-berlin.de
Billstedter Pfad 17 b   UUCP : ...!unido!fub!geminix.in-berlin.de!gemini
1000 Berlin 20
Germany
*/

/* Alas, SCO idinstall has no -z (Define) option like ISC does */
#if !defined(FASI)
#define FASI
#endif
#if !defined(SCO)
#define SCO
#endif

#if defined(FASI)
/* {quan,irq,addr1-addr2,type} */
char *fasi_space_ident =
"FAS/i 2.08:{1,4,03f8-03ff,COM1},{8,3,0210-024f,DIGI-PC8}";

#endif /* FASI */

#if !defined (M_I286) && !defined(__STDC__) && !defined(__GNUC__)
#ident	"@(#)space.c	2.08.0 COM1(STD) + COM2(DIGIBOARD PC-8)";
#endif

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
#include "digi-pc8.h"
#else
#include <local/fas.h>
#include <local/digi-pc8.h>
#endif

/* This is the number of devices to be handled by this driver.
   You may define up to 16 devices.  If this number is changed
   the arrays below must be filled in accordingly.
*/
#define NUM_PHYSICAL_UNITS	9

#if NUM_PHYSICAL_UNITS > MAX_UNITS
#undef NUM_PHYSICAL_UNITS
#define NUM_PHYSICAL_UNITS	MAX_UNITS
#endif

/* let the driver know the number of devices */
UINT fas_physical_units = NUM_PHYSICAL_UNITS;

/* array of base port addresses
   If you deliberately want to force off the FIFOs of a UART you have
   to "or" the NO_FIFO macro to its base port address. This is useful
   for mouse devices where you need immediate response to the mouse
   movement.
*/
UINT32 fas_port[NUM_PHYSICAL_UNITS] =
{
	0x3f8,
	COM21, COM22, COM23, COM24, COM25, COM26, COM27, COM28
};

/*
 * array of port names
 * Note this is a kludge to enable kmem seeking programs to
 * determine which tty is associated with which tty struct
 * and is <yetch> duplication of information appearing in
 * the Node (/etc/node.d/fas) file
 */
#if defined(FASI)
struct fas_name fas_names[NUM_PHYSICAL_UNITS * 2] =
{
	{"1a"},
	{"2a"},
	{"2b"},
	{"2c"},
	{"2d"},
	{"2e"},
	{"2f"},
	{"2g"},
	{"2h"},
	{"1A"},
	{"2A"},
	{"2B"},
	{"2C"},
	{"2D"},
	{"2E"},
	{"2F"},
	{"2G"},
	{"2H"}
};

#endif

/* array of interrupt vectors */
UINT fas_vec[NUM_PHYSICAL_UNITS] =
{
	4,
	3, 3, 3, 3, 3, 3, 3, 3
};

/* initialization sequence for serial card
   This array contains pairs of values of the form:

        portaddress, value,
              :
              :
        portaddress, value,
        0

   For every line `value' will be written to `portaddress'. If
   `value' is replaced with the macro `READ_PORT' then a value
   is read from `portaddress' instead. The value itself will be
   discarded. Therefor this makes only sense if the read access
   to the port has a side effect like setting or resetting
   certain flags.

   NOTE: This array *must* be terminated with a value of 0
         in the portaddress column!
*/
UINT fas_init_seq[] =
{
	0
};

/* initial modem control port info
   This value is ored into the modem control value for each UART. This is
   normaly used to force out2 which is used to enable the interrupts of
   the standard com1 and com2 ports. Several brands of cards have modes
   that allow them to work in compatible mode like com1 and com2 or as a
   shared interrupts card. One of these cards is the AST 4-port card. When
   this card is used in shared interrupts mode out2 must _not_ be set.

   Note: This is one of the major trouble-spots with shared interrupts
   cards. Check your manual.
*/
UINT fas_mcb[NUM_PHYSICAL_UNITS] =
{
	MC_SET_OUT2,
	MC_SET_OUT2, MC_SET_OUT2, MC_SET_OUT2, MC_SET_OUT2,
	MC_SET_OUT2, MC_SET_OUT2, MC_SET_OUT2, MC_SET_OUT2
};

/* array of modem control flags
   You can choose which signals to use for modem control. See fas.h
   for possible names and values. Whether or not modem control is
   used is determined by the minor device number at open time.
*/
UINT32 fas_modem[NUM_PHYSICAL_UNITS] =
{
	EO_DTR | EI_DTR | CA_DCD,
	EO_DTR | EI_DTR | CA_DCD, EO_DTR | EI_DTR | CA_DCD,
	EO_DTR | EI_DTR | CA_DCD, EO_DTR | EI_DTR | CA_DCD,
	EO_DTR | EI_DTR | CA_DCD, EO_DTR | EI_DTR | CA_DCD,
	EO_DTR | EI_DTR | CA_DCD, EO_DTR | EI_DTR | CA_DCD
};

/* array of hardware flow control flags
   You can choose which signals to use for hardware handshake. See fas.h
   for possible names and values. Whether or not hardware handshake is
   used is determined by the minor device number at open time and by the
   RTSFLOW/CTSFLOW termio(7) flags.
*/
UINT32 fas_flow[NUM_PHYSICAL_UNITS] =
{
	HI_RTS | HO_CTS_ON_DSR | HX_RTS,
	HI_RTS | HO_CTS_ON_DSR | HX_RTS,
	HI_RTS | HO_CTS_ON_DSR | HX_RTS,
	HI_RTS | HO_CTS_ON_DSR | HX_RTS,
	HI_RTS | HO_CTS_ON_DSR | HX_RTS,
	HI_RTS | HO_CTS_ON_DSR | HX_RTS,
	HI_RTS | HO_CTS_ON_DSR | HX_RTS,
	HI_RTS | HO_CTS_ON_DSR | HX_RTS,
	HI_RTS | HO_CTS_ON_DSR | HX_RTS
};

/* array of control register addresses
   There are serial boards available that have all serial ports
   multiplexed to one address location in order to save I/O address
   space (Bell Tech HUB-6 card etc.). This multiplexing is controlled
   by a special register that needs to be written to before the actual
   port registers can be accessed. This array contains the addresses
   of these special registers.
   Enter the addresses on a per unit base. An address of zero
   disables this feature.
*/
UINT fas_ctl_port[NUM_PHYSICAL_UNITS] =
{
	0,
	0, 0, 0, 0, 0, 0, 0, 0
};

/* array of control register values
   These values are written to the corresponding control register
   before the first access to the actual port registers. If not only
   entire UART chips (blocks of 8 contiguous addresses) but even the
   single registers of the UART chips need to be multiplexed to one
   address you have to "or" a bit mask (shifted 8 times to the left)
   to the control register value. This mask determines at which bit
   locations the UART chip register number is "xored" into the control
   register value at runtime. This implies that you can also use
   negative logic by setting the bits in the control register value
   to 1 at the locations corresponding to the bit mask.
*/
UINT fas_ctl_val[NUM_PHYSICAL_UNITS] =
{
	0,
	0, 0, 0, 0, 0, 0, 0, 0
};

/* additional configurations for shared interrupts boards
   If you have a shared interrupts board, you may have to acknowledge
   interrupts by writing to a special register. The following arrays
   contain the special register addresses and the corresponding values
   that are written to them in response to an interrupt.
*/

/* array of int ack register addresses
   These registers are written to every time after all interrupt
   sources in all of the UARTs that are tied to the corresponding
   interrupt vector have been cleared.
   Enter the addresses on a per vector base. An address of zero
   disables this feature.
*/
UINT fas_int_ack_port[NUM_INT_VECTORS] =
{
	0, 0, 0, 0,
	0, 0, 0, 0,
	0, 0, 0, 0,
	0, 0, 0, 0,
	0, 0, 0, 0,
	0, 0, 0, 0,
	0, 0, 0, 0,
	0, 0, 0, 0
};

/* array of int ack values
   These values are written to the corresponding int ack register
   in response to an interrupt.
*/
UINT fas_int_ack[NUM_INT_VECTORS] =
{
	0, 0, 0, 0,
	0, 0, 0, 0,
	0, 0, 0, 0,
	0, 0, 0, 0,
	0, 0, 0, 0,
	0, 0, 0, 0,
	0, 0, 0, 0,
	0, 0, 0, 0
};

/* NOTHING NEEDS TO BE CHANGED BELOW THIS LINE.
   ============================================
*/

/* array of structures to hold all info for a physical minor device */
struct fas_info fas_info[NUM_PHYSICAL_UNITS];

/* array of ttys for logical minor devices */
struct tty fas_tty[NUM_PHYSICAL_UNITS * 2];

/* array of pointers to fas_info structures
   this prevents time consuming multiplications for index calculation
*/
struct fas_info *fas_info_ptr[NUM_PHYSICAL_UNITS];

/* array of pointers to fas_tty structures
   this prevents time consuming multiplications for index calculation
*/
struct tty *fas_tty_ptr[NUM_PHYSICAL_UNITS * 2];
