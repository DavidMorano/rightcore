# MAKEFILE

T= comsat

TT= testcomsat

ALL= $(T)

SRCROOT= $(PCS)


BINDIR= $(SRCROOT)/bin
INCDIR= $(SRCROOT)/include
LIBDIR= $(SRCROOT)/lib
MANDIR= $(SRCROOT)/man/man1
HELPDIR= $(SRCROOT)/share/help
SBINDIR= $(SRCROOT)/sbin

#LDCRTDIR= /opt/SUNWspro/WS6/lib
#LDCRTDIR= /opt/SUNWspro/SC5.0/lib
#LDCRTDIR= /opt/SUNWspro/SC4.0/lib
#LDCRTDIR= /opt/SUNWspro/lib
LDCRTDIR= $(SRCROOT)/lib


CC= gcc
GPP= g++

CCOPTS_GCCALL= -pthreads -fexceptions
CCOPTS_CCALL= -MT

CCOPTS= $(CCOPTS_GCCALL) -O2 -mcpu=ultrasparc 
#CCOPTS= $(CCOPTS_GCCALL) -g -O
#CCOPTS= $(CCOPTS_GCCALL) -g -pg
#CCOPTS= $(CCOPTS_GCCALL) -g -Wstrict-aliasing -Wall

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

LDRPATH= $(LOCAL)/lib:$(EXTRA)/lib:/usr/lib/lwp

LDFLAGS= $(LF_LDFLAGS) -R$(LIBDIR):$(LDRPATH)


LIBDIRS= -L$(LIBDIR)

LIB0=
LIB1= -lpcs -ldam -lb
LIB2=
LIB3= -luc -lu
LIB4= -L$(GNU)/lib -liconv -lstdc++ -lgcc_eh -lgcc
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

CXREF= cxref
CXREFFLAGS= -R -s

CPP= cpp

LORDER= lorder
TSORT= tsort

RM= rm -f


INCS= config.h defs.h


OBJ00= main.o whatinfo.o proginfo.o proginfo_setpiv.o
OBJ01= progcs.o progcsmsg.o 
OBJ02= proglog.o progloglock.o
OBJ03= progerr.o prognote.o
OBJ04= 
OBJ05=
OBJ06=
OBJ07= printhelp.o 
OBJ08=
OBJ09= termnote.o 
OBJ10= tmpx.o tmpx_getuserlines.o
OBJ11= mailmsg.o mailmsg_loadfd.o
OBJ12= spawner.o
OBJ13= uiconv.o
OBJ14= comsatmsg.o
OBJ15= dispatcher.o

OBJ16= pcsgetprogpath.o prgetprogpath.o
OBJ17=
OBJ18=
OBJ19= mkaddrname.o
OBJ20= initnow.o inet4int.o
OBJ21= wsnwcpynarrow.o
OBJ22= mkplogid.o mnwcpy.o compactstr.o
OBJ23= listenudp.o openport.o opentermnote.o writeto.o getportnum.o

OBJA= $(OBJ00) $(OBJ01) $(OBJ02) $(OBJ03) $(OBJ04) $(OBJ05) $(OBJ06) $(OBJ07)
OBJB= $(OBJ08) $(OBJ09) $(OBJ10) $(OBJ11) $(OBJ12) $(OBJ13) $(OBJ14) $(OBJ15)
OBJC= $(OBJ16) $(OBJ17) $(OBJ18) $(OBJ19) $(OBJ20) $(OBJ21) $(OBJ22) $(OBJ23)

OBJ= $(OBJA) $(OBJB) $(OBJC)

OBJS= $(CRTFRONT) $(OBJ) $(CRTC) $(CRTBACK)

TOBJS= $(CRT0) testcomsat.o $(CRTC)


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

$(T).x:			$(OBJ) Makefile
	makedate -m $(T) > makedate.c
	$(CC) -c $(CFLAGS) makedate.c
	$(LD) -o $@ -m $(LDFLAGS) $(OBJS) $(LIBDIRS) $(LIBS) > $(T).lm

$(T).$(OFF) $(OFF):	$(T).x
	cp -p $(T).x $(T).$(OFF)

testcomsat.x:		$(TOBJS) Makefile
	$(LD) -o $@ $(LDFLAGS) $(TOBJS) $(LIBDIRS) $(LIBS)

$(T).nm nm:		$(T).x
	$(NM) $(NMFLAGS) $(T).x > $(T).nm

$(T).cxref:
	$(CXREF) -c $(CXREFFLAGS) $(SRC) > $(T).cxref

safe:
	makesafe -v=3 -I $(INCDIR) $(OBJ)

strip:			$(T).x
	strip $(T).x
	rm -f $(T).$(OFF) $(T)

install:		install-raw install-help

install-ee:		$(ALL)
	makenewer -r $(ALL) $(BINDIR)

install-raw:		$(T).x
	rm -f $(SBINDIR)/$(T).$(OFF)
	makenewer -r -o rmsuf $(T).x $(SBINDIR)
	-chgrp tty $(SBINDIR)/$(T)
	-chmod g+s $(SBINDIR)/$(T)
	makeinstall ${T}

install-help:		$(T).help
	-mkdir -p $(HELPDIR) 2> /dev/null
	makenewer -o rmsuf $(T).help $(HELPDIR)

again:
	rm -f $(ALL) $(T).x $(TT).x

clean:			again
	makeclean

control:
	uname -n > Control
	date >> Control


main.o:			main.c $(INCS)

whatinfo.o:		whatinfo.c config.h

proginfo.o:		proginfo.c defs.h

proginfo_setpiv.o:	proginfo_setpiv.c defs.h

progreader.o:		progreader.c $(INCS)

progmsg.o:		progmsg.c $(INCS)

progloglock.o:		progloglock.c $(INCS)

proglog.o:		proglog.c proglog.h $(INCS)


csro.o:			csro.c csro.h

comsatmsg.o:		comsatmsg.c comsatmsg.h


getutmpent.o:		getutmpent.c getutmpent.h

spawnproc.o:		spawnproc.c spawnproc.h

openport.o:		openport.c openport.h


userinfo.o:		userinfo.c userinfo.h

vecstr_loadfile.o:	vecstr_loadfile.c

vecstr_env.o:		vecstr_env.c

filemap.o:		filemap.c filemap.h

realname.o:		realname.c realname.h

ipasswd.o:		ipasswd.c ipasswd.h

lastlogfile.o:		lastlogfile.c lastlogfile.h

tmpx.o:			tmpx.c tmpx.h

tmpx_getuserlines.o:	tmpx_getuserlines.c tmpx.h

csr.o:			csr.c csr.h

fsi.o:			fsi.c fsi.h

fistore.o:		fistore.c fistore.h

ciq.o:			ciq.c ciq.h

termnote.o:		termnote.c termnote.h

logfile.o:		logfile.c logfile.h

mailmsg.o:		mailmsg.c mailmsg.h

mailmsg_loadfd.o:	mailmsg_loadfd.c mailmsg.h

sesnotes.o:		sesnotes.c sesnotes.h

spawner.o:		spawner.c spawner.h

uiconv.o:		uiconv.c uiconv.h

chartrans.o:		chartrans.c chartrans.h

termtrans.o:		termtrans.cc termtrans.h 
	$(GPP) -c $(CFLAGS) $(CPPFLAGS) termtrans.cc

hdrdecode.o:		hdrdecode.cc hdrdecode.h 
	$(GPP) -c $(CFLAGS) $(CPPFLAGS) hdrdecode.cc

dispatcher.o:		dispatcher.c dispatcher.h


mailmsgmatenv.o:	mailmsgmatenv.c mailmsgmatenv.h

mailmsgmathdr.o:	mailmsgmathdr.c mailmsgmathdr.h


