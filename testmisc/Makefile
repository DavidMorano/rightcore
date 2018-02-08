# MAKEFILE

T= testmisc

ALL= $(T)

SRCROOT= $(LOCAL)


BINDIR= $(SRCROOT)/bin
INCDIR= $(SRCROOT)/include
LIBDIR= $(SRCROOT)/lib
HELPDIR= $(SRCROOT)/share/help

LDCRTDIR= $(CGS_CRTDIR)
VALDIR= $(CGS_VALDIR)


CC= $(CGS_CC)
GPP= $(CGS_GPP)

GPPFLAGS= $(CGS_CCFLAGS)
CCOPTS_GCCALL= -pthreads -fexceptions 
CCOPTS_CCALL= -MT

CCOPTS_GCCOPTSGPP= $(CGS_CCFLAGS)
CCOPTS_GCCOPTSPARC= -mcpu=ultrasparc
CCOPTS_GCCLIBSPARC= -mno-app-regs

CCOPTS= $(CCOPTS_GCCALL) -O $(CCOPTS_GCCOPTSPARC)
#CCOPTS= $(CCOPTS_GCCALL) -g -O
#CCOPTS= $(CCOPTS_GCCALL) -g -pg
#CCOPTS= $(CCOPTS_GCCALL) -g -Wstrict-aliasing
CCOPTS= $(CCOPTS_GCCALL) -g -Wall

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
CCFLAGS= $(GPPFLAGS) $(CFLAGS) 

#LD= $(CC)
#LD= cc
LD= ld


LDRPATH=

LDFLAGS= -m -R$(LIBDIR):$(CGS_RPATH)


LIBDIRS= -L$(LIBDIR) -L$(CGS_LIBDIR)

LIB0=
LIB1= -ldam -lb
LIB2= -ldebug
LIB3= -luc -lu
LIB4= $(CGS_LIBS)
LIB5= -lm
LIB6= -lsecdb -lproject -lpthread -lrt -lxnet -lsocket -lnsl
LIB7= -ldl -lc

LIBS= $(LIB0) $(LIB1) $(LIB2) $(LIB3) $(LIB4) $(LIB5) $(LIB6) $(LIB7)

LIBINFO= $(LIBDIRS) $(LIBS)


TLIBDIRS= -L$(CGS_LIBDIR) -L$(LIBDIR)

TLIB0=
TLIB1= -ldam -lb
TLIB2= -ldebug
TLIB3= -luc -lu
TLIB4= $(CGS_LIBS)
TLIB5= -lm
TLIB6= -lsecdb -lproject -lpthread -lthread -lrt -lxnet -lsocket -lnsl
TLIB7= -ldl -lc

TLIBS= $(TLIB0) $(TLIB1) $(TLIB2) $(TLIB3) $(TLIB4) $(TLIB5) $(TLIB6) $(TLIB7)

TLIBINFO= $(TLIBDIRS) $(TLIBS)


VALUES= $(VALDIR)/$(CGS_VALUES)

CRT1= $(LDCRTDIR)/crt1.o
CRTI= $(LDCRTDIR)/crti.o
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


OBJ00= main.o
OBJ01=
OBJ02=
OBJ03=
OBJ04=
OBJ05=
OBJ06=
OBJ07=

OBJA= $(OBJ00) $(OBJ01) $(OBJ02) $(OBJ03) $(OBJ04) $(OBJ05) $(OBJ06) $(OBJ07)
OBJB= $(OBJ08) $(OBJ09) $(OBJ10) $(OBJ11) $(OBJ12) $(OBJ13) $(OBJ14) $(OBJ15)

OBJ= $(OBJA) $(OBJB)

OBJS= $(CRTFRONT) $(OBJ) $(CRTC) $(CRTBACK)


.SUFFIXES:		.ls .i .cx .cs


default:		$(T).x

all:			$(ALL)

.cc.o:
	$(GPP) -c $(GPPFLAGS) $(CFLAGS) $(CPPFLAGS) $<

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
	$(LD) -o $@ $(LDFLAGS) $(OBJS) $(LIBINFO) > $(T).lm

$(T).$(OFF) $(OFF):	$(T).x
	cp -p $(T).x $(T).$(OFF)

$(T).nm nm:		$(T).x
	$(NM) $(NMFLAGS) $(T).x > $(T).nm

strip:			$(T).x
	strip $(T).x
	rm -f $(T).$(OFF) $(T)

install:		$(ALL)
	bsdinstall $(ALL) $(BINDIR)

install-raw:		$(T).x
	cp -p $(T).x $(T)
	rm -f $(BINDIR)/$(T).$(OFF)
	bsdinstall $(T) $(BINDIR)

again:
	rm -f $(ALL) $(T).x

clean:			again
	makeclean

control:
	uname -n > Control
	date >> Control


main.o:			main.cc $(INCS)
	$(GPP) -c $(CCFLAGS) $(CPPFLAGS) main.cc

whatinfo.o:		whatinfo.c config.h

maininfo.o:		maininfo.c maininfo.h


bstree.o:		bstree.hh
	$(GPP) -c $(CCFLAGS) $(CPPFLAGS) bstree.hh

sethand.o:		sethand.hh
	$(GPP) -c $(CCFLAGS) $(CPPFLAGS) sethand.hh

fifoitem.o:		fifoitem.c fifoitem.h

fifoelem.o:		fifoelem.c fifoelem.h

vechand.o:		vechand.c vechand.h

vecsorthand.o:		vecsorthand.c vecsorthand.h


# special

timeval.o:		timeval.c timeval.h

timespec.o:		timespec.c timespec.h

itimerspec.o:		itimerspec.c itimerspec.h

sigevent.o:		sigevent.c sigevent.h

ciq.o:			ciq.c ciq.h

pq.o:			pq.c pq.h

uc_timeout.o:		uc_timeout.cc timeout.h timespec.h sigevent.h
	$(GPP) -c $(CCFLAGS) $(CPPFLAGS) uc_timeout.cc

uc_sigwaitinfo.o:	uc_sigwaitinfo.c


# testing

T01= testsethand

T01OBJa= $(T01).o
T01OBJb= sethand.o
T01OBJc=
T01OBJd=

T01OBJ= $(T01OBJa) $(T01OBJb) $(T01OBJc) $(T01OBJd)

$(T01).x:		Makefile $(T01OBJ)
	$(GPP) -o $@ $(T01OBJ) $(TLIBINFO)

$(T01).o:		testsethand.cc sethand.hh
	$(GPP) -c $(CCFLAGS) $(CPPFLAGS) testsethand.cc


T02= testsetint

T02OBJa= $(T02).o
T02OBJb=
T02OBJc=
T02OBJd=

T02OBJ= $(T02OBJa) $(T02OBJb) $(T02OBJc) $(T02OBJd)

T02OBJS= $(CRTFRONT) $(T02OBJ) $(CRTC) $(CRTBACK)

$(T02).x:		Makefile $(T02OBJ) 
	$(LD) -o $@ $(LDFLAGS) -z defs $(T02OBJS) -lm $(TLIBINFO) > $(T02).lm

$(T02).o:		testsetint.cc setint.h
	$(GPP) -c $(CCFLAGS) $(CPPFLAGS) testsetint.cc


T03= testop

T03OBJa= $(T03).o
T03OBJb=
T03OBJc=
T03OBJd=

T03OBJ= $(T03OBJa) $(T03OBJb) $(T03OBJc) $(T03OBJd)

T03OBJS= $(CRTFRONT) $(T03OBJ) $(CRTC) $(CRTBACK)

$(T03).x:		Makefile $(T03OBJ) 
	$(LD) -o $@ $(LDFLAGS) -z defs $(T03OBJS) -lm $(TLIBINFO) > $(T03).lm

$(T03).o:		$(T03).cc setint.h
	$(GPP) -c $(CCFLAGS) -Wno-reorder $(CPPFLAGS) $(T03).cc


T04= testnew

T04OBJa= $(T04).o
T04OBJb=
T04OBJc=
T04OBJd=

T04OBJ= $(T04OBJa) $(T04OBJb) $(T04OBJc) $(T04OBJd)

T04OBJS= $(CRTFRONT) $(T04OBJ) $(CRTC) $(CRTBACK)

$(T04).x:		Makefile $(T04OBJ) 
	$(LD) -o $@ $(LDFLAGS) -z defs $(T04OBJS) -lm $(TLIBINFO) > $(T04).lm

$(T04).o:		$(T04).cc setint.h
	$(GPP) -c $(CCFLAGS) $(CPPFLAGS) $(T04).cc


T05= bstree

T05OBJa= main$(T05).o
T05OBJb=
T05OBJc=
T05OBJd=

T05OBJ= $(T05OBJa) $(T05OBJb) $(T05OBJc) $(T05OBJd)

T05OBJS= $(CRTFRONT) $(T05OBJ) $(CRTC) $(CRTBACK)

$(T05).x:		Makefile $(T05OBJ) bstree.hh
	$(LD) -o $@ $(LDFLAGS) -z defs $(T05OBJS) -lm $(TLIBINFO) > $(T05).lm

main$(T05).o:		main$(T05).cc bstree.hh
	$(GPP) -c $(CCFLAGS) $(CPPFLAGS) main$(T05).cc


T06= listadd

T06OBJa= main$(T06).o
T06OBJb=
T06OBJc=
T06OBJd=

T06OBJ= $(T06OBJa) $(T06OBJb) $(T06OBJc) $(T06OBJd)

T06OBJS= $(CRTFRONT) $(T06OBJ) $(CRTC) $(CRTBACK)

$(T06).x:		Makefile $(T06OBJ)
	$(LD) -o $@ $(LDFLAGS) -z defs $(T06OBJS) -lm $(TLIBINFO) > $(T06).lm

main$(T06).o:		main$(T06).cc
	$(GPP) -c $(CCFLAGS) $(CPPFLAGS) main$(T06).cc


T07= forlist

T07OBJa= main$(T07).o
T07OBJb=
T07OBJc=
T07OBJd=

T07OBJ= $(T07OBJa) $(T07OBJb) $(T07OBJc) $(T07OBJd)

T07OBJS= $(CRTFRONT) $(T07OBJ) $(CRTC) $(CRTBACK)

$(T07).x:		Makefile $(T07OBJ)
	$(LD) -o $@ $(LDFLAGS) -z defs $(T07OBJS) -lm $(TLIBINFO) > $(T07).lm

main$(T07).o:		main$(T07).cc
	$(GPP) -c $(CCFLAGS) $(CPPFLAGS) main$(T07).cc


T08= prique

T08OBJa= main$(T08).o
T08OBJb=
T08OBJc=
T08OBJd=

T08OBJ= $(T08OBJa) $(T08OBJb) $(T08OBJc) $(T08OBJd)

T08OBJS= $(CRTFRONT) $(T08OBJ) $(CRTC) $(CRTBACK)

$(T08).x:		Makefile $(T08OBJ)
	$(LD) -o $@ $(LDFLAGS) -z defs $(T08OBJS) -lm $(TLIBINFO) > $(T08).lm

main$(T08).o:		main$(T08).cc
	$(GPP) -c $(CCFLAGS) $(CPPFLAGS) main$(T08).cc


T09= book

T09OBJa= main$(T09).o
T09OBJb=
T09OBJc=
T09OBJd=

T09OBJ= $(T09OBJa) $(T09OBJb) $(T09OBJc) $(T09OBJd)

T09OBJS= $(CRTFRONT) $(T09OBJ) $(CRTC) $(CRTBACK)

$(T09).x:		Makefile $(T09OBJ)
	$(LD) -o $@ $(LDFLAGS) -z defs $(T09OBJS) -lm $(TLIBINFO) > $(T09).lm

main$(T09).o:		main$(T09).cc
	$(GPP) -c $(CCFLAGS) $(CPPFLAGS) main$(T09).cc


T10= lister

T10OBJa= main$(T10).o
T10OBJb=
T10OBJc=
T10OBJd=

T10OBJ= $(T10OBJa) $(T10OBJb) $(T10OBJc) $(T10OBJd)

T10OBJS= $(CRTFRONT) $(T10OBJ) $(CRTBACK)

$(T10).x:		Makefile $(T10OBJ) 
	$(LD) -o $@ $(LDFLAGS) -z defs $(T10OBJS) $(TLIBINFO) > $(T10).lm

main$(T10).o:		main$(T10).cc
	$(GPP) -c $(CCFLAGS) $(CPPFLAGS) main$(T10).cc


T11= quick

T11OBJa= main$(T11).o
T11OBJb= quickselecti.o
T11OBJc= arrswapi.o partitionai.o partidxi.o nthai.o
T11OBJd= debugprintf.o

T11OBJ= $(T11OBJa) $(T11OBJb) $(T11OBJc) $(T11OBJd)

T11OBJS= $(CRTFRONT) $(T11OBJ) $(CRTBACK)

$(T11).x:		Makefile $(T11OBJ) 
	$(LD) -o $@ $(LDFLAGS) -z defs $(T11OBJS) $(TLIBINFO) > $(T11).lm

main$(T11).o:		main$(T11).cc
	$(GPP) -c $(CCFLAGS) $(CPPFLAGS) main$(T11).cc

nthai.o:		nthai.cc
	$(GPP) -c $(CCFLAGS) $(CPPFLAGS) nthai.cc

isort.o:		isort.cc
	$(GPP) -c $(CCFLAGS) $(CPPFLAGS) isort.cc

partitionai.o:		partitionai.c
	$(CC) -c $(CFLAGS) $(CPPFLAGS) partitionai.c


T12= heap

T12OBJa= main$(T12).o
T12OBJb=
T12OBJc=
T12OBJd=

T12OBJ= $(T12OBJa) $(T12OBJb) $(T12OBJc) $(T12OBJd)

T12OBJS= $(CRTFRONT) $(T12OBJ) $(CRTBACK)

$(T12).x:		Makefile $(T12OBJ) 
	$(LD) -o $@ $(LDFLAGS) -z defs $(T12OBJS) $(TLIBINFO) > $(T12).lm

main$(T12).o:		main$(T12).cc
	$(GPP) -c $(CCFLAGS) $(CPPFLAGS) main$(T12).cc


T13= testpart

T13OBJa= testpartitionai.o
T13OBJb= partitionai.o partidxi.o arrswapi.o
T13OBJc=
T13OBJd= debugprintf.o

T13OBJ= $(T13OBJa) $(T13OBJb) $(T13OBJc) $(T13OBJd)

T13OBJS= $(CRTFRONT) $(T13OBJ) $(CRTBACK)

$(T13).x:		Makefile $(T13OBJ) 
	$(LD) -o $@ $(LDFLAGS) -z defs $(T13OBJS) $(TLIBINFO) > $(T13).lm

testpartitionai.o:	testpartitionai.cc
	$(GPP) -c $(CCFLAGS) $(CPPFLAGS) testpartitionai.cc


T14= sorts

T14OBJa= main$(T14).o
T14OBJb= arrswapi.o
T14OBJc= isort.o
T14OBJd= debugprintf.o

T14OBJ= $(T14OBJa) $(T14OBJb) $(T14OBJc) $(T14OBJd)

T14OBJS= $(CRTFRONT) $(T14OBJ) $(CRTBACK)

$(T14).x:		Makefile $(T14OBJ) 
	$(LD) -o $@ $(LDFLAGS) -z defs $(T14OBJS) $(TLIBINFO) > $(T14).lm

main$(T14).o:		main$(T14).cc
	$(GPP) -c $(CCFLAGS) $(CPPFLAGS) main$(T14).cc


T15= quickless

T15OBJa= main$(T15).o
T15OBJb= arrswapi.o
T15OBJc= quickselecti.o partitionai.o
T15OBJd= debugprintf.o

T15OBJ= $(T15OBJa) $(T15OBJb) $(T15OBJc) $(T15OBJd)

T15OBJS= $(CRTFRONT) $(T15OBJ) $(CRTBACK)

$(T15).x:		Makefile $(T15OBJ) 
	$(LD) -o $@ $(LDFLAGS) -z defs $(T15OBJS) $(TLIBINFO) > $(T15).lm

main$(T15).o:		main$(T15).cc
	$(GPP) -c $(CCFLAGS) $(CPPFLAGS) main$(T15).cc


T16= two

T16OBJa= main$(T16).o
T16OBJb=
T16OBJc=
T16OBJd=

T16OBJ= $(T16OBJa) $(T16OBJb) $(T16OBJc) $(T16OBJd)

T16OBJS= $(CRTFRONT) $(T16OBJ) $(CRTBACK)

$(T16).x:		Makefile $(T16OBJ) 
	$(LD) -o $@ $(LDFLAGS) -z defs $(T16OBJS) $(TLIBINFO) > $(T16).lm

main$(T16).o:		main$(T16).c
	$(CC) -c $(CFLAGS) $(CPPFLAGS) main$(T16).c


T17= timeout

T17OBJa= main$(T17).o maininfo.o
T17OBJb= uc_timeout.o uc_sigwaitinfo.o upt.o
T17OBJc= sigaction.o sigevent.o timeout.o timespec.o itimerspec.o 
T17OBJd= pq.o ciq.o vecsorthand.o vechand.o

T17OBJ= $(T17OBJa) $(T17OBJb) $(T17OBJc) $(T17OBJd)

T17OBJS= $(CRTFRONT) $(T17OBJ) $(CRTBACK)

$(T17).x:		Makefile $(T17OBJ) 
	$(LD) -o $@ $(LDFLAGS) -z defs $(T17OBJS) $(TLIBINFO) > $(T17).lm

main$(T17).o:		main$(T17).cc
	$(GPP) -c $(CCFLAGS) $(CPPFLAGS) main$(T17).cc


T18= timer

T18OBJa= main$(T18).o maininfo.o
T18OBJb= uc_timer.o uc_sigwaitinfo.o upt.o
T18OBJc= sigaction.o sigevent.o timeout.o timespec.o itimerspec.o 
T18OBJd= pq.o ciq.o vecsorthand.o

T18OBJ= $(T18OBJa) $(T18OBJb) $(T18OBJc) $(T18OBJd)

T18OBJS= $(CRTFRONT) $(T18OBJ) $(CRTBACK)

$(T18).x:		Makefile $(T18OBJ) 
	$(LD) -o $@ $(LDFLAGS) -z defs $(T18OBJS) $(TLIBINFO) > $(T18).lm

main$(T18).o:		main$(T18).cc
	$(GPP) -c $(CCFLAGS) $(CPPFLAGS) main$(T18).cc


T19= callobj

T19OBJa= main$(T19).o maininfo.o
T19OBJb= uc_timeout.o uc_sigwaitinfo.o upt.o
T19OBJc= sigaction.o sigevent.o timeout.o timespec.o itimerspec.o 
T19OBJd= pq.o ciq.o vecsorthand.o vechand.o

T19OBJ= $(T19OBJa) $(T19OBJb) $(T19OBJc) $(T19OBJd)

T19OBJS= $(CRTFRONT) $(T19OBJ) $(CRTBACK)

$(T19).x:		Makefile $(T19OBJ) 
	$(LD) -o $@ $(LDFLAGS) -z defs $(T19OBJS) $(TLIBINFO) > $(T19).lm

main$(T19).o:		main$(T19).cc
	$(GPP) -c $(CCFLAGS) $(CPPFLAGS) main$(T19).cc


T20= testinit

T20OBJa= main$(T20).o
T20OBJb=
T20OBJc=
T20OBJd=

T20OBJ= $(T20OBJa) $(T20OBJb) $(T20OBJc) $(T20OBJd)

T20OBJS= $(CRTFRONT) $(T20OBJ) $(CRTBACK)

$(T20).x:		Makefile $(T20OBJ) 
	$(LD) -o $@ $(LDFLAGS) -z defs $(T20OBJS) $(TLIBINFO) > $(T20).lm

main$(T20).o:		main$(T20).cc
	$(GPP) -c $(CCFLAGS) $(CPPFLAGS) main$(T20).cc


T21= sing

T21OBJa= main$(T21).o
T21OBJb=
T21OBJc=
T21OBJd=

T21OBJ= $(T21OBJa) $(T21OBJb) $(T21OBJc) $(T21OBJd)

T21OBJS= $(CRTFRONT) $(T21OBJ) $(CRTC) $(CRTBACK)

$(T21).x:		Makefile $(T21OBJ)
	$(LD) -o $@ $(LDFLAGS) -z defs $(T21OBJS) -lm $(TLIBINFO) > $(T21).lm

main$(T21).o:		main$(T21).cc singlist.hh
	$(GPP) -c $(CCFLAGS) $(CPPFLAGS) main$(T21).cc


T22= log

T22OBJa= main$(T22).o
T22OBJb=
T22OBJc=
T22OBJd=

T22OBJ= $(T22OBJa) $(T22OBJb) $(T22OBJc) $(T22OBJd)

T22OBJS= $(CRTFRONT) $(T22OBJ) $(CRTC) $(CRTBACK)

$(T22).x:		Makefile $(T22OBJ)
	$(LD) -o $@ $(LDFLAGS) -z defs $(T22OBJS) -lm $(TLIBINFO) > $(T22).lm

main$(T22).o:		main$(T22).cc singlist.hh
	$(GPP) -c $(CCFLAGS) $(CPPFLAGS) main$(T22).cc


T23= vecint

T23OBJa= main$(T23).o
T23OBJb=
T23OBJc=
T23OBJd=

T23OBJ= $(T23OBJa) $(T23OBJb) $(T23OBJc) $(T23OBJd)

T23OBJS= $(CRTFRONT) $(T23OBJ) $(CRTC) $(CRTBACK)

$(T23).x:		Makefile $(T23OBJ)
	$(LD) -o $@ $(LDFLAGS) -z defs $(T23OBJS) -lm $(TLIBINFO) > $(T23).lm

main$(T23).o:		main$(T23).cc singlist.hh
	$(GPP) -c $(CCFLAGS) $(CPPFLAGS) main$(T23).cc


T24= hide

T24OBJa= main$(T24).o
T24OBJb=
T24OBJc=
T24OBJd=

T24OBJ= $(T24OBJa) $(T24OBJb) $(T24OBJc) $(T24OBJd)

T24OBJS= $(CRTFRONT) $(T24OBJ) $(CRTC) $(CRTBACK)

$(T24).x:		Makefile $(T24OBJ)
	$(LD) -o $@ $(LDFLAGS) -z defs $(T24OBJS) -lm $(TLIBINFO) > $(T24).lm

main$(T24).o:		main$(T24).cc singlist.hh
	$(GPP) -c $(CCFLAGS) $(CPPFLAGS) main$(T24).cc


