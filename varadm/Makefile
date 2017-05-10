# MAKEFILE

T= varadm

ALL= $(T)

SRCROOT= $(LOCAL)


BINDIR= $(SRCROOT)/bin
INCDIR= $(SRCROOT)/include
LIBDIR= $(SRCROOT)/lib
MANDIR= $(SRCROOT)/man/man1
HELPDIR= $(SRCROOT)/share/help

#LDCRTDIR= /opt/SUNWspro/WS6/lib
#LDCRTDIR= /opt/SUNWspro/SC5.0/lib
#LDCRTDIR= /opt/SUNWspro/SC4.0/lib
#LDCRTDIR= /opt/SUNWspro/lib
LDCRTDIR= $(SRCROOT)/lib


CC= gcc

CCOPTS= -O -mcpu=ultrasparc
#CCOPTS= -g
# debugging with extra symbol information w/ Sun Workshop C
#CCOPTS= -g -xs
# debugging with profiling w/ GNU C
#CCOPTS= -g -pg

# HyperSPARC
# regular
#CCOPTS= -xO5 -xtarget=ss20/hs22 -xdepend
# profiling 'prof(1)'
#CCOPTS= -p -xO5 -xtarget=ss20/hs22 -xdepend
# profiling 'gprof(1)' 
#CCOPTS= -xpg -xO5 -xtarget=ss20/hs22 -xdepend

# UltraSPARC
# regular
#CCOPTS= -xO5 -xtarget=ultra -xsafe=mem -xdepend
# profiling 'prof(1)'
#CCOPTS= -p -xO5 -xtarget=ultra -xsafe=mem -xdepend
# profiling 'gprof(1)' 
#CCOPTS= -xpg -xO5 -xtarget=ultra -xsafe=mem -xdepend


DEF0=
DEF1=
DEF2=
DEF3=
DEF4=
DEF5=
DEF6=
DEF7= $(LF_DEFS)

DEFS= $(DEF0) $(DEF1) $(DEF2) $(DEF3) $(DEF4) $(DEF5) $(DEF6) $(DEF7)


INCDIRS= -I$(INCDIR)

CPPFLAGS= $(DEFS) $(INCDIRS)

CFLAGS= $(CCOPTS)

#LD= $(CC)
#LD= cc
LD= ld

# regular
LDFLAGS= -R$(LIBDIR)
# for profiling
LDFLAGS_P= -L/usr/lib/libp # -m -R$(LIBDIR)


LIBDIRS= -L$(LIBDIR)

LIB0=
LIB1= -ldam -lb -luc
LIB2=
LIB3= -Bstatic -lu -Bdynamic
LIB4= -L$(GNU)/lib -lgcc
LIB5= 
LIB6= -lsecdb -lproject -lpthread -lrt -lxnet -lsocket -lnsl
LIB7= -ldl -lc

LIBS= $(LIB0) $(LIB1) $(LIB2) $(LIB3) $(LIB4) $(LIB5) $(LIB6) $(LIB7)


CRTI= $(LDCRTDIR)/crti.o
CRT1= $(LDCRTDIR)/crt1.o
MCRT1= $(LDCRTDIR)/mcrt1.o
GCRT1= $(LDCRTDIR)/gcrt1.o
VALUES= $(LDCRTDIR)/values-xa.o
CRTN= $(LDCRTDIR)/crtn.o

# for regular (no profiling)
CRT0= $(CRTI) $(CRT1) $(VALUES)
# for 'prof(1)'
MCRT0= $(CRTI) $(MCRT1) $(VALUES)
# for 'gprof(1)'
GCRT0= $(CRTI) $(GCRT1) $(VALUES)

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


INCS= config.h defs.h babycalc.h


OBJ00= main.o whatinfo.o proginfo.o proginfo_setpiv.o
OBJ01= b_$(T).o
OBJ02=
OBJ03=
OBJ04=
OBJ05=
OBJ06=
OBJ07= printhelp.o shio.o
OBJ08=
OBJ09=
OBJ10= tmz.o ptm.o ptma.o
OBJ11= babycalc.o babiesfu.o cvtdater.o
OBJ12=
OBJ13= 
OBJ14= heapsort.o
OBJ15=

OBJA= $(OBJ00) $(OBJ01) $(OBJ02) $(OBJ03) $(OBJ04) $(OBJ05) $(OBJ06) $(OBJ07)
OBJB= $(OBJ08) $(OBJ09) $(OBJ10) $(OBJ11) $(OBJ12) $(OBJ13) $(OBJ14) $(OBJ15)

OBJ= $(OBJA) $(OBJB)

# for regular (no profiling)
OBJS= $(CRT0) $(OBJ) $(CRTC)
# for 'prof(1)'
MOBJS= $(MCRT0) $(OBJ) $(CRTC)
# for 'gprof(1)'
GOBJS= $(GCRT0) $(OBJ) $(CRTC)


SRC= $(OBJ:.c=.o)


.SUFFIXES:		.ls .i .cx .cs


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

$(T).prof:		$(OBJ) Makefile
	makedate -m $(T) > makedate.c
	$(CC) -c makedate.c
	$(LD) -o $@ $(LDFLAGS_P) $(MOBJS) $(LIBDIRS) $(LIBS) $(CRTN) > $(T).lm

$(T).gprof:		$(OBJ) Makefile
	makedate -m $(T) > makedate.c
	$(CC) -c makedate.c
	$(LD) -o $@ $(LDFLAGS_P) $(GOBJS) $(LIBDIRS) $(LIBS) $(CRTN) > $(T).lm

$(T).$(OFF) $(OFF):	$(T).x
	cp -p $(T).x $(T).$(OFF)

$(T).nm nm:		$(T).x
	$(NM) $(NMFLAGS) $(T).x > $(T).nm

$(T).cxref:
	$(CXREF) -c $(CXREFFLAGS) $(SRC) > $(T).cxref

safe:
	makesafe -v=3 -I $(INCDIR) $(OBJ)

strip:			$(T).x
	strip $(T).x
	rm -f $(T).$(OFF) $(T)

install:		install-raw

install-ee:		$(ALL)
	makenewer -r $(ALL) $(BINDIR)

install-raw:		$(T).x
	rm -f $(T)
	ln -f $(T).x $(T)
	rm -f $(BINDIR)/$(T).$(OFF)
	makenewer -r $(T) $(BINDIR)

install-help:		$(T).help
	rm -f $(T)
	ln -f $(T).help $(T)
	-mkdir -p $(HELPDIR) 2> /dev/null
	makenewer $(T) $(HELPDIR)

again:
	rm -f $(ALL) $(T).x

clean:			again
	rm -f *.o

control:
	uname -n > Control
	date >> Control


main.o:			main.c $(INCS)

whatinfo.o:		whatinfo.c config.h

proginfo.o:		proginfo.c defs.h

proginfo_setpiv.o:	proginfo_setpiv.c defs.h

b_$(T).o:		b_$(T).c $(INCS)


getutmpent.o:		getutmpent.c getutmpent.h

getnodedomain.o:	getnodedomain.c filebuf.h


userinfo.o:		userinfo.c userinfo.h

shio.o:			shio.c shio.h

vecstr_loadfile.o:	vecstr_loadfile.c

vecstr_env.o:		vecstr_env.c

vecstr.o:		vecstr.c vecstr.h

filemap.o:		filemap.c filemap.h

filebuf.o:		filebuf.c filebuf.h

realname.o:		realname.c realname.h

ipasswd.o:		ipasswd.c ipasswd.h

lastlogfile.o:		lastlogfile.c lastlogfile.h

tmpx.o:			tmpx.c tmpx.h

nodedb.o:		nodedb.c nodedb.h

clusterdb.o:		clusterdb.c clusterdb.h

kvsfile.o:		kvsfile.c kvsfile.h

babycalc.o:		babycalc.c babycalc.h babiesfu.h

babiesfu.o:		babiesfu.c babiesfu.h cvtdater.h

cvtdater.o:		cvtdater.c cvtdater.h tmz.h

tmz.o:			tmz.c tmz.h

ptm.o:			ptm.c ptm.h

ptma.o:			ptma.c ptma.h


