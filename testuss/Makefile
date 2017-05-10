# MAKEFILE

T= testuss

ALL= $(T)

#SRCROOT= $(LOCAL)


BINDIR= $(SRCROOT)/bin
INCDIR= $(SRCROOT)/include
LIBDIR= $(SRCROOT)/lib

#LDCRTDIR= /opt/SUNWspro/WS6/lib
#LDCRTDIR= /opt/SUNWspro/SC4.0/lib
#LDCRTDIR= /opt/SUNWspro/lib
LDCRTDIR= $(SRCROOT)/lib


CC= gcc
#CCOPTS= -O3 -mcpu=ultrasparc # -fpic
CCOPTS= -g

# HyperSPARC
#CCOPTS= -xO5 -xtarget=ss20/hs22 -dalign -xdepend

# UltraSPARC
#CCOPTS= -xO5 -xtarget=ultra -xsafe=mem -dalign -xdepend


DEF0= -D$(OSTYPE)=1 -DSOLARIS=1 
DEF1= -DPOSIX=1 -D_POSIX_PTHREAD_SEMANTICS=1 -D_REENTRANT=1
DEF2= -D_POSIX_C_SOURCE=199506L -D_POSIX_PER_PROCESS_TIMER_SOURCE=1
DEF3= -D__EXTENSIONS__=1
DEF4=

DEFS= $(DEF0) $(DEF1) $(DEF2) $(DEF3) $(DEF4)


INCDIRS= -I$(INCDIR)
CFLAGS= $(CCOPTS) $(DEFS) $(INCDIRS)

#LD= $(CC)
#LD= cc
LD= ld
LDFLAGS= -m -R$(LIBDIR)/$(OFD):$(LIBDIR)


LIBDIRS= -L$(LIBDIR)/$(OFD) -L$(LIBDIR)

LIB0= -lut -Bstatic -lpcs -ldam -Bdynamic -lnews
LIB1= -Bstatic -lb -luc -Bdynamic
LIB2= -lu
LIB3= -L$(GNU)/lib -lgcc
LIB4= -lgen -lpthread -lrt -lsocket -lnsl -lc

LIBS= $(LIB0) $(LIB1) $(LIB2) $(LIB3) $(LIB4)


CRTI= $(LDCRTDIR)/crti.o
CRT1= $(LDCRTDIR)/crt1.o
CRTN= $(LDCRTDIR)/crtn.o

CRT0= $(CRTI) $(CRT1)
CRTC= makedate.o

CRT= $(CRTI) $(CRT1) $(CRTN)

LINT= lint
LINTFLAGS= -uxn -Dlint $(DEFS) $(INCDIRS)


CMDHELP= $(T).cmdhelp

HELP= -DCMDHELPFNAME=\"lib/vmail/cmdhelp\"

INCS= config.h


SLIB=

OBJ00=
OBJ01= main.o listenuss.o
OBJ02=
OBJ03=
OBJ04=
OBJ05=
OBJ06=
OBJ07=
OBJ08=
OBJ09=
OBJ10=
OBJ11=
OBJ12=
OBJ13=
OBJ14=
OBJ15=


OBJA= $(OBJ00) $(OBJ01) $(OBJ02) $(OBJ03) $(OBJ04) $(OBJ05) $(OBJ06) $(OBJ07)
OBJB= $(OBJ08) $(OBJ09) $(OBJ10) $(OBJ11) $(OBJ12) $(OBJ13) $(OBJ14) $(OBJ15)

OBJ= $(OBJA) $(OBJB) $(SLIB)

OBJS= $(CRT0) $(OBJ) $(CRTC) 


.SUFFIXES:		.ls .i .cx .cs


default:		$(T).x

all:			$(ALL)

.c.o:			defs.h
	$(CC) -c $(CFLAGS) $<

.c.t:
	$(LINT) -u $(DEFS) $(INCDIRS) $<

.c.ln:
	$(LINT) -u $(DEFS) $(INCDIRS) $<


$(T):			$(T).ee
	cp -p $(T).ee $(T)

$(T).x:			$(OBJ) Makefile
	makedate -m $(T) > makedate.c
	$(CC) -c makedate.c
	$(LD) -o $@ $(LDFLAGS) $(OBJS) $(LIBDIRS) $(LIBS) $(CRTN) > $(T).lm

$(T).$(OFF) $(OFF):	$(T).x
	cp -p $(T).x $(T).$(OFF)

strip:			$(T).x
	strip $(T).x
	rm -f $(T).$(OFF) $(T)

install:		$(ALL) $(T).$(OFF)
	rm -f n$(T) n$(T).x n$(T).$(OFF)
	bsdinstall $(ALL) $(BINDIR)
	cp -p $(CMDHELP) $(SRCROOT)/lib/vmail/cmdhelp

install-raw:		$(T).x
	cp -p $(T).x $(T)
	rm -f $(BINDIR)/$(T).$(OFF)
	bsdinstall $(T) $(BINDIR)

again:
	-rm -f $(ALL) $(T).x $(T).$(OFF)

clean:			again
	-rm -f $(OBJ) $(ALL)

control:
	(uname -n ; date) > Control



libcurses.a:		libcurses.obj
	cp -p libcurses.obj $@

vsnprintf.o:		vsnprintf.obj
	cp -p vsnprintf.obj $@


main.o:			main.c $(INCS)
	$(CC) -c $(CFLAGS) $(HELP) main.c

whatinfo.o:		whatinfo.c config.h

help.o:			help.c $(INCS)
	$(CC) -c $(CFLAGS) help.c

delete.o:		$(INCS)

undelete.o:		undelete.c $(INCS)

*.o:			$(INCS)


