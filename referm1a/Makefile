# MAKEFILE

T= referm

ALL= $(T) $(T).$(OFF)

SRCROOT= $(LOCAL)


BINDIR= $(SRCROOT)/bin
INCDIR= $(SRCROOT)/include
LIBDIR= $(SRCROOT)/lib



CC= gcc
#CCOPTS= -O3 -mcpu=ultrasparc # -fpic
CCOPTS=

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
LD= cc

LDFLAGS= -R$(LIBDIR)/$(OFD):$(LIBDIR)


LIBDIRS= -L$(LIBDIR)/$(OFD) -L$(LIBDIR)

LIBS0= -Bstatic -lpcs -ldam -Bdynamic
LIBS1= -Bstatic -lb -luc -Bdynamic
LIBS2= -lu
LIBS3=
LIBS4= -lsocket -lnsl

LIBS= $(LIBS0) $(LIBS1) $(LIBS2) $(LIBS3) $(LIBS4)


#CRT0= /lib/crt0.o
CRTI=
CRT1=
CRTN=
CRT0= $(CRTI) $(CRT1)
CRTC= compiletime.o
CRT= $(CRTI) $(CRT1) $(CRTN) $(CRTC)

LINT= lint
LINTFLAGS= -uxn -Dlint $(DEFINES) $(INCDIRS)


OBJ00= main.o version.o bdb.o configfile.o var.o expander.o
OBJ01= process.o hdb.o
OBJ= $(OBJ00) $(OBJ01)

OBJS= $(CRT0) $(OBJ) $(CRTC)


.SUFFIXES:		.ls .i .cx .cs


all:			$(ALL)

.c.o:
	$(CC) -c $(CFLAGS) $<

.c.ln:
	$(LINT) -u $(DEFINES) $(INCDIRS) $<


$(T):			$(T).sh
	cp $(T).sh $(T)

$(T).x:			$(OBJ) Makefile
	compiletime > compiletime.c
	$(CC) -c $(CFLAGS) compiletime.c
	$(LD) -o $@ $(LDFLAGS) $(CRT) $(OBJS) $(LIBDIRS) $(LIBS)

$(T).$(OFF) $(OFF):	$(T).x
	cp $(T).x $(T).$(OFF)

strip:			$(T).x
	strip $(T).x

install:		$(T).x
	strip $(T).x
	bsdinstall $(ALL) $(BINDIR)

again:
	rm -f $(ALL) $(T).x

clean:
	rm -f *.o $(T).x

control:
	(uname -n ; date) > Control


main.o:		main.c config.h

version.o:	config.h



