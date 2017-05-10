# MAKEFILE

T= testio

ALL= $(T)

#SRCROOT= $(LOCAL)


BINDIR= $(SRCROOT)/bin
INCDIR= $(SRCROOT)/include
LIBDIR= $(SRCROOT)/lib


#CC= gcc

#CCOPTS= -O3 -mcpu=ultrasparc -fpic
#CCOPTS= -mcpu=ultrasparc -fpic

# HyperSPARC
CCOPTS= -xO5 -xtarget=ss20/hs22 -dalign -xdepend

# UltraSPARC
#CCOPTS= -xO5 -xtarget=ultra -xsafe=mem -dalign -xdepend


DEFINES1= -DPOSIX=1 -D_POSIX_PTHREAD_SEMANTICS=1 -D_REENTRANT=1
DEFINES2= -D$(OSTYPE)=1 -DSOLARIS=1 

DEFINES= $(DEFINES1) $(DEFINES2)


INCDIRS= -I. -I$(INCDIR)
CFLAGS= $(CCOPTS) $(DEFINES) $(INCDIRS)

LD= $(CC)
LDFLAGS=


LIBDIRS= -L$(LIBDIR)/$(OFD)

LIBS= -ldam -lb -luc -lu -lsocket -lnsl


#CRT0= /lib/crt0.o
CRT0=

LINT= lint
LINTFLAGS= -uxn -Dlint $(DEFINES) $(INCDIRS)


OBJ= main.o eprintf.o


.SUFFIXES:		.ls .i .cx .cs


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



