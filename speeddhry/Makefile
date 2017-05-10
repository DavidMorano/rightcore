# MAKEFILE
# Dhrystone V2.1 and V1.1 Makefile.

T= dhry

V= 1

ALL= $(T).so $(T).o

SRCROOT= $(LOCAL)


BINDIR= $(SRCROOT)/bin
INCDIR= $(SRCROOT)/include
LIBDIR= $(SRCROOT)/lib

#LDCRTDIR= /opt/SUNWspro/WS6/lib
#LDCRTDIR= /opt/SUNWspro/SC5.0/lib
#LDCRTDIR= /opt/SUNWspro/SC4.0/lib
#LDCRTDIR= /opt/SUNWspro/lib
LDCRTDIR= $(SRCROOT)/lib


CC= gcc 
GPP= g++

CCOPTS_GCCOPT= -O3

CCOPTS_GCCALL= -pthreads -fexceptions
CCOPTS_CCALL= -MT

CCOPTS_GCCOPTSPARC= -mcpu=ultrasparc
CCOPTS_GCCLIBSPARC= -mno-app-regs

CCOPTS_GCCLIB= -fpic $(CCOPTS_GCCOPTSPARC) $(CCOPTS_GCCLIBSPARC)

CCOPTS= $(CCOPTS_GCCOPT) $(CCOPTS_GCCALL) $(CCOPTS_GCCLIB) -Wall
CCOPTS= $(CCOPTS_GCCOPT) $(CCOPTS_GCCALL) $(CCOPTS_GCCLIB)

# HyperSPARC
#CCOPTS= -xO5 -xtarget=ss20/hs22 -dalign -xdepend

# UltraSPARC
#CCOPTS= -xO5 -xtarget=ultra -xsafe=mem -dalign -xdepend

# UltraSPARC-v9 (64 bits)
#CCOPTS= -xO5 -xtarget=ultra -xarch=v9 -xsafe=mem -dalign -xdepend


DEF0=
DEF1=
DEF2=
DEF3=
DEF4=
DEF5=
DEF6= -DUNIX=1
DEF7=

DEFS= $(DEF0) $(DEF1) $(DEF2) $(DEF3) $(DEF4) $(DEF5) $(DEF6) $(DEF7)

INCDIRS= -I$(INCDIR)

CPPFLAGS= $(DEFS) $(INCDIRS)

CFLAGS= $(CCOPTS)


LD= ld

LDFLAGSR= -Bdirect -z noversion
LDFLAGS= -Bdirect -z noversion


SLIBDIRS= -L$(LIBDIR)

SLIB0=
SLIB1= -ldam -lb
SLIB2=
SLIB3= -luc -lu
SLIB4= -L$(GNU)/lib -lstdc++ -lgcc_eh -lgcc
SLIB5=  -L/usr/lib
SLIB6= -lsecdb -lproject -lpthread -lrt -lxnet -lsocket -lnsl
SLIB7= -ldl -lc

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

CXREF= cxref
CXREFFLAGS= -R -s

CPP= cpp

LORDER= lorder
TSORT= tsort

RM= rm -f


FILEA= Makefile dhry.doc dhryrun.doc dhry.h dhry21a.c

FILES= $(FILEA) dhry21b.c dhry11.c timers_b.c dhry.bat input


OBJ= dhry21a.o dhry21b.o timers_b.o


default:	all

all:		$(ALL)

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


$(T).o:		$(OBJ) Makefile $(T).map
	$(LD) -r -o $@ -M $(T).map $(LDFLAGSR) $(OBJ)

$(T).so:	$(OBJ) Makefile $(T).map
	$(LD) -G -o $@ -M $(T).map -h $(T).so.$(V) $(LDFLAGS) $(OBJ) \
		$(SLIBDIRS) $(SLIBS)

$(T).nm:		$(T).o
	$(NM) $(NMFLAGS) $(T).o > $(T).nm

install:		$(ALL) Makefile
	-mkdir $(SRCROOT)/lib/$(D) 2> /dev/null
	-mkdir $(SRCROOT)/lib/$(D)/$(OFD) 2> /dev/null
	makenewer -r $(ALL) -d $(SRCROOT)/lib/$(D)/$(OFD)

again:
	rm -f $(ALL)

clean:
	makeclean


