/*+-------------------------------------------------------------------------
	ecupde.c - various PDE-related functions
	wht@wht.net

  Defined functions:
	call_logical_telno(logical)
	choose_tty_for_pde(tpde)
	copy_pde_to_Lvariables(tpde, trial)
	logical_telno_to_pde(logical)
	pde_dial(tpde)
	pdetty_to_devtty(pdetty, devtty)

--------------------------------------------------------------------------*/
/*+:EDITS:*/
/*:04-26-2000-11:15-wht@bob-RELEASE 4.42 */
/*:01-24-1997-02:37-wht@yuriatin-SOURCE RELEASE 4.00 */
/*:09-16-1996-05:21-wht@yuriatin-move phdir_list_read to ecuphdir.c */
/*:09-11-1996-20:00-wht@yuriatin-3.48-major telnet,curses,structural overhaul */
/*:09-09-1996-20:51-wht@yuriatin-pde_dial handles telnet */
/*:08-20-1996-12:39-wht@kepler-locale/ctype fixes from ache@nagual.ru */
/*:08-11-1996-03:22-wht@kepler-use vlogevent */
/*:12-12-1995-14:40-wht@kepler-preserve trailing period in logical telno */
/*:11-23-1995-11:20-wht@kepler-source control 3.37 for tsx-11 */
/*:11-14-1995-10:23-wht@kepler-3.37.80-source control point: SOCKETS */
/*:11-03-1995-16:53-wht@wwtp1-use CFG_TelnetOption */
/*:09-17-1995-16:43-wht@kepler-ECU on Linux does not support dcdw==1 */
/*:08-27-1995-06:53-wht@n4hgf-handle shm->Lrtsctsval and pde->rtscts_val */
/*:06-15-1995-07:41-wht@kepler-ensure new tty exists before lclose other */
/*:05-04-1994-04:38-wht@n4hgf-ECU release 3.30 */
/*:05-29-1993-21:47-wht@n4hgf-better debug */
/*:05-29-1993-20:21-wht@n4hgf-change linst_err_text to LINST_text */
/*:03-27-1993-17:45-wht@n4hgf-SVR4 cc complained about strlen <= constant */
/*:09-10-1992-13:58-wht@n4hgf-ECU release 3.20 */
/*:09-05-1992-14:17-wht@n4hgf-was starting rcvr process too early on connect */
/*:08-22-1992-15:38-wht@n4hgf-ECU release 3.20 BETA */
/*:05-13-1992-13:27-wht@n4hgf-active_pde use */
/*:04-24-1992-21:59-wht@n4hgf-more SCO tty name normalizing */
/*:12-02-1991-20:58-wht@n4hgf-breakout into separate module */

#include "ecu.h"
#include "esd.h"
#include "var.h"
#include "ecupde.h"
#include "dvent.h"
#include "termecu.h"
#include "ecuerror.h"
#include "utmpstatus.h"

DVE *hdb_choose_Any();
DVE *hdb_choose_Device();
PDE *phdir_list_search();
char *LINST_text();

extern char errmsg[];
extern char *default_tty;
extern int windows_active;

char phonedir_name[PHONEDIR_NAME_SIZE];
char *phonedir_trigger = "#ECUPHONE\n";

/*+-------------------------------------------------------------------------
	pde_dial(tpde) - dial using a pde (using procedure if configured)

  If the tpde->logical points to a valid procedure, use it,
  otherwise call DCE_dial()

  returns proc error code
          $i0 value ($s0 is also set, BTW)

  assumes rcvr process dead (shm->rcvr_pid == -1)
--------------------------------------------------------------------------*/
int
pde_dial(tpde)
PDE *tpde;
{
	int status = 0;
	int restart_rcvr = need_rcvr_restart();
	char *cp;
	extern int dcdwatch_set; /* see ldcdwatch() in eculine.c */

	dcdwatch_set = 0;		 /* this is a rat-faced kludge */

	kill_rcvr_process(SIGUSR1);

	if (proc_level || !find_procedure(tpde->logical))
	{
#ifdef CFG_TelnetOption
		if (!strcmp(tpde->tty, "telnet"))
		{
			enum linst linst;

			shm->Lbitrate = tpde->baud;
			strcpy(shm->Llogical, tpde->logical);
			strcpy(shm->Ldescr, tpde->descr);
			strcpy(shm->Ltelno, tpde->telno);
			if (!proc_level)
				restart_rcvr = 1;
			linst = telnet_open();
			if (linst)
			{
				UINT32 colors_at_entry = colors_current;

				setcolor(colors_error);
				pprintf("CONNECT FAILED");
				setcolor(colors_at_entry);
				pprintf("\n");
			}
			if (restart_rcvr)
				start_rcvr_process(1);
			return ((!linst) ? 0 : eFATAL_ALREADY);
		}
#endif /* CFG_TelnetOption */
		if (copy_pde_to_Lvariables(tpde, 0))
		{
			pprintf("%s: %s\n", tpde->logical, errmsg);
			pprintf("Current line is %s (%s)\n",
				shm->Lline, (shm->Liofd < 0) ? "closed" : "open");
			status = eFATAL_ALREADY;
		}
		else
			status = DCE_dial();
	}
	else
	{
		char *pargv[2];

		pargv[0] = tpde->logical;
		pargv[1] = "!MENU";
		iv[0] = 0;
		if (do_proc(2, pargv))
			status = eFATAL_ALREADY;
		else
			status = (iv[0]) ? eConnectFailed : 0;
#ifdef CFG_TelnetOption
		if (shm->Ltelnet && (shm->Liofd > -1))
			start_rcvr_process(1);
#endif /* CFG_TelnetOption */
	}

	/*
	 * if we connected and a procedure did not change the DCD watcher,
	 * then use the dialing directory choice
	 */
	if (!status && !dcdwatch_set)
	{
		cp = (char *)0;
		switch (tpde->dcdwatch)
		{
			case '0':
				ldcdwatch(DCDW_OFF);
				cp = "OFF (ignore DCD loss)";
				break;
#ifndef linux
			case '1':
				ldcdwatch(DCDW_ON);
				cp = "ON (detect DCD loss)";
				break;
#endif
			case 't':
				ldcdwatch(DCDW_TERMINATE);
				cp = "TERMINATE (terminate ecu on DCD loss)";
				break;
			case 'n':
			default:
				break;
		}
		if (cp)
		{
			UINT32 colors_save;

			colors_save = colors_current;
			setcolor(colors_notify);
			pprintf("[DCD watcher set to %s]", cp);
			setcolor(colors_save);
			fputs("\r\n", se);
			pputs("\n");
		}
	}

	if (restart_rcvr)
		start_rcvr_process(1);

	return (status);

}							 /* end of pde_dial */

/*+-------------------------------------------------------------------------
	pdetty_to_devtty(pdetty,devtty) -> pde tty field to complete pathname
--------------------------------------------------------------------------*/
void
pdetty_to_devtty(pdetty, devtty)
char *pdetty;
char *devtty;
{
	strcpy(devtty, "/dev/");
	strcat(devtty, pdetty);

}							 /* end of pdetty_to_devtty */

/*+-------------------------------------------------------------------------
	choose_tty_for_pde(tpde) - new pde might mandate switching line

Returns 1 if new line open needed
        0 if no new line open needed
       -1 if request cannot be satisfied
--------------------------------------------------------------------------*/
int
choose_tty_for_pde(tpde)
PDE *tpde;
{
	int itmp = 0;
	int rtn = 0;
	DVE *tdve;
	struct stat st;
	char newtty[64];

#ifdef CHOOSE_DEBUG
	vlogevent(xmtr_pid, "CHOOSE_TTY_FOR_PDE '%s' %u baud",
		tpde->tty, tpde->baud);
	errmsg[0] = 0;
#endif

#ifdef CFG_TelnetOption
	if (shm->Ltelnet)
		goto FUNC_RETURN;
#endif

	/*
	 * if desired line is Devices type
	 */
	if ((tpde->tty[0] == '=') || (tpde->tty[0] == '/'))	/* Devices or device */
	{
		if (!(tdve = hdb_choose_Device(tpde->tty, tpde->baud)))
		{
			sprintf(errmsg, "no idle line matches type '%s' at %u baud",
				*tpde->tty ? tpde->tty : "Any", tpde->baud);
			rtn = -1;
			goto FUNC_RETURN;
		}
		sprintf(newtty, "/dev/%s", tdve->line);
		if (stat(newtty, &st))
		{
			sprintf(errmsg, "%s: %s", newtty, strerror(errno));
			rtn = -1;
			goto FUNC_RETURN;
		}
		if ((itmp = reserve_line(newtty)) && (itmp != LINST_WEGOTIT))
		{
			sprintf(errmsg, "%s: %s", newtty, LINST_text(itmp));
			rtn = -1;
			goto FUNC_RETURN;
		}
		if ((itmp = lock_tty(newtty)) && (itmp != LINST_WEGOTIT))
		{
			sprintf(errmsg, "%s (%s): %s",
				tpde->tty, tdve->line, LINST_text(itmp));
			rtn = -1;
			goto FUNC_RETURN;
		}
	}

	/*
	 * if desired line is a specific tty
	 */
	else if (tpde->tty[0])
	{

		pdetty_to_devtty(tpde->tty, newtty);
		if ((shm->Liofd > 0) && !strcmp(newtty, shm->Lline))
		{
			rtn = 0;		 /* requesting line we already have */
			goto FUNC_RETURN;
		}
		if (stat(newtty, &st))
		{
			sprintf(errmsg, "%s: %s", newtty, strerror(errno));
			rtn = -1;
			goto FUNC_RETURN;
		}
		if ((itmp = reserve_line(newtty)) && (itmp != LINST_WEGOTIT))
		{
			sprintf(errmsg, "%s: %s", newtty, LINST_text(itmp));
			rtn = -1;
			goto FUNC_RETURN;
		}
		if ((itmp = lock_tty(newtty)) && (itmp != LINST_WEGOTIT))
		{
			sprintf(errmsg, "%s: %s", newtty, LINST_text(itmp));
			rtn = -1;
			goto FUNC_RETURN;
		}
	}

	/*
	 * if desired line is "Any" (any Devices type beginning with ACU)
	 */
	else
		/* "Any" */
	{
		tdve = hdb_choose_Any(tpde->baud);
		if (!tdve)
		{
			sprintf(errmsg, "no idle ACU line at %u baud", tpde->baud);
			rtn = -1;
			goto FUNC_RETURN;
		}
		sprintf(newtty, "/dev/%s", tdve->line);
		if ((itmp = lock_tty(newtty)) && (itmp != LINST_WEGOTIT))
		{
			sprintf(errmsg, "%s: %s", newtty, LINST_text(itmp));
			rtn = -1;
			goto FUNC_RETURN;
		}
		if ((itmp = reserve_line(newtty)) && (itmp != LINST_WEGOTIT))
		{
			sprintf(errmsg, "%s: %s", newtty, LINST_text(itmp));
			rtn = -1;
			goto FUNC_RETURN;
		}
	}

	rtn = !(!strcmp(newtty, shm->Lline) && (shm->Liofd != -1));
	if (rtn)
	{
		lclose();
		strcpy(shm->Lline, newtty);
	}

  FUNC_RETURN:

#ifdef CHOOSE_DEBUG

	vlogevent(xmtr_pid, "CHOOSE_TTY_FOR_PDE rtn=%d line='%s' errmsg='%s' itmp=%d",
		rtn, shm->Lline, errmsg, itmp);
#endif

	return (rtn);

}							 /* end of choose_tty_for_pde */

/*+-------------------------------------------------------------------------
	copy_pde_to_Lvariables(tpde,trial)

  'trial' controls whether this is a dry run (setup) or a live
  request; if changing line, close old line and open new one; if
  cannot change line, return -1, else 0
--------------------------------------------------------------------------*/
int
copy_pde_to_Lvariables(tpde, trial)
PDE *tpde;
int trial;
{
	int reopen = 0;
	int lerr;
	DVE *tdve;

#ifdef CHOOSE_DEBUG
	vlogevent(xmtr_pid, "COPY_PDE1 trial=%d Lline='%s' pdetty='%s'",
		trial, shm->Lline, tpde->tty);
#endif

#ifdef CFG_TelnetOption
	if ((!strcmp(tpde->tty, "telnet")) || strchr(tpde->telno, '.'))
	{
		int itmp;

		strcpy(shm->Lline, "/dev/telnet");
		shm->Ltelnet = 1;
		shm->Lfull_duplex = 1;
		shm->Lbitrate = tpde->baud;
		shm->Lparity = 0;
		shm->Ladd_nl_incoming = 0;
		shm->Ladd_nl_outgoing = 0;
		strcpy(shm->Llogical, tpde->logical);
		strcpy(shm->Ldescr, tpde->descr);
		strcpy(shm->Ltelno, tpde->telno);
		if ((itmp = strlen(shm->Ltelno)) && (shm->Ltelno[itmp - 1] == '.'))
			shm->Ltelno[itmp - 1] = 0;
		if (!shm->Ldescr[0])
			strcpy(shm->Ldescr, shm->Llogical);
		return (0);
	}

	shm->Ltelnet = 0;
#endif

	enddvent();				 /* krock safety */

	if (!trial)
	{
		if ((reopen = choose_tty_for_pde(tpde)) < 0)
			return (-1);	 /* errmsg[] updated */
	}
	else if ((!tpde->tty[0]) || (!strcmp(tpde->tty, "Any")))
	{
		if (!(tdve = hdb_choose_Any(tpde->baud)))
		{
			strcpy(errmsg, "no idle line matches type 'Any'");
			return (-1);
		}
		strcpy(shm->Lline, "/dev/");
		strncat(shm->Lline, tdve->line, sizeof(shm->Lline) - 5);
		shm->Lline[sizeof(shm->Lline) - 1] = 0;
	}
	else if ((tpde->tty[0] == '/') || (tpde->tty[0] == '='))
	{
		if (!(tdve = hdb_choose_Device(tpde->tty, tpde->baud)))
		{
			sprintf(errmsg, "no idle line matches type '%s' at %u baud",
				*tpde->tty ? tpde->tty : "Any", tpde->baud);
			return (-1);
		}
		strcpy(shm->Lline, "/dev/");
		strncat(shm->Lline, tdve->line, sizeof(shm->Lline) - 5);
		shm->Lline[sizeof(shm->Lline) - 1] = 0;
	}
	else
		pdetty_to_devtty(tpde->tty, shm->Lline);

#ifdef CHOOSE_DEBUG
	vlogevent(xmtr_pid, "COPY_PDE2  Lline='%s' reopen=%d", shm->Lline, reopen);
#endif

	shm->Lbitrate = tpde->baud;
	strcpy(shm->Llogical, tpde->logical);
	strcpy(shm->Ldescr, tpde->descr);
	strcpy(shm->Ltelno, tpde->telno);
	if (!shm->Ldescr[0])
		strcpy(shm->Ldescr, shm->Llogical);
	shm->Lparity = tpde->parity;
	if (shm->Lbitrate != tpde->baud)
		shm->Lmodem_already_init = 0;
	Ldial_debug_level = tpde->debug_level;
	if (isdigit(tpde->rtscts_val))	/* 'n' n means leave alone */
	{
		char s2[2];

		s2[0] = tpde->rtscts_val;
		s2[1] = 0;
		shm->Lrtscts_val = atoi(s2);
	}
	/* tpde->dcdwatch is explicitly unused here; must defer until connect  */

	if (!trial)
	{
		if (reopen)
		{
			if (lerr = lopen())
			{
				tcap_curbotleft();
				pprintf("%s: %s\n", shm->Lline, LINST_text(lerr));
				lopen_error_reset();	/* clear static error area */
				termecu(TERMECU_LINE_OPEN_ERROR);
			}
		}
		else
		{
			lset_baud(1);
			lset_parity(1);
		}
	}
	return (0);

}							 /* end of copy_pde_to_Lvariables */

/*+-----------------------------------------------------------------------
	logical_telno_to_pde() - logical dial string to dialing info (PDE)

	NOT USED BY THE CURSES DIRECTORY MANAGER

A logical telephone number is one of:
1. null,
2. a symbolic identifer, or
3. an actual telephone number.

A symbolic identifer is a string whose initial character is a
letter.  An actual telephone number begins with a numeral.

This function converts a logical telephone number to a dialing
entry (PDE) from the directory or a constructed static one.  It
is called by command line processing when 'ecu logical-name' is
specified or in response to a %dial logical-name.

The function returns one of the following:

  directory or static PDE		if no error occurs
  (PDE *)0						if not numeric phone number
								and logical string not found in directory;
								global char[] errmsg has been plugged with
								error message

------------------------------------------------------------------------*/
PDE *
logical_telno_to_pde(logical)
char *logical;
{
	static PDE literal_number_pde;
	PDE *tpde = 0;
	extern int phdir_list_quan;

	/*
	 * if hostname, get funky-delic
	 */
#ifdef CFG_TelnetOption
	if (strchr(logical, '.'))
	{
		int itmp;

		tpde = &literal_number_pde;
		memset((char *)tpde, 0, sizeof(PDE));
		tpde->baud = 115200; /* a guess */
		tpde->parity = 0;
		strncpy(tpde->logical, logical, LOGICAL_LEN);
		tpde->logical[LOGICAL_LEN] = 0;
		strcpy(tpde->tty, "telnet");
		strcpy(tpde->telno, tpde->logical);
		strcpy(tpde->descr, "<telnet connect>");
		if ((itmp = strlen(tpde->telno)) && (tpde->telno[itmp - 1] == '.'))
			tpde->telno[itmp - 1] = 0;
		tpde->dcdwatch = 'n';
		tpde->rtscts_val = 'n';
		strcpy(shm->Lline, "/dev/telnet");
		shm->Ltelnet = 1;
		shm->Ladd_nl_incoming = 0;
		shm->Ladd_nl_outgoing = 0;
		shm->Lfull_duplex = 1;
		goto FUNC_RETURN;
	}

	shm->Ltelnet = 0;
#endif

	/*
	 * if literal phone number, return homemade, static "PDE" with most
	 * stuff dummied up with the status quo
	 */
	if (isdigit((uchar) * logical))
	{
		tpde = &literal_number_pde;
		memset((char *)tpde, 0, sizeof(PDE));
		tpde->baud = shm->Lbitrate;
		tpde->parity = shm->Lparity;
		strncpy(tpde->logical, logical, LOGICAL_LEN);
		tpde->logical[LOGICAL_LEN] = 0;
		strncpy(tpde->telno, logical, DESTREF_LEN);
		tpde->telno[DESTREF_LEN] = 0;
		strncpy(tpde->tty, shm->Lline + 5, PDE_TTY_LEN);
		tpde->tty[PDE_TTY_LEN] = 0;
		if ((unsigned)(strlen(logical) + 8) <= (unsigned)PDE_DESCR_LEN)
			sprintf(tpde->descr, "<telno %s>", logical);
		else
			strcpy(tpde->descr, "<telno>");
		tpde->dcdwatch = 'n';
		tpde->rtscts_val = 'n';
		goto FUNC_RETURN;
	}

	/*
	 * if empty phone number, return homemade, static "PDE" with most
	 * stuff dummied up with the status quo
	 */
	if (!*logical)
	{
		tpde = &literal_number_pde;
		memset((char *)tpde, 0, sizeof(PDE));
		tpde->baud = shm->Lbitrate;
		tpde->parity = shm->Lparity;
		strcpy(tpde->tty, shm->Lline + 5);
		if (!tpde->tty[0])
			strcpy(tpde->tty, default_tty + 5);
		strcpy(tpde->descr, "<direct connect>");
		tpde->dcdwatch = 'n';
		tpde->rtscts_val = 'n';
		goto FUNC_RETURN;
	}

	/*
	 * read directory if necessary
	 */
	if (!phdir_list_quan)
	{
		if (phdir_list_read() && !phdir_list_quan)
		{					 /* if still no good, ... */
			strcpy(errmsg, "phone directory empty");
			tpde = 0;
			goto FUNC_RETURN;
		}
	}

	/*
	 * now, look up the entry
	 */
	if (tpde = phdir_list_search(logical, 0))
		goto FUNC_RETURN;

	/*
	 * whoops ... not found
	 */
	shm->Lrname[0] = 0;
	shm->Ltelno[0] = 0;
	shm->Ldescr[0] = 0;
	strcpy(errmsg, "entry not found in directory");
	tpde = 0;

  FUNC_RETURN:

#ifdef CHOOSE_DEBUG
	vlogevent(xmtr_pid, "LOGICAL->PDE %s=%s chosen tty=%s",
		(tpde == &literal_number_pde) ? "TELNO" : "LOGICAL",
		logical, (tpde && tpde->tty[0]) ? tpde->tty : "<none>");
#endif

	return (tpde);

}							 /* end of logical_telno_to_pde */

/*+-------------------------------------------------------------------------
	call_logical_telno(logical) - call a logical or literal telephone number

-1 if no such entry or pde_dial() status
--------------------------------------------------------------------------*/
int
call_logical_telno(logical)
char *logical;
{
	int ret;
	PDE *tpde;

	if (tpde = logical_telno_to_pde(logical))
		ret = pde_dial(tpde);
	else
	{
		pprintf("%s: %s\n", logical, errmsg);
		ret = eFATAL_ALREADY;
	}

	return (ret);

}							 /* end of call_logical_telno */

/* vi: set tabstop=4 shiftwidth=4: */
/* end of ecupde.c */
