# MAKEFILE

T= referm

ALL= $(T) $(T).$(OFF)

HELPFILE= $(T).help

#SRCROOT= $(LOCAL)


BINDIR= $(SRCROOT)/bin
INCDIR= $(SRCROOT)/include
LIBDIR= $(SRCROOT)/lib



CC= gcc
#CCOPTS= -O3 -mv8 # -fpic
CCOPTS= -g

# HyperSPARC
#CCOPTS= -xO5 -xtarget=ss20/hs22 -dalign -xdepend

# UltraSPARC
#CCOPTS= -xO5 -xtarget=ultra -xsafe=mem -dalign -xdepend


DEFINES1= -DPOSIX=1 -D_POSIX_PTHREAD_SEMANTICS=1 -D_REENTRANT=1
DEFINES2= -D$(OSTYPE)=1 -DSOLARIS=1 

DEFINES= $(DEFINES1) $(DEFINES2)


INCDIRS= -I$(INCDIR)
CFLAGS= $(CCOPTS) $(DEFINES) $(INCDIRS)

#LD= $(CC)
LD= cc
LDFLAGS= -R$(LIBDIR)/$(OFD):$(LIBDIR)


LIBDIRS= -L$(LIBDIR)/$(OFD) -L$(LIBDIR)

LIBS0= -Bstatic -ldam -Bdynamic -lnews
LIBS1= -Bstatic -lb -luc -Bdynamic
LIBS2= -lu
LIBS3= -L$(GNU)/lib -lgcc
LIBS4= -lgen -lpostix4 -lpthread -lsocket -lnsl

LIBS= $(LIBS0) $(LIBS1) $(LIBS2) $(LIBS3) $(LIBS4)


LIBCC= /opt/SUNWspro/SC4.0/lib

#CRTI= $(LIBCC)/crti.o
#CRT1= $(LIBCC)/crt1.o
#CRTN= $(LIBCC)/crtn.o
CRT0= $(CRTI) $(CRT1)

CRTC= makedate.o
CRT= $(CRTI) $(CRT1) $(CRTN)

LINT= lint
LINTFLAGS= -uxn -Dlint $(DEFINES) $(INCDIRS)


OBJ00=
OBJ01= main.o whatinfo.o configfile.o var.o expander.o
OBJ02= process.o
OBJ03= eigendb.o bdb.o 
OBJ04= printhelp.o
OBJ05=
OBJ06=
OBJ07=

OBJ= $(OBJ00) $(OBJ01) $(OBJ02) $(ONJ03) $(OBJ04) $(OBJ05) $(OBJ06) $(OBJ07)

OBJS= $(CRT0) $(OBJ) $(CRTC)



#.SUFFIXES:	.t



default:		$(T).x

all:			$(ALL)

.c.o:
	$(CC) -c $(CFLAGS) $<

.c.ln:
	$(LINT) -u $(DEFINES) $(INCDIRS) $<


$(T):			$(T).ee
	cp -p $(T).ee $(T)

$(T).x:			$(OBJ) Makefile
	makedate > makedate.c
	$(CC) -c makedate.c
	$(LD) -o $@ $(LDFLAGS) $(OBJS) $(LIBDIRS) $(LIBS) $(CRTN)

$(T).$(OFF) $(OFF):	$(T).x
	cp -p $(T).x $(T).$(OFF)

strip:			$(T).x
	strip $(T).x
	rm -f $(T).$(OFF) $(T)

install:		$(ALL) install.help
	bsdinstall $(ALL) $(BINDIR)

install.raw:		$(T).x
	cp -p $(T).x $(T)
	rm -f $(BINDIR)/$(T).$(OFF)
	bsdinatall $(T) $(BINDIR)

install.help:
	mkdir -p $(SRCROOT)/lib/$(T)
	cp -p $(HELPFILE) $(SRCROOT)/lib/$(T)/help

again:
	rm -f $(ALL) $(T).x

clean:			again
	rm -f *.o $(T).x

control:
	(uname -n ; date) > Control



main.o:		main.c config.h defs.h

whatinfo.o:	config.h



