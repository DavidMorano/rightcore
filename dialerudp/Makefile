# MAKEFILE

T= udpfamily

ALL= $(T).so 

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

LDFLAGS= $(LF_LDFLAGS) -R$(LIBDIR) -B direct


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


SOBJ00= udp.o
SOBJ01= 
SOBJ02=
SOBJ03=
SOBJ04=
SOBJ05=
SOBJ06= hostaddr.o
SOBJ07=
SOBJ08= dialudp.o
SOBJ09=
SOBJ10=
SOBJ11=
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


$(T):			$(T).ee
	cp -p $(T).ee $(T)

$(T).so:		$(SOBJ) Makefile
	makedate > makedate.c
	$(CC) -c makedate.c
	$(LD) -G -o $@ -m $(LDFLAGS) $(SOBJ) $(SLIBDIRS) $(SLIBS) > $(T).lm

$(T).$(OFF) $(OFF):	$(T).x
	cp -p $(T).x $(T).$(OFF)

install:		$(ALL)
	makenewer -r $(ALL) $(INSDIR)

install-raw:		$(T).x
	rm -f $(BINDIR)/$(T).$(OFF)
	makenewer -r -o rmsuf $(T).x $(BINDIR)

$(T).nm nm:		$(T).x
	$(NM) $(NMFLAGS) $(T).x > $(T).nm

so:			$(T).so

safe:
	makesafe -v=3 -I $(INCDIR) $(SOBJ)

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


tcp.o:			tcp.c tcp.h sysdialer.h
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<

tcpnls.o:		tcpnls.c tcpnls.h tcp.h sysdialer.h
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<

tcpmux.o:		tcpmux.c tcpmux.h tcp.h sysdialer.h
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<

udp.o:			udp.c udp.h sysdialer.h
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


vecstr_loadfile.o:	vecstr_loadfile.c

vecstr_env.o:		vecstr_env.c

inetaddr.o:		inetaddr.c inetaddr.h
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<

sockaddress.o:		sockaddress.c sockaddress.h
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<

hostent.o:		hostent.c hostent.h
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<

hostaddr.o:		hostaddr.c hostaddr.h
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<


