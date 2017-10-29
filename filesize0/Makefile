# MAKEFILE

T= filesize

ALL= $(T)


#SRCROOT= $(LOCAL)

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


DEF0= -DOSNAME_$(OSNAME)=$(OSNUM)
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
LD= ld
LD= cc

LDFLAGS= -R$(LIBDIR)

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

LIBCC= /opt/SUNWspro/SC4.0/lib

#CRTI= $(LIBCC)/crti.o
#CRT1= $(LIBCC)/crt1.o
#CRTN= $(LIBCC)/crtn.o
CRT0= $(CRTI) $(CRT1)

CRTC= makedate.o
CRT= $(CRTI) $(CRT1) $(CRTN)

LINT= lint
LINTFLAGS= -u $(DEFS) $(INCDIRS)


OBJ0=
OBJ1= main.o whatinfo.o process.o checkname.o
OBJ2= paramopt.o

OBJ= $(OBJ1) $(OBJ2)

OBJS= $(CRT0) $(OBJ) $(CRTC)


all:		$(ALL)

.c.o:
	$(CC) -c $(CFLAGS) $<

.c.ln:
	$(LINT) -u $(DEFS) $(INCDIRS) $<


$(T):		$(OBJ) Makefile
	makedate > makedate.c
	$(CC) -c $(CFLAGS) makedate.c
	$(LD) -o $@ $(LDFLAGS) $(OBJS) $(LIBDIRS) $(LIBS) $(CRTN)

install:	strip
	bsdinstall $(ALL) $(BINDIR)

strip:		$(T)
	strip $(T)

again:
	rm -f $(ALL) $(T).X

clean:
	rm -f *.o $(ALL) $(T).x

control:
	(date ; uname -n) > Control

cflow:
	cflow -I$(PCS)/include *.c > bb.cflow


main.o:		main.c config.h defs.h

main.ln:	main.c config.h defs.h
	$(LINT) $(LINTFLAGS) main.c

process.o:	process.c config.h defs.h

process.ln:	process.c config.h defs.h
	$(LINT) $(LINTFLAGS) process.c

checkname.o:	checkname.c config.h defs.h




