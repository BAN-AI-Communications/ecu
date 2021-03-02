# FAS 2.08/FASI 1.00 Makefile for SCO UNIX SYS V/386
#+:EDITS:
#:01-24-1997-02:37-wht@yuriatin-SOURCE RELEASE 4.00
#:09-11-1996-20:00-wht@yuriatin-3.48-major telnet,curses,structural overhaul
#:11-23-1995-11:20-wht@kepler-source control 3.37 for tsx-11
#:11-14-1995-10:23-wht@kepler-3.37.80-source control point: SOCKETS
#:05-04-1994-04:39-wht@n4hgf-ECU release 3.30
#:09-10-1992-13:59-wht@n4hgf-ECU release 3.20
#:08-22-1992-15:38-wht@n4hgf-ECU release 3.20 BETA
#:01-17-1991-01:38-wht@n4hgf-adapt FAS 2.08 SCO make file

SHELL = /bin/sh
INCLLOC = /usr/include/local
LKDRVRDIR = /etc/conf/pack.d/fas
LKSCONFDIR = /etc/conf/sdevice.d
LKNCONFDIR = /etc/conf/node.d
LKICONFDIR = /etc/conf/init.d
LKKCONFDIR = /etc/conf/kconfig.d
DRVRNAME = Driver.o
CONFNAME = fas

CC = rcc
CFLAGS = -O -DINKERNEL -DSCO -DNEED_PUT_GETCHAR -DFASI

OBJS = fas.o

fas.o: 	fas.c $(INCLLOC)/fas.h $(INCLLOC)/digi-pc8.h Makefile

Driver.o: fas.o
	rm -rf Driver.o
	ln fas.o Driver.o

install: Driver.o
	sh -v idfasi

install2: install
	cd /etc/conf/cf.d;./link_unix

space.c:
	@echo "You must copy the proper space-xxxxx file to \`space.c'"
	@false
	
s_$(CONFNAME):
	@echo "You must copy the proper s_$(CONFNAME)-xxxxx file to \`s_$(CONFNAME)'"
	@false
		
n_$(CONFNAME):
	@echo "You must copy the proper n_$(CONFNAME)-xxxxx file to \`n_$(CONFNAME)'"
	@false
		
i_$(CONFNAME):
	@echo "You must copy the proper i_$(CONFNAME)-xxxxx file to \`i_$(CONFNAME)'"
	@false
		
$(INCLLOC)/fas.h: fas.h
	-mkdir $(INCLLOC) >/dev/null 2>&1
	cp fas.h $(INCLLOC)/fas.h

$(INCLLOC)/digi-pc8.h: digi-pc8.h
	-mkdir $(INCLLOC) >/dev/null 2>&1
	cp digi-pc8.h $(INCLLOC)/digi-pc8.h

clean:
	rm -f fas.o Driver.o

clobber: clean

