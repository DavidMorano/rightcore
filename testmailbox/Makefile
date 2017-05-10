# MAKEFILE

T= testmailbox

ALL= $(T)

#SRCROOT= $(LOCAL)


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
#CCOPTS= $(CCOPTS_GCCALL) -g -Wstrict-aliasing

CCOPTS_GCCLIB= -fpic $(CCOPTS_GCCLIBSPARC)

#CCOPTS= $(CCOPTS_CCALL) -g -xs

# HyperSPARC
# regular
#CCOPTS= -xO5 -xtarget=ss20/hs22 -dalign -xdepend
# profiling 'prof(1)'
#CCOPTS= -p -xO5 -xtarget=ss20/hs22 -dalign -xdepend
# profiling 'gprof(1)' 
#CCOPTS= -xpg -xO5 -xtarget=ss20/hs22 -dalign -xdepend

# UltraSPARC
# regular
#CCOPTS= -xO5 -xtarget=ultra -xsafe=mem -dalign -xdepend
# profiling 'prof(1)'
#CCOPTS= -p -xO5 -xtarget=ultra -xsafe=mem -dalign -xdepend
# profiling 'gprof(1)' 
#CCOPTS= -xpg -xO5 -xtarget=ultra -xsafe=mem -dalign -xdepend


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
LDFLAGS= -m $(LF_LDFLAGS) -R$(LIBDIR)
# for profiling
LDFLAGS_P= -L/usr/lib/libp # -m -R$(LIBDIR)


LIBDIRS= -L$(LIBDIR)

LIB0=
LIB1= -ldam -lb
LIB2= -ldebug
LIB3= -luc -lu
LIB4= -L$(GNU)/lib -lstdc++ -lgcc_eh -lgcc
LIB5= 
LIB6= -lsecdb -lproject -lpthread -lrt -lxnet -lsocket -lnsl
LIB7= -ldl -lc

LIBS= $(LIB0) $(LIB1) $(LIB2) $(LIB3) $(LIB4) $(LIB5) $(LIB6) $(LIB7)


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
CRT0= $(CRT1) $(CRTI) $(VALUES)
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


INCS= config.h defs.h


OBJ00= main.o whatinfo.o proginfo.o proginfo_setpiv.o 
OBJ01= mailbox.o mailmsghdrfold.o mailmsghdrs.o
OBJ02= mbcache.o
OBJ03= 
OBJ04= 
OBJ05=
OBJ06=
OBJ07= printhelp.o 
OBJ08=
OBJ09= mailmsgmatenv.o mailmsgmathdr.o
OBJ10= mailmsg.o mailmsg_loadfile.o mailmsg_enver.o
OBJ11= filebuf.o linefold.o
OBJ12= 
OBJ13= initnow.o mkbestaddr.o
OBJ14= mktmpuserdir.o sfsubstance.o timestr_scandate.o
OBJ15= uc_lockend.o strncpyblanks.o strlinelen.o

OBJA= $(OBJ00) $(OBJ01) $(OBJ02) $(OBJ03) $(OBJ04) $(OBJ05) $(OBJ06) $(OBJ07)
OBJB= $(OBJ08) $(OBJ09) $(OBJ10) $(OBJ11) $(OBJ12) $(OBJ13) $(OBJ14) $(OBJ15)

OBJ= $(OBJA) $(OBJB)

# for regular (no profiling)
OBJS= $(CRTFRONT) $(OBJ) $(CRTC) $(CRTBACK)
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


$(T):			$(T).ee
	cp -p $(T).ee $(T)

$(T).x:			$(OBJ) Makefile
	makedate -m $(T) > makedate.c
	$(CC) -c $(CFLAGS) makedate.c
	$(LD) -o $@ $(LDFLAGS) $(OBJS) $(LIBDIRS) $(LIBS) > $(T).lm

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

install:		install-raw install-help

install-ee:		$(ALL)
	makenewer -r $(ALL) $(BINDIR)

install-raw:		$(T).x
	rm -f $(BINDIR)/$(T).$(OFF)
	makenewer -r -o rmsuf $(T).x $(BINDIR)

install-help:		$(T).help
	-mkdir -p $(HELPDIR) 2> /dev/null
	makenewer -o rmsuf $(T).help $(HELPDIR)

again:
	rm -f $(ALL) $(T).x

clean:			again
	rm -f *.ls *.ln *.i *.o *.so *.x *.lm 

control:
	uname -n > Control
	date >> Control


main.o:			main.c $(INCS) mailbox.h

whatinfo.o:		whatinfo.c config.h

proginfo.o:		proginfo.c defs.h

proginfo_setpiv.o:	proginfo_setpiv.c defs.h


userinfo.o:		userinfo.c userinfo.h

getutmpent.o:		getutmpent.c getutmpent.h


vecstr_loadfile.o:	vecstr_loadfile.c

vecstr_env.o:		vecstr_env.c

filemap.o:		filemap.c filemap.h

filebuf.o:		filebuf.c filebuf.h

linefold.o:		linefold.c linefold.h

realname.o:		realname.c realname.h

ipasswd.o:		ipasswd.c ipasswd.h

lastlogfile.o:		lastlogfile.c lastlogfile.h

tmpx.o:			tmpx.c tmpx.h

mailbox.o:		mailbox.c mailbox.h mailmsghdrs.h mailmsghdrfold.h

mailmsghdrs.o:		mailmsghdrs.c mailmsghdrs.h

mailmsgmatenv.o:	mailmsgmatenv.c mailmsgmatenv.h

mailmsgmathdr.o:	mailmsgmathdr.c mailmsgmathdr.h

mailmsg.o:		mailmsg.c mailmsg.h

mailmsg_loadfile.o:	mailmsg_loadfile.c mailmsg.h

mailmsg_enver.o:	mailmsg_enver.c mailmsg.h

mailmsghdrfold.o:	mailmsghdrfold.c mailmsghdrfold.h

filebuf.o:		filebuf.c filebuf.h

mbcache.o:		mbcache.c mbcache.h mailbox.h mailmsg.h



