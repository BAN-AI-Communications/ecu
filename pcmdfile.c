/*+-------------------------------------------------------------------------
	pcmdfile.c - ecu file-related procedure commands
	wht@wht.net

  Defined functions:
	_file_not_open(filenum)
	_param_to_stat(param, pstat_rtnd)
	get_filenum(param, filenum)
	ifunc_fatime(param, pvalue)
	ifunc_fmode(param, pvalue)
	ifunc_fmtime(param, pvalue)
	ifunc_fsize(param, pvalue)
	ifunc_ftell(param, pvalue)
	ifunc_ischr(param, pvalue)
	ifunc_isdir(param, pvalue)
	ifunc_isreg(param, pvalue)
	pcmd_fchmod(param)
	pcmd_fclose(param)
	pcmd_fdel(param)
	pcmd_fgetc(param)
	pcmd_fgets(param)
	pcmd_fopen(param)
	pcmd_fputc(param)
	pcmd_fputs(param)
	pcmd_fread(param)
	pcmd_fflush(param)
	pcmd_fwrite(param)
	pcmd_mkdir(param)
	pcmd_pclose(param)
	pcmd_popen(param)
	proc_file_reset()
	str_to_filemode(modestr, filemode)

--------------------------------------------------------------------------*/
/*+:EDITS:*/
/*:04-26-2000-11:16-wht@bob-RELEASE 4.42 */
/*:03-31-1998-17:38-wht@kepler-add fdopen_socket */
/*:01-25-1997-13:31-wht@yuriatin-add fflush */
/*:01-24-1997-02:38-wht@yuriatin-SOURCE RELEASE 4.00 */
/*:09-11-1996-20:00-wht@yuriatin-3.48-major telnet,curses,structural overhaul */
/*:11-23-1995-11:20-wht@kepler-source control 3.37 for tsx-11 */
/*:11-14-1995-10:23-wht@kepler-3.37.80-source control point: SOCKETS */
/*:05-04-1994-04:39-wht@n4hgf-ECU release 3.30 */
/*:09-10-1992-14:00-wht@n4hgf-ECU release 3.20 */
/*:08-22-1992-15:39-wht@n4hgf-ECU release 3.20 BETA */
/*:07-03-1992-12:48-wht@n4hgf-why not let fchmod set any bits? */
/*:07-25-1991-12:59-wht@n4hgf-ECU release 3.10 */
/*:06-27-1991-13:45-wht@n4hgf-$i0 wasn't always plugged on failures */
/*:08-14-1990-20:40-wht@n4hgf-ecu3.00-flush old edit history */

#include "ecu.h"
#include "ecukey.h"
#include "ecuerror.h"
#include "esd.h"
#include "var.h"
#include "procedure.h"

#if !defined(S_IRUSR)
#define	S_IRUSR	00400		 /* read permission: owner */
#define	S_IWUSR	00200		 /* write permission: owner */
#define	S_IXUSR	00100		 /* execute permission: owner */
#define	S_IRWXG	00070		 /* read, write, execute: group */
#define	S_IRGRP	00040		 /* read permission: group */
#define	S_IWGRP	00020		 /* write permission: group */
#define	S_IXGRP	00010		 /* execute permission: group */
#define	S_IRWXO	00007		 /* read, write, execute: other */
#define	S_IROTH	00004		 /* read permission: other */
#define	S_IWOTH	00002		 /* write permission: other */
#define	S_IXOTH	00001		 /* execute permission: other */
#endif

extern PCB *pcb_stack[];

#define FILE_MAX	10

typedef struct pfile_struct
{
	FILE *f;				 /* file pointer */
	ESD *n;					 /* file name */
	int is_socket;			 /* true if file is fdopen_socket stuff */
	int in_use;
} PFILE;

PFILE pfile[FILE_MAX];

char fwrite_error_fmt[] = "file %d write error (not open for write?)\n";

/*+-------------------------------------------------------------------------
	close_ecu_file(filenum)
--------------------------------------------------------------------------*/
void
close_ecu_file(filenum)
UINT filenum;
{
	if (filenum > FILE_MAX)
		return;
	if (pfile[filenum].f)
	{
		if(pfile[filenum].is_socket)
			close(fileno(pfile[filenum].f));
		fclose(pfile[filenum].f);
		esdfree(pfile[filenum].n);
		memset((char *)&pfile[filenum],0,sizeof(pfile[0]));
	}

}	/* end of close_ecu_file */

/*+-------------------------------------------------------------------------
	proc_file_reset()
--------------------------------------------------------------------------*/
void
proc_file_reset()
{
	int itmp;

	for (itmp = 0; itmp < FILE_MAX; itmp++)
	{
		close_ecu_file(itmp);
	}
}							 /* end of proc_file_reset */

/*+-------------------------------------------------------------------------
	_file_not_open(filenum)
--------------------------------------------------------------------------*/
int
_file_not_open(filenum)
int filenum;
{
	pprintf("file %d not open\n", filenum);
	return (eFATAL_ALREADY);
}							 /* end of _file_not_open */

/*+-------------------------------------------------------------------------
	get_filenum(param,filenum)
--------------------------------------------------------------------------*/
int
get_filenum(param, filenum)
ESD *param;
int *filenum;
{
	int erc;
	UINT32 lvarnum;
	int old_index;

	skip_cmd_break(param);
	old_index = param->old_index;
	if (erc = gint(param, &lvarnum))
		return (erc);
	if (lvarnum > FILE_MAX)
		return (eBadFileNumber);
	*filenum = (int)lvarnum;
	param->old_index = old_index;
	return (0);
}							 /* end of get_filenum */

/*+-------------------------------------------------------------------------
	str_to_filemode(modestr,filemode) - "rwxrwxrwx" to mode integer
--------------------------------------------------------------------------*/
str_to_filemode(modestr, filemode)
char *modestr;
long *filemode;
{
	int i;
	int mode = 0;
	int erc = 0;

	if (strlen(modestr) != 9)
	{
		pprintf("bad length: '%s'\n", modestr);
		return (eFATAL_ALREADY);
	}

	for (i = 0; i < 9; i++)
	{
		switch (modestr[i])
		{

			case 'r':
				if (i == 0)
					mode |= S_IRUSR;
				else if (i == 3)
					mode |= S_IRGRP;
				else if (i == 6)
					mode |= S_IROTH;
				else
					erc = eSyntaxError;
				break;

			case 'w':
				if (i == 1)
					mode |= S_IWUSR;
				else if (i == 4)
					mode |= S_IWGRP;
				else if (i == 7)
					mode |= S_IWOTH;
				else
					erc = eSyntaxError;
				break;

			case 'x':
				if (i == 2)
					mode |= S_IXUSR;
				else if (i == 5)
					mode |= S_IXGRP;
				else if (i == 8)
					mode |= S_IXOTH;
				else
					erc = eSyntaxError;
				break;

			case 's':
				if (i == 2)
				{
					mode |= S_ISUID;
					mode |= S_IXUSR;
				}
				else if (i == 5)
				{
					mode |= S_ISGID;
					mode |= S_IXGRP;
				}
				else if (i == 7)
				{
					mode |= S_ISGID;
					mode |= S_IXGRP;
				}
				else
					erc = eSyntaxError;
				break;

			case 't':
#if defined(FULL_FEATURE_CHMODE)
				if (i == 8)
				{
					mode |= S_ISVTX;
					mode |= S_IXOTH;
				}
				else
					erc = eSyntaxError;
#else
				pputs("set sticky bit not allowed\n");
				erc = eFATAL_ALREADY;
#endif /* defined(FULL_FEATURE_CHMODE) */
				break;
			case 'l':
				if (i == 5)
				{
					mode |= S_ISGID;
					mode &= ~S_IXGRP;
				}
				else
					erc = eSyntaxError;
				break;
			case '-':
				break;
			default:
				erc = eSyntaxError;
		}					 /* end switch */

		if (erc)
			break;

	}						 /* end for */

	if (erc)
	{
		if (erc != eFATAL_ALREADY)
			pputs("invalid mode specifier\n");
		pputs(modestr);
		while (i--)
			pputc(' ');
		pputs("^\n");

	}
	else
		*filemode = (long)mode;

	return (erc);

}							 /* end of str_to_filemode */

/*+-------------------------------------------------------------------------
	pcmd_fgetc(param)

fgetc <filenum-int> [$][i<varspec> | $s<varspec>]
int variable receives 0 if EOF
str var receives null str on eof
--------------------------------------------------------------------------*/
int
pcmd_fgetc(param)
ESD *param;
{
	int erc;
	int filenum;
	int vartype;
	int inchar;
	ESD *svptr;
	long *ivptr;

	if (!proc_level)
		return (eNotExecutingProc);

	if (erc = get_filenum(param, &filenum))
		return (erc);

	if (!pfile[filenum].f)
		return (_file_not_open(filenum));

	skip_cmd_char(param, '$');
	if ((param->index >= param->cb) ||
		(((vartype = to_lower(*(param->pb + param->index))) != 'i') &&
			(vartype != 's')))
		return (eIllegalVarType);
	param->index++;
	switch (vartype)
	{
		case 'i':
			erc = get_ivptr(param, &ivptr, 1);
			break;
		default:
			erc = get_svptr(param, &svptr, 1);
			break;
	}
	if (erc)
		return (erc);

	if ((inchar = fgetc(pfile[filenum].f)) == EOF)
	{
		if (proc_trace)
			pputs("fgetc EOF\n");
		if (vartype == 'i')
			*ivptr = -1;
		else
			esdzero(svptr);
	}
	else if (vartype == 'i')
		*ivptr = inchar;
	else
	{
		*svptr->pb = inchar;
		svptr->cb = 1;
	}

	if (proc_trace)
	{
		pputs("fgetc set ");
		pputs((vartype == 'i') ? "int" : "str");
		pprintf(" var = %lu (0x%02x)\n", inchar, inchar);
	}
	return (0);

}							 /* end of pcmd_fgetc */

/*+-------------------------------------------------------------------------
	pcmd_fread(param)
--------------------------------------------------------------------------*/
/*ARGSUSED*/
int
pcmd_fread(param)
ESD *param;
{
	param = 0; /* unusued */
	return (eNotImplemented);
}							 /* end of pcmd_fread */

/*+-------------------------------------------------------------------------
	pcmd_fgets(param)
fgetc <filenum-int> [$][s]<varspec>
--------------------------------------------------------------------------*/
int
pcmd_fgets(param)
ESD *param;
{
	int erc;
	int filenum;
	char ctmp;
	ESD *svptr;

	if (!proc_level)
		return (eNotExecutingProc);

	if (erc = get_filenum(param, &filenum))
		return (erc);

	if (!pfile[filenum].f)
		return (_file_not_open(filenum));

	skip_cmd_char(param, '$');
	if (erc = get_cmd_char(param, &ctmp))
		return (erc);
	if (to_lower(ctmp) != 's')
		return (eIllegalVarType);
	if (erc = get_svptr(param, &svptr, 1))
		return (erc);
	*svptr->pb = 0;
	svptr->cb = 0;
	if (!(iv[0] = !fgets(svptr->pb, svptr->maxcb + 1, pfile[filenum].f)))
	{
		svptr->cb = strlen(svptr->pb);
		if (*(svptr->pb + svptr->cb - 1) == NL)
		{
			svptr->cb--;
			esd_null_terminate(svptr);
		}
	}
	if (proc_trace)
		pprintf("fgets set str var = '%s'\n", svptr->pb);
	return (0);

}							 /* end of pcmd_fgets */

/*+-------------------------------------------------------------------------
	pcmd_fclose(param)
fclose <filenum-int>
--------------------------------------------------------------------------*/
int
pcmd_fclose(param)
ESD *param;
{
	int erc;
	int filenum;

	if (!proc_level)
		return (eNotExecutingProc);

	if (erc = get_filenum(param, &filenum))
		return (erc);

	close_ecu_file(filenum);
	return (0);

}							 /* end of pcmd_fclose */

/*+-------------------------------------------------------------------------
	pcmd_fputc(param)
fputc <file-num> <int>
fputc <file-num> <str>
--------------------------------------------------------------------------*/
int
pcmd_fputc(param)
ESD *param;
{
	int erc;
	int filenum;
	ESD *buf = (ESD *) 0;
	char outchar = 0;
	long outlong;

	if (!proc_level)
		return (eNotExecutingProc);

	if (erc = get_filenum(param, &filenum))
		return (erc);

	if (!pfile[filenum].f)
		return (_file_not_open(filenum));

	if (!gint(param, &outlong))
		outchar = (char)outlong;
	else
	{
		if (!(buf = esdalloc(ESD_NOMSZ)))
			return (eNoMemory);
		if (erc = gstr(param, buf, 1))
			goto FUNC_RETURN;
		if (!buf->cb)
		{
			pputs("cannot fputc: zero length string\n");
			erc = eFATAL_ALREADY;
			goto FUNC_RETURN;
		}
		outchar = *buf->pb;
	}

	if (fputc(outchar, pfile[filenum].f) < 0)
	{
		pprintf(fwrite_error_fmt, filenum);
		erc = eFATAL_ALREADY;
	}

  FUNC_RETURN:
	if (buf)
		esdfree(buf);
	return (erc);
}							 /* end of pcmd_fputc */

/*+-------------------------------------------------------------------------
	pcmd_fopen(param)

fopen [-<fopen_switches>] <filenum-int> <filename-str>
sets $i0 with result
--------------------------------------------------------------------------*/
int
pcmd_fopen(param)
ESD *param;
{
	int erc;
	int filenum;
	ESD *fname = (ESD *) 0;
	char switches[8];

	if (!proc_level)
		return (eNotExecutingProc);

	if (get_switches(param, switches, sizeof(switches)))
	{
		strcpy(switches, "-r");
		if (proc_trace)
		{
			pputs("Warning: fopen defaulting to read\n");
			show_error_position(pcb_stack[proc_level - 1]);
		}
	}

	if (erc = get_filenum(param, &filenum))
		return (erc);

	if (pfile[filenum].f)
	{
		pprintf("file %d already open\n", filenum);
		return (eFATAL_ALREADY);
	}

	/*
	 * get temp for param (all exits after here must go through
	 * FUNC_RETURN)
	 */
	if (!(fname = esdalloc(ESD_NOMSZ)))
		return (eNoMemory);

	if (erc = gstr(param, fname, 1))
		goto FUNC_RETURN;

	iv[0] = 0;
	if (!(pfile[filenum].f = fopen(fname->pb, switches + 1)))
	{
		iv[0] = (long)errno;
		if (proc_trace)
		{
			pprintf("'%s'", fname->pb);
			pperror(" ");
		}
	}
	else if (proc_trace)
		pprintf("opened '%s' as ECU file %d\n", fname->pb, filenum);

	if (!erc)
	{
		pfile[filenum].n = fname;
		pfile[filenum].is_socket = 0;
	}

  FUNC_RETURN:
	if (erc)
		esdfree(fname);
	return (erc);
}							 /* end of pcmd_fopen */

/*+-------------------------------------------------------------------------
	pcmd_fputs(param)
fputs [-n] <filenum-int> <str>
-n do not output newline after <str>
<filenum-int> file number for operation
<str> string to write to file
--------------------------------------------------------------------------*/
int
pcmd_fputs(param)
ESD *param;
{
	int erc;
	int filenum;
	ESD *buf = (ESD *) 0;
	char switches[8];

	if (!proc_level)
		return (eNotExecutingProc);

	get_switches(param, switches, sizeof(switches));

	if (erc = get_filenum(param, &filenum))
		return (erc);

	if (!pfile[filenum].f)
		return (_file_not_open(filenum));

	if (!(buf = esdalloc(ESD_NOMSZ)))
		return (eNoMemory);

	if (erc = gstr(param, buf, 1))
		goto FUNC_RETURN;

	if (!fputs(buf->pb, pfile[filenum].f) && strlen(buf->pb))
	{
		pprintf(fwrite_error_fmt, filenum);
		erc = eFATAL_ALREADY;
		goto FUNC_RETURN;
	}

	if (!strchr(switches, 'n'))
		fputc(NL, pfile[filenum].f);

  FUNC_RETURN:
	esdfree(buf);
	return (erc);
}							 /* end of pcmd_fputs */

/*+-------------------------------------------------------------------------
	pcmd_fwrite(param)
fwrite <filenum-int> <str>
--------------------------------------------------------------------------*/
/*ARGSUSED*/
int
pcmd_fwrite(param)
ESD *param;
{
	param = 0; /* unusued */
	return (eNotImplemented);
#ifdef USE_FWRITE
	int erc;
	int filenum;
	ESD *buf = (ESD *) 0;

	if (!proc_level)
		return (eNotExecutingProc);

	if (erc = get_filenum(param, &filenum))
		return (erc);

	if (!pfile[filenum].f)
		return (_file_not_open(filenum));

	if (!(buf = esdalloc(ESD_NOMSZ)))
		return (eNoMemory);

	if (erc = gstr(param, buf, 1))
		goto FUNC_RETURN;

	if (!fputs(buf->pb, pfile[filenum].f) && strlen(buf->pb))
	{
		pprintf(fwrite_error_fmt, filenum);
		erc = eFATAL_ALREADY;
	}

  FUNC_RETURN:
	esdfree(buf);
	return (erc);
#endif
}							 /* end of pcmd_fwrite */

/*+-------------------------------------------------------------------------
	pcmd_fchmod(param)

fchmod <mode-str> | <mode-int> <filenum-int> | <filename-str>
$i0 = 0 if successful, else errno
--------------------------------------------------------------------------*/
int
pcmd_fchmod(param)
ESD *param;
{
	int erc;
	int filenum;
	ESD *fname = (ESD *) 0;
	ESD *mode = (ESD *) 0;
	long new_mode;
	char *path = "??";;

	if (!(fname = esdalloc(ESD_NOMSZ)))
		return (eNoMemory);

	if (!(mode = esdalloc(ESD_NOMSZ)))
	{
		esdfree(fname);
		return (eNoMemory);
	}

	if (erc = skip_cmd_break(param))
		goto FUNC_RETURN;
	else if (!gstr(param, mode, 0))
	{
		if (erc = str_to_filemode(mode->pb, &new_mode))
			goto FUNC_RETURN;
	}
	else if (erc = gint(param, &new_mode))
	{
		erc = eBadParameter;
		goto FUNC_RETURN;
	}

	if (erc = skip_cmd_break(param))
		goto FUNC_RETURN;
	else if (!gstr(param, fname, 1))
	{
		path = fname->pb;
		if (iv[0] = (long)(chmod(path, (unsigned short)new_mode)))
		{
			iv[0] = (long)errno;
			if (proc_trace)
				pperror(path);
		}
	}
	else if (!get_filenum(param, &filenum))
	{
		if (!pfile[filenum].f)
		{
			erc = (_file_not_open(filenum));
			iv[0] = EBADF;
		}
		else if (iv[0] = (long)chmod(pfile[filenum].n->pb,
				(unsigned short)new_mode))
		{
			iv[0] = (long)errno;
			if (proc_trace)
			{
				sprintf(fname->pb, "file %d", filenum);
				pperror(fname->pb);
			}
		}
		if (!iv[0])
			path = pfile[filenum].n->pb;
	}
	else
		erc = eBadParameter;

	if (proc_trace && !erc && !iv[0])
		pprintf("'%s' mode set to %o\n", path, (int)new_mode);

  FUNC_RETURN:
	esdfree(mode);
	esdfree(fname);

	return (erc);

}							 /* end of pcmd_fchmod */

/*+-------------------------------------------------------------------------
	pcmd_fdel(param)

fdel <filename-str>
$i0 = 0 if successful, else errno
--------------------------------------------------------------------------*/
int
pcmd_fdel(param)
ESD *param;
{
	int erc;
	ESD *fname = (ESD *) 0;

	if (!(fname = esdalloc(ESD_NOMSZ)))
		return (eNoMemory);

	if (erc = gstr(param, fname, 1))
		goto FUNC_RETURN;

	if (iv[0] = (long)unlink(fname->pb))
		iv[0] = (long)errno;

	if (proc_trace)
	{
		if (iv[0])
			pperror(fname->pb);
		else
			pprintf("'%s' deleted\n", fname->pb);
	}

  FUNC_RETURN:
	esdfree(fname);
	return (erc);
}							 /* end of pcmd_fdel */

/*+-------------------------------------------------------------------------
	pcmd_fseek(param)
fseek <filenum-int> <filepos-int>
sets $i0 with result
--------------------------------------------------------------------------*/
int
pcmd_fseek(param)
ESD *param;
{
	int erc;
	int filenum;
	long seekpos;

	if (!proc_level)
		return (eNotExecutingProc);
	if (erc = get_filenum(param, &filenum))
		return (erc);
	if (!pfile[filenum].f)
		return (_file_not_open(filenum));
	if (erc = gint(param, &seekpos))
		return (erc);

	iv[0] = 0;
	if (fseek(pfile[filenum].f, seekpos, 0) < 0)
	{
		iv[0] = (long)errno;
		if (proc_trace)
		{
			pprintf("file %d ", filenum);
			pperror("seek error");
		}
	}
	else if (proc_trace)
		pprintf("file %d set to position %ld\n", filenum, seekpos);

	return (erc);

}							 /* end of pcmd_fseek */

/*+-------------------------------------------------------------------------
	pcmd_fflush(param)
fflush <filenum-int> 
sets $i0 with result
--------------------------------------------------------------------------*/
int
pcmd_fflush(param)
ESD *param;
{
	int erc;
	int filenum;

	if (!proc_level)
		return (eNotExecutingProc);
	if (erc = get_filenum(param, &filenum))
		return (erc);
	if (!pfile[filenum].f)
		return (_file_not_open(filenum));

	iv[0] = 0;
	if (fflush(pfile[filenum].f) < 0)
	{
		iv[0] = (long)errno;
		if (proc_trace)
		{
			pprintf("file %d ", filenum);
			pperror("flush error");
		}
	}
	else if (proc_trace)
		pprintf("file %d flushed\n", filenum);

	return (erc);

}							 /* end of pcmd_fflush */

/*+-------------------------------------------------------------------------
	pcmd_mkdir(param)

mkdir <filename-str>
$i0 = 0 if successful, else errno
--------------------------------------------------------------------------*/
int
pcmd_mkdir(param)
ESD *param;
{
	int erc;
	ESD *fname = (ESD *) 0;

	if (!(fname = esdalloc(ESD_NOMSZ)))
		return (eNoMemory);

	if (erc = gstr(param, fname, 1))
		goto FUNC_RETURN;

	if (iv[0] = (long)mkdir(fname->pb, 0755))
		iv[0] = (long)errno;

	if (proc_trace)
	{
		if (iv[0])
			pperror(fname->pb);
		else
			pprintf("'%s' deleted\n", fname->pb);
	}

  FUNC_RETURN:
	esdfree(fname);
	return (erc);
}							 /* end of pcmd_mkdir */

/*+-------------------------------------------------------------------------
	pcmd_pclose(param)
pclose <filenum-int>
--------------------------------------------------------------------------*/
int
pcmd_pclose(param)
ESD *param;
{
	int erc;
	int filenum;

	if (!proc_level)
		return (eNotExecutingProc);

	if (erc = get_filenum(param, &filenum))
		return (erc);

	if (pfile[filenum].f)
	{
		pclose(pfile[filenum].f);
		pfile[filenum].f = (FILE *) 0;
		esdfree(pfile[filenum].n);
	}

	return (0);

}							 /* end of pcmd_pclose */

/*+-------------------------------------------------------------------------
	pcmd_popen(param)

popen [-<popen_switches>] <filenum-int> <filename-str>
sets $i0 with result
--------------------------------------------------------------------------*/
int
pcmd_popen(param)
ESD *param;
{
	int erc;
	int filenum;
	ESD *fname = 0;
	char switches[8];

#if !defined(M_UNIX)
	FILE *popen();

#endif

	if (!proc_level)
		return (eNotExecutingProc);

	if (get_switches(param, switches, sizeof(switches)))
	{
		strcpy(switches, "-r");
		if (proc_trace)
		{
			pputs("Warning: popen defaulting to read\n");
			show_error_position(pcb_stack[proc_level - 1]);
		}
	}

	if (erc = get_filenum(param, &filenum))
		return (erc);

	if (pfile[filenum].f)
	{
		pprintf("file %d already open\n", filenum);
		return (eFATAL_ALREADY);
	}

	if (!(fname = esdalloc(ESD_NOMSZ)))
		return (eNoMemory);

	if (erc = gstr(param, fname, 1))
		goto FUNC_RETURN;

	iv[0] = 0;
	if (pfile[filenum].f = popen(fname->pb, switches + 1))
	{
		iv[0] = (long)errno;
		if (proc_trace)
		{
			pprintf("'%s'", fname->pb);
			pperror(" ");
		}
	}
	else if (proc_trace)
		pprintf("opened '%s' as ECU file %d\n", fname->pb, filenum);

	if (!erc)
		pfile[filenum].n = fname;

  FUNC_RETURN:
	if (erc && fname)
		esdfree(fname);
	return (erc);
}							 /* end of pcmd_popen */

/*+-------------------------------------------------------------------------
	ifunc_ftell(param,pvalue)
%ftell(<filenum-int>)
--------------------------------------------------------------------------*/
int
ifunc_ftell(param, pvalue)
ESD *param;
long *pvalue;
{
	int erc;
	int filenum;
	long ftell();

	if (!proc_level)
		return (eNotExecutingProc);
	if (erc = skip_paren(param, 1))
		return (erc);
	if (erc = get_filenum(param, &filenum))
		return (erc);
	if (!pfile[filenum].f)
		return (_file_not_open(filenum));
	if (erc = skip_paren(param, 0))
		return (erc);

	*pvalue = ftell(pfile[filenum].f);
	return (0);
}							 /* end of ifunc_ftell */

/*+-------------------------------------------------------------------------
	_param_to_stat(param,pstat_rtnd)
--------------------------------------------------------------------------*/
int
_param_to_stat(param, pstat_rtnd)
ESD *param;
struct stat **pstat_rtnd;
{
	int erc;
	int filenum;
	static struct stat fst;
	struct stat *pstat = &fst;
	ESD *fname;

	errno = 0;

	if (erc = skip_paren(param, 1))
		return (erc);

	if (!(fname = esdalloc(ESD_NOMSZ)))
		return (eNoMemory);

	if (!gstr(param, fname, 1))
	{
		if (stat(fname->pb, pstat))
			pstat = (struct stat *)0;
	}
	else if (param->index = param->old_index, !get_filenum(param, &filenum))
	{
		if (!pfile[filenum].f)
		{
			esdfree(fname);
			return (_file_not_open(filenum));
		}
		if (stat(pfile[filenum].n->pb, pstat))
			pstat = (struct stat *)0;
	}
	else
		erc = eBadParameter;

	esdfree(fname);

	if (erc)
		return (erc);

	if (erc = skip_paren(param, 0))
		return (erc);

	*pstat_rtnd = pstat;
	if (proc_trace && !pstat)
		pperror("stat");
	return (0);

}							 /* end of _param_to_stat */

/*+-------------------------------------------------------------------------
	ifunc_fsize(param,pvalue)
%fsize(<filenum-int>)
%fsize('filename')
--------------------------------------------------------------------------*/
int
ifunc_fsize(param, pvalue)
ESD *param;
long *pvalue;
{
	int erc;
	struct stat *pstat;

	if (erc = _param_to_stat(param, &pstat))
		return (erc);
	if (!pstat)
		*pvalue = -1;
	else
		*pvalue = pstat->st_size;
	return (0);
}							 /* end of ifunc_fsize */

/*+-------------------------------------------------------------------------
	ifunc_fatime(param,pvalue)
%fatime(<filenum-int>)
%fatime('filename')
--------------------------------------------------------------------------*/
int
ifunc_fatime(param, pvalue)
ESD *param;
long *pvalue;
{
	int erc;
	struct stat *pstat;

	if (erc = _param_to_stat(param, &pstat))
		return (erc);
	if (!pstat)
		*pvalue = -1;
	else
		*pvalue = pstat->st_atime;
	return (0);
}							 /* end of ifunc_fatime */

/*+-------------------------------------------------------------------------
	ifunc_fmtime(param,pvalue)
%fmtime(<filenum-int>)
%fmtime('filename')
--------------------------------------------------------------------------*/
int
ifunc_fmtime(param, pvalue)
ESD *param;
long *pvalue;
{
	int erc;
	struct stat *pstat;

	if (erc = _param_to_stat(param, &pstat))
		return (erc);
	if (!pstat)
		*pvalue = -1;
	else
		*pvalue = pstat->st_mtime;
	return (0);
}							 /* end of ifunc_fmtime */

/*+-------------------------------------------------------------------------
	ifunc_fmode(param,pvalue)
%fmode(<filenum-int>)
%fmode('filename')
--------------------------------------------------------------------------*/
int
ifunc_fmode(param, pvalue)
ESD *param;
long *pvalue;
{
	int erc;
	struct stat *pstat;

	if (erc = _param_to_stat(param, &pstat))
		return (erc);
	if (!pstat)
		*pvalue = -1;
	else
		*pvalue = (long)pstat->st_mode;
	return (0);
}							 /* end of ifunc_fmode */

/*+-------------------------------------------------------------------------
	ifunc_isreg(param,pvalue)
%isreg(<filenum-int>)
%isreg('filename')
--------------------------------------------------------------------------*/
int
ifunc_isreg(param, pvalue)
ESD *param;
long *pvalue;
{
	int erc;

	if (erc = ifunc_fmode(param, pvalue))
		return (erc);
	if (*pvalue != -1)
		*pvalue = ((*pvalue & S_IFMT) == S_IFREG);
	return (0);
}							 /* end of ifunc_isreg */

/*+-------------------------------------------------------------------------
	ifunc_isdir(param,pvalue)
%isdir(<filenum-int>)
%isdir('filename')
--------------------------------------------------------------------------*/
int
ifunc_isdir(param, pvalue)
ESD *param;
long *pvalue;
{
	int erc;

	if (erc = ifunc_fmode(param, pvalue))
		return (erc);
	if (*pvalue != -1)
		*pvalue = ((*pvalue & S_IFMT) == S_IFDIR);
	return (0);
}							 /* end of ifunc_isdir */

/*+-------------------------------------------------------------------------
	ifunc_ischr(param,pvalue)
%ischr(<filenum-int>)
%ischr('filename')
--------------------------------------------------------------------------*/
int
ifunc_ischr(param, pvalue)
ESD *param;
long *pvalue;
{
	int erc;

	if (erc = ifunc_fmode(param, pvalue))
		return (erc);
	if (*pvalue != -1)
		*pvalue = ((*pvalue & S_IFMT) == S_IFCHR);
	return (0);
}							 /* end of ifunc_ischr */

/*+-------------------------------------------------------------------------
	get_free_filenum()
--------------------------------------------------------------------------*/
static int
get_free_filenum()
{
	int fnum;
	for(fnum = FILE_MAX; fnum >= 0; fnum--)
	{
		if(!pfile[fnum].f)
			return(fnum);
	}
	return(-1);
}	/* end of get_free_filenum */

/*+-------------------------------------------------------------------------
	socket_fdopen(fd,pecufnum) - open an ECU file descriptor for a socket
--------------------------------------------------------------------------*/
int
socket_fdopen(fd,pecufnum)
int fd;
int *pecufnum;
{
	int filenum;
	ESD *fname = (ESD *) 0;

	if (!proc_level)
		return (eNotExecutingProc);

	if((filenum = get_free_filenum()) < 0)
		return (eNoFreeFile);

	if (pfile[filenum].f)
	{
		pprintf("file %d already open\n", filenum);
		return (eFATAL_ALREADY);
	}

	if (!(fname = esdalloc(ESD_NOMSZ)))
		return (eNoMemory);

	sprintf(fname->pb,"socket_%03d",fd); fname->cb = strlen(fname->pb);
	fname->cb = strlen(fname->pb);

	pfile[filenum].f = fdopen(fd,"r+");
	setbuf(pfile[filenum].f,0);

	if (proc_trace)
		pprintf("fdopened '%s' as ECU file %d\n", fname->pb, filenum);

	pfile[filenum].n = fname;
	pfile[filenum].is_socket = 1;

	*pecufnum = filenum;
	return (0);

}	/* end of socket_fdopen */

/*+-------------------------------------------------------------------------
	ecufileno(filenum)
--------------------------------------------------------------------------*/
int
ecufileno(filenum)
UINT filenum;
{
	FILE *fp;
	if (filenum > FILE_MAX)
		return (-1);
	if(!(fp = pfile[filenum].f))
		return (-1);
	return(fileno(fp));
}	/* end of ecufileno */

/* vi: set tabstop=4 shiftwidth=4: */
/* end of pcmdfile.c */
