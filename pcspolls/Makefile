# MAKEFILE

T= testpcspolls

ST= pcspolls

ST1= pollprog
ST2= loguser
ST3= testcrash

POLLS= $(ST1).so $(ST2).so
#POLLS= $(ST1).so $(ST2).so $(ST3).so
#POLLS= $(ST3).so

ALL= $(ST).so $(POLLS)

SRCROOT= $(PCS)


BINDIR= $(SRCROOT)/bin
INCDIR= $(SRCROOT)/include
LIBDIR= $(SRCROOT)/lib
MANDIR= $(SRCROOT)/man/man1
HELPDIR= $(SRCROOT)/share/help
PLIBDIR= $(SRCROOT)/lib/pcspolls

LDCRTDIR= $(CGS_VALDIR)


CC= $(CGS_CC)
GPP= $(CGS_GPP)

GPPFLAGS= $(CGS_CCFLAGS)
CCOPTS_GCCALL= -pthreads -fexceptions
CCOPTS_CCALL= -MT

CCOPTS_GCCOPTSGPP= $(CGS_CCFLAGS)
CCOPTS_GCCOPTSPARC= -mcpu=ultrasparc
CCOPTS_GCCLIBSPARC= -mno-app-regs

CCOPTS= $(CCOPTS_GCCALL) -O3 $(CCOPTS_GCCOPTSPARC)
#CCOPTS= $(CCOPTS_GCCALL) -g -O
#CCOPTS= $(CCOPTS_GCCALL) -g -pg
#CCOPTS= $(CCOPTS_GCCALL) -g -Wstrict-aliasing
#CCOPTS= $(CCOPTS_GCCALL) -g -Wall

CCOPTS_GCCLIB= -fpic $(CCOPTS_GCCLIBSPARC)

#CCOPTS= $(CCOPTS_CCALL) -g -xs

# HyperSPARC
# regular
#CCOPTS= -xO5 -xtarget=ss20/hs22 -dalign -xdepend
# profiling 'prof(1)'
#CCOPTS= -p -xO5 -xtarget=ss20/hs22 -dalign -xdepend
# profiling 'gprof(1)' 
#CCOPTS= -xpg -xO5 -xtarget=ss20/hs22 -dalign -xdepend

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
LDFLAGS= -m $(LF_LDFLAGS) -R$(LIBDIR):$(LOCAL)/lib:$(CGS_RPATH):$(LDRPATH)
# for profiling
#LDFLAGS= $(LF_LDFLAGS) -L/usr/lib/libp # -m -R$(LIBDIR)

LDFLAGS_SO= $(LDFLAGS)


LIBDIRS= -L$(LIBDIR) -L$(LOCAL)/lib -L$(CGS_LIBDIR)

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


SLIBDIRS= -L$(LIBDIR) -L$(LOCAL)/lib -L$(CGS_LIBDIR)

SLIB0=
SLIB1= -lpcs -ldam -lb
SLIB2=
SLIB3= -luc -lu
SLIB4=
SLIB5= 
SLIB6= -lsecdb -lproject -lpthread -lthread -lrt -lxnet -lsocket -lnsl
SLIB7=

SLIBS= $(SLIB0) $(SLIB1) $(SLIB2) $(SLIB3) $(SLIB4) $(SLIB5) $(SLIB6) $(SLIB7)

SLIBINFO= $(SLIBDIRS) $(SLIBS)


XLIBDIRS= -L$(LIBDIR) -L$(LOCAL)/include -L$(CGS_LIBDIR)

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
OBJ01= proguserlist.o proglog.o
OBJ02= progdname.o 
OBJ03= 
OBJ04= 
OBJ05=
OBJ06=
OBJ07= printhelp.o 
OBJ08=
OBJ09= fsdirtree.o fsdir.o fifostr.o 
OBJ10= pcsconf.o pcspoll.o var.o varmk.o
OBJ11=
OBJ12=
OBJ13=
OBJ14=
OBJ15=

OBJ16= expcook.o hdbstr.o
#OBJ17= pcsconfs.o
#OBJ18= modload.o
OBJ19= sfcookkey.o
#OBJ20= debugprintf.o strlinelen.o debug.o
OBJ21=
OBJ22= pcspolls.o thrcomm.o fileobject.o hasfext.o
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

SOBJ00= pcspolls.o
SOBJ01= thrcomm.o
SOBJ02= hasfext.o
SOBJ03=
SOBJ04=
SOBJ05=
SOBJ06=
SOBJ07=

SOBJA= $(SOBJ00) $(SOBJ01) $(SOBJ02) $(SOBJ03) 
SOBJB= $(SOBJ04) $(SOBJ05) $(PTCSOBJ06) $(SOBJ07) 

SOBJ= $(SOBJA) $(SOBJB)


.SUFFIXES:		.ls .i .cx .cs


default:		$(ALL)

all:			$(ALL)

so:			$(ST).so $(POLLS)

polls:			$(POLLS)


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
	ln -f $(T).x $(T)

$(T).x:			$(OBJ) Makefile
	makedate -m $(T) > makedate.c
	$(CC) -c makedate.c
	$(LD) -o $@ $(LDFLAGS) $(OBJS) $(XLIBINFO) > $(T).lm

$(T).prof:		$(OBJ) Makefile
	makedate -m $(T) > makedate.c
	$(CC) -c makedate.c
	$(LD) -o $@ $(LDFLAGS) $(MOBJS) $(LIBDIRS) $(LIBS) > $(T).lm

$(T).gprof:		$(OBJ) Makefile
	makedate -m $(T) > makedate.c
	$(CC) -c makedate.c
	$(LD) -o $@ $(LDFLAGS) $(GOBJS) $(LIBDIRS) $(LIBS) > $(T).lm

$(T).$(OFF) $(OFF):	$(T).x
	cp -p $(T).x $(T).$(OFF)

$(T).nm nm:		$(T).x
	$(NM) $(NMFLAGS) $(T).x > $(T).nm

$(T)-safe:
	makesafe -v=3 -I $(INCDIR) $(OBJ)

safe:			$(T)-safe te-safe testpollobj-safe ts-safe

strip:			$(T).x
	strip $(T).x
	rm -f $(T).$(OFF) $(T)

install:		install-so

install-so:		install-pcspolls install-polls 

install-raw:		$(T).x
	rm -f $(BINDIR)/$(T).$(OFF)
	makenewer -r -o rmsuf $(T).x $(BINDIR)
	-chmod g+s $(BINDIR)/$(T)

install-help:		$(T).help
	-mkdir -p $(HELPDIR) 2> /dev/null
	makenewer -o rmsuf $(T).help $(HELPDIR)


$(ST).so:		$(SOBJ) Makefile $(ST).map
	$(LD) -G -o $@ -M $(ST).map $(LDFLAGS_SO) $(SOBJ) $(SLIBINFO) > $(ST).lm

$(ST)-safe:
	makesafe -v=3 -I $(INCDIR) $(SOBJ)

install-pcspolls:	$(ST).so
	makenewer -r $(ST).so $(LIBDIR)

install-polls:		$(POLLS)
	makenewer -r $(POLLS) $(PLIBDIR)

again:
	rm -fr $(ALL) $(T).x

clean:			again
	makeclean

control:
	uname -n > Control
	date >> Control


# testing

TE= te

TEOBJ01= te.o sfcookkey.o expcook.o hdbstr.o
TEOBJ02= debugprintf.o strlinelen.o

TEOBJ= $(TEOBJ01) $(TEOBJ02)

TEOBJS= $(CRTFRONT) $(TEOBJ) $(CRTC) $(CRTBACK)

te-safe:
	makesafe -v=3 -I $(INCDIR) $(TEOBJ)

te.x:			$(TEOBJ)
	$(CC) -o $@ $(CFLAGS) $(TEOBJ) $(XLIBDIRS) $(XLIBS)


TA= ta

TAOBJ01= ta.o 
TAOBJ02= debugprintf.o strlinelen.o

TAOBJ= $(TAOBJ01) $(TAOBJ02)

TAOBJS= $(CRTFRONT) $(TAOBJ) $(CRTC) $(CRTBACK)

ta.x:			$(TAOBJ)
	$(CC) -o $@ $(CFLAGS) $(TAOBJ) $(XLIBDIRS) $(XLIBS)


TSR= testspawner

TWOBJ01= testspawner.o spawner.o envhelp.o
TWOBJ02= debugprintf.o strlinelen.o

TWOBJ= $(TWOBJ01) $(TWOBJ02)

TWOBJS= $(CRTFRONT) $(TWOBJ) $(CRTC) $(CRTBACK)

ts-safe:
	makesafe -v=3 -I $(INCDIR) $(TWOBJ)

ts.x:			$(TWOBJ)
	$(CC) -o $@ $(CFLAGS) $(TWOBJ) $(XLIBDIRS) $(XLIBS)


ST1OBJ00=
ST1OBJ01= pollproger.o pollprogcheck.o 
ST1OBJ02= thrbase.o spawner.o envhelp.o
ST1OBJ03=

ST1OBJa= $(ST1OBJ00) $(ST1OBJ01) $(ST1OBJ02) $(ST1OBJ03)
ST1OBJb= $(ST1OBJ04) $(ST1OBJ05) $(ST1OBJ06) $(ST1OBJ07)

ST1OBJ= $(ST1OBJa) $(ST1OBJb)

$(ST1)-safe:
	makesafe -v=3 -I $(INCDIR) $(ST1OBJ)

$(ST1).so:		$(ST1OBJ) Makefile $(ST1).map
	$(LD) -G -o $@ -M $(ST1).map $(LDFLAGS_SO) \
		$(ST1OBJ) $(SLIBDIRS) $(SLIBS) > $(ST1).lm

pollproger.o:		pollproger.c pcspolls.h
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<

pollprogcheck.o:	pollprogcheck.c pcspolls.h
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<


ST2OBJ00=
ST2OBJ01= loguser.o
ST2OBJ02= thrbase.o spawner.o envhelp.o

ST2OBJ= $(ST2OBJ00) $(ST2OBJ01) $(ST2OBJ02)

$(ST2)-safe:
	makesafe -v=3 -I $(INCDIR) $(ST2OBJ)

$(ST2).so:		$(ST2OBJ) Makefile $(ST2).map
	$(LD) -G -o $@ -M $(ST2).map $(LDFLAGS_SO) \
		$(ST2OBJ) $(SLIBINFO) > $(ST2).lm

$(ST2).o:		loguser.c pcspolls.h
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<


ST3OBJ00=
ST3OBJ01= testcrash.o
ST3OBJ02= thrbase.o spawner.o envhelp.o

ST3OBJ= $(ST3OBJ00) $(ST3OBJ01) $(ST3OBJ02)

$(ST3)-safe:
	makesafe -v=3 -I $(INCDIR) $(ST3OBJ)

$(ST3).so:		$(ST3OBJ) Makefile $(ST3).map
	$(LD) -G -o $@ -M $(ST3).map $(LDFLAGS_SO) \
		$(ST3OBJ) $(SLIBINFO) > $(ST3).lm

$(ST3).o:		testcrash.c pcspolls.h thrbase.h
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<


# general

main.o:			main.c $(INCS) 

whatinfo.o:		whatinfo.c config.h

proginfo.o:		proginfo.c $(INCS)

proginfo_setpiv.o:	proginfo_setpiv.c $(INCS)

proguserlist.o:		proguserlist.c defs.h

proglog.o:		proglog.c defs.h

proglogid.o:		proglogid.c defs.h


varsub_extras.o:	varsub_extras.c 

vecstr_loadfile.o:	vecstr_loadfile.c

vecstr_env.o:		vecstr_env.c

fsdirtree.o:		fsdirtree.c fsdirtree.h

fsdir.o:		fsdir.c fsdir.h

msg.o:			msg.c msg.h

filebuf.o:		filebuf.c filebuf.h

matenv.o:		matenv.c matenv.h

lfm.o:			lfm.c lfm.h

pq.o:			pq.c pq.h

fifostr.o:		fifostr.c fifostr.h

pcsconf.o:		pcsconf.c pcsconf.h

pcspoll.o:		pcspoll.c pcspoll.h

var.o:			var.c var.h

varmk.o:		varmk.c varmk.h

expcook.o:		expcook.c expcook.h

hdbstr.o:		hdbstr.c hdbstr.h

modload.o:		modload.c modload.h


pcsconfs.o:		pcsconfs.c pcsconfs.h
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<

pcspolls.o:		pcspolls.c pcspolls.h pcsconf.h thrcomm.h
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<

thrcomm.o:		thrcomm.c thrcomm.h
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<

thrbase.o:		thrbase.c thrbase.h thrcomm.h
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<

vars.o:			vars.c vars.h
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<

varmks.o:		varmks.c varmks.h
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<

varhdr.o:		varhdr.c varhdr.h
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<

spawner.o:		spawner.c spawner.h
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<

envhelp.o:		envhelp.c envhelp.h
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<


fileobject.o:		fileobject.c
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<

hasfext.o:		hasfext.c
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<


# poll-objects

TPSOBJ=	testpollobj.o thrbase.o

testpollobj-safe:
	makesafe -v=3 -I $(INCDIR) $(TPSOBJ)

testpollobj.so:		$(TPSOBJ) Makefile
	$(LD) -G -o $@ $(LDFLAGS_SO) \
		$(TPSOBJ) $(SLIBINFO) > testpollobj.lm

testpollobj.o:		testpollobj.c pcspolls.h pcsconf.h thrbase.h
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<


