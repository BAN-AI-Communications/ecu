#------------------------------------------------------------
# Makefile for cmostime5 - MAC/NBS cmos/rtc programs
# wht@n4hgf.Mt-Park.GA.US
#------------------------------------------------------------
#+:EDITS:
#:01-24-1997-02:38-wht@yuriatin-SOURCE RELEASE 4.00
#:12-29-1992-11:15-root@n4hgf-add utc, nbs_refresh, bump rev
#:05-14-1992-03:41-wht@n4hgf-resurrect for ecu 3.20
#:01-28-1990-04:55-wht-working on cmostime version 4

#--> set CLK_TYPE to cmos if you have /dev/cmos, rtc if you have /dev/rtc
CLK_TYPE = cmos
#CLK_TYPE = rtc

#--> set TIME_SRC to MAC if you have the Heath Most Accurate Clock
#                 to NBS if you want to call NBS in DC
TIME_SRC = NBS
#TIME_SRC = MAC

#--> set TTY to the device you want to be the default for the MAC
#    or the Hayes-compatible modem for calling NBS
#    (if using under SCO, use the non-modem, lower-case tty file name)
TTY = /dev/tty2b
#TTY = /dev/tty00

#--> set UTC to -DUTC_CLK if you want your cmos clock kept in UTC
#    (a __MUST__ for UNIX 3.2.x); leave blank if you want the cmos clock
#    kept in local time (that's what I use for XENIX)
UTC = -DUTC_CLK -DWHT
#UTC = -DUTC_CLK
#UTC =

SHELL = /bin/sh

.SUFFIXES:
.SUFFIXES: .o .c .h  

C_LD_COMMON = -i -s
CFLAGS  = -O -K $(UTC) '-DDEFTTY="$(TTY)"' $(C_LD_COMMON)
LIB = -lx

NBSSETCLK_OFILES = \
	NBSsetclk.o\
	dev_$(CLK_TYPE).o\
	bcdcvt.o\
	date2epsec.o

MACSETCLK_OFILES = \
	MACsetclk.o\
	dev_$(CLK_TYPE).o\
	bcdcvt.o\
	date2epsec.o

CMOSSTIME_OFILES = \
	CMOSstime.o\
	dev_$(CLK_TYPE).o\
	bcdcvt.o\
	date2epsec.o

CMOS_DISP_OFILES = \
	cmos_disp.o\
	dev_cmos.o\
	bcdcvt.o

RTC_DISP_OFILES = \
	rtc_disp.o

S24H_DISP_OFILES = \
	set_24hr.o

UTC-LOGIN-OFILES = \
	utc_login.o

SHAR_FILES = \
	README\
	Makefile\
	MACsetclk.c\
	NBSsetclk.c\
	CMOSstime.c\
	cmos_disp.c\
	dev_cmos.c\
	dev_cmos.h\
	dev_rtc.c\
	dev_rtc.h\
	bcdcvt.c\
	date2epsec.c\
	set_24hr.c\
	utc.c\
	root.crontab\
	nbs_refresh\
	nbssetclk.ep

all: $(TIME_SRC)setclk CMOSstime $(CLK_TYPE)_disp utc

NBSsetclk: $(NBSSETCLK_OFILES)
	$(CC) $(C_LD_COMMON) $(NBSSETCLK_OFILES) $(LIB) -o $@

MACsetclk: $(MACSETCLK_OFILES)
	$(CC) $(C_LD_COMMON) $(MACSETCLK_OFILES) $(LIB) -o $@

CMOSstime: $(CMOSSTIME_OFILES)
	$(CC) $(C_LD_COMMON) $(CMOSSTIME_OFILES) $(LIB) -o $@

cmos_disp: $(CMOS_DISP_OFILES)
	$(CC) $(C_LD_COMMON) $(CMOS_DISP_OFILES) $(LIB) -o $@

rtc_disp: $(RTC_DISP_OFILES)
	$(CC) $(C_LD_COMMON) $(RTC_DISP_OFILES) $(LIB) -o $@

set_24hr: $(S24H_DISP_OFILES)
	$(CC) $(C_LD_COMMON) $(S24H_DISP_OFILES) $(LIB) -o $@

shar:
	ls -l $(SHAR_FILES) > MANIFEST
	shar -c -ncmostime5 -l30 -ocmostime5 MANIFEST $(SHAR_FILES)

clean:
	-rm -f *.o

clobber: clean
	rm -f NBSsetclk MACsetclk CMOSstime cmos_disp rtc_disp set_24hr

