SHELL=/bin/sh
CFLAGS= -Octl 
HDR_DIR = /usr/include/local

all: Driver.o testmorse usemorse

Driver.o: mors.c
	rm -f Driver.o
	cc -c -Octl -Gs -DM_KERNEL -DCMERGE -DINTRINSICS -DM_S_UNIX -DINKERNEL  mors.c
	ln mors.o Driver.o

debug:
	$(CC) -E $(CFLAGS) mors.c

install: Driver.o
	-mkdir $(HDR_DIR)
	-rm -rf $(HDR_DIR)/morse_dvr.h
	cp morse_dvr.h $(HDR_DIR)
	chown bin $(HDR_DIR)/morse_dvr.h
	chgrp bin $(HDR_DIR)/morse_dvr.h
	chmod 444 $(HDR_DIR)/morse_dvr.h
	sh idmors
	chmod 222 /dev/morse

usemorse: usemorse.o
	$(CC) -o $@ usemorse.o -lm -lx

clean:
	-rm -f [a-z]*.o *~ *.bak
	-rm -f tags cscope.out
clobber: clean
	-rm -f testmorse usemorse Driver.o

