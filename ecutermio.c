/*+-------------------------------------------------------------------------
	ecutermio.c

  Defined functions:
	ecubreak(fd)
	ecudrain(fd)
	ecuflow(fd, command)
	ecuflush(fd, flush_type)
	ecugetattr(fd, t)
	ecugetspeed(t)
	ecusetattr(fd, flag, t)
	ecusetspeed(t, speed)

  "Tell the moon; don't tell the March Hare:  He is here to look around."
  -- Yes.

--------------------------------------------------------------------------*/
/*+:EDITS:*/
/*:04-26-2000-11:15-wht@bob-RELEASE 4.42 */
/*:01-24-1997-02:37-wht@yuriatin-SOURCE RELEASE 4.00 */
/*:09-11-1996-20:00-wht@yuriatin-3.48-major telnet,curses,structural overhaul */
/*:11-23-1995-11:20-wht@kepler-source control 3.37 for tsx-11 */
/*:11-14-1995-10:23-wht@kepler-3.37.80-source control point: SOCKETS */
/*:10-14-1995-17:08-wht@kepler-'struct termio' to 'struct TERMIO' */
/*:03-06-1995-17:43-wht@n4hgf-sort out "standard" POSIX tcsendbreak() */
/*:05-04-1994-04:39-wht@n4hgf-ECU release 3.30 */
/*:03-13-1994-18:20-wht@fep-only CFG_TermiosLineio needed */
/*:03-13-1994-18:18-wht@fep-use cfsetispeed/cfsetospeed in lieu of cfsetspeed */
/*:03-13-1994-18:18-wht@fep-first use of edit history */
/*:12-19-1994-12:00-dharris-ecudrain added */
/*:12-18-1994-12:00-dharris-transplant into 3.28.06 */
/*:12-16-1994-12:00-dharris-creation */

#include "ecu.h"

/*+-------------------------------------------------------------------------
	ecugetattr(fd, t)
--------------------------------------------------------------------------*/
int
ecugetattr(fd, t)
int fd;
struct TERMIO *t;
{
#if defined(CFG_TermiosLineio)
	return (tcgetattr(fd, t));
#else
	return (ioctl(fd, TCGETA, t));
#endif
}							 /* end of ecugetattr */

/*+-------------------------------------------------------------------------
	ecusetattr(fd, flag, t)
--------------------------------------------------------------------------*/
int
ecusetattr(fd, flag, t)
int fd, flag;
struct TERMIO *t;
{
#if defined(CFG_TermiosLineio)
	return (tcsetattr(fd, flag, t));
#else
	return (ioctl(fd, flag, (char *)t));
#endif
}							 /* end of ecusetattr */

/*+-------------------------------------------------------------------------
	ecuflow(fd, command)
--------------------------------------------------------------------------*/
int
ecuflow(fd, command)
int fd, command;
{
#if defined(CFG_TermiosLineio)
	return (tcflow(fd, command));
#else
	return (ioctl(fd, TCXONC, command));
#endif
}							 /* end of ecuflow */

/*+-------------------------------------------------------------------------
	ecuflush(fd, flush_type)
--------------------------------------------------------------------------*/
int
ecuflush(fd, flush_type)
int fd, flush_type;
{
#if defined(CFG_TermiosLineio)
	return (tcflush(fd, flush_type));
#else
	return (ioctl(fd, TCFLSH, flush_type));
#endif
}							 /* end of ecuflush */

/*+-------------------------------------------------------------------------
	ecubreak(fd)
--------------------------------------------------------------------------*/
int
ecubreak(fd)
int fd;
{
#if defined(CFG_TermiosLineio)
#if defined(sun) && defined(SVR4)	/* Solaris */

	/*
	 * Solaris 2.3 man page says:
	 * 
	 * Line Control If the terminal is using asynchronous serial data
	 * transmis- sion,  the  tcsendbreak()  function causes transmission
	 * of a continuous stream of zero-valued bits for a  specific  dura-
	 * tion.   If duration is zero, it causes transmission of zero- valued
	 * bits for at least 0.25 seconds, and not more than 0.5 seconds.  If
	 * duration is not zero, it behaves in a way simi- lar to tcdrain().
	 * ... The tcdrain() function waits until all output written to the
	 * object referred to by fildes has been transmitted.
	 */

	return (tcsendbreak(fd, 0));
#else
#if defined(sun) && !defined(SVR4)	/* SunOS 4.1.x */

	/*
	 * SunOS man page Last change: 21 January 1990
	 * 
	 * If the terminal is using asynchronous serial data  transmis- sion,
	 * tcsendbreak()  transmits a continuous stream of zero- valued bits
	 * for a specific duration.  If duration  is  zero, it transmits
	 * zero-valued bits for at least 0.25 seconds, and not more that 0.5
	 * seconds.  If  duration  is  not  zero,  it sends zero-valued bits
	 * for duration*N seconds, where N is at least 0.25, and not more than
	 * 0.5.
	 */
	return (tcsendbreak(fd, 0));
#else

	/*
	 * use the dharris value
	 */
	return (tcsendbreak(fd, 250000L));
#endif
#endif

#else
	return (ioctl(fd, TCSBRK, (char *)0));
#endif
}							 /* end of ecubreak */

/*+-------------------------------------------------------------------------
	ecudrain(fd)
--------------------------------------------------------------------------*/
int
ecudrain(fd)
int fd;
{
#if defined(CFG_TermiosLineio)
	return (tcdrain(fd));
#else
	return (ioctl(fd, TCSBRK, 1));
#endif
}							 /* end of ecudrain */

/*+-------------------------------------------------------------------------
	ecugetspeed(t)
--------------------------------------------------------------------------*/
int
ecugetspeed(t)
struct TERMIO *t;
{
#if defined(CFG_TermiosLineio)
	return (cfgetispeed(t));
#else
	return ((unsigned)t->c_cflag & CBAUD);
#endif
}							 /* end of ecugetspeed */

/*+-------------------------------------------------------------------------
	ecusetspeed(t, speed)
--------------------------------------------------------------------------*/
void
ecusetspeed(t, speed)
struct TERMIO *t;
int speed;
{

	/*
	 * we use termios on sunos 4.1.x, but cfsetspeed() is not defined
	 * Actually, in at least Solaris 2.3, it isn't here either.... RLipe
	 * [not in Motorola SVR4.1 either; use i/o pair everywhere -- wht]
	 */
#if defined(CFG_TermiosLineio)
#if 1
	cfsetospeed(t, speed);
	cfsetispeed(t, speed);
#else
	cfsetspeed(t, speed);
#endif /* decommitted */
#else
	t->c_cflag &= ~CBAUD;
	t->c_cflag |= speed;
#endif /* CFG_TermiosLineio */
}							 /* end of ecusetspeed */

/* vi: set tabstop=4 shiftwidth=4: */
/* end of ecutermio.c */
