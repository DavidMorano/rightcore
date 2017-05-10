# MAKEFILE

T= daytime

ALL= $(T) $(T).$(OFF)

SRCROOT= $(LOCAL)


BINDIR= $(SRCROOT)/bin
INCDIR= $(SRCROOT)/include
LIBDIR= $(SRCROOT)/lib

#LDCRTDIR= /opt/SUNWspro/WS6/lib
#LDCRTDIR= /opt/SUNWspro/SC4.0/lib
#LDCRTDIR= /opt/SUNWspro/lib
LDCRTDIR= $(SRCROOT)/lib


CC= gcc
#CCOPTS= -O3 -mv8 # -fpic
CCOPTS= -g

# HyperSPARC
#CCOPTS= -xO5 -xtarget=ss20/hs22 -dalign -xdepend

# UltraSPARC
#CCOPTS= -xO5 -xtarget=ultra -xsafe=mem -dalign -xdepend


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
LIB6= -lposix4 -lpthread -lsocket -lnsl
LIB7= -lc

LIBS= $(LIB0) $(LIB1) $(LIB2) $(LIB3) $(LIB4) $(LIB5) $(LIB6) $(LIB7)

CRTI= $(LDCRTDIR)/crti.o
CRT1= $(LDCRTDIR)/crt1.o
VALUES= $(LDCRTDIR)/values-xa.o
CRTN= $(LDCRTDIR)/crtn.o

CRT0= $(CRTI) $(CRT1) $(VALUES)
CRTC= makedate.o

LINT= lint
LINTFLAGS= -uxn

NM= nm
NMFLAGS= -xs

CPP= cpp



INCS= config.h defs.h


OBJ00= 
OBJ01= main.o whatinfo.o proginfo.o printhelp.o vecstr_loadfile.o
OBJ02= timestr_nist.o
OBJ03= debug.o
OBJ04= systems.o
OBJ05= timestr_date.o
OBJ06= hostent.o inetaddr.o sockaddress.o
OBJ07= dialuss.o dialussmux.o dialussnls.o
OBJ08= dialtcp.o dialtcpmux.o dialtcpnls.o
OBJ09= dialticotsord.o dialticotsordnls.o 
OBJ10=
OBJ11= u_connect.o
OBJ12=
OBJ13= ut_open.o ut_close.o ut_bind.o ut_connect.o ut_alloc.o ut_free.o
OBJ14= ut_look.o ut_sync.o
OBJ15=

OBJA= $(OBJ00) $(OBJ01) $(OBJ02) $(OBJ03) $(OBJ04) $(OBJ05) $(OBJ06) $(OBJ07)
OBJB= $(OBJ08) $(OBJ09) $(OBJ10) $(OBJ11) $(OBJ12) $(OBJ13) $(OBJ14) $(OBJ15)

OBJ= $(OBJA) $(OBJB)

OBJS= $(CRT0) $(OBJ) $(CRTC)



.SUFFIXES:	.ls .i


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


$(T):			$(T).ee
	cp -p $(T).ee $(T)

$(T).x:			$(OBJ) Makefile
	makedate > makedate.c
	$(CC) -c makedate.c
	$(LD) -o $@ $(LDFLAGS) $(OBJS) $(LIBDIRS) $(LIBS) $(CRTN) > $(T).lm

$(T).$(OFF) $(OFF):	$(T).x
	cp -p $(T).x $(T).$(OFF)

install:		$(ALL)
	makenewer -r $(ALL) $(BINDIR)

install.raw:		$(T).x
	rm -f $(T)
	cp -p $(T).x $(T)
	rm -f $(BINDIR)/$(T).$(OFF)
	makenewer -r $(T) $(BINDIR)

$(T).nm nm:		$(T).x
	$(NM) $(NMFLAGS) $(T).x > $(T).nm

strip:			$(T).x
	strip $(T).x
	rm -f $(T).$(OFF)

again:
	rm -f $(ALL) $(T).x $(T).$(OFF)

clean:			again
	rm -f $(ALL) *.o

control:
	(uname -n ; date) > Control



main.lint:		main.c
	$(LINT) -uxn $(DEFS) $(INCDIRS) main.c



main.o:			main.c $(INCS)

whatinfo.o:		whatinfo.c config.h

proginfo.o:		proginfo.c $(INCS)

printhelp.o:		printhelp.c

vecstr_loadfile.o:	vecstr_loadfile.c

systems.o:		systems.c systems.h



