# MAKEFILE (movemail)

T= movemail

ALL= $(T) $(T).$(OFF)

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
DEFINES2= -DSOLARIS=1 
DEFINES3= -D$(OSTYPE)=1

DEFINES= $(DEFINES1) $(DEFINES2) $(DEFINES3)


INCDIRS= -I$(INCDIR)
CFLAGS= $(CCOPTS) $(DEFINES) $(INCDIRS)

#LD= $(CC)
#LD= cc
LD= ld

LDFLAGS= -m -R$(LIBDIR)/$(OFD):$(LIBDIR)


LIBDIRS= -L$(LIBDIR)/$(OFD) -L$(LIBDIR)

LIBS0= -lut -Bstatic -lpcs -ldam -Bdynamic -lnews
LIBS1= -Bstatic -lb -luc -Bdynamic
LIBS2= -lu
#LIBS3= -Bstatic -lcurses -Bdynamic
LIBS4= -lsocket -lnsl -lc

LIBS= $(LIBS0) $(LIBS1) $(LIBS2) $(LIBS3) $(LIBS4)


CRTI= $(LDCRTDIR)/crti.o
CRT1= $(LDCRTDIR)/crt1.o
CRTN= $(LDCRTDIR)/crtn.o
CRT0= $(CRTI) $(CRT1)

CRTC= makedate.o
CRT= $(CRTI) $(CRT1) $(CRTN)

LINT= lint
LINTFLAGS= -uxn -Dlint $(DEFINES) $(INCDIRS)


OBJ= main.o whatinfo.o

OBJS= $(CRT0) $(OBJ) $(CRTC) 


default:		$(T).x

all:			$(ALL)

.c.o:			defs.h
	$(CC) -c $(CFLAGS) $<


$(T):			$(T).sh
	cp -p $(T).sh $(T)

$(T).x:			$(OBJ) Makefile
	makedate > makedate.c
	$(CC) -c makedate.c
	$(LD) -o $@ $(LDFLAGS) $(OBJS) $(LIBDIRS) $(LIBS) $(CRTN) > $(T).lm

$(T).$(OFF) $(OFF):	$(T).x
	strip $(T).x
	cp -p $(T).x $(T).$(OFF)

strip:			$(T).x
	strip $(T).x
	rm -f $(T) $(T).$(OFF)

install:		$(ALL)
	bsdinstall $(ALL) $(BINDIR)
	-chgrp mail $(SRCROOT)/bin/movemail.s5
	-chmod g+s $(SRCROOT)/bin/movemail.s5

again:
	rm -fr $(ALL) $(T).x

clean:			again
	rm -f $(OBJ) 

control:
	uname -n > Control
	date >> Control



