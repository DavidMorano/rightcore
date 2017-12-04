# MAKEFILE

T= fingers
SVST= sysvars
MKST= varmks
VIST= vars

ALL= $(T).x $(SVST).so $(MKST).so $(VIST).so 

SRCROOT= $(LOCAL)


BINDIR= $(SRCROOT)/bin
INCDIR= $(SRCROOT)/include
LIBDIR= $(SRCROOT)/lib
MANDIR= $(SRCROOT)/man/man1
HELPDIR= $(SRCROOT)/share/help

EXTRABIN= /usr/extra/sbin


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
#CCOPTS= -xO5 -xtarget=ultra -xsafe=mem -dalign -xdepend


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


LDRPATH= ${EXTRA}/lib:/usr/lib/lwp

LDFLAGS= -m $(LF_LDFLAGS) -R$(LIBDIR):$(CGS_RPATH):$(LDRPATH)


LIBDIRS= -L$(LIBDIR) -L$(CGS_LIBDIR)

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


SLIBDIRS= -L$(LIBDIR) -L$(CGS_LIBDIR)

#SLIB0= -lpcs
SLIB1= -ldam -lb
#SLIB2= -luctest
#SLIB3= -lu
SLIB4= $(CGS_LIBS)
SLIB5= 
SLIB6= -lsecdb -lproject -lpthread -lrt -lxnet -lsocket -lnsl
#SLIB7= -ldl -lc

SLIBS= $(SLIB0) $(SLIB1) $(SLIB2) $(SLIB3) $(SLIB4) $(SLIB5) $(SLIB6) $(SLIB7)

SLIBINFO= $(SLIBDIRS) $(SLIBS)


TLIBDIRS= -L$(LIBDIR) -L$(CGS_LIBDIR)

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

CRT0= $(CRT1) $(CRTI) $(VALUES)
CRTC= makedate.o

LINT= lint
LINTFLAGS= -uxn -Dlint

NM= nm
NMFLAGS= -xs -v

CPP= cpp


INCS= config.h defs.h jobdb.h


OBJ00= main.o whatinfo.o proginfo.o proginfo_setpiv.o proginfo_rootname.o
OBJ01= proginfo_nameid.o proguserlist.o
OBJ02= progtmpdir.o proglogout.o progcheckstamp.o
OBJ03= progprocess.o progwatch.o progcheckdir.o proghandle.o progreqfile.o 
OBJ04= progserve.o progsrvargs.o progpeername.o
OBJ05= progcmd.o progpass.o proguseracct.o progexec.o progshlib.o
OBJ06= progconfig.o progpid.o progsvc.o progacc.o proglog.o progipc.o
OBJ07= printhelp.o 

OBJ08= varsub_extras.o
OBJ09= field_svcargs.o field_srvarg.o
OBJ10= 
OBJ11= standing.o builtin.o sysmisc.o muximsg.o procse.o
OBJ12= defproc.o clientinfo.o
OBJ13= 
OBJ14= envs.o envs_procxe.o envs_subs.o
OBJ15= lfm.o svcfile.o acctab.o logfile.o

OBJ16= jobdb.o svckey.o connection.o hostinfo.o
OBJ17= sysvar.o dirseen.o linefold.o poller.o listenspec.o
OBJ18= openportmsg.o
OBJ19= vecitem.o
OBJ20= sfthing.o getfname.o securefile.o nlspeername.o 
OBJ21= strdomain.o mkutmpid.o
OBJ22= strnnlen.o strlinelen.o snabbr.o
OBJ23= listenuss.o listentcp.o listenpass.o

OBJ24= mkplogid.o mksublogid.o permsched.o
OBJ25= mktmpfile.o opentmpfile.o openport.o
OBJ26= mkquoted.o netgroupcheck.o getprogpath.o
OBJ27= 
OBJ28= 
OBJ29=
OBJ30= 
OBJ31=

#OBJ32= 
#OBJ33=
#OBJ34= sysvars.o var.o
#OBJ35= sysvarproc.o
#OBJ36= varmk.o 
#OBJ37=
#OBJ38=
#OBJ39=

#OBJ40= vars.o varhdr.o
#OBJ41=
#OBJ42=
#OBJ43=
#OBJ44=
#OBJ45=
#OBJ46= 
#OBJ47=

OBJ48= getsystypenum.o getaflen.o getaf.o getproviderid.o
OBJ49=
OBJ50= isBadSend.o isBadRecv.o isBadMsg.o
OBJ50= 
OBJ51=
OBJ52=
OBJ53=
OBJ54=
OBJ55= debug.o debugprintf.o nprintf.o

OBJA= $(OBJ00) $(OBJ01) $(OBJ02) $(OBJ03) $(OBJ04) $(OBJ05) $(OBJ06) $(OBJ07)
OBJB= $(OBJ08) $(OBJ09) $(OBJ10) $(OBJ11) $(OBJ12) $(OBJ13) $(OBJ14) $(OBJ15)
OBJC= $(OBJ16) $(OBJ17) $(OBJ18) $(OBJ19) $(OBJ20) $(OBJ21) $(OBJ22) $(OBJ23)
OBJD= $(OBJ24) $(OBJ25) $(OBJ26) $(OBJ27) $(OBJ28) $(OBJ29) $(OBJ30) $(OBJ31)
OBJE= $(OBJ32) $(OBJ33) $(OBJ34) $(OBJ35) $(OBJ36) $(OBJ37) $(OBJ38) $(OBJ39)
OBJF= $(OBJ40) $(OBJ41) $(OBJ42) $(OBJ43) $(OBJ44) $(OBJ45) $(OBJ46) $(OBJ47)
OBJG= $(OBJ48) $(OBJ49) $(OBJ50) $(OBJ51) $(OBJ52) $(OBJ53) $(OBJ54) $(OBJ55)

OBJ= $(OBJA) $(OBJB) $(OBJC) $(OBJD) $(OBJE) $(OBJF) $(OBJG)

OBJS= $(CRTFRONT) $(OBJ) $(CRTC) $(CRTBACK)


SVSOBJ0= sysvars.o var.o varmk.o dirseen.o 
SVSOBJ1= sysvarproc.o
SVOBBJ2=
SVSOBJ= $(SVSOBJ0) $(SVSOBJ1) $(SVSOBJ2) 

VISOBJ0= vars.o varhdr.o hashagain.o hashelf.o randlc.o mkfnamesuf.o
VISOBJ1=
VISOBJ= $(VISOBJ0) $(VISOBJ1)

MKSOBJ= varmks.o varhdr.o 


SRC= $(OBJ:.c=.o)


.SUFFIXES:		.ls .i .cx .cs .lm


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

.o.lm:
	$(NM) $(NMFLAGS) $< > $(*).lm


$(T):			$(T).ee
	cp -p $(T).ee $(T)

$(T).x:			$(OBJ) Makefile
	rm -f $(T).x
	makedate -m $(T) > makedate.c
	$(CC) -c $(CFLAGS) makedate.c
	$(LD) -o $@ $(LDFLAGS) $(OBJS) $(LIBINFO) > $(T).lm

$(T).$(OFF) $(OFF):	$(T).x
	cp -p $(T).x $(T).$(OFF)

so:			$(SVST).so $(MKST).so $(VIST).so 

$(SVST).so:		$(SVSOBJ) Makefile $(SVST).map
	$(LD) -G -o $@ -M $(SVST).map $(LDFLAGS) \
		$(SVSOBJ) $(SLIBINFO) > $(SVST).lm

$(MKST).so:		$(MKSOBJ) Makefile $(MKST).map
	$(LD) -G -o $@ -M $(MKST).map $(LDFLAGS) \
		$(MKSOBJ) $(SLIBINFO) > $(MKST).lm

$(VIST).so:		$(VISOBJ) Makefile $(VIST).map
	$(LD) -G -o $@ -M $(VIST).map $(LDFLAGS) \
		$(VISOBJ) $(SLIBINFO) > $(VIST).lm

$(T).nm nm:		$(T).x
	$(NM) $(NMFLAGS) $(T).x > $(T).nm

safe:
	makesafe -I $(INCDIR) $(OBJ) $(SVSOBJ) $(MKSOBJ) $(VISOBJ)

strip:			$(T).x
	strip $(T).x
	rm -f $(T).$(OFF) $(T)

install:		install-raw install-help

install-ee:		$(ALL)
	makenewer -r $(ALL) $(BINDIR)

install-so:		$(SVST).so
	makenewer -r $(SVST).so $(LIBDIR)

install-raw:		$(T).x
	rm -f $(BINDIR)/$(T).$(OFF)
	makenewer -r -o rmsuf $(T).x $(BINDIR)

install-help:		$(T).help
	-mkdir -p $(HELPDIR) 2> /dev/null
	makenewer -o rmsuf $(T).help $(HELPDIR)

install.telserv:		$(T).x
	rm -f telserv
	ln -d $(T).x telserv
	makenewer -r telserv $(EXTRABIN)

again:
	rm -f $(ALL) $(T).x $(T).lm 
	rm -f $(SVST).so $(MKST).so $(VIST).so

clean:			again
	rm -f *.ls *.ln *.i *.o *.so *.x *.lm

control:
	uname -n > Control
	date >> Control


main.o:			main.c $(INCS)

whatinfo.o:		whatinfo.c config.h

proginfo.o:		proginfo.c defs.h

proginfo_setpiv_o:	proginfo_setpiv.c defs.h

proginfo_rootname.o:	proginfo_rootname.c defs.h

proguserlist.o:		proguserlist.c $(INCS)

progcmd.o:		progcmd.c $(INCS)

progpass.o:		progpass.c $(INCS)

progprocess.o:		progprocess.c $(INCS)

progconfig.o:		progconfig.c $(INCS)

progpid.o:		progpid.c $(INCS)

progsvc.o:		progsvc.c $(INCS)

progacc.o:		progacc.c $(INCS)

proglog.o:		proglog.c $(INCS)

progwatch.o:		progwatch.c $(INCS) poller.h listenspec.h

progipc.o:		progipc.c $(INCS)

proghandle.o:		proghandle.c $(INCS)

proglogout.o:		proglogout.c $(INCS)

progpcsconfig.o:	progpcsconfig.c $(INCS)

progcheckstamp.o:	progcheckstamp.c $(INCS)

progserve.o:		progserve.c $(INCS)

progsvcargs.o:		progsvcargs.c $(INCS)

progreqfile.o:		progreqfile.c $(INCS)

progexec.o:		progexec.c $(INCS)

progpeername.o:		progpeername.c $(INCS)

proguseracct.o:		proguseracct.c $(INCS)

standing.o:		standing.c standing.h $(INCS)

builtin.o:		builtin.c builtin.h $(INCS)

clientinfo.o:		clientinfo.c $(INCS)


jobdb.o:		jobdb.c jobdb.h

sysmisc.o:		sysmisc.c sysmisc.h

muximsg.o:		muximsg.c muximsg.h

procse.o:		procse.c procse.h

svckey.o:		svckey.c svckey.h


vecstr_loadfile.o:	vecstr_loadfile.c

vecstr_env.o:		vecstr_env.c

vecstr.o:		vecstr.c vecstr.h

pcspoll.o:		pcspoll.c pcsconf.h

filemap.o:		filemap.c filemap.h

filebuf.o:		filebuf.c filebuf.h

realname.o:		realname.c realname.h

ipasswd.o:		ipasswd.c ipasswd.h

lastlogfile.o:		lastlogfile.c lastlogfile.h

tmpx.o:			tmpx.c tmpx.h

expcook.o:		expcook.c expcook.h

envs.o:			envs.c envs.h

envs_procxe.o:		envs_procxe.c envs.h

envs_subs.o:		envs_subs.c envs.h

lfm.o:			lfm.c lfm.h

svcfile.o:		svcfile.c svcfile.h

acctab.o:		acctab.c acctab.h

msg.o:			msg.c msg.h

matenv.o:		matenv.c matenv.h

sysvar.o:		sysvar.c sysvar.h dirseen.h nulstr.h

nulstr.o:		nulstr.c nulstr.h

logfile.o:		logfile.c logfile.h

connection.o:		connection.c connection.h

sockaddress.o:		sockaddress.c sockaddress.h

linefold.o:		linefold.c linefold.h

poller.o:		poller.c poller.h

listenspec.o:		listenspec.c listenspec.h

serialbuf.o:		serialbuf.c serialbuf.h

hostinfo.o:		hostinfo.c hostinfo.h

hostaddr.o:		hostaddr.c hostaddr.h

field_svcargs.o:	field_svcargs.c field.h

field_srvarg.o:		field_srvarg.c field.h

userinfo.o:		userinfo.c userinfo.h

mkprogenv.o:		mkprogenv.c mkprogenv.h

openportmsg.o:		openportmsg.c openportmsg.h

vecitem.o:		vecitem.c vecitem.h


spawnproc.o:		spawnproc.c spawnproc.h
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $(*).c

modload.o:		modload.c modload.h
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $(*).c

dirseen.o:		dirseen.c dirseen.h
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $(*).c

sysvars.o:		sysvars.c sysvars.h
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $(*).c

var.o:			var.c var.h
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $(*).c

varmk.o:		varmk.c varmk.h
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $(*).c

vars.o:			vars.c vars.h
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $(*).c

varmks.o:		varmks.c varmks.h
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $(*).c

varhdr.o:		varhdr.c varhdr.h
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $(*).c

sysvarproc.o:		sysvarproc.c
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $(*).c


# testing

T01= testopenport

T01OBJa= $(T01).o
T01OBJb= 
T01OBJc=
T01OBJd=

T01OBJ= $(T01OBJa) $(T01OBJb) $(T01OBJc) $(T01OBJd)

$(T01).o:		$(T01).c

$(T01).x:		Makefile $(T01OBJ)
	$(GPP) -o $@ $(T01OBJ) $(TLIBDIRS) $(TLIBS)


