# MAKEFILE

T= testcurses

ALL= $(T) $(T).$(OFF)

CMDHELP= $(T).cmdhelp

#PCS= /usr/add-on/pcs


BINDIR= $(PCS)/bin
INCDIR= $(PCS)/include
LIBDIR= $(PCS)/lib

#LDCRTDIR= /opt/SUNWspro/WS6/lib
#LDCRTDIR= /opt/SUNWspro/SC4.0/lib
#LDCRTDIR= /opt/SUNWspro/lib


CC= gcc
CCOPTS= -O3 -mcpu=ultrasparc # -fpic
#CCOPTS= -g

# HyperSPARC
#CCOPTS= -xO5 -xtarget=ss20/hs22 -dalign -xdepend

# UltraSPARC
#CCOPTS= -xO5 -xtarget=ultra -xsafe=mem -dalign -xdepend


DEFINES0= -D$(OSTYPE)=1 -DSOLARIS=1 
DEFINES1= -DPOSIX=1 -D_POSIX_PTHREAD_SEMANTICS=1 -D_REENTRANT=1
DEFINES2= -D_POSIX_C_SOURCE=199506L -D_POSIX_PER_PROCESS_TIMER_SOURCE=1
DEFINES3= -D__EXTENSIONS__=1
DEFINES4=

DEFINES= $(DEFINES0) $(DEFINES1) $(DEFINES2) $(DEFINES3) $(DEFINES4)


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
LIBS3= -L$(GNU)/lib -lgcc
LIBS4= -lcurses -lgen -lpthread -lrt -lsocket -lnsl -lc

LIBS= $(LIBS0) $(LIBS1) $(LIBS2) $(LIBS3) $(LIBS4)


CRTI= $(LDCRTDIR)/crti.o
CRT1= $(LDCRTDIR)/crt1.o
CRTN= $(LDCRTDIR)/crtn.o
CRT0= $(CRTI) $(CRT1)

CRTC= makedate.o
CRT= $(CRTI) $(CRT1) $(CRTN)

LINT= lint
LINTFLAGS= -uxn -Dlint $(DEFINES) $(INCDIRS)


HELP= -DCMDHELPFNAME=\"lib/vmail/cmdhelp\"

INC= config.h defs.h profile.h


SLIBS= libcurses.a vsnprintf.o

OBJ00= main.o 
OBJ01= 
OBJ02= 
OBJ03= 
OBJ04= 
OBJ05= 
OBJ06= 
OBJ07= 
OBJ08= 
OBJ09= 
OBJ10= 

OBJA= $(OBJ00) $(OBJ01) $(OBJ02) $(OBJ03) $(OBJ04) $(OBJ05) $(OBJ06) $(OBJ07)
OBJB= $(OBJ08) $(OBJ09) $(OBJ10) $(OBJ11) $(OBJ12) $(OBJ13) $(OBJ14) $(OBJ15)

OBJ= $(OBJA) $(OBJB)

OBJS= $(CRT0) $(OBJ) $(CRTC) 


.SUFFIXES:		.ls .i .cx .cs


default:		$(T).x

all:			$(ALL)

.c.o:			defs.h
	$(CC) -c $(CFLAGS) $<

.c.t:
	$(LINT) -u $(DEFINES) $(INCDIRS) $<

.c.ln:
	$(LINT) -u $(DEFINES) $(INCDIRS) $<


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
	cp -p $(CMDHELP) $(PCS)/lib/vmail/cmdhelp

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


main.o:			main.c $(INC)
	$(CC) -c $(CFLAGS) $(HELP) main.c

whatinfo.o:		whatinfo.c config.h

help.o:			help.c $(INC)
	$(CC) -c $(CFLAGS) help.c

delete.o:		$(INC)

undelete.o:		undelete.c $(INC)

*.o:			$(INC)


