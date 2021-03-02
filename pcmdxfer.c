/*+-------------------------------------------------------------------------
	pcmdxfer.c - ecu file transfer related procedure commands
	wht@wht.net

  Defined functions:
	_adjust_erc_and_iv0(adj_erc)
	_make_bottom_label(param, default_flag, sending_flag)
	_pcmd_report_send_status()
	_smart_sender_common(cmd)
	pcmd_rk(param)
	pcmd_rx(param)
	pcmd_ry(param)
	pcmd_rz(param)
	pcmd_sk(param)
	pcmd_sx(param)
	pcmd_sy(param)
	pcmd_sz(param)

--------------------------------------------------------------------------*/
/*+:EDITS:*/
/*:04-26-2000-11:16-wht@bob-RELEASE 4.42 */
/*:02-25-1997-14:36-wht@yuriatin-anonymous caller fix for sx/sy (-@ missing) */
/*:01-24-1997-02:38-wht@yuriatin-SOURCE RELEASE 4.00 */
/*:09-11-1996-20:01-wht@yuriatin-3.48-major telnet,curses,structural overhaul */
/*:01-01-1996-18:57-wht@kepler-fix sz with force overwrite */
/*:11-27-1995-20:16-wht@kepler-add -T on sz */
/*:11-27-1995-20:15-wht@kepler-CMDESD_INITIAL_SIZE from 384 to 512 */
/*:11-27-1995-11:57-wht@kepler-honor protocol_log_packets */
/*:11-24-1995-09:57-wht@kepler-improve rz command for telnet */
/*:11-23-1995-11:20-wht@kepler-source control 3.37 for tsx-11 */
/*:11-14-1995-10:23-wht@kepler-3.37.80-source control point: SOCKETS */
/*:10-14-1995-23:49-wht@kepler-switch -@ now passed line baud value */
/*:10-14-1995-23:22-wht@kepler-drop SEAlink support */
/*:03-12-1995-03:27-wht@kepler-use ECU_MAXPN */
/*:05-04-1994-04:39-wht@n4hgf-ECU release 3.30 */
/*:10-21-1992-19:15-wht@n4hgf-proc file xfer didnt learn about eculibdir */
/*:09-10-1992-14:00-wht@n4hgf-ECU release 3.20 */
/*:09-05-1992-15:35-wht@n4hgf-add -r to sz */
/*:08-22-1992-15:39-wht@n4hgf-ECU release 3.20 BETA */
/*:07-25-1991-12:59-wht@n4hgf-ECU release 3.10 */
/*:04-23-1991-23:44-wht@n4hgf-big time overhaul - better but flames expected */
/*:04-23-1991-05:10-wht@n4hgf-new cmd build mechanism for long file lists */
/*:01-17-1991-17:01-wht@n4hgf-skipped files in sz aborted proc */
/*:08-14-1990-20:40-wht@n4hgf-ecu3.00-flush old edit history */

#include "ecu.h"
#include "ecukey.h"
#include "ecuerror.h"
#include "esd.h"
#include "var.h"
#include "procedure.h"

/*
 * file transmission command and pathname list ESD initial size
 * (lengths can grow to ESD_MAXSZ)
 */
#define CMDESD_INITIAL_SIZE  512	/* executable command initial size */
#define PATHESD_INITIAL_SIZE 256	/* pathname list initial size */
#define CMDSTR_SIZE          384	/* receive command string size */

extern int last_child_wait_status;
extern char curr_dir[ECU_MAXPN];
extern int protocol_log_packets;

static char bottom_label[80];

/*+-------------------------------------------------------------------------
	_make_bottom_label(param,default_flag)
--------------------------------------------------------------------------*/
int
_make_bottom_label(param, default_flag, sending_flag)
ESD *param;
int default_flag;
int sending_flag;
{
	int erc;
	ESD *label = (ESD *) 0;

	if (default_flag)
	{
		sprintf(bottom_label,
			(sending_flag) ? "-C \"'Connected to %s'\" "
			: "-C 'Connected to %s' ",
			(shm->Lrname[0]) ? shm->Lrname : "?");
		return (0);
	}
	if ((label = esdalloc(64)) == (ESD *) 0)
		return (eNoMemory);
	if (erc = gstr(param, label, 0))
	{
		esdfree(label);
		return (erc);
	}
	strcpy(bottom_label, (sending_flag) ? "-C \"'" : "-C '");
	strcat(bottom_label, label->pb);
	strcat(bottom_label, (sending_flag) ? "'\"" : "'");
	esdfree(label);
	return (0);

}							 /* end of _make_bottom_label */

/*+-------------------------------------------------------------------------
	_adjust_erc_and_iv0(adj_erc)
--------------------------------------------------------------------------*/
int
_adjust_erc_and_iv0(adj_erc)
int adj_erc;
{
	if (adj_erc)
		return (adj_erc);

	iv[0] = (last_child_wait_status & 0xFF)
		? 0x100L : (long)((last_child_wait_status >> 8) & 0xFFFF);

	if (proc_trace)
		pprintf("$i00 = %ld (transfer %s)\n", iv[0],
			(iv[0] == 0x100L) ? "interrupted" : "program exit status");

	return (0);

}							 /* end of _adjust_erc_and_iv0 */

/*+-------------------------------------------------------------------------
	_pcmd_report_send_status() - report file transmission result

for "ecu knowledgeable" protocols only
returns proc-type erc
--------------------------------------------------------------------------*/
int
_pcmd_report_send_status()
{
	UINT16 ustmp;
	int erc = eFATAL_ALREADY;
	int iv0_set = 0;
	char *signal_name_text();

	pputs("\n");
	ustmp = last_child_wait_status;
	if ((ustmp & 0xFF) == 0) /* exit() called */
	{
		ustmp >>= 8;
		if (!ustmp)
		{
			if (proc_trace)
				pputs("transfer successful\n");
			erc = 0;
		}
		else if (ustmp == 255)
			pputs("ecu error: transfer program usage error\n");
		else if (ustmp == 254)
		{
			pputs(
				"protocol failure: bad line conditions or remote not ready\n");
		}
		else if (ustmp == 253)
			pputs("no requested files exist\n");
		else if (ustmp < 128)
		{
			if (proc_trace)
			{
				if (ustmp == 127)
					pputs("127 or more files skipped\n");
				else
					pprintf("%u files rejected\n", ustmp);
			}
			iv[0] = (long)ustmp;
			iv0_set = 1;
			last_child_wait_status = 0;
			erc = 0;
		}
		else
		{
			pprintf("transfer aborted by %s\n", signal_name_text(ustmp & 0x7F));
			erc = eProcAttn_Interrupt;
		}
	}
	else
	{
		pprintf("transfer killed by %s\n", signal_name_text(ustmp & 0x7F));
		erc = eProcAttn_Interrupt;
	}

	if (!iv0_set)
	{
		iv[0] = (last_child_wait_status & 0xFF)
			? 0x100L : (long)((last_child_wait_status >> 8) & 0xFFFF);
	}

	if (proc_trace)
	{
		pprintf("$i00 = %ld (transfer %s)\n", iv[0],
			(iv[0] == 0x100L) ? "interrupted" : "program exit status");
	}

	return (erc);

}							 /* end of _pcmd_report_send_status */

/*+-------------------------------------------------------------------------
	_smart_sender_common(cmd) - common exec and bottom processing

for "ecu knowledgeable" protocols only
returns proc-type erc
--------------------------------------------------------------------------*/
int
_smart_sender_common(cmd)
char *cmd;
{
	int erc = 0;

	last_child_wait_status = 0;
	file_xfer_start();

	if (find_shell_chars(cmd))
	{
		char *expcmd;

		if (expand_wildcard_list(cmd, &expcmd))
		{
			pputs("No files match wildcard list\n");
			iv[0] = -1;
			return (0);
		}
		else
		{
			exec_cmd(expcmd);
			erc = _pcmd_report_send_status();
			free(expcmd);
		}
	}
	else
	{
		exec_cmd(cmd);
		erc = _pcmd_report_send_status();
	}

	lreset_ksr();
	file_xfer_done_bell();

	return (erc);

}							 /* end of _smart_sender_common */

/*+-------------------------------------------------------------------------
	pcmd_sx(param)

sx [-ak[l]] [<label-str>] <filelist-str>
--------------------------------------------------------------------------*/
int
pcmd_sx(param)
ESD *param;
{
	int erc;
	char switches[8];
	ESD *pathesd = (ESD *) 0;
	ESD *cmdesd = esdalloc(CMDESD_INITIAL_SIZE);

	get_switches(param, switches, sizeof(switches));

	if (erc = _make_bottom_label(param, !strchr(switches, 'l'), 1))
		return (erc);

	/*
	 * build command at beginning of 'cmdesd' ESD
	 */
	sprintf(cmdesd->pb, "%s/ecusz -X -@ %d -. %d ",
		eculibdir, shm->Lbitrate, shm->Liofd);
	strcat(cmdesd->pb, bottom_label);

	if (strchr(switches, 'p') || protocol_log_packets)
		strcat(cmdesd->pb, "-, ");

	if (strchr(switches, 'a'))
		strcat(cmdesd->pb, "-a ");

	if (strchr(switches, 'k'))
		strcat(cmdesd->pb, "-k");

	if (tty_not_char_special || (shm->ttyuse == TTYUSE_FORCE_SIMPLE) ||
		strchr(switches, 'N'))	/* force "no curses" */
	{
		strcat(cmdesd->pb, "-_ ");
	}

	/*
	 * update cmdesd esd
	 */
	cmdesd->cb = strlen(cmdesd->pb);

	if (!(pathesd = esdalloc(PATHESD_INITIAL_SIZE)))
	{
		erc = eNoMemory;
		goto FREE_MEM_AND_EXIT;
	}

	/*
	 * get list of pathnames to send
	 */
	if (erc = gstr(param, pathesd, 1))
		goto FREE_MEM_AND_EXIT;

	/*
	 * append filelist to command
	 */
	if (erc = esdcat(cmdesd, pathesd, 1))
		goto FREE_MEM_AND_EXIT;

	/*
	 * perform the operation
	 */
	erc = _smart_sender_common(cmdesd->pb);

  FREE_MEM_AND_EXIT:
	if (pathesd)
		esdfree(pathesd);
	if (cmdesd)
		esdfree(cmdesd);
	return (erc);

}							 /* end of pcmd_sx */

/*+-------------------------------------------------------------------------
	pcmd_sy(param)

sy [-a[l]] [<label-str>] <filelist-str>
--------------------------------------------------------------------------*/
int
pcmd_sy(param)
ESD *param;
{
	int erc;
	char switches[8];
	ESD *pathesd = (ESD *) 0;
	ESD *cmdesd = esdalloc(CMDESD_INITIAL_SIZE);

	get_switches(param, switches, sizeof(switches));

	if (erc = _make_bottom_label(param, !strchr(switches, 'l'), 1))
		return (erc);

	/*
	 * build command at beginning of 'cmdesd' ESD
	 */
	sprintf(cmdesd->pb, "%s/ecusz -Y -@ %d -. %d ",
		eculibdir, shm->Lbitrate, shm->Liofd);
	strcat(cmdesd->pb, bottom_label);
	if (strchr(switches, 'a'))
		strcat(cmdesd->pb, "-a ");
	else
		strcat(cmdesd->pb, "-b ");
	if (strchr(switches, 'p') || protocol_log_packets)
		strcat(cmdesd->pb, "-, ");
	if (tty_not_char_special || (shm->ttyuse == TTYUSE_FORCE_SIMPLE) ||
		strchr(switches, 'N'))	/* force "no curses" */
	{
		strcat(cmdesd->pb, "-_ ");
	}

	/*
	 * update cmdesd esd
	 */
	cmdesd->cb = strlen(cmdesd->pb);

	if (!(pathesd = esdalloc(PATHESD_INITIAL_SIZE)))
	{
		erc = eNoMemory;
		goto FREE_MEM_AND_EXIT;
	}

	/*
	 * get list of pathnames to send
	 */
	if (erc = gstr(param, pathesd, 1))
		goto FREE_MEM_AND_EXIT;

	/*
	 * append filelist to command
	 */
	if (erc = esdcat(cmdesd, pathesd, 1))
		goto FREE_MEM_AND_EXIT;

	/*
	 * perform the operation
	 */
	erc = _smart_sender_common(cmdesd->pb);

  FREE_MEM_AND_EXIT:
	if (pathesd)
		esdfree(pathesd);
	if (cmdesd)
		esdfree(cmdesd);
	return (erc);

}							 /* end of pcmd_sy */

/*+-------------------------------------------------------------------------
	pcmd_sz(param)

sz [-anf[l]] [<label-str>] <filelist-str>
-a ascii, else binary
-n send only newer, else all files
-f full, else simple pathnames
-r resume interrupted xfer
-l non-default bottom line label on transfer string
$i0 set to:
	0: file transfer completely successful
	-1 program did not run
--------------------------------------------------------------------------*/
int
pcmd_sz(param)
ESD *param;
{
	int erc;
	char switches[8];
	ESD *pathesd = (ESD *) 0;
	ESD *cmdesd = esdalloc(CMDESD_INITIAL_SIZE);

	if (!cmdesd)
		return (eNoMemory);

	get_switches(param, switches, sizeof(switches));

	if (erc = _make_bottom_label(param, !strchr(switches, 'l'), 1))
		return (erc);

	/*
	 * build command at beginning of 'cmdesd' ESD
	 */
	sprintf(cmdesd->pb, "%s/ecusz -Z -@ %d -. %d ",
		eculibdir, shm->Lbitrate, shm->Liofd);
	strcat(cmdesd->pb, bottom_label);

	if (tty_not_char_special || (shm->ttyuse == TTYUSE_FORCE_SIMPLE) ||
		strchr(switches, 'N'))	/* force "no curses" */
	{
		strcat(cmdesd->pb, "-_ ");
	}

	if (shm->Ltelnet)
		strcat(cmdesd->pb, "-T ");

	if (strchr(switches, 'a'))
		strcat(cmdesd->pb, "-a ");

	if (strchr(switches, 'n'))
		strcat(cmdesd->pb, "-n ");
	else
		strcat(cmdesd->pb, "-y ");

	if (strchr(switches, 'f'))
		strcat(cmdesd->pb, "-f ");

	if (strchr(switches, 'r'))
		strcat(cmdesd->pb, "-r ");

	if (strchr(switches, 'p') || protocol_log_packets)
		strcat(cmdesd->pb, "-, ");

	/*
	 * update cmdesd esd
	 */
	cmdesd->cb = strlen(cmdesd->pb);

	if (!(pathesd = esdalloc(PATHESD_INITIAL_SIZE)))
	{
		erc = eNoMemory;
		goto FREE_MEM_AND_EXIT;
	}

	/*
	 * get list of pathnames to send
	 */
	if (erc = gstr(param, pathesd, 1))
		goto FREE_MEM_AND_EXIT;

	/*
	 * append filelist to command
	 */
	if (erc = esdcat(cmdesd, pathesd, 1))
		goto FREE_MEM_AND_EXIT;

	/*
	 * perform the operation
	 */
	erc = _smart_sender_common(cmdesd->pb);

  FREE_MEM_AND_EXIT:
	if (pathesd)
		esdfree(pathesd);
	if (cmdesd)
		esdfree(cmdesd);
	return (erc);

}							 /* end of pcmd_sz */

/*+-------------------------------------------------------------------------
	pcmd_sk(param)

sk [-a] <str>
--------------------------------------------------------------------------*/
int
pcmd_sk(param)
ESD *param;
{
	int erc;
	char switches[8];
	ESD *pathesd = (ESD *) 0;
	ESD *cmdesd = esdalloc(CMDESD_INITIAL_SIZE);

	if (!cmdesd)
		return (eNoMemory);

	get_switches(param, switches, sizeof(switches));

	/*
	 * build command at beginning of 'cmdesd' ESD
	 */
	sprintf(cmdesd->pb, "ckermit -l %d -b %u -p %c%s%s -s ",
		shm->Liofd, shm->Lbitrate,
		(shm->Lparity) ? shm->Lparity : 'n',
		(strchr(switches, 'a')) ? "" : " -i",
		(1 /* overwrite */ )? "" : " -w");

	if (strchr(switches, 'b'))
		strcat(cmdesd->pb, "-a ");
	else
		strcat(cmdesd->pb, "-b ");

	if (strchr(switches, 'n'))
		strcat(cmdesd->pb, "-n ");	/* overrides -y choice earlier */
	if (strchr(switches, 'f'))
		strcat(cmdesd->pb, "-f ");

	/*
	 * update cmdesd esd
	 */
	cmdesd->cb = strlen(cmdesd->pb);

	if (!(pathesd = esdalloc(PATHESD_INITIAL_SIZE)))
	{
		erc = eNoMemory;
		goto FREE_MEM_AND_EXIT;
	}

	/*
	 * get list of pathnames to send
	 */
	if (erc = gstr(param, pathesd, 1))
		goto FREE_MEM_AND_EXIT;

	/*
	 * append filelist to command
	 */
	if (erc = esdcat(cmdesd, pathesd, 1))
		goto FREE_MEM_AND_EXIT;

	/*
	 * perform the operation
	 */
	last_child_wait_status = 0;
	if (exec_cmd(cmdesd->pb))
		erc = eFATAL_ALREADY;

  FREE_MEM_AND_EXIT:
	if (pathesd)
		esdfree(pathesd);
	if (cmdesd)
		esdfree(cmdesd);

	file_xfer_done_bell();
	lreset_ksr();

	return (_adjust_erc_and_iv0(erc));

}							 /* end of pcmd_sk */

/*+-------------------------------------------------------------------------
	pcmd_rx(param)

rx [-b] <str>
--------------------------------------------------------------------------*/
int
pcmd_rx(param)
ESD *param;
{
	int erc = 0;
	char cmdstr[CMDSTR_SIZE];
	char switches[8];
	ESD *pathesd = esdalloc(PATHESD_INITIAL_SIZE);

	if (!pathesd)
		return (eNoMemory);

	get_switches(param, switches, sizeof(switches));

	if (erc = gstr(param, pathesd, 1))
	{
		esdfree(pathesd);
		return (erc);
	}

	if (erc = _make_bottom_label(param, !strchr(switches, 'l'), 0))
		return (erc);

	/*
	 * build command in 'cmdstr' string
	 */
	sprintf(cmdstr, "%s/ecurz -X -. %d ", eculibdir, shm->Liofd);
	strcat(cmdstr, bottom_label);
	if (!strchr(switches, 'b'))
		strcat(cmdstr, "-b ");
	else
		strcat(cmdstr, "-a ");
	strcat(cmdstr, pathesd->pb);
	esdfree(pathesd);
	file_xfer_start();

	last_child_wait_status = 0;

	if (exec_cmd(cmdstr))
		erc = eFATAL_ALREADY;
	else
		erc = _adjust_erc_and_iv0(erc);

	file_xfer_done_bell();
	lreset_ksr();
	return (erc);

}							 /* end of pcmd_rx */

/*+-------------------------------------------------------------------------
	pcmd_ry(param)

ry
--------------------------------------------------------------------------*/
int
pcmd_ry(param)
ESD *param;
{
	int erc = 0;
	char cmdstr[CMDSTR_SIZE];
	char switches[8];

	get_switches(param, switches, sizeof(switches));

	last_child_wait_status = 0;

	if (erc = _make_bottom_label(param, !strchr(switches, 'l'), 0))
		return (erc);

	/*
	 * build command in 'cmdstr' string
	 */
	sprintf(cmdstr, "%s/ecurz -Y -. %d ", eculibdir, shm->Liofd);
	strcat(cmdstr, bottom_label);
	file_xfer_start();
	if (exec_cmd(cmdstr))
		erc = eFATAL_ALREADY;
	else
		erc = _adjust_erc_and_iv0(erc);

	file_xfer_done_bell();
	lreset_ksr();

	return (erc);
}							 /* end of pcmd_ry */

/*+-------------------------------------------------------------------------
	pcmd_rz(param)
--------------------------------------------------------------------------*/
int
pcmd_rz(param)
ESD *param;
{
	int erc = 0;
	char switches[8];
	char *argv[16];
	int argc = 0;
	ESD *label = (ESD *) 0;

	get_switches(param, switches, sizeof(switches));

	last_child_wait_status = 0;

	if (strchr(switches, 'l'))
	{
		if ((label = esdalloc(64)) == (ESD *) 0)
			return (eNoMemory);
		if (erc = gstr(param, label, 0))
			goto FUNC_EXIT;
		esdprefix(label, "\"");
		esdstrcat(label, "\"");
	}

	/*
	 * call xfer program
	 */
	argv[argc++] = "rz";
	if (strchr(switches, 'N'))	/* force "no curses" */
		argv[argc++] = "-_";
	if (strchr(switches, 'c'))	/* ZCAN upon receive ZEOF */
		argv[argc++] = "-:";
	if (strchr(switches, 'p'))	/* log packets */
		argv[argc++] = "-,"; /* protocol_log_packets handled by
							  * receive_files_from_remote */
	if (label)				 /* bottom label */
	{
		argv[argc++] = "-C";
		argv[argc++] = label->pb;
	}
	if (receive_files_from_remote(argc, argv) < 0)
		erc = eFATAL_ALREADY;
	else
		erc = _adjust_erc_and_iv0(erc);

  FUNC_EXIT:
	esdfree(label);
	return (erc);
}							 /* end of pcmd_rz */

/*+-------------------------------------------------------------------------
	pcmd_rk(param)

rk [-a]
--------------------------------------------------------------------------*/
int
pcmd_rk(param)
ESD *param;
{
	int erc = 0;
	char cmdstr[CMDSTR_SIZE];
	char switches[8];

	get_switches(param, switches, sizeof(switches));

	last_child_wait_status = 0;
	sprintf(cmdstr, "ckermit -r -e 512 -l %d -b %d -p %c",
		shm->Liofd, shm->Lbitrate, (shm->Lparity) ? shm->Lparity : 'n');
	if (strchr(switches, 'a'))
		strcat(cmdstr, "-i ");
	file_xfer_start();

	if (exec_cmd(cmdstr))
		erc = eFATAL_ALREADY;
	else
		erc = _adjust_erc_and_iv0(erc);

	file_xfer_done_bell();
	lreset_ksr();

	return (erc);
}							 /* end of pcmd_rk */

/* vi: set tabstop=4 shiftwidth=4: */
/* end of pcmdxfer.c */
