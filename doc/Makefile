#+------------------------------------------------------
# ECU manual Makefile
#-------------------------------------------------------
#+:EDITS:
#:01-24-1997-02:36-wht@yuriatin-SOURCE RELEASE 4.00
#:09-11-1996-19:59-wht@yuriatin-3.48-major telnet,curses,structural overhaul
#:11-23-1995-11:20-wht@kepler-source control 3.37 for tsx-11
#:11-14-1995-10:23-wht@kepler-3.37.80-source control point: SOCKETS
#:05-04-1994-04:38-wht@n4hgf-ECU release 3.30
#:09-10-1992-13:58-wht@n4hgf-ECU release 3.20
#:08-22-1992-15:38-wht@n4hgf-ECU release 3.20 BETA
#:08-22-1992-15:07-wht@n4hgf-whoops - forgot -mm
#:06-16-1992-02:05-wht@n4hgf-creation

NROFF = nroff -mm 

#COL_PHRASE =
COL_PHRASE = | col
#COL_PHRASE = | col -b

ECU_FILES = \
	ecu.txt \
	_top.txt \
	_intro.txt \
	_basic.txt \
	_features.txt \
	_startup.txt \
	_icmd.txt \
	_hdb.txt \
	_exits.txt \
	_end.txt

PROC_FILES = \
	proc.txt \
	_p_param.txt \
	_p_ifunc.txt \
	_p_sfunc.txt \
	_p_cmd.txt

all: ecu.man proc.man

ecu.man: $(ECU_FILES)
	$(NROFF) ecu.txt $(COL_PHRASE) > $@

proc.man: $(PROC_FILES)
	$(NROFF) proc.txt $(COL_PHRASE) > $@

# no funky characters in distribution shars
dist:
	$(NROFF) ecu.txt | col -b > $@
	$(NROFF) proc.txt | col -b > $@

