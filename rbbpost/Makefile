# MAKEFILE

T= rbbpost

ALL= $(T)

#PCS= /usr/add-on/pcs
SRCROOT= $(PCS)


BINDIR= $(SRCROOT)/bin
INCDIR= $(SRCROOT)/include
LIBDIR= $(SRCROOT)/lib
MANDIR= $(SRCROOT)/man/man1
HELPDIR= $(SRCROOT)/share/help

LDCRTDIR= $(CGS_CRTDIR)
VALDIR= $(CGS_VALDIR)


CC= $(CGS_CC)
GPP= $(CGS_GPP)

GPPFLAGS= $(CGS_CCFLAGS)
CCOPTS_GCCALL= -pthreads -fexceptions
CCOPTS_CCALL= -MT

CCOPTS_GCCOPTSGPP= $(CGS_CCFLAGS)
CCOPTS_GCCOPTSPARC= -mcpu=ultrasparc
CCOPTS_GCCLIBSPARC= -mno-app-regs

CCOPTS= $(CCOPTS_GCCALL) -O2 $(CCOPTS_GCCOPTSPARC)
#CCOPTS= $(CCOPTS_GCCALL) -g -O
#CCOPTS= $(CCOPTS_GCCALL) -g -pg
#CCOPTS= $(CCOPTS_GCCALL) -g -Wstrict-aliasing
#CCOPTS= $(CCOPTS_GCCALL) -g -Wall

CCOPTS_GCCLIB= -fpic $(CCOPTS_GCCLIBSPARC)

#CCOPTS= $(CCOPTS_CCALL) -g -xs

# HyperSPARC
#CCOPTS= -xO5 -xtarget=ss20/hs22 -dalign -xdepend

# UltraSPARC
# regular
#CCOPTS= -xO5 -xtarget=ultra -xsafe=mem -dalign -xdepend
# profiling 'prof(1)'
#CCOPTS= -p -xO5 -xtarget=ultra -xsafe=mem -dalign -xdepend
# profiling 'gprof(1)' 
#CCOPTS= -xpg -xO5 -xtarget=ultra -xsafe=mem -dalign -xdepend


DEF0=
DEF1=
DEF2=
DEF3=
DEF4=
DEF5=
DEF6=
DEF7= $(LF_DEFS)

DEFS= $(DEF0) $(DEF1) $(DEF2) $(DEF3) $(DEF4) $(DEF5) $(DEF6) $(DEF7)


INCDIRS= -I$(INCDIR) -I$(LOCAL)/include

CPPFLAGS= $(DEFS) $(INCDIRS)

CFLAGS= $(CCOPTS)

#LD= $(CC)
#LD= cc
LD= ld


LDRPATH= $(EXTRA)/lib:/usr/lib/lwp

# regular
LDFLAGS= $(LF_LDFLAGS) -R$(LIBDIR):$(CGS_RPATH):$(LDRPATH)
# for profiling
#LDFLAGS= $(LF_LDFLAGS) -L/usr/lib/libp # -R$(LIBDIR)


LIBDIRS= -L$(LIBDIR) -L$(CGS_LIBDIR)

LIB0=
LIB1= -lpcs -ldam -lb
LIB2=
LIB3= -luc -lu
LIB4= $(CGS_LIBS)
LIB5= 
LIB6= -lsecdb -lproject -lpthread -lthread -lrt -lxnet -lsocket -lnsl
LIB7= -ldl -lc

LIBS= $(LIB0) $(LIB1) $(LIB2) $(LIB3) $(LIB4) $(LIB5) $(LIB6) $(LIB7)

LIBINFO= $(LIBDIRS) $(LIBS)


XLIBDIRS= -L$(LIBDIR) -L$(CGS_LIBDIR)

XLIB0=
XLIB1= -lpcs -ldam -lb
XLIB2= -ldebug
XLIB3= -luc -lu
XLIB4= $(CGS_LIBS)
XLIB5= 
XLIB6= -lsecdb -lproject -lpthread -lthread -lrt -lxnet -lsocket -lnsl
XLIB7= -ldl

XLIBS= $(XLIB0) $(XLIB1) $(XLIB2) $(XLIB3) $(XLIB4) $(XLIB5) $(XLIB6) $(XLIB7)

XLIBINFO= $(XLIBDIRS) $(XLIBS)


VALUES= $(VALDIR)/$(CGS_VALUES)

CRT1= $(LDCRTDIR)/crt1.o
CRTI= $(LDCRTDIR)/crti.o
CRTBEGIN= $(LDCRTDIR)/crtbegin.o
MCRT1= $(LDCRTDIR)/mcrt1.o
GCRT1= $(LDCRTDIR)/gcrt1.o
CRTEND= $(LDCRTDIR)/crtend.o
CRTN= $(LDCRTDIR)/crtn.o

CRTFRONT= $(CRT1) $(CRTI) $(VALUES) $(CRTBEGIN)
CRTBACK= $(CRTEND) $(CRTN)

# for regular (no profiling)
CRT0= $(CRT1) $(CRTI) $(VALUES)
# for 'prof(1)'
MCRT0= $(CRTI) $(MCRT1) $(VALUES)
# for 'gprof(1)'
GCRT0= $(CRTI) $(GCRT1) $(VALUES)

CRTC= makedate.o

LINT= lint
LINTFLAGS= -uxn -Dlint

NM= nm
NMFLAGS= -xs -v

CPP= cpp


INCS= config.h defs.h


OBJ00= main.o whatinfo.o proginfo.o proginfo_setpiv.o
OBJ01= proglog.o proguserlist.o prognamecache.o
OBJ02= progprinthdr.o
OBJ03= progarts.o progartmaint.o progmsgid.o
OBJ04= progout.o
OBJ05=
OBJ06= initnow.o bbcmp.o bbcpy.o
OBJ07= printhelp.o 

OBJ08=
OBJ09= ema_haveaddr.o emaentry_getbestaddr.o
OBJ10= outbuf.o outline.o buffer_stropaque.o
OBJ11= logzones.o received.o
OBJ12= ng.o retpath.o stackaddr.o
OBJ13= article.o
OBJ14= bfliner.o
OBJ15=

OBJ16= pcsconf.o pcspoll.o
OBJ17= dater.o emainfo.o
OBJ18= mailmsg.o mailmsg_enver.o mailmsgmatenv.o mailmsghdrs.o
OBJ19= tmz.o
#OBJ20= pcsconfs.o var.o varmk.o
OBJ21= 
OBJ22= hdrextid.o
OBJ23= pcsngdname.o mkartfile.o

OBJA= $(OBJ00) $(OBJ01) $(OBJ02) $(OBJ03) $(OBJ04) $(OBJ05) $(OBJ06) $(OBJ07) 
OBJB= $(OBJ08) $(OBJ09) $(OBJ10) $(OBJ11) $(OBJ12) $(OBJ13) $(OBJ14) $(OBJ15)
OBJC= $(OBJ16) $(OBJ17) $(OBJ18) $(OBJ19) $(OBJ20) $(OBJ21) $(OBJ22) $(OBJ23)

OBJ= $(OBJA) $(OBJB) $(OBJC)

# for regular (no profiling)
OBJS= $(CRTFRONT) $(OBJ) $(CRTC) $(CRTBACK)
# for 'prof(1)'
MOBJS= $(MCRT0) $(OBJ) $(CRTC)
# for 'gprof(1)'
GOBJS= $(GCRT0) $(OBJ) $(CRTC)


SRC= $(OBJ:.c=.o)


.SUFFIXES:		.ls .i .cx .ln


default:		$(T).x

all:			$(ALL)

.cc.o:
	$(GPP) -c $(GPPFLAGS) $(CFLAGS) $(CPPFLAGS) $<

.c.o:
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $<

.c.ln:
	$(LINT) -c -u $(CPPFLAGS) $<

.c.ls:
	$(LINT) $(LINTFLAGS) $(CPPFLAGS) $<

.c.i:
	$(CPP) $(CPPFLAGS) $< > $(*).i


$(T):			$(T).ee
	cp -p $(T).ee $(T)

$(T).x:			$(OBJ) Makefile
	makedate -m $(T) > makedate.c
	$(CC) -c makedate.c
	$(LD) -o $@ $(LDFLAGS) $(OBJS) $(LIBINFO) > $(T).lm

$(T).prof:		$(OBJ) Makefile
	makedate -m $(T) > makedate.c
	$(CC) -c makedate.c
	$(LD) -o $@ $(LDFLAGS) $(MOBJS) $(LIBINFO)

$(T).gprof:		$(OBJ) Makefile
	makedate -m $(T) > makedate.c
	$(CC) -c makedate.c
	$(LD) -o $@ $(LDFLAGS) $(GOBJS) $(LIBINFO)

$(T).$(OFF) $(OFF):	$(T).x
	cp -p $(T).x $(T).$(OFF)

$(T).nm nm:		$(T).x
	$(NM) $(NMFLAGS) $(T).x > $(T).nm

safe:
	makesafe -v=3 -I $(INCDIR) $(OBJ)

strip:			$(T).x
	strip $(T).x
	rm -f $(T).$(OFF) $(T)

install:		install-raw install-help

install-raw:		$(T).x
	rm -f $(BINDIR)/$(T).$(OFF)
	makenewer -r -o rmsuf $(T).x $(BINDIR)
	-chmod u+s ${BINDIR}/$(T)

install-help:		$(T).help
	-mkdir -p $(HELPDIR) 2> /dev/null
	makenewer -o rmsuf $(T).help $(HELPDIR)

again:
	rm -fr $(ALL) $(T).x

clean:			again
	makeclean

control:
	uname -n > Control
	date >> Control

bb.cflow:
	cflow $(INCDIRS) *.c > $@


main.o:			main.c $(INCS) ng.h article.h

whatinfo.o:		whatinfo.c config.h

proginfo.o:		proginfo.c $(INCS)

proginfo_setpiv.o:	proginfo_setpiv.c defs.h

proglog.o:		proglog.c $(INCS)

proguserlist.o:		proguserlist.c $(INCS)

prognamecache.o:	prognamecache.c $(INCS)

progprinthdr.o:		progprinthdr.c $(INCS)

progout.o:		progout.c $(INCS)

progarts.o:		progarts.c $(INCS) ng.h retpath.h article.h

progartmaint.o:		progartmaint.c $(INCS)

progmsgfromname.o:	progmsgfromname.c $(INCS)

progmsgfromaddr.o:	progmsgfromaddr.c $(INCS)

progmsgid.o:		progmsgid.c $(INCS)

broadcast.o:		broadcast.c $(INCS) ng.h retpath.h

forward.o:		forward.c $(INCS) ng.h retpath.h


opendial.o:		opendial.c opendial.h


vecstr_loadfile.o:	vecstr_loadfile.c vecstr.h

vecstr_env.o:		vecstr_env.c vecstr.h

vecstr.o:		vecstr.c vecstr.h

ng.o:			ng.c ng.h

bbhosts.o:		bbhosts.c bbhosts.h

retpath.o:		retpath.c retpath.h

outline.o:		outline.c outline.h

mailmsgmatenv.o:	mailmsgmatenv.c mailmsgmatenv.h

mailmsg.o:		mailmsg.c mailmsg.h

mailmsg_enver.o:	mailmsg_enver.c mailmsg.h

mailmsghdrs.o:		mailmsghdrs.c mailmsghdrs.h

logzones.o:		logzones.c logzones.h

buffer.o:		buffer.c buffer.h

buffer_stropaque.o:	buffer_stropaque.c buffer.h

outbuf.o:		outbuf.c outbuf.h

pcsconf.o:		pcsconf.c pcsconf.h

pcspoll.o:		pcspoll.c pcspoll.h

article.o:		article.c article.h

dater.o:		dater.c dater.h

stackaddr.o:		stackaddr.c stackaddr.h

emainfo.o:		emainfo.c emainfo.h

bfliner.o:		bfliner.c bfliner.h

received.o:		received.c received.h

ema.o:			ema.c ema.h

ema_haveaddr.o:		ema_haveaddr.c ema.h

emaentry_getbestaddr.o:	emaentry_getbestaddr.c ema.h

base64.o:		base64.c base64.h

tmz.o:			tmz.c tmz.h

fsdirtree.o:		fsdirtree.c fsdirtree.h


pcsconfs.o:		pcsconfs.c pcsconfs.h
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $(*).c

var.o:			var.c var.h
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $(*).c

varmk.o:		varmk.c varmk.h
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $(*).c


T01= teststackaddr

$(T01).o:		$(T01).c stackaddr.h

T01OBJ= $(T01).o stackaddr.o strwcmp.o

$(T01).x:		$(T01OBJ) Makefile
	$(CC) -o $@ $(LDFLAGS) $(T01OBJ) $(XLIBDIRS) $(XLIBS)


T02= testpcsngdname

$(T02).o:		$(T02).c

T02OBJ= $(T02).o pcsngdname.o

$(T02).x:		$(T02OBJ) Makefile
	$(CC) -o $@ $(LDFLAGS) $(T02OBJ) $(XLIBDIRS) $(XLIBS)


T03= teststring

$(T03).o:		$(T03).cc

T03OBJ= $(T03).o

$(T03).x:		$(T03OBJ) Makefile
	$(GPP) -o $@ $(LDFLAGS) $(T03OBJ) $(XLIBDIRS) $(XLIBS)


T04= testsendfile

$(T04).o:		$(T04).c opendial.h

T04OBJ= $(T04).o opendial.o dialtcpmux.o

$(T04).x:		$(T04OBJ) Makefile
	$(CC) -o $@ $(LDFLAGS) $(T04OBJ) $(XLIBDIRS) $(XLIBS) -lut


