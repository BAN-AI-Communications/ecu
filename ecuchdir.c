/*+-------------------------------------------------------------------------
	ecuchdir.c - ECU change directory command/history
	wht@wht.net

  Defined functions:
	cd_array_delete(arg, narg)
	cd_array_delete_usage()
	cd_array_init()
	cd_array_read()
	cd_array_save()
	change_directory(dirname, arg_present_flag)
	expand_dirname(dirname, maxlen)
	pop_directory(arg, arg_present, pcmd)
	push_directory(dirname, arg_present, pcmd)

  Paraphrase: "It just goes to show you: no matter where you find
  yourself -- there you are!" -- Pogo

--------------------------------------------------------------------------*/
/*+:EDITS:*/
/*:04-26-2000-11:15-wht@bob-RELEASE 4.42 */
/*:01-24-1997-02:37-wht@yuriatin-SOURCE RELEASE 4.00 */
/*:09-11-1996-20:00-wht@yuriatin-3.48-major telnet,curses,structural overhaul */
/*:08-20-1996-12:39-wht@kepler-locale/ctype fixes from ache@nagual.ru */
/*:12-06-1995-13:30-wht@n4hgf-termecu w/errno -1 consideration */
/*:11-23-1995-11:20-wht@kepler-source control 3.37 for tsx-11 */
/*:11-14-1995-10:23-wht@kepler-3.37.80-source control point: SOCKETS */
/*:03-12-1995-01:03-wht@kepler-use ECU_MAXPN */
/*:01-12-1995-15:19-wht@n4hgf-apply Andrew Chernov 8-bit clean+FreeBSD patch */
/*:05-04-1994-04:38-wht@n4hgf-ECU release 3.30 */
/*:09-10-1992-13:58-wht@n4hgf-ECU release 3.20 */
/*:08-22-1992-15:38-wht@n4hgf-ECU release 3.20 BETA */
/*:09-25-1991-18:24-wht@n4hgf2-fix seg viol in popd w/o argument on Sun */
/*:07-25-1991-12:55-wht@n4hgf-ECU release 3.10 */
/*:07-14-1991-18:18-wht@n4hgf-new ttygets functions */
/*:05-21-1991-18:53-wht@n4hgf-add push_directory and pop_directory */
/*:08-14-1990-20:40-wht@n4hgf-ecu3.00-flush old edit history */

#include "ecu.h"
#include "ecukey.h"
#include "ecutty.h"
#include "termecu.h"

int push_directory();

#define CD_QUAN		44
char *cd_array[CD_QUAN];
int cd_in_use = 0;

#define DIR_PUSH_MAX	10
char *dir_push_stack[DIR_PUSH_MAX];
int dir_push_level = 0;

extern char errmsg[];
extern int errno;

/*+-------------------------------------------------------------------------
	cd_array_read() - read ~/.ecu/dir
--------------------------------------------------------------------------*/
void
cd_array_read()
{
	char dirpath[ECU_MAXPN];
	FILE *fpcd;
	FILE *fopen();
	char *cp;
	char *skip_ld_break();

	get_home_dir(dirpath);
	strcat(dirpath, "/.ecu/dir");
	if ((fpcd = fopen(dirpath, "r")) == (FILE *) 0)
		return;				 /* none found */

	for (cd_in_use = 0; cd_in_use < CD_QUAN; cd_in_use++)
	{
		if (fgets(dirpath, sizeof(dirpath), fpcd) == (char *)0)
			break;
		dirpath[strlen(dirpath) - 1] = 0;
		cp = skip_ld_break(dirpath);
		if (strlen(cp) == 0)
		{
			--cd_in_use;
			continue;
		}
		strcpy(cd_array[cd_in_use], cp);
	}
	fclose(fpcd);
}							 /* end of cd_array_read */

/*+-------------------------------------------------------------------------
	cd_array_save() - save ~/.ecu/dir
--------------------------------------------------------------------------*/
void
cd_array_save()
{
	int icd;
	char savepath[256];
	FILE *fpcd;
	FILE *fopen();

	get_home_dir(savepath);
	strcat(savepath, "/.ecu/dir");

	if (cd_in_use == 0)
	{
		ff(se, "No directory list to save in %s\r\n", savepath);
		return;
	}
	if ((fpcd = fopen(savepath, "w")) == (FILE *) 0)
	{
		ff(se, "%s could not be opened\r\n", savepath);
		return;
	}

	for (icd = 0; icd < cd_in_use; icd++)
		fprintf(fpcd, "%s\n", cd_array[icd]);
	fclose(fpcd);
	ff(se, "%d entries saved in %s\r\n", cd_in_use, savepath);

}							 /* end of cd_array_save */

/*+-------------------------------------------------------------------------
	cd_array_delete_usage()
--------------------------------------------------------------------------*/
void
cd_array_delete_usage()
{
	ff(se, "usage: d[elete] <1st> [<last>]\r\n");
}							 /* end of cd_array_delete_usage */

/*+-------------------------------------------------------------------------
	cd_array_delete(arg,narg)
--------------------------------------------------------------------------*/
int
cd_array_delete(arg, narg)
char **arg;
int narg;
{
	int first;				 /* 1st to delete */
	int last;				 /* last to delete */

	if ((narg < 2) || (narg > 3))
	{
		cd_array_delete_usage();
		return (-1);
	}

	first = atoi(arg[1]) - 1;
	if (narg == 2)
		last = first;
	else
		last = atoi(arg[2]) - 1;

	if ((first > (cd_in_use - 1)) || (last > (cd_in_use - 1)) || (last < first))
	{
		cd_array_delete_usage();
		return (-1);
	}

	if (last == (cd_in_use - 1))
	{
		cd_in_use = first;
	}
	else
	{
		int count_less = last - first + 1;

		last++;
		while (last != cd_in_use)
			strcpy(cd_array[first++], cd_array[last++]);
		cd_in_use -= count_less;
	}
	return (0);
}							 /* end of cd_array_delete */

/*+-------------------------------------------------------------------------
	cd_array_init()
--------------------------------------------------------------------------*/
void
cd_array_init()
{
	int itmp;

/*allocate change_directory stack */
	for (itmp = 0; itmp < CD_QUAN; itmp++)
	{
		if (!(cd_array[itmp] = malloc(ECU_MAXPN + 1)))
		{
			ff(se, "Not enough memory for cd stack\r\n");
			errno = -1;
			termecu(TERMECU_MALLOC);
		}
		*cd_array[itmp] = 0;
	}
	(void)cd_array_read();
}							 /* end of cd_array_init */

/*+-------------------------------------------------------------------------
	expand_dirname(dirname,maxlen) - convert dirname with shell chars
--------------------------------------------------------------------------*/
int
expand_dirname(dirname, maxlen)
char *dirname;
int maxlen;
{
	char s256[256];
	char *expcmd;

	if (!find_shell_chars(dirname))
		return (0);

	sprintf(s256, "`ls -d %s`", dirname);
	if (expand_wildcard_list(s256, &expcmd))
	{
		strcpy(errmsg, "No such directory");
		return (-1);
	}
	strncpy(dirname, expcmd, maxlen);
	dirname[maxlen - 1] = 0;
	free(expcmd);
	if (strchr(dirname, ' '))
	{
		strcpy(errmsg, "Too many files");
		return (-1);
	}
	return (0);

}							 /* end of expand_dirname */

/*+-------------------------------------------------------------------------
	change_directory(dirname,arg_present_flag) - 'cd' interactive cmd hdlr

  Change directory to 'dirname' if arg_present_flag is true,
  else if flag 0, ask for new directory name and change to it
  This procedure maintains the global variable 'curr_dir' that
  reflects the ecu transmitter process current working directory.
--------------------------------------------------------------------------*/
int
change_directory(dirname, arg_present_flag)
char *dirname;
int arg_present_flag;
{
	int icd;
	int itmp;
	char s256[256];
	UINT delim;

#define BLD_ARG_MAX	5
	char *arg[BLD_ARG_MAX];
	int narg;
	int longest;
	int pushing = 0;

	if (cd_in_use == 0)
		cd_array_read();

	fputs("  ", se);

	if (arg_present_flag)	 /* if there is an argument ... */
	{
		if (isdigit((uchar) * dirname))	/* ... and first char is digit */
		{
			icd = atoi(dirname) - 1;
			if ((icd < 0) || (icd >= cd_in_use))
				goto DISPLAY_CD_ARRAY;
			strncpy(s256, cd_array[icd], sizeof(s256));
		}
		else
			strncpy(s256, dirname, sizeof(s256));	/* literal dir spec */

		s256[sizeof(s256) - 1] = 0;
	}
	else
		/* no arg to cd command */
	{
	  DISPLAY_CD_ARRAY:
		fputs("\r\n", se);
		longest = 0;
		for (icd = 0; icd < CD_QUAN / 2; icd++)
		{
			if (icd >= cd_in_use)
				break;
			if (longest < (itmp = strlen(cd_array[icd])))
				longest = itmp;
		}
		longest += 4;
		if (longest < 36)
			longest += 4;
		for (icd = 0; icd < CD_QUAN / 2; icd++)
		{
			if (icd >= cd_in_use)
				break;
			sprintf(s256, "%2d %s ", icd + 1, cd_array[icd]);
			fputs(s256, se);
			if (icd + CD_QUAN / 2 >= cd_in_use)
				fputs("\r\n", se);
			else
			{
				itmp = longest - strlen(s256);
				while (itmp-- > 0)
					fputc(' ', se);
				sprintf(s256, "%2d %s\r\n",
					icd + 1 + CD_QUAN / 2, cd_array[icd + CD_QUAN / 2]);
				fputs(s256, se);

			}
		}
		fputs("current dir: ", se);
		tcap_stand_out();
		ff(se, " %s ", curr_dir);
		tcap_stand_end();
		tcap_eeol();
		fputs("\r\n", se);

	  GET_NEW_DIR:
		fputs(
			"New dir, <#>, %save, %read, %del, %xmitcd, %pushd, <enter>:  ", se);
		ttygets(s256, sizeof(s256), TG_CRLF, &delim, (int *)0);

	  TRY_AGAIN:
		if ((delim == ESC) || !strlen(s256))
		{
			ff(se, "no directory change\r\n");
			return (0);
		}
		else if (s256[0] == '%')
		{
			if (pushing)
			{
				ff(se, "syntax error\r\n");
				return (-1);
			}
			build_str_array(s256, arg, BLD_ARG_MAX, &narg);

			if (minunique("save", &s256[1], 1))
			{
				cd_array_save();
				goto GET_NEW_DIR;
			}
			else if (minunique("read", &s256[1], 1))
			{
				cd_array_read();
				goto DISPLAY_CD_ARRAY;
			}
			else if (minunique("delete", &s256[1], 1))
			{
				if (cd_array_delete(arg, narg))
					goto GET_NEW_DIR;
				else
					goto DISPLAY_CD_ARRAY;
			}
			else if (minunique("xmitcd", &s256[1], 1))
			{
				lputs("cd ");
				lputs(curr_dir);
				lputc('\r');
				return (0);
			}
			else if (minunique("pushd", &s256[1], 1))
			{
				strcpy(s256, arg[1]);
				pushing = 1;
				goto TRY_AGAIN;
			}
			else
			{
				ff(se, "Invalid cd subcommand\r\n");
				goto GET_NEW_DIR;
			}
		}
		else if (icd = atoi(s256))
		{
			icd--;
			if (icd >= cd_in_use)
				goto GET_NEW_DIR;
			strncpy(s256, cd_array[icd], sizeof(s256));
			s256[sizeof(s256) - 1] = 0;
		}
	}
	if (pushing)
	{
		if (push_directory(s256, 1, 1))
			return (-1);
	}
	else
	{
		if (expand_dirname(s256, sizeof(s256)))
		{
			ff(se, "%s\r\n", errmsg);
			return (-1);
		}
		if (chdir(s256) < 0) /* now change to the new directory */
		{
			perror(s256);	 /* print error if we get one */
			ff(se, "\r\n");
			return (-1);
		}
		get_curr_dir(curr_dir, sizeof(curr_dir));
		fputs("confirmed: ", se);
		tcap_stand_out();
		ff(se, " %s ", curr_dir);
		tcap_stand_end();
		fputs("\r\n", se);
	}

	for (icd = 0; icd < cd_in_use; icd++)
	{
		if (strcmp(curr_dir, cd_array[icd]) == 0)
			return (0);
	}
	if (cd_in_use == CD_QUAN)
	{
		for (icd = 1; icd < CD_QUAN; icd++)
		{
			strcpy(cd_array[icd - 1], cd_array[icd]);
		}
		strcpy(cd_array[CD_QUAN - 1], curr_dir);
	}
	else
		strcpy(cd_array[cd_in_use++], curr_dir);

	return (0);

}							 /* end of change_directory */

/*+-------------------------------------------------------------------------
	push_directory(dirname,arg_present,pcmd) - 'pushd' function

This function performs 'pushd' actions for both the interactive
and procedure 'pushd' commands

dirname is new directory name if arg_present true
pcmd true if procedure command or cd %p interactive, else interactive command

returns -1 if error, else 0
--------------------------------------------------------------------------*/
int
push_directory(dirname, arg_present, pcmd)
char *dirname;
int arg_present;
int pcmd;
{
	int itmp;
	char s256[256];

	if (!pcmd)
		ff(se, "\r\n");

	if (!arg_present)
	{
		if (!dir_push_level)
		{
			if (!pcmd || proc_trace)
				pprintf("---: no directories pushed\n");
		}
		else
		{
			for (itmp = 0; itmp < dir_push_level; itmp++)
				pprintf("%3d: %s\n", itmp, dir_push_stack[itmp]);
		}
		pprintf("cwd: %s\n", curr_dir);
		return (0);
	}

	if (dir_push_level == DIR_PUSH_MAX)
	{
		pputs("too many pushds\n");
		return (-1);
	}

	if (!dir_push_stack[dir_push_level])
	{
		if (!(dir_push_stack[dir_push_level] = malloc(ECU_MAXPN)))
		{
			pputs("no memory for pushd\n");
			return (-1);
		}
	}

	get_curr_dir(dir_push_stack[dir_push_level], ECU_MAXPN);

	strncpy(s256, dirname, sizeof(s256));
	s256[sizeof(s256) - 1] = 0;
	if (expand_dirname(s256, sizeof(s256)))
	{
		pprintf("'%s': %s\n", s256, errmsg);
		return (-1);
	}
	if (chdir(s256) < 0)	 /* now change to the new directory */
	{
		pperror(s256);		 /* print error if we get one */
		return (-1);
	}
	get_curr_dir(curr_dir, sizeof(curr_dir));

	if (pcmd && proc_trace)
		pprintf("pushed to directory %s\n", curr_dir);
	else
	{
		fputs("confirmed: ", se);
		tcap_stand_out();
		ff(se, " %s ", curr_dir);
		tcap_stand_end();
		ff(se, " (level %d)\r\n", dir_push_level);
	}

	dir_push_level++;
	return (0);

}							 /* end of push_directory */

/*+-------------------------------------------------------------------------
	pop_directory(arg,arg_present,pcmd) - 'popd' function

This function performs 'popd' actions for both the interactive
and procedure 'popd' commands

arg is empty if arg_present false
if not empty, it is == 'all' or a numeric level to pop to
pcmd true if procedure command, else interactive command

returns -1 if error, else 0
--------------------------------------------------------------------------*/
int
pop_directory(arg, arg_present, pcmd)
char *arg;
int arg_present;
int pcmd;
{
	int new_level = -1;

	if (!pcmd)
		pputs("\n");

	if (!dir_push_level)
	{
		if (!pcmd || proc_trace)
		{
			pprintf("---: no directories pushed\n");
			pprintf("cwd: %s\n", curr_dir);
		}
		return (-1);
	}

	if (!arg_present)
		new_level = dir_push_level - 1;
	else if (minunique("all", arg, 1))
		new_level = 0;
	else if (isdigit((uchar) * arg))
		new_level = atoi(arg);
	else
	{
		pprintf("argument error: '%s'\n", arg);
		return (-1);
	}

	if ((new_level < 0) || (new_level > (dir_push_level - 1)))
	{
		pprintf("invalid popd argument (or not pushed to level): '%s'\n", arg);
		return (-1);
	}

	dir_push_level = new_level;
	if (chdir(dir_push_stack[dir_push_level]) < 0)
	{
		pperror(dir_push_stack[dir_push_level]);
		return (-1);
	}
	get_curr_dir(curr_dir, sizeof(curr_dir));

	if (pcmd && proc_trace)
		pprintf("popped to directory %s (level %d)\n", curr_dir, dir_push_level);
	else
	{
		fputs("confirmed: ", se);
		tcap_stand_out();
		ff(se, " %s ", curr_dir);
		tcap_stand_end();
		ff(se, " (level %d)\r\n", dir_push_level);
	}

	return (0);

}							 /* end of pop_directory */

/* end of ecuchdir.c */
/* vi: set tabstop=4 shiftwidth=4: */
