# MAKEFILE

T= testgettermfd

ALL= $(T)

SRCROOT= $(LOCAL)


BINDIR= $(SRCROOT)/bin
INCDIR= $(SRCROOT)/include
LIBDIR= $(SRCROOT)/lib


CC= gcc
CCOPTS= -O3 -mcpu=ultrasparc -fpic
#CCOPTS= -g

# HyperSPARC
#CCOPTS= -xO5 -xtarget=ss20/hs22 -dalign -xdepend

# UltraSPARC
#CCOPTS= -xO5 -xtarget=ultra -xsafe=mem -dalign -xdepend


DEFINES0= -D$(OSTYPE)=1 -DSOLARIS=1 
DEFINES1= -DPOSIX=1 -D_POSIX_PTHREAD_SEMANTICS=1 -D_REENTRANT=1
DEFINES2= -D_POSIX_C_SOURCE=199506L -D_POSIX_PER_PROCESS_TIMER_SOURCE=1
DEFINES3= -D__EXTENSIONS__=1

DEFINES= $(DEFINES0) $(DEFINES1) $(DEFINES2) $(DEFINES3)


INCDIRS= -I$(INCDIR)
CFLAGS= $(CCOPTS) $(DEFINES) $(INCDIRS)

#LD= $(CC)
#LD= cc
LD= ld
LDFLAGS= -R${LIBDIR}/$(OFD):$(LIBDIR)


LIBDIRS= -L$(LIBDIR)/$(OFD) -L$(LIBDIR)

LIBS0= -Bstatic -lpcs -ldam -Bdynamic -lnews
LIBS1= -Bstatic -lb -luc -Bdynamic
LIBS2= -lu
LIBS3= -L$(GNU)/lib -lgcc
LIBS4= -lgen -lpthread -lrt -lsocket -lnsl -lc

LIBS= $(LIBS0) $(LIBS1) $(LIBS2) $(LIBS3) $(LIBS4)


LIBCC=/opt/SUNWspro/SC4.0/lib

CRTI= $(LIBCC)/crti.o
CRT1= $(LIBCC)/crt1.o
CRTN= $(LIBCC)/crtn.o
CRT0= $(CRTI) $(CRT1)

#CRTC= compiletime.o
CRTC=

LINT= lint
LINTFLAGS= -uxn -Dlint $(DEFINES) $(INCDIRS)


OBJ0=
OBJ1= main.o whatinfo.o
OBJ2=
OBJ3=
OBJ4=
OBJ5=
OBJ6=
OBJ7=

OBJ= $(OBJ0) $(OBJ1) $(OBJ2) $(OBJ3) $(OBJ4) $(OBJ5) $(OBJ6) $(OBJ7)

OBJS= $(CRT0) $(OBJ) $(CRTC)



default:	$(T).x

all:		$(ALL)

.c.o:
	$(CC) -c $(CFLAGS) $<

.c.ln:
	$(LINT) -u $(DEFINES) $(INCDIRS) $<


$(T):			$(T).x
	cp -p $(T).x $(T)

$(T).x:			$(OBJ) Makefile
	$(LD) -o $@ $(LDFLAGS) $(OBJS) $(LIBDIRS) $(LIBS) $(CRTN)

$(T).$(OFF):		$(T).x
	cp -p $(T).x $(T).$(OFF)

install.$(OFF):		$(T).$(OFF)
	bsdinstall $(ALL) $(BINDIR)

install:		$(ALL)
	bsdinstall $(ALL) $(BINDIR)

install-raw:		$(T).x
	cp -p $(T).x $(T)
	rm -f $(BINDIR)/$(T).$(OFF)
	bsdinstall $(T) $(BINDIR)

strip:			$(T).x
	strip $(T).x
	rm -f $(T).$(OFF) $(T)

again:
	rm -f $(ALL)

clean:			again
	rm -f *.o $(ALL)

control:
	(date ; uname -n) > Control

cflow:
	cflow -I$(PCS)/include *.c > bb.cflow



main.o:		main.c config.h defs.h

whatinfo.o:	whatinfo.c config.h

main.li:	main.c config.h defs.h
	$(LINT) $(LINTFLAGS) main.c




