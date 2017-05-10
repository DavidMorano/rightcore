# MAKEFILE

T= ripper

SRCROOT= $(NCMP)

BINDIR= $(SRCROOT)/bin
INCDIR= $(SRCROOT)/include
LIBDIR= $(SRCROOT)/lib
HELPDIR= $(SRCROOT)/share/help


CC= gcc

#CCOPTS= -O3 -mcpu=ultrasparc -fpic
CCOPTS= -mcpu=ultrasparc # -fpic

# HyperSPARC
#CCOPTS= -xO5 -xtarget=ss20/hs22 -dalign -xdepend

# UltraSPARC
#CCOPTS= -xO5 -xtarget=ultra -xsafe=mem -dalign -xdepend


DEFINES1= -DPOSIX=1 -D_POSIX_PTHREAD_SEMANTICS=1 -D_REENTRANT=1
DEFINES2= -D$(OSTYPE)=1 -DSOLARIS=1 

DEFINES= $(DEFINES1) $(DEFINES2)


INCDIRS= -I$(INCDIR)
CFLAGS= $(CCOPTS) $(DEFINES) $(INCDIRS)

#LD= $(CC)
LDFLAGS=


LIBDIRS= -L$(LIBDIR)/$(OFD)

LIBS= -lvolmgt


LINT= lint
LINTFLAGS= -uxn -Dlint $(DEFINES) $(INCDIRS)


OBJ= main.o process.o format.o convert.o ripmisc.o cd.o


all:		$(T)


$(T):		$(OBJ)
	$(LD) -o $@ $(OBJ) $(LIBDIRS) $(LIBS)

clean:
	rm -f *.o

install:	$(T)
	bsdinstall $(T) $(NCMP)/bin

control:
	(uname -n ; date) > Control



main.o:		main.c localmisc.h config.h defs.h

process.o:	process.c localmisc.h config.h defs.h



