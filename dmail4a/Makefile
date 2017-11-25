# MAKEFILE

T= dmail

ALL= $(T) dmailbox

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
#CCOPTS= -xO5 -xtarget=ss20/hs22 -xdepend

# UltraSPARC
#CCOPTS= -xO5 -xtarget=ultra -xsafe=mem -xdepend


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

LDFLAGS= $(LF_LDFLAGS) -R$(LIBDIR):$(LOCAL)/lib:$(CGS_RPATH):$(LDRPATH)


LIBDIRS= -L$(LIBDIR) -L$(LOCAL)/lib -L$(CGS_LIBDIR)

LIB0=
LIB1= -lpcs -ldam -lb
LIB2=
LIB3= -luc -lu
LIB4= -liconv $(CGS_LIBS)
LIB5= 
LIB6= -lsecdb -lproject -lpthread -lrt -lxnet -lsocket -lnsl
LIB7= -ldl -lc

LIBS= $(LIB0) $(LIB1) $(LIB2) $(LIB3) $(LIB4) $(LIB5) $(LIB6) $(LIB7)

LIBINFO= $(LIBDIRS) $(LIBS)


XLIBDIRS= -L$(LIBDIR) -L$(LOCAL)/lib -L$(CGS_LIBDIR)

XLIB0=
XLIB1= -ldam -lb
XLIB2= -ldebug
XLIB3= -luc -lu
XLIB4= $(CGS_LIBS)
XLIB5= 
XLIB6= -lsecdb -lproject -lpthread -lrt -lxnet -lsocket -lnsl
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

CRT0= $(CRT1) $(CRTI) $(VALUES)
CRTC= makedate.o

LINT= lint
LINTFLAGS= -uxn -Dlint

NM= nm
NMFLAGS= -xs -v

CPP= cpp


INCS= config.h defs.h


OBJ00= main.o whatinfo.o proginfo.o proginfo_setpiv.o 
OBJ01= proglog.o proglogzone.o proglogenv.o proguserlist.o
OBJ02= progmsgs.o progprinthdr.o progmsgid.o progcomsat.o
OBJ03= progexpand.o prognamecache.o
OBJ04= progdeliver.o progboxer.o 
OBJ05=
OBJ06= parsenodespec.o recipcopyparts.o
OBJ07= printhelp.o 

OBJ08=
OBJ09= vecobj_recip.o
OBJ10= 
OBJ11= recip.o mcmsg.o outline.o
OBJ12= lookaddr.o
OBJ13= mailspool.o
OBJ14= 
OBJ15= 

OBJ16= received.o mhcom.o
OBJ17= whitelist.o splitaddr.o bfliner.o
OBJ18= emainfo.o msgid.o msgide.o lkmail.o ema.o
OBJ19= logzones.o dater.o comparse.o tmz.o
OBJ20= mailmsgmatenv.o mailmsgmathdr.o 
OBJ21= mailmsg.o mailmsg_enver.o
OBJ22= 
OBJ23=

OBJ24= field_word.o 
OBJ25= mailmsghdrs.o namecache.o
OBJ26=
OBJ27= sysrealname.o ipasswd.o realname.o
OBJ28=
OBJ29= 
OBJ30= 
OBJ31= 

OBJ32= pcsconf.o 
OBJ33= pcstrustuser.o
OBJ34=
OBJ35=
OBJ36= sfsubstance.o headkeymat.o hdrextnum.o hdrextid.o
OBJ37=
OBJ38=
OBJ39=

OBJA= $(OBJ00) $(OBJ01) $(OBJ02) $(OBJ03) $(OBJ04) $(OBJ05) $(OBJ06) $(OBJ07)
OBJB= $(OBJ08) $(OBJ09) $(OBJ10) $(OBJ11) $(OBJ12) $(OBJ13) $(OBJ14) $(OBJ15)
OBJC= $(OBJ16) $(OBJ17) $(OBJ18) $(OBJ19) $(OBJ20) $(OBJ21) $(OBJ22) $(OBJ23)
OBJD= $(OBJ24) $(OBJ25) $(OBJ26) $(OBJ27) $(OBJ28) $(OBJ29) $(OBJ30) $(OBJ31)
OBJE= $(OBJ32) $(OBJ33) $(OBJ34) $(OBJ35) $(OBJ26) $(OBJ37) $(OBJ38) $(OBJ39)

OBJ= $(OBJA) $(OBJB) $(OBJC) $(OBJD) $(OBJE)

OBJS= $(CRTFRONT) $(OBJ) $(CRTC) $(CRTBACK)


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


$(T):			$(T).ee
	rcp -p $(T).ee $(T)

$(T).x:			$(OBJ) Makefile
	makedate -m $(T) > makedate.c
	$(CC) -c $(CFLAGS) makedate.c
	$(LD) -o $@ $(LDFLAGS) $(OBJS) $(LIBINFO) > $(T).lm

$(T).$(OFF) $(OFF):	$(T).x
	rcp -p $(T).x $(T).$(OFF)

$(T).nm nm:		$(T).x
	$(NM) $(NMFLAGS) $(T).x > $(T).nm

safe:
	makesafe -v=3 -I $(INCDIR) $(OBJ)

strip:			$(T).x
	strip $(T).x
	rm -f $(T).$(OFF) $(T)

install:		install-raw install-help

install-ee:		$(ALL)
	makenewer -r $(ALL) $(BINDIR)
	-chgrp mail $(BINDIR)/$(T).$(OFF)
	-chmod g+s $(BINDIR)/$(T).$(OFF)

install-raw:		$(T).x
	makeinstall $(ALL)

install-help:		$(T).help
	makeinstall-help $(ALL)

again:
	rm -f $(ALL) $(T).x $(T).lm 

clean:			again
	rm -f *.ls *.ln *.i *.o *.so *.x *.lm

control:
	uname -n > Control
	date >> Control


main.o:			main.c $(INCS) msgid.h

whatinfo.o:		whatinfo.c config.h

proginfo.o:		proginfo.c defs.h

proginfo_setpiv.o:	proginfo_setpiv.c defs.h

proglog.o:		proglog.c $(INCS)

proglogzone.o:		proglogzone.c $(INCS)

proglogenv.o:		proglogenv.c $(INCS)

proguserlist.o:		proguserlist.c $(INCS)

progprinthdr.o:		progprinthdr.c $(INCS)

progmsgid.o:		progmsgid.c $(INCS)

progmsgs.o:		progmsgs.c $(INCS) mailmsg.h

progexpand.o:		progexpand.c $(INCS)

prognamecache.o:	prognamecache.c $(INCS)

progcomsat.o:		progcomsat.c $(INCS)

prognotify.o:		prognotify.c $(INCS) mcmsg.h

progdeliver.o:		progdeliver.c $(INCS) lkmail.h

progboxer.o:		progboxer.c $(INCS)

mkenvfrom.o:		mkenvfrom.c $(INCS)

mailspooldir.o:		mailspooldir.c $(INCS)

recipcopyparts.o:	recipcopyparts.c $(INCS)

parsenodespec.o:	parsenodespec.c $(INCS)

trailaddr.o:		trailaddr.c

mailmsgheadkey.o:	mailmsgheadkey.c

headkeymat.o:		headkeymat.c


pcstrustuser.o:		pcstrustuser.c


recip.o:		recip.c recip.h

outline.o:		outline.c outline.h

mcmsg.o:		mcmsg.c mcmsg.h

bfliner.o:		bfliner.c bfliner.h

vecstr_loadfile.o:	vecstr_loadfile.c

vecstr_env.o:		vecstr_env.c

vecstr.o:		vecstr.c vecstr.h

vecobj.o:		vecobj.c vecobj.h

vecobj_recip.o:		vecobj_recip.c recip.h

mailmsg_loadfile:	mailmsg_loadfile.c mailmsg.h

emainfo.o:		emainfo.c emainfo.h

ema.o:			ema.c ema.h

mailmsghdrs.o:		mailmsghdrs.c mailmsghdrs.h mailmsg.h

mailmsg.o:		mailmsg.c mailmsg.h mailmsgmatenv.h

mailmsg_loadfile.o:	mailmsg_loadfile.c mailmsg.h

mailmsg_enver.o:	mailmsg_enver.c mailmsg.h mailmsg_enver.h

mailmsgmatenv.o:	mailmsgmatenv.c mailmsgmatenv.h

lkmail.o:		lkmail.c lkmail.h

mapstrint.o:		mapstrint.c mapstrint.h

logfile.o:		logfile.c logfile.h

logzones.o:		logzones.c logzones.h

dater.o:		dater.c dater.h tmz.h comparse.h

comparse.o:		comparse.c comparse.h

tmz.o:			tmz.c tmz.h

msgid.o:		msgid.c msgid.h msgide.h

msgide.o:		msgide.c msgide.h

vecobj.o:		vecobj.c vecobj.h lookaside.h

lookaside.o:		lookaside.c lookaside.h pq.h

pq.o:			pq.c pq.h

filebuf.o:		filebuf.c filebuf.h

ids.o:			ids.c ids.h

sbuf.o:			sbuf.c sbuf.h

splitaddr.o:		splitaddr.c splitaddr.h

whitelist.o:		whitelist.c whitelist.h splitaddr.h

received.o:		received.c received.h mhcom.h

mhcom.o:		mhcom.c mhcom.h

densitydb.o:		densitydb.c densitydb.h densitydbe.h

densitydbe.o:		densitydbe.c densitydbe.h

keyopt.o:		keyopt.c keyopt.h

pcsconf.o:		pcsconf.c pcsconf.h

pcspoll.o:		pcspoll.c pcspoll.h

namecache.o:		namecache.c namecache.h

sysrealname.o:		sysrealname.c sysrealname.h

grmems.o:		grmems.c grmems.h vecelem.h

vecelem.o:		vecelem.c vecelem.h

vechand.o:		vechand.c vechand.h

tmtime.o:		tmtime.c tmtime.h

ugetpw.o:		ugetpw.c ugetpw.h

upwcache.o:		upwcache.c upwcache.h

recarr.o:		recarr.c recarr.h

modload.o:		modload.c modload.h

realname.o:		realname.c realname.h

ipasswd.o:		ipasswd.c ipasswd.h
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<

lookaddr.o:		lookaddr.c lookaddr.h whitelist.h splitaddr.h

mailspool.o:		mailspool.c mailspool.h

ms.o:			ms.c ms.h

field.o:		field.c field.h

field_word.o:		field_word.c field.h


# testing

T01= testgrmems

T01OBJ00= $(T01).o
T01OBJ01= grmems.o
T01OBJ02= vecelem.o vechand.o vecobj.o
T01OBJ03= ugetpw.o upwcache.o recarr.o

T01OBJ= $(T01OBJ00) $(T01OBJ01) $(T01OBJ02) $(T01OBJ03)

T01OBJS= $(T01OBJ)

$(T01).o:		$(T01).c grmems.h vecelem.h

$(T01)-safe:
	makesafe -v=3 -I $(INCDIR) $(T01OBJ)

$(T01).x:		Makefile $(T01OBJ)
	$(CC) -o $@ $(T01OBJ) $(XLIBDIRS) $(XLIBS)


T02= testsysrealname

T02OBJ00= $(T02).o
T02OBJ01= sysrealname.o
T02OBJ02= vecelem.o vechand.o vecobj.o
T02OBJ03= ugetpw.o upwcache.o recarr.o

T02OBJ= $(T02OBJ00) $(T02OBJ01) $(T02OBJ02) $(T02OBJ03)

T02OBJS= $(T02OBJ)

$(T02).o:		$(T02).c sysrealname.h vecelem.h

$(T02)-safe:
	makesafe -v=3 -I $(INCDIR) $(T02OBJ)

$(T02).x:		Makefile $(T02OBJ)
	$(CC) -o $@ $(T02OBJ) $(XLIBDIRS) $(XLIBS)


