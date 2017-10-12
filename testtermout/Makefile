# MAKEFILE

T= testtermout

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
GPP= g++

CCOPTS_GCCALL= -pthreads -fexceptions
CCOPTS_CCALL= -MT

CCOPTS_GCCOPTSPARC= -mcpu=ultrasparc
CCOPTS_GCCLIBSPARC= -mno-app-regs

CCOPTS= $(CCOPTS_GCCALL) -O $(CCOPTS_GCCOPTSPARC)
#CCOPTS= $(CCOPTS_GCCALL) -g -O
#CCOPTS= $(CCOPTS_GCCALL) -g -pg
#CCOPTS= $(CCOPTS_GCCALL) -g -Wstrict-aliasing -Wall

CCOPTS_GCCLIB= -fpic $(CCOPTS_GCCLIBSPARC)

#CCOPTS= $(CCOPTS_CCALL) -g -xs

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
GLD= g++

# regular
LDFLAGS= -R$(LIBDIR)
GLDFLAGS=
# for profiling
LDFLAGS_P= -L/usr/lib/libp # -R$(LIBDIR)


LIBDIRS= -L$(LIBDIR)

LIB0=
LIB1= -ldam -lb -luc
LIB2=
LIB3= -lu
LIB4= -L$(GNU)/lib -lstdc++ -lgcc_eh -lgcc
LIB5= 
LIB6= -lsecdb -lproject -lpthread -lrt -lxnet -lsocket -lnsl
LIB7= -ldl -lc

LIBS= $(LIB0) $(LIB1) $(LIB2) $(LIB3) $(LIB4) $(LIB5) $(LIB6) $(LIB7)


GLIBDIRS= -L$(LIBDIR)

GLIB0=
GLIB1= -ldam -lb -luc
GLIB2=
GLIB3= -lu
GLIB4=
GLIB5=
GLIB6= -lsecdb -lproject -lpthread -lrt -lxnet -lsocket -lnsl
GLIB7= -ldl

GLIBS= $(GLIB0) $(GLIB1) $(GLIB2) $(GLIB3) $(GLIB4) $(GLIB5) $(GLIB6) $(GLIB7)


CRT1= $(LDCRTDIR)/crt1.o
CRTI= $(LDCRTDIR)/crti.o
VALUES= $(LDCRTDIR)/values-xa.o
CRTBEGIN= $(LDCRTDIR)/crtbegin.o
MCRT1= $(LDCRTDIR)/mcrt1.o
GCRT1= $(LDCRTDIR)/gcrt1.o
CRTEND= $(LDCRTDIR)/crtend.o
CRTN= $(LDCRTDIR)/crtn.o

CRTFRONT= $(CRT1) $(CRTI) $(VALUES) $(CRTBEGIN)
CRTBACK= $(CRTEND) $(CRTN)

# for regular (no profiling)
#CRT0= $(CRT1) $(CRTI) $(VALUES)
# for 'prof(1)'
MCRT0= $(CRT1) $(MCRTI) $(VALUES)
# for 'gprof(1)'
GCRT0= $(CRT1) $(GCRTI) $(VALUES)

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


OBJ00= main.o mainer.o whatinfo.o proginfo.o proginfo_setpiv.o 
OBJ01= 
OBJ02= termout.o
OBJ03=
OBJ04=
OBJ05=
OBJ06=
OBJ07= printhelp.o
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

OBJ= $(OBJA) $(OBJB)

XXOBJ= $(OBJ)

TOBJ= maintermout.o termout.o

# for regular (no profiling)
OBJS= $(CRTFRONT) $(OBJ) $(CRTC) $(CRTBACK)
XXOBJS= $(OBJ) $(CRTC)
# for 'prof(1)'
MOBJS= $(MCRT0) $(OBJ) $(CRTC)
# for 'gprof(1)'
GOBJS= $(GCRT0) $(OBJ) $(CRTC)

TOBJS= $(CRTFRONT) maintermout.o termout.o $(CRTC) $(CRTBACK)


SRC= $(OBJ:.c=.o)


.SUFFIXES:		.ls .i .cx .cs


default:		$(T).xx

all:			$(ALL)

.c.o:
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $<

.cc.o:
	$(GPP) -c $(CFLAGS) $(CPPFLAGS) $<

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


maintermout.x:		$(TOBJ)
	$(LD) -o $@ $(LDFLAGS) $(TOBJS) $(LIBDIRS) $(LIBS)

$(T):			$(T).ee
	cp -p $(T).ee $(T)

$(T).xx:		$(OBJ) Makefile
	makedate -m $(T) > makedate.c
	$(CC) -c $(CFLAGS) makedate.c
	$(GLD) -o $@ $(GLDFLAGS) $(XXOBJS) $(LIBDIRS) $(GLIBS)

$(T).x:			$(XXOBJ) Makefile
	makedate -m $(T) > makedate.c
	$(CC) -c $(CFLAGS) makedate.c
	$(GLD) -o $@ $(LDFLAGS) $(XXOBJS) $(LIBDIRS) $(GLIBS) > $(T).lm

$(T).prof:		$(OBJ) Makefile
	makedate -m $(T) > makedate.c
	$(CC) -c makedate.c
	$(LD) -o $@ $(LDFLAGS_P) $(MOBJS) $(LIBDIRS) $(LIBS) > $(T).lm

$(T).gprof:		$(OBJ) Makefile
	makedate -m $(T) > makedate.c
	$(CC) -c makedate.c
	$(LD) -o $@ $(LDFLAGS_P) $(GOBJS) $(LIBDIRS) $(LIBS) > $(T).lm

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
	ln $(T).x $(T)
	rm -f $(BINDIR)/$(T).$(OFF)
	makeinstall $(BINDIR)

install-help:
	makeinstall-help $(HELPDIR)

again:
	rm -f $(ALL) $(T).x $(T).xx

clean:			again
	rm -f *.ls *.ln *.i *.o *.so *.x *.lm *.xx

control:
	uname -n > Control
	date >> Control


main.o:			main.cc $(INCS)
	$(GPP) -c $(CFLAGS) $(CPPFLAGS) main.cc

mainer.o:		mainer.c $(INCS)

maintermout.o:		maintermout.c $(INCS) termout.h

whatinfo.o:		whatinfo.c config.h

proginfo.o:		proginfo.c defs.h

proginfo_setpiv.o:	proginfo_setpiv.c defs.h


getutmpent.o:		getutmpent.c getutmpent.h

wdt.o:			wdt.c wdt.h


userinfo.o:		userinfo.c userinfo.h

vecstr_loadfile.o:	vecstr_loadfile.c

vecstr_env.o:		vecstr_env.c

vecpstr.o:		vecpstr.c vecpstr.h

filemap.o:		filemap.c filemap.h

filebuf.o:		filebuf.c filebuf.h

realname.o:		realname.c realname.h

ipasswd.o:		ipasswd.c ipasswd.h

lastlogfile.o:		lastlogfile.c lastlogfile.h

tmpx.o:			tmpx.c tmpx.h

fsdirtree.o:		fsdirtree.c fsdirtree.h

fifostr.o:		fifostr.c fifostr.h

hdb.o:			hdb.c hdb.h

termout.o:		termout.cc termout.h ansigr.h
	$(GPP) -c $(GPPFLAGS) $(CFLAGS) $(CPPFLAGS) termout.cc


