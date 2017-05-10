# MAKEFILE

T= uux
SUT= uunames

ALL= $(T).so $(SUT).so

SRCROOT= $(NCMP)


BINDIR= $(SRCROOT)/bin
INCDIR= $(SRCROOT)/include
LIBDIR= $(SRCROOT)/lib
MANDIR= $(SRCROOT)/man/man7
INSDIR= $(SRCROOT)/lib/sysdialer

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

LDFLAGS= -m $(LF_LDFLAGS) -R$(LIBDIR) -B direct


SLIBDIRS= -L$(LIBDIR) -L$(PCS)/lib

SLIB0=
SLIB1= -lpcs -ldam -lb
SLIB2=
SLIB3= -luc -lu
SLIB4= -L$(GNU)/lib -lstdc++ -lgcc_eh -lgcc
SLIB5=
SLIB6=
SLIB7=

SLIBS= $(SLIB0) $(SLIB1) $(SLIB2) $(SLIB3) $(SLIB4) $(SLIB5) $(SLIB6) $(SLIB7)


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


SOBJ00= uux.o
SOBJ01=
SOBJ02=
SOBJ03= 
SOBJ04=
SOBJ05=
SOBJ06=
SOBJ07=
SOBJ08=
SOBJ09= uuname.o
SOBJ10= 
SOBJ11= dialuux.o pcsgetprogpath.o getprogroot.o nulstr.o
SOBJ12=
SOBJ13=
SOBJ14=
SOBJ15=

SOBJAa= $(SOBJ00) $(SOBJ01) $(SOBJ02) $(SOBJ03) 
SOBJAb= $(SOBJ04) $(SOBJ05) $(SOBJ06) $(SOBJ07)
SOBJBa= $(SOBJ08) $(SOBJ09) $(SOBJ10) $(SOBJ11) 
SOBJBb= $(SOBJ12) $(SOBJ13) $(SOBJ14) $(SOBJ15)

SOBJA= $(SOBJAa) $(SOBJAb)
SOBJB= $(SOBJBa) $(SOBJBb)

SOBJ= $(SOBJA) $(SOBJB)


XOBJ= main.o proginfo.o printhelp.o headkeymat.o $(SOBJ)

XOBJS= $(CRTFRONT) $(XOBJ) $(CRTC) $(CRTBACK)


SUSOBJ0= uunames.o vecstr_env.o nulstr.o
SUSOBJ1= pcsgetprogpath.o getprogroot.o strnnlen.o
SUSOBJ2= 

SUSOBJ= $(SUSOBJ0) $(SUSOBJ1) $(SUSOBJ2)


HELPS= uux.help


.SUFFIXES:		.ls .i .cx .cs


default:		$(T).so

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


testdialprog:		main.c $(XOBJS) Makefile
	$(LD) -o $@ $(LDFLAGS) $(XOBJS) $(XLIBDIRS) $(XLIBS)

$(T):			$(T).ee
	cp -p $(T).ee $(T)

$(T).so:		$(SOBJ) Makefile
	makedate > makedate.c
	$(CC) -c makedate.c
	$(LD) -G -o $@ $(LDFLAGS) $(SOBJ) $(SLIBDIRS) $(SLIBS) > $(T).lm

$(SUT).so:		$(SUSOBJ) Makefile $(SUT).map
	$(LD) -G -o $@ $(LDFLAGS) -M $(SUT).map \
		$(SUSOBJ) $(SLIBDIRS) $(SLIBS) > $(SUT).lm

$(T).$(OFF) $(OFF):	$(T).x
	cp -p $(T).x $(T).$(OFF)

so:			$(T).so $(SUT).so

install:		$(ALL)
	-mkdir -p $(INSDIR) 2> /dev/null
	makenewer -r $(T).so $(INSDIR)
	makenewer -r $(SUT).so $(LIBDIR)

install-raw:		$(T).x
	rm -f $(BINDIR)/$(T).$(OFF)
	makenewer -r -o rmsuf $(T).x $(BINDIR)

install-help:		$(T).help
	makeinstall-help $(INSDIR)

install-so:		$(T).so $(SUT).so
	makenewer -r $(T).so $(INSDIR)
	makenewer -r $(SUT).so $(LIBDIR)

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
	rm -f $(ALL) *.o

control:
	(uname -n ; date) > Control


main.o:			main.c $(INCS)

whatinfo.o:		whatinfo.c config.h

proginfo.o:		proginfo.c defs.h

proginfo_setpiv.o:	proginfo_setpiv.c defs.h


uux.o:			uux.c uux.h sysdialer.h nulstr.h
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<

tcp.o:			tcp.c tcp.h sysdialer.h
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<

tcpnls.o:		tcpnls.c tcpnls.h tcp.h sysdialer.h
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<

tcpmux.o:		tcpmux.c tcpmux.h tcp.h sysdialer.h
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<

prog.o:			prog.c prog.h sysdialer.h
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<


dialprog.o:		dialprog.c
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<

dialuux.o:		dialuux.c uuname.h
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


dialer.o:		dialer.c sysdialer.h
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<

pcsgetprogpath.o:	pcsgetprogpath.c
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<

getprogroot.o:		getprogroot.c nulstr.h dirseen.h
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<


vecstr_loadfile.o:	vecstr_loadfile.c
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<

vecstr_env.o:		vecstr_env.c
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<

vecstr.o:		vecstr.c vecstr.h
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<

sockaddress.o:		sockaddress.c sockaddress.h
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<

inetaddr.o:		inetaddr.c inetaddr.h
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<

hostent.o:		hostent.c hostent.h
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<

ids.o:			ids.c ids.h
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<

dirseen.o:		dirseen.c dirseen.h
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<

nulstr.o:		nulstr.c nulstr.h
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<

vecobj.o:		vecobj.c vecobj.h
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<

uuname.o:		uuname.c uuname.h dirseen.h uunames.h
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<

uunames.o:		uunames.c uunames.h
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<


optbool.o:		optbool.c
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<



