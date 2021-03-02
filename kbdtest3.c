char *revision = "3.37";

/*+-----------------------------------------------------------------------
	kbdtest3.c -- test keyboard values
	wht@wht.net

  See ecu manual section titled Function Key Recognition

  Defined functions:
	dump_putc(ch)
	dump_puts(str)
	hex_dump16(int16)
	hex_dump32(int32)
	hex_dump4(int4)
	hex_dump8(int8)
	hex_dump_fp(fp, str, len, title, terse_flag)
	k3ttymode(arg)
	main(argc, argv)
	read_kbd_string(buf, maxsize)
	termio_parity_text(cflag)
	tputstrs(strs)
	write_funckeymap_desc(fp, buf, buflen, name)
	xtoasc(ch, incl_3char)

------------------------------------------------------------------------*/
/*+:EDITS:*/
/*:04-26-2000-11:16-wht@bob-RELEASE 4.42 */
/*:02-26-1998-03:10-wht@kepler-fix some warnings: fidget */
/*:01-24-1997-02:37-wht@yuriatin-SOURCE RELEASE 4.00 */
/*:09-11-1996-20:00-wht@yuriatin-3.48-major telnet,curses,structural overhaul */
/*:11-23-1995-11:20-wht@kepler-source control 3.37 for tsx-11 */
/*:11-14-1995-10:23-wht@kepler-3.37.80-source control point: SOCKETS */
/*:10-15-1995-15:31-wht@jonah-must use TERMIO too now */
/*:01-12-1995-15:20-wht@n4hgf-apply Andrew Chernov 8-bit clean+FreeBSD patch */
/*:11-16-1994-19:50-wht@n4hgf-xtoasc call had only 1 arg */
/*:05-04-1994-04:39-wht@n4hgf-ECU release 3.30 */
/*:01-11-1994-15:37-wht@n4hgf-remove explicit paths from stty and sed */
/*:09-10-1992-13:59-wht@n4hgf-ECU release 3.20 */
/*:08-22-1992-15:39-wht@n4hgf-ECU release 3.20 BETA */
/*:04-19-1992-20:59-wht@n4hgf-nonansikeys to funckeymap */
/*:02-22-1992-12:46-root@n4hgf-thank markd@phoenix.pub.uu.oz.au for typo fix */
/*:01-06-1992-17:56-wht@tridom-much more error checking */
/*:01-06-1992-17:56-wht@tridom-eliminate sun stty - causes problems */
/*:08-28-1991-14:07-wht@n4hgf2-SVR4 cleanup by aega84!lh */
/*:08-17-1991-13:58-root@n4hgf-make kbd entirely raw */
/*:08-06-1991-13:12-wht@n4hgf-add parity reporting */
/*:08-06-1991-13:12-wht@n4hgf-some terminals reinvent parity bit's use */
/*:07-25-1991-12:58-wht@n4hgf-ECU release 3.10 */
/*:04-29-1991-18:24-wht@n4hgf-let us see what keyboards say */

#include <stdio.h>
#include <signal.h>
#include <ctype.h>
#include <sys/errno.h>
#include <string.h>
#include "ecu_config.h"
#include "ecu_types.h"
#include "ecu_stat.h"
#include "ecutermio.h"

char *ctime();
char *getenv();

#define VMIN_VALUE  32
#define VTIME_VALUE 2

#define TTYIN   0			 /* mnemonic */
#define TTYOUT  1			 /* mnemonic */
#define TTYERR  2			 /* mnemonic */

struct TERMIO lv;			 /* attributes for the line to remote */
struct TERMIO tv0;			 /* for saving, changing TTY atributes */
struct TERMIO tv;			 /* for saving, changing TTY atributes */

char *strs_intro[] =
{
	"\n",
	"Let's learn your keyboard.  I'll be making a file named kbdtest3.out\n",
	"For each of the key names shown below, please press the key and wait.\n",
	"\n",
	"If you do not have a key on your keyboard, make an alternate choice\n",
	"or press the space bar if nothing seems reasonable.\n",
	"\n",
	"If you press a key but don't see further activity after a second or two\n",
	"press the slash '/' key unless you can choose a reasonable alternate.\n",
	"Keys which produce duplicate the keystroke sequence of other keys are\n",
	"not acceptable.\n",
	"\n",
	"Do not use the same key for more than one function.\n",
	"\n",
	(char *)0
};

char *strs_thanks[] =
{
	"\n",
	"Thank you.  If you wish to mail me the contents of kbdtest3.out,\n",
	"please include  a detailed description of the system and software\n",
	"(i.e., \"Metrolink xterm keyboard on SCO 3.2r2\")\n",
	"(I WANT you to mail me results for non-SCO/non-ISC-console keyboards.)\n",
	"If you had to hack this program, mail it in its entirety as well.\n",
	"\n",
	"My address: wht@wht.net\n",
	(char *)0
};

char *strs_bktab[] =
{
	"You'll have to pick another function key (like F11?).  If you are\n",
	"using an xterm here perhaps a <Shift>Tab VT100 override will help.\n",
	(char *)0
};

struct keystruc
{
	char *ecuname;
	char *peoplename;
	int count;
	unsigned char str[VMIN_VALUE + 1];
};

struct keystruc need_names[] =
{
	{"BkTab", "Back Tab (Shift Tab)"},	/* 0 */
	{"CU5", "Unshifted Keypad 5"},	/* 1 */
	{"F1", "F1"},			 /* 2 */
	{"F2", "F2"},			 /* 3 */
	{"F3", "F3"},			 /* 4 */
	{"F4", "F4"},			 /* 5 */
	{"F5", "F5"},			 /* 6 */
	{"F6", "F6"},			 /* 7 */
	{"F7", "F7"},			 /* 8 */
	{"F8", "F8"},			 /* 9 */
	{"F9", "F9"},			 /* 10 */
	{"F10", "F10"},			 /* 11 */
	{"F11", "F11"},			 /* 12 */
	{"F12", "F12"},			 /* 13 */
	{"Ins", "Ins"},			 /* 14 */
#define I_HOME	15
	{"Home", "Home"},		 /* 15 */
#define I_END	16
	{"End", "End"},			 /* 16 */
	{"PgUp", "PgUp"},		 /* 17 */
	{"PgDn", "PgDn"},		 /* 18 */
	{"CUU", "Cursor Up"},	 /* 19 */
	{"CUD", "Cursor Down"},	 /* 21 */
	{"CUL", "Cursor Left"},	 /* 22 */
	{"CUR", "Cursor Right"}, /* 23 */
	{(char *)0, (char *)0}
};

char *parity_text = "<undetermined>";
static FILE *dumpfp;

/*+-------------------------------------------------------------------------
	dump_putc(ch)
--------------------------------------------------------------------------*/
void
dump_putc(ch)
char ch;
{
	fputc(ch, dumpfp);
}							 /* end of dump_putc */

/*+-------------------------------------------------------------------------
	dump_puts(str)
--------------------------------------------------------------------------*/
void
dump_puts(str)
char *str;
{
	fputs(str, dumpfp);
}							 /* end of dump_puts */

/*+-----------------------------------------------------------------------
	hex_dump#... subservient routines
------------------------------------------------------------------------*/
void
hex_dump4(int4)
unsigned char int4;
{
	int4 &= 15;
	dump_putc((int4 >= 10) ? (int4 + 'A' - 10) : (int4 + '0'));
}

void
hex_dump8(int8)
unsigned char int8;
{
	hex_dump4(int8 >> 4);
	hex_dump4(int8);
}

void
hex_dump16(int16)
unsigned short int16;
{
	hex_dump8(int16 >> 8);
	hex_dump8(int16);
}

void
hex_dump32(int32)
unsigned long int32;
{
	hex_dump16(int32 >> 16);
	hex_dump16(int32);
}

/*+-----------------------------------------------------------------
	hex_dump_fp(fp,str,len,title,terse_flag)

  if 'title' not NULL, title is printed... 'terse_flag'
  controls whether or not the title is "conspicuous" with
  hyphens before and after it making title line >70 chars long
------------------------------------------------------------------*/
void
hex_dump_fp(fp, str, len, title, terse_flag)
FILE *fp;
char *str;
int len;
char *title;
int terse_flag;
{
	int istr;
	int ipos = 0;
	int itmp;

	dumpfp = fp;

	if (title && (istr = strlen(title)))
	{
		if (!terse_flag)
		{
			ipos = (73 - istr) / 2;
			itmp = ipos;
			while (itmp--)
				dump_putc('-');
			dump_putc(' ');
			if (istr & 1)
				ipos--;
		}
		dump_puts(title);
		if (!terse_flag)
		{
			dump_putc(' ');
			while (ipos--)
				dump_putc('-');
		}
		dump_puts("\n");

	}

	istr = 0;
	while (istr < len)
	{
		hex_dump16(istr);
		dump_putc(' ');
		for (itmp = 0; itmp < 16; ++itmp)
		{
			ipos = istr + itmp;
			if (ipos >= len)
			{
				if (!terse_flag)
					dump_puts("   ");
				continue;
			}
			dump_putc(' ');
			hex_dump8(str[ipos]);
		}
		dump_puts(" | ");
		for (itmp = 0; itmp < 16; ++itmp)
		{
			ipos = istr + itmp;
			if ((ipos) >= len)
			{
				if (!terse_flag)
					dump_putc(' ');
			}
			else
			{
				dump_putc((str[ipos] >= ' ' && str[ipos] < 0x7f)
					? str[ipos] : '.');
			}
		}
		if (dumpfp == stdout)
			dump_puts(" |\r\n");
		else
			dump_puts(" |\n");
		istr += 16;
	}						 /* end of while(istr < len) */

}							 /* end of hex_dump_fp */

/*+-----------------------------------------------------------------------
	xtoasc(character) - Make all chars "printable"

  returns pointer to a static string containing printable version
  of a character.  If control char, printed as "^A", etc.
  if incl_3char set true, then space + ASCII assignment (e.g. "NUL") is
  appended to the string for non-printable graphics
------------------------------------------------------------------------*/
char *
xtoasc(ch)
unsigned char ch;
{
	static char gg[8];
	char *ascii_ctlstr =
	"nulsohstxetxeotenqackbelbs ht nl vt ff cr so si dle\
dc1dc2dc3dc4naksynetbcanem subescfs gs rs us sp ";

	if (ch == 0x7F)
		strcpy(gg, "del");
	else if (ch == 0x9b)
		strcpy(gg, "csi");
	else if (ch > 0x7F)
		sprintf(gg, "0x%02x", (unsigned char)ch);
	else if (ch > 0x20)
	{
		gg[0] = ch;
		gg[1] = 0;
	}
	else
	{
		strncpy(gg, ascii_ctlstr + (ch * 3), 3);
		gg[3] = 0;
	}
	return (gg);
}							 /* end of xtoasc */

/*+-------------------------------------------------------------------------
	write_funckeymap_desc(fp,buf,buflen,name)
--------------------------------------------------------------------------*/
void
write_funckeymap_desc(fp, buf, buflen, name)
FILE *fp;
unsigned char *buf;
int buflen;
char *name;
{
	char s256[256];

	sprintf(s256, "    %s:%s:", name, name);
	while (strlen(s256) < (unsigned)20)
		strcat(s256, " ");

	while (buflen--)
	{
		strcat(s256, xtoasc(*buf++, 0));
		if (buflen)
			strcat(s256, " ");
	}
	strcat(s256, "\n");

	fputs(s256, fp);

}							 /* end of write_funckeymap_desc */

/*+-------------------------------------------------------------------------
	tputstrs(strs)
--------------------------------------------------------------------------*/
void
tputstrs(strs)
char **strs;
{
	while (*strs)
		fputs(*strs++, stdout);
}							 /* end of tputstrs */

/*+-----------------------------------------------------------------------
	k3ttymode(arg) -- control user console (kbd/screen)

  Where arg ==
	0 restore attributes saved at start of execution
	1 raw mode

------------------------------------------------------------------------*/
void
k3ttymode(arg)
int arg;
{
	if (arg)
	{
		(void)ecugetattr(TTYIN, &tv);
		tv.c_cflag &= ~(CS8 | PARENB | PARODD);
		tv.c_cflag |= CS8;
		tv.c_iflag &= ~(INLCR | ICRNL | IGNCR | IXOFF | ISTRIP);
#if defined(IUCLC)
		tv.c_iflag &= ~IUCLC;
#endif
		tv.c_lflag &= ~(ICANON | ISIG | ECHO);
		tv.c_cc[VEOF] = '\01';
		tv.c_cc[VEOL] = '\0';
		tv.c_cc[VMIN] = VMIN_VALUE;
		tv.c_cc[VTIME] = VTIME_VALUE;
		(void)ecusetattr(TTYIN, TCSETAW, &tv);
	}
	else
		(void)ecusetattr(TTYIN, TCSETAW, &tv0);
}

/*+-------------------------------------------------------------------------
	read_kbd_string(buf,max)
--------------------------------------------------------------------------*/
int
read_kbd_string(buf, maxsize)
unsigned char *buf;
int maxsize;
{
	int count = read(TTYIN, buf, maxsize);

	return (count);

}							 /* end of read_kbd_string */

/*+-------------------------------------------------------------------------
	termio_parity_text(cflag)
--------------------------------------------------------------------------*/
char *
termio_parity_text(cflag)
int cflag;
{
	return ((cflag & PARENB) ? ((cflag & PARODD) ? "odd" : "even") : "none");
}							 /* end of termio_parity_text */

/*+-----------------------------------------------------------------------
	main()
------------------------------------------------------------------------*/
void
main(argc, argv)
int argc;
char **argv;
{
	int itmp;
	int count;
	int got_ctrl;
	int found_dup;
	int unusable = 0;
	char ch;
	char *ttype;
	char *cp;
	struct keystruc *key = need_names;
	struct keystruc *key2;
	unsigned char instr[VMIN_VALUE + 1];

#if !defined(sun)
	char s128[128];

#endif
	FILE *fpout;
	long now;
	int errflg = 0;
	char *outfile = "kbdtest3.out";

/* extern char *optarg; */
	extern int optind;

	setbuf(stdout, NULL);
	setbuf(stderr, NULL);

	while ((itmp = getopt(argc, argv, "")) != -1)
	{
		switch (itmp)
		{
			case '?':
				errflg++;
		}
	}

	if (optind == (argc - 1))
		outfile = argv[optind++];

	if (errflg || (optind != argc))
	{
		(void)fprintf(stderr, "usage: kbdtest3 [-hx] [outfile]\n");
		exit(1);
	}

	printf("\n\n\necu kbdtest3 revision %s\n", revision);
	tputstrs(strs_intro);
	if (!(fpout = fopen(outfile, "a")))
	{
		perror(outfile);
		exit(1);
	}

	ecugetattr(TTYIN, &tv0); /* get original status */
	parity_text = termio_parity_text(tv0.c_cflag);

	if (!(ttype = getenv("TERM")))
		ttype = "??";
	time(&now);
	fprintf(fpout, "# funckeymap for '%s' under ", ttype);
#if defined(M_SYSV) || defined(SCO32v5)
	fputs("SCO\n", fpout);
#else
#if defined(ISC)
	fputs("ISC\n", fpout);
#else
#if defined(MOTSVR3)
	fputs("MotSVR3\n", fpout);
#else
#if defined(sun)
	fputs("SunOS\n", fpout);
#else
#if defined(__FreeBSD__)
	fputs("FreeBSD\n", fpout);
#else
	fputs("??? OS\n", fpout);
#endif /* __FreeBSD__ */
#endif /* sun */
#endif /* MOTSVR3 */
#endif /* ISC */
#endif /* M_SYSV */

	fprintf(fpout, "# built by kbdtest3 %s %s", revision, ctime(&now));
	fprintf(fpout, "# keyboard parity required = %s\n", parity_text);
#if !defined(sun)
	fprintf(fpout, "# stty -a at kbdtest3 execution time:\n");
	fclose(fpout);
	strcpy(s128, "stty -a | sed -e 's/^/# /' >> ");
	strcat(s128, outfile);
	system(s128);
	if (!(fpout = fopen(outfile, "a")))
	{
		perror(outfile);
		exit(1);
	}
#endif
	fprintf(fpout, "%s\n", ttype);

	printf("Your keyboard driver parity is set to %s\n", parity_text);
	printf("press ^D (0x04) to terminate program early\n\n");
	k3ttymode(1);

	while (key->ecuname)
	{
		key->count = -1;
		printf("%-20.20s: ", key->peoplename);
		count = read_kbd_string(instr, VMIN_VALUE);
		if (!count)
		{
			printf("whoops ..... zero length read\n");
			break;
		}
		if (!count)
		{
			perror("keyboard");
			break;
		}

		if (!strcmp(key->ecuname, "BkTab") && (count == 1) &&
			(instr[0] == 9))
		{
			printf("produced the same keystroke sequence as TAB\n");
			tputstrs(strs_bktab);
			continue;
		}

		if ((count == 1) && ((instr[0] & 0x7F) == 4))
		{
			printf("--abort--\n");
			fputs("# User aborted entry.\n", fpout);
			unusable = 2;
			goto DONE;
		}

		if ((count == 1) && (instr[0] == '/'))
		{
			printf("--dead key--\n");
			fprintf(fpout, "# %s: dead key and no reasonable alternate\n",
				key->ecuname);
		}
		else if ((count == 1) && (instr[0] == ' '))
		{
			printf("--no key--\n");
			fprintf(fpout, "# %s: no key and no reasonable alternate\n",
				key->ecuname);
		}
		else
		{
			for (itmp = 0; itmp < count; itmp++)
				printf("%02x ", instr[itmp]);
			fputc(' ', stdout);
			got_ctrl = 0;
			for (itmp = 0; itmp < count; itmp++)
			{
				ch = instr[itmp] & 0x7F;
				if ((ch < ' ') || (ch > '~'))
					ch = '.', got_ctrl = 1;
				fputc(ch, stdout);
			}
			printf("\n");

			key->count = count;
			memcpy(key->str, instr, sizeof(key->str));
			write_funckeymap_desc(fpout, (unsigned char *)instr, count,
				key->ecuname);
			if (!got_ctrl)
			{
				printf("This looks like a printable character string.\n");
				printf("You might want to reconsider another key.\n");
				fprintf(fpout, "# the above entry is suspect\n");
			}
		}

		key++;
	}
	printf("\n");

	/*
	 * check for dup sequences
	 */
	found_dup = 0;
	for (key = need_names, key2 = key + 1;; key2++)
	{
		if (!key2->ecuname)
		{
			key++;
			if (!key->ecuname)
				break;
			key2 = key + 1;
			if (!key2->ecuname)
				break;
		}
		if ((key->count < 0) || (key2->count < 0) || (key->count != key2->count))
			continue;
		if (!memcmp(key->str, key2->str, key->count))
		{
			printf("'%s' and '%s' produced the same key sequence\n",
				key->peoplename, key2->peoplename);
			found_dup++;
		}
	}

	if (found_dup)
	{
		fprintf(fpout,
			"# found %d keystroke sequence duplication(s)\n", found_dup);
		unusable = 1;
	}

	if (need_names[I_HOME].count < 0)
	{
		cp = "# No Home key was successfully defined!\n";
		printf(cp + 2);
		fprintf(fpout, cp);
		unusable = 1;
	}
	if (need_names[I_END].count < 0)
	{
		cp = "# No End key was successfully defined!\n";
		printf(cp + 2);
		fprintf(fpout, cp);
		unusable = 1;
	}

  DONE:
	if (unusable)
	{
		printf("\nThis will be unusable.  Please try again.\n");
		fprintf(fpout, "# above entry is unusable\n");
	}
	else
	{
		printf("\nRemember to set keyboard parity to \"%s\" ",
			parity_text);
		fputs("when using this entry.\n", stdout);
		tputstrs(strs_thanks);
	}

	fputs("\n", fpout);
	fclose(fpout);
	k3ttymode(0);
	exit(0);

}							 /* end of main */

/* vi: set tabstop=4 shiftwidth=4: */
