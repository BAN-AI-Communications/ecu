/*+-------------------------------------------------------------------------
	ecucmd.h -- command definitions
	wht@wht.net
--------------------------------------------------------------------------*/
/*+:EDITS:*/
/*:04-26-2000-11:15-wht@bob-RELEASE 4.42 */
/*:03-31-1998-21:50-wht@kepler-add CFG_TelnetServer support */
/*:11-03-1997-02:10-wht@kepler-3.08a-option command */
/*:01-25-1997-13:31-wht@yuriatin-add fflush */
/*:01-24-1997-02:37-wht@yuriatin-SOURCE RELEASE 4.00 */
/*:10-16-1996-03:32-wht@yuriatin-add kill,fork procedure command */
/*:10-09-1996-21:00-wht@yuriatin-add whilez/whilenz */
/*:09-18-1996-06:25-wht@yuriatin-CTayt Are You There added */
/*:09-11-1996-20:00-wht@yuriatin-3.48-major telnet,curses,structural overhaul */
/*:11-23-1995-11:20-wht@kepler-source control 3.37 for tsx-11 */
/*:11-14-1995-10:23-wht@kepler-3.37.80-source control point: SOCKETS */
/*:11-12-1995-00:02-wht@gyro-add telopt and ansif */
/*:10-14-1995-23:22-wht@kepler-drop SEAlink support */
/*:09-16-1995-16:32-root@kepler-add td - tcap display command */
/*:05-09-1995-17:23-wht@kepler-add attrtest */
/*:04-02-1995-04:48-wht@n4hgf-added sgrto1 and sgrto2 */
/*:03-21-1995-14:59-wht@n4hgf-add erto and erverbose */
/*:05-04-1994-04:38-wht@n4hgf-ECU release 3.30 */
/*:08-07-1993-11:03-wht@n4hgf-resurrect mkdir cmd */
/*:03-01-1993-03:53-wht@n4hgf-include conxout in interactive help */
/*:12-24-1992-13:57-wht@n4hgf-add eeod */
/*:10-18-1992-14:26-wht@n4hgf-add conxon */
/*:09-10-1992-13:58-wht@n4hgf-ECU release 3.20 */
/*:08-30-1992-23:06-wht@n4hgf-add fkmap */
/*:08-22-1992-15:38-wht@n4hgf-ECU release 3.20 BETA */
/*:04-19-1992-19:54-wht@n4hgf-kbdtest command now visible to users */
/*:03-27-1992-16:21-wht@n4hgf-re-include protection for all .h files */
/*:03-01-1992-13:28-wht@n4hgf-come up to modern times ... enum for CT */
/*:11-16-1991-14:34-wht@n4hgf-add upon + rearrance pcmd_... decls */
/*:11-11-1991-14:33-wht@n4hgf-add dcdwatch */
/*:09-01-1991-18:12-wht@n4hgf2-add setline */
/*:09-01-1991-18:11-wht@n4hgf2-add setline */
/*:08-17-1991-16:41-wht@n4hgf-add kbdtest */
/*:07-29-1991-17:57-wht@n4hgf-add memstat */
/*:07-25-1991-12:55-wht@n4hgf-ECU release 3.10 */
/*:07-04-1991-20:07-wht@n4hgf-add procedure rlog cmd */
/*:05-21-1991-18:07-wht@n4hgf-add pushd/popd commands */
/*:03-20-1991-05:25-root@n4hgf-experimental eto command */
/*:03-16-1991-15:24-wht@n4hgf-add nice */
/*:08-14-1990-20:40-wht@n4hgf-ecu3.00-flush old edit history */

#ifndef _ecucmd_h
#define _ecucmd_h

/* interactive command tokens */

enum CT_codes
{
	CTRSVD = 15,			 /* codes 0-15 reserved */
	CTansif,
	CTattrtest,
	CTautorz,
	CTax,
	CTayt,
	CTbaud,
	CTbn,
	CTbreak,
	CTcd,
	CTclrx,
	CTconxon,
	CTda,
	CTdcdwatch,
	CTdial,
	CTdo,
	CTduplex,
	CTerto,
	CTerverbose,
	CTeto,
	CTexit,
	CTfasi,
	CTfi,
	CTfkey,
	CTfkmap,
	CTgetf,
	CThangup,
	CThelp,
	CTkbdtest,
	CTllp,
	CTloff,
	CTlog,
	CTmemstat,
	CTmkdir,
	CTnice,
	CTnl,
	CTnlin,
	CTnlout,
	CToa,
	CTparity,
	CTpcmd,
	CTpid,
	CTplog,
	CTpopd,
	CTptrace,
	CTpushd,
	CTputf,
	CTpwd,
	CTredial,
	CTrev,
	CTrk,
	CTrtscts,
	CTrx,
	CTry,
	CTrz,
	CTsdname,
	CTsgr,
	CTsgrto1,
	CTsgrto2,
	CTsk,
	CTstat,
	CTsx,
	CTsy,
	CTsz,
	CTtime,
	CTtd,
	CTtelopt,
	CTts,
	CTtty,
	CTxa,
	CTxlog,
	CTxon,
#if defined(CFG_TelnetServer)
	CTserve,
	CTservewire,
	CTsockserve,
	CTsockclose,
#endif
	CT____end,

	CTdummy1 = 120,
	CTdummy2,
	CTdummy3,
	CTdummy4
};

#define TOKEN_QUAN	128		 /* for help package */

/*
 * if compiling helpgen, we need dummy functions for linker
 * to resolve though they are never called; when compiling ecu,
 * we just declare them as integer functions
 */
#if defined(HELPGEN)
#define S static
#define BODY {return(-1);}
#else
#define S
#define BODY ;
#endif

S int
pcmd_ansif() BODY
S int
pcmd_autorz() BODY
S int
pcmd_ayt() BODY
S int
pcmd_baud() BODY
S int
pcmd_break() BODY
S int
pcmd_cd() BODY
S int
pcmd_clrx() BODY
S int
pcmd_cls() BODY
S int
pcmd_color() BODY
S int
pcmd_continue() BODY
S int
pcmd_conxon() BODY
S int
pcmd_cursor() BODY
S int
pcmd_dcdwatch() BODY
S int
pcmd_delline() BODY
S int
pcmd_dial() BODY
S int
pcmd_do() BODY
S int
pcmd_duplex() BODY
S int
pcmd_echo() BODY
S int
pcmd_eeod() BODY
S int
pcmd_eeol() BODY
S int
pcmd_else() BODY
S int
pcmd_erto() BODY
S int
pcmd_erverbose() BODY
S int
pcmd_exec() BODY
S int
pcmd_exit() BODY
S int
pcmd_expresp() BODY
S int
pcmd_getf() BODY
#if	defined(FASI)
S int
pcmd_fasi() BODY
#endif
S int
pcmd_fchmod() BODY
S int
pcmd_fclose() BODY
S int
pcmd_fflush() BODY
S int
pcmd_fgetc() BODY
S int
pcmd_fgets() BODY
S int
pcmd_fkey() BODY
S int
pcmd_fkmap() BODY
S int
pcmd_flush() BODY
S int
pcmd_fopen() BODY
S int
pcmd_fork() BODY
S int
pcmd_fputc() BODY
S int
pcmd_fputs() BODY
S int
pcmd_fread() BODY
S int
pcmd_fseek() BODY
S int
pcmd_fdel() BODY
S int
pcmd_fwrite() BODY
S int
pcmd_gosub() BODY
S int
pcmd_gosubb() BODY
S int
pcmd_goto() BODY
S int
pcmd_gotob() BODY
S int
pcmd_hangup() BODY
S int
pcmd_hexdump() BODY
S int
pcmd_home() BODY
S int
pcmd_icolor() BODY
S int
pcmd_insline() BODY
S int
pcmd_ifge() BODY
S int
pcmd_ifgt() BODY
S int
pcmd_ifi() BODY
S int
pcmd_ifle() BODY
S int
pcmd_iflt() BODY
S int
pcmd_ifnz() BODY
S int
pcmd_ifs() BODY
S int
pcmd_ifz() BODY
S int
pcmd_kill() BODY
S int
pcmd_lbreak() BODY
S int
pcmd_lgets() BODY
S int
pcmd_logevent() BODY
S int
pcmd_lookfor() BODY
S int
pcmd_mkdir() BODY
S int
pcmd_mkvar() BODY
S int
pcmd_nap() BODY
S int
pcmd_nice() BODY
int
pcmd_option() BODY
S int
pcmd_parity() BODY
S int
pcmd_pclose() BODY
S int
pcmd_plog() BODY
S int
pcmd_popd() BODY
S int
pcmd_popen() BODY
S int
pcmd_prompt() BODY
S int
pcmd_ptrace() BODY
S int
pcmd_pushd() BODY
S int
pcmd_putf() BODY
S int
pcmd_return() BODY
S int
pcmd_rk() BODY
S int
pcmd_rlog() BODY
S int
pcmd_rname() BODY
S int
pcmd_rtscts() BODY
S int
pcmd_rx() BODY
S int
pcmd_ry() BODY
S int
pcmd_rz() BODY
S int
pcmd_scrdump() BODY
S int
pcmd_send() BODY
S int
pcmd_set() BODY
S int
pcmd_setline() BODY
S int
pcmd_sk() BODY
S int
pcmd_sx() BODY
S int
pcmd_sy() BODY
S int
pcmd_system() BODY
S int
pcmd_sz() BODY
S int
pcmd_telopt() BODY
S int
pcmd_upon() BODY
S int
pcmd_vidcolor() BODY
S int
pcmd_vidnorm() BODY
S int
pcmd_vidrev() BODY
S int
pcmd_whilei() BODY
S int
pcmd_whilenz() BODY
S int
pcmd_whiles() BODY
S int
pcmd_whilez() BODY
S int
pcmd_xon() BODY

#if defined(CFG_TelnetServer)
S int pcmd_serve() BODY
S int pcmd_servewire() BODY
S int pcmd_sockserve() BODY
S int pcmd_sockclose() BODY
#endif

/* command classification */
#define ccG  1				 /* general command */
#define ccC  2				 /* comm command */
#define ccT  3				 /* transfer command */
#define ccP  4				 /* procedure-related command */

typedef struct p_cmd
{
	char *cmd;				 /* command string */
	short min_ch;			 /* min chars for match (0 if not interactive) */
	short token;			 /* command number (if interactive) */
	char *descr;			 /* command description (if interactive) */
	PFI proc;				 /* procedure cmd handler (or 0) */
	short cmdclass;			 /* cc{C,G,P,X} or 0 (for help processor) */
}
P_CMD;

#if !defined(DECLARE_P_CMD)
#if defined(NEED_P_CMD)
extern P_CMD icmd_cmds[];

#endif
#else
P_CMD icmd_cmds[] =
{
	{"ansif", 4, CTansif, "ANSI filter state", pcmd_ansif, ccG},
	{"ax", 2, CTax, "ascii char to hex/oct/dec", 0, ccG},
	{"ayt", 2, CTayt, "send telnet Are You There?", pcmd_ayt, ccC},
	{"attrtest", 5, CTattrtest, "console attribute test", 0, ccG},
	{"autorz", 6, CTautorz, "auto ZMODEM receive state", pcmd_autorz, ccT},
	{"baud", 2, CTbaud, "set/display line bit rate", pcmd_baud, ccC},
	{"bn", 2, CTbn, "all console event alarm", 0, ccG},
	{"break", 2, CTbreak, "send break to remote", pcmd_break, ccC},
	{"cd", 2, CTcd, "change current directory", pcmd_cd, ccG},
	{"clrx", 2, CTclrx, "clear local transmit XOFF", pcmd_clrx, ccC},
	{"cls", 0, 0, "", pcmd_cls, 0},
	{"color", 0, 0, "", pcmd_color, 0},
	{"continue", 0, 0, "", pcmd_continue, 0},
	{"conxon", 4, CTconxon, "console software flow control", pcmd_conxon, ccG},
	{"cursor", 0, 0, "", pcmd_cursor, 0},
	{"da", 2, CTda, "decimal to ascii char", 0, ccG},
	{"dcdwatch", 3, CTdcdwatch, "control DCD disconnect", pcmd_dcdwatch, ccC},
	{"dial", 1, CTdial, "dial remote destination", pcmd_dial, ccC},
	{"delline", 0, 0, "", pcmd_delline, 0},
	{"do", 2, CTdo, "perform procedure", pcmd_do, ccP},
	{"duplex", 2, CTduplex, "set/display duplex", pcmd_duplex, ccC},
	{"echo", 0, 0, "", pcmd_echo, 0},
	{"eeod", 0, 0, "", pcmd_eeod, 0},
	{"eeol", 0, 0, "", pcmd_eeol, 0},
	{"else", 0, 0, "", pcmd_else, 0},
	{"erto", 3, CTerto, "expect-respond timeout", pcmd_erto, ccC},
	{"erverbose", 3, CTerverbose, "expect-respond verbosity",
		pcmd_erverbose, ccC},
	{"eto", 3, CTeto, "ESC/fkey timeout", 0, ccG},
	{"exec", 0, 0, "", pcmd_exec, 0},
	{"exit", 2, CTexit, "hang up, exit program", pcmd_exit, ccG},
	{"expresp", 0, 0, "", pcmd_expresp, 0},
#if	defined(FASI)
	{"fasi", 2, CTfasi, "FAS/i driver control", pcmd_fasi, ccC},
#endif
	{"fchmod", 0, 0, "", pcmd_fchmod, 0},
	{"fclose", 0, 0, "", pcmd_fclose, 0},
	{"fdel", 0, 0, "", pcmd_fdel, 0},
	{"fflush", 0, 0, "", pcmd_fflush, 0},
	{"fgetc", 0, 0, "", pcmd_fgetc, 0},
	{"fgets", 0, 0, "", pcmd_fgets, 0},
	{"fi", 2, CTfi, "send text file to line", 0, ccG},
	{"fkey", 3, CTfkey, "function key definition", pcmd_fkey, ccG},
	{"fkmap", 3, CTfkmap, "redefine function key map", pcmd_fkmap, ccG},
	{"flush", 0, 0, "", pcmd_flush, 0},
	{"fopen", 0, 0, "", pcmd_fopen, 0},
	{"fork", 0, 0, "", pcmd_fork, 0},
	{"fputc", 0, 0, "", pcmd_fputc, 0},
	{"fputs", 0, 0, "", pcmd_fputs, 0},
	{"fread", 0, 0, "", pcmd_fread, 0},
	{"fseek", 0, 0, "", pcmd_fseek, 0},
	{"fwrite", 0, 0, "", pcmd_fwrite, 0},
	{"getf", 0, 0, "", pcmd_getf, 0},
	{"gosub", 0, 0, "", pcmd_gosub, 0},
	{"gosubb", 0, 0, "", pcmd_gosubb, 0},
	{"goto", 0, 0, "", pcmd_goto, 0},
	{"gotob", 0, 0, "", pcmd_gotob, 0},
	{"hangup", 2, CThangup, "hang up modem", pcmd_hangup, ccC},
	{"help", 2, CThelp, "invoke help", 0, ccG},
	{"hexdump", 0, 0, "", pcmd_hexdump, 0},
	{"home", 0, 0, "", pcmd_home, 0},
	{"icolor", 0, 0, "", pcmd_icolor, 0},
	{"ifge", 0, 0, "", pcmd_ifge, 0},
	{"ifgt", 0, 0, "", pcmd_ifgt, 0},
	{"ifi", 0, 0, "", pcmd_ifi, 0},
	{"ifle", 0, 0, "", pcmd_ifle, 0},
	{"iflt", 0, 0, "", pcmd_iflt, 0},
	{"ifnz", 0, 0, "", pcmd_ifnz, 0},
	{"ifs", 0, 0, "", pcmd_ifs, 0},
	{"ifz", 0, 0, "", pcmd_ifz, 0},
	{"insline", 0, 0, "", pcmd_insline, 0},
	{"kbdtest", 4, CTkbdtest, "test keyboard mapping", 0, ccG},
	{"kill", 0, 0, "", pcmd_kill, 0},
	{"lbreak", 0, 0, "", pcmd_lbreak, 0},
	{"llp", 2, CTllp, "set session log to /dev/lp", 0, ccG},
	{"lgets", 0, 0, "", pcmd_lgets, 0},
	{"loff", 3, CTloff, "turn off session logging", 0, ccG},
	{"log", 3, CTlog, "session logging control", 0, ccG},
	{"logevent", 0, 0, "", pcmd_logevent, 0},
	{"lookfor", 0, 0, "", pcmd_lookfor, 0},
#if defined(CFG_Malloc3X)
	{"memstat", 3, CTmemstat, "", 0, 0},
#endif
	{"mkdir", 3, CTmkdir, "mkdir <dirname>", pcmd_mkdir, ccG},
	{"mkvar", 0, 0, "", pcmd_mkvar, 0},
	{"nap", 0, 0, "", pcmd_nap, 0},
	{"nice", 2, CTnice, "change process nice (0-39)", pcmd_nice, 0},
	{"nl", 2, CTnl, "display CR/LF mapping", 0, ccC},
	{"nlin", 3, CTnlin, "receive CR/LF mapping", 0, ccC},
	{"nlout", 3, CTnlout, "transmit CR/LF mapping", 0, ccC},
	{"oa", 2, CToa, "octal to ascii char", 0, ccG},
	{"option", 0, 0, "", pcmd_option, ccG},
	{"parity", 3, CTparity, "set/display line parity", pcmd_parity, ccC},
	{"pclose", 0, 0, "", pcmd_pclose, 0},
	{"pcmd", 2, CTpcmd, "execute procedure command", 0, ccP},
	{"pid", 2, CTpid, "display process ids", 0, ccG},
	{"plog", 2, CTplog, "control procedure logging", pcmd_plog, ccP},
	{"popd", 2, CTpopd, "pop to previous directory", pcmd_popd, ccG},
	{"popen", 0, 0, "", pcmd_popen, 0},
	{"prompt", 0, 0, "", pcmd_prompt, 0},
	{"ptrace", 2, CTptrace, "control procedure trace", pcmd_ptrace, ccP},
	{"pushd", 2, CTpushd, "push to new directory", pcmd_pushd, ccG},
	{"putf", 0, 0, "", pcmd_putf, 0},
	{"pwd", 2, CTpwd, "print working directory", 0, ccG},
	{"redial", 3, CTredial, "redial last number", 0, ccC},
	{"return", 0, 0, "", pcmd_return, 0},
	{"rev", 3, CTrev, "ecu revision/make date", 0, ccG},
	{"rk", 2, CTrk, "receive via C-Kermit", pcmd_rk, ccT},
	{"rlog", 0, 0, "", pcmd_rlog, 0},
	{"rname", 0, 0, "", pcmd_rname, 0},
	{"rtscts", 3, CTrtscts, "RTS/CTS flow control", pcmd_rtscts, ccC},
	{"rx", 2, CTrx, "receive via XMODEM/CRC", pcmd_rx, ccT},
	{"ry", 2, CTry, "receive via YMODEM Batch", pcmd_ry, ccT},
	{"rz", 2, CTrz, "receive via ZMODEM/CRC32", pcmd_rz, ccT},
	{"scrdump", 0, 0, "", pcmd_scrdump, 0},
	{"sdname", 3, CTsdname, "select screen dump name", 0, ccG},
	{"send", 0, 0, "", pcmd_send, 0},
#if defined(CFG_TelnetServer)
	{"serve", 0, CTserve, "", pcmd_serve, 0},
	{"servewire", 0, CTservewire, "", pcmd_servewire, 0},
	{"sockserve", 0, CTsockserve, "", pcmd_sockserve, 0},
	{"sockclose", 0, CTsockclose, "", pcmd_sockclose, 0},
#endif
	{"set", 0, 0, "", pcmd_set, 0},
	{"setline", 0, 0, "", pcmd_setline, 0},
	{"sgr", 3, CTsgr, "send command/get response", 0, ccC},
	{"sgrto1", 6, CTsgrto1, "set SGr 1st char timeout", 0, ccC},
	{"sgrto2", 6, CTsgrto2, "set SGr later char timeout", 0, ccC},
	{"sk", 2, CTsk, "send via C-Kermit", pcmd_sk, ccT},
	{"stat", 2, CTstat, "general status", 0, ccG},
	{"sx", 2, CTsx, "send via XMODEM/CRC", pcmd_sx, ccT},
	{"sy", 2, CTsy, "send via YMODEM Batch", pcmd_sy, ccT},
	{"system", 0, CTsy, "", pcmd_system, 0},
	{"sz", 2, CTsz, "send via ZMODEM/CRC32", pcmd_sz, ccT},
	{"telopt", 3, CTtelopt, "telnet options display state", pcmd_telopt, ccC},
	{"time", 2, CTtime, "time of day", 0, ccG},
	{"td", 2, CTtd, "termcap variable display", 0, ccG},
	{"ts", 2, CTts, "termio display", 0, ccC},
	{"tty", 2, CTtty, "console tty name", 0, ccG},
	{"upon", 0, 0, "", pcmd_upon, 0},
	{"vidcolor", 0, 0, "", pcmd_vidcolor, 0},
	{"vidnorm", 0, 0, "", pcmd_vidnorm, 0},
	{"vidrev", 0, 0, "", pcmd_vidrev, 0},
	{"whilei", 0, 0, "", pcmd_whilei, 0},
	{"whilenz", 0, 0, "", pcmd_whilenz, 0},
	{"whiles", 0, 0, "", pcmd_whiles, 0},
	{"whilez", 0, 0, "", pcmd_whilez, 0},
	{"xa", 2, CTxa, "hex to ascii char", 0, ccG},
	{"xlog", 2, CTxlog, "protocol packet logging", 0, ccT},
	{"xon", 2, CTxon, "line xon/xoff flow control", pcmd_xon, ccC},
/*
 * These cmds are interecepted by special code in ecucmd.h and appear
 * here only so they will be picked up by the help system.
 */
	{"!", 1, CTdummy1, "execute shell (tty)", 0, ccG},
	{"$", 1, CTdummy2, "execute shell (comm line)", 0, ccG},
	{"-", 1, CTdummy3, "execute program", 0, ccG},
	{"?", 1, CTdummy4, "get help", 0, ccG},
	{"", 0, -1, "", 0, 0}	 /* list ends with token value of -1 */
};
#endif

#endif /* _ecucmd_h */

/* end of ecucmd.h */
/* vi: set tabstop=4 shiftwidth=4: */
