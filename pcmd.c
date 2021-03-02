/*+-------------------------------------------------------------------------
	pcmd.c - ecu miscellaneous procedure commands
	wht@wht.net

  Defined functions:
	get_big_endian_16(ptr)
	get_big_endian_32(ptr)
	pcmd_ansif(param)
	pcmd_autorz(param)
	pcmd_ayt(param)
	pcmd_baud(param)
	pcmd_cd(param)
	pcmd_clrx(param)
	pcmd_dcdwatch(param)
	pcmd_dial(param)
	pcmd_duplex(param)
	pcmd_echo(param)
	pcmd_erto(param)
	pcmd_erverbose(param)
	pcmd_exec(param)
	pcmd_exit(param)
	pcmd_flush(param)
	pcmd_fork(param)
	pcmd_getf(param)
	pcmd_hangup(param)
	pcmd_hexdump(param)
	pcmd_kill(param)
	pcmd_lbreak(param)
	pcmd_lgets(param)
	pcmd_logevent(param)
	pcmd_lookfor(param)
	pcmd_nap(param)
	pcmd_nice(param)
	pcmd_parity(param)
	pcmd_popd(param)
	pcmd_prompt(param)
	pcmd_ptrace(param)
	pcmd_pushd(param)
	pcmd_putf(param)
	pcmd_rname(param)
	pcmd_rtscts(param)
	pcmd_send(param)
	pcmd_set(param)
	pcmd_setline(param)
	pcmd_system(param)
	pcmd_telopt(param)
	pcmd_xon(param)

--------------------------------------------------------------------------*/
/*+:EDITS:*/
/*:04-26-2000-11:16-wht@bob-RELEASE 4.42 */
/*:01-08-2000-14:50-wht@menlo-sockserve support */
/*:04-05-1998-17:57-wht@kepler-fix numeric ptrace arg */
/*:11-03-1997-02:10-wht@kepler-4.08a-option command */
/*:01-24-1997-02:37-wht@yuriatin-SOURCE RELEASE 4.00 */
/*:12-10-1996-19:25-wht@yuriatin-ensure echo has but one argument */
/*:10-16-1996-18:21-wht@yuriatin-clean up fork */
/*:10-16-1996-03:36-wht@yuriatin-add kill,fork */
/*:10-16-1996-03:08-wht@yuriatin-allow numeric argument to ptrace */
/*:10-16-1996-02:07-wht@yuriatin-emit newline before "lgets ended..." */
/*:09-18-1996-07:00-wht@yuriatin-beef up doc */
/*:09-18-1996-06:40-wht@yuriatin-pcmd_ayt */
/*:09-11-1996-20:00-wht@yuriatin-3.48-major telnet,curses,structural overhaul */
/*:08-20-1996-12:39-wht@kepler-locale/ctype fixes from ache@nagual.ru */
/*:08-11-1996-02:10-wht@kepler-rename ecu_log_event to logevent */
/*:11-23-1995-11:20-wht@kepler-source control 3.37 for tsx-11 */
/*:11-14-1995-10:23-wht@kepler-3.37.80-source control point: SOCKETS */
/*:11-12-1995-00:21-wht@gyro-add ansif and telopt cmds */
/*:11-04-1995-14:05-wht@wwtp1-ignore some commands if telnet */
/*:11-03-1995-16:54-wht@wwtp1-use CFG_TelnetOption */
/*:10-19-1995-01:14-wht@kepler-Ltelnet xon intercept */
/*:08-27-1995-07:04-wht@n4hgf-some line cmds now get only warn if no line */
/*:08-27-1995-06:28-wht@n4hgf-rtscts command w/o attached line ok */
/*:08-27-1995-06:24-wht@n4hgf-use shm->Lrtscts_val */
/*:03-21-1995-15:46-wht@n4hgf-add erto and erverbose */
/*:03-12-1995-01:03-wht@kepler-use ECU_MAXPN for up get_curr_dir */
/*:05-04-1994-04:39-wht@n4hgf-ECU release 3.30 */
/*:08-01-1993-02:39-wht@n4hgf-pass LRWT got_delim back to lgets caller in $i1 */
/*:09-10-1992-14:00-wht@n4hgf-ECU release 3.20 */
/*:09-06-1992-13:44-wht@n4hgf-rtscts would not accept a numeric argument */
/*:08-22-1992-15:39-wht@n4hgf-ECU release 3.20 BETA */
/*:01-12-1992-20:54-wht@n4hgf-add autorz command */
/*:12-12-1991-05:27-wht@n4hgf-proc_trace of intvar shows char value if 0-255 */
/*:11-11-1991-14:38-wht@n4hgf-add pcmd_dcdwatch code */
/*:10-09-1991-21:54-wht@n4hgf-add -p and -v switch to send */
/*:10-09-1991-20:32-wht@n4hgf-proc_trace code for send */
/*:09-01-1991-19:10-wht@n4hgf2-baud cmd can set rate even if no line open */
/*:09-01-1991-18:10-wht@n4hgf2-add setline */
/*:08-25-1991-14:39-wht@n4hgf-SVR4 port thanks to aega84!lh */
/*:08-06-1991-21:18-wht@n4hgf-nap -m test wrong sense ... old bug! */
/*:08-05-1991-16:22-wht@n4hgf-add nap -1 return and proc_trace */
/*:07-25-1991-12:58-wht@n4hgf-ECU release 3.10 */
/*:07-17-1991-07:04-wht@n4hgf-avoid SCO UNIX nap bug */
/*:06-05-1991-22:50-wht@n4hgf-fix parity cmd not taking alpha str */
/*:05-21-1991-18:52-wht@n4hgf-add pcmd_pushd and pcmd_popd */
/*:03-16-1991-15:12-wht@n4hgf-add pcmd_nice */
/*:01-09-1991-22:31-wht@n4hgf-ISC port */
/*:12-26-1990-02:34-wht@n4hgf-add cmd_rtscts */
/*:12-03-1990-04:59-wht@n4hgf-beef up pcmd_exit */
/*:09-19-1990-19:36-wht@n4hgf-logevent now gets pid for log from caller */
/*:08-14-1990-20:40-wht@n4hgf-ecu3.00-flush old edit history */

#include "ecu.h"
#include "ecuerror.h"
#include "termecu.h"
#include "ecukey.h"
#include "esd.h"
#include "var.h"
#include "procedure.h"

#define NAMED_VARIABLE_FLAG 0x1000L

#if defined(SVR4)
#include <sys/termiox.h>
extern int hx_flag;

#endif

char *tod_plus_msec_text();

extern int rc_ep_has_run;
extern UINT32 colors_current;
extern char errmsg[];
extern int expresp_verbosity;

#ifndef CFG_TelnetOption
static char *no_telnet_msg = "ECU is not configured for telnet operation.\n";

#endif

/*+-------------------------------------------------------------------------
	pcmd_autorz(param) - control automatic rz
--------------------------------------------------------------------------*/
int
pcmd_autorz(param)
ESD *param;
{
	char s8[8];

	if (get_alpha_zstr(param, s8, sizeof(s8)))
		return (eSyntaxError);
	if (!strcmp(s8, "on"))
		shm->autorz = 1;
	else if (!strcmp(s8, "off"))
		shm->autorz = 0;
	else
		return (eSyntaxError);
	shm->autorz_pos = 0;
	return (0);
}							 /* end of pcmd_autorz */

/*+-------------------------------------------------------------------------
	pcmd_baud(param) - set line or default bit rate

  The command sets shm->Lbitrate whether or not a line is open.
  If a line is open, the bit rate is actually set.
--------------------------------------------------------------------------*/
int
pcmd_baud(param)
ESD *param;
{
	long new_bitrate;
	int erc;

	if (erc = gint(param, &new_bitrate))
		return (erc);
	if (lnew_bitrate((UINT) new_bitrate))
	{
		pprintf("invalid bit rate: %lu\n", new_bitrate);
		return (eFATAL_ALREADY);
	}
	if (proc_trace)
		pprintf("bit rate set to %u\n", shm->Lbitrate);
#if defined(CFG_TelnetOption)
	if (!shm->Ltelnet)
		pputs("setting bit rate on telnet connections means little\n");
#endif /* defined(CFG_TelnetOption) */
	return (0);

}							 /* end of pcmd_baud */

/*+-------------------------------------------------------------------------
	pcmd_cd(param) - change directory
--------------------------------------------------------------------------*/
int
pcmd_cd(param)
ESD *param;
{
	int erc;
	ESD *tesd = esdalloc(ESD_NOMSZ);

	if (!tesd)
		return (eNoMemory);
	if (erc = gstr(param, tesd, 0))
		goto FUNC_RETURN;
	if (expand_dirname(tesd->pb, tesd->maxcb))
	{
		pprintf("%s\n", errmsg);
		param->index = param->old_index;
		erc = eFATAL_ALREADY;
		goto FUNC_RETURN;
	}
	if (chdir(tesd->pb) < 0) /* now change to the new directory */
	{
		pperror(tesd->pb);	 /* print error if we get one */
		pputs("\n");
		erc = eFATAL_ALREADY;
		goto FUNC_RETURN;
	}
	get_curr_dir(curr_dir, ECU_MAXPN);

  FUNC_RETURN:
	esdfree(tesd);
	return (erc);
}							 /* end of pcmd_cd */

/*+-------------------------------------------------------------------------
	pcmd_pushd(param) - push to another directory
--------------------------------------------------------------------------*/
int
pcmd_pushd(param)
ESD *param;
{
	int erc = 0;
	int arg_present;
	ESD *tesd = (ESD *) 0;

	if (arg_present = !!end_of_cmd(param))
	{
		if (!(tesd = esdalloc(ESD_NOMSZ)))
			return (eNoMemory);
		if (erc = gstr(param, tesd, 0))
			goto FUNC_RETURN;
	}

	if (!push_directory((arg_present) ? tesd->pb : "", arg_present, 1))
	{
		param->index = param->old_index;
		erc = eFATAL_ALREADY;
	}

  FUNC_RETURN:
	if (tesd)
		esdfree(tesd);
	return (erc);

}							 /* end of pcmd_pushd */

/*+-------------------------------------------------------------------------
	pcmd_popd(param) - pop to a previous directory
--------------------------------------------------------------------------*/
int
pcmd_popd(param)
ESD *param;
{
	int erc = 0;
	int arg_present;
	char allstr[8];

	allstr[0] = 0;
	if (arg_present = !!end_of_cmd(param))
	{
		if (get_alpha_zstr(param, allstr, sizeof(allstr)))
		{
			param->index = param->old_index;
			return (eSyntaxError);
		}
	}

	if (!pop_directory(allstr, arg_present, 1))
	{
		param->index = param->old_index;
		erc = eFATAL_ALREADY;
	}

	return (erc);

}							 /* end of pcmd_popd */

/*+-------------------------------------------------------------------------
	pcmd_clrx(param) - clear local xmtr XOFF hold
--------------------------------------------------------------------------*/
/*ARGSUSED*/
int
pcmd_clrx(param)
ESD *param;
{
	param = 0; /* not used */
#if defined(CFG_TelnetOption)
	if (shm->Ltelnet)
	{
		if (proc_trace)
			pprintf("clrx command ignored for telnet\n");
		return (0);
	}
#endif /* defined(CFG_TelnetOption) */

	if ((shm->Liofd < 0) && proc_trace)
	{
		pprintf("line not attached ... clrx ignored\n");
		return (0);
	}

	lclear_xmtr_xoff();
	if (proc_trace)
		pputs("transmitter XOFF cleared\n");
	return (0);
}							 /* end of pcmd_clrx */

/*+-------------------------------------------------------------------------
	pcmd_dcdwatch(param) - DCD watcher control
--------------------------------------------------------------------------*/
int
pcmd_dcdwatch(param)
ESD *param;
{
	int erc;
	char s16[16];
	char *cp;

	if (erc = get_alpha_zstr(param, s16, sizeof(s16)))
		return (erc);

#if defined(CFG_TelnetOption)
	if (shm->Ltelnet)
	{
		if (proc_trace)
			pprintf("dcdwatch command ignored for telnet\n");
		return (0);
	}
#endif /* defined(CFG_TelnetOption) */

	erc = (ldcdwatch_str(s16)) ? eSyntaxError : 0;
	if (!erc && proc_trace)
	{
		pputs("DCD watch set to ");
		cp = "???";
		switch (shm->Ldcdwatch)
		{
			case DCDW_OFF:
				cp = "off";
				break;
			case DCDW_ON:
				cp = "on";
				break;
			case DCDW_TERMINATE:
				cp = "TERMINATE";
				break;
		}
		pprintf("%s\n", cp);
	}
	return (0);

}							 /* end of pcmd_dcdwatch */

/*+-------------------------------------------------------------------------
	pcmd_dial(param) - connect to a remote DTE, local DCE, or inet host

  sets I0 to 0==connect,
             1==failed to connect,
             2==interrupted,
             3==modem error
  sets S0 to modem result code
--------------------------------------------------------------------------*/
int
pcmd_dial(param)
ESD *param;
{
	int erc;
	ESD *tesd = (ESD *) 0;

	if (shm->Lconnected)
	{
		pprintf("Already connected (to %s)\n", shm->Llogical);
		return (eFATAL_ALREADY);
	}

	if (!(tesd = esdalloc(64)))
		return (eNoMemory);

	if (erc = gstr(param, tesd, 0))
	{
		esdfree(tesd);
		return (erc);
	}

	if ((erc = call_logical_telno(tesd->pb)) && (erc == eConnectFailed))
		erc = 0;

	if (!erc && (shm->Liofd < 0))
		erc = eNoLineAttached;

	esdfree(tesd);

	return (erc);
}							 /* end of pcmd_dial */

/*+-------------------------------------------------------------------------
	pcmd_duplex(param) - set half/full duplex

duplex [f | h]
duplex ['f' | 'h']
duplex <int>  0 == half, non-0 == full
--------------------------------------------------------------------------*/
int
pcmd_duplex(param)
ESD *param;
{
	int erc;
	int new_duplex;
	ESD *tesd;

	if (erc = skip_cmd_break(param))
		return (erc);
	if (!(tesd = esdalloc(64)))
		return (eNoMemory);
	erc = gstr(param, tesd, 0);
	new_duplex = to_lower((erc) ? param->pb[param->index] : *tesd->pb);
	esdfree(tesd);
	erc = 0;

#if defined(CFG_TelnetOption)
	if (shm->Ltelnet)
	{
		if (proc_trace)
			pprintf("duplex command ignored for telnet\n");
		return (0);
	}
#endif /* defined(CFG_TelnetOption) */

	switch (new_duplex)
	{
		case 'f':
			shm->Lfull_duplex = 1;
			break;
		case 'h':
			shm->Lfull_duplex = 0;
			break;
		default:
			erc = eBadParameter;
	}
	if (proc_trace && !erc)
		pprintf("duplex set to %s\n", (shm->Lfull_duplex) ? "full" : "half");
	return (erc);

}							 /* end of pcmd_duplex */

/*+-------------------------------------------------------------------------
	pcmd_echo(param) - workhorse printer of strings

echo [-n] <str>
--------------------------------------------------------------------------*/
int
pcmd_echo(param)
ESD *param;
{
	int erc = 0;
	ESD *tesd = 0;
	char switches[8];

	if (!(tesd = esdalloc(ESD_NOMSZ)))
		return (eNoMemory);

	get_switches(param, switches, sizeof(switches));

	if (erc = gstr(param, tesd, 1))
		goto FUNC_RETURN;

	if (!end_of_cmd(param))
	{
		erc = eSyntaxError;
		goto FUNC_RETURN;
	}

	pputs(tesd->pb);
	if (!strchr(switches, 'n'))	/* if no -n */
		pputs("\n");

  FUNC_RETURN:
	esdfree(tesd);
	return (erc);

}							 /* end of pcmd_echo */

/*+-------------------------------------------------------------------------
	pcmd_erto(param) - expect/respond timeout control
--------------------------------------------------------------------------*/
int
pcmd_erto(param)
ESD *param;
{
	int erc;
	long int1;
	extern UINT32 expect_timeout_msecs;

	if (erc = gint(param, &int1))
		return (erc);
	if (int1 < 0)
		return (eBadParameter);
	expect_timeout_msecs = (UINT32) int1;
	if (proc_trace)
	{
		pprintf("expect-respond timeout set to %ld msec\n",
			expect_timeout_msecs);
	}
	return (0);
}							 /* end of pcmd_erto */

/*+-------------------------------------------------------------------------
	pcmd_erverbose(param) - expect/respond verbosity control
--------------------------------------------------------------------------*/
int
pcmd_erverbose(param)
ESD *param;
{
	int erc;
	uchar new_erverbose[8];

	if (erc = get_alphanum_zstr(param, new_erverbose, sizeof(new_erverbose)))
		return (erc);

	if ((tolower(new_erverbose[0]) != 'y') &&
		(tolower(new_erverbose[0]) != 'n'))
	{
		return (eBadParameter);
	}

	expresp_verbosity = (new_erverbose[0] == 'y');

	return (erc);
}							 /* end of pcmd_erverbose */

/*+-------------------------------------------------------------------------
	pcmd_exec(param) - Godel recursion: execute ecucmd in string
--------------------------------------------------------------------------*/
int
pcmd_exec(param)
ESD *param;
{
	int erc = 0;
	ESD *tesd = (ESD *) 0;

	if (!(tesd = esdalloc(64)))
		return (eNoMemory);
	if (erc = gstr(param, tesd, 1))
		goto FUNC_RETURN;

	/* reset indices */
	tesd->index = 0;
	tesd->old_index = 0;

	if (proc_trace)
		pprintf("executing: <%s>\n", tesd->pb);
	if (erc = execute_esd(tesd))
	{
		esdshow(tesd, "error executing dynamic statement:");
		proc_error(erc);
		erc = eFATAL_ALREADY;
	}

  FUNC_RETURN:
	if (tesd)
		esdfree(tesd);
	return (erc);

}							 /* end of pcmd_exec */

/*+-------------------------------------------------------------------------
	pcmd_exit(param) - terminate ECU process
--------------------------------------------------------------------------*/
int
pcmd_exit(param)
ESD *param;
{
	long int1;
	UINT32 colors_at_entry = colors_current;

	if (!gint(param, &int1) && int1)
	{
		setcolor(colors_error);
		pprintf("[procedure terminating ecu: user code %ld]\n", int1);
		setcolor(colors_at_entry);
		if ((int1 += TERMECU_USER1 - 1) > TERMECU_USERN)
		{
			int1 = TERMECU_USERN;
			pprintf("user exit code too large, using %d\r\n",
				TERMECU_USERN - TERMECU_USER1);
		}
		termecu((int)int1);
	}
	setcolor(colors_success);
	pputs("[procedure terminating ecu: normal exit]\n");
	setcolor(colors_at_entry);
	termecu(TERMECU_OK);
	return (0);				 /* for pesky compilers */
}							 /* end of pcmd_exit */

/*+-------------------------------------------------------------------------
	pcmd_lgets(param) - get a string from line

lgets [-er] <strvar> <int1> <int2> [<str>]

read string into string variable number <stvar>
waiting <int1> 1/10th secs for first char,
waiting <int2> 1/10th secs for subsequent chars,
optionally terminating read upon detection of <str>
-e echos to screen
-r completely raw, else strip CRs & NLs from either end of string
$i0 receives the length of the read
$i1 receives 1 if your specified delimiter was read, else zero
<strvar> receives the string
--------------------------------------------------------------------------*/
int
pcmd_lgets(param)
ESD *param;
{
	int erc;
	long int2;
	long int3;
	ESD *tesd1 = (ESD *) 0;
	ESD *svptr;
	LRWT lr;
	char switches[8];
	ESD *esdalloc();
	char ctmp;

	if (shm->Liofd < 0)
		return (eNoLineAttached);

	get_switches(param, switches, sizeof(switches));

	skip_cmd_char(param, '$');
	if (erc = get_cmd_char(param, &ctmp))
		return (erc);
	if (to_lower(ctmp) != 's')
		return (eIllegalVarType);
	if (erc = get_svptr(param, &svptr, 1))
		return (erc);

	if (erc = gint(param, &int2))
		return (erc);

	if (erc = gint(param, &int3))
		return (erc);

	if (!(tesd1 = esdalloc(64)))
		return (eNoMemory);
	if (gstr(param, tesd1, 1))	/* optional delimiter */
	{
		esdfree(tesd1);
		tesd1 = (ESD *) 0;
	}

	esdzero(svptr);			 /* no characters read yet */
	iv[0] = 0;				 /* no characters read yet */
	iv[1] = 0;				 /* no delim detected yet */

	lr.to1 = int2 * 100L;
	lr.to2 = int3 * 100L;
	/* allow interrupts + raw read per -r */
	lr.raw_flag = (strchr(switches, 'r')) ? 0x81 : 0x80;
	lr.buffer = svptr->pb;
	lr.bufsize = svptr->maxcb;
	lr.delim = (tesd1) ? tesd1->pb : (char *)0;
	lr.echo_flag = (strchr(switches, 'e') != (char *)0);

	if (proc_trace)
	{
		pprintf("\nlgets (to1=%ld,to2=%ld) started at %s\n",
			lr.to1, lr.to2, tod_plus_msec_text());
	}

	(void)lgets_timeout(&lr);
	if (tesd1)
		esdfree(tesd1);
	if(shm->Lsockserve && !shm->Lconnected)
	{
		if (proc_trace)
			pprintf("LOST CONNECTION DURING READ\n");
		sockserveException();
		return(eProcAttn_GOTO);
	}

	svptr->cb = lr.count;
	esd_null_terminate(svptr);
	iv[0] = (long)lr.count;
	iv[1] = (long)lr.got_delim;
	if (zero_length_read_detected)
	{
		zero_length_read_detected = 0;
		erc = eProcAttn_DCDloss;
	}
	if (proc_trace)
	{
		pprintf("\nlgets ended at %s (read %d chars) ",
			tod_plus_msec_text(), lr.count);
		pprintf("lgets set $i0 = %ld, $i01 = %ld\n", iv[0], iv[1]);
	}
	return (erc);

}							 /* end of pcmd_lgets */

/*+-------------------------------------------------------------------------
	pcmd_flush(param) - flush pending I/O
--------------------------------------------------------------------------*/
/*ARGSUSED*/
int
pcmd_flush(param)
ESD *param;
{
	param = 0; /* not used */
	if (shm->Liofd < 0)
	{
		if (proc_trace)
			pprintf("line not attached ... flush ignored\n");
		return (0);
	}

	lflush(2);
	if (proc_trace)
		pputs("line flushed\n");
	return (0);
}							 /* end of pcmd_flush */

/*+-------------------------------------------------------------------------
	pcmd_hangup(param) - disconnect from session

1. flashes DTR on serial line
2. closes telnet socket
--------------------------------------------------------------------------*/
/*ARGSUSED*/
int
pcmd_hangup(param)
ESD *param;
{
	param = 0; /* not used */
	if (shm->Liofd < 0)
	{
		if (proc_trace)
			pputs("no line attached ... hangup ignored\n");
		DCE_now_on_hook();
		return (0);
	}

	if (proc_trace)
		pputs("hanging up ... ");
	DCE_hangup();
	if (proc_trace)
		pputs("line on hook\n");
	return (0);
}							 /* end of pcmd_hangup */

/*+-------------------------------------------------------------------------
	pcmd_hexdump(param) - classic IBM-style columnar hex dump

hexdump [-s] <str>
hexdump -t[s] <str1> <str>
<str> buf to dump
<str1> title (if -t)
-s short (terse) dump
--------------------------------------------------------------------------*/
int
pcmd_hexdump(param)
ESD *param;
{
	int erc;
	ESD *title = 0;
	ESD *tesd = 0;
	char switches[8];
	extern FILE *plog_fp;

	if (!(tesd = esdalloc(ESD_NOMSZ)))
		return (eNoMemory);

	get_switches(param, switches, sizeof(switches));

	if (strchr(switches, 't'))	/* if -t */
	{
		if (!(title = esdalloc(ESD_NOMSZ)))
		{
			erc = eNoMemory;
			goto FUNC_RETURN;
		}
		if (erc = gstr(param, title, 0))
			goto FUNC_RETURN;
	}

	if (erc = gstr(param, tesd, 1))
		goto FUNC_RETURN;

	hex_dump(tesd->pb, tesd->cb, (title) ? title->pb : "",
		(strchr(switches, 's')) ? 1 : 0);

	if (plog_fp)
		hex_dump_fp(plog_fp, tesd->pb, tesd->cb, (title) ? title->pb : "",
			(strchr(switches, 's')) ? 1 : 0);

  FUNC_RETURN:
	if (tesd)
		esdfree(tesd);
	if (title)
		esdfree(title);
	return (erc);

}							 /* end of pcmd_hexdump */

/*+-------------------------------------------------------------------------
	pcmd_lbreak(param) - send asynchronous break
--------------------------------------------------------------------------*/
/*ARGSUSED*/
int
pcmd_lbreak(param)
ESD *param;
{
	param = 0; /* not used */
#if defined(CFG_TelnetOption)
	if (shm->Ltelnet)
	{
		if (proc_trace)
			pprintf("lbreak command ignored for telnet\n");
		return (0);
	}
#endif /* defined(CFG_TelnetOption) */

	if (shm->Liofd < 0)
	{
		if (proc_trace)
			pprintf("line not attached ... lbreak ignored\n");
		return (0);
	}

	lbreak();
	return (0);
}							 /* end of pcmd_lbreak */

/*+-------------------------------------------------------------------------
	pcmd_logevent(param) - write to ~/.ecu/log

logevent 'cmd'
--------------------------------------------------------------------------*/
int
pcmd_logevent(param)
ESD *param;
{
	int erc;
	ESD *eventstr;
	char switches[8];

	if (!(eventstr = esdalloc(ESD_NOMSZ)))
		return (eNoMemory);

	get_switches(param, switches, sizeof(switches));

	/* a hack */
	strcpy(eventstr->pb, "PROC ");
	eventstr->pb += 5;
	eventstr->maxcb -= 5;

	if (erc = gstr(param, eventstr, 0))
	{
		eventstr->pb -= 5;	 /* be nice */
		eventstr->maxcb += 5;/* or surely this will haunt us one day */
		esdfree(eventstr);
		return (erc);
	}

	/* rehack */
	eventstr->pb -= 5;
	eventstr->maxcb += 5;
	eventstr->cb += 5;

	logevent(getpid(), eventstr->pb);
	esdfree(eventstr);
	return (0);

}							 /* end of eventstr_logevent */

/*+-------------------------------------------------------------------------
	pcmd_lookfor(param) - scan line input for string

lookfor [-e] [quiet | <str>] [<int>]

-e echo to screen while looking
quiet means look for quiet
<str> means look for string
<int> number 1/10ths secs (default 5.0 second) for timeout

in case of lookfor <str>, $i0 plugged 1 if found, else 0
--------------------------------------------------------------------------*/
int
pcmd_lookfor(param)
ESD *param;
{
	int erc;
	char switches[8];
	char *cp = (char *)0;
	ESD *tesd = (ESD *) 0;
	UINT32 decisecs = 50;	 /* default wait is 5 seconds */
	int echo_flag;
	char s8[8];
	long start_secs;

	if (shm->Liofd < 0)
		return (eNoLineAttached);

	get_switches(param, switches, sizeof(switches));
	echo_flag = (strchr(switches, 'e') != (char *)0);

	if (!get_alpha_zstr(param, s8, sizeof(s8)))
	{
		if (strcmp(s8, "quiet"))
			return (eSyntaxError);
	}
	else
	{
		if (!(tesd = esdalloc(ESD_NOMSZ)))
			return (eNoMemory);
		if (erc = gstr(param, tesd, 0))
			goto FUNC_RETURN;
		if (!tesd->cb)
		{
			pputs("lookfor null string\n");
			erc = eFATAL_ALREADY;
			goto FUNC_RETURN;
		}
		cp = tesd->pb;
	}

	if (erc = gint(param, &decisecs))
	{
		/* if something there non-integer */
		if (!end_of_cmd(param))
		{
			erc = eSyntaxError;
			goto FUNC_RETURN;
		}
	}
	erc = 0;

	if (proc_trace)
		time(&start_secs);

	if (cp)
	{
		iv[0] = (long)llookfor(cp, decisecs * 100L, echo_flag);
		if (proc_trace)
			pprintf("lookfor set $i00 = %ld\n", iv[0]);
	}
	else
		lquiet(decisecs * 100L, echo_flag);

	if (proc_trace)
		pprintf("waited %ld secs\n", time((long *)0) - start_secs);

  FUNC_RETURN:
	if (tesd)
		esdfree(tesd);
	if (zero_length_read_detected)
	{
		zero_length_read_detected = 0;
		erc = eProcAttn_DCDloss;
	}
	return (erc);

}							 /* end of pcmd_lookfor */

/*+-------------------------------------------------------------------------
	pcmd_nap(param) - yawn for a while

nap [-m] <int>
<int> number 1/10ths secs, except if -m, nap <int> milliseconds
--------------------------------------------------------------------------*/
int
pcmd_nap(param)
ESD *param;
{
	int erc;
	char switches[8];
	UINT32 interval;

	get_switches(param, switches, sizeof(switches));

	if (erc = gint(param, &interval))
		return (erc);
	if (interval)
	{
		if (!strchr(switches, 'm'))
			interval *= 100L;
		if (interval < hzmsec)	/* SCO nap bug */
			interval = hzmsec;	/* SCO nap bug */
		if (proc_trace && (interval > 100))	/* short naps hurt by pprintf */
			pprintf("nap %ld msec\n", interval);
		if (Nap(interval) == -1)	/* EINTR is the only error returned
									 * ... */
		{					 /* but check anyway */
			if (errno == EINTR)
				erc = eCONINT;
		}
	}
	return (erc);
}							 /* end of pcmd_nap */

/*+-------------------------------------------------------------------------
	pcmd_nice(param) - change process nice
--------------------------------------------------------------------------*/
int
pcmd_nice(param)
ESD *param;
{
	long new_nice;
	int erc;
	int old_nice;
	int nice();

	if (erc = gint(param, &new_nice))
		return (erc);
	if ((new_nice < 0) || (new_nice > 39))
	{
		pprintf("warning: invalid nice %ld ignored (valid range 0-39)\n",
			new_nice);
		return (0);
	}

	old_nice = nice(0) + 20;
	nice(-old_nice + (int)new_nice);

	if (proc_trace)
		pprintf("nice desired %u, set to %u\n", (UINT) new_nice, nice(0) + 20);
	return (0);

}							 /* end of pcmd_nice */

/*+-------------------------------------------------------------------------
	pcmd_parity(param) - asynchronous line parity control

parity [e | o | n]
parity ['e' | 'o' | 'n']
--------------------------------------------------------------------------*/
int
pcmd_parity(param)
ESD *param;
{
	int erc = 0;
	int new_parity = 0;
	ESD *tesd = 0;
	char s64[64];

	if (erc = skip_cmd_break(param))
		goto FUNC_RETURN;
	if (!(tesd = esdalloc(64)))
	{
		erc = eNoMemory;
		goto FUNC_RETURN;
	}
	if (!gstr(param, tesd, 0))
		new_parity = to_lower(*tesd->pb);
	else if (!get_alpha_zstr(param, s64, sizeof(s64)))
		new_parity = to_lower(s64[0]);
	else
	{
		erc = eSyntaxError;
		goto FUNC_RETURN;
	}

#ifdef CFG_TelnetOption
	if (shm->Ltelnet)
	{
		if (proc_trace)
			pprintf("parity command ignored for telnet\n");
		goto FUNC_RETURN;
	}
#endif

	switch (new_parity)
	{
		case 'n':
			new_parity = 0;
		case 'e':
		case 'o':
			shm->Lparity = new_parity;
			if (shm->Liofd >= 0)
				lset_parity(1);
			break;
		default:
			erc = eBadParameter;
	}
	if (proc_trace && !erc)
	{
		pprintf("parity is %s\n",
			(shm->Lparity)
			? ((shm->Lparity == 'e') ? "even" : "odd")
			: "none");
	}

  FUNC_RETURN:
	esdfree(tesd);
	return (erc);

}							 /* end of pcmd_parity */

/*+-------------------------------------------------------------------------
	pcmd_prompt(param) - set interactive HOME prompt
--------------------------------------------------------------------------*/
int
pcmd_prompt(param)
ESD *param;
{
	extern ESD *icmd_prompt;

	return (gstr(param, icmd_prompt, 0));
}							 /* end of pcmd_prompt */

/*+-------------------------------------------------------------------------
	pcmd_ptrace(param) - procedure tracing control
--------------------------------------------------------------------------*/
int
pcmd_ptrace(param)
ESD *param;
{
	char s8[8];

	if (get_alphanum_zstr(param, s8, sizeof(s8)))
		return (eSyntaxError);
	if (isdigit((uchar) s8[0]))
		proc_trace = atoi(s8);
	else if (!strcmp(s8, "on"))
		proc_trace = 1;
	else if (!strcmp(s8, "off"))
		proc_trace = 0;
	else
		return (eSyntaxError);
	return (0);
}							 /* end of pcmd_ptrace */

/*+-------------------------------------------------------------------------
	pcmd_rname(param) - set remote name
--------------------------------------------------------------------------*/
int
pcmd_rname(param)
ESD *param;
{
	int itmp;
	int erc = 0;
	ESD *rname = 0;

	if (shm->Liofd < 0)
		return (eNoLineAttached);

	if (!shm->Lconnected)
	{
		pputs("Not connected\n");
		return (eFATAL_ALREADY);
	}

	/*
	 * get temp for rname param (all exits after here must go through
	 * FUNC_RETURN)
	 */
	if (!(rname = esdalloc(ESD_NOMSZ)))
		return (eNoMemory);

	if (erc = gstr(param, rname, 0))
		goto FUNC_RETURN;

	if (rname->cb > (itmp = sizeof(shm->Lrname) - 1))
	{
		pprintf("rname may only be %d characters long\n", itmp);
		erc = eFATAL_ALREADY;
		goto FUNC_RETURN;
	}

	strcpy(shm->Lrname, rname->pb);
	if (proc_trace)
		pprintf("rname set to '%s'\n", rname->pb);

  FUNC_RETURN:
	if (rname)
		esdfree(rname);
	return (erc);

}							 /* end of pcmd_rname */

/*+-------------------------------------------------------------------------
	pcmd_send(param) - send string on line

send [-nc] <str>
-n do not send trailing CR
-c send crlf
-v turn on proc_trace for just this statement
-p## pace characters ## msec apart
--------------------------------------------------------------------------*/
int
pcmd_send(param)
ESD *param;
{
	int erc;
	char *cp;
	ESD *buf = 0;
	ESD *exp = 0; /* NL to CRNL expansion */
	char switches[32];
	int send_cr;
	int send_nl;
	int tell_it;
	long pace_msec = 0L;

	if (shm->Liofd < 0)
		return (eNoLineAttached);

	if ((buf = esdalloc(ESD_NOMSZ)) == (ESD *) 0)
		return (eNoMemory);

	get_switches(param, switches, sizeof(switches));
	send_cr = !strchr(switches, 'n') | !!strchr(switches, 'c');
	send_nl = !!strchr(switches, 'c');
	tell_it = !!strchr(switches, 'v');
	if (cp = strchr(switches, 'p'))
		sscanf(cp + 1, "%ld", &pace_msec);

	if (erc = gstr(param, buf, 1))
	{
		esdfree(buf);
		return (erc);
	}

	if (proc_trace || tell_it)
	{
		hex_dump(buf->pb, buf->cb,
			(send_cr) ? "send with CR" : "send w/o CR", 1);
	}

	if(shm->Ltelnet)
	{
		int maxcb = buf->maxcb * 100;
		maxcb /= 90;
		if(!(exp = esdalloc(maxcb)))
		{
			erc = eNoMemory;
			goto EXIT;
		}
		buf->index = 0;
		while((buf->index < buf->cb) && (exp->cb < exp->maxcb - 1))
		{
			char ch = *(buf->pb + buf->index++);
			if(ch == NL)
				*(exp->pb + exp->cb++) = CRET;
			*(exp->pb + exp->cb++) = ch;
		}
		if(send_cr && (exp->cb < exp->maxcb))
			*(exp->pb + exp->cb++) = CRET;
		if(send_nl && (exp->cb < exp->maxcb))
			*(exp->pb + exp->cb++) = NL;
		if(write(shm->Liofd,exp->pb,exp->cb) < 0)
		{
			close(shm->Liofd);
			shm->Lconnected = 0;
			if(shm->Lsockserve)
			{
				sockserveException();
				erc = eProcAttn_GOTO;
				goto EXIT;
			}
		}
	}
	else
	{
		if (pace_msec)
			lputs_paced(pace_msec, buf->pb);
		else
			lputs(buf->pb);

		if (send_cr)
		{
			lputc(CRET);
			if (pace_msec)
				Nap(pace_msec);
		}
		if (send_nl)
		{
			lputc(NL);
			if (pace_msec)
				Nap(pace_msec);
		}
	}

EXIT:
	esdfree(buf);
	esdfree(exp);
	return (erc);
}							 /* end of pcmd_send */

/*+-------------------------------------------------------------------------
	pcmd_set(param) - set/display ECU variables
--------------------------------------------------------------------------*/
int
pcmd_set(param)
ESD *param;
{
	int erc;
	int itmp;
	UINT32 varnum;
	UINT varmax;
	char vartype;
	char varstr[16];
	int show_status;
	long *ivptr;
	ESD *svptr;
	char *cp;

	if (erc = skip_cmd_break(param))
		return (erc);

	do
	{

		/* $ is optional */
		if ((erc = skip_cmd_char(param, '$')) && (erc != eSyntaxError))
			return (erc);

		/* get variable type */
		if (get_cmd_char(param, &vartype))
			return (eSyntaxError);

		/* validate variable type */
		vartype = to_lower(vartype);
		switch (vartype)
		{
			case 'i':
				varmax = IVQUAN;
				break;
			case 's':
				varmax = SVQUAN;
				break;
			default:
				return (eIllegalVarType);
		}

		/*
		 * get variable pointer
		 */
		if (!get_numeric_value(param, &varnum))
			goto TEST_VARNUM;
		else if (*(param->pb + param->index) == '[')
		{
			if (erc = get_subscript(param, &varnum))
				return (erc);
		  TEST_VARNUM:
			if ((UINT) varnum >= varmax)
				return (eIllegalVarNumber);
			switch (vartype)
			{
				case 'i':
					ivptr = &iv[(int)varnum];
					break;
				default:
					svptr = sv[(int)varnum];
			}
		}
		else if (get_alphanum_zstr(param, varstr, sizeof(varstr)))
			return (eInvalidVarName);
		else
		{
			varnum = NAMED_VARIABLE_FLAG;
			switch (vartype)
			{
				case 'i':
					erc = find_mkvi(varstr, &ivptr, 1);
					break;
				default:
					erc = find_mkvs(varstr, &svptr, 1);
			}
			if (erc)
				return (erc);
		}

		/*
		 * now we assign ("=") any variable, or we
		 * increment ("++") or decrement ("--")
		 * integer variables
		 */
		show_status = 1;
		if (!skip_cmd_char(param, '='))	/* "=" assignment */
		{
			switch (vartype)
			{
				case 'i':
					if (erc = gint(param, ivptr))
						return (erc);
					break;
				default:
					if (erc = gstr(param, svptr, 1))
						return (erc);
					break;
			}
			if (!proc_trace)
				show_status = 0;
		}
		else if (((param->cb - param->index) >= 2) &&	/* "++" increment */
			!strncmp(param->pb + param->index, "++", 2))
		{
			param->index += 2;
			switch (vartype)
			{
				case 'i':
					(*ivptr)++;
					break;
				default:
					return (eInvalidStringOp);
					break;
			}
			if (!proc_trace)
				show_status = 0;
		}
		else if (((param->cb - param->index) >= 2) &&	/* "--" decrement */
			!strncmp(param->pb + param->index, "--", 2))
		{
			param->index += 2;
			switch (vartype)
			{
				case 'i':
					(*ivptr)--;
					break;
				default:
					return (eInvalidStringOp);
					break;
			}
			if (!proc_trace)
				show_status = 0;
		}
		else if ((((param->cb - param->index)) &&
			(*(param->pb + param->index) != ',')) && !end_of_cmd(param))
		{
		 /* not end of segment and not end of command */
			return (eSyntaxError);
		}

		/*
		 * the operation has been performed; do we show status?
		 */
		if (show_status)
		{
			switch (vartype)
			{
				case 'i':
					if (varnum != NAMED_VARIABLE_FLAG)
						pprintf("$i%02ld = %7ld (0x%08lx,0%03lo", varnum,
							*ivptr, *ivptr, *ivptr);
					else
						pprintf("$i%s = %ld (0x%08lx,0%03lo", varstr,
							*ivptr, *ivptr, *ivptr);
					if ((*ivptr >= 0) && (*ivptr <= 255))
						pprintf(",'%s'", graphic_char_text((char)*ivptr, 1));
					pputs(")\n");
					break;
				default:
					if (varnum != NAMED_VARIABLE_FLAG)
						pprintf("$s%02ld = '", varnum);
					else
						pprintf("$s%s = '", varstr);
					itmp = svptr->cb;
					cp = svptr->pb;
					while (itmp--)
						pputs(graphic_char_text(*cp++, 0));
					pputs("'\n");
					break;
			}
		}
	}
	while (!skip_comma(param));

	if (!end_of_cmd(param))
		return (eSyntaxError);

	return (0);
}							 /* end of pcmd_set */

/*+-------------------------------------------------------------------------
	pcmd_kill(param) - kill process

kill pid [signal]

signal defaults to 15

returns $i0 set to 0 if successful, else -1
if signal is 0 and error is not ESEARCH, treated as success
--------------------------------------------------------------------------*/
int
pcmd_kill(param)
ESD *param;
{
	int erc;
	long pid;
	long sig = SIGTERM;

	if (erc = gint(param, &pid))
		return (erc);

	if (erc = gint(param, &sig))
	{
		if (!end_of_cmd(param))
			return (eSyntaxError);
		erc = 0;
	}

	if (proc_trace)
		pprintf("killing pid %ld with signal %ld\n", pid, sig);

	iv[0] = (long)kill((CFG_PidType) pid, (int)sig);
	if ((sig == 0) && (iv[0] < 0) && (errno != ESRCH))
		iv[0] = 0;

	if (proc_trace)
		pprintf("kill set $i0 = %ld\n", iv[0]);

	return (0);
}							 /* end of pcmd_kill */

/*+-------------------------------------------------------------------------
	pcmd_fork(param) - spawn command and return

fork 'cmd'

returns $i0 set to pid of started process, else 0
--------------------------------------------------------------------------*/
int
pcmd_fork(param)
ESD *param;
{
	int erc;
	ESD *cmd;

	if (!(cmd = esdalloc(ESD_NOMSZ)))
		return (eNoMemory);

	if (erc = gstr(param, cmd, 1))
	{
		esdfree(cmd);
		return (erc);
	}

	if (proc_trace)
	{
		pputs("forking: ");
		pputs(cmd->pb);
		pputs("\n");
	}

	iv[0] = fork_cmd(cmd->pb);

	if (proc_trace)
		pprintf("fork set $i0 = %ld\n", iv[0]);

	esdfree(cmd);
	return (0);
}							 /* end of pcmd_fork */

/*+-------------------------------------------------------------------------
	pcmd_system(param) - spawn shell command and wait for completion

system [-ls] 'cmd'
-l makes comm line stdin/stdout
-s keeps all fds the same

returns $i0 set to exit status of program or 0x100 if interrupted
--------------------------------------------------------------------------*/
int
pcmd_system(param)
ESD *param;
{
	int erc;
	ESD *cmd;
	extern int last_child_wait_status;
	char switches[8];

	if ((cmd = esdalloc(ESD_NOMSZ)) == (ESD *) 0)
		return (eNoMemory);

	get_switches(param, switches, sizeof(switches));

/* a hack */
	*cmd->pb++ = (strchr(switches, 's')) ? '>' :
		((strchr(switches, 'l')) ? '$' : '!');

	cmd->maxcb--;

	if (erc = gstr(param, cmd, 1))
	{
		cmd->pb--;			 /* be nice */
		cmd->maxcb++;		 /* or surely this will haunt us one day */
		esdfree(cmd);
		return (erc);
	}

/* rehack */
	cmd->pb--;
	cmd->cb++;
	cmd->maxcb++;

	if (proc_trace)
	{
		pputs(cmd->pb + 1);
		pputs("\n");
	}

	last_child_wait_status = 0xFF00;
	shell(cmd->pb);
	iv[0] = (last_child_wait_status & 0xFF)
		? 0x100L : (long)last_child_wait_status >> 8;
	if (proc_trace)
		pprintf("system set $i0 = %ld (%s)\n", iv[0],
			(iv[0] == 0x100L) ? "interrupted" : "program exit status");

	esdfree(cmd);
	return (0);
}							 /* end of pcmd_system */

/*+-------------------------------------------------------------------------
	get_big_endian_16(ptr) - byte order hack
--------------------------------------------------------------------------*/
UINT16
get_big_endian_16(ptr)
uchar *ptr;
{
	UINT16 uint16 = ((UINT16) ptr[0] << 8) | ptr[1];

	return (uint16);

}							 /* end of get_big_endian_16 */

/*+-------------------------------------------------------------------------
	get_big_endian_32(ptr) - byte order hack
--------------------------------------------------------------------------*/
UINT32
get_big_endian_32(ptr)
uchar *ptr;
{
	UINT32 uint32 = ((UINT32) * ptr++) << 24;

	uint32 |= ((UINT32) * ptr++) << 16;
	uint32 |= ((UINT32) * ptr++) << 8;
	uint32 |= (UINT32) * ptr++;
	return (uint32);

}							 /* end of get_big_endian_32 */

/*+-------------------------------------------------------------------------
	pcmd_getf(param) - get friend memory

getf -x <int-var-spec> <offset>
where: -x ==
   -b byte
   -w word (little-endian)
   -W word (big-endian)
   -l 32-bits (little-endian)
   -L 32-bits (big-endian)
--------------------------------------------------------------------------*/
int
pcmd_getf(param)
ESD *param;
{
	int erc;
	uchar switches[8];
	long *piv;
	long offset;
	int size;
	int big_endian;

	if (erc = get_switches(param, switches, sizeof(switches)))
		return (erc);
	if ((strlen(switches) != 2) || !strchr("bwWlL", switches[1]))
	{
		pputs("invalid switch\n");
		return (eFATAL_ALREADY);
	}
	size = to_lower(switches[1]);
	big_endian = isupper(switches[1]);

#if 0
	if (!get_svptr(param, &psv))
		return (eNotImplemented);
	else
#endif
	if (!strncmp(param->pb + param->index, "$i", 2))
		param->index += 2;
	if (erc = get_ivptr(param, &piv, 1))
		return (erc);

	if (erc = gint(param, &offset))
		return (erc);

	if (proc_trace)
		pprintf("getf %s offset=0x%lx", switches, offset);

	switch (size)
	{
		case 'b':
			if (offset > ((long)sizeof(shm->friend_space) - 1))
				goto OFFSET_TOO_LARGE;
			*piv = *(((uchar *) shm->friend_space) + (int)offset) & 0xFF;
			break;
		case 'w':
			if (offset > ((long)sizeof(shm->friend_space) - 2))
				goto OFFSET_TOO_LARGE;
			if (big_endian)
				*piv = get_big_endian_16((uchar *) shm->friend_space +
					(int)offset);
			else
				*piv = *(((UINT16 *) shm->friend_space) + (int)offset) & 0xFFFF;
			break;
		case 'l':
			if (offset > ((long)sizeof(shm->friend_space) - 4))
				goto OFFSET_TOO_LARGE;
			if (big_endian)
			{
				*piv = get_big_endian_32((uchar *) shm->friend_space +
					(int)offset);
			}
			else
				*piv = *((long *)((char *)shm->friend_space + (int)offset));
			break;
	}

	if (proc_trace)
		pprintf(" value=%ld (%08lx)\n", *piv, *piv);
	return (0);

  OFFSET_TOO_LARGE:
	if (proc_trace)
		pputs("\n");
	pprintf("offset 0x%02lx too large for -%c (0x%02x bytes available)\n",
		offset, switches[1], sizeof(shm->friend_space));
	return (eFATAL_ALREADY);

}							 /* end of pcmd_getf */

/*+-------------------------------------------------------------------------
	pcmd_putf(param) - put friend memory
--------------------------------------------------------------------------*/
/*ARGSUSED*/
int
pcmd_putf(param)
ESD *param;
{
	param = 0; /* not used */
	return (eNotImplemented);
}							 /* end of pcmd_putf */

/*+-------------------------------------------------------------------------
	pcmd_xon(param) - control asynchronous software flow control
--------------------------------------------------------------------------*/
int
pcmd_xon(param)
ESD *param;
{
	int erc;
	char new_state[8];
	char *xon_status();

	if (erc = get_alpha_zstr(param, new_state, sizeof(new_state)))
		return (erc);

#if defined(CFG_TelnetOption)
	if (shm->Ltelnet)
	{
		if (proc_trace)
			pprintf("xon command ignored for telnet\n");
		return (0);
	}
#endif /* defined(CFG_TelnetOption) */

	if (set_xon_xoff_by_arg(new_state))
		return (eBadParameter);

	if (shm->Liofd < 0)
		return (0);

	if (proc_trace)
		pprintf("xon/xoff flow control set to %s\n", xon_status());

	return (erc);

}							 /* end of pcmd_xon */

/*+-------------------------------------------------------------------------
	pcmd_rtscts(param) - control asynchronous hardware flow control
--------------------------------------------------------------------------*/
int
pcmd_rtscts(param)
ESD *param;
{
	int erc;
	char new_rtscts[8];

#if defined(CFG_TelnetOption)
	if (shm->Ltelnet)
	{
		if (proc_trace)
			pprintf("rtscts command ignored for telnet\n");
		return (0);
	}
#endif /* defined(CFG_TelnetOption) */

	if ((shm->Liofd < 0) && proc_trace)
		pprintf("line not attached ... rtscts setting saved for later\n");

	if (erc = get_alphanum_zstr(param, new_rtscts, sizeof(new_rtscts)))
		return (erc);

#if defined(HW_FLOW_CONTROL) /* see ecu.h */
	lRTSCTS_control(shm->Lrtscts_val = yes_or_no(new_rtscts));

	if (proc_trace)
		display_hw_flow_config();
#else
	if (proc_trace)
		pprintf("hardware flow control not available .... rtscts ignored\n");
#endif /* RTSFLOW */

	return (erc);
}							 /* end of pcmd_rtscts */

/*+-------------------------------------------------------------------------
	pcmd_setline(param) - _rc.ep setline command

This command can be used to set the initial line in _rc.ep
--------------------------------------------------------------------------*/
int
pcmd_setline(param)
ESD *param;
{
	int erc;
	ESD *tesd;

	if (rc_ep_has_run)
	{
		pprintf("setline command legal only in _rc.ep\n");
		return (eFATAL_ALREADY);
	}

	if (!(tesd = esdalloc(sizeof(shm->Lline))))
		return (eNoMemory);
	if (erc = gstr(param, tesd, 0))
		goto FUNC_RETURN;
	shm->Lline[0] = 0;
	if (strncmp(tesd->pb, "/dev/", 5))
		strcat(shm->Lline, "/dev/");
	strncat(shm->Lline, tesd->pb, sizeof(shm->Lline) - strlen(shm->Lline));
	shm->Lline[sizeof(shm->Lline) - 1] = 0;
	if (proc_trace)
		pprintf("line set to %s\n", shm->Lline);

  FUNC_RETURN:
	esdfree(tesd);
	return (erc);
}							 /* end of pcmd_setline */

/*+-------------------------------------------------------------------------
	pcmd_ansif(param) - ANSI filter control
--------------------------------------------------------------------------*/
pcmd_ansif(param)
ESD *param;
{
	int erc;
	char new_state[8];

	if (erc = get_alpha_zstr(param, new_state, sizeof(new_state)))
		return (erc);

	rcvr_ansi_filter_control(yes_or_no(new_state), !!proc_trace);

	return (erc);

}							 /* end of pcmd_ansif */

/*+-------------------------------------------------------------------------
	pcmd_ayt(param) - send telnet AYT
--------------------------------------------------------------------------*/
/*ARGSUSED*/
pcmd_ayt(param)
ESD *param;
{
	param = 0; /* not used */

#ifdef CFG_TelnetOption
	telnet_xmit_AYT();
#else
	if (proc_trace)
		pputs(no_telnet_msg);
#endif
	return (0);

}							 /* end of pcmd_ayt */

/*+-------------------------------------------------------------------------
	pcmd_telopt(param) - control telnet option traffic display
--------------------------------------------------------------------------*/
int
pcmd_telopt(param)
ESD *param;
{
	int erc = 0;
	char new_state[8];

	if (erc = get_alpha_zstr(param, new_state, sizeof(new_state)))
		return (erc);

#ifdef CFG_TelnetOption
	shm->show_telnet_traffic = yes_or_no(new_state);

	if (proc_trace)
	{
		if (!shm->Ltelnet)
			pputs("although you are not currently in a telnet session,\n");
		pprintf("telnet option display set to %s\n",
			(shm->show_telnet_traffic) ? "ON" : "off");
	}
#else
	if (proc_trace)
		pputs(no_telnet_msg);
#endif

	return (erc);

}							 /* end of pcmd_telopt */

/*+-------------------------------------------------------------------------
	pcmd_option(param) - procedure options
--------------------------------------------------------------------------*/
int
pcmd_option(param)
ESD *param;
{
	long state;
	char state_name[32];
	int state_num = 0;
	int erc;

	if (erc = get_alpha_zstr(param, state_name, sizeof(state_name)))
		return (erc);

#define PS_LOCAL_VARS 1
	if (!strcmp(state_name, "localvars"))
		state_num = PS_LOCAL_VARS;
	else
	{
		pprintf("invalid option name\n");
		return (eFATAL_ALREADY);
	}
	if (erc = skip_cmd_break(param))
		return (erc);
	if ((param->index >= param->cb) || (*(param->pb + param->index) != '='))
	{
		pprintf("expected '=' here\n");
		return (eFATAL_ALREADY);
	}
	param->index++;
	if (erc = gint(param, &state))
		return (erc);

	state = !!state;

	if (state_num == PS_LOCAL_VARS)
		proc_option_localvars = (int)state;
	else
		return (eInternalLogicError);

	if (proc_trace)
	{
		pprintf("option command set \"%s\" to %s\n", state_name,
			(state) ? "TRUE" : "FALSE");
	}
	return (0);
}	/* end of pcmd_option */

/* vi: set tabstop=4 shiftwidth=4: */
/* end of pcmd.c */
