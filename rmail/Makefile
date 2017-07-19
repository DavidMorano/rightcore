# MAKEFILE

T= dmail

ALL= $(T)

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


INCDIRS= -I$(INCDIR)

CPPFLAGS= $(DEFS) $(INCDIRS)

CFLAGS= $(CCOPTS)

#LD= $(CC)
#LD= cc
LD= ld

LDFLAGS= -R$(LIBDIR)


LIBDIRS= -L$(LIBDIR)

LIB0=
LIB1= -lpcs -ldam -lb
LIB2=
LIB3= -luc -lu
LIB4= -L$(GNU)/lib -lgcc
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

CRT0= $(CRT1) $(CRTI) $(VALUES)
CRTC= makedate.o

LINT= lint
LINTFLAGS= -uxn -Dlint

NM= nm
NMFLAGS= -xs -v

CPP= cpp


INCS= config.h defs.h


OBJ00= main.o whatinfo.o proginfo.o proginfo_setpiv.o 
OBJ01= expander.o deliver.o copyparts.o
OBJ02= progmsgs.o progprinthdr.o progcomsat.o
OBJ03= mkmsgid.o mkenvfrom.o
OBJ04= parsenodespec.o
OBJ05= received.o mhcom.o recip.o outline.o
OBJ06= 
OBJ07= printhelp.o 
OBJ08=
OBJ09= vecobj_recip.o
OBJ10= pcsconf.o pcsuserfile.o pcstrustuser.o
OBJ11= whitelist.o splitaddr.o
OBJ12= emainfo.o msgid.o msgide.o lkmail.o ema.o
OBJ13= logzones.o dater.o comparse.o tmz.o
OBJ14= mailmsgmatenv.o mailmsgmathdr.o mailmsg.o mailmsg_loadfile.o
OBJ15= ids.o logfile.o keyopt.o mailmsghdrs.o
OBJ16= field_word.o 
OBJ17=
OBJ18= getfname.o getmailgid.o initnow.o getfstype.o
OBJ19= sfsubstance.o headkeymat.o hdrextnum.o
OBJ20=
OBJ21=
OBJ22=
OBJ23=

OBJA= $(OBJ00) $(OBJ01) $(OBJ02) $(OBJ03) $(OBJ04) $(OBJ05) $(OBJ06) $(OBJ07)
OBJB= $(OBJ08) $(OBJ09) $(OBJ10) $(OBJ11) $(OBJ12) $(OBJ13) $(OBJ14) $(OBJ15)
OBJC= $(OBJ16) $(OBJ17) $(OBJ18) $(OBJ19) $(OBJ20) $(OBJ21) $(OBJ22) $(OBJ23)

OBJ= $(OBJA) $(OBJB) $(OBJC)

OBJS= $(CRTFRONT) $(OBJ) $(CRTC) $(CRTBACK)


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
	rcp -p $(T).ee $(T)

$(T).x:			$(OBJ) Makefile
	makedate -m $(T) > makedate.c
	$(CC) -c $(CFLAGS) makedate.c
	$(LD) -o $@ $(LDFLAGS) $(OBJS) $(LIBDIRS) $(LIBS) > $(T).lm

$(T).$(OFF) $(OFF):	$(T).x
	rcp -p $(T).x $(T).$(OFF)

$(T).nm nm:		$(T).x
	$(NM) $(NMFLAGS) $(T).x > $(T).nm

safe:
	makesafe -v=3 -I $(INCDIR) $(OBJ)

strip:			$(T).x
	strip $(T).x
	rm -f $(T).$(OFF) $(T)

install:		install-raw

install-ee:		$(ALL)
	makenewer -r $(ALL) $(BINDIR)
	-chgrp mail $(BINDIR)/$(T).$(OFF)
	-chmod g+s $(BINDIR)/$(T).$(OFF)

install-raw:		$(T).x
	rm -f $(T)
	ln $(T).x $(T)
	rm -f $(BINDIR)/$(T).$(OFF)
	makenewer -r $(T) $(BINDIR)
	-chgrp mail $(BINDIR)/$(T)
	-chmod g+s $(BINDIR)/$(T)

install-help:		$(T).help
	-mkdir -p $(HELPDIR) 2> /dev/null
	makenewer -o rmsuf $(T).help $(HELPDIR)

again:
	rm -f $(ALL) $(T).x $(T).lm 

clean:			again
	makeclean

control:
	uname -n > Control
	date >> Control


main.o:			main.c $(INCS) msgid.h

whatinfo.o:		whatinfo.c config.h

proginfo.o:		proginfo.c defs.h

proginfo_setpiv.o:	proginfo_setpiv.c defs.h

progprinthdr.o:		progprinthdr.c $(INCS)

progmsgs.o:		progmsgs.c $(INCS) mailmsg.h

progcomsat.o:		progcomsat.c $(INCS)

expander.o:		expander.c $(INCS)

mailspooldir.o:		mailspooldir.c $(INCS)

deliver.o:		deliver.c $(INCS) lkmail.h

mkmsgid.o:		mkmsgid.c $(INCS)

mkenvfrom.o:		mkenvfrom.c $(INCS)

boxer.o:		boxer.c $(INCS) lkmail.h

copyparts.o:		copyparts.c $(INCS)

progout.o:		progout.c $(INCS)

mailnotify.o:		mailnotify.c $(INCS) mcmsg.h

parsenodespec.o:	parsenodespec.c $(INCS)

trailaddr.o:		trailaddr.c

mailmsgheadkey.o:	mailmsgheadkey.c

headkeymat.o:		headkeymat.c


recip.o:		recip.c recip.h

outline.o:		outline.c outline.h

mcmsg.o:		mcmsg.c mcmsg.h

getfname.o:		getfname.c getfname.h


vecstr_loadfile.o:	vecstr_loadfile.c

vecstr_env.o:		vecstr_env.c

vecstr.o:		vecstr.c vecstr.h

vecobj_recip.o:		vecobj_recip.c recip.h

mailmsg_loadfile:	mailmsg_loadfile.c mailmsg.h

emainfo.o:		emainfo.c emainfo.h

ema.o:			ema.c ema.h

mailmsghdrs.o:		mailmsghdrs.c mailmsghdrs.h mailmsg.h

mailmsg.o:		mailmsg.c mailmsg.h mailmsgmatenv.h

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

tmtime.o:		tmtime.c tmtime.h


pcsconf.o:		pcsconf.c pcsconf.h

pcsuserfile.o:		pcsuserfile.c

pcstrustuser.o:		pcstrustuser.c



