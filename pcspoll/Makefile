# MAKEFILE

T= pcspoll

ALL= $(T)

#PCS= /usr/add-on/pcs
SRCROOT= $(PCS)


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


# NOTES:
#
# To 'make' for 'prof(1)' profiling, execute 'make $(T).prof'.
# To 'make' for 'gprof(1)' profiling, execute 'make $(T).gprof'.
#

CC= gcc
GPP= g++

CCOPTS_GCCALL= -pthreads -fexceptions
CCOPTS_CCALL= -MT

CCOPTS_GCCOPTSPARC= -mcpu=ultrasparc
CCOPTS_GCCLIBSPARC= -mno-app-regs

CCOPTS= $(CCOPTS_GCCALL) -O2 $(CCOPTS_GCCOPTSPARC)
#CCOPTS= $(CCOPTS_GCCALL) -g -O
#CCOPTS= $(CCOPTS_GCCALL) -g -pg
#CCOPTS= $(CCOPTS_GCCALL) -g -Wstrict-aliasing -Wall

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
LDFLAGS= $(LF_LDFLAGS) -m -R$(LIBDIR):$(LDRPATH)
# for profiling
#LDFLAGS= $(LF_LDFLAGS) -L/usr/lib/libp # -m -R$(LIBDIR):$(LDRPATH)


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

CPP= cpp


INCS= config.h defs.h


OBJ00= main.o whatinfo.o proginfo.o proginfo_setpiv.o proginfo_rootname.o
OBJ01= proginfo_nameid.o proguserlist.o
OBJ02= progwatch.o progexec.o proglog.o progsvc.o progacc.o
OBJ03= proglogout.o progpass.o progtmpdir.o progreqfile.o progcheckdir.o
OBJ04= progcmd.o progprocess.o 
OBJ05= muximsg.o svckey.o svcentry.o
OBJ06= progconfig.o progpcsconfig.o progpid.o progstampcheck.o 
OBJ07= printhelp.o
OBJ08=
OBJ09= varsub.o varsub_extras.o
OBJ10=
OBJ11=
OBJ12= field_svcargs.o
OBJ13= sysvar.o dirseen.o linefold.o poller.o listenspec.o
OBJ14= envs.o envs_procxe.o envs_subs.o
OBJ15= svcfile.o acctab.o cq.o lfm.o var.o spawner.o

OBJ16= pcsuserfile.o
OBJ17= mkplogid.o mksublogid.o
OBJ18= prmktmpdir.o mktmpuserdir.o
OBJ19= getfname.o getarchitecture.o
OBJ20= defproc.o securefile.o
OBJ21= strnnlen.o strlinelen.o 
OBJ22= strfieldcmp.o snsdd.o sfthing.o
OBJ23= nextfieldterm.o

OBJ24= getsystypenum.o getproviderid.o getaf.o getaflen.o
OBJ25= 
OBJ26= isBadSend.o isBadRecv.o isBadMsg.o
OBJ27=
OBJ28=
OBJ29= uc_waitwritable.o
#OBJ30= authfile.o onckeyalready.o onckeygetset.o checkonc.o havenis.o
#OBJ31= debug.o debugprintf.o debugfork.o

OBJA= $(OBJ00) $(OBJ01) $(OBJ02) $(OBJ03) $(OBJ04) $(OBJ05) $(OBJ06) $(OBJ07) 
OBJB= $(OBJ08) $(OBJ09) $(OBJ10) $(OBJ11) $(OBJ12) $(OBJ13) $(OBJ14) $(OBJ15)
OBJC= $(OBJ16) $(OBJ17) $(OBJ18) $(OBJ19) $(OBJ20) $(OBJ21) $(OBJ22) $(OBJ23)
OBJD= $(OBJ24) $(OBJ25) $(OBJ26) $(OBJ27) $(OBJ28) $(OBJ29) $(OBJ30) $(OBJ31)

OBJ= $(OBJA) $(OBJB) $(OBJC) $(OBJD)

# for regular (no profiling)
OBJS= $(CRTFRONT) $(OBJ) $(CRTC) $(CRTBACK)
# for 'prof(1)'
MOBJS= $(MCRT0) $(OBJ) $(CRTC)
# for 'gprof(1)'
GOBJS= $(GCRT0) $(OBJ) $(CRTC)


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
	rm -f $(T)
	cp -p $(T).ee $(T)

$(T).x:			$(OBJ) Makefile
	makedate -m $(T) > makedate.c
	$(CC) -c makedate.c
	$(LD) -o $@ $(LDFLAGS) $(OBJS) $(LIBDIRS) $(LIBS) > $(T).lm

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

safe:
	makesafe -v=3 -I $(INCDIR) $(OBJ)

strip:			$(T).x
	strip $(T).x
	rm -f $(T).$(OFF) $(T)

install-debug:		$(T).x
	makenewer -r $(T).x $(BINDIR)
	-chmod u+s $(BINDIR)/$(T).x
	rm -f $(T)
	ln $(T).debug $(T)
	makenewer -r $(T) $(BINDIR)

install:		install-raw install-help

install-ee:		$(ALL)
	makenewer -r $(ALL) $(BINDIR)
	-chmod u+s $(BINDIR)/$(T).$(OFF)

install-raw:		$(T).x
	rm -f $(BINDIR)/$(T).$(OFF)
	makenewer -o rmsuf -r $(T).x $(BINDIR)
	-chmod u+s $(BINDIR)/$(T)

install-help:		$(T).help
	-mkdir -p $(HELPDIR) 2> /dev/null
	makenewer -o rmsuf $(T).help $(HELPDIR)

again:
	rm -fr $(ALL) $(T).x

clean:			again
	rm -f *.ls *.ln *.i *.o *.so *.x *.lm

control:
	uname -n > Control
	date >> Control


main.o:			main.c $(INCS)

whatinfo.o:		whatinfo.c config.h

proginfo.o:		proginfo.c defs.h

proginfo_setpiv.o:	proginfo_setpiv.c defs.h

proginfo_rootname.o:	proginfo_rootname.c defs.h

proginfo_nameid.o:	proginfo_nameid.c defs.h

proguserlist.o:		proguserlist.c $(INCS)

progconfig.o:		progconfig.c $(INCS)

progpcsconfig.o:	progpcsconfig.c $(INCS)

progpid.o:		progpid.c $(INCS)

progstampcheck.o:	progstampcheck.c $(INCS)

progprocess.o:		progprocess.c $(INCS)

progpass.o:		progpass.c $(INCS)

proglog.o:		proglog.c $(INCS)

progsvc.o:		progsvc.c $(INCS)

progacc.o:		progacc.c $(INCS)

progwatch.o:		progwatch.c $(INCS)

progexec.o:		progexec.c $(INCS)

progcmd.o:		progcmd.c $(INCS)

progtmpdir.o:		progtmpdir.c $(INCS)

progcheckdir.o:		progcheckdir.c $(INCS)

progreqfile.o:		progreqfile.c $(INCS)

proglogout.o:		proglogout.c $(INCS)

mkplogid.o:		mkplogid.c $(INCS)

util.o:			util.c $(INCS)


progentry.o:		progentry.c progentry.h svckey.h

svcentry.o:		svcentry.c svcentry.h svckey.h

svckey.o:		svckey.c svckey.h

muximsg.o:		muximsg.c muximsg.h

pcsuserfile.o:		pcsuserfile.c pcsconf.h


vecstr_loadfile.o:	vecstr_loadfile.c

vecstr_env.o:		vecstr_env.c

vecstr.o:		vecstr.c vecstr.h

pcspoll.o:		pcspoll.c pcsconf.h

expcook.o:		expcook.c expcook.h

lfm.o:			lfm.c lfm.h

envs.o:			envs.c envs.h

envs_procxe.o:		envs_procxe.c envs.h

envs_subs.o:		envs_subs.c envs.h

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

poller.o:		poller.c poller.h

listenspec.o:		listenspec.c listenspec.h

spawner.o:		spawner.c spawner.h

varsub.o:		varsub.c varsub.h

varsub_extras.o:	varsub_extras.c varsub.h


