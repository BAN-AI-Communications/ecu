/* CHK=0xF1CF */
/*+-------------------------------------------------------------------------
	morse_dvr.h -- morse driver driver/user equates
--------------------------------------------------------------------------*/
/*+:EDITS:*/
/*:01-24-1997-02:38-wht@yuriatin-SOURCE RELEASE 4.00 */
/*:11-08-1990-13:26-wht@n4hgf-add morse_silent and friends */
/*:11-06-1988-18:54-wht-creation */

/* ioctl(morse_fd,MORSE_??,&integer_arg); */
#define MORSE_SET_SPEED			0x80	/* arg in Hz */
#define MORSE_SET_FREQUENCY		0x81	/* arg in tone duration in ticks */
/* the next two have dummy arguments */
#define MORSE_ENABLE			0x82	/* enable code generation */
#define MORSE_DISABLE			0x83	/* disable code generation */

/* non-ascii character equates */
#define AA		0x01	/* .-.- */
#define BT		0x02	/* -...- */
#define SK		0x1B	/* ...-.- */
