# MAKEFILE

T= misc

TT= test

ALL= $(T).so 

SRCROOT= $(EXTRA)
LIBENT= opendialers


BINDIR= $(SRCROOT)/bin
INCDIR= $(SRCROOT)/include
LIBDIR= $(SRCROOT)/lib
MANDIR= $(SRCROOT)/man/man7
DIALERDIR= $(SRCROOT)/lib/$(LIBENT)
PREROOTLIBDIR= $(PREROOT)/lib
PREROOTDIALERDIR= $(PREROOT)/lib/$(LIBENT)

#LDCRTDIR= /opt/SUNWspro/WS6/lib
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
#CCOPTS= $(CCOPTS_GCCALL) -g -Wall

CCOPTS_GCCLIB= -fpic $(CCOPTS_GCCLIBSPARC)

#CCOPTS= $(CCOPTS_CCALL) -g -xs

# HyperSPARC
#CCOPTS= -xO5 -xtarget=ss20/hs22 -dalign -xdepend

# UltraSPARC
#CCOPTS= -xO5 -xtarget=ultra -xsafe=mem -dalign -xdepend


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

LDRPATH= $(PREROOTLIBDIR)

LDFLAGS= -m $(LF_LDFLAGS) -R$(LIBDIR):$(LDRPATH)
SLDFLAGS= $(LDFLAGS) -M $(T).map


LIBDIRS= -L$(LIBDIR)

LIB0=
LIB1= -ldam -lb
LIB2=
LIB3= -luc -lu
LIB4= -L$(GNU)/lib -lstdc++ -lgcc_eh -lgcc
LIB5= 
LIB6= -lproject -lpthread -lrt -lxnet -lsocket -lnsl
LIB7= -ldl -lc

LIBS= $(LIB0) $(LIB1) $(LIB2) $(LIB3) $(LIB4) $(LIB5) $(LIB6) $(LIB7)

LIBINFO= $(LIBDIRS) $(LIBS)


SLIBDIRS= -L$(LIBDIR)

SLIB0=
SLIB1= -ldam -lb
SLIB2=
SLIB3= -luc -lu
SLIB4= -L$(GNU)/lib -lstdc++ -lgcc_eh -lgcc
SLIB5=
SLIB6= -lproject -lpthread -lrt -lxnet -lsocket -lnsl
SLIB7=

SLIBA= $(SLIB0) $(SLIB1) $(SLIB2) $(SLIB3) 
SLIBB= $(SLIB4) $(SLIB5) $(SLIB6) $(SLIB7)

SLIBS= $(SLIBA) $(SLIBB)

SLIBINFO= $(SLIBDIRS) $(SLIBS)


XLIBDIRS= -L$(LIBDIR)

XLIB0=
XLIB1= -ldam -lb
XLIB2= -ldebug
XLIB3= -luc -lut -lu
XLIB4= -L$(GNU)/lib -lstdc++ -lgcc_eh -lgcc
XLIB5= 
XLIB6= -lsecdb -lproject -lpthread -lrt -lxnet -lsocket -lnsl
XLIB7= -ldl

XLIBS= $(XLIB0) $(XLIB1) $(XLIB2) $(XLIB3) $(XLIB4) $(XLIB5) $(XLIB6) $(XLIB7)

XLIBINFO= $(XLIBDIRS) $(XLIBS)


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
NMFLAGS= -xs -v

CPP= cpp


INCS= defs.h


SOBJ00= opendialer_hello.o 
SOBJ01= opendialer_prog.o
SOBJ02= opendialer_tcp.o opendialer_tcpmux.o opendialer_tcpnls.o
SOBJ03= opendialer_uss.o
SOBJ04= opendialer_finger.o
SOBJ05=
SOBJ06=
SOBJ07=

SOBJ08= vecstr_addpath.o
SOBJ09=
SOBJ10=
SOBJ11= 
SOBJ12= 
SOBJ13=
SOBJ14=
SOBJ15=

SOBJ16= getprogpath.o
SOBJ17= getourenv.o
SOBJ18= isNotPresent.o isNotLib.o isSpecialObject.o
SOBJ19=
SOBJ20=
SOBJ21=
SOBJ22=
SOBJ23=

SOBJAa= $(SOBJ00) $(SOBJ01) $(SOBJ02) $(SOBJ03) 
SOBJAb= $(SOBJ04) $(SOBJ05) $(SOBJ06) $(SOBJ07)
SOBJBa= $(SOBJ08) $(SOBJ09) $(SOBJ10) $(SOBJ11) 
SOBJBb= $(SOBJ12) $(SOBJ13) $(SOBJ14) $(SOBJ15)
SOBJCa= $(SOBJ16) $(SOBJ17) $(SOBJ18) $(SOBJ19)
SOBJCb= $(SOBJ20) $(SOBJ21) $(SOBJ22) $(SOBJ23)

SOBJ= $(SOBJAa) $(SOBJAb) $(SOBJBa) $(SOBJBb) $(SOBJCa) $(SOBJCb)

SOBJS= $(VALUES) $(SOBJ) $(CRTC)


AOBJ0= main.o proginfo.o proginfo_setpiv.o
AOBJ1= headkeymat.o 
AOBJ2= printhelp.o 
AOBJ3= $(SOBJ)

AOBJ= $(AOBJ0) $(AOBJ1) $(AOBJ2) $(AOBJ3)

AOBJS= $(VALUES) $(AOBJ) $(CRTC)


.SUFFIXES:		.ls .i .cx .cs


default:		$(T).so

all:			$(ALL)

.c.o:
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<

.cc.o:
	$(GPP) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<

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


test:			$(TT).x

$(TT).x:		main.c $(AOBJ) Makefile
	$(CC) -o $@ $(LDFLAGS) $(AOBJS) $(XLIBDIRS) $(XLIBS) > $(TT).lm

$(T):			$(T).ee
	cp -p $(T).ee $(T)

$(T).so:		$(SOBJ) Makefile $(T).map
	makedate > makedate.c
	$(CC) -c makedate.c
	$(LD) -G -o $@ $(SLDFLAGS) $(SOBJS) $(SLIBDIRS) $(SLIBS) > $(T).lm

$(T).$(OFF) $(OFF):	$(T).x
	cp -p $(T).x $(T).$(OFF)

so:			$(T).so

install:		install-so

install-so:		$(T).so
	makenewer -r $(T).so $(DIALERDIR)
	makenewer -r $(T).so $(PREROOTDIALERDIR)

install-raw:		$(T).x
	rm -f $(BINDIR)/$(T).$(OFF)
	makenewer -r -o rmsuf $(T).x $(BINDIR)

$(T).nm nm:		$(T).x
	$(NM) $(NMFLAGS) $(T).x > $(T).nm

safe:
	makesafe -v=3 -I $(INCDIR) $(SOBJ)

strip:			$(T).so
	strip $(T).so
	rm -f $(T).$(OFF)

again:
	rm -f $(ALL) $(T).x $(T).$(OFF)

clean:			again
	makeclean

control:
	(uname -n ; date) > Control


main.o:			main.c $(INCS)

whatinfo.o:		whatinfo.c config.h

proginfo.o:		proginfo.c $(INCS)


opendialer_hello.o:	opendialer_hello.c opendialer_hello.h
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<

opendialer_prog.o:	opendialer_prog.c opendialer_prog.h
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<

opendialer_tcp.o:	opendialer_tcp.c opendialer_tcp.h
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<

opendialer_tcpnls.o:	opendialer_tcpnls.c opendialer_tcpnls.h
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<

opendialer_tcpmux.o:	opendialer_tcpmux.c opendialer_tcpmux.h
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<

opendialer_uss.o:	opendialer_uss.c opendialer_uss.h
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<

opendialer_finger.o:	opendialer_finger.c opendialer_finger.h
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<


opendialer_loginblueb.o:	opendialer_loginblurb.c opendialer_loginblurb.h
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<

opendialer_fshome.o:		opendialer_fshome.c opendialer_fshome.h
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<

opendialer_org.o:		opendialer_org.c opendialer_org.h
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<

opendialer_issue.o:		opendialer_issue.c opendialer_issue.h
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<

opendialer_motd.o:		opendialer_motd.c opendialer_motd.h
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<

opendialer_imail.o:		opendialer_imail.c opendialer_imail.h
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<

opendialer_termnote.o:		opendialer_termnote.c opendialer_termnote.h
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<

opendialer_statmsg.o:		opendialer_statmsg.c opendialer_statmsg.h
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<


dialprog.o:		dialprog.c
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<

dialuux.o:		dialuux.c
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<

dialudp.o:		dialudp.c
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<

dialtcp.o:		dialtcp.c
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<

dialtcpnls.o:		dialtcpnls.c
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<

dialtcpmux.o:		dialtcpmux.c
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<

dialuss.o:		dialuss.c
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<

dialussnls.o:		dialussnls.c
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<

dialussmux.o:		dialussmux.c
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<


defproc.o:		defproc.c
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<

sperm.o:		sperm.c 
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<

headkeymat.o:		headkeymat.c
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<

ncpu.o:			ncpu.c
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<

hasalldig.o:		hasalldig.c
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<

strnnlen.o:		strnnlen.c
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<

findfile.o:		findfile.c
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<

getev.o:		getev.c
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<

timestr_nist.o:		timestr_nist.c
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<

attachso.o:		attachso.c
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<


vecstr_loadfile.o:	vecstr_loadfile.c
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<

vecstr_env.o:		vecstr_env.c
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<

sockaddress.o:		sockaddress.c sockaddress.h
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<

inetaddr.o:		inetaddr.c inetaddr.h
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<

hostent.o:		hostent.c hostent.h
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<

ids.o:			ids.c ids.h
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<

sysvar.o:		sysvar.c sysvar.h
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<

dirseen.o:		dirseen.c dirseen.h
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<

paramfile.o:		paramfile.c paramfile.h
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<

nulstr.o:		nulstr.c nulstr.h
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<

motd.o:			motd.c motd.h
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<

issue.o:		issue.c issue.h
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<

finduid.o:		finduid.c finduid.h
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<

pwcache.o:		pwcache.c pwcache.h
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<

statmsg.o:		statmsg.c statmsg.h
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<

termnote.o:		termnote.c termnote.h
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<

tmpx.o:			tmpx.c tmpx.h
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<

tmpx_getuserlines.o:	tmpx_getuserlines.c tmpx.h
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<


# testing

XDIAL0= opendialer_uss.o
XDIAL1= opendialer_tcp.o
XDIAL2= dialtcp.o inetpton.o
XDIAL3= uc_opendialer.o uc_openprog.o uc_openproto.o

XDIALERS= $(XDIAL0) $(XDIAL1) $(XDIAL2) $(XDIAL3)

X03= testopendialer

X03OBJ= $(X03).o $(XDIALERS)

$(X03).o:		$(X03).c

$(X03).x:		$(X03OBJ)
	$(CC) -o $@ $(X03OBJ) $(XLIBDIRS) $(XLIBS)


