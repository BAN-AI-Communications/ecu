/* CHK=0x8848 */
char *rev = "4.40";

/* #define FORCE_DASH_G */

#if defined(WHT) && !defined(FORCE_DASH_G)
#define FORCE_DASH_G
#endif

/*+-------------------------------------------------------------------------
	config.c - Makefile configuration program for ECU
	wht@n4hgf.atl.ga.us

  This must be compiled with the *NATIVE* cc or else you must
  fake all of the predefines the native compiler supplies.  The
  Configure script can pass stuff in CFLAGS to tickle config a
  bit.

  'I love it.  God help me, I love it so.  I love it more than my
  life.' -- George C.  Scott as Patton in _Patton_

  Defined functions:
	config_setup()
	find_string_in_file(fname, str)
	find_stuff()
	fix_libraries(libspec)
	gen_bsd_ldflags()
	gen_cc_cflags()
	gen_common_cflags()
	gen_freebsd_ldflags()
	gen_gcc_cflags()
	gen_hpux_ldflags()
	gen_isc_ldflags()
	gen_linux_ldflags()
	gen_motsvr3_ldflags()
	gen_sco_ldflags()
	gen_solaris2_ldflags()
	gen_sun_ldflags()
	gen_svr4_ldflags(include_socket)
	generate_config(mdir)
	goodbye(sig)
	main(argc, argv)
	tgetc()
	tgetopt(prompt, choices, deflt)
	tgets(buf)
	tputstrs(strs)
	tputstrsfp(strs)
	type (or the `nearest equivalent')

--------------------------------------------------------------------------*/
/*+:EDITS:*/
/*:04-26-2000-11:15-wht@bob-RELEASE 4.42 */
/*:03-24-2000-19:17-wht@blue-3.41-Linux uucp spool dir was wrong */
/*:02-25-1998-21:30-wht@menlo-port to redhat5/linux2 */
/*:12-22-1997-15:48-wht@kepler-use FIONREAD for SVR4 */
/*:12-21-1997-13:50-wht@sidonia-add CFG_UseSetupterm and CFG_UseStructWinsize */
/*:12-21-1997-00:18-wht@kepler-FIONREAD no good on OS5 console */
/*:12-18-1997-18:58-wht@kepler-adjust solaris */
/*:12-18-1997-06:05-wht@sidonia-port to AIX */
/*:12-18-1997-16:18-wht@kepler-w/help from robertle@sco.com fixOS5/OS7 */
/*:12-15-1997-21:14-wht@kepler-look for FIONREAD in linux/termios */
/*:12-15-1997-19:57-wht@fep-find_string_in_file prints stat diagnostics */
/*:12-15-1997-19:42-wht@fep-add CFG_FionreadInSocketH */
/*:12-15-1997-19:34-wht@fep-rename CFG_FilioH to CFG_FionreadInFilioH */
/*:12-12-1997-17:48-wht@kepler-minor minor tweak to add OS7 for Bob Lewis */
/*:03-16-1997-02:36-rll@felton.felton.ca.us-make boxes for SCO Products */
/*:01-24-1997-02:36-wht@yuriatin-SOURCE RELEASE 4.00 */
/*:11-12-1996-12:57-wht@yuriatin-SCO UnixWare and Motorola SVR4 equivalent */
/*:10-11-1996-02:09-wht@yuriatin-add CFG_FilioH */
/*:09-20-1996-05:12-wht@yuriatin-more comprehensive inet support check */
/*:09-17-1996-04:53-wht@yuriatin-autoconfig network for SCO */
/*:09-11-1996-19:59-wht@yuriatin-3.48-major telnet,curses,structural overhaul */
/*:09-05-1996-17:38-wht@kepler-WHT gets 132x85 */
/*:09-05-1996-17:26-wht@kepler-ncurses: look_for_ncurses,DCFG_UseNcursesH */
/*:09-04-1996-05:39-wht@kepler-use cua0, not ttyS0 for linux default */
/*:08-29-1996-15:52-wht@yuriatin-my Motorola gcc can pipe */
/*:08-22-1996-14:28-wht@fep-accommodate SVR4 socket */
/*:08-21-1996-16:27-wht@fep-use CFG_ ident for all config */
/*:07-31-1996-16:40-wht@kepler-use /etc/uucp for FreeBSD per dgy@rtd.com */
/*:07-24-1996-20:37-wht@n4hgf-add NL to end of ecu_euid_test.c */
/*:06-12-1996-19:17-wht@kepler-fix GCC_CFLAGS_EXTRA use */
/*:04-02-1996-11:06-wht@kepler-apply ache FreeBSD fix: cc switches */
/*:12-28-1995-12:54-wht@kepler-Andrey Chernov FreeBSD fixes */
/*:12-04-1995-01:12-wht@kepler-SCO system() wont work with SIGCLD SIG_IGN */
/*:12-03-1995-19:54-wht@gyro-add CFG_UseSeteuid */
/*:11-23-1995-11:20-wht@kepler-source control 3.37 for tsx-11 */
/*:11-23-1995-11:16-wht@kepler-default setuid uucp to yes */
/*:11-14-1995-10:23-wht@kepler-3.37.80-source control point: SOCKETS */
/*:11-12-1995-03:14-wht@kepler-CFG_TelnetOption good to go for SunOs & Linux */
/*:11-12-1995-00:00-wht@gyro-NO_ANSI_EMULATION becomes CFG_NoAnsiEmulation */
/*:11-04-1995-20:15-wht@kepler-24 lines are minimum ... what happened to fix? */
/*:11-03-1995-17:00-wht@wwtp1-allow for LOCAL_LIBS */
/*:11-03-1995-16:21-wht@wwtp1-v or 5 default for SCO */
/*:10-18-1995-04:29-wht@kepler-always use select for nap */
/*:10-15-1995-00:36-wht@n4hgf-drop SEAlink support */
/*:10-14-1995-16:26-wht@kepler-remove pretense of 286 support */
/*:09-02-1995-12:16-wht@n4hgf-runtime vs. ifdef for ecuungetty_appropriate */
/*:09-02-1995-11:53-wht@n4hgf-add predefines 32v5ers abandoned */
/*:09-01-1995-17:31-wht@n4hgf-OS5 gets -DSCO32v5 */
/*:08-27-1995-07:15-wht@n4hgf-take queue on libdir based on bin subdir */
/*:08-02-1995-19:13-wht@n4hgf-SCO engineer nice enough to finish OS5 port */
/*:06-23-1995-01:01-wht@n4hgf-use ECUOWNER,ECUGROUP,ECUMODE */
/*:06-15-1995-07:09-wht@kepler-dynamic CFG_HasStrerror */
/*:06-14-1995-06:58-wht@n4hgf-no more SCO -nointl or obsolete cross-compile */
/*:06-14-1995-06:32-wht@n4hgf-add SCO 3.2v5/Open Server 5 */
/*:06-13-1995-22:14-wht@n4hgf-was overwriting screen_lines with columns */
/*:04-09-1995-03:23-wht@gyro-add BSD_COMP to Solaris CFLAGS */
/*:03-28-1995-21:35-wht@n4hgf-clean up lines/cols input */
/*:03-11-1995-17:12-wht@kepler-auto-detect linux for 1st choice default */
/*:03-11-1995-17:09-wht@kepler-check linux/types.h if found */
/*:03-11-1995-17:08-wht@kepler-Linux can -pipe */
/*:01-15-1995-02:00-wht@n4hgf-rid rubbish undoc switches + add gcc_wall */
/*:01-15-1995-01:51-wht@n4hgf-patch the patch -- Andrew is patient with me */
/*:01-12-1995-15:19-wht@n4hgf-apply Andrew Chernov 8-bit clean+FreeBSD patch */
/*:06-10-1994-14:56-wht@kepler-was not including -g CFLAG on FORCE_DASH_G */
/*:05-11-1994-16:03-wht@n4hgf-#define WHT causes -DWHT */
/*:05-04-1994-04:38-wht@n4hgf-ECU release 3.30 */
/*:05-02-1994-03:10-wht@fep-cannot fix for all: warn of gcc overflow warnings */
/*:04-05-1994-17:11-wht@n4hgf-correct my first ever = for == substitution */
/*:03-13-1994-16:28-wht@fep-add Motorola SVR4 */
/*:01-25-1994-17:36-wht@n4hgf-USE_DECIMAL_PIDS->CFG_BinaryUucpPids */
/*:01-04-1994-07:23-wht@fep-implement CFG_DialTimeout */
/*:12-12-1993-13:44-wht@fep-use enum to catch missing cases */
/*:12-12-1993-13:36-wht@fep-remove obsolete -DISC22 */
/*:12-12-1993-13:31-wht@fep-MOTSVR3 == Motorola Delta SVR3 for 88k */
/*:12-10-1993-22:09-wht@n4vu-more Linux fixes + add sun -DCFG_TermiosLineio */
/*:12-02-1993-15:44-wht@n4hgf-some Linux includes may be root-readable only */
/*:11-14-1993-12:33-wht@n4hgf-HP-UX port by Carl Wuebker at HP */
/*:11-12-1993-11:00-wht@n4hgf-Linux changes by bob@vancouver.zadall.com */
/*:10-04-1993-04:07-wht@n4hgf-simplify SCO optimization choice */
/*:08-30-1993-11:44-wht@n4hgf-remove obsolete i86-specific gcc opts */
/*:08-17-1993-04:14-wht@n4hgf-Wuninitialized always on */
/*:08-17-1993-04:14-wht@n4hgf-remove -fforce-mem/-fforce-addr from gcc flags */
/*:03-17-1993-14:55-wht@n4hgf-remove soon to be obsolete LNG353 warnings */
/*:02-16-1993-13:18-wht@n4hgf-remove -Mlt128 from 286 per vancleef@netcom.com */
/*:01-30-1993-13:20-wht@n4hgf-add -Wuninitialized */
/*:01-30-1993-12:06-wht@n4hgf-gcc 2.3.3 gets -O6 */
/*:01-21-1993-00:07-wht@n4hgf-3.2v4 now the SCO UNIX default */
/*:01-21-1993-00:06-wht@n4hgf-augment gcc switches */
/*:12-12-1992-12:22-wht@n4hgf-no built in functions when using gcc */
/*:09-10-1992-13:58-wht@n4hgf-ECU release 3.20 */
/*:09-10-1992-04:39-wht@n4hgf-admonition about SunOS IPC config */
/*:08-22-1992-15:38-wht@n4hgf-ECU release 3.20 BETA */
/*:08-21-1992-13:46-wht@n4hgf-don't configure ecuungetty if not used on a sys */
/*:08-11-1992-06:04-wht@n4hgf-FORCE_DASH_G and 3.2v4 LNG353 warnings */
/*:07-12-1992-07:17-wht@n4hgf-3.2v4 has a fully functional nap and select */
/*:07-02-1992-20:41-wht@n4hgf-rework for more options + 3.2v4 CFG_PidType */
/*:06-18-1992-11:19-root@n4hgf-SCO 3.2v4 gcc CFLAG additions */
/*:04-23-1992-14:04-wht@n4hgf-had XENIX curses lib paths wrong */
/*:04-19-1992-21:55-wht@n4hgf-pressing return or enter gets tgeopt default */
/*:04-19-1992-21:43-wht@n4hgf-add default for tgetopt */
/*:04-19-1992-02:55-wht@n4hgf-add ESIX SVR4 config */
/*:04-17-1992-20:08-wht@n4hgf-add tty, bit_rate and parity questions */
/*:03-20-1992-03:08-wht@n4hgf-correct XENIX tcap/tlib test thanks to tbetz */
/*:03-01-1992-13:36-wht@n4hgf-add -Wswitch to gcc compiles */
/*:02-06-1992-15:23-wht@n4hgf-depressing ... SCO keeps chging their minds */
/*:10-17-1991-14:51-wht@n4hgf-add can_pipe code */
/*:09-03-1991-12:53-wht@n4hgf2-iron out sun gcc options */
/*:09-01-1991-16:32-wht@n4hgf2-show package and config versions */
/*:09-01-1991-15:59-wht@n4hgf2-generalize HDB Devices, etc. files location */
/*:08-28-1991-14:07-wht@n4hgf2-SVR4 cleanup by aega84!lh */
/*:08-25-1991-14:39-wht@n4hgf-SVR4 port thanks to aega84!lh */
/*:08-23-1991-01:37-wht@n4hgf-sun port */
/*:07-25-1991-12:55-wht@n4hgf-ECU release 3.10 */
/*:07-12-1991-14:02-wht@n4hgf-GCC140 update */
/*:05-02-1991-02:46-wht@n4hgf-take out M_TERMCAP in favor of ecucurses.h */
/*:04-28-1991-03:44-wht@n4hgf-add -Dconst= for X286 under UNIX */
/*:04-20-1991-17:26-wht@n4hgf-creation */

#include <stdio.h>
#include <ctype.h>
#include <signal.h>
#include <string.h>
#include <errno.h>

#include "ecu_config.h"
#include "ecu_types.h"
#include "ecu_stat.h"
#include "patchlevel.h"

/*
 * this definition must be made w/o the aid of config,
 * since we are just building it now
 */
#if defined(SVR4) || defined(USE_TERMIOS) || defined(CFG_TermiosLineio)
#include <termios.h>
#define	termio	termios
#define	setty(arg) tcsetattr(0,TCSADRAIN,(arg) ? &tty1 : &tty0)
#else
#include <termio.h>
#define setty(arg) ioctl(0,TCSETAW,(arg) ? (char *)&tty1 : (char *)&tty0)
#endif

struct termio tty0;
struct termio tty1;

#define	CBUFSIZE	128

enum sys_type
{
	S_SCO,
	S_ISC,
	S_SUN,
	S_ISCSVR4,
	S_ESIXSVR4,
	S_LINUX,
	S_HPUX,
	S_MOTSVR3,
	S_SVR4,
	S_BSD,
	S_SOLARIS2,
	S_FREEBSD,
	S_AIX,
};

enum sco_type
{
	X_X386,
	X_UNIX,
	X_32v4,
	X_32v5,
	X_OS7,
};

enum cc_type
{
	C_CC,
	C_GCC,
};

char *makedirs[] =
{
	".",
	"./bperr",
	"./ecufriend",
	"./ecuungetty",
	"./gendial",
	"./help",
	"./z",
	(char *)0
};

char *strs_intro1[] =
{
	"\n",
	".-------------------.\n",
	"| ECU configuration |\n",
	"`-------------------'\n",
	(char *)0
};
char *strs_intro2[] =
{
#ifdef WHT
	"WHT features enabled.\n",
	"\n",
#endif
	"Please answer these questions so that I can configure ECU.\n",
	"There are two types of answers, single character and string.\n",
	"Single character questions have the choices in () followed by a ?\n",
	"String questions are followed by a :\n",
	"Default answers appearing in [] are guesses.  If you wish to use\n",
	"the default answer for a question, just press CR.\n",
	"\n",
	"Abort (DEL, ^C, etc.) now if you do not wish to continue.\n",
	(char *)0
};

char *strs_no_timeval[] =
{
	"\n",
	"I am supposed to use select() for a nap() replacement, but I can't find\n",
	"a definition for `struct timeval' in sys/time.h nor sys/select.h\n",
	"By all means try a compile, but expect `unknown size', 'undefined' and\n",
	"other type error messages associated with missing struct definitions.\n",
	"Communicate your particulars to wht@n4hgf.atl.ga.us, PLEASE!\n",
	(char *)0
};

char *strs_mkdep[] =
{
	"\n",
	"You are ready to make the program. It make take a while (now is a good\n",
	"time to Read The Fine Manual).\n",
	"\n",
	"You MAY wish to make depend first.  It is not necessary to do this\n",
	"until you make source modifications or apply patches.\n",
	(char *)0
};

char *sco_cc_opts[] =
{
	"\t-M3e -O \\\n",		 /* XENIX/386 */
	"\t-M3e -O \\\n",		 /* UNIX/386 3.2.0-3.2v2 */
	"\t-M3e -O \\\n",		 /* UNIX/386 3.2v4 */
	"\t-b elf -dy -O \\\n",  /* 32v5 OS5 */
	"\t-b elf -dy -O \\\n"   /* OS7 */
};

char *sco_sigtype[] =
{
	"void",					 /* XENIX/386 */
	"void",					 /* UNIX/386 3.2.0-3.2v2 */
	"void",					 /* UNIX/386 3.2v4 */
	"void",					 /* UNIX/386 3.2v5 */
	"void"                   /* OS7 */
};

char *pid_type = "\t-DCFG_PidType=int \\\n";

/*
 * GCC CFLAGS
 */
char *gcc_opts[] =
{
#ifdef PEDANTIC
	"\t-pedantic -ansi -O2 -fno-builtin\\\n",	/* UH OH: see ecu README */
#else
	"\t-O2 -fno-builtin\\\n",
#endif
	"\t-fpcc-struct-return -fwritable-strings -finline-functions\\\n",
	"\t-W -Wuninitialized -Wunused -Wshadow -Wcomment -Wswitch\\\n",
	"\t-Dconst=\\\n",
	(char *)0
};

char *sco_gcc_opts[] =
{

 /*
  * XENIX/386
  */
	"\t-DM_XENIX -DM_SYSV -DM_SYS5 -DM_I386 \\\n\
\t-DM_BITFIELDS -DM_COFF -DM_I386 -DM_I86 -DM_I86SM \\\n\
\t-DM_INTERNAT -DM_SDATA -DM_STEXT -DM_SYS3 \\\n\
\t-DM_SYSIII -DM_WORDSWAP -Di386\\\n",

 /*
  * UNIX/386 3.2.0..3.2v2
  */
	"\t-DM_BITFIELDS -DM_COFF -DM_I386 -DM_I86 -DM_I86SM \\\n\
\t-DM_INTERNAT -DM_SDATA -DM_STEXT -DM_SYS3 -DM_SYS5 \\\n\
\t-DM_SYSIII -DM_SYSV -DM_UNIX -DM_WORDSWAP -DM_XENIX -Dunix -Di386\\\n",

 /*
  * UNIX/386 3.2v4
  */
	"\t-D_SVID -D_KR -D__STDC__=0 \\\n\
\t-DM_BITFIELDS -DM_COFF -DM_I386 -DM_I86 -DM_I86SM \\\n\
\t-DM_INTERNAT -DM_SDATA -DM_STEXT -DM_SYS3 -DM_SYS5 \\\n\
\t-DM_SYSIII -DM_SYSV -DM_UNIX -DM_WORDSWAP -DM_XENIX -Dunix -Di386\\\n",

 /*
  * 3.2v5 OS5
  */
	"\t-Wa,-belf  -D_NO_STATIC \\\n\
\t-DM_SDATA -DM_STEXT -DM_SYS3 -DM_SYS5 \\\n\
\t-DM_SYSV -DM_UNIX -Dunix -Di386\\\n",
 /*
  * OS7
  */
	"\t-Wa,-belf \\\n",
};

char *motsvr3_cc_opts = "\t-O -DMOTSVR3 \\\n";
char *motsvr3_gcc_opts = "\t-DMOTSVR3 \\\n";
char *motsvr3_sigtype = "void";

char *isc_cc_opts = "\t-O -DISC \\\n";
char *isc_gcc_opts = "\t-DISC -Di386\\\n";
char *isc_sigtype = "void";

char *sun_cc_opts = "\t-O \\\n";
char *sun_gcc_opts = "\t-Dsun=1 \\\n";	/* ansi gets __sun__ only */
char *sun_sigtype = "void";

char *solaris2_cc_opts = "\t-Dsun=1 -DSVR4 -DBSD_COMP -DCFG_GettodFtime \\\n";
char *solaris2_gcc_opts = "\t-Dsun=1 -DSVR4 -DBSD_COMP -DCFG_GettodFtime \\\n";

char *hpux_cc_opts = "\t-O -Dhpux=1\\\n";
char *hpux_gcc_opts = "\t-Dhpux=1 \\\n";
char *hpux_sigtype = "void";

char *svr4_cc_opts = "\t-O -DSVR4 \\\n";
char *svr4_gcc_opts = "\t-DSVR4 \\\n";
char *svr4_sigtype = "void";

char *aix_cc_opts = "\t-O -DAIX \\\n";
char *aix_gcc_opts = "\t-DAIX \\\n";
char *aix_sigtype = "void";

/*
 * ncurses.h location varies on Linux;
 * this works for Slackware 1.1.59 and some previous port
 */
char *linux_gcc_opts = "\t-Di386 -I/usr/include/ncurses \\\n";
char *linux_sigtype = "void";

char *bsd_gcc_opts = "\t-DBSD";
char *bsd_sigtype = "void";

#ifdef __FreeBSD__
char *freebsd_gcc_opts = "";

#else
char *freebsd_gcc_opts = "\t-D__FreeBSD__ \\\n";

#endif
char *freebsd_sigtype = "void";

int gcc_wall = 0;

/*
 * LIBS
 */
char *_sco_libs[] =
{
	"-lcurses -lx",
	"-lcurses -lmalloc -lc_s -lc -lx",
	"-lcurses -lmalloc -lc_s -lc -lx",
	"-L/usr/lib -lcurses -lmalloc -lc -lx",
	"-L/usr/lib -lcurses -lmalloc",              /* OS7 */
};
char *_sco2_libs[] =
{
	"-lcurses -lx",
	"-lcurses -lmalloc -lc_s -lc -lx",
	"-lcurses -lmalloc -lc_s -lc -lx",
	"",						 /* not used for 3.2v5 */
	"",						 /* not used for OS7 */
};
char **sco_libs = _sco_libs;

char *isc_libs = "-lcurses -ltelnet -lmalloc -lc_s -lc -lx";

char *motsvr3_libs = "-lcurses -ltermcap";

char *sun_libs = "-lcurses -ltermcap";

char *solaris2_libs = "-lcurses -ltermcap -lmalloc -lsocket -lnsl";

char *svr4_libs = "-lcurses -lmalloc";
char *svr4socket_libs = "-lcurses -lmalloc -lsocket -lnsl";

char *aix_libs = "-lcurses";

char *linux_libs = "-lncurses";

char *hpux_libs = "-lcurses -ltermcap";

char *bsd_libs = "-lcurses -ltermcap";

char *freebsd_libs = "-lncurses -lmytinfo -lcompat";

char *sco_system[] =
{
	"XENIX/386",
	"UNIX/386",
	"UNIX/386 3.2v4",
	"OS5/3.2v5",
	"OS7",
};

int intdial_to = 60;

char *malloc_3x = "\t-DCFG_Malloc3X \\\n";

char s256[256];
char *tty = "tty1A";
char *bit_rate = "9600";
int parity = 0;
char *bindir = "/usr/local/bin";
char *libdir = "/usr/local/lib/ecu";
char *symbolic = "";

int ecuungetty_appropriate = 0;
char *use_ecuungetty = "no";
char *ecuungetty_all_lines = "no";
char *ecu_owner = "bin";
char *ecu_group = "bin";
char *ecu_mode = "711";

int major_version_number;

int sys = -1;
int compiler = -1;
int sco_type = -1;
int look_for_network = 0;
char *libsocket_discovered = 0;
int can_pipe = 0;
int has_fd_set = 0;
char cfg_fdset[128] = "fd_set";
int seteuid_discovered = 0;
int has_strerror = 0;
int has_timeval = 0;
int need_timeval = 0;
int need_select_h = 0;
int fionread_in_filio_h = 0;
int fionread_in_socket_h = 0;
int look_for_ncurses = 0;
int need_ncurses_h = 0;

#ifdef WHT
int screen_lines = 85;
int screen_columns = 132;

#else
int screen_lines = 50;
int screen_columns = 80;

#endif

FILE *fpmake;

/*+-------------------------------------------------------------------------
	goodbye(sig)
--------------------------------------------------------------------------*/
void
goodbye(sig)
int sig;
{
	setty(0);
	printf("\r\n");
	exit(sig);
}							 /* end of goodbye */

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

/*+-------------------------------------------------------------------------
	tputstrsfp(strs)
--------------------------------------------------------------------------*/
void
tputstrsfp(strs)
char **strs;
{
	while (*strs)
		fputs(*strs++, fpmake);
}							 /* end of tputstrsfp */

/*+-------------------------------------------------------------------------
	tgetc()
--------------------------------------------------------------------------*/
char
tgetc()
{
	char rtn;

	setty(1);
	read(0, &rtn, 1);
	rtn &= 0x7F;
	setty(0);
	if (rtn != 0x0A)
		printf("\n", rtn);
	return (rtn);
}							 /* end of tgetc */

/*+-------------------------------------------------------------------------
	tgets(buf)
--------------------------------------------------------------------------*/
void
tgets(buf)
char *buf;
{
	char *tmp;

	if (!(fgets(buf, CBUFSIZE, stdin)))
		goodbye(1);

	if ((tmp = strchr(buf, '\n')))
		*tmp = '\0';
}							 /* end of tgets */

/*+-------------------------------------------------------------------------
	tgetopt(prompt, choices, deflt)
--------------------------------------------------------------------------*/
char
tgetopt(prompt, choices, deflt)
char *prompt;
char *choices;
char deflt;
{
	char rtn = 0;
	char response;
	char *cp;

	printf("\n%s (", prompt);
	while (!rtn)
	{
		cp = choices;
		while (*cp)
		{
			if (cp - choices)
				putchar(',');
			if (*cp == deflt)
				putchar('[');
			putchar(*cp);
			if (*cp == deflt)
				putchar(']');
			cp++;
		}
		printf(")? ");
		response = tgetc();
		if (strchr(choices, response))
			rtn = response;
		else if ((response == 0x0D) || (response == 0x0A))
			rtn = deflt;
		else
			printf("Please answer with one of (");
	}
	return (rtn);
}							 /* end of tgetopt */

/*+-------------------------------------------------------------------------
	gen_cc_cflags()
--------------------------------------------------------------------------*/
void
gen_cc_cflags()
{

	fputs("CFLAGS = \\\n", fpmake);
	switch (sys)
	{
		case S_SCO:
			fputs(sco_cc_opts[sco_type], fpmake);
			if (sco_type >= X_32v4)
			{
				fprintf(fpmake, "\t-DSCO32v4%s%s \\\n",
					(sco_type >= X_32v5) ? " -DSCO32v5" : "",
					(sco_type >= X_OS7) ? " -DSCOOS7" : "");
				need_timeval = 1;
			}
			if (sco_type >= X_UNIX)
				fputs(malloc_3x, fpmake);
			fputs("\t-DCFG_LockDir='\"/usr/spool/uucp\"' \\\n", fpmake);
			fputs("\t-DCFG_GettodFtime \\\n", fpmake);
			if(sco_type >= X_OS7) /* FIONREAD no good on OS5 console */
				fputs("\t-DCFG_FionreadRdchk \\\n", fpmake);
			break;
		case S_ISC:
			fputs(isc_cc_opts, fpmake);
			fputs(malloc_3x, fpmake);
			need_timeval = 1;
			fputs("\t-DCFG_LockDir='\"/usr/spool/locks\"' \\\n", fpmake);
			break;
		case S_MOTSVR3:
			fputs(motsvr3_cc_opts, fpmake);
			fputs(malloc_3x, fpmake);
			fputs("\t-DCFG_GettodFtime -DCFG_FionreadRdchk \\\n",
				fpmake);
			need_timeval = 1;
			fputs("\t-DCFG_LockDir='\"/usr/spool/locks\"' \\\n", fpmake);
			break;
		case S_SUN:
			fputs(sun_cc_opts, fpmake);
			fputs(malloc_3x, fpmake);
			fputs("\t-DCFG_TermiosLineio -DCFG_TelnetOption \\\n", fpmake);
			need_timeval = 1;
			fputs("\t-DCFG_FionreadRdchk \\\n", fpmake);
			fputs("\t-DCFG_LockDir='\"/var/spool/locks\"' \\\n", fpmake);
			break;
		case S_SOLARIS2:
			fputs(solaris2_cc_opts, fpmake);
			fputs(malloc_3x, fpmake);
			fputs("\t-DCFG_TermiosLineio \\\n", fpmake);
			need_timeval = 1;
			fputs("\t-DCFG_FionreadRdchk \\\n", fpmake);
			fputs("\t-DCFG_LockDir='\"/var/spool/locks\"' \\\n", fpmake);
			break;
		case S_HPUX:
			fputs(hpux_cc_opts, fpmake);
			fputs(malloc_3x, fpmake);
			fputs("\t-DCFG_FionreadRdchk\\\n", fpmake);
			need_timeval = 1;
			fputs("\t-DCFG_BinaryUucpPids\\\n", fpmake);
			fputs("\t-DCFG_LockDir='\"/usr/spool/uucp\"' \\\n", fpmake);
			break;
		case S_ISCSVR4:
			fputs(svr4_cc_opts, fpmake);
			fputs("\t-DISCSVR4 \\\n", fpmake);
			fputs(malloc_3x, fpmake);
			need_timeval = 1;
			fputs("\t-DCFG_LockDir='\"/var/spool/locks\"' \\\n", fpmake);
			break;
		case S_ESIXSVR4:
			fputs(svr4_cc_opts, fpmake);
			fputs("\t-DESIXSVR4 \\\n", fpmake);
			fputs(malloc_3x, fpmake);
			need_timeval = 1;
			fputs("\t-DCFG_LockDir='\"/var/spool/locks\"' \\\n", fpmake);
			break;
		case S_SVR4:
			fputs(svr4_cc_opts, fpmake);
			fputs("\t-DSVR4 \\\n", fpmake);
			fputs(malloc_3x, fpmake);
			need_timeval = 1;
			fputs("\t-DCFG_FionreadRdchk\\\n", fpmake);
			fputs("\t-DCFG_TermiosLineio -DCFG_TelnetOption \\\n", fpmake);
			fputs("\t-DCFG_LockDir='\"/var/spool/locks\"' \\\n", fpmake);
			break;
		case S_AIX:
			fputs(aix_cc_opts, fpmake);
			fputs(malloc_3x, fpmake);
			need_timeval = 1;
			fputs("\t-DCFG_TelnetOption \\\n", fpmake);
			fputs("\t-DCFG_FionreadRdchk -DCFG_GettodFtime\\\n", fpmake);
			fputs("\t-DCFG_UseStructWinsize \\\n", fpmake);
			fputs("\t-DCFG_LockDir='\"/var/spool/uucp\"' \\\n", fpmake);
			fputs("\t-DCFG_NeedStdlibH -DCFG_GetpgrpVoidArg\\\n", fpmake);
			break;
		default:
			printf("\ngen_cc_flags logic error %d.\n",sys);
			exit(1);
	}
	fputs("\t$(CFLAGS_EXTRA) $(CC_CFLAGS_EXTRA) \\\n ", fpmake);

}							 /* end of gen_cc_cflags */

/*+-------------------------------------------------------------------------
	gen_gcc_cflags()
--------------------------------------------------------------------------*/
void
gen_gcc_cflags()
{
	char **strs;

	fputs("CFLAGS = \\\n", fpmake);
	strs = gcc_opts;
	while (*strs)
		fputs(*strs++, fpmake);
	if (can_pipe)
		fputs("\t-pipe \\\n", fpmake);
	if (gcc_wall)
		fputs("\t-Wall \\\n", fpmake);
	switch (sys)
	{
		case S_SCO:
			fputs(sco_gcc_opts[sco_type], fpmake);
			if (sco_type >= X_32v4)
			{
				fprintf(fpmake, "\t-DSCO32v4%s%s \\\n",
					(sco_type >= X_32v5) ? " -DSCO32v5" : "",
					(sco_type >= X_OS7) ? " -DSCOOS7" : "");
				need_timeval = 1;
			}
			if (sco_type >= X_UNIX)
				fputs(malloc_3x, fpmake);
			fputs("\t-DCFG_LockDir='\"/usr/spool/uucp\"' \\\n", fpmake);
			fputs("\t-DCFG_GettodFtime \\\n", fpmake);
			if(sco_type >= X_OS7) /* FIONREAD no good on OS5 console */
				fputs("\t-DCFG_FionreadRdchk \\\n", fpmake);
			break;
		case S_ISC:
			fputs(isc_gcc_opts, fpmake);
			fputs(malloc_3x, fpmake);
			need_timeval = 1;
			fputs("\t-DCFG_LockDir='\"/usr/spool/locks\"' \\\n", fpmake);
			break;
		case S_MOTSVR3:
			fputs(motsvr3_gcc_opts, fpmake);
			fputs(malloc_3x, fpmake);
			fputs("\t-DCFG_GettodFtime \\\n", fpmake);
			need_timeval = 1;
			fputs("\t-DCFG_FionreadRdchk \\\n", fpmake);
			fputs("\t-DCFG_LockDir='\"/usr/spool/locks\"' \\\n", fpmake);
			break;
		case S_SUN:
			fputs(sun_gcc_opts, fpmake);
			fputs(malloc_3x, fpmake);
			fputs("\t-DCFG_TermiosLineio -DCFG_TelnetOption \\\n", fpmake);
			need_timeval = 1;
			fputs("\t-DCFG_FionreadRdchk \\\n", fpmake);
			fputs("\t-DCFG_LockDir='\"/var/spool/locks\"' \\\n", fpmake);
			break;
		case S_SOLARIS2:
			fputs(solaris2_gcc_opts, fpmake);
			fputs(malloc_3x, fpmake);
			fputs("\t-DCFG_TermiosLineio \\\n", fpmake);
			need_timeval = 1;
			fputs("\t-DCFG_FionreadRdchk \\\n", fpmake);
			fputs("\t-DCFG_LockDir='\"/var/spool/locks\"' \\\n", fpmake);
			break;
		case S_ISCSVR4:
			fputs(svr4_gcc_opts, fpmake);
			fputs("\t-DISCSVR4 \\\n", fpmake);
			fputs(malloc_3x, fpmake);
			need_timeval = 1;
			fputs("\t-DCFG_LockDir='\"/var/spool/locks\"' \\\n", fpmake);
			break;
		case S_ESIXSVR4:
			fputs(svr4_gcc_opts, fpmake);
			fputs("\t-DESIXSVR4 \\\n", fpmake);
			fputs(malloc_3x, fpmake);
			need_timeval = 1;
			fputs("\t-DCFG_LockDir='\"/var/spool/locks\"' \\\n", fpmake);
			break;
		case S_SVR4:
			fputs(svr4_gcc_opts, fpmake);
			fputs("\t-DSVR4 \\\n", fpmake);
			fputs(malloc_3x, fpmake);
			fputs("\t-DCFG_FionreadRdchk\\\n", fpmake);
			fputs("\t-DCFG_TermiosLineio -DCFG_TelnetOption \\\n", fpmake);
			need_timeval = 1;
			fputs("\t-DCFG_LockDir='\"/var/spool/locks\"' \\\n", fpmake);
			break;
		case S_AIX:
			fputs(aix_gcc_opts, fpmake);
			fputs(malloc_3x, fpmake);
			need_timeval = 1;
			fputs("\t-DCFG_TelnetOption \\\n", fpmake);
			fputs("\t-DCFG_FionreadRdchk -DCFG_GettodFtime\\\n", fpmake);
			fputs("\t-DCFG_UseStructWinsize \\\n", fpmake);
			fputs("\t-DCFG_LockDir='\"/var/spool/uucp\"' \\\n", fpmake);
			break;
		case S_LINUX:
			fputs(linux_gcc_opts, fpmake);
			fputs("\t-DCFG_FionreadRdchk -DCFG_TelnetOption \\\n", fpmake);
			fputs("\t-DCFG_UseSetupterm -DCFG_UseStructWinsize \\\n", fpmake);
			need_timeval = 1;
#if 0 /* no more -- redhat 5 complained bitterly */
			fputs("\t-DCFG_TermiosLineio \\\n", fpmake);
#endif
			fputs("\t-DCFG_LockDir='\"/var/spool/uucp\"' \\\n", fpmake);
			break;
		case S_HPUX:
			fputs(hpux_gcc_opts, fpmake);
			fputs("\t-DCFG_FionreadRdchk \\\n", fpmake);
			need_timeval = 1;
			fputs("\t-DCFG_BinaryUucpPids\\\n", fpmake);
			fputs("\t-DCFG_LockDir='\"/usr/spool/uucp\"' \\\n", fpmake);
			break;
		case S_BSD:
			fputs(bsd_gcc_opts, fpmake);
			fputs("\t-DCFG_FionreadRdchk \\\n", fpmake);
			need_timeval = 1;
			fputs("\t-DCFG_TermiosLineio \\\n", fpmake);
			fputs("\t-DCFG_MmapSHM -DCFG_GettodFtime \\\n", fpmake);
			fputs("\t-DCFG_NoAnsiEmulation \\\n", fpmake);
			fputs("\t-DCFG_LockDir='\"/var/spool/lock\"' \\\n", fpmake);
			break;
		case S_FREEBSD:
			fputs(freebsd_gcc_opts, fpmake);
			fputs("\t-DCFG_FionreadRdchk -DCFG_NeedStdlibH\\\n", fpmake);
			need_timeval = 1;
			fputs("\t-DCFG_UseSetupterm \\\n", fpmake);
			fputs("\t-DCFG_TelnetOption \\\n", fpmake);
			fputs("\t-DCFG_TermiosLineio \\\n", fpmake);
			fputs("\t-DCFG_MmapSHM \\\n", fpmake);
			fputs("\t-DCFG_LockDir='\"/var/spool/lock\"' \\\n", fpmake);
			break;
		default:
			printf("\ngen_gcc_cflags logic error %d.\n",sys);
			exit(1);
	}
	fputs("\t$(CFLAGS_EXTRA) $(GCC_CFLAGS_EXTRA) \\\n ", fpmake);
}							 /* end of gen_gcc_cflags */

/*+-------------------------------------------------------------------------
	gen_common_cflags()
--------------------------------------------------------------------------*/
void
gen_common_cflags()
{
	fputs("\t-DCFG_SigType=$(SIGTYPE) \\\n", fpmake);
	if (has_fd_set || has_strerror || need_select_h || fionread_in_filio_h)
	{
		fprintf(fpmake, "\t%s%s%s%s%s\\\n",
			(has_strerror) ? "-DCFG_HasStrerror " : "",
			(need_select_h) ? "-DCFG_IncSelectH " : "",
			(fionread_in_filio_h) ? "-DCFG_FionreadInFilioH " : "",
			(fionread_in_socket_h) ? "-DCFG_FionreadInSocketH " : "",
			(has_fd_set) ? "-DCFG_HasFdSet " : "");
		if(has_fd_set)
			fprintf(fpmake,"\t-DCFG_FDSET='%s' \\\n",cfg_fdset);
	}
	if (need_ncurses_h == 1)
		fputs("\t-DCFG_UseNcursesH \\\n",fpmake);
	else if (need_ncurses_h == 2)
		fprintf(fpmake, "\t-DCFG_UseNcursesNcursesH \\\n");
	if (seteuid_discovered)
		fprintf(fpmake, "\t-DCFG_UseSeteuid \\\n");
	if (libsocket_discovered)
		fputs("\t-DCFG_TelnetOption \\\n", fpmake);
	if (*use_ecuungetty == 'y')
		fputs("\t-DCFG_UseUngetty -DCFG_UngettyChown \\\n", fpmake);
	if (*ecuungetty_all_lines == 'y')
		fputs("\t-DCFG_UngettyAllLines \\\n", fpmake);
	fprintf(fpmake, "\t-DCFG_ScreenLinesMax=%d -DCFG_ScreenColsMax=%d \\\n",
		screen_lines, screen_columns);
	fprintf(fpmake, "\t-DCFG_DefaultTty='\"/dev/%s\"' \\\n", tty);
	fprintf(fpmake, "\t-DCFG_DefaultBitRate=%s \\\n", bit_rate);
	fprintf(fpmake, "\t-DCFG_DefaultParity=\"'%c'\" \\\n", parity);
	fprintf(fpmake, "\t-DCFG_DialTimeout=%d \\\n", intdial_to);
	fputs("\t-DCFG_HdbLibDir='\"$(HDBLIBDIR)\"' \\\n", fpmake);
	fputs("\t-DCFG_EcuLibDir='\"$(ECULIBDIR)\"' \\\n", fpmake);
	fputs(pid_type, fpmake);
	fputs("\t-DCFG_LogXfer \\\n", fpmake);

#ifdef WHT
	fputs("\t-DWHT \\\n", fpmake);
#endif
	fputs("\t$(SYMBOLIC) $(LOCAL_CFLAGS)\n\n", fpmake);

}							 /* end of gen_common_cflags */

/*+-------------------------------------------------------------------------
	fix_libraries(libspec) - add libraries to template (maybe)

  We want to place -lsocket into an SVR3 library list if we have
  "discovered sockets" on the machine

  Look for -lc_s or -lc on the lib line and place -lsocket in front
  of it.  If neither found, throw it on the end

  This is a kludge, but I don't have time to think.

--------------------------------------------------------------------------*/
char *
fix_libraries(libspec)
char *libspec;
{
	static char fixed[256];
	int found;
	int pos;
	char *search;
	char *cp;
	char ch;

	if (!libsocket_discovered)
		return (libspec);

	found = 0;
	cp = libspec;
	while (*cp)
	{
		if (!strncmp(cp, "-lc_s", 5) && (!(ch = *(cp + 5)) || (ch == ' ')))
		{
			found = 1;
			break;
		}
		cp++;
	}
	if (!found)
	{
		cp = libspec;
		while (*cp)
		{
			if (!strncmp(cp, "-lc", 3) && (!(ch = *(cp + 3)) || (ch == ' ')))
			{
				found = 1;
				break;
			}
			cp++;
		}
	}

	if (!found)
	{
		strcpy(fixed, libspec);
		strcat(fixed, " ");
		strcat(fixed, libsocket_discovered);
		return (fixed);
	}

	pos = (int)(cp - libspec);
	memcpy(fixed, libspec, pos);
	strcpy(fixed + pos, libsocket_discovered);
	strcat(fixed, " ");
	strcat(fixed, cp);
	return (fixed);

}							 /* end of fix_libraries */

/*+-------------------------------------------------------------------------
	gen_sco_ldflags()
--------------------------------------------------------------------------*/
void
gen_sco_ldflags()
{
	char *fixlib = fix_libraries(sco_libs[sco_type]);

	fputs("LDFLAGS = \\\n", fpmake);
	if (compiler == C_CC)
		fputs(sco_cc_opts[sco_type], fpmake);
	fputs("\t$(SYMBOLIC) $(LDFLAGS_EXTRA)\n\n", fpmake);

	fprintf(fpmake, "LIBS= $(LOCAL_LIBS) %s\n\n", fixlib);
}							 /* end of gen_sco_ldflags */

/*+-------------------------------------------------------------------------
	gen_isc_ldflags()
--------------------------------------------------------------------------*/
void
gen_isc_ldflags()
{
	char *fixlib = fix_libraries(isc_libs);

	fputs("LDFLAGS = \\\n", fpmake);
	fputs("\t$(SYMBOLIC) $(LDFLAGS_EXTRA)\n\n", fpmake);

	fprintf(fpmake, "LIBS= $(LOCAL_LIBS) %s\n\n", fixlib);
}							 /* end of gen_isc_ldflags */

/*+-------------------------------------------------------------------------
	gen_motsvr3_ldflags()
--------------------------------------------------------------------------*/
void
gen_motsvr3_ldflags()
{
	char *fixlib = fix_libraries(motsvr3_libs);

	fputs("LDFLAGS = \\\n", fpmake);
	fputs("\t$(SYMBOLIC) $(LDFLAGS_EXTRA)\n\n", fpmake);

	fprintf(fpmake, "LIBS= $(LOCAL_LIBS) %s\n\n", fixlib);
}							 /* end of gen_motsvr3_ldflags */

/*+-------------------------------------------------------------------------
	gen_sun_ldflags()
--------------------------------------------------------------------------*/
void
gen_sun_ldflags()
{
	fputs("LDFLAGS = \\\n", fpmake);
	fputs("\t$(SYMBOLIC) $(LDFLAGS_EXTRA)\n\n", fpmake);

	fprintf(fpmake, "LIBS= $(LOCAL_LIBS) %s\n\n", sun_libs);
}							 /* end of gen_sun_ldflags */

/*+-------------------------------------------------------------------------
	gen_solaris2_ldflags()
--------------------------------------------------------------------------*/
void
gen_solaris2_ldflags()
{
	fputs("LDFLAGS = \\\n", fpmake);
	fputs("\t$(SYMBOLIC) $(LDFLAGS_EXTRA)\n\n", fpmake);

	fprintf(fpmake, "LIBS= $(LOCAL_LIBS) %s\n\n", solaris2_libs);
}							 /* end of gen_solaris2_ldflags */

/*+-------------------------------------------------------------------------
	gen_hpux_ldflags()
--------------------------------------------------------------------------*/
void
gen_hpux_ldflags()
{
	fputs("LDFLAGS = \\\n", fpmake);
	fputs("\t$(SYMBOLIC) $(LDFLAGS_EXTRA)\n\n", fpmake);

	fprintf(fpmake, "LIBS= $(LOCAL_LIBS) %s\n\n", hpux_libs);
}							 /* end of gen_hpux_ldflags */

/*+-------------------------------------------------------------------------
	gen_svr4_ldflags(include_socket)
--------------------------------------------------------------------------*/
void
gen_svr4_ldflags(include_socket)
int include_socket;
{
	fputs("LDFLAGS = \\\n", fpmake);
	fputs("\t$(SYMBOLIC) $(LDFLAGS_EXTRA)\n\n", fpmake);
	fprintf(fpmake, "LIBS= $(LOCAL_LIBS) %s\n\n",
		(include_socket) ? svr4socket_libs : svr4_libs);
}							 /* end of gen_svr4_ldflags */

/*+-------------------------------------------------------------------------
	gen_aix_ldflags()
--------------------------------------------------------------------------*/
void
gen_aix_ldflags()
{
	fputs("LDFLAGS = \\\n", fpmake);
	fputs("\t$(SYMBOLIC) $(LDFLAGS_EXTRA)\n\n", fpmake);
	fprintf(fpmake, "LIBS= $(LOCAL_LIBS) %s\n\n",
		aix_libs);
}							 /* end of gen_aix_ldflags */

/*+-------------------------------------------------------------------------
	gen_linux_ldflags()
--------------------------------------------------------------------------*/
void
gen_linux_ldflags()
{
	fputs("LDFLAGS = \\\n", fpmake);
	fputs("\t$(SYMBOLIC) $(LDFLAGS_EXTRA)\n\n", fpmake);

	fprintf(fpmake, "LIBS= $(LOCAL_LIBS) %s\n\n", linux_libs);
}							 /* end of gen_linux_ldflags */

/*+-------------------------------------------------------------------------
	gen_bsd_ldflags()
--------------------------------------------------------------------------*/
void
gen_bsd_ldflags()
{
	fputs("LDFLAGS = \\\n", fpmake);
	fputs("\t$(SYMBOLIC) $(LDFLAGS_EXTRA)\n\n", fpmake);

	fprintf(fpmake, "LIBS= $(LOCAL_LIBS) %s\n\n", bsd_libs);
}							 /* end of gen_bsd_ldflags */

/*+-------------------------------------------------------------------------
	gen_freebsd_ldflags()
--------------------------------------------------------------------------*/
void
gen_freebsd_ldflags()
{
	fputs("LDFLAGS = \\\n", fpmake);
	fputs("\t$(SYMBOLIC) $(LDFLAGS_EXTRA)\n\n", fpmake);

	fprintf(fpmake, "LIBS= $(LOCAL_LIBS) %s\n\n", freebsd_libs);
}							 /* end of gen_freebsd_ldflags */

/*+-------------------------------------------------------------------------
	generate_config(mdir)
--------------------------------------------------------------------------*/
void
generate_config(mdir)
char *mdir;
{
	char mksrc[CBUFSIZE];
	char mkfl[CBUFSIZE];
	char s_cbuf[CBUFSIZE];
	FILE *fpsrc;
	FILE *local = fopen("config.local", "r");
	static int reported_config_local = 0;

	sprintf(mksrc, "%s/Make.src", mdir);
	sprintf(mkfl, "%s/Makefile", mdir);

	printf("Configuring %s from %s\n", mkfl, mksrc);

	if (!(fpsrc = fopen(mksrc, "r")))
	{
		perror(mksrc);
		return;
	}

	unlink(mkfl);
	if (!(fpmake = fopen(mkfl, "w")))
	{
		perror(mkfl);
		fclose(fpsrc);
		if (local)
			fclose(local);
		return;
	}

	fprintf(fpmake, "\
#+-----------------------------------------------------------------\n");
	fprintf(fpmake, "#         Makefile built from %s\n", mksrc);
	fprintf(fpmake, "#\n", mksrc);
	fprintf(fpmake, "\
# Configured by Configure revision %s for ECU %d.%02d\n",
		rev, major_version_number, PATCHLEVEL);
	fprintf(fpmake, "\
# Edits to this file will not survive another Configure operation.\n");
	fprintf(fpmake, "\
# Edit %s and rerun Configure to produce this file.\n",
		mksrc);
	fprintf(fpmake, "\
#+-----------------------------------------------------------------\n\n");

	if (local)
	{
		fprintf(fpmake, "# config.local inclusions\n");
		fprintf(fpmake, "# -----------------------\n");
		if (!reported_config_local)
			fputs("Found config.local:\n", stdout);
		while (fgets(s_cbuf, sizeof(s_cbuf), local))
		{
			if (!reported_config_local)
				fputs(s_cbuf, stdout);
			fputs(s_cbuf, fpmake);
		}
		if (!reported_config_local)
			fputs("\n", stdout);
		reported_config_local = 1;
		fclose(local);
		fprintf(fpmake, "# -----------------------\n");
		fputs("\n", fpmake);
	}

	switch (sys)
	{
		case S_SCO:
			fprintf(fpmake, "SIGTYPE = %s\n\n", sco_sigtype[sco_type]);
			break;
		case S_ISC:
			fprintf(fpmake, "SIGTYPE = %s\n\n", isc_sigtype);
			break;
		case S_MOTSVR3:
			fprintf(fpmake, "SIGTYPE = %s\n\n", motsvr3_sigtype);
			break;
		case S_LINUX:
			fprintf(fpmake, "SIGTYPE = %s\n\n", linux_sigtype);
			break;
		case S_SUN:
			fprintf(fpmake, "SIGTYPE = %s\n\n", sun_sigtype);
			break;
		case S_ISCSVR4:
		case S_ESIXSVR4:
		case S_SVR4:
		case S_SOLARIS2:
			fprintf(fpmake, "SIGTYPE = %s\n\n", svr4_sigtype);
			break;
		case S_AIX:
			fprintf(fpmake, "SIGTYPE = %s\n\n", aix_sigtype);
			break;
		case S_HPUX:
			fprintf(fpmake, "SIGTYPE = %s\n\n", hpux_sigtype);
			break;
		case S_FREEBSD:
			fprintf(fpmake, "SIGTYPE = %s\n\n", freebsd_sigtype);
			break;
		case S_BSD:
			fprintf(fpmake, "SIGTYPE = %s\n\n", bsd_sigtype);
			break;
		default:
			printf("\ngenerate_config logic error %d.\n",sys);
			exit(1);
	}

	switch (compiler)
	{
		case C_CC:
			fputs("CC = cc\n", fpmake);
			gen_cc_cflags();
			break;
		case C_GCC:
			if (sys == S_FREEBSD)
				fputs("CC = cc\n", fpmake);
			else
				fputs("CC = gcc\n", fpmake);
			gen_gcc_cflags();
			break;
		default:
			printf("I cannot handle compiler type %d\n", compiler);
			goodbye(1);
	}
	gen_common_cflags();

	switch (sys)
	{
		case S_SCO:
			gen_sco_ldflags();
			break;
		case S_ISC:
			gen_isc_ldflags();
			break;
		case S_MOTSVR3:
			gen_motsvr3_ldflags();
			break;
		case S_SUN:
			gen_sun_ldflags();
			break;
		case S_SOLARIS2:
			gen_solaris2_ldflags();
			break;
		case S_ISCSVR4:
		case S_ESIXSVR4:
			gen_svr4_ldflags(0);
			break;
		case S_SVR4:
			gen_svr4_ldflags(1);
			break;
		case S_AIX:
			gen_aix_ldflags();
			break;
		case S_LINUX:
			gen_linux_ldflags();
			break;
		case S_HPUX:
			gen_hpux_ldflags();
			break;
		case S_BSD:
			gen_bsd_ldflags();
			break;
		case S_FREEBSD:
			gen_freebsd_ldflags();
			break;
	}

	fprintf(fpmake, "LBIN = %s\n", bindir);
	fprintf(fpmake, "ECULIBDIR = %s\n", libdir);

	/*
	 * we need this info in Makefiles, so it is put here rather than
	 * ecu.h; must also have this passed thru in CFLAGS
	 */
	switch (sys)
	{
		case S_SCO:
			fprintf(fpmake, "SYSTEM = %s\n", sco_system[sco_type]);
			fputs("HDBLIBDIR = /usr/lib/uucp\n", fpmake);
			break;
		case S_ISC:
			fputs("SYSTEM = ISC-SVR3\n", fpmake);
			fputs("HDBLIBDIR = /usr/lib/uucp\n", fpmake);
			break;
		case S_SUN:
			fputs("SYSTEM = SunOS-4.1\n", fpmake);
			fputs("HDBLIBDIR = /etc/uucp\n", fpmake);
			break;
		case S_SOLARIS2:
			fputs("SYSTEM = Solaris 2.x\n", fpmake);
			fputs("HDBLIBDIR = /etc/uucp\n", fpmake);
			break;
		case S_MOTSVR3:
			fputs("SYSTEM = MotSVR3\n", fpmake);
			fputs("HDBLIBDIR = /usr/lib/uucp\n", fpmake);
			break;
		case S_ISCSVR4:
			fputs("SYSTEM = ISC-SVR4\n", fpmake);
			fputs("HDBLIBDIR = /etc/uucp\n", fpmake);
			break;
		case S_ESIXSVR4:
			fputs("SYSTEM = ESIX-SVR4\n", fpmake);
			fputs("HDBLIBDIR = /etc/uucp\n", fpmake);
			break;
		case S_SVR4:
			fputs("SYSTEM = SVR4\n", fpmake);
			fputs("HDBLIBDIR = /etc/uucp\n", fpmake);
			break;
		case S_AIX:
			fputs("SYSTEM = AIX\n", fpmake);
			fputs("HDBLIBDIR = /etc/uucp\n", fpmake);
			break;
		case S_LINUX:
			fputs("SYSTEM = Linux\n", fpmake);
			fputs("HDBLIBDIR = /usr/lib/uucp\n", fpmake);
			break;
		case S_HPUX:
			fputs("SYSTEM = HP-UX\n", fpmake);
			fputs("HDBLIBDIR = /usr/lib/uucp\n", fpmake);
			break;
		case S_BSD:
			fputs("SYSTEM = BSD\n", fpmake);
			break;
		case S_FREEBSD:
			fputs("SYSTEM = FreeBSD\n", fpmake);
			fputs("HDBLIBDIR = /etc/uucp\n", fpmake);
			break;
	}

	fprintf(fpmake, "USEUNGETTY = %s\n", use_ecuungetty);
	fprintf(fpmake, "ECUOWNER = %s\n", ecu_owner);
	fprintf(fpmake, "ECUGROUP = %s\n", ecu_group);
	fprintf(fpmake, "ECUMODE  = %s\n", ecu_mode);

	/*
	 * I want /bin/time in front of my commands
	 */
#if !defined(WHT)
	fputs("#", fpmake);
#endif
	fputs("BINTIME = time\n", fpmake);

	fprintf(fpmake, "\n#---- end of configured make variables\n");

	while (fgets(s_cbuf, sizeof(s_cbuf), fpsrc))
		fputs(s_cbuf, fpmake);

	fclose(fpsrc);
	fclose(fpmake);

}							 /* end of generate_config */

/*+-------------------------------------------------------------------------
	find_string_in_file(fname, str)
--------------------------------------------------------------------------*/
int
find_string_in_file(fname, str)
char *fname;
char *str;
{
	FILE *fp;
	char s1024[1024];
	int found = 0;
	struct stat _st,*st = &_st;

	printf("   Looking in %s ... ", fname);
#if 0
	if(stat(fname,st))
	{
		if(errno == EEXIST)
			printf("not on your system\n");
		else
			perror("\n        cannot find it");
		return(0);
	}
#endif
	sprintf(s1024, "fgrep %s %s 2>/dev/null", str, fname);
	if (!(fp = popen(s1024, "r")))
	{
		printf("I cannot `%s'", s1024);
		perror(" ");
		return (0);
	}
	while (fgets(s1024, sizeof(s1024), fp))
	{
		char *cp = s1024;
		int str_len = strlen(str);

		while (*cp)
		{
			if (!strncmp(cp, str, str_len))
			{
				found = 1;
				goto BREAK2;
			}
			cp++;
		}
	}
  BREAK2:
	fclose(fp);
	if (found)
		printf("found it!\n");
	else
		printf("not there.\n");
	return (found);
}							 /* end of find_string_in_file */

/*+-------------------------------------------------------------------------
	try_fd_set_shape(trial_definition)
--------------------------------------------------------------------------*/
int
try_fd_set_shape(trial_definition)
char *trial_definition;
{
	FILE *fp;
	int success = 0;
	char *fn = "/tmp/ecu_fdset.c";

	unlink(fn);
	if (!(fp = fopen(fn, "w")))
	{
		perror(fn);
		exit(1);
	}

	fputs("\
	#include <stdio.h>\n\
	#include <errno.h>\n\
	#include <sys/types.h>\n\
",fp);

	if(need_select_h)
		fputs("\t#include <sys/select.h>\n", fp);

	fputs("\
	#include <signal.h>\n\
	#include <termio.h>\n\
	\n\
	main(argc,argv)\n\
	int argc;\n\
	char **argv;\n\
	{\n\
",fp);

	fprintf(fp,"\t%s x;\n",trial_definition);
	fprintf(fp,"\
		printf(\"%s is the attempt\\n\");\n",trial_definition);
	fputs("\
		printf(\"success\\n\");\n\
		exit(0);\n\
	}	/* end of main */\n\
",fp);


	fclose(fp);
	unlink("/tmp/ecu_fdset.o");
	unlink("/tmp/ecu_fdset");
	system("cd /tmp; cc -o ecu_fdset ecu_fdset.c >/dev/null 2>&1");
	if (!(fp = popen("/tmp/ecu_fdset 2>&1", "r")))
		;
	else while (fgets(s256, sizeof(s256), fp))
	{
		if (!strncmp(s256, "success", 7))
		{
			success = 1;
			break;
		}
	}
	pclose(fp);
	unlink("/tmp/ecu_fdset.c");
	unlink("/tmp/ecu_fdset.o");
	unlink("/tmp/ecu_fdset");

	return(success);

}	/* end of try_fd_set_shape */

/*+-------------------------------------------------------------------------
	determine_fd_set_shape()
--------------------------------------------------------------------------*/
int
determine_fd_set_shape()
{
	int success = 0;
	int i;
#define DETERM_TRIAL_COUNT 2
	static char *trial[DETERM_TRIAL_COUNT ] =
	{
		"fd_set",
		"struct fd_set",
	};
	fputs("Let's see what shape fd_set takes ... \n",stdout);
	for(i = 0; i < DETERM_TRIAL_COUNT; i++)
	{
		printf("   testing `%s' ... ",trial[i]);
		success = try_fd_set_shape(trial[i]);
		printf("%s\n",(success) ? "SUCCESS" : "failed");
		if(success)
		{
			strcpy(cfg_fdset,trial[i]);
			break;
		}
	}
	return(success);

}	/* end of determine_fd_set_shape */

/*+-------------------------------------------------------------------------
	find_stuff()
--------------------------------------------------------------------------*/
void
find_stuff()
{
	FILE *fp;
	char *cp;
	struct stat st;
	static char *_socklibs[] =
	{
		"",
		"-lsocket",
		"-lsocket -lnsl",
		"-lsocket -ldl",
		0
	};
	char **socklib;
	static char *_headers[] =
	{
		"/usr/include/sys/socket.h",
		"/usr/include/netdb.h",
		"/usr/include/net/if.h",
		"/usr/include/netinet/in.h",
		"/usr/include/arpa/telnet.h",
		"/usr/include/arpa/inet.h",
		0
	};
	char **header;

	printf("Looking for fd_set ... \n");
	if(has_fd_set = find_string_in_file("/usr/include/sys/select.h",
			"fd_set"))
	{
		need_select_h = 1;
	}

	if (!has_fd_set)
	{
		has_fd_set = find_string_in_file("/usr/include/sys/types.h",
			"fd_set");
	}

	if (!has_fd_set && !stat("/usr/include/linux/types.h", &st))
	{
		if (has_fd_set = find_string_in_file("/usr/include/linux/types.h",
				"fd_set"))
		{
			printf(
				"      We trust sys/types.h will include linux/types.h.\n");
		}
	}
	if (has_fd_set)
		printf("We'll use fdset for select() arguments\n",cfg_fdset);
	else
	{
		printf("There is no `fd_set' in my visual range.\n");
		printf("So we'll use the old BSD int array bitmask for select().\n");
		printf("If I've missed my guess, you'll get harmless warnings\n");
		printf("about incorrect arguments to select().\n");
	}
	printf("\n");
	if (has_fd_set)
	{
		int success = determine_fd_set_shape();
		if(!success)
		{
			printf("\
I couldn't compile a program using fd_set with ways I know.\n");
			printf("Things look grim, but we'll see.\n");
		}
	}
	printf("We'll use `%s' for select() arguments\n",cfg_fdset);
	printf("\n");

	printf("Looking for `struct timeval' ... \n");

	has_timeval = find_string_in_file("/usr/include/sys/time.h",
		"struct timeval");
	if (!has_timeval)
	{
		if (has_timeval = find_string_in_file("/usr/include/sys/select.h",
				"struct timeval"))
		{
			need_select_h = 1;
		}
	}
	if (has_timeval)
		printf("We'll need `struct timeval' for select() arguments\n");
	else
	{
		printf("There is no `struct timeval' in my visual range.\n");
		printf("We can't use select() for a nap() replacement.\n");
	}
	printf("\n");

	printf("Looking for `strerror' ... \n");

	has_strerror = find_string_in_file("/usr/include/string.h",
		"strerror");
	if (has_strerror)
		printf("We'll use it.\n");
	else
	{
		printf("There is no `strerror' in my visual range.\n");
		printf("We will fake it.\n");
	}
	printf("\n");

	/*
	 * if the platform uses ncurses, lets's go looking for it
	 */
	if (look_for_ncurses)
	{
		struct stat st;
		char *ncurses = "<null>";

		if ((!stat("/usr/include/ncurses.h", &st)) &&
			((st.st_mode & S_IFMT) == S_IFREG))
		{
			need_ncurses_h = 1;
			ncurses = "ncurses";
		}
		else if ((!stat("/usr/include/ncurses/ncurses.h", &st)) &&
			((st.st_mode & S_IFMT) == S_IFREG))
		{
			need_ncurses_h = 2;
			ncurses = "ncurses/ncurses";
		}
		else
		{
			ncurses = "curses";	/* wont work */
			printf("I cannot find ncurses.h! TROUBLE predicted\n");
		}
		printf("We will look for ncurses in <%s.h> and -lncurses.\n\n",
			ncurses);
	}

	/*
	 * if we are going to setuid to unix, find out if we should use setuid
	 * or seteuid
	 */
	if (!strcmp(ecu_owner, "uucp"))
	{
		FILE *fp;
		char *fn = "/tmp/ecu_euid_test.c";

		printf("Looking for `seteuid' ... \n");
		seteuid_discovered = 1;
		unlink(fn);
		if (!(fp = fopen(fn, "w")))
		{
			perror(fn);
			exit(1);
		}
		fputs("main() { seteuid(0); printf(\"success\\n\"); exit(0); }\n", fp);
		fclose(fp);
		system("cd /tmp; cc -o ecu_euid_test ecu_euid_test.c >/dev/null 2>&1");
		if (!(fp = popen("/tmp/ecu_euid_test 2>&1", "r")))
			seteuid_discovered = 0;
		else if (!fgets(s256, sizeof(s256), fp))
			seteuid_discovered = 0;
		else if (strncmp(s256, "success", 7))
			seteuid_discovered = 0;
		pclose(fp);
		unlink("/tmp/ecu_euid_test.c");
		unlink("/tmp/ecu_euid_test.o");
		unlink("/tmp/ecu_euid_test");

		if (seteuid_discovered)
			printf("Found it.\n");
		else
		{
			printf("It appears there is no seteuid() available.\n");
			printf("We will use setuid().\n");
		}
		printf("\n");
	}

	/*
	 * do we need sys/filio.h for FIONREAD?
	 */
	printf("Looking for `FIONREAD' ... \n");
	fionread_in_filio_h = find_string_in_file("/usr/include/sys/filio.h",
		"FIONREAD");
	if (fionread_in_filio_h)
		printf("We will include sys/filio.h.\n");
	else if(fionread_in_socket_h =
		find_string_in_file("/usr/include/sys/socket.h","FIONREAD"))
	{
		printf("We will include sys/socket.h.\n");
	}
	else if(find_string_in_file("/usr/src/linux/include/linux/termios.h",
		"FIONREAD"))
	{
		printf("Linux (at least early ones) stash it here??!  Go figure!\n");
		printf("We already include this.\n");
	}
	else if(!find_string_in_file("/usr/include/sys/ioctl.h","FIONREAD"))
	{
		printf("I cannot find FIONREAD in places that I know to look.\n");
		printf("This may be ok if your system supports rdchk().  We'll see.\n");
		printf("This is expected wih Linux 2.0.\n");
	}
	printf("\n");


	/*
	 * if we should look for libsocket.a presence, now is the time ...
	 * since there are too many places where libraries can hide, we will
	 * let cc do the looking
	 */
	if (look_for_network)
	{
		FILE *fp;
		int got_it;
		char *fn = "/tmp/ecu_netld_test.c";

		printf("Looking for development system support for telnet/inet ... \n");
		libsocket_discovered = 0;

		/*
		 * create temp source using -lsocket material
		 */
		unlink(fn);
		if (!(fp = fopen(fn, "w")))
		{
			perror(fn);
			exit(1);
		}
		fputs("\
	main()\n\
	{\n\
	    printf(\"success\\n\");\n\
	    exit(0);\n\
	    gethostbyname();\n\
	    socket(0);\n\
		connect();\n\
	}\n", fp);
		fclose(fp);

		/*
		 * try "", "-lsocket", etc in turn
		 */
		libsocket_discovered = 0;
		for (socklib = _socklibs; !libsocket_discovered && *socklib; socklib++)
		{
			got_it = 1;		 /* assume success */
			sprintf(s256, "\
cd /tmp; cc -o ecu_netld_test ecu_netld_test.c %s >/dev/null 2>&1", *socklib);
			system(s256);
			if (!(fp = popen("/tmp/ecu_netld_test 2>&1", "r")))
				got_it = 0;
			else if (!fgets(s256, sizeof(s256), fp))
				got_it = 0;
			else if (strncmp(s256, "success", 7))
				got_it = 0;
			pclose(fp);
			if (got_it)
				libsocket_discovered = *socklib;
			else
				printf("   Cannot link networking code with %s\n",
					(**socklib) ? *socklib : "libc alone");

		}

		/*
		 * lose temporary files
		 */
#if 1
		unlink("/tmp/ecu_netld_test.c");
		unlink("/tmp/ecu_netld_test.o");
		unlink("/tmp/ecu_netld_test");
#endif

		if (libsocket_discovered)
			printf("   Can link with \"%s\"!!!\n", libsocket_discovered);

		/*
		 * now spot check headers
		 */
		if (libsocket_discovered)
		{

			for (header = _headers; *header; header++)
			{
				int i = stat(*header, &st);

				printf("   %s %s\n", (i) ? "Could not find" : "Found", *header);
				if (i)
					libsocket_discovered = 0;

			}
		}

		/*
		 * good news
		 */
		if (libsocket_discovered)
		{
			printf("\
You have networking ... linking with \"%s\".\n",
				libsocket_discovered);
		}
		else
		{
			printf("I can find no network libraries I know about.\n");
			printf("We will not attempt to include telnet support.\n");
		}
		printf("\n");
	}
}							 /* end of find_stuff */

/*+-------------------------------------------------------------------------
	config_setup()
--------------------------------------------------------------------------*/
void
config_setup()
{
	int itmp;

	/*
	 * set up raw/cooked tty setty() macro
	 */
#if defined(__NetBSD__) || defined(__FreeBSD__)
	tcgetattr(0, &tty0);
#else
	ioctl(0, TCGETA, &tty0);
#endif
	tty1 = tty0;
	tty1.c_lflag &= ~(ICANON);
	tty1.c_cc[VMIN] = 1;
	tty1.c_cc[VTIME] = 0;

	/*
	 * any signal except death of a child causes abort
	 */
	for (itmp = 1; itmp < NSIG; itmp++)
	{
		if (
#ifdef SIGCLD
			(itmp == SIGCLD) ||
#endif /* SIGCLD */
#ifdef SIGCHLD				 /* duplicate work on some systems */
			(itmp == SIGCHLD) ||
#endif /* SIGCLD */
			0)
		{
			;
		}
		else
			signal(itmp, goodbye);
	}
}							 /* end of config_setup */

/*+-------------------------------------------------------------------------
	main(argc, argv)
--------------------------------------------------------------------------*/
main(argc, argv)
int argc;
char **argv;
{
	int itmp;
	int opt;
	char s_cbuf[CBUFSIZE];
	char s2048[2048];
	char **makedir;
	char *tlib = "";
	char *cp;
	char *tcap = "";
	struct stat fst;
	extern int optind;
	extern char *optarg;

	setbuf(stdin, NULL);
	setbuf(stdout, NULL);

	/*
	 * a bit of foolishness that will come back to bite me one day
	 */
	if (PATCHLEVEL < 49)
		major_version_number = 4;
	else
		major_version_number = 3;

	while ((itmp = getopt(argc, argv, "W")) != -1)
	{
		switch (itmp)
		{
			case 'W':
				gcc_wall = 1;
				break;

		}
	}

	config_setup();

	tputstrs(strs_intro1);
	printf("ecu version = %d.%02d, config version %s\n\n",
		major_version_number, PATCHLEVEL, rev);
	tputstrs(strs_intro2);

	/*
	 * try to make a stab at default some vendor compilers have
	 * thoughtfully provided a reliable built-in default to ISC if not one
	 * of those two
	 */
	itmp = 's';
#ifdef M_SYSV
	itmp = 's';
#endif
#if defined(hpux)
	itmp = 'h';
#endif
#if defined(m68k) || defined(m88k)
	itmp = 'M';
#endif
#if defined(MOTSVR3)
	itmp = 'm';
#endif
#if defined(__NetBSD__)
	itmp = 'N';
#endif
#if defined(__FreeBSD__)
	itmp = 'F';
#endif
#if defined(linux)
	itmp = 'l';
#endif
#ifdef sun
	itmp = 'S';
#endif
#if defined (sun) && (defined (__svr4__) || defined(SVR4))

	/*
	 * line 27 of gcc/config/sparc/sol2.h in CPP_PREDEFINES has __svr4__
	 * Your mileage may vary.....
	 */
	itmp = 'O';
#endif

	if (sys < 0)
	{
		switch (opt = tgetopt("\
Choose your system type (or the `nearest equivalent')\n\
      s  SCO family (incl. UnixWare)    S  SunOS 4.1.x\n\
      h  HP-UX 9.01 or later            O  Solaris 2.x\n\
      4  SVR4                           i  ISC SVR32 ** (2.2 or later)\n\
      m  Motorola Delta SVR32/[68]8k    I  ISC SVR4 **\n\
      l  Linux 1.1.58 or later          E  ESIX SVR4 **\n\
      N  NetBSD                         F  FreeBSD\n\
      A  AIX\n\
Those marked ** have not been tested or verified in quite some time.\n\
\n\
",
				"shl4mNSOiIEFA", itmp))
		{
			case 's':
				sys = S_SCO;
				tty = "tty1A";
				look_for_network = 1;	/* old basic SCO DS has no inet */
				break;
			case 'i':
				sys = S_ISC;
				tty = "acu00";
				look_for_network = 1;	/* I dont have ISC; guess at config
										 * time */
				break;
			case 'm':
				sys = S_MOTSVR3;
				tty = "tty00";
				look_for_network = 1;	/* I dont have 1; guess at config
										 * time */
				break;
			case '4':
				sys = S_SVR4;
				tty = "ttys0";
				break;
			case 'A':
				sys = S_AIX;
				tty = "tty0";
				break;
			case 'S':
				sys = S_SUN;
				tty = "ttya";
				printf("\
\nYou need System V IPC configured in your kernel. I didn't check for it.\n");
				break;
			case 'O':
				sys = S_SOLARIS2;
				tty = "ttya";
				break;
			case 'I':
				sys = S_ISCSVR4;
				tty = "tty00";
				look_for_network = 1;	/* I dont have 1; guess at config
										 * time */
				break;
			case 'E':
				sys = S_ESIXSVR4;
				tty = "tty00";
				look_for_network = 1;	/* I dont have 1; guess at config
										 * time */
				break;
			case 'l':
				sys = S_LINUX;
				tty = "cua0";
#if defined(WHT) /* it is good to be da king */
				tty = "ttyS42";
#endif
				compiler = C_GCC;
				look_for_ncurses = 1;
				can_pipe = 1;
				break;
			case 'h':
				sys = S_HPUX;
				tty = "cul00";
				look_for_network = 1;	/* I dont have 1; guess at config
										 * time */
				break;
			case 'N':
				sys = S_BSD;
				tty = "com1";
				compiler = C_GCC;
				printf("Gcc will be used for all compilations.\n");
				can_pipe = 1;
				break;
			case 'F':
				sys = S_FREEBSD;
				tty = "cuaa0";
				compiler = C_GCC;
				can_pipe = 1;
				look_for_ncurses = 1;
				pid_type = "\t-DCFG_PidType=pid_t \\\n";
				break;
		}
	}

	if ((sys == S_SCO) && (sco_type < 0))
	{
		char defchar = 'v';
		char choice;
		struct stat st;

		if (!stat("/stand", &st))
			defchar = '5';
		if (!stat("/etc/vfstab", &st))
			defchar = 'U';
		switch (choice = tgetopt("\
The most recent SCO platforms tested with this rev of ecu are\n\
   SCO OpenServer 5.0.5\n\
   UnixWare 7.1.1\n\
\n\
Choose between 3  XENIX/386,\n\
               u  ODT 1.0.x, ODT 1.1, UNIX 3.2.0-3.2v2 or\n\
               v  ODT 2.0, ODT 3.0, UNIX 3.2v4\n\
               5  OpenServer 5, UNIX 3.2v5\n\
               U  UnixWare 7.0.0 or later\n\
 ",
				"23uv5U",
				defchar
			))
		{
			case '3':
				ecuungetty_appropriate = 1;
				need_select_h = 1;
				sco_type = X_X386;
				tlib = "/lib/386/Slibtermlib.a";
				tcap = "/lib/386/Slibtermcap.a";
				break;
			case 'u':
				ecuungetty_appropriate = 1;
				sco_type = X_UNIX;
				need_select_h = 1;
				tlib = "/usr/ccs/lib/libcurses.a";
				tcap = "LOGIC ERROR";
				break;
			case 'v':
				ecuungetty_appropriate = 1;
				sco_type = X_32v4;
				pid_type = "\t-DCFG_PidType=pid_t\\\n";
				tlib = "/usr/lib/libtermlib.a";
				tcap = "/usr/lib/libtermcap.a";
				break;
			case '5':
#if 0
			case '7':
#endif
				if(choice == '5')
					sco_type = X_32v5;
				else if(choice == '7')
					sco_type = X_OS7;
				pid_type = "\t-DCFG_PidType=pid_t\\\n";
				tlib = "LOGIC_ERROR";
				tcap = tlib;
				ecu_owner = "uucp";
				ecu_group = "bin";
				ecu_mode = "4711";
				fputs("\
You will be able to access only lines writable by the user or by uucp.\n",
					stdout);
				fputs("\
ECU will be automatically installed to run setuid uucp.\n", stdout);
				break;
			case 'U':
				tty = "tty00";
				sys = S_SVR4;
				ecu_owner = "uucp";
				ecu_group = "bin";
				ecu_mode = "4711";
				break;
		}
		/*
		 * multivariate SCO development environment
		 */
		if (sys == S_SVR4)
			;
		else if(sco_type < 0)
		{
			printf("logic error in sco_type determination\n");
			goodbye(255);
		}
		else if ((sco_type == X_32v5) || (sco_type == X_OS7))
			sco_libs = _sco_libs;
		else if (!access(tcap, 0))
			sco_libs = _sco2_libs;
		else if (!access(tlib, 0))
			sco_libs = _sco_libs;
		else
		{
			printf("\nI find neither %s nor %s.\n", tcap, tlib);
			printf("Do you have curses support installed? I cannot proceed.\n");
			goodbye(1);
		}
	}

	if (compiler < 0)
	{
		switch (tgetopt("Do you wish to use cc or gcc", "cg", 'c'))
		{
			case 'c':
				compiler = C_CC;
				break;

			case 'g':
				compiler = C_GCC;
#ifdef PEDANTIC
				printf("I see we are being brave, er pedantic.\n\n");
#endif
				switch (sys)
				{
					case S_SUN:
					case S_SOLARIS2:
					case S_ISCSVR4:
					case S_ESIXSVR4:
						can_pipe = 1;	/* native as reads from stdin ok */
						break;

						/*
						 * systems which use only GCC set can_pipe
						 * elsewhere
						 */
					default:
						break;
				}

				break;
		}
	}

	if (compiler == C_GCC)
	{
		printf("\
\nYou may safely ignore a large number of `overflow in implicit constant\n\
conversion' and `empty body in else-statement' warnings that some versions\n\
of gcc may emit.\n");
	}

	/*
	 * ecuungetty support
	 */
	if (ecuungetty_appropriate)
	{
		switch (tgetopt("\
If you have gettys running on any of your modem lines, you want to use\n\
ecuungetty to manage the shared dial-in/dial-out protocol (including\n\
changing ownership of ports to the requesting user when opening and\n\
restoring to uucp ownership upon closing).  If you do not use UUCP nor\n\
have dial-in access to your computer, you may wish to omit ecuungetty\n\
support.  Answer 'y' if you are not sure.\n\
Do you want ecuungetty support? ", "yn", 'y'))
		{
			case 'y':
				use_ecuungetty = "yes";
				break;
			case 'n':
				use_ecuungetty = "no";
				break;
		}
		if (*use_ecuungetty == 'y')
		{
			switch (tgetopt("\
Generally speaking, serial lines appearing in your UUCP Devices file are\n\
associated with shared dial-in/dial-out use, while single use lines\n\
do not appear there; your customs may vary.  You may wish to restrict\n\
ecuungetty operation only to serial lines which appear in your UUCP\n\
Devices file.  If you answer yes below, ecuungetty will only manage\n\
serial lines appearing in Devices, leaving ownership and permissions\n\
alone on other lines.  Answer 'y' if you are not sure.\n\
Do you want ecuungetty to manage only lines in Devices? ", "yn", 'y'))
			{
				case 'y':
					ecuungetty_all_lines = "no";
					break;
				case 'n':
					ecuungetty_all_lines = "yes";
					break;
			}
		}
	}

	if (!strcmp(ecu_owner, "bin"))
	{
		switch (tgetopt("\
If you execute ecu with uid set to uucp lines, then uucp will be\n\
able to access any serial line owned by the user or owned by uucp.\n\
In addition, you need not provide for world-write access to the UUCP\n\
lock directory.  Answer 'n' if you are not sure.\n\
Do you wish to run ecu setuid to uucp? ", "yn", 'y'))
		{
			case 'y':
				ecu_owner = "uucp";
				ecu_group = "bin";
				ecu_mode = "4711";
				break;
			case 'n':
				break;
		}
	}

	printf("\nWhat do you want for a default tty? [%s]\n:", tty);
	tgets(s_cbuf);
	if (s_cbuf[0])
		tty = strdup(s_cbuf);

	printf("\nWhat do you want for a default bit rate? [%s]\n:", bit_rate);
	tgets(s_cbuf);
	if (s_cbuf[0])
		bit_rate = strdup(s_cbuf);

	parity = tgetopt("What do you want for default parity", "neo", 'n');

	printf(
		"\nWhere do you want the public executables placed? [%s]\n: ",
		bindir);
	tgets(s_cbuf);
	if (s_cbuf[0])
		bindir = strdup(s_cbuf);

	strcpy(s2048, bindir);
	if ((cp = strrchr(s2048, '/')) && !strncmp(cp + 1, "bin", 3))
	{
		*cp = 0;
		strcat(s2048, "/lib/ecu");
		libdir = strdup(s2048);
	}

	printf(
		"\nWhere do you want the ECU library placed? [%s]\n: ", libdir);
	tgets(s_cbuf);
	if (s_cbuf[0])
		libdir = strdup(s_cbuf);

	printf("\nHow many seconds should the built-in dialer ");
	printf("wait for carrier? [%d]\n: ", intdial_to);
	tgets(s_cbuf);
	if (s_cbuf[0])
		intdial_to = atoi(s_cbuf);

	itmp = 0;
	while (!itmp)
	{
		printf("\nWhat is the maximum number of screen lines (>= 24)? ");
		printf("[%d]\n: ", screen_lines);
		tgets(s_cbuf);
		if (s_cbuf[0] && (screen_lines = atoi(s_cbuf)) < 24)
			fputs("\7", stderr);
		else
			itmp = 1;
	}

	itmp = 0;
	while (!itmp)
	{
		printf("\nWhat is the maximum number of screen columns (>= 80)? ");
		printf("[%d]\n: ", screen_columns);
		tgets(s_cbuf);
		if (s_cbuf[0] && (screen_columns = atoi(s_cbuf)) < 80)
			fputs("\7", stderr);
		else
			itmp = 1;
	}

	printf("\nThank you.\n\n");
	find_stuff();

	makedir = makedirs;
	while (*makedir)
	{
		if (!strcmp(*makedir, "./ecuungetty") && (*use_ecuungetty == 'n'))
		{
			makedir++;
			continue;
		}
		generate_config(*makedir++);
	}

	/*
	 * check on our buddy timeval
	 */
	if (need_timeval && !has_timeval)
	{
		tputstrs(strs_no_timeval);
		goodbye(1);
	}

#ifdef WHT
	printf("\7Doing it anyway for WHT\n");
	sleep(2);
	for (itmp = 1; itmp < NSIG; itmp++)
		signal(itmp, SIG_DFL);
	system("make depend");
#endif

	goodbye(0);
}							 /* end of main */

/* vi: set tabstop=4 shiftwidth=4: */
/* end of config.c */
