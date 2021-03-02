/*+-------------------------------------------------------------------------
	pcmdtty.c - tty (console) related procedure commands
	wht@wht.net

  Defined functions:
	ifunc_colors(pvalue)
	pcmd_cls(param)
	pcmd_color(param)
	pcmd_conxon(param)
	pcmd_cursor(param)
	pcmd_delline(param)
	pcmd_eeod(param)
	pcmd_eeol(param)
	pcmd_fkey(param)
	pcmd_fkmap(param)
	pcmd_home(param)
	pcmd_icolor(param)
	pcmd_insline(param)
	pcmd_scrdump(param)
	pcmd_vidcolor(param)
	pcmd_vidnorm(param)
	pcmd_vidrev(param)

--------------------------------------------------------------------------*/
/*+:EDITS:*/
/*:04-26-2000-11:16-wht@bob-RELEASE 4.42 */
/*:01-24-1997-02:38-wht@yuriatin-SOURCE RELEASE 4.00 */
/*:09-11-1996-20:00-wht@yuriatin-3.48-major telnet,curses,structural overhaul */
/*:11-23-1995-11:20-wht@kepler-source control 3.37 for tsx-11 */
/*:11-14-1995-10:23-wht@kepler-3.37.80-source control point: SOCKETS */
/*:05-04-1994-04:39-wht@n4hgf-ECU release 3.30 */
/*:12-24-1992-13:58-wht@n4hgf-add eeod */
/*:10-18-1992-14:26-wht@n4hgf-add conxon */
/*:09-10-1992-14:00-wht@n4hgf-ECU release 3.20 */
/*:08-30-1992-23:15-wht@n4hgf-add fkmap */
/*:08-22-1992-15:39-wht@n4hgf-ECU release 3.20 BETA */
/*:07-25-1991-12:59-wht@n4hgf-ECU release 3.10 */
/*:05-21-1991-00:45-wht@n4hgf-added -3 error code to keyset_read */
/*:01-23-1991-01:58-wht@n4hgf-illegal color name make hi_white on hi_white */
/*:08-14-1990-20:40-wht@n4hgf-ecu3.00-flush old edit history */

#include "ecu.h"
#include "ecuerror.h"
#include "esd.h"
#include "ecutty.h"
#include "esd.h"
#include "procedure.h"

extern UINT32 colors_current;

/*+-------------------------------------------------------------------------
	pcmd_color(param)

Usage:   color [-r] [argument] [argument]
Options:
   color color      Set foreground and background normal video colors
   -r color color   Set foreground & background reverse video colors

Color names
   blue      magenta      brown      black
   lt_blue   lt_magenta   yellow     gray
   cyan      white        green      red
   lt_cyan   hi_white     lt_green   lt_red

--------------------------------------------------------------------------*/
pcmd_color(param)
ESD *param;
{
	int erc;
	char switches[8];
	int normal;
	char s32[32];
	UINT32 foreground;
	UINT32 background;

	get_switches(param, switches, sizeof(switches));
	if (!strlen(switches))
		normal = 1;
	else if (switches[1] == 'r')
		normal = 0;			 /* reverse */
	else
	{
		pputs("unrecognized switch\n");
		return (eFATAL_ALREADY);
	}

	if ((erc = get_alpha_zstr(param, s32, sizeof(s32))) ||
		((int)(foreground = color_name_to_num(s32)) < 0))
		goto ERROR;

	if (erc = get_alpha_zstr(param, s32, sizeof(s32)))
	{
		if (!end_of_cmd(param))
			goto ERROR;
		background = 0;
	}
	else if ((int)(background = color_name_to_num(s32)) < 0)
		goto ERROR;

	if (normal)
	{
		colors_current &= 0xFFFF0000;
		colors_current |= (foreground << 8) | background;
		if (proc_trace > 1)
		{
			pprintf("normal %ld,%ld current=0x%08lx\n",
				foreground, background, colors_current);
		}
	}
	else
	{
		colors_current &= 0x0000FFFF;
		colors_current |= (foreground << 24) | (background << 16);
		if (proc_trace > 1)
		{
			pprintf("reverse %ld,%ld current=0x%08lx\n",
				foreground, background, colors_current);
		}
	}

	setcolor(colors_current);
	return (0);

  ERROR:
	if (erc)
		return (erc);
	pputs("invalid color\n");
	return (eFATAL_ALREADY);

}							 /* end of pcmd_color */

/*+-------------------------------------------------------------------------
	ifunc_colors(pvalue)
--------------------------------------------------------------------------*/
int
ifunc_colors(pvalue)
UINT32 *pvalue;
{
	*pvalue = colors_current;
	return (0);
}							 /* end of ifunc_colors */

/*+-------------------------------------------------------------------------
	pcmd_icolor(param)
--------------------------------------------------------------------------*/
pcmd_icolor(param)
ESD *param;
{
	int erc;
	UINT32 new_colors;

	if (erc = gint(param, &new_colors))
		return (erc);

	setcolor(new_colors);
	return (0);
}							 /* end of pcmd_icolor */

/*+-------------------------------------------------------------------------
	pcmd_cls(param)
--------------------------------------------------------------------------*/
/*ARGSUNUSED*/
int
pcmd_cls(param)
ESD *param;
{
	param = 0; /* unused */
	tcap_clear_screen();
	return (0);
}							 /* end of pcmd_cls */

/*+-------------------------------------------------------------------------
	pcmd_cursor(param)
--------------------------------------------------------------------------*/
int
pcmd_cursor(param)
ESD *param;
{
	int erc;
	long row;
	long col = 0;

	if (erc = gint(param, &row))
		return (erc);
	if (gint(param, &col))
	{
		/* if something there non-integer */
		if (!end_of_cmd(param))
			return (eSyntaxError);
	}
	tcap_cursor((int)row, (int)col);
	return (0);
}							 /* end of pcmd_cursor */

/*+-------------------------------------------------------------------------
	pcmd_scrdump(param)
--------------------------------------------------------------------------*/
int
pcmd_scrdump(param)
ESD *param;
{
	int erc;
	ESD *fname;
	FILE *fp;

	if (!(fname = esdalloc(ESD_NOMSZ)))
		return (eNoMemory);

	if (erc = gstr(param, fname, 1))
	{
		if (!end_of_cmd(param))
		{
			erc = eSyntaxError;
			goto FUNC_RETURN;
		}
	}

	if (fname->cb)
	{
		if (!(fp = fopen(fname->pb, "a")))
		{
			pperror(fname->pb);
			erc = eFATAL_ALREADY;
			goto FUNC_RETURN;
		}
		fclose(fp);
	}

	screen_dump((fname->cb) ? fname->pb : (char *)0);

  FUNC_RETURN:
	esdfree(fname);
	return (erc);
}							 /* end of pcmd_scrdump */

/*+-------------------------------------------------------------------------
	pcmd_vidnorm(param)
--------------------------------------------------------------------------*/
/*ARGSUNUSED*/
int
pcmd_vidnorm(param)
ESD *param;
{
	param = 0; /* unused */
	tcap_stand_end();
	return (0);
}							 /* end of pcmd_vidnorm */

/*+-------------------------------------------------------------------------
	pcmd_vidrev(param)
--------------------------------------------------------------------------*/
/*ARGSUNUSED*/
int
pcmd_vidrev(param)
ESD *param;
{
	param = 0; /* unused */
	tcap_stand_out();
	return (0);
}							 /* end of pcmd_vidrev */

/*+-------------------------------------------------------------------------
	pcmd_fkey(param)
--------------------------------------------------------------------------*/
pcmd_fkey(param)
ESD *param;
{
	int erc;
	ESD *tesd;

	if ((tesd = esdalloc(64)) == (ESD *) 0)
		return (eNoMemory);

	if (erc = gstr(param, tesd, 0))
		goto FUNC_RETURN;

	switch (keyset_read(tesd->pb))
	{
		case 0:
			if (proc_trace)
				keyset_display();
			break;
		case -1:
			pprintf("cannot find ~/.ecu/keys\n");
			erc = eFATAL_ALREADY;
			break;
		case -2:
			pprintf("'%s' not found in ~/.ecu/keys\n", tesd->pb);
			erc = eFATAL_ALREADY;
			break;
		case -3:
			pprintf("'%s' has a syntax error\n", tesd->pb);
			erc = eFATAL_ALREADY;
			break;
	}

  FUNC_RETURN:
	esdfree(tesd);
	return (erc);
}							 /* end of pcmd_fkey */

/*+-------------------------------------------------------------------------
	pcmd_fkmap(param)
--------------------------------------------------------------------------*/
pcmd_fkmap(param)
ESD *param;
{
	int erc = 0;
	int in_quotes = 0;
	char *cmd;
	char *cp;
	char ch;

#define FKMAP_MAXARGS 36
	char *arg[FKMAP_MAXARGS];
	int narg;

	skip_cmd_break(param);

	if (!(cmd = strdup(param->pb + param->index)))
		return (eNoMemory);

	cp = cmd;
	while (ch = *cp++)
	{
		if (ch == '\'')
		{
			in_quotes = !in_quotes;
			continue;
		}
		else if (ch == '\\')
		{
			cp++;
			continue;
		}
		else if (!in_quotes && strchr("#;", ch))
			break;
	}
	param->index += (int)(cp - cmd);
	*cp = 0;

	arg[0] = "fkmap";
	build_arg_array(cmd, arg + 1, FKMAP_MAXARGS - 1, &narg);
	narg++;
	erc = fkmap_command(narg, arg);
	free(cmd);
	return (erc);

}							 /* end of pcmd_fkmap */

/*+-------------------------------------------------------------------------
	pcmd_vidcolor(param)

vidcolor normal|reverse|notify|success|alert|error fcolor [bcolor]
--------------------------------------------------------------------------*/
int
pcmd_vidcolor(param)
ESD *param;
{
	int erc = eNoParameter;
	int ntokens = 0;
	char *tokens[3];
	int param_index[3];
	char tokenbuf[64];

	tokens[0] = tokenbuf;
	tokens[1] = tokenbuf + 20;
	tokens[2] = tokenbuf + 40;

	while (ntokens < 3)
	{
		skip_cmd_break(param);
		param_index[ntokens] = param->index;
		if (erc = get_word_zstr(param, tokens[ntokens], 20))
			break;
		ntokens++;
	}

	if (erc && ((erc != eNoParameter) || (ntokens < 2)))
		return (erc);

	switch (erc = setcolor_internal(ntokens, tokens))
	{
		case 0:
			break;
		default:
			param->old_index = param->index = param_index[erc - 1];
			erc = eBadParameter;
	}
	return (erc);
}							 /* end of pcmd_vidcolor */

/*+-------------------------------------------------------------------------
	pcmd_home(param) - home the cursor
--------------------------------------------------------------------------*/
/*ARGSUNUSED*/
int
pcmd_home(param)
ESD *param;
{
	param = 0; /* unused */
	tcap_cursor(0, 0);
	return (0);
}							 /* end of pcmd_home */

/*+-------------------------------------------------------------------------
	pcmd_eeod(param) - erase to end of display
--------------------------------------------------------------------------*/
/*ARGSUNUSED*/
int
pcmd_eeod(param)
ESD *param;
{
	param = 0; /* unused */
	tcap_eeod();
	return (0);
}							 /* end of pcmd_eeod */

/*+-------------------------------------------------------------------------
	pcmd_eeol(param) - erase to end of line
--------------------------------------------------------------------------*/
/*ARGSUNUSED*/
int
pcmd_eeol(param)
ESD *param;
{
	param = 0; /* unused */
	tcap_eeol();
	return (0);
}							 /* end of pcmd_eeol */

/*+-------------------------------------------------------------------------
	pcmd_insline(param) - insert line in display
--------------------------------------------------------------------------*/
/*ARGSUNUSED*/
int
pcmd_insline(param)
ESD *param;
{
	param = 0; /* unused */
	tcap_insert_lines(1);
	return (0);
}							 /* end of pcmd_insline */

/*+-------------------------------------------------------------------------
	pcmd_delline(param) - delete line from display
--------------------------------------------------------------------------*/
/*ARGSUNUSED*/
int
pcmd_delline(param)
ESD *param;
{
	param = 0; /* unused */
	tcap_delete_lines(1);
	return (0);
}							 /* end of pcmd_delline */

/*+-------------------------------------------------------------------------
	pcmd_conxon(param)
--------------------------------------------------------------------------*/
int
pcmd_conxon(param)
ESD *param;
{
	int erc;
	char new_xonxoff[8];
	char *conxon_status();

	if (shm->Liofd < 0)
		return (eNoLineAttached);

	if (erc = get_alpha_zstr(param, new_xonxoff, sizeof(new_xonxoff)))
		return (erc);

	if (set_console_xon_xoff_by_arg(new_xonxoff))
		return (eBadParameter);

	if (proc_trace)
	{
		pprintf("console xon/xoff flow control set to %s\n",
			console_xon_status());
	}

	return (erc);

}							 /* end of pcmd_conxon */

/* vi: set tabstop=4 shiftwidth=4: */
/* end of pcmdtty.c */
