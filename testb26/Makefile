# MAKEFILE

T= testb26

ALL = $(T) $(TR)

SRCROOT= $(LOCAL)


BINDIR= $(SRCROOT)/bin
INCDIR= $(SRCROOT)/include
LIBDIR= $(SRCROOT)/lib
MANDIR= $(SRCROOT)/man/man1
HELPDIR= $(SRCROOT)/share/help

EXTRABIN= /usr/extra/sbin


#LDCRTDIR= /opt/SUNWspro/WS6/lib
#LDCRTDIR= /opt/SUNWspro/SC5.0/lib
#LDCRTDIR= /opt/SUNWspro/SC4.0/lib
#LDCRTDIR= /opt/SUNWspro/lib
LDCRTDIR= $(SRCROOT)/lib


CC= gcc

CCOPTS= -O -mcpu=ultrasparc # -fpic
CCOPTS= -g
#CCOPTS=
#CCOPTS= -g -pg

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

LDFLAGS= -m -R$(LIBDIR)


LIBDIRS= -L$(LIBDIR)

LIB0=
LIB1= -Bstatic -lpcs -ldam -Bdynamic
LIB2= -Bstatic -lb -luc -Bdynamic
LIB3= -Bstatic -lu -Bdynamic
LIB4= -L$(GNU)/lib -lgcc
LIB5= -L/usr/lib -lbsm -lpam -lcmd
LIB6= -lsecdb -lproject -lpthread -lrt -lxnet -lsocket -lnsl
LIB7= -ldl -lc

LIBS= $(LIB0) $(LIB1) $(LIB2) $(LIB3) $(LIB4) $(LIB5) $(LIB6) $(LIB7)


CRTI= $(LDCRTDIR)/crti.o
CRT1= $(LDCRTDIR)/crt1.o
VALUES= $(LDCRTDIR)/values-xa.o
CRTN= $(LDCRTDIR)/crtn.o

CRT0= $(CRTI) $(CRT1) $(VALUES)
CRTC= makedate.o

LINT= lint
LINTFLAGS= -uxn -Dlint

NM= nm
NMFLAGS= -xs

CPP= cpp


INCS= config.h defs.h


OBJ00= main.o whatinfo.o proginfo.o proginfo_setpiv.o 
OBJ01= 
OBJ02= ctb26.o cfb26.o
OBJ03= 
OBJ04= 
OBJ05= 
OBJ06=
OBJ07= printhelp.o
OBJ08=
OBJ09= 
OBJ10= 
OBJ11= 
OBJ12= 
OBJ13= 
OBJ14= 
OBJ15= 
OBJ16= 
OBJ17= 
OBJ18=
OBJ19= 
OBJ20= 
OBJ21= 
OBJ22= 
OBJ23= 
OBJ24= 
OBJ25= 
OBJ26= 
OBJ27= 
OBJ28= 
OBJ29= 
OBJ30=
OBJ31= 

OBJA= $(OBJ00) $(OBJ01) $(OBJ02) $(OBJ03) $(OBJ04) $(OBJ05) $(OBJ06) $(OBJ07)
OBJB= $(OBJ08) $(OBJ09) $(OBJ10) $(OBJ11) $(OBJ12) $(OBJ13) $(OBJ14) $(OBJ15)
OBJC= $(OBJ16) $(OBJ17) $(OBJ18) $(OBJ19) $(OBJ20) $(OBJ21) $(OBJ22) $(OBJ23)
OBJD= $(OBJ24) $(OBJ25) $(OBJ26) $(OBJ27) $(OBJ28) $(OBJ29) $(OBJ30) $(OBJ31)

OBJ= $(OBJA) $(OBJB) $(OBJC) $(OBJD)

OBJS= $(CRT0) $(OBJ) $(CRTC)



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

$(T).x:			$(OBJ) Makefile
	rm -f $(T).x
	makedate -m $(T) > makedate.c
	$(CC) -c $(CFLAGS) makedate.c
	$(LD) -o $@ $(LDFLAGS) $(OBJS) $(LIBDIRS) $(LIBS) $(CRTN) > $(T).lm

$(T).$(OFF) $(OFF):	$(T).x
	cp -p $(T).x $(T).$(OFF)

$(T).nm nm:		$(T).x
	$(NM) $(NMFLAGS) $(T).x > $(T).nm

safe:
	makesafe -I $(INCDIR) $(OBJ)

strip:			$(T).x
	strip $(T).x
	rm -f $(T).$(OFF) $(T)

install:		install-raw

install-ee:		$(ALL)
	makenewer -r $(ALL) $(BINDIR)

install-raw:		$(T).x
	rm -f $(T)
	ln -f $(T).x $(T)
	rm -f $(BINDIR)/$(T).$(OFF)
	makenewer -r $(T) $(BINDIR)

install-help:		$(T).help
	rm -f $(T)
	ln $(T).help $(T)
	-mkdir -p $(HELPDIR) 2> /dev/null
	makenewer $(T) $(HELPDIR)

install.telserv:		$(T).x
	rm -f telserv
	ln -d $(T).x telserv
	makenewer -r telserv $(EXTRABIN)

again:
	rm -f $(ALL) $(T).x $(T).lm

clean:			again
	rm -f *.o *.i *.s *.ls

control:
	uname -n > Control
	date >> Control


main.o:			main.c $(INCS)

whatinfo.o:		whatinfo.c config.h

proginfo.o:		proginfo.c defs.h

proginfo_setpiv_o:	proginfo_setpiv.c defs.h

proginfo_rootname.o:	proginfo_rootname.c defs.h

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

procse.o:		procse.c procse.h

svckey.o:		svckey.c svckey.h


getutmpent.o:		getutmpent.c getutmpent.h

pcspoll.o:		pcspoll.c pcsconf.h

pcsuserfile.o:		pcsuserfile.c pcsconf.h


userinfo.o:		userinfo.c userinfo.h

vecstr_loadfile.o:	vecstr_loadfile.c

vecstr_env.o:		vecstr_env.c

vecstr.o:		vecstr.c vecstr.h

filemap.o:		filemap.c filemap.h

filebuf.o:		filebuf.c filebuf.h

realname.o:		realname.c realname.h

ipasswd.o:		ipasswd.c ipasswd.h

lastlogfile.o:		lastlogfile.c lastlogfile.h

tmpx.o:			tmpx.c tmpx.h

expcook.o:		expcook.c expcook.h

envs.o:			envs.c envs.h

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

serialbuf.o:		serialbuf.c serialbuf.h

hostinfo.o:		hostinfo.c hostinfo.h

hostaddr.o:		hostaddr.c hostaddr.h

field_svcargs.o:	field_svcargs.c field.h

field_srvarg.o:		field_srvarg.c field.h



