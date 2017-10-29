# MAKEFILE

T= libuterm

ALL= $(T).a

SRCROOT= $(LOCAL)


BINDIR= $(SRCROOT)/bin
INCDIR= $(SRCROOT)/include
LIBDIR= $(SRCROOT)/lib
MANDIR= $(SRCROOT)/man/man1

#LDCRTDIR= /opt/SUNWspro/WS6/lib
#LDCRTDIR= /opt/SUNWspro/SC5.0/lib
#LDCRTDIR= /opt/SUNWspro/SC4.0/lib
#LDCRTDIR= /opt/SUNWspro/lib
LDCRTDIR= $(SRCROOT)/lib

CC= gcc
GPP= g++

CCOPTS= $(CCOPTS_GCCALL) $(CCOPTS_GCCOPT) $(CCOPTS_GCCLIB)
#CCOPTS= $(CCOPTS_GCCALL) $(CCOPTS_GCCLIB)
#CCOPTS= $(CCOPTS_GCCALL) $(CCOPTS_GCCLIB) -g -Wstrict-aliasing

# HyperSPARC
#CCOPTS= -Kpic -xO5 -xtarget=ss20/hs22 -dalign -xdepend

# UltraSPARC
#CCOPTS= -Kpic -xO5 -xtarget=ultra -xsafe=mem -dalign -xdepend


DEF0= -DOSNAME_$(OSNAME)=$(OSNUM)
DEF1=
DEF2=
DEF3=
DEF4=
DEF5=
DEF6=
DEF7= $(LF_DEFS) 

DEFS= $(DEF0) $(DEF1) $(DEF2) $(DEF3) $(DEF4) $(DEF5)

INCDIRS= -I$(INCDIR)

CPPFLAGS= $(DEFS) $(INCDIRS)

CFLAGS= $(CCOPTS) 

#LD= $(CC)
LD= cc
LD= ld

LDFLAGS= -m -R$(LIBDIR)

LIBDIRS= -L$(LIBDIR)

LIB0=
LIB1= -ldam -lb -luc
LIB2=
LIB3= -Bstatic -lu -Bdynamic
LIB4= -L$(GNU)/lib -lgcc
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
#CRTC= makedate.o

CRT= $(CRTI) $(CRT1) $(CRTN)

LINT= lint
LINTFLAGS= -uxn

NM= nm
NMFLAGS= -vs -x

CPP= cpp


INCS= uterm.h


OBJ0=
OBJ1= uterm.o
OBJ2=
OBJ3= charq.o
OBJ4=
OBJ5=
OBJ6=
OBJ7=

OBJ= $(OBJ0) $(OBJ1) $(OBJ2) $(OBJ3) $(OBJ4) $(OBJ5) $(OBJ6) $(OBJ7)

OBJS= $(CRTFRONT) $(OBJ) $(CRTC) $(CRTBACK)


OBJT= testuterm.o uterm.o charq.o

OBJS= $(CRTFRONT) $(OBJY) $(CRTC) $(CRTBACK)


.SUFFIXES:		.ls .i .cx .cs


default:		all testuterm

all:			$(ALL)

.c.o:
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $<

.cc.o:
	$(GPP) -c $(CFLAGS) $(CPPFLAGS) $<

.c.ln:
	$(LINT) -c -u $(CPPFLAGS) $<

.c.ls:
	$(LINT) $(LINTFLAGS) $(CPPFLAGS) $<


$(T).a:			$(OBJ)
	$(AR) -rc $(T).a $?

$(T).so:		$(OBJS) Makefile localmisc.h $(T).a
	$(LD) -o $@ -G $(LDFLAGS) $(OBJS)

safe:
	makesafe -v=3 -I $(INCDIR) $(OBJ)

again:
	$(RM) $(ALL)

clean:			again
	makeclean

install:		$(ALL) Makefile install.incs
	bsdinstall $(ALL) $(LIBDIR)

install.incs:		$(INCS)
	makenewer $(INCS) -d $(INCDIR) -t $@


testuterm:	$(OBJT) config.h
	$(LD) -o $@ $(CRT0) $(OBJT) $(CRTC) $(CRTN) $(LIBDIRS) $(LIBS)


uterm.o:	uterm.c uterm.h ucb.h charq.h

charq.o:	charq.c charq.h

aiq.o:		aiq.c aiq.h

plainq.o:	plainq.c plainq.h

q.o:		q.c q.h

fdt.o:		fdt.c fdt.h

ucb.o:		ucb.c ucb.h


utty.o:		utty.c ucb.h localmisc.h vsystem.h
	$(CC) -c $(CFLAGS) $*.c
	$(AR) -rc $(T).a $@

vs.o:		vs.c ucb.h

testut.o:	testut.c



