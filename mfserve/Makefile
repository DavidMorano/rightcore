# MAKEFILE

T= mfserve

TL= svcs

TUSERNAME= local
TGROUPNAME= tools

ALL= $(T) $(TL)

SRCROOT= $(LOCAL)


BINDIR= $(SRCROOT)/bin
INCDIR= $(SRCROOT)/include
LIBDIR= $(SRCROOT)/lib
MANDIR= $(SRCROOT)/man/man1
HELPDIR= $(SRCROOT)/share/help

SVCSDIR= $(LIBDIR)/$(T)/svcs

LDCRTDIR= $(CGS_CRTDIR)
VALDIR= ${CGS_VALDIR}


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


INCDIRS= -I$(INCDIR)

CPPFLAGS= $(DEFS) $(INCDIRS)

CFLAGS= $(CCOPTS)

#LD= $(CC)
#LD= cc
LD= ld


LDRPATH= $(EXTRA)/lib

# regular
LDFLAGS= $(LF_LDFLAGS) -R$(LIBDIR):$(CGS_RPATH):$(LDRPATH)
# shared-object
SLDFLAGS_E= -B symbolic -z nodefs
SLDFLAGS= $(LF_LDFLAGS) -R$(LIBDIR):$(CGS_RPATH):$(LDRPATH)


LIBDIRS= -L$(LIBDIR) -L$(CGS_LIBDIR)

LIB0=
LIB1= -ldam -lb
LIB2=
LIB3= -luc -lu
LIB4= $(CGS_LIBS)
LIB5= 
LIB6= -lsecdb -lproject -lpthread -lrt -lxnet -lsocket -lnsl
LIB7= -ldl -lc

LIBS= $(LIB0) $(LIB1) $(LIB2) $(LIB3) $(LIB4) $(LIB5) $(LIB6) $(LIB7)

LIBINFO= $(LIBDIRS) $(LIBS)


SLIBDIRS= -L$(LIBDIR) -L$(CGS_LIBDIR) 

SLIB0=
SLIB1= -ldam -lb
SLIB2= -ldebug
SLIB3= -luc -lu
SLIB4= $(CGS_LIBS)
SLIB5= 
SLIB6= -lsecdb -lproject -lpthread -lrt -lxnet -lsocket -lnsl
SLIB7=

SLIBS= $(SLIB0) $(SLIB1) $(SLIB2) $(SLIB3) $(SLIB4) $(SLIB5) $(SLIB6) $(SLIB7)

SLIBINFO= $(SLIBDIRS) $(SLIBS)


TLIBDIRS= -L$(CGS_LIBDIR) -L$(LIBDIR)

TLIB0=
TLIB1= -ldam -lb
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


INCS= mfsmain.h defs.h sreqdb.h

MFSINC0= mfsmain.h mfslocinfo.h mfsconfig.h mfslisten.h mfsadj.h 
MFSINC1= mfslog.h mfsmsg.h

MFSINCS= $(MFSINC0) $(MFSINC1)


OBJ00= main.o whatinfo.o proginfo.o proginfo_setpiv.o proginfo_rootname.o
OBJ01= proglog.o proglogout.o proguserlist.o
OBJ02= mfsmain.o
OBJ03= mfslocinfo.o mfsconfig.o mfsadj.o mfslog.o mfscmd.o mfsdebug.o
OBJ04= mfslisten.o mfswatch.o mfsbuilt.o mfslibload.o
OBJ05= mfsns.o mfsc.o mfsmsg.o
OBJ06= sreqdb.o sreq.o svcentsub.o
OBJ07= printhelp.o shio.o kshlib.o maininfo.o sesmsg.o msgdata.o
OBJ08= 
OBJ09= 
OBJ10= osetstr.o osetstr_loadfile.o
OBJ11= vecstr_svcargs.o vecstr_srvargs.o vecstr_avmkstr.o
OBJ12= intiq.o
OBJ13=
OBJ14= listenspec.o poller.o
OBJ15= msfile.o msfilee.o ebuf.o msghdr.o cmsghdr.o

OBJ16= spawnproc.o cpuspeed.o securefile.o strval.o
OBJ17=
OBJ18=
OBJ19=
OBJ20= svckv.o svcent.o
OBJ21= mkshlibname.o
OBJ22= isBadSend.o isBadRecv.o isBadMsg.o
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


LOBJ00= daytime.o hello.o
LOBJ01= timestr_nist.o
LOBJ02=
LOBJ03=

LOBJ= $(LOBJ00) $(LOBJ01) $(LBOBJ02) $(LOBJ03) 


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


so:			$(TL).so

x:			$(T).x


$(TL):			$(TL).so

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

$(TL).so:		$(LOBJ) Makefile $(TL).map
	$(LD) -G -o $@ $(SLDFLAGS) $(LOBJ) $(SLIBINFO) > $(T).lm

safe:
	makesafe -v=3 -I $(INCDIR) $(OBJ) $(LOBJ)

strip:			$(T).x
	strip $(T).x
	rm -f $(T).$(OFF) $(T)

install:		install-raw install-help

install-so:		$(TL).so
	-mkdir -p $(SVCSDIR) 2> /dev/null
	makenewer -s so=sx $(TL).so -d $(SVCSDIR)

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

proglogout.o:		proglogout.c proglog.h $(INCS)

proguserlist.o:		proguserlist.c proguserlist.h $(INCS)

testmfs.o:		testmfs.c $(INCS)

maininfo.o:		maininfo.c maininfo.h kshlib.h

kshlib.o:		kshlib.c kshlib.h sesmsg.h msgdata.h

sesmsg.o:		sesmsg.c sesmsg.h

msgdata.o:		msgdata.c msgdata.h

svcentsub.o:		svcentsub.c svcentsub.h mfslocinfo.h svcfile.h


# MFS additional code pieces

mfsmain.o:		mfsmain.c $(INCS) $(MFSINCS)

mfscmd.o:		mfscmd.c $(INCS) $(MFSINCS) mfsc.h

mfslocinfo.o:		mfslocinfo.c mfslocinfo.h $(INCS) mfslog.h

mfsadj.o:		mfsadj.c mfsadj.h $(INCS) mfslocinfo.h mfslog.h

mfsconfig.o:		mfsconfig.c mfsconfig.h $(INCS) mfslocinfo.h

mfslog.o:		mfslog.c mfslog.h $(INCS)

mfslisten.o:		mfslisten.c mfslisten.h $(INCS) mfslocinfo.h mfslog.h

mfswatch.o:		mfswatch.c mfswatch.h $(INCS) mfslocinfo.h mfslog.h

mfsbuilt.o:		mfsbuilt.c mfsbuilt.h $(INCS)

mfslibload.o:		mfslibload.c mfslibload.h $(INCS)
	$(GPP) -c $(GPPFLAGS) $(CFLAGS) $(CPPFLAGS) mfslibload.cc

mfsdebug.o:		mfsdebug.c $(INCS)

mfsc.o:			mfsc.c mfsc.h

mfsmsg.o:		mfsmsg.c mfsmsg.h

mfsc.o:			mfsc.c mfsc.c mfsmsg.o

sreq.o:			sreq.c sreq.h

sreqdb.o:		sreqdb.c sreqdb.h sreq.h


# modules

daytime.o:		daytime.c daytime.h mfserve.h

hello.o:		hello.c hello.h mfserve.h


# other standard pieces (duplicated here)

getnodedomain.o:	getnodedomain.c filebuf.h

getutmpent.o:		getutmpent.c getutmpent.h

getutmpterm.o:		getutmpterm.c getutmpent.h

svckv.o:		svckv.c svckv.h

svcent.o:		svcent.c svcent.h

format.o:		format.c format.h


# standard objects duplicated here

shio.o:			shio.c shio.h

userinfo.o:		userinfo.c userinfo.h

vecstr.o:		vecstr.c vecstr.h

vecstr_loadfile.o:	vecstr_loadfile.c vecstr.h

vecstr_env.o:		vecstr_env.c vecstr.h

vecstr_svcargs.o:	vecstr_svcargs.c vecstr.h

vecstr_srvargs.o:	vecstr_srvargs.c vecstr.h

vecstr_avmkstr.o:	vecstr_avmkstr.c vecstr.h

osetstr.o:		osetstr.cc osetstr.h
	$(GPP) -c $(GPPFLAGS) $(CFLAGS) $(CPPFLAGS) osetstr.cc

osetstr_loadfile.o:	osetstr_loadfile.c osetstr.h

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

ebuf.o:			ebuf.c ebuf.h

serialbuf.o:		serialbuf.c serialbuf.h

spawner.o:		spawner.c spawner.h

listenspec.o:		listenspec.c listenspec.h

poller.o:		poller.c poller.h

envmgr.o:		envmgr.c envmgr.h

varsub.o:		varsub.c varsub.h

varsub_extras.o:	varsub_extras.c

intiq.o:		intiq.c intiq.h

spawnproc.o:		spawnproc.c spawnproc.h


# testing

T01= testmfs

T01OBJa= $(T01).o mfsc.o
T01OBJb= mfsmsg.o
T01OBJc= envmgr.o
T01OBJd= debugprintf.o debug.o

T01OBJ= $(T01OBJa) $(T01OBJb) $(T01OBJc) $(T01OBJd)

$(T01).x:		Makefile $(T01OBJ)
	$(CC) -o $@ $(T01OBJ) $(TLIBINFO)

$(T01).o:		testmfs.c
	$(CC) -c $(CFLAGS) $(CPPFLAGS) testmfs.c


