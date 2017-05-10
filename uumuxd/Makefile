# MAKEFILE

T= tcpmuxd
 
ALL= $(T)

#SRCROOT= $(HOME)
SRCROOT= $(LOCAL)


BINDIR= $(SRCROOT)/bin
INCDIR= $(SRCROOT)/include
LIBDIR= $(SRCROOT)/lib

#LDCRTDIR= /opt/SUNWspro/WS6/lib
#LDCRTDIR= /opt/SUNWspro/SC4.0/lib
#LDCRTDIR= /opt/SUNWspro/lib


CC= gcc
#CCOPTS= -O3 -mcpu=ultrasparc # -fpic
CCOPTS= -g

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

LIBS0= -lnews -lut -Bstatic -lpcs -ldam -Bdynamic
LIBS1= -Bstatic -lb -luc -Bdynamic
LIBS2= -lu
#LIBS3= -Bstatic -lcurses -Bdynamic
LIBS4= -lgen -lsocket -lnsl -lc

LIBS= $(LIBS0) $(LIBS1) $(LIBS2) $(LIBS3) $(LIBS4)


CRTI= $(LDCRTDIR)/crti.o
CRT1= $(LDCRTDIR)/crt1.o
CRTN= $(LDCRTDIR)/crtn.o
CRT0= $(CRTI) $(CRT1)

CRTC= makedate.o
CRT= $(CRTI) $(CRT1) $(CRTN)

LINT= lint
LINTFLAGS= -uxn -Dlint $(DEFINES) $(INCDIRS)


OBJ00=
OBJ01= main.o configfile.o checklockfile.o
OBJ02= var.o expander.o
OBJ03= watch.o
OBJ04= handle.o processargs.o execute.o
OBJ05= srvpe.o srventry.o builtin.o
OBJ06= jobdb.o
OBJ07= debug.o util.o strkeycmp.o
OBJ08= srvtab.o fieldsrvarg.o
OBJ09= procfileenv.o procfilepaths.o strfieldcmp.o
OBJ10= bopensched.o bopenroot.o
OBJ11= permsched.o schedvar.o sbuf.o outbuf.o
OBJ12= getfname.o
OBJ13=
OBJ14=
OBJ15=

OBJA= $(OBJ00) $(OBJ01) $(OBJ02) $(OBJ03) $(OBJ04) $(OBJ05) $(OBJ06) $(OBJ07) 
OBJB= $(OBJ08) $(OBJ09) $(OBJ10) $(OBJ11) $(OBJ12) $(OBJ13) $(OBJ14) $(OBJ15)

OBJ= $(OBJA) $(OBJB)

OBJS= $(CRT0) $(OBJ) $(CRTC)


.SUFFIXES:		.ls .i .cx .cs


default:		$(T).x

all:			$(ALL)

.c.o:
	$(CC) -c $(CFLAGS) $<


$(T):			$(T).sh
	cp $(T).sh $(T)

$(T).x:			$(OBJ) Makefile
	makedate > makedate.c
	$(CC) -c $(CFLAGS) makedate.c
	$(LD) -o $@ $(LDFLAGS) $(OBJS) $(LIBDIRS) $(LIBS) $(CRTN) > $(T).lm

$(T).$(OFF) $(OFF):	$(T).x
	cp -p $(T).x $(T).$(OFF)

strip:			$(T).x
	strip $(T).x
	rm -f $(T) $(T).$(OFF)

install:		$(ALL)
	bsdinstall $(ALL) $(BINDIR)

again:
	rm -fr $(ALL) $(T) $(T).x

clean:			again
	rm -f *.o $(ALL)

control:
	(uname -n ; date) > Control


main.o:			main.c config.h defs.h


