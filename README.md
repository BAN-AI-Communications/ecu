# ECU

[![Maintainability](https://api.codeclimate.com/v1/badges/4f384a777370a1f7f820/maintainability)](https://codeclimate.com/github/BAN-AI-Communications/go-hdlc/maintainability)

## Extended Call Utility

---

```text
.---------------------------------------------------------.
| ECU README - last revised Wed Sep  2 13:52:15 EDT 1998  |
`---------------------------------------------------------'

ECU is a asynchronous serial and telnet communications program
for these environments:

  SCO XENIX System V/286          Support has been dropped; ECU
                                  is now too large. Try versions
                                  prior to 3.30.

  SCO XENIX System V/386          ECU is stable on SCO XENIX/386
                                  (telnet support attempted
                                  if your DS has inet support)

  SCO UNIX System V/386           ECU is very robust on SCO UNIX,
  SCO ODT 1.x,2.0,3.0             ODT and OS5 (telnet support works
  SCO Open Server 5               if your DS has inet support)
  SCO UnixWare 2.1

  SunOS 4.1.x                     a robust, stable, limited subset
                                  (telnet support automatic)

  ISC 386/ix 2.2 or later         Ports to these systems are
  ISC System V Release 4          not supported as regularly
  ESIX System V Release 4         and I cannot vouch for
                                  them at time of release
                                  PLEASE GIVE ME FEEDBACK!

  Linux 1.1.58                    tested well enough under RedHat 5
    or later                      (telnet support automatic)

  HP-UX 9.01                      reasonably functional; a few
                                  rough edges with keyboards and
                                  modem control signals

  Motorola Delta [68]8K SVR40     stable; some testing completed
                                  (telnet support automatic)

  Motorola Delta [68]8K SVR32     compiles OK with Green Hills and GCC;
                                  (now known to NOT work)
                                  no machine to debug upon

  NetBSD                          Still under development;
                                  ANSI emulation disabled until
                                  work on termcap support is complete

  Solaris 2.x                     compiles; some testing done; needs more

  FreeBSD                         very reliable and full-featured port

  AIX                             Motorola's version anyway; compiles OK
                                  but untested

-----------------------------------------------------------------------
ECU 4.0 is released in the month of ECU's eleventh anniversary!!!!
ECU 4.12 Thu Dec 18 06:11:05 EST 1997
ECU 4.30 Wed Sep  2 13:52:48 EDT 1998
-----------------------------------------------------------------------

ECU (Extended Call Utility) is a research and engineering
communications program originally written for users of SCO UNIX
V.3.2/386 and XENIX V on 80286 and 80386 systems.  Support for
other systems has been added and further porting is possible with
"minor" effort to other systems based on or similar to UNIX
System V (see README.porting).

ECU provides the classic terminal communications facility of
passing keyboard data to a serial line and incoming data to the
computer video display.  In addition, a dialing directory, a
function key mapping feature, session logging, and other
basic features are available.

ECU presents to the host a flexible "ANSI" terminal type,
accepting any valid video control sequences from MS-DOS or SCO
documentation as of late 1990.  It also fares well, though
imperfectly, with Sun and VT-100 video control sequences.
Standards are great: everybody should have one, especially if
they call it "ANSI". For more information, refer to the manual
section titled "ANSI Filter."  (NOTE: the porter to BSD did
not engineer the ANSI terminal emulation. Therefore the incoming
data is passed directly to your native screen. This may be
repaired in a future release.)  As of ECU 3.37, the ANSI filter
may be turned on and off.

ECU supports numerous file transfer protocols: as of this
writing, XMODEM, XMODEM/CRC, XMODEM-1K, YMODEM/CRC Batch,
ZMODEM/CRC-16, ZMODEM/CRC-32, and C-Kermit 5 are
supported.  For more information, refer to the manual sections
describing the individual interactive and procedure file transfer
commands. (Usage of serial file transfer protocols over telnet/inet
is not supported.)

A very flexible procedure (script) language is also incorporated
to automate many communications tasks.  In addition to augmenting
interactive tasks, by using shell scripts and ECU procedures, ECU
can perform batch-style communications sessions in an entirely
"unattended" fashion.

For applications too unwieldy for the procedure language,
"ecufriend" programs are supported.  Friends are spawned by ECU
having access to the shared memory segment containing an
ECU-managed "screen image" and other data and having use of the
attached communications line.

The doc subdirectory has all of the .txt files used to produce
ecu.man, the manual of sorts for the program.  A copy of it is
reluctantly included (net.bandwidth) for those who do not have
my squirrely 1985 Xenix/286 nroff.  (Gotta rework that doc someday.)

*Please* take the time to read the (tedious) manuals and READMEs.
This will do me honor and yourself justice because there are a
lot of goodies in here, many of which are not traditional
features you'll be looking for.

-------------------------------------------------------------------------

RIGHTS TO USE

This program, it sources, objects and utilities are in the public
domain.

Alas, placing "popular" programs in the public domain can cause
peculiar problems.  As others adopt, modify and redistribute the
code, it tends to fall apart.  I regularly get mail from folks
who have found fragmented or damaged buckets of code with my
e-mail address in it.  Invariably, it has come from some off-net
BBS where someone has tried to PKZIP it or something :).

Your license to redistribute this version of ECU is, of course,
unlimited.  Even though I'll spend hours poring over patches
trying to help a user, my patience with folks who modify my code
without CLEARLY marking it so is at end.

These are my requests:

1.  If you use or redistribute this program, it should retain the
ECU name unless you ask me.  This means the base name of the
program executable must be "ecu" and the program must announce
itself with the strings "ECU" and "wht@n4hgf".

2.  If you modify the program "substantially" you should not
redistribute it without asking me.  For purposes of this
paragraph, "substantially" refers to changes of more than a
bug-fix or cosmetic nature.  For larger changes, read
README.porting and, if appropriate, submit patches to
wht@wht.net.  If they "pass muster" (we are usually
easy), then the changes will be incorporated into a released
patch level of ECU.

You can give modified versions to a associate, but if s/he gives
it to someone else, does the fourth party know not to contact me
for bug fixes?

3.  You are still free to use substantial code fragments or
entire logical subsections of the program in any way you see fit,
obeying of course any other copyrights and requests made by other
authors whose code fragments appear within ECU.

4.  Anybody who has contributed to ECU development will find it
awfully easy to get my permission to do anything they want with
ECU ("Noooooooooooooooooo ...  not THAT!")

--------------------------------------------------------------------

ACKNOWLEDGMENTS

MANY THANKS to those who helped me improve the program, especially
upaya!tbetz, ache@hq.demos.su (now ache@astral.msk.su), spel@hippo.ru.ac.za,
bel@trout.nosc.mil, dhmadsen@icaen.uiowa.edu, dug@kd4nc.atl.ga.us,
jts@ki4xo, jsm@n4vu.atl.ga.us, lamy@glsys.in-berlin.de, cma@tridom,
tabbs!aris, neal@clkwrka, extel@quagga.ru.ac.za,
mjb@mjbtn, tmcsys.uucp!lothar, mju@mudos.ann-arbor.mi.us
elastic!fche, genrad!rob and spooley@compulink.co.uk.  There were
lots of others and I know I've forgotten someone who helped a
great deal; I apologize.

Very special thanks go to Dion Johnson and Bob Lewis at SCO for
their untiring and generous support.

Lothar Hirschbiegel <aega84!lh> did the ISC SVR4 port -- THANKS,
Lothar!

Joseph H Buehler <jhpb@sarto.budd-lake.nj.us> extended the SVR4
port to ESIX -- THANKS, Joesph!

Bob Lewis <robertle@sco.com> proofread the manual (as of 3.30 --
don't blame them for errors creeping in since then <smile>).
This is tedious work and special thanks go to them.

Robert Lipe has contributed greatly to ECU in various esoteric areas
(wild signals and tty driver dependencies particularly).  Also,
I do not believe there ever has been an alpha revision Robert has
not compiled and tested at least a bit (usually thoroughly).
Robert also did early stabs at the ports to Solaris 2.x.
Thanks, Robert!

Robert "Bob" Broughton ported ECU to LINUX with the help of
Toomas Losin <a776@mindlink.bc.ca>.  THANKS, Bob and Toomas!

Carl Wuebker ported ECU to HP-UX.  THANKS, Carl!

Daniel Harris ported ECU to NetBSD and helped settle POSIX
termios/SYSV termio issues.  THANKS, Daniel!

Andrey Chernov <ache@nagual.ru> (formerly <ache@astral.msk.su>)
ported ECU to FreeBSD.  His port was not only flawless and
seamless, but exquisite.  All I had to do was patch -p0 and make.
He was patient with the minor fractures I made in pursuing my own
agenda in parallel.  (Parallel source development across
timezones is difficult.  Corollary: Linux is a miracle.)

Don Yuniskis <dgy@rtd.com> also contributed to solidifying
FreeBSD support.

Robert Lipe, Bob Friesenhahn and Roger Fujii all
submitted input and patches for Solaris 2.x .

Thanks go to Pat Davitt for checking the guy out under
XENIX/386 2.3.1.

-------------------------------------------------------------------------

CHEST THUMPING

There is a reason for all the edit notes and funky comment
formats in ECU.  As a bonus for ECU's 11th anniversary, docxtr.c
and edlists.c should amuse True Hackers(TM) at least momentarily.
These tools have often 1) amused me when I wanted to burn up some
time on a new CPU, 2) unconfused me when I had to look at ancient
code, and 3) shown me how much of my life I've thrown away at the
keyboard.

cd ecu-src-dir/
cc -o docxtr docxtr.c
cc -o edlists edlists.c
docxtr *.c | more-or-less
edlists *.c | sort | more-or-less
edlists -a *.c | sort | more-or-less

In 1985, these commands were BATCH JOBS (redirect to a file on a
55 msec disk, unloose an EIGHT MHz (wow!) CPU, and go watch a movie).
-------------------------------------------------------------------------

CORRESPONDENCE

NOTE: All correspondence to the author regarding ECU must be sent
to wht@wht.net.  One exception: commercial
users wishing support or custom work, please contact me at the
phone number below.

Warren H. Tucker                      wht@wht.net
150 West Lake Drive    Roswell, GA 30075-1153 USA
+1 (770) 587-5766
```
