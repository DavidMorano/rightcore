# MAKEFILE

T= pcs

TUSERNAME= local
TGROUPNAME= tools

ALL= $(T)

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
# regular
#CCOPTS= -xO5 -xtarget=ss20/hs22 -xdepend
# profiling 'prof(1)'
#CCOPTS= -p -xO5 -xtarget=ss20/hs22 -xdepend
# profiling 'gprof(1)' 
#CCOPTS= -xpg -xO5 -xtarget=ss20/hs22 -xdepend

# UltraSPARC
# regular
#CCOPTS= -xO5 -xtarget=ultra -xsafe=mem -xdepend
# profiling 'prof(1)'
#CCOPTS= -p -xO5 -xtarget=ultra -xsafe=mem -xdepend
# profiling 'gprof(1)' 
#CCOPTS= -xpg -xO5 -xtarget=ultra -xsafe=mem -xdepend


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


LDRPATH= $(EXTRA)/lib

# regular
LDFLAGS= $(LF_LDFLAGS) -R$(LIBDIR):$(LOCAL)/lib:$(CGS_RPATH)
# for profiling
LDFLAGS_P= $(LF_LDFLAGS) -L/usr/lib/libp # -R$(LIBDIR)


LIBDIRS= -L$(LIBDIR) -L$(LOCAL)/lib -L$(CGS_LIBDIR)

LIB0=
LIB1= -lpcs -ldam -lb
LIB2=
LIB3= -luc -lu
LIB4= $(CGS_LIBS)
LIB5= 
LIB6= -lsecdb -lproject -lpthread -lrt -lxnet -lsocket -lnsl
LIB7= -ldl -lc

LIBS= $(LIB0) $(LIB1) $(LIB2) $(LIB3) $(LIB4) $(LIB5) $(LIB6) $(LIB7)

LIBINFO= $(LIBDIRS) $(LIBS)


SLIBDIRS= -L$(LIBDIR) -L$(LOCAL)/lib -L$(CGS_LIBDIR)

SLIB0=
SLIB1= -ldam -lb
SLIB2=
SLIB3= -luc -lu
SLIB4= $(CGS_LIBS)
SLIB5= 
SLIB6= -lsecdb -lproject -lpthread -lrt -lxnet -lsocket -lnsl
SLIB7=

SLIBS= $(SLIB0) $(SLIB1) $(SLIB2) $(SLIB3) $(SLIB4) $(SLIB5) $(SLIB6) $(SLIB7)

SLIBINFO= $(SLIBDIRS) $(SLIBS)


TLIBDIRS= -L$(LIBDIR) -L$(LOCAL)/lib -L$(CGS_LIBDIR)

TLIB0=
TLIB1= -lpcs -ldam -lb
TLIB2= -ldebug
TLIB3= -luc -lu
TLIB4= $(CGS_LIBS)
TLIB5=
TLIB6= -lsecdb -lproject -lpthread -lthread -lrt -lxnet -lsocket -lnsl
TLIB7= -ldl

TLIBS= $(TLIB0) $(TLIB1) $(TLIB2) $(TLIB3) $(TLIB4) $(TLIB5) $(TLIB6) $(TLIB7)

TLIBINFO= $(TLIBDIRS) $(TLIBS)


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

CXREF= cxref
CXREFFLAGS= -R -s

CPP= cpp

LORDER= lorder
TSORT= tsort

RM= rm -f


INCS= pcsmain.h defs.h

PCSINC0= pcsmain.h pcslocinfo.h pcsconfig.h pcsadj.h 
PCSINC1= pcslog.h pcsmsg.h

PCSINCS= $(PCSINC0) $(PCSINC1)


OBJ00= main.o whatinfo.o proginfo.o proginfo_setpiv.o proginfo_rootname.o
OBJ01= proglog.o proguserlist.o
OBJ02= pcsmain.o
OBJ03= pcslocinfo.o pcsconfig.o pcsadj.o pcslog.o pcscmd.o pcsdebug.o
OBJ04=
OBJ05=
OBJ06=
OBJ07= printhelp.o shio.o kshlib.o maininfo.o sesmsg.o msgdata.o
OBJ08= cpuspeed.o securefile.o strval.o
OBJ09=
OBJ10= msfile.o msfilee.o 
OBJ11= ebuf.o
OBJ12=
OBJ13= pcsnsc.o
OBJ14=
OBJ15=

OBJ16= msghdr.o cmsghdr.o
OBJ17=
OBJ18=
OBJ19=
OBJ20=
OBJ21=
OBJ22= isBadSend.o
OBJ23=

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

SOBJ= pcsnsc.o pcsmsg.o


SRC= $(OBJ:.c=.o)


.SUFFIXES:		.ls .i .cx .cs


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

.c.cx:
	$(CXREF) -C $(CXREFFLAGS) $(CPPFLAGS) $<

.c.cs:
	$(CXREF) $(CXREFFLAGS) $(CPPFLAGS) -o $(*).cs $<


$(T):			$(T).x
	rm -f $(T)
	ln -f $(T).x $(T)

$(T).x:			$(OBJ) Makefile
	makedate -m $(T) > makedate.c
	$(CC) -c $(CFLAGS) makedate.c
	$(LD) -o $@ -m $(LDFLAGS) $(OBJS) $(LIBINFO) > $(T).lm

$(T).prof:		$(OBJ) Makefile
	makedate -m $(T) > makedate.c
	$(CC) -c makedate.c
	$(LD) -o $@ $(LDFLAGS_P) $(MOBJS) $(LIBINFO) > $(T).lm

$(T).gprof:		$(OBJ) Makefile
	makedate -m $(T) > makedate.c
	$(CC) -c makedate.c
	$(LD) -o $@ $(LDFLAGS_P) $(GOBJS) $(LIBINFO) > $(T).lm

$(T).$(OFF) $(OFF):	$(T).x
	cp -p $(T).x $(T).$(OFF)

$(T).nm nm:		$(T).x
	$(NM) $(NMFLAGS) $(T).x > $(T).nm

$(T).cxref:
	$(CXREF) -c $(CXREFFLAGS) $(SRC) > $(T).cxref

safe:
	makesafe -v=3 -I $(INCDIR) $(OBJ) $(SOBJ)

strip:			$(T).x
	strip $(T).x
	rm -f $(T).$(OFF) $(T)

install:		install-raw install-help

install-ee:		$(ALL)
	makenewer -r $(ALL) $(BINDIR)

install-raw:		$(T).x
	makenewer -r -o rmsuf $(T).x $(BINDIR)

install-help:		$(T).help
	-mkdir -p $(HELPDIR) 2> /dev/null
	makenewer -o rmsuf $(T).help $(HELPDIR)

again:
	rm -f $(ALL) $(T).x

clean:			again
	rm -f *.ls *.ln *.i *.o *.so *.x *.lm

control:
	uname -n > Control
	date >> Control


main.o:			main.c $(INCS) maininfo.h

whatinfo.o:		whatinfo.c config.h

proginfo.o:		proginfo.c defs.h

proginfo_setpiv_o:	proginfo_setpiv.c defs.h

proginfo_rootname.o:	proginfo_rootname.c defs.h

proglog.o:		proglog.c proglog.h $(INCS)

proguserlist.o:		proguserlist.c proguserlist.h $(INCS)

maininfo.o:		maininfo.c maininfo.h kshlib.h

kshlib.o:		kshlib.c kshlib.h sesmsg.h msgdata.h

sesmsg.o:		sesmsg.c sesmsg.h

msgdata.o:		msgdata.c msgdata.h


# PCS additional code pieces

pcsmain.o:		pcsmain.c $(INCS) $(PCSINCS)

pcscmd.o:		pcscmd.c $(INCS) $(PCSINCS)

pcslocinfo.o:		pcslocinfo.c pcslocinfo.h $(INCS) pcslog.h

pcsadj.o:		pcsadj.c pcsadj.h $(INCS) pcslocinfo.h pcslog.h

pcsconfig.o:		pcsconfig.c pcsconfig.h $(INCS) pcslocinfo.h

pcslog.o:		pcslog.c pcslog.h $(INCS)

pcsdebug.o:		pcsdebug.c $(INCS)


# other standard pieces (duplicated here)

varsub_extras.o:	varsub_extras.c

getnodedomain.o:	getnodedomain.c filebuf.h

getutmpent.o:		getutmpent.c getutmpent.h

getutmpterm.o:		getutmpterm.c getutmpent.h


# standard objects duplicated here

shio.o:			shio.c shio.h

userinfo.o:		userinfo.c userinfo.h

vecstr_loadfile.o:	vecstr_loadfile.c

vecstr_env.o:		vecstr_env.c

filemap.o:		filemap.c filemap.h

filebuf.o:		filebuf.c filebuf.h

realname.o:		realname.c realname.h

ipasswd.o:		ipasswd.c ipasswd.h

lastlogfile.o:		lastlogfile.c lastlogfile.h

tmpx.o:			tmpx.c tmpx.h

msfile.o:		msfile.c msfile.h msfilee.h

msfilee.o:		msfilee.c msfilee.h

lfm.o:			lfm.c lfm.h

paramfile.o:		paramfile.c paramfile.h

vecobj.o:		vecobj.c vecobj.h

varsub.o:		varsub.c varsub.h

ebuf.o:			ebuf.c ebuf.h

serialbuf.o:		serialbuf.c serialbuf.h

spawner.o:		spawner.c spawner.h

listenspec.o:		listenspec.c listenspec.h

pcsmsg.o:		pcsmsg.c pcsmsg.h

pcsnsc.o:		pcsnsc.c pcsnsc.h

pcsclient.o:		pcsclient.c pcsclient.c pcsmsg.o

envmgr.o:		envmgr.c envmgr.h


# testing

T01= testpcs

T01OBJa= $(T01).o pcsnsc.o
T01OBJb= pcsmsg.o
T01OBJc= envmgr.o
T01OBJd= debugprintf.o debug.o

T01OBJ= $(T01OBJa) $(T01OBJb) $(T01OBJc) $(T01OBJd)

testpcs.o:		testpcs.c $(INCS)

$(T01).x:		Makefile $(T01OBJ)
	$(CC) -o $@ $(T01OBJ) $(TLIBINFO)

$(T01).o:		testpcs.c
	$(CC) -c $(CFLAGS) $(CPPFLAGS) testpcs.c


