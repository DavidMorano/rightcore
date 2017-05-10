# MAKEFILE

T= prog
TT= testdialerprog

ALL= $(TT).x $(T).so 

SRCROOT= $(LOCAL)


BINDIR= $(SRCROOT)/bin
INCDIR= $(SRCROOT)/include
LIBDIR= $(SRCROOT)/lib
MANDIR= $(SRCROOT)/man/man7
INSDIR= $(SRCROOT)/lib/sysdialers

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

LDFLAGS= -m $(LF_LDFLAGS) -R$(LIBDIR)


SLIBDIRS= -L$(LIBDIR)

SLIB0=
SLIB1= -ldam -lb
SLIB2=
SLIB3= -luc -lu
SLIB4= -L$(GNU)/lib -lstdc++ -lgcc_eh -lgcc
SLIB5= 
SLIB6=
SLIB7=

SLIBS= $(SLIB0) $(SLIB1) $(SLIB2) $(SLIB3) $(SLIB4) $(SLIB5) $(SLIB6) $(SLIB7)

SLIBINFO= $(SLIBDIRS) $(SLIBS)


XLIBDIRS= -L$(LIBDIR)

XLIB0=
XLIB1= -ldam -lb
XLIB2=
XLIB3= -luc -lu
XLIB4= -L$(GNU)/lib -lstdc++ -lgcc_eh -lgcc
XLIB5= 
XLIB6= -lsecdb -lproject -lpthread -lrt -lxnet -lsocket -lnsl
XLIB7= -ldl -lc

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


INCS= config.h defs.h


SOBJ00= prog.o 
SOBJ01= defproc.o
SOBJ02= getev.o
SOBJ03= 
SOBJ04= envs.o envs_procxe.o envs_subs.o
SOBJ05=
SOBJ06=
SOBJ07=
SOBJ08=
SOBJ09=
SOBJ10= sysvar.o dirseen.o
SOBJ11= 
SOBJ12=
SOBJ13=
SOBJ14= hasalldig.o sfthing.o strnnlen.o
SOBJ15= findfile.o procxpath.o

SOBJAa= $(SOBJ00) $(SOBJ01) $(SOBJ02) $(SOBJ03) 
SOBJAb= $(SOBJ04) $(SOBJ05) $(SOBJ06) $(SOBJ07)
SOBJBa= $(SOBJ08) $(SOBJ09) $(SOBJ10) $(SOBJ11) 
SOBJBb= $(SOBJ12) $(SOBJ13) $(SOBJ14) $(SOBJ15)

SOBJA= $(SOBJAa) $(SOBJAb)
SOBJB= $(SOBJBa) $(SOBJBb)

SOBJ= $(SOBJA) $(SOBJB)

SOBJS= $(VALUES) $(SOBJ) $(CRTC)


XOBJ0= main.o proginfo.o proginfo_setpiv.o
XOBJ1= headkeymat.o 
XOBJ2= printhelp.o 
XOBJ3= $(SOBJ)

XOBJ= $(XOBJ0) $(XOBJ1) $(XOBJ2) $(XOBJ3)

XOBJS= $(CRTFRONT) $(XOBJ) $(CRTC) $(CRTBACK)


.SUFFIXES:		.ls .i .cx .cs


default:		$(T).so $(TT).x

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


test:			$(TT).x

$(TT).x:		main.c $(XOBJS) Makefile
	$(LD) -o $@ $(LDFLAGS) $(XOBJS) $(XLIBINFO) > $(TT).lm

$(T):			$(T).ee
	cp -p $(T).ee $(T)

$(T).so:		$(SOBJ) Makefile
	makedate > makedate.c
	$(CC) -c makedate.c
	$(LD) -G -o $@ $(LDFLAGS) $(SOBJ) $(SLIBINFO) > $(T).lm

$(T).$(OFF) $(OFF):	$(T).x
	cp -p $(T).x $(T).$(OFF)

so:			$(T).so

install:		install-so

install-so:		$(T).so
	makenewer -r $(T).so $(INSDIR)

install-raw:		$(T).x
	rm -f $(BINDIR)/$(T).$(OFF)
	makenewer -r -o rmsuf $(T).x $(BINDIR)

$(T).nm nm:		$(T).x
	$(NM) $(NMFLAGS) $(T).x > $(T).nm

safe:
	makesafe -v=3 -I $(INCDIR) $(SOBJ) $(XOBJ)

strip:			$(T).so
	strip $(T).so
	rm -f $(T).$(OFF)

again:
	rm -f $(ALL) $(T).x $(T).$(OFF)

clean:			again
	rm -f *.ls *.ln *.i *.o *.so *.x *.lm

control:
	(uname -n ; date) > Control


main.o:			main.c $(INCS)

whatinfo.o:		whatinfo.c config.h

proginfo.o:		proginfo.c $(INCS)


tcp.o:			tcp.c tcp.h sysdialer.h
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<

tcpnls.o:		tcpnls.c tcpnls.h tcp.h sysdialer.h
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<

tcpmux.o:		tcpmux.c tcpmux.h tcp.h sysdialer.h
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<

prog.o:			prog.c prog.h sysdialer.h envs.h sysvar.h
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


envs.o:			envs.c envs.h
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<

envs_procxe.o:		envs_procxe.c envs.h
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<

envss_ubs.o:		envs_subs.c envs.h
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<


defproc.o:		defproc.c
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<

procxpath.o:		procxpath.c
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<

sperm.o:		sperm.c 
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<

headkeymat.o:		headkeymat.c
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<

hasalldig.o:		hasalldig.c
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<

strnnlen.o:		strnnlen.c
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<

findfile.o:		findfile.c
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<

getev.o:		getev.c
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


