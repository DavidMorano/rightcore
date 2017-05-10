# MAKEFILE

T= randomd

ALL= $(T) $(T).$(OFF)

SRCROOT= $(LOCAL)


BINDIR= $(SRCROOT)/bin
INCDIR= $(SRCROOT)/include
LIBDIR= $(SRCROOT)/lib

#LDCRTDIR= /opt/SUNWspro/WS6/lib
#LDCRTDIR= /opt/SUNWspro/SC4.0/lib
#LDCRTDIR= /opt/SUNWspro/lib


CC= gcc
#CCOPTS= -O3 -mcpu=ultrasparc # -fpic
CCOPTS= -g

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
#LD= cc
LD= ld

LDFLAGS= -m -R$(LIBDIR)/$(OFD):$(LIBDIR)


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
CRT= $(CRTI) $(CRT1) $(CRTN)

LINT= lint
LINTFLAGS= -uxn -Dlint $(DEFINES) $(INCDIRS)


OBJ00= main.o whatinfo.o varsub_extras.o expander.o
OBJ01= configfile.o
OBJ02= util.o debug.o varsub_dumpfd.o
OBJ03=
OBJ04= getfname.o
OBJ05= procfileenv.o procfilepaths.o strfieldcmp.o
OBJ06= bopensched.o bopenroot.o
OBJ07= permsched.o
OBJ08= randomvar.o
OBJ09=
OBJ10=
OBJ11=
OBJ12=

OBJA= $(OBJ00) $(OBJ01) $(OBJ02) $(OBJ03) $(OBJ04) $(OBJ05) $(OBJ06) $(OBJ07)
OBJB= $(OBJ08) $(OBJ09) $(OBJ10) $(OBJ11) $(OBJ12)

OBJ= $(OBJA) $(OBJB)

OBJS= $(CRT0) $(OBJ) $(CRTC) 




.SUFFIXES:	.t



default:		$(T).x

all:			$(ALL)

.c.o:
	$(CC) -c $(CFLAGS) $<

.c.t:
	lint -uxn $(DEFINES) $(INCDIRS) $<


$(T):			$(T).sh
	cp -p $(T).sh $(T)

$(T).x:			$(OBJ) Makefile
	makedate -m $(T) > makedate.c
	$(CC) -c makedate.c
	$(LD) -o $@ $(LDFLAGS) $(OBJS) $(LIBDIRS) $(LIBS) $(CRTN) > $(T).lm

$(T).$(OFF) $(OFF):	$(T).x
	cp -p $(T).x $(T).$(OFF)

strip:			$(T).x
	strip $(T).x
	rm -f $(T) $(T).$(OFF)

again:
	rm -f $(ALL) $(T).x

clean:			again
	rm -f *.o

control:
	uname -n > Control
	date >> Control


main.o:			main.c config.h defs.h

whatinfo.o:		whatinfo.c config.h






