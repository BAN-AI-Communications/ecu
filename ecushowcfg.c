/*+-------------------------------------------------------------------------
	ecushowcfg.c - show configuration
	wht@wht.net

  Defined functions:
	show_config()

--------------------------------------------------------------------------*/
/*+:EDITS:*/
/*:04-26-2000-11:15-wht@bob-RELEASE 4.42 */
/*:12-15-1997-19:34-wht@fep-rename CFG_FilioH to CFG_FionrdInFilioH */
/*:01-24-1997-02:37-wht@yuriatin-SOURCE RELEASE 4.00 */
/*:10-11-1996-02:09-wht@yuriatin-add CFG_FionreadInFilioH */
/*:10-11-1996-01:38-wht@yuriatin-update with later CFGs */
/*:09-11-1996-20:00-wht@yuriatin-3.48-major telnet,curses,structural overhaul */
/*:08-21-1996-16:43-wht@fep-creation */

#include "ecu.h"

/*+-------------------------------------------------------------------------
	show_config()
--------------------------------------------------------------------------*/
void
show_config()
{

	pprintf("Max screen geometry: %d cols x %d lines\n",
		CFG_ScreenColsMax, CFG_ScreenLinesMax);

	pprintf("Default tty: %s, %u bits/s (baud), %c parity\n",
		CFG_DefaultTty, CFG_DefaultBitRate, CFG_DefaultParity);

	pprintf("Internal dialer timeout: %d secs\n", CFG_DialTimeout);

	pprintf("ECU library (CFG_EcuLibDir): %s\n", CFG_EcuLibDir);
	pprintf("HDB library (CFG_HdbLibDir): %s\n", CFG_HdbLibDir);
	pprintf("Lock directory (CFG_LockDir): %s\n", CFG_LockDir);

#ifdef CFG_BinaryUucpPids
	pputs("CFG_BinaryUucpPids used, ");
#else
	pputs("CFG_BinaryUucpPids not used, ");
#endif

#ifdef CFG_FakeRename
	pputs("CFG_FakeRename used\n");
#else
	pputs("CFG_FakeRename not used\n");
#endif

#ifdef CFG_FionreadRdchk
	pputs("CFG_FionreadRdchk used, ");
#else
	pputs("CFG_FionreadRdchk not used, ");
#endif

#ifdef CFG_GettodFtime
	pputs("CFG_GettodFtime used\n");
#else
	pputs("CFG_GettodFtime not used\n");
#endif

#ifdef CFG_HasFdSet
	pputs("CFG_HasFdSet used, ");
#else
	pputs("CFG_HasFdSet not used, ");
#endif

#ifdef CFG_HasStrerror
	pputs("CFG_HasStrerror used\n");
#else
	pputs("CFG_HasStrerror not used\n");
#endif

#ifdef CFG_IncSelectH
	pputs("CFG_IncSelectH used, ");
#else
	pputs("CFG_IncSelectH not used, ");
#endif

#ifdef CFG_LogXfer
	pputs("CFG_LogXfer used\n");
#else
	pputs("CFG_LogXfer not used\n");
#endif

#ifdef CFG_Malloc3X
	pputs("CFG_Malloc3X used, ");
#else
	pputs("CFG_Malloc3X not used, ");
#endif

#ifdef CFG_MmapSHM
	pputs("CFG_MmapSHM used\n");
#else
	pputs("CFG_MmapSHM not used\n");
#endif

#ifdef CFG_NoAnsiEmulation
	pputs("CFG_NoAnsiEmulation used, ");
#else
	pputs("CFG_NoAnsiEmulation not used, ");
#endif

#ifdef CFG_SemWithShm
	pputs("CFG_SemWithShm used, ");
#else
	pputs("CFG_SemWithShm not used, ");
#endif

#ifdef CFG_SigType
	pputs("CFG_SigType used\n");
#else
	pputs("CFG_SigType not used\n");
#endif

#ifdef CFG_TelnetOption
	pputs("CFG_TelnetOption used, ");
#else
	pputs("CFG_TelnetOption not used, ");
#endif

#ifdef CFG_TermiosLineio
	pputs("CFG_TermiosLineio used\n");
#else
	pputs("CFG_TermiosLineio not used\n");
#endif

#ifdef CFG_UngettyAllLines
	pputs("CFG_UngettyAllLines used, ");
#else
	pputs("CFG_UngettyAllLines not used, ");
#endif

#ifdef CFG_UngettyChown
	pputs("CFG_UngettyChown used\n");
#else
	pputs("CFG_UngettyChown not used\n");
#endif

#ifdef CFG_UseEcuDial
	pputs("CFG_UseEcuDial used, ");
#else
	pputs("CFG_UseEcuDial not used, ");
#endif

#ifdef CFG_UseSeteuid
	pputs("CFG_UseSeteuid used\n");
#else
	pputs("CFG_UseSeteuid not used\n");
#endif

#ifdef CFG_UseUngetty
	pputs("CFG_UseUngetty used ");
#else
	pputs("CFG_UseUngetty not used ");
#endif
	pputs("\n");

#ifdef CFG_FionreadInFilioH
	pputs("CFG_FionreadInFilioH used ");
#else
	pputs("CFG_FionreadInFilioH not used ");
#endif

#ifdef CFG_FionreadInSocketH
	pputs("CFG_FionreadInFilioH used ");
#else
	pputs("CFG_FionreadInSocketH not used ");
#endif

#ifdef CFG_UseNcursesH
	pputs("CFG_UseNcursesH used ");
#else
	pputs("CFG_UseNcursesH not used ");
#endif

#ifdef CFG_UseNcursesNcursesH
	pputs("CFG_UseNcursesNcursesH used ");
#else
	pputs("CFG_UseNcursesNcursesH not used ");
#endif
	pputs("\n");

}							 /* end of show_config */

/* vi: set tabstop=4 shiftwidth=4: */
/* end of ecushowcfg.c */
