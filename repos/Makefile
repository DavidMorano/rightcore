# MAKEFILE

T= repos

ALL= $(T).x

SRCROOT= $(LOCAL)


BINDIR= $(SRCROOT)/bin
INCDIR= $(SRCROOT)/include
LIBDIR= $(SRCROOT)/lib
MANDIR= $(SRCROOT)/man/man7
SVCDIR= $(SRCROOT)/lib/opensvcs

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
#CCOPTS= $(CCOPTS_GCCALL) -g -Wstrict-aliasing -Wall

CCOPTS_GCCLIB= -fpic $(CCOPTS_GCCLIBSPARC)


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

LDRPATH= $(EXTRALIBDIR)

LDFLAGS= -m $(LF_LDFLAGS) -R$(LIBDIR)


LIBDIRS= -L$(LIBDIR)

LIB0=
LIB1=
LIB2=
LIB3=
LIB4= -L$(GNU)/lib -lstdc++ -lgcc_eh -lgcc
LIB5= 
LIB6=
LIB7= -lc

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
NMFLAGS= -xs -v

CPP= cpp


INCS= config.h defs.h


OBJ00= main.o
OBJ01= 
OBJ02= nleadstr.o
OBJ03= strnchr.o strwcmp.o
OBJ04=
OBJ05=
OBJ06=
OBJ07=

OBJAa= $(OBJ00) $(OBJ01) $(OBJ02) $(OBJ03)
OBJAb= $(OBJ04) $(OBJ05) $(OBJ06) $(OBJ07)
OBJA= $(OBJAa) $(OBJAb)

OBJ= $(OBJA)

OBJS= $(CRTFRONT) $(OBJ) $(CRTC) $(CRTBACK)

LIBINFO= $(LIBDIRS) $(LIBS)


.SUFFIXES:		.ls .i .cx .cs


default:		$(T).x

all:			$(ALL)

.c.o:
	$(CC) -c $(CFLAGS) $(CCOPTS) $(CPPFLAGS) $<

.cc.o:
	$(GPP) -c $(CFLAGS) $(CCOPTS) $(CPPFLAGS) $<

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

$(T).x:		$(OBJ) Makefile
	makedate -m $(T) > makedate.c
	$(CC) -c $(CFLAGS) makedate.c
	$(LD) -o $@ $(LDFLAGS) $(OBJS) $(LIBINFO) > $(T).lm

install:		$(T).x
	makenewer -r -o rmsuf $(T).x ${BINDIR}

safe:
	makesafe -v=3 -I $(INCDIR) $(OBJ)

strip:			$(T).x
	strip $(T).x
	rm -f $(T).$(OFF)

again:
	rm -f $(ALL) $(T).x $(T).$(OFF)

clean:			again
	makeclean

control:
	(uname -n ; date) > Control


main.o:			main.c

whatinfo.o:		whatinfo.c config.h


