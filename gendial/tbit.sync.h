/*+-------------------------------------------------------------------------
	sync_Telebit() - sync modem with our DTE speed
--------------------------------------------------------------------------*/
/*+:EDITS:*/
/*:01-24-1997-02:37-wht@yuriatin-SOURCE RELEASE 4.00 */
/*:09-11-1996-20:00-wht@yuriatin-3.48-major telnet,curses,structural overhaul */
/*:11-23-1995-11:20-wht@kepler-source control 3.37 for tsx-11 */
/*:11-14-1995-10:23-wht@kepler-3.37.80-source control point: SOCKETS */
/*:05-04-1994-04:39-wht@n4hgf-ECU release 3.30 */
/*:09-10-1992-13:59-wht@n4hgf-ECU release 3.20 */
/*:08-22-1992-15:38-wht@n4hgf-ECU release 3.20 BETA */
/*:03-30-1992-14:18-root@n4hgf-add lbreak before wakeup */
void
sync_Telebit()
{
	register int maxretry = 4;
	register int count;
	unsigned char rdchar;
	long pace_msec_save = DCE_write_pace_msec;

	while (--maxretry)
	{
		lbreak();
		Nap(200L);
		DCE_write_pace_msec = 50;
		lwrite("aaaaaate1\r");
		DCE_write_pace_msec = pace_msec_save;
		Nap(200L);
		lflush();
		count = 5;
		while (count)		 /* wait 120-200 msec for character, depending
							  * on HZ */
		{
			write(dce_fd, "a", 1);
			ioctl(dce_fd, TCSETAW, &dce_termio);	/* wait for I/O to drain */
			Nap(120L);		 /* must handle 110 baud */
			if (Rdchk(dce_fd))
				break;
			count--;
		}
		rdchar = 0;
		if (count && (read(dce_fd, &rdchar, 1) == 1) && ((rdchar & 0x7F) == 'a'))
		{
			Nap(120L);
			lflush();
			return;
		}
		DEBUG(2, "Telebit SYNC failed ... retrying (%02x)\n", rdchar);
		lflash_DTR();
	}

	DEBUG(1, "Telebit SYNC FAILED\n", 0);
	myexit(RC_FAIL | RCE_TIMOUT);

}							 /* end of sync_Telebit */
