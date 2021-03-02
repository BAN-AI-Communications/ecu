CFLAGS = -DSYSV

OBJ = \
	tts.o \
	hexdump.o \
	sockutil.o

LIBS = -lsocket

all: tts

tts: $(OBJ)
	$(CC) -o $@ $(OBJ) $(LIBS)
