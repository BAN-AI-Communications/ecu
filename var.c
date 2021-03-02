/*+-------------------------------------------------------------------------
	var.c - ecu variable routines
	wht@wht.net

  Defined functions:
	alloc_MKV(name)
	build_mkvi(param)
	build_mkvi_primitive(name)
	build_mkvs(param)
	build_mkvs_primitive(name, length)
	find_mkvi(name, pplong, auto_create)
	find_mkvs(name, ppesd, auto_create)
	free_mkvi(mkv)
	free_mkvs(mkv)
	get_ivptr(param, ppiv, auto_create)
	get_subscript(param, psubscript)
	get_svptr(param, ppsv, auto_create)
	mkv_proc_starting(pcb)
	mkv_proc_terminating(pcb)
	pcmd_mkvar(param)
	var_init()

--------------------------------------------------------------------------*/
/*+:EDITS:*/
/*:04-26-2000-11:16-wht@bob-RELEASE 4.42 */
/*:01-24-1997-02:38-wht@yuriatin-SOURCE RELEASE 4.00 */
/*:09-11-1996-20:01-wht@yuriatin-3.48-major telnet,curses,structural overhaul */
/*:12-06-1995-13:30-wht@n4hgf-termecu w/errno -1 consideration */
/*:11-23-1995-11:20-wht@kepler-source control 3.37 for tsx-11 */
/*:11-14-1995-10:23-wht@kepler-3.37.80-source control point: SOCKETS */
/*:05-04-1994-04:40-wht@n4hgf-ECU release 3.30 */
/*:09-10-1992-14:00-wht@n4hgf-ECU release 3.20 */
/*:08-22-1992-15:39-wht@n4hgf-ECU release 3.20 BETA */
/*:07-25-1991-12:59-wht@n4hgf-ECU release 3.10 */
/*:08-14-1990-20:40-wht@n4hgf-ecu3.00-flush old edit history */

#include "ecu.h"
#include "esd.h"
#define VDECL
#include "var.h"
#include "procedure.h"
#include "ecukey.h"
#include "ecuerror.h"
#include "termecu.h"


typedef union mkvu_type
{
	ESD *sv;
	long iv;
}
MKVU;

typedef struct mkv_type
{
	MKVU item;				 /* pointer to esd if sv or long if iv */
	struct mkv_type *next;	 /* next MKV in chain; if NULL, no more in
							  * chain */
	struct mkv_type *prev;	 /* previous MKV in chain; if NULL, top of
							  * chain */
	char *name;				 /* name of variable */
}
MKV;

MKV *mkvi_last = (MKV *) 0;
MKV *mkvs_last = (MKV *) 0;

/*+-------------------------------------------------------------------------
	var_init()
--------------------------------------------------------------------------*/
void
var_init()
{
	int itmp;

	for (itmp = 0; itmp < SVQUAN; itmp++)
	{
		if ((sv[itmp] = esdalloc(SVLEN)) == (ESD *) 0)
		{
			pputs("out of memory during variable initialization\n");
			errno = -1;
			termecu(TERMECU_MALLOC);
		}
	}

	for (itmp = 0; itmp < IVQUAN; itmp++)
		iv[itmp] = 0;

}							 /* end of var_init */

/*+-------------------------------------------------------------------------
	alloc_MKV(name)
--------------------------------------------------------------------------*/
MKV *
alloc_MKV(name)
char *name;
{
	MKV *mkv;

	if (!(mkv = (MKV *) malloc(sizeof(MKV))))
		return ((MKV *) 0);
	if (!(mkv->name = malloc(strlen(name) + 1)))
	{
		free((char *)mkv);
		return ((MKV *) 0);
	}
	strcpy(mkv->name, name);
	mkv->item.iv = 0;
	return (mkv);
}							 /* end of alloc_MKV */

/*+-------------------------------------------------------------------------
	build_mkvi_primitive(name)
--------------------------------------------------------------------------*/
build_mkvi_primitive(name)
char *name;
{
	MKV *mkv;

	if ((mkv = alloc_MKV(name)) == (MKV *) 0)
		return (eNoMemory);
	if (mkvi_last)
		mkvi_last->next = mkv;
	mkv->prev = mkvi_last;
	mkv->next = (MKV *) 0;
	mkvi_last = mkv;
	return (0);
}							 /* end of build_mkvi_primitive */

/*+-------------------------------------------------------------------------
	build_mkvi(param)
--------------------------------------------------------------------------*/
build_mkvi(param)
ESD *param;
{
	int erc;
	char name[16];

	if (erc = get_alphanum_zstr(param, name, sizeof(name)))
		return (erc);
	return (build_mkvi_primitive(name));

}							 /* end of build_mkvi */

/*+-------------------------------------------------------------------------
	build_mkvs_primitive(name,length)

trusts caller not to exceed ESD_MAXSZ
--------------------------------------------------------------------------*/
build_mkvs_primitive(name, length)
char *name;
int length;
{
	MKV *mkv;
	ESD *text;

	if ((text = esdalloc((int)length)) == (ESD *) 0)
		return (eNoMemory);

	if ((mkv = alloc_MKV(name)) == (MKV *) 0)
	{
		esdfree(text);
		return (eNoMemory);
	}

	mkv->item.sv = text;

	if (mkvs_last)
		mkvs_last->next = mkv;
	mkv->prev = mkvs_last;
	mkv->next = (MKV *) 0;
	mkvs_last = mkv;
	return (0);

}							 /* end of build_mkvs_primitive */

/*+-------------------------------------------------------------------------
	build_mkvs(param)
--------------------------------------------------------------------------*/
build_mkvs(param)
ESD *param;
{
	int erc;
	char name[16];
	UINT32 length;

	if (erc = get_alphanum_zstr(param, name, sizeof(name)))
		return (erc);

	if (erc = skip_paren(param, 1))
		return (erc);
	if (erc = gint(param, &length))
		return (erc);
	if (length > ESD_MAXSZ)
	{
		pprintf("max string size is %d ... cannot make %lu byte string\n",
			ESD_MAXSZ, length);
		return (eFATAL_ALREADY);
	}
	if (erc = skip_paren(param, 0))
		return (erc);

	return (build_mkvs_primitive(name, (int)length));

}							 /* end of build_mkvs */

/*+-------------------------------------------------------------------------
	pcmd_mkvar(param)

mkvar i<name>
mkvar s<name>(<size-int>)
--------------------------------------------------------------------------*/
int
pcmd_mkvar(param)
ESD *param;
{
	int erc;
	char vartype;

	if (!proc_level)
		return (eNotExecutingProc);

	do
	{
		if (erc = get_cmd_char(param, &vartype))
			return (erc);
		if (vartype == '$')
		{
			if (erc = get_cmd_char(param, &vartype))
				return (erc);
		}
		vartype = to_lower(vartype);
		switch (vartype)
		{
			case 'i':
				erc = build_mkvi(param);
				break;
			case 's':
				erc = build_mkvs(param);
				break;
			default:
				return (eIllegalVarType);
		}
		if (erc)
			return (erc);
	}
	while (!skip_comma(param));

	if (!end_of_cmd(param))
		return (eSyntaxError);

	return (0);

}							 /* end of pcmd_mkvar */

/*+-------------------------------------------------------------------------
	free_mkvi(mkv)
--------------------------------------------------------------------------*/
void
free_mkvi(mkv)
MKV *mkv;
{
	free(mkv->name);
	free((char *)mkv);
}							 /* end of free_mkvi */

/*+-------------------------------------------------------------------------
	free_mkvs(mkv)
--------------------------------------------------------------------------*/
void
free_mkvs(mkv)
MKV *mkv;
{
	esdfree(mkv->item.sv);
	free(mkv->name);
	free((char *)mkv);
}							 /* end of free_mkvs */

/*+-------------------------------------------------------------------------
	mkv_proc_starting(pcb)
--------------------------------------------------------------------------*/
void
mkv_proc_starting(pcb)
PCB *pcb;
{
	pcb->mkvs_last = (char *)mkvs_last;
	pcb->mkvi_last = (char *)mkvi_last;
}							 /* end of mkv_proc_starting */

/*+-------------------------------------------------------------------------
	mkv_proc_terminating(pcb)
--------------------------------------------------------------------------*/
void
mkv_proc_terminating(pcb)
PCB *pcb;
{
	MKV *pmkv;

	while (mkvi_last != (MKV *) pcb->mkvi_last)
	{
		pmkv = mkvi_last->prev;
		free_mkvi(mkvi_last);
		mkvi_last = pmkv;
	}
	while (mkvs_last != (MKV *) pcb->mkvs_last)
	{
		pmkv = mkvs_last->prev;
		free_mkvs(mkvs_last);
		mkvs_last = pmkv;
	}

}							 /* end of mkv_proc_terminating */

/*+-------------------------------------------------------------------------
	find_mkvs(name,ppesd,auto_create)
--------------------------------------------------------------------------*/
int
find_mkvs(name, ppesd, auto_create)
char *name;
ESD **ppesd;
int auto_create;
{
	int erc;
	MKV *mkv = mkvs_last;

	while (mkv)
	{
		if (!strcmp(name, mkv->name))
		{
			*ppesd = mkv->item.sv;
			return (0);
		}
		mkv = mkv->prev;
	}

	if (auto_create)
	{
		if (proc_trace)
			pprintf("automatic creation $s%s(256)\n", name);
		if (erc = build_mkvs_primitive(name, 256))
			return (erc);
		*ppesd = mkvs_last->item.sv;
		return (0);
	}

	return (eNoSuchVariable);

}							 /* end of find_mkvs */

/*+-------------------------------------------------------------------------
	find_mkvi(name,pplong,auto_create)
--------------------------------------------------------------------------*/
int
find_mkvi(name, pplong, auto_create)
char *name;
long **pplong;
int auto_create;
{
	int erc;
	MKV *mkv = mkvi_last;

	while (mkv)
	{
		if (!strcmp(name, mkv->name))
		{
			*pplong = &mkv->item.iv;
			return (0);
		}
		mkv = mkv->prev;
	}

	if (auto_create)
	{
		if (proc_trace)
			pprintf("creating $i%s\n", name);
		if (erc = build_mkvi_primitive(name))
			return (erc);
		*pplong = &mkvi_last->item.iv;
		return (0);
	}

	return (eNoSuchVariable);

}							 /* end of find_mkvi */

/*+-------------------------------------------------------------------------
	get_subscript(param,psubscript)
only called when '[' at pb + index
--------------------------------------------------------------------------*/
get_subscript(param, psubscript)
ESD *param;
UINT32 *psubscript;
{
	int erc;

	param->index++;
	if (erc = gint(param, psubscript))
		return (erc);
	if (skip_cmd_char(param, ']'))
		return (eSyntaxError);
	return (0);
}							 /* end of get_subscript */

/*+-------------------------------------------------------------------------
	get_ivptr(param,ppiv,auto_create)
called with index set to $i.....
                           ^
--------------------------------------------------------------------------*/
get_ivptr(param, ppiv, auto_create)
ESD *param;
long **ppiv;
int auto_create;
{
	int erc;
	UINT32 varnum;
	char name[16];

	if (end_of_cmd(param))
		return (eSyntaxError);
	else if (!get_numeric_value(param, &varnum))
		goto TEST_VARNUM;
	else if (*(param->pb + param->index) == '[')
	{
		if (erc = get_subscript(param, &varnum))
			return (erc);
	  TEST_VARNUM:
		if (varnum >= IVQUAN)
			return (eIllegalVarNumber);
		*ppiv = &iv[(int)varnum];
		return (0);
	}
	else if (get_alphanum_zstr(param, name, sizeof(name)))
		return (eInvalidVarName);

	return (find_mkvi(name, ppiv, auto_create));

}							 /* end of get_ivptr */

/*+-------------------------------------------------------------------------
	get_svptr(param,ppsv,auto_create)
called with index set to $s.....
--------------------------------------------------------------------------*/
int
get_svptr(param, ppsv, auto_create)
ESD *param;
ESD **ppsv;
int auto_create;
{
	int erc;
	UINT32 varnum;
	char name[16];

	if (end_of_cmd(param))
		return (eSyntaxError);
	else if (!get_numeric_value(param, &varnum))
		goto TEST_VARNUM;
	else if (*(param->pb + param->index) == '[')
	{
		if (erc = get_subscript(param, &varnum))
			return (erc);
	  TEST_VARNUM:
		if (varnum >= SVQUAN)
			return (eIllegalVarNumber);
		*ppsv = sv[(int)varnum];
		return (0);
	}
	if (get_alphanum_zstr(param, name, sizeof(name)))
		return (eInvalidVarName);
	return (find_mkvs(name, ppsv, auto_create));

}							 /* end of get_svptr */

/* vi: set tabstop=4 shiftwidth=4: */
/* end of var.c */
