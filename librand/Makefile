# MAKEFILE

T= librand

SRCROOT= $(LOCAL)


LIBDIR= $(SRCROOT)/lib


DEF0=
DEF1=
DEF2=
DEF3=
DEF4=
DEF5=
DEF6=
DEF7= $(LF_DEFS)

DEFS= $(DEF0) $(DEF1) $(DEF2) $(DEF3) $(DEF4) $(DEF5) $(DEF6) $(DEF7)


INCDIRS= -I$(LOCAL)/include


SRCS=randbyte.c unix_truerand.c nt_truerand.c qshs.c
OBJS=randbyte.o unix_truerand.o qshs.o

CC=gcc


CFLAGS= $(DEFS) $(INCDIRS)


# No -O in CFLAGS! On some compilers, this optimizes out the counter...

librand.a: $(OBJS)
	ar -rcv librand.a $(OBJS)
	ranlib librand.a

librand.shar: README makefile $(SRCS) 
	shar README makefile $(SRCS) > librand.shar

clean:
	rm -f $(OBJS) librand.a

install:
	bsdinstall $(T).a $(LIBDIR)


