# MAKEFILE

T= telserv

ALL= $(T) $(TR)

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

CCOPTS= $(CCOPTS_GCCALL) -O $(CCOPTS_GCCOPTSPARC)
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
DEF2= -DSYSV -DSTRNET -DBSD_COMP -DSOCKADDR_DL
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


LDRPATH= $(EXTRA)/lib:$(PCS)/lib

# LDFLAGS (-i means do not reference LD_LIBRARY_PATH)
LDFLAGS= -i -m $(LF_LDFLAGS) -R$(LIBDIR):$(LDRPATH):$(CGS_RPATH)


LIBDIRS= -L$(LIBDIR) -L$(PCS)/lib -L$(CGS_LIBDIR)

LIB0= -lpcs
LIB1= -ldam -lb
LIB2=
LIB3= -luc -lu
LIB4= $(CGS_LIBS)
LIB5= -lpam -lbsm /usr/lib/libcmd.so
LIB6= -lsecdb -lproject -lpthread -lrt -lxnet -lsocket -lnsl
LIB7= -ldl -lc

LIBS= $(LIB0) $(LIB1) $(LIB2) $(LIB3) $(LIB4) $(LIB5) $(LIB6) $(LIB7)

LIBINFO= $(LIBDIRS) $(LIBS)


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
NMFLAGS= -xs

CPP= cpp


INCS= config.h defs.h listenspec.h

OBJ00= main.o whatinfo.o proginfo.o proginfo_setpiv.o proginfo_rootname.o
OBJ01= proginfo_nameid.o proguserlist.o proglog.o
OBJ02= progjobdir.o proglogout.o progcheckstamp.o
OBJ03= progwatch.o progcheckdir.o proghandle.o progserve.o 
OBJ04= progsrvargs.o progreqfile.o progpeername.o progexec.o progshlib.o
OBJ05= progcmd.o progpass.o progprocess.o 
OBJ06= progconfig.o progpid.o progsvc.o progacc.o progipc.o
OBJ07= printhelp.o
OBJ08=
OBJ09= field_svcargs.o
OBJ10= varsub_extras.o
OBJ11= defproc.o clientinfo.o 
OBJ12= standing.o builtin.o sysmisc.o muximsg.o procse.o
OBJ13= 
OBJ14= envs.o envs_procxe.o envs_subs.o
OBJ15= lfm.o svcfile.o acctab.o
OBJ16= jobdb.o svckey.o connection.o
OBJ17= sysvar.o dirseen.o linefold.o poller.o listenspec.o
OBJ18=
OBJ19= netgroupcheck.o mkplogid.o mksublogid.o
OBJ20= sfthing.o getfname.o securefile.o nlspeername.o getaflen.o
OBJ21= strdomain.o mkutmpid.o pcsuserfile.o
OBJ22= strnnlen.o strlinelen.o getaf.o
OBJ23= getsystypenum.o

OBJ24=
OBJ25=
OBJ26=
OBJ27= mkbasename.o mkshlibname.o sfbaselib.o
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

OBJ48= getproviderid.o
OBJ49=
OBJ50= isBadSend.o isBadRecv.o isBadMsg.o
OBJ50= 
OBJ51= 
OBJ52=
OBJ53=
OBJ54=
#OBJ55= debug.o debugfork.o debugprintf.o nprintf.o

OBJA= $(OBJ00) $(OBJ01) $(OBJ02) $(OBJ03) $(OBJ04) $(OBJ05) $(OBJ06) $(OBJ07)
OBJB= $(OBJ08) $(OBJ09) $(OBJ10) $(OBJ11) $(OBJ12) $(OBJ13) $(OBJ14) $(OBJ15)
OBJC= $(OBJ16) $(OBJ17) $(OBJ18) $(OBJ19) $(OBJ20) $(OBJ21) $(OBJ22) $(OBJ23)
OBJD= $(OBJ24) $(OBJ25) $(OBJ26) $(OBJ27) $(OBJ28) $(OBJ29) $(OBJ30) $(OBJ31)
OBJE= $(OBJ32) $(OBJ33) $(OBJ34) $(OBJ35) $(OBJ36) $(OBJ37) $(OBJ38) $(OBJ39)
OBJF= $(OBJ40) $(OBJ41) $(OBJ42) $(OBJ43) $(OBJ44) $(OBJ45) $(OBJ46) $(OBJ47)
OBJG= $(OBJ48) $(OBJ49) $(OBJ50) $(OBJ51) $(OBJ52) $(OBJ53) $(OBJ54) $(OBJ55)

OBJ= $(OBJA) $(OBJB) $(OBJC) $(OBJD) $(OBJE) $(OBJF) $(OBJG)

OBJS= $(CRTFRONT) $(OBJ) $(CRTC) $(CRTBACK)


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


$(T):			$(T).ee
	cp -p $(T).ee $(T)

$(T).x:			$(OBJ) Makefile
	rm -f $(T).x
	makedate -m $(T) > makedate.c
	$(CC) -c $(CFLAGS) makedate.c
	$(LD) -o $@ $(LDFLAGS) $(OBJS) $(LIBINFO) > $(T).lm

$(T).$(OFF) $(OFF):	$(T).x
	cp -p $(T).x $(T).$(OFF)

$(T).nm nm:		$(T).x
	$(NM) $(NMFLAGS) $(T).x > $(T).nm

safe:
	makesafe -I $(INCDIR) $(OBJ)

strip:			$(T).x
	strip $(T).x
	rm -f $(T).$(OFF) $(T)

install:		install-raw install-help

install-ee:		$(ALL)
	makenewer -r $(ALL) $(BINDIR)

install-raw:		$(T).x
	rm -f $(BINDIR)/$(T).$(OFF)
	makenewer -r -o rmsuf $(T).x $(BINDIR)

install-help:		$(T).help
	-mkdir -p $(HELPDIR) 2> /dev/null
	makenewer -o rmsuf $(T).help $(HELPDIR)

again:
	rm -f $(ALL) $(T).x $(T).lm

clean:			again
	rm -f *.ls *.ln *.i *.o *.so *.x *.lm

control:
	uname -n > Control
	date >> Control


main.o:			main.c $(INCS)

whatinfo.o:		whatinfo.c config.h

proginfo.o:		proginfo.c defs.h

proginfo_setpiv_o:	proginfo_setpiv.c defs.h

proginfo_rootname:	proginfo_rootname.c defs.h

progconfig.o:		progconfig.c $(INCS)

progpid.o:		progpid.c $(INCS)

progsvc.o:		progsvc.c $(INCS)

progacc.o:		progacc.c $(INCS)

proglog.o:		proglog.c proglog.h $(INCS)

progprocess.o:		progprocess.c $(INCS)

progwatch.o:		progwatch.c $(INCS) poller.h listenspec.h

progipc.o:		progipc.c $(INCS)

proghandle.o:		proghandle.c $(INCS)

proglogout.o:		proglogout.c $(INCS)

progpcsconfig.o:	progpcsconfig.c $(INCS)

progcheckstamp.o:	progcheckstamp.c $(INCS)

progserve.o:		progserve.c $(INCS)

progsvcargs.o:		progsvcargs.c $(INCS)

progreqfile.o:		progreqfile.c $(INCS)

progcmd.o:		progcmd.c $(INCS)

progpass.o:		progpass.c $(INCS)

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


getutmpent.o:		getutmpent.c getutmpent.h

pcsuserfile.o:		pcsuserfile.c


userinfo.o:		userinfo.c userinfo.h

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

dirseen.o:		dirseen.c dirseen.h

nulstr.o:		nulstr.c nulstr.h

var.o:			var.c var.h varhdr.h

varhdr.o:		varhdr.c varhdr.h

logfile.o:		logfile.c logfile.h

connection.o:		connection.c connection.h

sockaddress.o:		sockaddress.c sockaddress.h

linefold.o:		linefold.c linefold.h

poller.o:		poller.c poller.h

listenspec.o:		listenspec.c listenspec.h

openportmsg.o:		openportmsg.c openportmsg.h


