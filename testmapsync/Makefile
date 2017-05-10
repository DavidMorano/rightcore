# MAKEFILE

T= testmapsync

ALL= $(T)

HELPFILE= $(T).help

SRCROOT= $(LOCAL)


BINDIR= $(SRCROOT)/bin
INCDIR= $(SRCROOT)/include
LIBDIR= $(SRCROOT)/lib

#LDCRTDIR= /opt/SUNWspro/WS6/lib
#LDCRTDIR= /opt/SUNWspro/SC5.0/lib
#LDCRTDIR= /opt/SUNWspro/SC4.0/lib
#LDCRTDIR= /opt/SUNWspro/lib
LDCRTDIR= $(SRCROOT)/lib


#CC= gcc
#CCOPTS= -O3 -mcpu=ultrasparc # -fpic
#CCOPTS= -g -O
#CCOPTS= -g
#CCOPTS= -g -pg

# HyperSPARC
#CCOPTS= -xO5 -xtarget=ss20/hs22 -dalign -xdepend

# UltraSPARC
CCOPTS= -xO5 -xtarget=ultra -xsafe=mem -dalign -xdepend


DEF0= -D$(OSTYPE)=1 -DSOLARIS=8 -DPOSIX=1 -DPTHREAD=1
DEF1= 
DEF2= -D__EXTENSIONS__=1 -D_REENTRANT=1
DEF3= -D_POSIX_C_SOURCE=199506L 
DEF4= -D_POSIX_PTHREAD_SEMANTICS=1 -D_POSIX_PER_PROCESS_TIMER_SOURCE=1
DEF5=
DEF6=
DEF7=

DEFS= $(DEF0) $(DEF1) $(DEF2) $(DEF3) $(DEF4) $(DEF5) $(DEF6) $(DEF7)


INCDIRS= -I$(INCDIR)

CPPFLAGS= $(DEFS) $(INCDIRS)
CFLAGS= $(CCOPTS)

#LD= $(CC)
#LD= cc
LD= ld
LDFLAGS= -m -R$(LIBDIR)/$(OFD):$(LIBDIR)


LIBDIRS= -L$(LIBDIR)/$(OFD) -L$(LIBDIR)

LIB0=
LIB1= -Bstatic -ldam -Bdynamic
LIB2=
LIB3= -Bstatic -lb -luc -Bdynamic
LIB4= -Bstatic -lu -Bdynamic
LIB5= -L$(GNU)/lib -lgcc
LIB6= -lpthread -lrt -lsocket -lnsl
LIB7= -lc

LIBS= $(LIB0) $(LIB1) $(LIB2) $(LIB3) $(LIB4) $(LIB5) $(LIB6) $(LIB7)


CRTI= $(LDCRTDIR)/crti.o
CRT1= $(LDCRTDIR)/crt1.o
VALUES= $(LDCRTDIR)/values-xa.o
CRTN= $(LDCRTDIR)/crtn.o

CRT0= $(CRTI) $(CRT1) $(VALUES)
CRTC= makedate.o

LINT= lint
LINTFLAGS= -uxn -Dlint

NM= nm
NMFLAGS= -xs -v

CXREF= cxref
CXREFFLAGS= -R -s

CPP= cpp

LORDER= lorder
TSORT= tsort

RM= rm -f


INCS= config.h defs.h


OBJ00= main.o whatinfo.o proginfo.o proginfo_setpiv.o
OBJ01=
OBJ02=
OBJ03=
OBJ04=
OBJ05=
OBJ06=
OBJ07= printhelp.o

OBJA= $(OBJ00) $(OBJ01) $(OBJ02) $(OBJ03) $(OBJ04) $(OBJ05) $(OBJ06) $(OBJ07)
OBJB= $(OBJ08) $(OBJ09) $(OBJ10) $(OBJ11) $(OBJ12) $(OBJ13) $(OBJ14) $(OBJ15)

OBJ= $(OBJA) $(OBJB)

OBJS= $(CRT0) $(OBJ) $(CRTC)


SRC= $(OBJ:.c=.o)


.SUFFIXES:	.ls .i .cx .cs


default:		$(T).x

all:			$(ALL)

.c.o:
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $<

.c.ln:
	$(LINT) -c -u $(CPPFLAGS) $<

.c.ls:
	$(LINT) $(LINTFLAGS) $(CPPFLAGS) $<

.c.i:
	$(CPP) $(CPPFLAGS) $< > $(*).i

.c.cx:
	$(CXREF) -C $(CXREFFLAGS) $(CPPFLAGS) $<

.c.cs:
	$(CXREF) $(CXREFFLAGS) $(CPPFLAGS) -o $(*).cs $<


$(T):			$(T).ee
	cp -p $(T).ee $(T)

$(T).x:			$(OBJ) Makefile
	makedate -m $(T) > makedate.c
	$(CC) -c $(CFLAGS) makedate.c
	$(LD) -o $@ $(LDFLAGS) $(OBJS) $(LIBDIRS) $(LIBS) $(CRTN) > $(T).lm

$(T).$(OFF) $(OFF):	$(T).x
	cp -p $(T).x $(T).$(OFF)

$(T).nm nm:		$(T).x
	$(NM) $(NMFLAGS) $(T).x > $(T).nm

$(T).cxref:
	$(CXREF) -c $(CXREFFLAGS) $(SRC) > $(T).cxref

strip:			$(T).x
	strip $(T).x
	rm -f $(T).$(OFF) $(T)

install:		$(ALL)
	makenewer -r $(ALL) $(BINDIR)

install-raw:		$(T).x
	cp -p $(T).x $(T)
	rm -f $(BINDIR)/$(T).$(OFF)
	makenewer -r $(T) $(BINDIR)

install-help:		$(T).help
	-mkdir -p $(SRCROOT)/share/help
	cp -p $(T).help $(T)
	makenewer $(T) $(SRCROOT)/share/help

again:
	rm -f $(ALL) $(T).x

clean:			again
	rm -f *.o

control:
	uname -n > Control
	date >> Control



main.o:			main.c $(INCS)

whatinfo.o:		whatinfo.c config.h

proginfo.o:		proginfo.c $(INCS)

printhelp.o:		printhelp.c

vecstr_loadfile.o:	vecstr_loadfile.c

filemap.o:		filemap.c filemap.h

realname.o:		realname.c realname.h

ipasswd.o:		ipasswd.c ipasswd.h

lastlogfile.o:		lastlogfile.c lastlogfile.h

udomain.o:		udomain.c



