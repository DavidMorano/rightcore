# MAKEFILE

T= daytimed

ALL= $(T) $(T).$(OFF)

SRCROOT= $(EXTRA)


BINDIR= $(SRCROOT)/bin
INCDIR= $(SRCROOT)/include
LIBDIR= $(SRCROOT)/lib
HELPDIR= $(SRCROOT)/lib/help

#LDCRTDIR= /opt/SUNWspro/WS6/lib
#LDCRTDIR= /opt/SUNWspro/SC5.0/lib
#LDCRTDIR= /opt/SUNWspro/SC4.0/lib
#LDCRTDIR= /opt/SUNWspro/lib
LDCRTDIR= $(SRCROOT)/lib


CC= gcc

CCOPTS= -O
#CCOPTS= -g


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

CPPFLAGS= $(DEFS) $(INCDIRS)

CFLAGS= $(CCOPTS)

#LD= $(CC)
#LD= cc
LD= ld

# regular
LDFLAGS=

#LIBDIRS= -L$(LIBDIR)
LIBDIRS=

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
MCRT1= $(LDCRTDIR)/mcrt1.o
GCRT1= $(LDCRTDIR)/gcrt1.o
VALUES= $(LDCRTDIR)/values-xa.o
CRTN= $(LDCRTDIR)/crtn.o

# for regular (no profiling)
CRT0= $(CRTI) $(CRT1) $(VALUES)

CRTC= makedate.o

LINT= lint
LINTFLAGS= -uxn

NM= nm
NMFLAGS= -xs -v

CXREF= cxref
CXREFFLAGS= -R -s

CPP= cpp

LORDER= lorder
TSORT= tsort

RM= rm -f


INCS= nistinfo.h


OBJ00=
OBJ01= main.o
OBJ02=
OBJ03=
OBJ04=
OBJ05=
OBJ06=
OBJ07=
OBJ08=
OBJ09= timestr_nist.o
OBJ10= sncpy.o
OBJ11= strnlen.o
OBJ12= bufprintf.o format.o
OBJ13= ctdec.o
OBJ14=
OBJ15= uc_gmtime.o u_getsockopt.o

OBJA= $(OBJ00) $(OBJ01) $(OBJ02) $(OBJ03) $(OBJ04) $(OBJ05) $(OBJ06) $(OBJ07)
OBJB= $(OBJ08) $(OBJ09) $(OBJ10) $(OBJ11) $(OBJ12) $(OBJ13) $(OBJ14) $(OBJ15)

OBJ= $(OBJA) $(OBJB)

OBJS= $(OBJ)


SRC= $(OBJ:.c=.o)


.SUFFIXES:		.ls .i .cx .cs


default:		$(T).x

all:			$(ALL)

.c.o:
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $<

.c.ln:
	$(LINT) -c -u $(CPPFLAGS) $<

.c.ls:
	$(LINT) $(LINTFLAGS) $(CPPFLAGS) $<

.c.i:
	$(CPP) $(CPPFLAGS) $< > $(*).i

.c.cx:
	$(CXREF) -C $(CXREFFLAGS) $(CPPFLAGS) $<

.c.cs:
	$(CXREF) $(CXREFFLAGS) $(CPPFLAGS) -o $(*).cs $<


$(T):			$(T).ee
	cp -p $(T).ee $(T)

$(T).x:			$(OBJS)
	$(CC) -o $@ $(LDFLAGS) $(OBJS) $(LIBDIRS) $(LIBS)

$(T).$(OFF) $(OFF):	$(T).x
	cp -p $(T).x $(T).$(OFF)

$(T).nm nm:		$(T).x
	$(NM) $(NMFLAGS) $(T).x > $(T).nm

$(T).cxref:
	$(CXREF) -c $(CXREFFLAGS) $(SRC) > $(T).cxref

safe:
	makesafe -v -I $(INCDIR) $(OBJ)

strip:			$(T).x
	strip $(T).x
	rm -f $(T).$(OFF) $(T)

install:		$(ALL)
	bsdinstall $(ALL) $(BINDIR)

install.raw:		$(T).x
	rm -f $(T)
	cp -p $(T).x $(T)
	rm -f $(BINDIR)/$(T).$(OFF)
	bsdinstall $(T) $(BINDIR)

install.help:		$(T).help
	rm -f $(T)
	cp -p $(T).help $(T)
	-mkdir -p $(HELPDIR) 2> /dev/null
	bsdinstall $(T) $(HELPDIR)

again:
	rm -f $(ALL) $(T).x

clean:			again
	rm -f *.o

control:
	uname -n > Control
	date >> Control


main.o:			main.c $(INCS)

whatinfo.o:		whatinfo.c config.h

proginfo.o:		proginfo.c $(INCS)

shio.o:			shio.c shio.h defs.h


userinfo.o:		userinfo.c userinfo.h

udomain.o:		udomain.c

getutmpname.o:		getutmpname.c

getnodedomain.o:	getnodedomain.c filebuf.h

printhelp.o:		printhelp.c


# dependencies

cfdec.o:	cfdec.c
cfdect.o:	cfdect.c
ctdec.o:	ctdec.c
matpstr.o:	matpstr.c
sncpy1.o:	sncpy1.c
sncpy2.o:	sncpy2.c
sncpy3.o:	sncpy3.c
strwcpy.o:	strwcpy.c
strbasename.o:	strbasename.c
gecosname.o:	gecosname.c

strnchr.o:	strnchr.c
strnpbrk.o:	strnpbrk.c
vstrkeycmp.o:	vstrkeycmp.c
strkeycmp.o:	strkeycmp.c

uc_malloc.o:	uc_malloc.c

uc_realloc.o:	uc_realloc.c


vecstr_loadfile.o:	vecstr_loadfile.c vecstr.h

vecstr_env.o:		vecstr_env.c vecstr.h

vecstr.o:		vecstr.c vecstr.h

char.o:			char.c char.h

filemap.o:		filemap.c filemap.h

filebuf.o:		filebuf.c filebuf.h

realname.o:		realname.c realname.h

ipasswd.o:		ipasswd.c ipasswd.h

lastlogfile.o:		lastlogfile.c lastlogfile.h

tmpx.o:			tmpx.c tmpx.h

objfile.o:		objfile.c objfile.h



