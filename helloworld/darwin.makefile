# MAKEFILE

T= logdir

ALL= $(T) $(T).$(OFF)

#SRCROOT= $(LOCAL)


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

CCOPTS= -O2
#CCOPTS= -g



DEF0= -DPOSIX=1 -DPTHREAD=1
DEF1= -DOSNAME_$(SYSNAME) -DOSTYPE_$(OSTYPE) -DOSNUM=$(OSNUM) 
DEF2= -D__EXTENSIONS__=1 -D_REENTRANT=1
DEF3= -D_POSIX_C_SOURCE=199506L 
DEF4= -D_POSIX_PTHREAD_SEMANTICS=1 -D_POSIX_PER_PROCESS_TIMER_SOURCE=1
DEF5= $(LF_DEFS)
DEF6=
DEF7=

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
LIB1= -Bstatic -ldam -Bdynamic
LIB2=
LIB3= -Bstatic -lb -luc -Bdynamic
LIB4= -Bstatic -lu -Bdynamic
LIB5= -L$(GNU)/lib -lgcc
LIB6= -lsecdb -lproject -lrt -lpthread -lsocket -lnsl
LIB7= -ldl -lc

#LIBS= $(LIB0) $(LIB1) $(LIB2) $(LIB3) $(LIB4) $(LIB5) $(LIB6) $(LIB7)
LIBS=

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



INCS= $(T)_config.h defs.h


OBJ00=
OBJ01= b_$(T).o
OBJ02=
OBJ03=
OBJ04=
OBJ05=
OBJ06=
OBJ07=
OBJ08= vecstr_env.o
OBJ09= cfdec.o cfdect.o ctdec.o 
OBJ10= strwcpy.o sncpy1.o sncpy2.o sncpy3.o 
OBJ11= gecosname.o strbasename.o
OBJ12= vecstr.o char.o
OBJ13= strnlen.o strnchr.o strnpbrk.o
OBJ14= matpstr.o vstrkeycmp.o strkeycmp.o
OBJ15= uc_malloc.o uc_realloc.o

OBJA= $(OBJ00) $(OBJ01) $(OBJ02) $(OBJ03) $(OBJ04) $(OBJ05) $(OBJ06) $(OBJ07)
OBJB= $(OBJ08) $(OBJ09) $(OBJ10) $(OBJ11) $(OBJ12) $(OBJ13) $(OBJ14) $(OBJ15)

OBJ= $(OBJA) $(OBJB)

OBJS= main.o $(OBJ)



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

$(T).x:			$(OBJS) Makefile
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



