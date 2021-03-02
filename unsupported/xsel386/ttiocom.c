
#include <sys/types.h>
#include <sys/tty.h>
#include <sys/select.h>

ttiocom(ttyp, com, arg, flag)
struct tty *ttyp;
int com, arg, flag;			 /* there should be better types for this :-) */
{
	if (com == IOC_SELECT)
	{
		ttselect(ttyp, flag);
		return (0);			 /*** THIS IS IMPORTANT ***/
	}
	return (Ttiocom(ttyp, com, arg, flag));
}
