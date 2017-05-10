# MAKEFILE

T= ma
 
ALL= $(T)

SRCROOT= $(PCS)


BINDIR= $(SRCROOT)/bin
INCDIR= $(SRCROOT)/include
LIBDIR= $(SRCROOT)/lib
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
#CCOPTS= -xO4 -xtarget=ss20/hs22 -dalign -xdepend

# UltraSPARC
#CCOPTS= -xO4 -xtarget=ultra -xsafe=mem -dalign -xdepend


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

LDFLAGS= -m $(LF_LDFLAGS) -R$(LIBDIR)


LIBDIRS= -L$(LIBDIR)

LIB0=
LIB1= -ldam -lb
LIB2=
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

CRT0= $(CRT1) $(CRTI) $(VALUES)
CRTC= makedate.o

LINT= lint
LINTFLAGS= -uxn -Dlint

NM= nm
NMFLAGS= -xs

CPP= cpp


INCS= config.h defs.h


OBJ00= main.o whatinfo.o proginfo.o proginfo_setpiv.o
OBJ01= jobdb.o builtin.o standing.o procse.o muximsg.o
OBJ02= configfile.o checklockfile.o
OBJ03= varsub.o varsub_extras.o expander.o
OBJ04= watch.o
OBJ05= handle.o handle_srventry.o field_svcargs.o processargs.o execute.o
OBJ06= srventry.o
OBJ07= printhelp.o 
OBJ08=
OBJ09= varsub_dumpfd.o util.o debug.o 
OBJ10= nlspeername.o kinfo.o msfile.o msfileentry.o
OBJ11= lockfile.o
OBJ12= snddd.o snsds.o
OBJ13= timestr_nist.o
OBJ14= hostent.o headkey.o
OBJ15= procfileenv.o procfilepaths.o strfieldcmp.o
OBJ16= bopensched.o bopenroot.o connection.o
OBJ17= permsched.o
OBJ18= pwfile.o srvtab.o acctab.o
OBJ19= field_srvarg.o
OBJ20= starmat.o logfile.o
OBJ21= sockaddress.o mapstrint.o uc_readlinetimed.o
OBJ22= getfname.o serialbuf.o chmods.o
OBJ23= mktmpdir.o checkdirs.o badback.o 

OBJA= $(OBJ00) $(OBJ01) $(OBJ02) $(OBJ03) $(OBJ04) $(OBJ05) $(OBJ06) $(OBJ07) 
OBJB= $(OBJ08) $(OBJ09) $(OBJ10) $(OBJ11) $(OBJ12) $(OBJ13) $(OBJ14) $(OBJ15)
OBJC= $(OBJ16) $(OBJ17) $(OBJ18) $(OBJ19) $(OBJ20) $(OBJ21) $(OBJ22) $(OBJ23)

OBJ= $(OBJA) $(OBJB) $(OBJC)

OBJS= $(CRTFRONT) $(OBJ) $(CRTC) $(CRTBACK)


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
	$(CC) -c makedate.c
	rm -f $(T).x
	$(LD) -o $@ $(LDFLAGS) $(OBJS) $(LIBDIRS) $(LIBS) > $(T).lm

$(T).$(OFF) $(OFF):	$(T).x
	cp -p $(T).x $(T).$(OFF)

$(T).nm:		$(T).x
	nm -x $(T).x > $(T).nm

safe:
	makesafe -v=3 -I $(INCDIR) $(OBJ)

strip:			$(T).x
	strip $(T).x
	rm -f $(T).$(OFF) $(T)

install:		install-raw

install-raw:		$(T).x
	rm -f $(BINDIR)/$(T).$(OFF)
	makenewer -r -o rmsuf $(T).x $(BINDIR)

again:
	rm -fr $(ALL) $(T) $(T).x

clean:			again
	rm -f *.o $(ALL)

control:
	(uname -n ; date) > Control


main.o:			main.c $(INCS)

whatinfo.o:		whatinfo.c config.h

proginfo.o:		proginfo.c $(INCS)

printhelp.o:		printhelp.c

expander.o:		expander.c $(INCS)

watch.o:		watch.c $(INCS) muximsg.h acctab.h

builtin.o:		builtin.c builtin.h $(INCS) muximsg.h

standing.o:		standing.c standing.h $(INCS) muximsg.h
standing.o:		msfile.h

handle.o:		handle.c $(INCS)

handle_srventry.o:	handle_srventry.c $(INCS)

jobdb.o:		jobdb.c jobdb.h

badback.o:		badback.c $(INCS)

mktmpdir.o:		mktmpdir.c $(INCS)

checklockfile.o:	checklockfile.c $(INCS)


sockaddress.o:		sockaddress.c sockaddress.h

timestr_nist.o:		timestr_nist.c nistinfo.h

muximsg.o:		muximsg.c muximsg.h

pwfile.o:		pwfile.c pwfile.h

kinfo.o:		kinfo.c kinfo.h

logfile.o:		logfile.c logfile.h

varsub.o:		varsub.c varsub.h

msfile.o:		msfile.c msfile.h msfileentry.h

msfileentry.o:		msfileentry.c msfileentry.h

mapstrint.o:		mapstrint.c mapstrint.h

keyopt.o:		keyopt.c keyopt.h

srvtab.o:		srvtab.c srvtab.h

acctab.o:		acctab.c acctab.h


