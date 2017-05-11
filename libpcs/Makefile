# MAKEFILE

T= libpcs

TPCS= pcsconfs

TPPS= pcspolls

PCSNSO= pcsnso

#ALL= $(T).a $(T).so
ALL= $(T).a $(T).so $(TPCS).so $(TPPS).so $(PCSNSO).so

SRCROOT= $(PCS)


BINDIR= $(SRCROOT)/bin
INCDIR= $(SRCROOT)/include
LIBDIR= $(SRCROOT)/lib
MANDIR= $(SRCROOT)/man/man1
HELPDIR= $(SRCROOT)/share/help


CC= $(CGS_CC)
GPP= $(CGS_GPP)

CCOPTS= $(CCOPTS_GCCOPT) $(CCOPTS_GCCALLL) $(CCOPTS_GCCLIB)
#CCOPTS= $(CCOPTS_GCCALLL) $(CCOPTS_GCCLIB)
#CCOPTS= -g -Wstrict-aliasing $(CCOPTS_GCCALL) $(CCOPTS_GCCLIB) 
#CCOPTS= -g -Wall $(CCOPTS_GCCALL) $(CCOPTS_GCCLIB) 


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


CFLAGS= $(CCOPTS) $(LF_CFLAGS)

#LD= $(CC)


LDRPATH= $(EXTRA)/lib:/usr/lib/lwp

LDFLAGS= $(LF_LDFLAGS) -R$(LIBDIR):$(LOCAL)/lib:$(CGS_RPATH):$(LDRPATH)


LIBDIRS= -L$(LIBDIR) -L$(LOCAL)/lib -L$(CGS_LIBDIR)

LIB0=
LIB1= -ldam -lb
LIB2=
LIB3= -luc -lu
LIB4= -liconv $(CGS_LIBS)
LIB5= $(LF_LIBS) 
LIB6= -lsecdb -lproject -lpthread -lthread -lrt -lxnet -lsocket -lnsl
LIB7= -ldl

LIBS= $(LIB0) $(LIB1) $(LIB2) $(LIB3) $(LIB4) $(LIB5) $(LIB6) $(LIB7)

LIBINFO= $(LIBDIRS) $(LIBS)


SLIBDIRS= -L$(LIBDIR) 

SLIB0= -lpcs 
SLIB1= -ldam -lb
SLIB2=
SLIB3= -luc -lu
SLIB4= $(CGS_LIBS)
SLIB5= $(LF_LIBS) 
SLIB6= -lsecdb -lproject -lpthread -lthread -lrt -lxnet -lsocket -lnsl
SLIB7=

SLIBS= $(SLIB0) $(SLIB1) $(SLIB2) $(SLIB3) $(SLIB4) $(SLIB5) $(SLIB6) $(SLIB7)

SLIBINFO= $(SLIBDIRS) $(SLIBS)


XLIBDIRS= -L$(LIBDIR)

XLIB0=
XLIB1= -ldam -lb
XLIB2= -ldebug
XLIB3= -luc -lu
XLIB4=
XLIB5= $(LF_LIBS) 
XLIB6= -lsecdb -lproject -lpthread -lthread -lrt -lxnet -lsocket -lnsl
XLIB7= -ldl

XLIBS= $(XLIB0) $(XLIB1) $(XLIB2) $(XLIB3) $(XLIB4) $(XLIB5) $(XLIB6) $(XLIB7)

XLIBINFO= $(XLIBDIRS) $(XLIBS)


LINT= lint
LINTFLAGS= $(LF_LINTFLAGS) -uxn -Dlint

NM= nm
NMFLAGS= -sx -v

CPP= cpp

LORDER= lorder
TSORT= tsort

RM= rm -f
TOUCH= /bin/touch


INCS= pcsconf.h mimetypes.h


OBJ00=
OBJ01= pcsconf.o pcspoll.o
OBJ02= pcsgetserial.o pcsmsgid.o pcsuserfile.o
OBJ03= pcsngdir.o pcsgetprog.o pcsgetprogpath.o pcsdialuucp.o
OBJ04= pcstrustuser.o pcsmailhost.o pcsgetnames.o pcsmailcheck.o
OBJ05= pcsgetfacility.o pcsgetorg.o
OBJ06= pcsinfoset.o
OBJ07= pcsns.o pcsnsmgr.o pcsnsrecs.o pcsnsc.o pcsmsg.o
OBJ08= pcsunodes.o
OBJ09= lockfile.o initnow.o
OBJ10= mheader.o strnnlen.o ndigits.o
OBJ11= datestr_envelope.o datestr_header.o
OBJ12= securefile.o bopenroot.o 
OBJ13= sfsubstance.o 
OBJ14=
OBJ15= envmgr.o outbuf.o
OBJ16= useraccdb.o nulstr.o
OBJ17= mailmsgatt.o mailmsgattent.o mimetypes.o mailalias.o
OBJ18= mailmsghdrs.o
OBJ19= contentencodings.o ismmclass.o
OBJ20= modload.o
OBJ21= thrbase.o thrcomm.o
OBJ22=
OBJ23=
OBJ24= prmktmpdir.o
OBJ25= fileobject.o nchr.o sfcookkey.o
OBJ26=
OBJ27=
OBJ28=
OBJ29=
OBJ30=
OBJ31=

OBJ32= mkbestfrom.o
OBJ33= hasalldig.o hasfext.o
OBJ34=
OBJ35=
OBJ36=
OBJ37=
OBJ38=
OBJ39=

OBJA= $(OBJ00) $(OBJ01) $(OBJ02) $(OBJ03) $(OBJ04) $(OBJ05) $(OBJ06) $(OBJ07)
OBJB= $(OBJ08) $(OBJ09) $(OBJ10) $(OBJ11) $(OBJ12) $(OBJ13) $(OBJ14) $(OBJ15)
OBJC= $(OBJ16) $(OBJ17) $(OBJ18) $(OBJ19) $(OBJ20) $(OBJ21) $(OBJ22) $(OBJ23) 
OBJD= $(OBJ24) $(OBJ25) $(OBJ26) $(OBJ27) $(OBJ28) $(OBJ29) $(OBJ30) $(OBJ31) 
OBJE= $(OBJ32) $(OBJ33) $(OBJ34) $(OBJ35) $(OBJ36) $(OBJ37) $(OBJ38) $(OBJ39)

OBJ= $(OBJA) $(OBJB) $(OBJC) $(OBJD) $(OBJE)


TPCSOBJ00= pcsconfs.o
TPCSOBJ01= var.o varmk.o varhdr.o

TPCSOBJ= $(TPCSOBJ00) $(TPCSOBJ01)


TPPSOBJ00= pcspolls.o
TPPSOBJ01= thrcomm.o

TPPSOBJ= $(TPPSOBJ00) $(TPPSOBJ01)


PCSNSOBJ= pcsnso.o


.SUFFIXES:		.ls .i .cx .cs


default:		all

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


so:			$(TPCS).so $(TPPS).so $(PCSNSO).so

$(TPCS).so:		$(TPCSOBJ) Makefile $(TPCS).map
	$(LD) -G -o $@ -M $(TPCS).map $(LDFLAGS) $(TPCSOBJ) $(SLIBINFO)

$(TPPS).so:		$(TPPSOBJ) Makefile $(TPPS).map
	$(LD) -G -o $@ -M $(TPPS).map $(LDFLAGS) $(TPPSOBJ) $(SLIBINDO)

$(PCSNSO).so:		$(PCSNSOBJ) Makefile $(PCSNSO).map
	$(LD) -G -o $@ -M $(PCSNSO).map $(LDFLAGS) $(PCSNSOBJ) $(SLIBINFO)

$(T).a:			$(OBJ)
	$(AR) -rc $(T).a $?

$(T).so:		$(OBJ) Makefile $(T).a
	$(LD) -G -o $@ $(LDFLAGS) $(OBJ) $(LIBINFO)

$(T).order order:	$(OBJ) $(T).a
	$(LORDER) $(T).a | $(TSORT) > $(T).order
	$(RM) $(T).a
	while read O ; do $(AR) -cr $(T).a $${O} ; done < $(T).order

$(T).nm:		$(T).so
	$(NM) $(NMFLAGS) $(T).so > $(T).nm

install-pre:
	filefind . -s h | makenewer -af - -d $(INCDIR)

install:		install-pre $(ALL) $(INCS)
	makenewer -r $(ALL) $(LIBDIR)
	makenewer -z $(INCS) $(INCDIR)
	makeinstall $(T) $(TPCS) $(TPPS) $(PCSNSO)

install-so:		$(ALL)
	makenewer -r $(ALL) $(LIBDIR)

safe:
	makesafe -v=3 -I $(INCDIR) $(OBJ)

again:
	rm -f $(ALL) $(T).so $(T).a

clean:			again
	makeclean

control:
	uname -n > Control
	date >> Control


pcsconf.o:		pcsconf.c pcsconf.h

pcsconfs.o:		pcsconfs.c pcsconf.h

pcspolls.o:		pcspolls.c pcspolls.h

pcsnsc.o:		pcsnsc.c pcsnsc.h

pcspoll.o:		pcspoll.c pcsconf.h


pcsgetserial.o:		pcsgetserial.c pcsconf.h

pcsdialuucp.o:		pcsdialuucp.c pcsconf.h

pcsgetdate.o:		pcsgetdate.c pcsconf.h

pcsgetprog.o:		pcsgetprog.c pcsconf.h

pcsgetprogpath.o:	pcsgetprogpath.c pcsconf.h

pcsmailhost.o:		pcsmailhost.c pcsconf.h

pcsmsgid.o:		pcsmsgid.c pcsconf.h

pcsngdir.o:		pcsngdir.c pcsconf.h

pcsuserfile.o:		pcsuserfile.c pcsconf.h

pcstrustuser.o:		pcstrustuser.c pcsconf.h

pcsgetnames.o:		pcsgetnames.c pcsconf.h

pcsmailcheck.o:		pcsmailcheck.c pcsconf.h mailbox.h

pcsgetfacility.o:	pcsgetfacility.c


mimetypes.o:		mimetypes.c mimetypes.h

mailmsgatt.o:		mailmsgatt.c mailmsgatt.h

mailmsgattent.o:	mailmsgattent.c mailmsgattent.h

mailmsghdrs.o:		mailmsghdrs.c mailmsghdrs.h

mailalias.o:		mailalias.c mailalias.h

ids.o:			ids.c ids.h

nulstr.o:		nulstr.c nulstr.h

mailbox.o:		mailbox.c mailbox.h

mailbox_getfrom.o:	mailbox_getfrom.c mailbox.h

mailmsghdrval.o:	mailmsghdrval.c mailmsghdrval.h

dirseen.o:		dirseen.c dirseen.h

contentencodings.o:	contentencodings.c contentencodings.h

envmgr.o:		envmgr.c envmgr.h

outbuf.o:		outbuf.c outbuf.h

useraccdb.o:		useraccdb.c useraccdb.h

nulstr.o:		nulstr.c nulstr.h

paramfile.o:		paramfile.c paramfile.h

modload.o:		modload.c modload.h

vars.o:			vars.c vars.h

thrbase.o:		thrbase.c thrbase.h

thrcomm.o:		thrcomm.c thrcomm.h

mailmsghdrfold.o:	mailmsghdrfold.h

pcsns.o:		pcsns.c pcsns.h

pcsnso.o:		pcsnso.c pcsnso.h

pcsnsmgr.o:		pcsnsmgr.c pcsnsmgr.h

pcsnsrecs.o:		pcsnsrecs.c pcsnsrecs.h

pcsmsg.o:		pcsmsg.c pcsmsg.h

pcsunodes.o:		pcsunodes.c pcsunodes.h


upt.o:			upt.c upt.h

securefile.o:		securefile.c

ismmclass.o:		ismmclass.c ismmclass.h

hdrextnum.o:		hdrextnum.c

hdrextid.o:		hdrextid.c


# testing

X01= testmailmsghdrfold

X01OBJ= $(X01).o mailmsghdrfold.o ncolchar.o ncolstr.o

$(X01).o:		$(X01).c mailmsghdrfold.h

$(X01).x:		$(X01OBJ)
	$(CC) -o $@ $(X01OBJ) $(XLIBDIRS) $(XLIBS)


X02= testpcsns

X02OBJ= $(X02).o pcsns.o

$(X02).o:		$(X02).c $(X02).h

$(X02).x:		$(X02OBJ)
	$(CC) -o $@ $(X02OBJ) $(XLIBDIRS) $(XLIBS)


X03= testpcsunodes

X03OBJ= $(X03).o pcsunodes.o

$(X03).o:		$(X03).c $(X03).h

$(X03).x:		$(X03OBJ)
	$(CC) -o $@ $(X03OBJ) $(XLIBDIRS) $(XLIBS)


