# MAKEFILE

T= testput

ALL= $(T)


BINDIR= $(SRCROOT)/bin
INCDIR= $(SRCROOT)/include
LIBDIR= $(SRCROOT)/lib



CC= gcc
CCOPTS= -O3 -mv8 -fpic

#CC= cc
#CCOPTS=

# HyperSPARC
#CCOPTS= -xO5 -xtarget=ss20/hs22 -dalign -xdepend

# UltraSPARC
#CCOPTS= -xO5 -xtarget=ultra -xsafe=mem -dalign -xdepend


DEFINES1= -DPOSIX=1 -D_POSIX_PTHREAD_SEMANTICS=1 -D_REENTRANT=1
DEFINES2= -D$(OSTYPE)=1 -DSOLARIS=1 

DEFINES= $(DEFINES1) $(DEFINES2)


INCDIRS= -I. -I$(INCDIR)
CFLAGS= $(CCOPTS) $(DEFINES) $(INCDIRS)

#LD= $(CC)
LD= cc
LDFLAGS=


LIBDIRS= -L$(LIBDIR)/$(OFD)

LIBS= -ldam -lb -luc -lu -lgen -lsocket -lnsl


#CRT0= /lib/crt0.o
CRT0=

RANLIB= ranlib

LINT= lint
LINTFLAGS= -uxn -Dlint $(DEFINES) $(INCDIRS)


OBJ= main.o


.SUFFIXES:		.ls .i .cx .cs


default:	$(T)

all:		$(ALL)

.c.o:
	$(CC) -c $(CFLAGS) $<


$(T):		$(OBJ) Makefile
	$(LD) -o $@ $(LDFLAGS) $(CRT0) $(OBJ) $(LIBDIRS) $(LIBS)

strip:		$(T)
	strip $(T)

install:	$(T)
	strip $(T)
	cp $(T) $(BINDIR)

again:
	rm -f $(ALL)

clean:
	rm -f *.o $(ALL)



