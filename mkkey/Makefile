# MAKEFILE

T= mkkey
 
ALL= $(T) $(T).$(OFF)

HELPFILE= $(T).help

SRCROOT= $(LOCAL)


BINDIR= $(SRCROOT)/bin
INCDIR= $(SRCROOT)/include
LIBDIR= $(SRCROOT)/lib

#LDCRTDIR= /opt/SUNWspro/WS6/lib
#LDCRTDIR= /opt/SUNWspro/SC4.0/lib
#LDCRTDIR= /opt/SUNWspro/lib


CC= gcc
CCOPTS= -O -mv8 # -fpic
#CCOPTS= -g

# HyperSPARC
#CCOPTS= -xO5 -xtarget=ss20/hs22 -dalign -xdepend

# UltraSPARC
#CCOPTS= -xO5 -xtarget=ultra -xsafe=mem -dalign -xdepend


DEFINES0= -D$(OSTYPE)=1 -DSOLARIS=1 
DEFINES1= -DPOSIX=1 -D_POSIX_PTHREAD_SEMANTICS=1 -D_REENTRANT=1
DEFINES2= -D_POSIX_C_SOURCE=199600L -D_POSIX_PER_PROCESS_TIMER_SOURCE=1
DEFINES3= -D__EXTENSIONS__=1

DEFINES= $(DEFINES0) $(DEFINES1) $(DEFINES2) $(DEFINES3)


INCDIRS= -I$(INCDIR)
CFLAGS= $(CCOPTS) $(DEFINES) $(INCDIRS)

#LD= $(CC)
#LD= cc
LD= ld

LDFLAGS= -m -R$(LIBDIR)/$(OFD):$(LIBDIR)


LIBDIRS= -L$(LIBDIR)/$(OFD) -L$(LIBDIR)

LIBS0= -Bstatic -ldam -Bdynamic -lnews
LIBS1= -Bstatic -lb -luc -Bdynamic
LIBS2= -lu
LIBS3= -L$(GNU)/lib -lgcc
LIBS4= -lgen -lposix4 -lpthread -lsocket -lnsl -lc

LIBS= $(LIBS0) $(LIBS1) $(LIBS2) $(LIBS3) $(LIBS4)


CRTI= $(LDCRTDIR)/crti.o
CRT1= $(LDCRTDIR)/crt1.o
CRTN= $(LDCRTDIR)/crtn.o
CRT0= $(CRTI) $(CRT1)

CRTC= makedate.o
CRT= $(CRTI) $(CRT1) $(CRTN)

LINT= lint
LINTFLAGS= -uxn Dlint $(DEFINES) $(INCDIRS)


OBJ00=
OBJ01= main.o whatinfo.o 
OBJ02= configfile.o expander.o eigendb.o
OBJ03= process.o keys.o
OBJ04= field_word.o
OBJ05= varsub_extras.o
OBJ06= printhelp.o
OBJ07=

OBJ= $(OBJ00) $(OBJ01) $(OBJ02) $(OBJ03) $(OBJ04) $(OBJ05) $(OBJ06) $(OBJ07)

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
	$(LD) -o $@ $(LDFLAGS) $(OBJS) $(LIBDIRS) $(LIBS) $(CRTN) > $(T).lm

$(T).$(OFF) $(OFF):	$(T).x
	cp -p $(T).x $(T).$(OFF)

strip:			$(T).x
	strip $(T).x
	rm -f $(T).$(OFF) $(T)

install:		$(ALL) install.help
	bsdinstall $(ALL) $(BINDIR)

install.raw:		$(T).x install.help
	cp -p $(T).x $(T)
	rm -f $(BINDIR)/$(T).$(OFF)
	bsdinstall $(T) $(BINDIR)

install.help:
	cp -p $(HELPFILE) $(SRCROOT)/lib/$(T)/help

install.$(OFF) $(OFF):	$(T).$(OFF) install.help
	bsdinstall $(ALL) $(BINDIR)
	chmod u+s $(BINDIR)/$(T).$(OFF)

install.x:		$(T).x
	bsdinstall $(T).x $(BINDIR)
	chmod u+s $(BINDIR)/$(T).x

again:
	rm -f $(ALL) $(T) $(T).x

clean:			again
	rm -f *.o

control:
	(uname -n ; date) > Control



main.o:			main.c config.h defs.h

whatinfo.o:		config.h





