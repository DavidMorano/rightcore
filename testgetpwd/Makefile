# MAKEFILE

T= testgetpwd
 
ALL= $(T) $(T).$(OFF)

SRCROOT= $(HOME)


BINDIR= $(SRCROOT)/bin
INCDIR= $(SRCROOT)/include
LIBDIR= $(SRCROOT)/lib

#LDCRTDIR= /opt/SUNWspro/WS6/lib
#LDCRTDIR= /opt/SUNWspro/SC4.0/lib
#LDCRTDIR= /opt/SUNWspro/lib
LDCRTDIR= $(SRCROOT)/lib


CC= gcc
CCOPTS= -O3 -mcpu=ultrasparc # -fpic
#CCOPTS= -g

# HyperSPARC
#CCOPTS= -xO5 -xtarget=ss20/hs22 -dalign -xdepend

# UltraSPARC
#CCOPTS= -xO5 -xtarget=ultra -xsafe=mem -dalign -xdepend


DEF0= -D$(OSTYPE)=1 -DSOLARIS=1 
DEF1= -DPOSIX=1 -D_POSIX_PTHREAD_SEMANTICS=1 -D_REENTRANT=1
DEF2= -D_POSIX_C_SOURCE=199506L -D_POSIX_PER_PROCESS_TIMER_SOURCE=1
DEF3= -D__EXTENSIONS__=1
DEF4=

DEFS= $(DEF0) $(DEF1) $(DEF2) $(DEF3) $(DEF4)


INCDIRS= -I$(INCDIR)
CFLAGS= $(CCOPTS) $(DEFS) $(INCDIRS)

#LD= $(CC)
#LD= cc
LD= ld
LDFLAGS= -m -R$(LIBDIR)/$(OFD):$(LIBDIR)


LIBDIRS= -L$(LIBDIR)/$(OFD) -L$(LIBDIR)

LIB0=
LIB1= -Bstatic -ldam -Bdynamic
LIB2=
LIB3= -Bstatic -lb -lut -luc -Bdynamic
LIB4= -Bstatic -lu -Bdynamic
LIB5= -L$(GNU)/lib -lgcc
LIB6= -lkstat -lpthread -lsocket -lnsl
LIB7= -lc

LIBS= $(LIB0) $(LIB1) $(LIB2) $(LIB3) $(LIB4) $(LIB5) $(LIB6) $(LIB7)


CRTI= $(LDCRTDIR)/crti.o
CRT1= $(LDCRTDIR)/crt1.o
CRTN= $(LDCRTDIR)/crtn.o

CRT0= $(CRTI) $(CRT1)
CRTC= makedate.o

LINT= lint
LINTFLAGS= -uxn -Dlint $(DEFS) $(INCDIRS)

NM= nm
NMFLAGS= -xs

CPP= cpp


OBJ00= 
OBJ01= main.o whatinfo.o
OBJ02= getpwd.o
OBJ03=
OBJ04=
OBJ05=
OBJ06=
OBJ07=
OBJ08=
OBJ09=
OBJ10=
OBJ11=
OBJ12=
OBJ13=
OBJ14=
OBJ15=
OBJ16=
OBJ17=
OBJ18=
OBJ19=
OBJ20=
OBJ21=
OBJ22=
OBJ23=

OBJA= $(OBJ00) $(OBJ01) $(OBJ02) $(OBJ03) $(OBJ04) $(OBJ05) $(OBJ06) $(OBJ07) 
OBJB= $(OBJ08) $(OBJ09) $(OBJ10) $(OBJ11) $(OBJ12) $(OBJ13) $(OBJ14) $(OBJ15)
OBJC= $(OBJ16) $(OBJ17) $(OBJ18) $(OBJ19) $(OBJ20) $(OBJ21) $(OBJ22) $(OBJ23)

OBJ= $(OBJA) $(OBJB) $(OBJC)

OBJS= $(CRT0) $(OBJ) $(CRTC)




default:		$(T).x

all:			$(ALL)

.c.o:
	$(CC) -c $(CFLAGS) $<

.c.ln:
	$(LINT) -uxn $(DEFS) $(INCDIRS) $<


$(T):			$(T).ee
	cp -p $(T).ee $(T)

$(T).x:			$(OBJ) Makefile
	makedate -m $(T) > makedate.c
	$(CC) -c makedate.c
	$(LD) -o $@ $(LDFLAGS) $(OBJS) $(LIBDIRS) $(LIBS) $(CRTN) > $(T).lm

$(T).$(OFF) $(OFF):	$(T).x
	cp -p $(T).x $(T).$(OFF)

$(T).nm:		$(T).x
	nm -x $(T).x > $(T).nm

strip:			$(T).x
	strip $(T).x
	rm -f $(T) $(T).$(OFF)

install:		$(ALL)
	bsdinstall $(ALL) $(BINDIR)

install-raw:		$(T).x
	cp -p $(T).x $(T)
	rm -f $(BINDIR)/$(T).$(OFF)
	bsdinstall $(T) $(BINDIR)

again:
	rm -fr $(ALL) $(T) $(T).x

clean:			again
	rm -f *.o $(ALL)

control:
	(uname -n ; date) > Control




main.o:			main.c config.h defs.h

whatinfo.o:		whatinfo.c config.h

main.p:			main.c
	$(CPP) $(DEFS) $(INCDIRS) main.c > main.p

main.ln:
	$(LINT) -uxn $(DEFS) $(INCDIRS) $<




