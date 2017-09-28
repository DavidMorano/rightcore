# MAKEFILE

T= testcplus

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

#CCOPTS= $(CCOPTS_GCCALL) -O $(CCOPTS_GCCOPTSPARC)
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

XFLAGS= $(CCOPTS_GCCALL)
XXFLAGS= $(CCOPTS_GCCOPTSGPP) $(CCOPTS_GCCALL) 

CFLAGS= $(CCOPTS)
CCFLAGS= $(CCOPTS_GCCOPTSGPP) $(CCOPTS) 
GPPFLAGS= $(CCOPTS_GCCOPTSGPP) $(CCOPTS) 

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
	echo LDCRTDIR=$(LDCRTDIR)
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


dijkstra1.o:		dijkstra1.cc dijkstra1.hh
	$(GPP) -c $(GPPFLAGS) $(CFLAGS) $(CPPFLAGS) dijkstra1.cc

dijkstra2.o:		dijkstra2.cc dijkstra2.hh
	$(GPP) -c $(GPPFLAGS) $(CFLAGS) $(CPPFLAGS) dijkstra2.cc

bellmanford1.o:		bellmanford1.cc bellmanford1.hh
	$(GPP) -c $(GPPFLAGS) $(CFLAGS) $(CPPFLAGS) bellmanford1.cc

bellmanford2.o:		bellmanford2.cc bellmanford2.hh
	$(GPP) -c $(GPPFLAGS) $(CFLAGS) $(CPPFLAGS) bellmanford2.cc


# testing

T01= testsethand

T01OBJa= $(T01).o
T01OBJb=
T01OBJc=
T01OBJd=

T01OBJ= $(T01OBJa) $(T01OBJb) $(T01OBJc) $(T01OBJd)

T02OBJS= $(CRTFRONT) $(T01OBJ) $(CRTBACK)

$(T01).x:		Makefile $(T01OBJ)
	$(GPP) -o $@ $(T01OBJS) $(TLIBINFO)

$(T01).o:		testsethand.cc sethand.h
	$(GPP) -c $(CCFLAGS) $(CPPFLAGS) testsethand.cc


T02= testsetint

T02OBJa= $(T02).o
T02OBJb=
T02OBJc=
T02OBJd=

T02OBJ= $(T02OBJa) $(T02OBJb) $(T02OBJc) $(T02OBJd)

T02OBJS= $(CRTFRONT) $(T02OBJ) $(CRTBACK)

$(T02).x:		Makefile $(T02OBJ) 
	$(LD) -o $@ $(LDFLAGS) -z defs $(T02OBJS) $(TLIBINFO) > $(T02).lm

$(T02).o:		testsetint.cc setint.h
	$(GPP) -c $(CCFLAGS) $(CPPFLAGS) testsetint.cc


T03= testop

T03OBJa= $(T03).o
T03OBJb=
T03OBJc=
T03OBJd=

T03OBJ= $(T03OBJa) $(T03OBJb) $(T03OBJc) $(T03OBJd)

T03OBJS= $(CRTFRONT) $(T03OBJ) $(CRTBACK)

$(T03).x:		Makefile $(T03OBJ) 
	$(LD) -o $@ $(LDFLAGS) -z defs $(T03OBJS) $(TLIBINFO) > $(T03).lm

$(T03).o:		$(T03).cc setint.h
	$(GPP) -c $(CCFLAGS) $(CPPFLAGS) $(T03).cc


T04= choose

T04OBJa= main$(T04).o permutations.o combinations.o
T04OBJb=
T04OBJc=
T04OBJd=

T04OBJ= $(T04OBJa) $(T04OBJb) $(T04OBJc) $(T04OBJd)

T04OBJS= $(CRTFRONT) $(T04OBJ) $(CRTBACK)

$(T04).x:		Makefile $(T04OBJ) 
	$(LD) -o $@ $(LDFLAGS) -z defs $(T04OBJS) $(TLIBINFO) > $(T04).lm

main$(T04).o:		main$(T04).cc
	$(GPP) -c $(CCFLAGS) $(CPPFLAGS) main$(T04).cc


T05= graph

T05OBJa= main$(T05).o 
T05OBJb= dijkstra1.o dijkstra2.o 
T05OBJc= bellmanford1.o bellmanford2.o
T05OBJd=

T05OBJ= $(T05OBJa) $(T05OBJb) $(T05OBJc) $(T05OBJd)

T05OBJS= $(CRTFRONT) $(T05OBJ) $(CRTBACK)

$(T05).x:		Makefile $(T05OBJ) 
	$(LD) -o $@ $(LDFLAGS) -z defs $(T05OBJS) $(TLIBINFO) > $(T05).lm

main$(T05).o:		main$(T05).cc
	$(GPP) -c $(CCFLAGS) $(CPPFLAGS) main$(T05).cc


T06= trailing

T06OBJa= main$(T06).o
T06OBJb=
T06OBJc=
T06OBJd=

T06OBJ= $(T06OBJa) $(T06OBJb) $(T06OBJc) $(T06OBJd)

T06OBJS= $(CRTFRONT) $(T06OBJ) $(CRTBACK)

$(T06).x:		Makefile $(T06OBJ) 
	$(LD) -o $@ $(LDFLAGS) -z defs $(T06OBJS) $(TLIBINFO) > $(T06).lm

main$(T06).o:		main$(T06).cc
	$(GPP) -c $(CCFLAGS) $(CPPFLAGS) main$(T06).cc


T07= web

T07OBJa= main$(T07).o
T07OBJb=
T07OBJc=
T07OBJd=

T07OBJ= $(T07OBJa) $(T07OBJb) $(T07OBJc) $(T07OBJd)

T07OBJS= $(CRTFRONT) $(T07OBJ) $(CRTBACK)

$(T07).x:		Makefile $(T07OBJ) 
	$(LD) -o $@ $(LDFLAGS) -z defs $(T07OBJS) $(TLIBINFO) > $(T07).lm

main$(T07).o:		main$(T07).cc
	$(GPP) -c $(CCFLAGS) $(CPPFLAGS) main$(T07).cc


T08= bal

T08OBJa= main$(T08).o
T08OBJb=
T08OBJc=
T08OBJd=

T08OBJ= $(T08OBJa) $(T08OBJb) $(T08OBJc) $(T08OBJd)

T08OBJS= $(CRTFRONT) $(T08OBJ) $(CRTBACK)

$(T08).x:		Makefile $(T08OBJ) 
	$(LD) -o $@ $(LDFLAGS) -z defs $(T08OBJS) $(TLIBINFO) > $(T08).lm

main$(T08).o:		main$(T08).cc
	$(GPP) -c $(CCFLAGS) $(CPPFLAGS) main$(T08).cc


T09= strs

T09OBJa= main$(T09).o
T09OBJb=
T09OBJc=
T09OBJd=

T09OBJ= $(T09OBJa) $(T09OBJb) $(T09OBJc) $(T09OBJd)

T09OBJS= $(CRTFRONT) $(T09OBJ) $(CRTBACK)

$(T09).x:		Makefile $(T09OBJ) 
	$(LD) -o $@ $(LDFLAGS) -z defs $(T09OBJS) $(TLIBINFO) > $(T09).lm

main$(T09).o:		main$(T09).cc
	$(GPP) -c $(CCFLAGS) $(CPPFLAGS) main$(T09).cc


T10= minsquares

T10OBJa= main$(T10).o
T10OBJb= igcd.o
T10OBJc=
T10OBJd=

T10OBJ= $(T10OBJa) $(T10OBJb) $(T10OBJc) $(T10OBJd)

T10OBJS= $(CRTFRONT) $(T10OBJ) $(CRTBACK)

$(T10).x:		Makefile $(T10OBJ) 
	$(LD) -o $@ $(LDFLAGS) -z defs $(T10OBJS) $(TLIBINFO) > $(T10).lm

main$(T10).o:		main$(T10).cc
	$(GPP) -c $(XXFLAGS) $(CPPFLAGS) main$(T10).cc


T11= anas

T11OBJa= main$(T11).o
T11OBJb=
T11OBJc=
T11OBJd=

T11OBJ= $(T11OBJa) $(T11OBJb) $(T11OBJc) $(T11OBJd)

T11OBJS= $(CRTFRONT) $(T11OBJ) $(CRTBACK)

$(T11).x:		Makefile $(T11OBJ) 
	$(LD) -o $@ $(LDFLAGS) -z defs $(T11OBJS) $(TLIBINFO) > $(T11).lm

main$(T11).o:		main$(T11).cc
	$(GPP) -c $(XXFLAGS) $(CPPFLAGS) main$(T11).cc


T12= cir

T12OBJa= main$(T12).o
T12OBJb=
T12OBJc=
T12OBJd=

T12OBJ= $(T12OBJa) $(T12OBJb) $(T12OBJc) $(T12OBJd)

T12OBJS= $(CRTFRONT) $(T12OBJ) $(CRTBACK)

$(T12).x:		Makefile $(T12OBJ) 
	$(LD) -o $@ $(LDFLAGS) -z defs $(T12OBJS) $(TLIBINFO) > $(T12).lm

main$(T12).o:		main$(T12).cc
	$(GPP) -c $(XXFLAGS) $(CPPFLAGS) main$(T12).cc


T13= snip20

T13OBJa= main$(T13).o
T13OBJb=
T13OBJc=
T13OBJd=

T13OBJ= $(T13OBJa) $(T13OBJb) $(T13OBJc) $(T13OBJd)

T13OBJS= $(CRTFRONT) $(T13OBJ) $(CRTBACK)

$(T13).x:		Makefile $(T13OBJ) 
	$(LD) -o $@ $(LDFLAGS) -z defs $(T13OBJS) $(TLIBINFO) > $(T13).lm

main$(T13).o:		main$(T13).cc
	$(GPP) -c $(XXFLAGS) $(CPPFLAGS) main$(T13).cc


