# MAKEFILE

T= hangup

ALL= $(T)

SRCROOT= $(LOCAL)


BINDIR= $(SRCROOT)/bin
INCDIR= $(SRCROOT)/include
LIBDIR= $(SRCROOT)/lib

LDCRTDIR= $(SRCROOT)/lib


CC= gcc

CCOPTS= -O3 -mcpu=ultrasparc -fpic
#CCOPTS= -g

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

CFLAGS= $(CCOPTS) $(DEFS) $(INCDIRS)

#LD= $(CC)
#LD= cc
LD= ld

LDFLAGS= -R${LIBDIR}


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


CRTI= $(LDCRTDIR)/crti.o
CRT1= $(LDCRTDIR)/crt1.o
CRTN= $(LDCRTDIR)/crtn.o
CRT0= $(CRTI) $(CRT1)

#CRTC= compiletime.o
CRTC=

LINT= lint
LINTFLAGS= -uxn -Dlint $(DEFS) $(INCDIRS)


OBJ0=
OBJ1= main.o whatinfo.o
OBJ2=
OBJ3= process.o
OBJ4=

OBJ= $(OBJ0) $(OBJ1) $(OBJ2) $(OBJ3) $(OBJ4) $(OBJ5)

OBJS= $(CRT0) $(OBJ) $(CRTC)


default:	$(T).x

all:		$(ALL)

.c.o:
	$(CC) -c $(CFLAGS) $<

.c.ln:
	$(LINT) -u $(DEFS) $(INCDIRS) $<


$(T):			$(T).x
	cp -p $(T).x $(T)

$(T).x:			$(OBJ) Makefile
	$(LD) -o $@ $(LDFLAGS) $(OBJS) $(LIBDIRS) $(LIBS) $(CRTN)

$(T).$(OFF):		$(T).x
	cp -p $(T).x $(T).$(OFF)

install.$(OFF):		$(T).$(OFF)
	bsdinstall $(ALL) $(BINDIR)

install:		$(ALL)
	bsdinstall $(ALL) $(BINDIR)

install-raw:		$(T).x
	cp -p $(T).x $(T)
	rm -f $(BINDIR)/$(T).$(OFF)
	bsdinstall $(T) $(BINDIR)

strip:			$(T).x
	strip $(T).x
	rm -f $(T).$(OFF) $(T)

again:
	rm -f $(ALL)

clean:			again
	rm -f *.o $(ALL)

control:
	(date ; uname -n) > Control

cflow:
	cflow -I$(PCS)/include *.c > bb.cflow


main.o:		main.c config.h defs.h

whatinfo.o:	whatinfo.c config.h

main.li:	main.c config.h defs.h
	$(LINT) $(LINTFLAGS) main.c

process.o:	process.c config.h defs.h

process.li:	process.c config.h defs.h
	$(LINT) $(LINTFLAGS) process.c



