# MAKEFILE

T= copycd

ALL= $(T)

SRCROOT= $(LOCAL)


BINDIR= $(SRCROOT)/bin
INCDIR= $(SRCROOT)/include
LIBDIR= $(SRCROOT)/lib
HELPDIR= $(SRCROOT)/share/help


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
#CCOPTS= -xO5 -xtarget=ss20/hs22 -dalign -xdepend

# UltraSPARC
#CCOPTS= -xO5 -xtarget=ultra -xsafe=mem -dalign -xdepend


INCS= config.h defs.h


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

CFLAGS= $(CCOPTS) $(DEFS) $(INCDIRS)

#LD= $(CC)
#LD= cc
LD= ld

LDFLAGS= -R$(LIBDIR)


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

CRTC= makedate.o
CRT= $(CRT1) $(CRTI) $(CRTN)

LINT= lint
LINTFLAGS= -u -Dlint $(DEFS) $(INCDIRS)


OBJ0= main.o whatinfo.o proginfo.o proginfo_setpiv.o
OBJ1= 
OBJ2= process.o checkname.o
OBJ3=
OBJ4=
OBJ5=
OBJ6=
OBJ7= printhelp.o

OBJ= $(OBJ0) $(OBJ1) $(OBJ2) $(OBJ3) $(OBJ4) $(OBJ5) $(OBJ6) $(OBJ7)

OBJS= $(CRTFRONT) $(OBJ) $(CRTC) $(CRTBACK)


default:		$(T).x

all:			$(ALL)

.c.o:
	$(CC) -c $(CFLAGS) $<

.cc.o:
	$(GPP) -c $(CFLAGS) $<

.c.ln:
	$(LINT) -u $(DEFS) $(INCDIRS) $<


$(T):			$(T).ee
	cp -p $(T).ee $(T)

$(T).x:			$(OBJ) Makefile
	makedate -m $(T) > makedate.c
	$(CC) -c makedate.c
	$(LD) -o $@ $(LDFLAGS) $(OBJS) $(LIBDIRS) $(LIBS)

install:		install-raw

install.$(OFF):		$(T).$(OFF)
	makenewer -r -o rmsuf $(T).$(OFF) $(BINDIR)

install-raw:		$(T).x
	rm -fr $(BINDIR)/$(T).$(OFF)
	makenewer -o rmsuf $(T).help $(BINDIR)

strip:			$(T).x
	strip $(T).x
	rm -f $(T).$(OFF) $(T)

again:
	rm -f $(ALL) $(T).x

clean:			again
	rm -f *.o

control:
	(date ; uname -n) > Control

cflow:
	cflow -I$(PCS)/include *.c > bb.cflow


main.o:			main.c $(INCS)

whatinfo.o:		whatinfo.c config.h

proginfo.o:		proginfo.c $(INCS)

proginfo_setpiv.o:	proginfo.c $(INCS)

process.o:		process.c $(INCS)

checkname.o:		checkname.c $(INCS)


