# MAKEFILE

T= kshld

SVST= sysvars

NCST= ncpus

ALL= $(T)

SRCROOT= $(LOCAL)


BINDIR= $(SRCROOT)/bin
INCDIR= $(SRCROOT)/include
LIBDIR= $(SRCROOT)/lib
MANDIR= $(SRCROOT)/man/man1
HELPDIR= $(SRCROOT)/share/help

#LDCRTDIR= /opt/SUNWspro/WS6/lib
#LDCRTDIR= /opt/SUNWspro/SC5.0/lib
#LDCRTDIR= /opt/SUNWspro/SC4.0/lib
#LDCRTDIR= /opt/SUNWspro/lib
LDCRTDIR= $(SRCROOT)/lib


CC= gcc
GPP= g++

CCOPTS_GCCALL= -pthreads -fexceptions
CCOPTS_CCALL= -MT

CCOPTS_GCCOPTSPARC= -mcpu=ultrasparc
CCOPTS_GCCLIBSPARC= -mno-app-regs

CCOPTS= $(CCOPTS_GCCALL) -O $(CCOPTS_GCCOPTSPARC)
#CCOPTS= $(CCOPTS_GCCALL) -g -O
#CCOPTS= $(CCOPTS_GCCALL) -g -pg
#CCOPTS= $(CCOPTS_GCCALL) -g -Wstrict-aliasing

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
DEF7= $(LD_DEFS)

DEFS= $(DEF0) $(DEF1) $(DEF2) $(DEF3) $(DEF4) $(DEF5) $(DEF6) $(DEF7)

INCDIRS= -I$(INCDIR)

CPPFLAGS= $(DEFS) $(INCDIRS)

CFLAGS= $(CCOPTS)

#LD= $(CC)
#LD= cc
LD= ld

LDRPATH= $(EXTRA)/lib

# regular
LDFLAGS= -m $(LF_LDFLAGS) -R$(LIBDIR):$(LDRPATH)
# for profiling
LDFLAGS_P= -L/usr/lib/libp # -m -R$(LIBDIR)


LIBDIRS= -L$(LIBDIR)

LIB0=
LIB1= -ldam -lb
LIB2=
LIB3= -luc -lu
LIB4= -L$(GNU)/lib -lstdc++ -lgcc_eh -lgcc
LIB5= 
LIB6= -lsecdb -lproject -lpthread -lrt -lxnet -lsocket -lnsl
LIB7= -ldl -lc

LIBS= $(LIB0) $(LIB1) $(LIB2) $(LIB3) $(LIB4) $(LIB5) $(LIB6) $(LIB7)


# SLIBS is a subset of libraries
SLIBS= $(LIB0) $(LIB1) $(LIB4) $(LIB5) $(LIB6)


XLIBDIRS= -L$(LIBDIR)

XLIB0=
XLIB1= -ldam -lb
XLIB2= -ldebug
XLIB3= -luc -lu
XLIB4=
XLIB5= 
XLIB6= -lsecdb -lproject -lpthread -lrt -lxnet -lsocket -lnsl
XLIB7= -ldl

XLIBS= $(XLIB0) $(XLIB1) $(XLIB2) $(XLIB3) $(XLIB4) $(XLIB5) $(XLIB6) $(XLIB7)


CRT1= $(LDCRTDIR)/crt1.o
CRTI= $(LDCRTDIR)/crti.o
VALUES= $(LDCRTDIR)/values-xa.o
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


INCS= config.h defs.h


OBJ00= main.o whatinfo.o proginfo.o proginfo_setpiv.o proginfo_rootname.o
OBJ01= progfindprog.o progexec.o
OBJ02= defproc.o
OBJ03= getev.o
OBJ04= 
OBJ05=
OBJ06= envs.o envs_procxe.o envs_subs.o
OBJ07= printhelp.o
OBJ08=
OBJ09= userattr.o
OBJ10= sysvar.o dirseen.o paramfile.o varsub.o buffer.o
OBJ11= 
OBJ12= localgetorg.o
OBJ13= sfthing.o matkeystr.o nleadkeystr.o strkeycmp.o strnkeycmp.o 
OBJ14= hasalldig.o hasallplusminus.o hasallminus.o strnnlen.o
#OBJ15= 
#OBJ16=
#OBJ17=
#OBJ18= sysvars.o var.o varmk.o sysvarproc.o
OBJ19=
OBJ20=
OBJ21= getsystypenum.o getnodeinfo.o udomain.o inittimezone.o
OBJ22= userinfo.o getnodedomain.o readfileline.o
OBJ23= nextfieldterm.o snwcpyclean.o

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


SVSOBJ= sysvars.o var.o varmk.o dirseen.o sysvarproc.o

NCSOBJ= ncpus.o


SRC= $(OBJ:.c=.o)


.SUFFIXES:		.ls .i .cx .cs


default:		$(T).x

all:			$(ALL)

.c.o:
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $<

.cc.o:
	$(GPP) -c $(CFLAGS) $(CPPFLAGS) $<

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

#so:			$(SVST).so $(NCST).so
so:			$(SVST).so 

$(T).x:			$(OBJ) Makefile
	makedate -m $(T) > makedate.c
	$(CC) -c $(CFLAGS) makedate.c
	$(LD) -o $@ $(LDFLAGS) $(OBJS) $(LIBDIRS) $(LIBS) > $(T).lm

$(T).prof:		$(OBJ) Makefile
	makedate -m $(T) > makedate.c
	$(CC) -c makedate.c
	$(LD) -o $@ $(LDFLAGS_P) $(MOBJS) $(LIBDIRS) $(LIBS) > $(T).lm

$(T).gprof:		$(OBJ) Makefile
	makedate -m $(T) > makedate.c
	$(CC) -c makedate.c
	$(LD) -o $@ $(LDFLAGS_P) $(GOBJS) $(LIBDIRS) $(LIBS) > $(T).lm

$(T).$(OFF) $(OFF):	$(T).x
	cp -p $(T).x $(T).$(OFF)

$(SVST).so:		$(SVSOBJ) Makefile
	$(LD) -G -o $@ -M $(SVST).map $(LDFLAGS) \
		$(SVSOBJ) $(LIBDIRS) $(SLIBS) > $(SVST).lm

$(NCST).so:		$(NCSOBJ) Makefile $(NCST).map
	$(LD) -G -o $@ -M $(NCST).map $(LDFLAGS) \
		$(MKSOBJ) $(LIBDIRS) $(SLIBS) > $(NCST).lm

$(T).nm nm:		$(T).x
	$(NM) $(NMFLAGS) $(T).x > $(T).nm

$(T).cxref:
	$(CXREF) -c $(CXREFFLAGS) $(SRC) > $(T).cxref

safe:
	makesafe -v=3 -I $(INCDIR) $(OBJ) $(SVSOBJ) $(NCSOBJ)

strip:			$(T).x
	strip $(T).x
	rm -f $(T).$(OFF) $(T)

install:		install-raw install-help install-man

install-ee:		$(ALL)
	makenewer -r $(ALL) $(BINDIR)

install-raw:		$(T).x
	rm -f $(BINDIR)/$(T).$(OFF)
	makenewer -r -o rmsuf $(T).x $(BINDIR)

install-help:		$(T).help
	-mkdir -p $(HELPDIR) 2> /dev/null
	makenewer -o rmsuf $(T).help $(HELPDIR)

install-man:		$(T).man
	makenewer -m man=1 $(T).man $(MANDIR)

install-so:		$(SVST).so
	makenewer -r $(SVST).so $(LIBDIR)

again:
	rm -f $(ALL) $(T).x $(T).lm

clean:			again
	rm -f *.ls *.ln *.i *.o *.so *.x *.lm

control:
	uname -n > Control
	date >> Control


main.o:			main.c $(INCS) envs.h

whatinfo.o:		whatinfo.c config.h

proginfo.o:		proginfo.c defs.h

proginfo_setpiv.o:	proginfo_setpiv.c defs.h

proginfo_rootname.o:	proginfo_rootname.c defs.h

progfindprog.o:		progfindprog.c $(INCS)

progexec.o:		progexec.c $(INCS)


envs.o:			envs.c envs.h

envs_procxe.o:		envs_procxe.c envs.h

envs_subs.o:		envs_subs.c envs.h


getutmpent.o:		getutmpent.c getutmpent.h

userinfo.o:		userinfo.c userinfo.h


vecstr_loadfile.o:	vecstr_loadfile.c

vecstr_env.o:		vecstr_env.c

filemap.o:		filemap.c filemap.h

realname.o:		realname.c realname.h

ipasswd.o:		ipasswd.c ipasswd.h

lastlogfile.o:		lastlogfile.c lastlogfile.h

filebuf.o:		filebuf.c filebuf.h

buffer.o:		buffer.c buffer.h

vecobj.o:		vecobj.c vecobj.h

lookaside.o:		lookaside.c lookaside.h

pq.o:			pq.c pq.h

schedvar.o:		schedvar.c schedvar.h

sysvar.o:		sysvar.c sysvar.h sysvars.h dirseen.h

nulstr.o:		nulstr.c nulstr.h

varsub.o:		varsub.c varsub.h

paramfile.o:		paramfile.c paramfile.h

userattr.o:		userattr.c userattr.h


sysvars.o:		sysvars.c sysvars.h
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $(*).c

var.o:			var.c var.h vars.h dirseen.h
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $(*).c

varmk.o:		varmk.c varmk.h varmks.h dirseen.h
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $(*).c

vars.o:			vars.c vars.h varhdr.h
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $(*).c

varhdr.o:		varhdr.c varhdr.h
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $(*).c

dirseen.o:		dirseen.c dirseen.h
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $(*).c

msfile.o:		msfile.c msfile.h
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $(*).c


sysvarproc.o:		sysvarproc.c varmk.h
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $(*).c

ncpus.o:		ncpus.c
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $(*).c

getsysmisc.o:		getsysmisc.c getsysmisc.h

getsystypenum.o:	getsystypenum.c

bprintcleanline.o:	bprintcleanline.c


# testing

T01= testgetsystypenum

T01OBJ= testgetsystypenum.o getsystypenum.o

$(T01).x:			$(T01OBJ) Makefile
	$(CC) -o $@ $(CFLAGS) $(T01OBJ) $(LIBDIRS) $(XLIBS)


