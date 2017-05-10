# MAKEFILE

T= noticed
 
ALL= $(T) $(T).$(OFF)

SRCROOT= $(PCS)


BINDIR= $(SRCROOT)/bin
INCDIR= $(SRCROOT)/include
LIBDIR= $(SRCROOT)/lib

#LDCRTDIR= /opt/SUNWspro/WS6/lib
#LDCRTDIR= /opt/SUNWspro/SC5.0/lib
#LDCRTDIR= /opt/SUNWspro/SC4.0/lib
#LDCRTDIR= /opt/SUNWspro/lib
LDCRTDIR= $(SRCROOT)/lib


CC= gcc
CCOPTS= -O3 -mcpu=ultrasparc # -fpic
#CCOPTS= -g

# HyperSPARC
#CCOPTS= -xO4 -xtarget=ss20/hs22 -dalign -xdepend

# UltraSPARC
#CCOPTS= -xO4 -xtarget=ultra -xsafe=mem -dalign -xdepend


DEFS0= -D$(OSTYPE)=1 -DSOLARIS=1 
DEFS1= -DPOSIX=1 -D_POSIX_PTHREAD_SEMANTICS=1 -D_REENTRANT=1
DEFS2= -D_POSIX_C_SOURCE=199506L -D_POSIX_PER_PROCESS_TIMER_SOURCE=1
DEFS3= -D__EXTENSIONS__=1
DEFS4=

DEFS= $(DEFS0) $(DEFS1) $(DEFS2) $(DEFS3) $(DEFS4)


INCDIRS= -I$(INCDIR) -I${NCMP}/include
CFLAGS= $(CCOPTS) $(DEFS) $(INCDIRS)

#LD= $(CC)
#LD= cc
LD= ld

LDFLAGS= -m -R$(LIBDIR)/$(OFD):$(LIBDIR)


LIBDIRS= -L$(LIBDIR)/$(OFD) -L$(LIBDIR) -L${NCMP}/lib

LIBS0= -ldes
LIBS1= -Bstatic -lpcs -ldam -Bdynamic -lnews
LIBS2=
LIBS3= -Bstatic -lb -luc -Bdynamic
LIBS4= -Bstatic -lu -Bdynamic
LIBS5= -L$(GNU)/lib -lgcc
LIBS6= -lkstat -lgen -lpthread -lrt -lsocket -lnsl
LIBS7= -lc

LIBS= $(LIBS0) $(LIBS1) $(LIBS2) $(LIBS3) $(LIBS4) $(LIBS5) $(LIBS6) $(LIBS7)


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
OBJ02= configfile.o checklockfile.o
OBJ03= varsub_extras.o expander.o
OBJ04= watch.o
OBJ05= handle.o handle_srventry.o field_svcargs.o processargs.o execute.o
OBJ06= srventry.o builtin.o pwfile.o inputlogin.o passwdok.o
OBJ07= jobdb.o standing.o
OBJ08= debug.o util.o 
OBJ09= varsub_dumpfd.o
OBJ10=
OBJ11= getfname.o findfilepath.o
OBJ12= serialbuf.o listenusd.o
OBJ13=
OBJ14= hostent.o
OBJ15= procfileenv.o procfilepaths.o strfieldcmp.o
OBJ16= bopensched.o bopenroot.o connection.o
OBJ17= permsched.o schedvar.o
OBJ18= srvtab.o acctab.o field_srvarg.o
OBJ19=
OBJ20= starmat.o
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
	rm -f $(T).$(OFF) $(T)

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

builtin.o:		builtin.c builtin.h config.h defs.h

standing.o:		standing.c standing.h config.h defs.h

watch.o:		watch.c config.h defs.h




