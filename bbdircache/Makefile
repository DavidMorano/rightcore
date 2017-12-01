# MAKEFILE

T= bbdircache

TPCS= pcsconfs

TPPS= pcspolls

ALL= $(T)

SRCROOT= $(PCS)


BINDIR= $(SRCROOT)/bin
INCDIR= $(SRCROOT)/include
LIBDIR= $(SRCROOT)/lib
MANDIR= $(SRCROOT)/man/man1
HELPDIR= $(SRCROOT)/share/help
SOLIBDIR= $(SRCROOT)/lib/pcsconf

LDCRTDIR= $(CGS_CRTDIR)
VALDIR= $(CGS_VALDIR)


# NOTES:
#
# To 'make' for 'prof(1)' profiling, execute 'make bb.prof'.
# To 'make' for 'gprof(1)' profiling, execute 'make bb.prof'.
#
#

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


LDRPATH= /usr/lib/lwp

# regular
LDFLAGS= -m $(LF_LDFLAGS) -R$(LIBDIR):$(CGS_RPATH):$(LDRPATH)
# for profiling
#LDFLAGS= $(LF_LDFLAGS) -L/usr/lib/libp # -m -R$(LIBDIR)

LDFLAGS_SO= $(LF_LDFLAGS) $(LDFLAGS)


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


SLIBDIRS= -L$(LIBDIR) -L$(CGS_LIBDIR)

SLIB0=
SLIB1= -lpcs -ldam -lb
SLIB2=
SLIB3= -luc -lu
SLIB4= $(CGS_LIBS)
SLIB5= 
SLIB6= -lsecdb -lproject -lpthread -lthread -lrt -lxnet -lsocket -lnsl
SLIB7= -ldl

SLIBS= $(SLIB0) $(SLIB1) $(SLIB2) $(SLIB3) $(SLIB4) $(SLIB5) $(SLIB6) $(SLIB7)

SLIBINFO= $(SLIBDIRS) $(SLIBS)


XLIBDIRS= -L$(LIBDIR) -L$(CGS_LIBDIR)

XLIB0=
XLIB1= -lpcs -ldam -lb
XLIB2=
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
OBJ09=
OBJ10=
OBJ11= pcsconf.o pcspoll.o var.o varmk.o
OBJ12=
OBJ13=
OBJ14=
OBJ15= vecpstr_addsubdirs.o

#OBJ16= expcook.o sfcookkey.o
#OBJ17= pcsconfs.o
#OBJ18= modload.o
OBJ19=
#OBJ20= debugprintf.o strlinelen.o debug.o
#OBJ21=
#OBJ22= pcspolls.o thrcomm.o fileobject.o hasfext.o
#OBJ23= 

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

TPCSOBJ00= pcsconfs.o
TPCSOBJ01= 
TPCSOBJ02= var.o varmk.o
TPCSOBJ03=
TPCSOBJ04= pathadd.o
TPCSOBJ05=
TPCSOBJ06=
TPCSOBJ07=

TPCSOBJA= $(TPCSOBJ00) $(TPCSOBJ01) $(TPCSOBJ02) $(TPCSOBJ03) 
TPCSOBJB= $(TPCSOBJ04) $(TPCSOBJ05) $(PTCSOBJ06) $(TPCSOBJ07) 

TPCSOBJ= $(TPCSOBJA) $(TPCSOBJB)


TPPSOBJ= pcspolls.o thrcomm.o


.SUFFIXES:		.ls .i .cx .cs


default:		$(T).x

all:			$(ALL)

so:			$(TPCS).so


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
	$(LD) -o $@ $(LDFLAGS) $(OBJS) $(LIBINFO) > $(T).lm

$(T).prof:		$(OBJ) Makefile
	makedate -m $(T) > makedate.c
	$(CC) -c makedate.c
	$(LD) -o $@ $(LDFLAGS) $(MOBJS) $(LIBDIRS) $(LIBS) > $(T).lm

$(T).gprof:		$(OBJ) Makefile
	makedate -m $(T) > makedate.c
	$(CC) -c makedate.c
	$(LD) -o $@ $(LDFLAGS) $(GOBJS) $(LIBDIRS) $(LIBS) > $(T).lm

$(TPCS)-safe:
	makesafe -v=3 -I $(INCDIR) $(TPCSOBJ)

$(TPCS).so:		$(TPCSOBJ) Makefile $(TPCS).map
	$(LD) -G -o $@ -M $(TPCS).map $(LDFLAGS_SO) \
		$(TPCSOBJ) $(SLIBINFO) > $(TPCS).lm

$(TPPS)-safe:
	makesafe -v=3 -I $(INCDIR) $(TPPSOBJ)

$(TPPS).so:		$(TPPSOBJ) Makefile $(TPPS).map
	$(LD) -G -o $@ -M $(TPPS).map $(LDFLAGS_SO) \
		$(TPPSOBJ) $(SLIBINFO) > $(TPPS).lm

$(T).$(OFF) $(OFF):	$(T).x
	cp -p $(T).x $(T).$(OFF)

$(T).nm nm:		$(T).x
	$(NM) $(NMFLAGS) $(T).x > $(T).nm

safe:			$(TPCS)-safe te-safe testpollobj-safe ts-safe
	makesafe -v=3 -I $(INCDIR) $(OBJ)

strip:			$(T).x
	strip $(T).x
	rm -f $(T).$(OFF) $(T)

install:		install-raw install-help

install-so:		$(TPCS).so
	makenewer -r $(TPCS).so $(LIBDIR)

install-raw:		$(T).x
	rm -f $(BINDIR)/$(T).$(OFF)
	makenewer -r -o rmsuf $(T).x $(BINDIR)
	-chmod g+s $(BINDIR)/$(T)

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


TEOBJ01= te.o sfcookkey.o expcook.o hdbstr.o
TEOBJ02= debugprintf.o strlinelen.o

TEOBJ= $(TEOBJ01) $(TEOBJ02)

TEOBJS= $(CRTFRONT) $(TEOBJ) $(CRTC) $(CRTBACK)

te-safe:
	makesafe -v=3 -I $(INCDIR) $(TEOBJ)

testexpcook.x:	te.x
	ln -f te.x $@

te.x:			$(TEOBJ)
	$(CC) -o $@ $(CFLAGS) $(TEOBJ) $(XLIBINFO)


TAOBJ01= ta.o 
TAOBJ02= debugprintf.o strlinelen.o

TAOBJ= $(TAOBJ01) $(TAOBJ02)

TAOBJS= $(CRTFRONT) $(TAOBJ) $(CRTC) $(CRTBACK)

ta.x:			$(TAOBJ)
	$(CC) -o $@ $(CFLAGS) $(TAOBJ) $(XLIBINFO)


TSR= testspawner

TWOBJ01= testspawner.o spawner.o envhelp.o
TWOBJ02= debugprintf.o strlinelen.o

TWOBJ= $(TWOBJ01) $(TWOBJ02)

TWOBJS= $(CRTFRONT) $(TWOBJ) $(CRTC) $(CRTBACK)

ts-safe:
	makesafe -v=3 -I $(INCDIR) $(TWOBJ)

testspawner.x:		ts.x
	ln -f ts.x $@

ts.x:			$(TWOBJ)
	$(CC) -o $@ $(CFLAGS) $(TWOBJ) $(XLIBINFO)


TPG= pollprog

TPGSOBJ01= pollproger.o pollprogcheck.o 
TPGSOBJ02= thrbase.o

TPGSOBJ= $(TPGSOBJ01) $(TPGSOBJ02)

$(TPG)-safe:
	makesafe -v=3 -I $(INCDIR) $(TPGSOBJ)

$(TPG).so:		$(TPGSOBJ) Makefile $(TPG).map
	$(LD) -G -o $@ -M $(TPG).map $(LDFLAGS_SO) \
		$(TPGSOBJ) $(SLIBINFO) > $(TPG).lm


main.o:			main.c $(INCS) 

whatinfo.o:		whatinfo.c config.h

proginfo.o:		proginfo.c $(INCS)

proginfo_setpiv.o:	proginfo_setpiv.c $(INCS)

proguserlist.o:		proguserlist.c defs.h

proglog.o:		proglog.c defs.h

proglogid.o:		proglogid.c defs.h

progdname.o:		progdname.c $(INCS)


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

vecpstr.o:		vecpstr.c vecpstr.h


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


