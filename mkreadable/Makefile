# MAKEFILE

T= mkreadable

ALL= $(T)

SRCROOT= $(LOCAL)


BINDIR= $(SRCROOT)/bin
INCDIR= $(SRCROOT)/include
LIBDIR= $(SRCROOT)/lib


CC= gcc
CCOPTS= -O3 -mcpu=ultrasparc # -fpic
#CCOPTS= -g

# HyperSPARC
#CCOPTS= -xO5 -xtarget=ss20/hs22 -dalign -xdepend

# UltraSPARC
#CCOPTS= -xO5 -xtarget=ultra -xsafe=mem -dalign -xdepend


DEFINES1= -DPOSIX=1 -D_POSIX_PTHREAD_SEMANTICS=1 -D_REENTRANT=1
DEFINES2= -DPOSIX=1 -DSOLARIS=1 
DEFINES3= -D$(OSTYPE)=1

DEFINES= $(DEFINES1) $(DEFINES2) $(DEFINES3)


INCDIRS= -I$(INCDIR)

CFLAGS= $(CCOPTS) $(DEFINES) $(INCDIRS)

LD= $(CC)
#LD= ld
#LD= cc

LDFLAGS= # -R$(LIBDIR)/$(OFD):$(LIBDIR)


LIBDIRS= -L$(LIBDIR)/$(OFD) -L$(LIBDIR)

LIBS0= -lpcs -ldam
LIBS1= -lb -luc
LIBS2= -lu
LIBS3=
LIBS4= -lgen -lsocket -lnsl

LIBS= $(LIBS0) $(LIBS1) $(LIBS2) $(LIBS3) $(LIBS4)


LIBCC= /opt/SUNWspro/SC4.0/lib

#CRTI= $(LIBCC)/crti.o
#CRT1= $(LIBCC)/crt1.o
#CRTN= $(LIBCC)/crtn.o
CRT0= $(CRTI) $(CRT1)

CRTC= makedate.o
CRT= $(CRTI) $(CRT1) $(CRTN)

LINT= lint
LINTFLAGS= -u -Dlint $(DEFINES) $(INCDIRS)


OBJ0=
OBJ1= main.o whatinfo.o process.o checkname.o
OBJ2= paramopt.o
OBJ3=

OBJ= $(OBJ1) $(OBJ2) $(OBJ3)

OBJS= $(CRT0) $(OBJ) $(CRTC)



all:		$(ALL)

.c.o:
	$(CC) -c $(CFLAGS) $<

.c.ln:
	$(LINT) -u $(DEFINES) $(INCDIRS) $<


$(T):		$(OBJ) Makefile
	makedate > makedate.c
	$(CC) -c $(CFLAGS) makedate.c
	$(LD) -o $@ $(LDFLAGS) $(OBJS) $(LIBDIRS) $(LIBS) $(CRTN)

install:	strip
	bsdinstall $(ALL) $(BINDIR)

strip:		$(T)
	strip $(T)

again:
	rm -f $(ALL) $(T).X

clean:
	rm -f *.o $(ALL) $(T).x

control:
	(date ; uname -n) > Control

cflow:
	cflow -I$(PCS)/include *.c > bb.cflow


main.o:		main.c config.h defs.h

main.ln:	main.c config.h defs.h
	$(LINT) $(LINTFLAGS) main.c

process.o:	process.c config.h defs.h

process.ln:	process.c config.h defs.h
	$(LINT) $(LINTFLAGS) process.c

checkname.o:	checkname.c config.h defs.h




