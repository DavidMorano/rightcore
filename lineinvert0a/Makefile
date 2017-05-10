# MAKEFILE

T= lineinvert

SRCROOT= $(LOCAL)

BINDIR= $(SRCROOT)/bin
INCDIR= $(SRCROOT)/include
LIBDIR= $(SRCROOT)/lib


CC= gcc

CCOPTS= -O3 -mv8 # -fpic
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

LDFLAGS= -R$(LIBDIR)/$(OFD):$(LIBDIR)


LIBDIRS= -L$(LIBDIR)/$(OFD) -L$(LIBDIR)

LIB0=
LIB1= -ldam -lb -luc
LIB2=
LIB3= -Bstatic -lu -Bdynamic
LIB4= -L$(GNU)/lib -lgcc
LIB5= 
LIB6= -lsecdb -lproject -lpthread -lrt -lxnet -lsocket -lnsl
LIB7= -ldl -lc

LIBS= $(LIB0) $(LIB1) $(LIB2) $(LIB3) $(LIB4) $(LIB5) $(LIB6) $(LIB7)


LIBCC= /opt/SUNWspro/SC4.0/lib

CRTI= $(LIBCC)/crti.o
CRT1= $(LIBCC)/crt1.o
CRTN= $(LIBCC)/crtn.o
CRT0= $(CRTI) $(CRT1)

CRTC= makedate.o
CRT= $(CRTI) $(CRT1) $(CRTN)

LINT= lint
LINTFLAGS= -uxn -Dlint $(DEFS) $(INCDIRS)


OBJ0=
OBJ1= main.o whatinfo.o
OBJ2=

OBJ= $(OBJ0) $(OBJ1) $(OBJ2)


all:			$(T)

.c.o:
	$(CC) -c $(CFLAGS) $<


$(T):			$(OBJ) Makefile
	$(LD) -o $@ $(LDFLAGS) $(CRT0) $(OBJ) $(CRTN) $(LIBDIRS) $(LIBS)

strip:			$(T)
	strip $(T)
	rm -f $(T).$(OFF)

install:		$(T)
	bsdinstall $(T) $(BINDIR)

again:
	rm -f $(ALL) $(T) $(T).x $(T).$(OFF)

clean:			again
	rm -f *.o


main.o:			main.c config.h

whatinfo.o:		whatinfo.c config.h



