# MAKEFILE

T= testalarm

ALL= $(T) $(T).x

#SRCROOT= $(LOCAL)


BINDIR= $(SRCROOT)/bin
INCDIR= $(SRCROOT)/include
LIBDIR= $(SRCROOT)/lib

#LDCRTDIR= /opt/SUNWspro/WS6/lib
#LDCRTDIR= /opt/SUNWspro/SC4.0/lib
#LDCRTDIR= /opt/SUNWspro/lib


CC= gcc
#CCOPTS= -O3 -mcpu=ultrasparc # -fpic
CCOPTS=

# HyperSPARC
#CCOPTS= -xO5 -xtarget=ss20/hs22 -dalign -xdepend

# UltraSPARC
#CCOPTS= -xO5 -xtarget=ultra -xsafe=mem -dalign -xdepend


DEFINES0= -D$(OSTYPE)=1 -DSOLARIS=1 
DEFINES1= -DPOSIX=1 -D_POSIX_PTHREAD_SEMANTICS=1 -D_REENTRANT=1
DEFINES2= -D_POSIX_C_SOURCE=199506L -D_POSIX_PER_PROCESS_TIMER_SOURCE=1
DEFINES3= -D__EXTENSIONS__=1
DEFINES4=

DEFINES= $(DEFINES0) $(DEFINES1) $(DEFINES2) $(DEFINES3) $(DEFINES4)


INCDIRS= -I$(INCDIR)
CFLAGS= $(CCOPTS) $(DEFINES) $(INCDIRS)

#LD= $(CC)
LD= ld
LDFLAGS= -R$(LIBDIR)/$(OFD):$(LIBDIR)


LIBDIRS= -L$(LIBDIR)/$(OFD) -L$(LIBDIR)

LIBS0= -Bstatic -ldam -Bdynamic -lnews
LIBS1= -Bstatic -lb -luc -Bdynamic
LIBS2= -lu 
LIBS3= -L$(GNU)/lib -lgcc
LIBS4= -lgen -lpthread -lrt -lsocket -lnsl -lc

LIBS= $(LIBS0) $(LIBS1) $(LIBS2) $(LIBS3) $(LIBS4)


CRTI= $(LDCRTDIR)/crti.o
CRT1= $(LDCRTDIR)/crt1.o
CRTN= $(LDCRTDIR)/crtn.o
CRT0= $(CRTI) $(CRT1)

CRTC= makedate.o

LINT= lint
LINTFLAGS= -uxn -Dlint $(DEFINES) $(INCDIRS)


OBJ0=
OBJ1= main.o whatinfo.o
OBJ2= process.o
OBJ3=
OBJ4=
OBJ5=
OBJ6=
OBJ7=

OBJ= $(OBJ0) $(OBJ1) $(OBJ2) $(OBJ3) $(OBJ4) $(OBJ5) $(OBJ6) $(OBJ7)

OBJS= $(CRT0) $(OBJ) $(CRTC)



all:		$(ALL)

.c.o:
	$(CC) -c $(CFLAGS) $<

.s.o:
	$(AS) -o $@ $<

.c~.c:
	null "want to get $@"

.s~.s:
	null "want to get $@"

.h~.h:
	null "want to get $@"


$(T).x:		$(OBJ) Makefile
	makedate > makedate.c
	$(CC) -c makedate.c
	$(LD) -o $@ $(LDFLAGS) $(OBJS) $(LIBDIRS) $(LIBS) $(CRTN) > $(T).lm

strip:		$(T).x
	strip $(T).x
	rm -f $(T) $(T).$(OFF)

install:	$(T)
	cp -p $(T) $(BINDIR)

again:
	rm -f $(ALL) $(T) $(T).$(OFF)

clean:			again
	rm -f *.o


main.o:		main.c

whatinfo.o:	whatinfo.c



