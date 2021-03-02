/* #define USE_S7 */
/*+-------------------------------------------------------------------------
	ecuDCE.c - ECU DCE dialing and management
	wht@wht.net

  Defined functions:
	DCE_autoanswer()
	DCE_dial()
	DCE_get_result(msec_to_wait)
	DCE_get_sreg_value(regnum)
	DCE_hangup()
	DCE_modem_init()
	DCE_now_on_hook()
	DCE_read_modem_init()
	DCE_redial(arg, argc)
	DCE_report_iv_set(varnum)
	DCE_send_cmd(cmd)
	DCE_set_sreg(regnum, value)
	check_queued_sigint()
	is_extant_dialproc(device)
	process_modem_init(str)
	show_modem_init_error(erc, iesd)

Lothar Hirschbiegel <emory!tmcsys.uucp!lothar> added the ability to
specify a modem acknowledgement string other than "OK".
Example use:
#+-----------------------------------------------------------------
#  tty1a.mi - Microcom QX/V.32c
#------------------------------------------------------------------
init_default:sceon
dial_default:dp
ok_string:!
connect_string:CONNECT

--------------------------------------------------------------------------*/
/*+:EDITS:*/
/*:04-26-2000-11:15-wht@bob-RELEASE 4.42 */
/*:02-09-1997-20:13-wht@yuriatin-add dialer procedures */
/*:01-24-1997-02:37-wht@yuriatin-SOURCE RELEASE 4.00 */
/*:09-11-1996-19:59-wht@yuriatin-3.48-major telnet,curses,structural overhaul */
/*:08-20-1996-12:39-wht@kepler-locale/ctype fixes from ache@nagual.ru */
/*:08-11-1996-02:10-wht@kepler-rename ecu_log_event to logevent */
/*:08-10-1996-13:52-wht@kepler-Ltelnet hangup no longer kills ecu */
/*:07-31-1996-17:00-dgy@rtd.com-built-in dialer fixes */
/*:12-06-1995-13:30-wht@n4hgf-termecu w/errno -1 consideration */
/*:11-23-1995-11:20-wht@kepler-source control 3.37 for tsx-11 */
/*:11-20-1995-12:15-wht@n4hgf-"Type INT to abort" only if tty char special */
/*:11-14-1995-10:23-wht@kepler-3.37.80-source control point: SOCKETS */
/*:11-03-1995-16:53-wht@wwtp1-use CFG_TelnetOption */
/*:05-11-1995-15:55-wht@n4hgf-ck_sigint fools optimizing compilers */
/*:01-12-1995-15:19-wht@n4hgf-apply Andrew Chernov 8-bit clean+FreeBSD patch */
/*:05-04-1994-04:38-wht@n4hgf-ECU release 3.30 */
/*:02-11-1994-16:03-wht@n4hgf-add connect string */
/*:01-04-1994-05:45-wht@n4hgf-add CFG_DialTimeout */
/*:04-06-1993-11:51-wht@n4hgf-need to kill rcvr to use built-in dialer */
/*:09-10-1992-13:58-wht@n4hgf-ECU release 3.20 */
/*:09-05-1992-14:17-wht@n4hgf-was starting rcvr process too early on connect */
/*:08-22-1992-15:38-wht@n4hgf-ECU release 3.20 BETA */
/*:04-19-1992-03:21-jhpb@sarto.budd-lake.nj.us-3.18.37 has ESIX SVR4 */
/*:02-16-1992-01:41-wht@n4hgf-turn off xterm_title */
/*:02-04-1992-04:49-wht@n4hgf-fix bug in kill_rcvr_process logic */
/*:01-17-1992-15:32-wht@n4hgf-.credit open to public */
/*:11-07-1991-16:54-tmcsys!lothar-alternate OK string for built-in dialer */
/*:08-28-1991-14:07-wht@n4hgf2-SVR4 cleanup by aega84!lh */
/*:07-25-1991-12:55-wht@n4hgf-ECU release 3.10 */
/*:07-17-1991-07:04-wht@n4hgf-avoid SCO UNIX nap bug */
/*:06-29-1991-15:42-wht@n4hgf-if WHT and xterm, play with title bar */
/*:06-16-1991-23:24-wht@n4hgf-ensure hangup since proc could fail connected */
/*:06-07-1991-04:09-wht@n4hgf-remove unnecessary naps after kill_rcvr_process */
/*:05-20-1991-00:56-wht@n4hgf-fix/upgrade auto fkey load */
/*:01-09-1991-22:31-wht@n4hgf-ISC port */
/*:01-09-1991-21:23-wht@n4hgf-fix statement not reached */
/*:09-19-1990-19:36-wht@n4hgf-logevent now gets pid for log from caller */
/*:08-14-1990-20:40-wht@n4hgf-ecu3.00-flush old edit history */

#include "ecu.h"
#include "ecukey.h"
#include "esd.h"
#include "var.h"
#include "ecupde.h"
#include "ecuerror.h"
#include "relop.h"

char *elapsed_time_text();
void DCE_hangup();

extern UINT32 colors_current;
extern char kbdintr;

int mi_line;
char mi_name[64];
char last_dial_result[64];

#define MI_MAX_LEN 65
char Lmodem_init[MI_MAX_LEN] = "";	/* modem init string w/o trailing CR */
char Lmodem_dial[MI_MAX_LEN] = "";	/* modem dialing prefix */
char Lmodem_autoans[MI_MAX_LEN] = "";	/* modem autoanswer */
char Lmodem_okstring[MI_MAX_LEN] = "";	/* modem ok_answer */
char Lmodem_connstring[MI_MAX_LEN] = "";	/* modem ok_answer */

char *interrupted_string = "!Interrupted";
UINT ttygetc();

/*+-------------------------------------------------------------------------
	check_queued_sigint()
--------------------------------------------------------------------------*/
int
check_queued_sigint()
{
	while (ttyrdchk())
	{
		if (ttygetc(1) == (uchar) kbdintr)
		{
			sigint = 1;
			return (1);
		}
	}
	return (0);
}							 /* end of check_queued_sigint */

/*+-------------------------------------------------------------------------
	show_modem_init_error(erc,iesd)
--------------------------------------------------------------------------*/
void
show_modem_init_error(erc, iesd)
int erc;
ESD *iesd;
{
	int itmp;

	pputs(mi_name);
	pprintf(" line %d: ", mi_line);
	proc_error(erc);
	pputs(iesd->pb);
	pputc(NL);
	itmp = iesd->old_index;
	while (itmp--)
		pputc(' ');
	pputs("^\n\n");

}							 /* end of show_modem_init_error */

/*+-----------------------------------------------------------------------
	process_modem_init(str)

sample /usr/lib/ecu/tty??.mi lines:
init_9600:ATS11=47X4S0=0S7=30\Q0\X0\N0
init_>2400:ATS11=47X4S0=0S7=30\Q0\X0\N0
init_<=2400:ATS11=47X4S0=0S7=30\Q1\X1\N3
ATDT

return 0 if entire list read, else 1 if error (error msg in errmsg)
------------------------------------------------------------------------*/
void
process_modem_init(str)
char *str;
{
#define MI_INIT		1
#define MI_DIAL		2
#define MI_AUTOANS	3
#define MI_OKSTRING	4
#define MI_CONNSTR	5
	int erc;
	ESD sesd;
	char typestr[32];
	int relop;
	int truth = 0;
	int type;
	long test_baud;

	sesd.pb = str;
	sesd.cb = strlen(str);
	sesd.maxcb = strlen(str);
	sesd.index = 0;
	sesd.old_index = 0;

	if (get_alpha_zstr(&sesd, typestr, sizeof(typestr)))
	{
		erc = eSyntaxError;
		goto SHOW_ERROR;
	}
	if (ulindex(typestr, "init_") == 0)
		type = MI_INIT;
	else if (ulindex(typestr, "dial_") == 0)
		type = MI_DIAL;
	/* DGY 07/25/96 allow "connect_string_" to be conditioned by baud */
	else if (ulindex(typestr, "connect_string_") == 0)
		type = MI_CONNSTR;
	else if (!strcmpi(typestr, "autoanswer"))
		type = MI_AUTOANS;
	else if (!strcmpi(typestr, "ok_string"))
		type = MI_OKSTRING;
	else
	{
		erc = eSyntaxError;
		goto SHOW_ERROR;
	}

/* test for default ... if none, check bit rate */
	if ((type == MI_AUTOANS) || (type == MI_OKSTRING))
		truth = 1;
	else if (ulindex(typestr, "_default") > 0)
	{
		truth = !(((type == MI_INIT) && Lmodem_init[0]) ||
			((type == MI_CONNSTR) && Lmodem_connstring[0]) ||
			((type == MI_DIAL) && Lmodem_dial[0]));
	}
	else
	{
		/* get optional operator */
		if (get_relop(&sesd, &relop))
			relop = OP_EQ;
		if (erc = gint_constant(&sesd, &test_baud))
			goto SHOW_ERROR;
		truth = test_truth_int((long)shm->Lbitrate, relop, test_baud);
	}

/* if no match, skip this one */
	if (!truth)
		return;

/* skip over colon */
	if (erc = skip_colon(&sesd))
		goto SHOW_ERROR;

/* make sure init connect or dial string not empty or too long */
	if ((erc = skip_cmd_break(&sesd)) &&
		!((type == MI_AUTOANS) || (type == MI_OKSTRING)))
		goto SHOW_ERROR;

	if ((sesd.cb - sesd.index) > (MI_MAX_LEN - 1))
	{
		erc = eBufferTooSmall;
		goto SHOW_ERROR;
	}

	erc = eDuplicateMatch;	 /* in case of show error in switch */
	switch (type)
	{
		case MI_INIT:
			if (Lmodem_init[0])
				goto SHOW_ERROR;
			strcpy(Lmodem_init, sesd.pb + sesd.index);
			break;

		case MI_DIAL:
			if (Lmodem_dial[0])
				goto SHOW_ERROR;
			strcpy(Lmodem_dial, sesd.pb + sesd.index);
			break;

		case MI_AUTOANS:
			if (Lmodem_autoans[0])
				goto SHOW_ERROR;
			if (!sesd.cb)
				strcpy(Lmodem_autoans, "!null!");
			else
				strcpy(Lmodem_autoans, sesd.pb + sesd.index);
			break;

		case MI_OKSTRING:
			if (Lmodem_okstring[0])
				goto SHOW_ERROR;
			strcpy(Lmodem_okstring, sesd.pb + sesd.index);
			break;

		case MI_CONNSTR:
			if (Lmodem_connstring[0])
				goto SHOW_ERROR;
			strcpy(Lmodem_connstring, sesd.pb + sesd.index);
			break;

	}
	return;					 /* <<<<====== done */

  SHOW_ERROR:
	show_modem_init_error(erc, &sesd);

}							 /* end of process_modem_init */

/*+-----------------------------------------------------------------------
	DCE_read_modem_init()
0123456789
/dev/ttyxx
------------------------------------------------------------------------*/
void
DCE_read_modem_init()
{
	char *cp;
	FILE *fp_modem;
	char *skip_ld_break();
	char buffer[128];

/* zap init information */
	Lmodem_init[0] = 0;
	Lmodem_dial[0] = 0;

/* build filename */
	sprintf(mi_name, "%s/%s.mi", CFG_EcuLibDir, shm->Lline + 5);

/* read modem initialization */
	if (!(fp_modem = fopen(mi_name, "r")))
		pperror(mi_name);
	else
	{
		mi_line = 0;
		while ((!Lmodem_init[0] || !Lmodem_dial[0] || !Lmodem_okstring[0] ||
				!Lmodem_connstring[0]) &&
			fgets(buffer, sizeof(buffer), fp_modem))
		{
			mi_line++;
			buffer[strlen(buffer) - 1] = 0;
			cp = skip_ld_break(buffer);
			/* skip comments and null lines */
			if (!strlen(cp) || (*cp == '#'))
				continue;
			process_modem_init(cp);
		}
		fclose(fp_modem);
	}

/* default */
	if (!Lmodem_init[0])
	{
		strcpy(Lmodem_init, "ATE1Q0V1");
		pputs("modem init string not found (using default '");
		pputs(Lmodem_init);
		pputs("')\n");
	}
	if (!Lmodem_dial[0])
	{
		strcpy(Lmodem_dial, "ATDT");
		pputs("modem dial string not found (using default '");
		pputs(Lmodem_dial);
		pputs("')\n");
	}

	if (!Lmodem_autoans[0])
		strcpy(Lmodem_autoans, "ATQ1S0=1");
	if (!Lmodem_okstring[0])
		strcpy(Lmodem_okstring, "OK");
	if (!Lmodem_connstring[0])
		strcpy(Lmodem_connstring, "CONNECT");
	if (!strcmp(Lmodem_autoans, "!null!"))
		Lmodem_autoans[0] = 0;

	if (proc_trace > 1)
	{
		pprintf("init:       '%s'\n", Lmodem_init);
		pprintf("dial:       '%s'\n", Lmodem_dial);
		pprintf("autoanswer: '%s'\n", Lmodem_autoans);
		pprintf("ok str:     '%s'\n", Lmodem_okstring);
		pprintf("conn str:   '%s'\n", Lmodem_connstring);
	}
}							 /* end of DCE_read_modem_init */

/*+-------------------------------------------------------------------------
	DCE_get_result(msec_to_wait)
return pointer to static buf containing result code
--------------------------------------------------------------------------*/
char *
DCE_get_result(msec_to_wait)
long msec_to_wait;
{
	static char s32[32];
	LRWT lr;

	if (ck_sigint())
		return ("!Interrupted");
	s32[0] = 0;
	lr.to1 = msec_to_wait;
	lr.to2 = 200L;
	lr.raw_flag = 0x80;		 /* allow interrupts */
	lr.buffer = s32;
	lr.bufsize = sizeof(s32);
	lr.delim = (char *)0;
	lr.echo_flag = 0;
	lgets_timeout(&lr);

	if (ck_sigint())
		return ("!Interrupted");

	return (lr.buffer);
}							 /* end of DCE_get_result */

/*+-------------------------------------------------------------------------
	DCE_modem_init()
--------------------------------------------------------------------------*/
DCE_modem_init()
{
	int itmp;
	int retries = 0;
	char *cmd;
	char *cp;
	int old_ttymode = get_ttymode();

	if (shm->Lmodem_already_init)
		return (0);

	DCE_read_modem_init();

	ttymode(2);
	lputs_paced(0, "\b\b\b\b\b\b\b\b\b");
	(void)Nap(200L);
	lflush(0);

	while (!shm->Lmodem_already_init)
	{
	  INIT_LOOP:
		if (retries > 3)
			goto ERROR_RETURN;

		if (ck_sigint())
			goto ERROR_RETURN;

		if (retries)
		{
			lflash_dtr();
			lputs_paced(0, "AT\r");
			(void)Nap(200L);
			lputs_paced(0, "ATQ0V1E1\r");
			(void)Nap(200L);
		}

		lflush(0);
		cmd = Lmodem_init;
		itmp = 0;
#ifdef NEUROTIC
		while (*cmd)
		{
			lputc_paced(0, *cmd++);
			if (++itmp < 2)
				(void)Nap(40L);
			if ((itmp = lgetc_timeout(500L)) < 0)
			{
				if (ck_sigint())
					goto ERROR_RETURN;
				retries++;
				goto INIT_LOOP;
			}
			pputc(itmp);
		}
#else
		lputs(cmd);
		pputs(cmd);
#endif

		pputc(NL);
		lputc_paced(0, CRET);

		itmp = 0;
		while (itmp != CRET)
		{
			if ((itmp = lgetc_timeout(500L)) < 0)
			{
				if (ck_sigint())
					goto ERROR_RETURN;
				pputs("missed expected carriage return\n");
				if (itmp < 0)
					pprintf("got nothing\n");
				else
					pprintf("got: %x\n", itmp);
				retries++;
				goto INIT_LOOP;
			}
		}

		if (strcmp(cp = DCE_get_result(1200L), Lmodem_okstring))
		{
			if (!strcmp(cp, interrupted_string))
			{
				sigint = 1;
				goto ERROR_RETURN;
			}
			pprintf("unexpected result: '%s'\n", cp);
			retries++;
			continue;
		}
		shm->Lmodem_already_init = 1;
	}

	ttymode(old_ttymode);
	return (0);

  ERROR_RETURN:
	ttymode(old_ttymode);
	return (-1);

}							 /* end of DCE_modem_init */

/*+-------------------------------------------------------------------------
	DCE_send_cmd(cmd)
--------------------------------------------------------------------------*/
int
DCE_send_cmd(cmd)
char *cmd;
{
	int itmp;

	DCE_modem_init();
	(void)Nap(600L);

	lflush(0);
#ifdef NEUROTIC
	while (*cmd)
	{
		lputc_paced(20, *cmd++);
		if (++char_count < 2)
			(void)Nap(40L);
		if ((itmp = lgetc_timeout(500L)) < 0)
			return (-1);
		pputc(itmp);
	}
#else
	lputs(cmd);
	pputs(cmd);
#endif
	pputc(NL);
	lputc_paced(20, CRET);
	itmp = 0;
	if (ck_sigint() || check_queued_sigint())
		return (-1);
	while (itmp != CRET)
	{
		if ((itmp = lgetc_timeout(1000L)) < 0)
		{
			pputs("missed expected carriage return\n");
			return (-1);
		}
		if (ck_sigint() || check_queued_sigint())
			return (-1);
	}
	return (0);

}							 /* end of DCE_send_cmd */

/*+-------------------------------------------------------------------------
	DCE_report_iv_set(varnum)
--------------------------------------------------------------------------*/
void
DCE_report_iv_set(varnum)
int varnum;
{
	if (proc_trace)
		pprintf("Dialer set $i%02d = %ld\n", varnum, iv[varnum]);
}							 /* end of DCE_report_iv_set */

/*+-------------------------------------------------------------------------
	is_extant_dialproc(device) - find dialer procedure for device
--------------------------------------------------------------------------*/
char *
is_extant_dialproc(device)
char *device;
{
	char *devbase = base_name(device);
	char dpnm[ECU_MAXPN];

	strcpy(dpnm, "dialproc/");
	strcat(dpnm, devbase);
	return ((find_procedure(dpnm)) ? dpnm : 0);

}							 /* end of is_extant_dialproc */

/*+-----------------------------------------------------------------------
	DCE_dial() - dial a remote or connect

  All shm->L... variables have been set up and are used to
  drive the dialing sequence

  returns 0 on success (CONNECT),
          eConnectFailed if failure
          eCONINT on interrupt

  sets #I0 to 0==connect,
              1==failed to connect,
              2==interrupted
              3==modem error
  sets #S0 to modem result code or uucp status code string

  This function has become quite NASTY and needs rewriting!
------------------------------------------------------------------------*/
int
DCE_dial()
{
	char s128[128];
	char *_proc_args[10];
	int itmp;
	int erc = -1;			 /* assume error unless chg'd */
	int s7;
	char *result = "";
	int restart_rcvr = need_rcvr_restart();
	UINT32 colors_at_entry = colors_current;
	char s64[64];
	FILE *fp;
	char *cp;
	char credit_file[128];
	extern int tty_not_char_special;

	kill_rcvr_process(SIGUSR1);

	lclear_xmtr_xoff();
	if (shm->Ldescr[0])
	{
		setcolor(colors_success);
		timeofday_text(1, s64);
		pprintf("%s %s\n",
			(shm->Ltelno[0]) ? "Dialing" : "Connecting to", shm->Ldescr);
		pprintf("on %s at %u baud (%s)\n", shm->Lline, shm->Lbitrate, s64);
	}

	if (shm->Liofd > 0)
		DCE_hangup();

	last_dial_result[0] = 0;
	setcolor(colors_alert);
	if (!shm->Ltelno[0])	 /* if no phone number, direct connect */
	{
		sprintf(s64, "CONNECT %u", shm->Lbitrate);
		result = s64;
		iv[0] = 0;
		erc = 0;
		shm->Lconnected = 1;
		Ldial_debug_level = 0;
		goto CONNECTED;
	}
	else
	{
		char *dialproc = is_extant_dialproc(shm->Lline);

		/*
		 * if there is a dialer procedure, invoke it else call the
		 * traditional dialer
		 */
		if (!dialproc)
		{
			itmp = hdb_dial(&result);
			lreset_ksr();	 /* dialer may have changed termio */
		}
		else
		{
			char dialproc_buf[ECU_MAXPN];

			/*
			 * copy static result
			 */
			strcpy(dialproc_buf,dialproc);

			/*
			 * call procedure
			 */
			pprintf("Using dialer procedure %s\n",
				find_procedure(dialproc_buf));
			_proc_args[0] = dialproc_buf;
			_proc_args[1] = shm->Ltelno;
			if (do_proc(2, _proc_args))	/* if procedure executes well */
			{
				erc = eConnectFailed;
				goto START_RCVR_PROCESS;
			}

			/*
			 * dialproc responsible for plugging $i0 and $s0
			 */
			itmp = iv[0];	 /* dial result code 0-n returned in $i0 */
			result = sv[0]->pb;
		}

		/*
		 * check status of dialing operation
		 */
		switch (itmp)
		{
			case 0:		 /* success */
				goto CONNECTED;
			case 1:		 /* failure -- iv[0] set by hdb_dial */
				DCE_report_iv_set(0);
				erc = eConnectFailed;
				goto START_RCVR_PROCESS;
			case 2:		 /* interrupted -- iv[0] set by hdb_dial */
				erc = eCONINT;
				DCE_report_iv_set(0);
				goto START_RCVR_PROCESS;
			case 3:		 /* modem error */
				DCE_report_iv_set(0);
				setcolor(colors_error);
				pprintf("%s\n", result);
				goto CANNOT_TALK_TO_MODEM;
			case 4:		 /* try built-in dialer */
				break;
			default:
				DCE_report_iv_set(0);
				setcolor(colors_error);
				pprintf("Undefined dialer result %d\n", itmp);
				erc = eConnectFailed;
				goto START_RCVR_PROCESS;
		}

		/*
		 * brain-damaged "built-in dialer"
		 */

		pputs("\nTrying ecu dialer\n");
		kill_rcvr_process(SIGUSR1);
		DCE_modem_init();

#if defined(USE_S7)
		if ((s7 = DCE_get_sreg_value(7)) < 0)
			s7 = CFG_DialTimeout;
#else
		s7 = CFG_DialTimeout;
#endif

		/*
		 * build dial command
		 */
		strcpy(s128, Lmodem_dial);
		strcat(s128, shm->Ltelno);

		/*
		 * if trailing '$', read and append ~/.ecu/.credit
		 */
		if (*(cp = s128 + strlen(s128) - 1) == '$')
		{
			*cp = 0;
			get_home_dir(credit_file);
			strcat(credit_file, "/.ecu/.credit");
			if (fp = fopen(credit_file, "r"))
			{
				fgets(cp, 30, fp);
				fclose(fp);
			}
			if (!fp || !(*cp))
			{
				result = "!CREDIT CARD ERROR";
				goto CONNECT_FAILED;
			}
			if (*(cp + strlen(cp) - 1) == 0x0A)
				*(cp + strlen(cp) - 1) = 0;	/* kill NL */
		}

		if (ck_sigint() || check_queued_sigint())
			goto SEND_CMD_ERROR;

		if (DCE_send_cmd(s128))
			goto SEND_CMD_ERROR;

		/*
		 * some modems (ahem, the Hayes 2400) do not accurately honor S7
		 * so our timer is twice sreg 7
		 */
		if (!tty_not_char_special)
		{
			pprintf("Type %s to abort ... ", (kbdintr == DEL) ? "DEL" :
				graphic_char_text(kbdintr, 0));
		}
		setcolor(colors_normal);
		lflush(0);
		strcpy(s64, DCE_get_result(s7 * 2 * 1000L));
		result = s64;

		if (ck_sigint() || !strcmp(result, interrupted_string))
		{
			setcolor(colors_error);
			pprintf("%s\n", result);
			sigint = 0;
			lputc(0);		 /* send char in case DTR ignored */
			lflash_dtr();	 /* force on hook */
			(void)DCE_get_result(2000L);	/* wait for any result code */
			erc = eCONINT;
			iv[0] = 2;
			DCE_report_iv_set(0);
			goto START_RCVR_PROCESS;
		}
		if (!strncmp(result, Lmodem_connstring, strlen(Lmodem_connstring)))
		{
			if (strlen(result) > 7)
			{
				UINT speed = atoi(result + strlen(Lmodem_connstring));

				if (speed && (shm->Lbitrate != speed))
				{
					setcolor(colors_alert);
					pprintf("%s (warning: unexpected rate)\n", result);
				}
			}
		  CONNECTED:
#if defined(WHT2) && defined(XTERM_FRIEND)

			/*
			 * if xterm, put connection in title bar but this really
			 * should be done in _connect.ep
			 */
			sprintf(s128, "connected to %s", shm->Llogical);
			xterm_title(s128, 1);
#endif
			setcolor(colors_success);
			pprintf("%s\n", result);
			sprintf(s128, "CONNECT %s (%s) %u baud",
				shm->Llogical, shm->Ltelno, shm->Lbitrate);
			strcpy(shm->Lrname, shm->Llogical);
			logevent(getpid(), s128);
			if (isalpha(shm->Llogical[0]))
			{
				if (!keyset_read(shm->Llogical))
					pprintf("[autoloaded fkeys for %s]\n", shm->Llogical);
			}
			shm->rcvd_chars_this_connect = 0;
			shm->xmit_chars_this_connect = 0;
			shm->Loff_hook_time = time((long *)0);
			iv[0] = 0;
			DCE_report_iv_set(0);
			erc = 0;
			shm->Lconnected = 1;
			lCLOCAL(!shm->Ldcdwatch);	/* set CLOCAL per DCD watcher */
			goto START_RCVR_PROCESS;
		}

	  CONNECT_FAILED:
		setcolor(colors_error);
		pprintf("%s\n", result);
		iv[0] = 1;
		DCE_report_iv_set(0);
		erc = eConnectFailed;
		goto START_RCVR_PROCESS;
	}

  SEND_CMD_ERROR:
	if (ck_sigint())
	{
		sigint = 0;
		result = interrupted_string;
		iv[0] = 2;
		DCE_report_iv_set(0);
		erc = eCONINT;
	}
	else
	{
	  CANNOT_TALK_TO_MODEM:
		setcolor(colors_error);
		pprintf("Cannot talk to modem\n");
		result = "!Modem Error";
		iv[0] = 3;
		DCE_report_iv_set(0);
		erc = eConnectFailed;
	}

  START_RCVR_PROCESS:
	setcolor(colors_at_entry);
	strcpy(sv[0]->pb, result);
	sv[0]->cb = strlen(result);
	strcpy(last_dial_result, result);

	/*
	 * do the _connect.ep or _connfail.ep execution
	 */
	if (!iv[0])
	{
		if (find_procedure("_connect"))
		{
			int erc2;

			_proc_args[0] = "_connect";	/* _connect.ep */
			_proc_args[1] = result;	/* "CONNECT XXXX" */
			if (erc2 = do_proc(2, _proc_args))
			{
				DCE_hangup();
				if (erc2 < 256)
				{
					sprintf(s64, "!CONNECT PROCEDURE RETURNED %d", erc2);
					result = s64;
				}
				else
					result = "!CONNECT PROCEDURE ABNORMAL TERMINATION";
				strcpy(sv[0]->pb, result);
				sv[0]->cb = strlen(result);
				setcolor(colors_error);
				pprintf("%s\n", result);
				iv[0] = 1;
				DCE_report_iv_set(0);
				erc = eConnectFailed;
			}
		}
	}
	else if (find_procedure("_connfail"))
	{
		_proc_args[0] = "_connfail";	/* _connfail.ep */
		_proc_args[1] = shm->Llogical;
		_proc_args[2] = result;
		(void)do_proc(3, _proc_args);
	}
	rcvr_conditional_restart(restart_rcvr, 1);
	return (erc);
}							 /* end of DCE_dial */

/*+-------------------------------------------------------------------------
	DCE_redial(arg,argc)
--------------------------------------------------------------------------*/
DCE_redial(arg, argc)
char **arg;
int argc;
{
	int erc = 0;
	int delay = 60;
	int retries = 10;
	long nap_msec;
	char ans;

	if (shm->Ltelno[0] == 0)
	{
		pprintf("   no previous number\n");
		return (-1);
	}

	if ((argc > 1) && ((retries = atoi(arg[1])) == 0))
	{
		pprintf("  invalid retry count\n");
		return (-1);
	}

	if ((argc > 2) && ((delay = atoi(arg[2])) == 0))
	{
		pprintf("  invalid delay\n");
		return (-1);
	}

	if (delay < 0)			 /* try to be nice to telcos */
		delay = 0;			 /* (they are our friends :-) */

	pprintf("  for %d retries, pause between: %d secs\n",
		retries, delay);

	kill_rcvr_process(SIGUSR1);	/* kill rcvr process */

	DCE_hangup();
	while (retries--)
	{
		if (!isdigit((uchar) shm->Llogical[0]) &&
			find_procedure(shm->Llogical))
		{
			char *_proc_args[2];
			UINT32 colors_at_entry = colors_current;

			_proc_args[0] = shm->Llogical;
			_proc_args[1] = "!REDIAL;";
			sigint = 0;
			ttymode(2);
			erc = do_proc(2, _proc_args);
			proc_file_reset();
			ttymode(1);
			setcolor(colors_notify);
			ff(se, "[procedure finished]");
			setcolor(colors_at_entry);
			ff(se, "\r\n");
			if (!erc)
			{
				start_rcvr_process(0);
				return (0);
			}
			lflash_dtr();
			sigint = 0;
		}
		else if (!(erc = DCE_dial()))
		{
			start_rcvr_process(1);
			return (0);
		}

		if (ck_sigint() || !strcmp(last_dial_result, interrupted_string))
			goto ABORT_CYCLE;

		if ((retries == 0) || (erc >= e_FATAL))
			break;

		pprintf("%d %s left ... ",
			retries, (retries == 1) ? "retry" : "retries");
		nap_msec = delay * 1000L;
		ff(se, "waiting %d seconds ... 'c' to cycle, %s to abort\r\n",
			delay, (kbdintr == DEL) ? "DEL" : graphic_char_text(kbdintr, 0));
		while (nap_msec > 0)
		{
			nap_msec -= Nap(100L);
			while (ttyrdchk())
			{
				ans = to_lower(ttygetc(1));
				if (ans == 'c')
					goto CONTINUE_CYCLE;
				else if (ans == (char)kbdintr)
					goto ABORT_CYCLE;
				else
					ring_bell();
			}
			if (ck_sigint())
				goto ABORT_CYCLE;
		}
	  CONTINUE_CYCLE:
		DCE_hangup();
	}

  ERROR_RETURN:
	DCE_hangup();
	start_rcvr_process(1);
	return (-1);

  ABORT_CYCLE:
	DCE_hangup();
	ff(se, "redial ABORTED\r\n");
	sigint = 0;
	goto ERROR_RETURN;
}							 /* end of DCE_redial */

/*+-------------------------------------------------------------------------
	DCE_now_on_hook() - DCE no longer in connection

This may be called, however, when no connection is active
--------------------------------------------------------------------------*/
void
DCE_now_on_hook()
{
	char s128[128];
	long connect_secs;

	lCLOCAL(1);				 /* turn on CLOCAL */

	if (shm->Lconnected)
	{
		connect_secs = time((long *)0) - shm->Loff_hook_time;
		sprintf(s128, "DISCONNECT %s (%s) %ld %s",
			shm->Llogical, shm->Ltelno, connect_secs,
			elapsed_time_text(connect_secs));
		logevent(getpid(), s128);
#if defined(WHT2) || defined(XTERM_FRIEND)

		/*
		 * if xterm, put disconnected status in title bar but this really
		 * should be done in _hangup.ep
		 */
		xterm_title("disconnected", 1);
#endif

		/*
		 * do the _hangup.ep execution
		 */
		if (find_procedure("_hangup"))
		{
			char *_hangup_args[2];

			sprintf(s128, "%ld", connect_secs);
			_hangup_args[0] = "_hangup";
			_hangup_args[1] = s128;
			(void)do_proc(2, _hangup_args);
		}
		shm->Lconnected = 0;
	}
	shm->Lrname[0] = 0;
	set_default_escape_prompt();

#ifdef CFG_TelnetOption
	if (shm->Ltelnet)
	{
		lclose();
		shm->Ltelnet = 0;
	}
#endif

}							 /* end of DCE_now_on_hook */

/*+-------------------------------------------------------------------------
	DCE_hangup() - terminate any active connection
--------------------------------------------------------------------------*/
void
DCE_hangup()
{
	int restart_rcvr = need_rcvr_restart();

	if (restart_rcvr)
		kill_rcvr_process(SIGUSR1);

	lflash_dtr();
	DCE_now_on_hook();

	if (restart_rcvr)
		start_rcvr_process(0);

}							 /* end of DCE_hangup */

/*+-------------------------------------------------------------------------
	DCE_get_sreg_value(regnum)
 assumes rcvr process has been killed
--------------------------------------------------------------------------*/
int
DCE_get_sreg_value(regnum)
int regnum;
{
	char s128[128];
	LRWT lr;

	sprintf(s128, "ATS%d?", regnum);
	DCE_send_cmd(s128);
	lflush(0);
	lr.to1 = 2000L;
	lr.to2 = 140L;
	lr.raw_flag = 0;
	lr.buffer = s128;
	lr.bufsize = sizeof(s128);
	lr.delim = (char *)0;
	lr.echo_flag = 0;
	lgets_timeout(&lr);
	if (lr.count != 3)
		return (-1);
	return (atoi(s128));

}							 /* end of DCE_get_sreg_value */

/*+-------------------------------------------------------------------------
	DCE_set_sreg(regnum,value)
 assumes rcvr process has been killed
 returns 0 if no error (reads back value set),
 else -1 and error message has been printed
--------------------------------------------------------------------------*/
int
DCE_set_sreg(regnum, value)
int regnum;
int value;
{
	char s128[128];
	int value2;
	LRWT lr;

	sprintf(s128, "ATS%d=%d", regnum, value);
	DCE_send_cmd(s128);
	lflush(0);
	lr.to1 = 2000L;
	lr.to2 = 140L;
	lr.raw_flag = 0;
	lr.buffer = s128;
	lr.bufsize = sizeof(s128);
	lr.delim = (char *)0;
	lr.echo_flag = 0;
	lgets_timeout(&lr);
	value2 = DCE_get_sreg_value(regnum);
	if (value2 < 0)
		pprintf("PROBLEM setting modem S%d=%d; cannot talk to modem\n",
			regnum, value);
	else if (value != value2)
		pprintf("PROBLEM setting modem S%d=%d; got %d back\n",
			regnum, value, value2);
	return ((value != value2) ? -1 : 0);

}							 /* end of DCE_set_sreg */

/*+-------------------------------------------------------------------------
	DCE_autoanswer()
--------------------------------------------------------------------------*/
void
DCE_autoanswer()
{
	if (!Lmodem_autoans[0])
		return;
	(void)Nap(200L);
	lputs_paced(20, "AT\r");
	(void)Nap(100L);
	lputs_paced(20, Lmodem_autoans);	/* quiet modem */
	lputs_paced(20, "\r");
	(void)Nap(200L);
	lputs_paced(20, Lmodem_autoans);	/* quiet modem */
	lputs_paced(20, "\r");
	(void)Nap(200L);
	lflush(0);
}							 /* end of DCE_autoanswer */

/* end of ecuDCE.c */
/* vi: set tabstop=4 shiftwidth=4: */
