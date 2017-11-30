# MAKEFILE

T= libpcs

TPCS= pcsconfs

TPPS= pcspolls

PCSNSO= pcsnso

ALLSO= $(T).so $(TPCS).so $(TPPS).so $(PCSNSO).so
ALL= $(T).a $(ALLSO)

SRCROOT= $(PCS)


BINDIR= $(SRCROOT)/bin
INCDIR= $(SRCROOT)/include
LIBDIR= $(SRCROOT)/lib
MANDIR= $(SRCROOT)/man/man1
HELPDIR= $(SRCROOT)/share/help


CC= $(CGS_CC)
GPP= $(CGS_GPP)

GPPFLAGS= $(CGS_CCFLAGS)

#CCOPTS= $(CCOPTS_CCALL) $(CCOPTS_CCOPT) $(CCOPTS_CCLIB)
#CCOPTS= $(CCOPTS_CCALL) $(CCOPTS_CCOPT) $(CCOPTS_CCLIB) -g

CCOPTS= $(CCOPTS_GCCALL) $(CCOPTS_GCCOPT) $(CCOPTS_GCCLIB)
#CCOPTS= $(CCOPTS_GCCALL) $(CCOPTS_GCCOPT) $(CCOPTS_GCCLIB) -g -Wall 


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
CCFLAGS= $(GPPFLAGS) $(CCOPTS) $(LF_CFLAGS)

#LD= $(CC)


LDA= -z initarray=pcsnsmgr_init -z finiarray=pcsnsmgr_fini

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


SLDFLAGS= $(LF_LDFLAGS) -R$(LIBDIR):$(LOCAL)/lib:$(CGS_RPATH):$(LDRPATH)

SLIBDIRS= -L$(LIBDIR) -L$(LOCAL)/lib

SLIB0= -lpcs 
SLIB1= -ldam -lb
SLIB2=
SLIB3= -luc -lu
SLIB4= $(CGS_LIBS)
SLIB5= -liconv $(LF_LIBS) 
SLIB6= -lsecdb -lproject -lpthread -lthread -lrt -lxnet -lsocket -lnsl
SLIB7=

SLIBS= $(SLIB0) $(SLIB1) $(SLIB2) $(SLIB3) $(SLIB4) $(SLIB5) $(SLIB6) $(SLIB7)

SLIBINFO= $(SLIBDIRS) $(SLIBS)


XLDFLAGS= $(LF_LDFLAGS) -R$(LIBDIR):$(LOCAL)/lib:$(CGS_RPATH):$(LDRPATH)

XLIBDIRS= -L$(LIBDIR) -L$(CGS_LIBDIR)

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


SOBJ00=
SOBJ01= pcsconf.o pcspoll.o
SOBJ02= pcsgetserial.o pcsmsgid.o pcsuserfile.o
SOBJ03= pcsngdir.o pcsgetprog.o pcsgetprogpath.o pcsdialuucp.o
SOBJ04= pcstrustuser.o pcsmailhost.o pcsgetnames.o pcsmailcheck.o
SOBJ05= pcsgetfacility.o pcsgetorg.o
SOBJ06= pcsinfoset.o
SOBJ07= pcsns.o pcsnsmgr.o pcsnsrecs.o pcsnsc.o pcsmsg.o
SOBJ08= pcsunodes.o
SOBJ09= lockfile.o initnow.o
SOBJ10= mheader.o strnnlen.o ndigits.o
SOBJ11= datestr_envelope.o datestr_header.o
SOBJ12= securefile.o
SOBJ13= sfsubstance.o 
SOBJ14=
SOBJ15= envmgr.o outbuf.o
SOBJ16= useraccdb.o nulstr.o
SOBJ17= mailmsgatt.o mailmsgattent.o mimetypes.o mailalias.o
SOBJ18= mailmsghdrs.o
SOBJ19= contypevals.o contentencodings.o ismmclass.o
SOBJ20= modload.o
SOBJ21= thrbase.o thrcomm.o
SOBJ22=
SOBJ23=
SOBJ24= prmktmpdir.o
SOBJ25= fileobject.o nchr.o sfcookkey.o
SOBJ26=
SOBJ27=
SOBJ28=
SOBJ29=
SOBJ30=
SOBJ31=

SOBJ32= mkbestfrom.o
SOBJ33= hasfext.o
SOBJ34=
SOBJ35=
SOBJ36=
SOBJ37=
SOBJ38=
SOBJ39=

SOBJAa= $(SOBJ00) $(SOBJ01) $(SOBJ02) $(SOBJ03)
SOBJAb= $(SOBJ04) $(SOBJ05) $(SOBJ06) $(SOBJ07)
SOBJA= $(SOBJAa) $(SOBJAb)

SOBJBa= $(SOBJ08) $(SOBJ09) $(SOBJ10) $(SOBJ11)
SOBJBb= $(SOBJ12) $(SOBJ13) $(SOBJ14) $(SOBJ15)
SOBJB= $(SOBJBa) $(SOBJBb)

SOBJCa= $(SOBJ16) $(SOBJ17) $(SOBJ18) $(SOBJ19)
SOBJCb= $(SOBJ20) $(SOBJ21) $(SOBJ22) $(SOBJ23) 
SOBJC= $(SOBJCa) $(SOBJCb)

SOBJDa= $(SOBJ24) $(SOBJ25) $(SOBJ26) $(SOBJ27)
SOBJDb= $(SOBJ28) $(SOBJ29) $(SOBJ30) $(SOBJ31) 
SOBJD= $(SOBJDa) $(SOBJDb)

SOBJEa= $(SOBJ32) $(SOBJ33) $(SOBJ34) $(SOBJ35)
SOBJEb= $(SOBJ36) $(SOBJ37) $(SOBJ38) $(SOBJ39)
SOBJE= $(SOBJEa) $(SOBJEb)

SOBJ= $(SOBJA) $(SOBJB) $(SOBJC) $(SOBJD) $(SOBJE)


TPCSOBJ00= pcsconfs.o
TPCSOBJ01= var.o varmk.o varhdr.o

TPCSOBJ= $(TPCSOBJ00) $(TPCSOBJ01)


TPPSOBJ00= pcspolls.o
TPPSOBJ01= thrcomm.o

TPPSOBJ= $(TPPSOBJ00) $(TPPSOBJ01)


PCSNSOBJ= pcsnso.o


AOBJ= $(SOBJ) $(TPCSOBJ) $(TPPSOBJ) $(PCSNSOBJ)


.SUFFIXES:		.ls .i .cx .cs


default:		all

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


so:			$(T).so $(TPCS).so $(TPPS).so $(PCSNSO).so

$(TPCS).so:		$(TPCSOBJ) Makefile $(TPCS).map
	$(LD) -G -o $@ -M $(TPCS).map $(SLDFLAGS) $(TPCSOBJ) $(SLIBINFO)

$(TPPS).so:		$(TPPSOBJ) Makefile $(TPPS).map
	$(LD) -G -o $@ -M $(TPPS).map $(SLDFLAGS) $(TPPSOBJ) $(SLININFO)

$(PCSNSO).so:		$(PCSNSOBJ) Makefile $(PCSNSO).map
	$(LD) -G -o $@ -M $(PCSNSO).map $(SLDFLAGS) $(PCSNSOBJ) $(SLIBINFO)

$(T).a:			$(SOBJ)
	$(AR) -rc $(T).a $?

$(T).so:		$(SOBJ) Makefile $(T).a
	$(LD) -G -o $@ $(SLDFLAGS) $(LDA) $(SOBJ) $(SLIBINFO)

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

install-so:		$(ALLSO)
	makenewer -r $(ALLSO) $(LIBDIR)

safe:
	makesafe -z -v=3 -I $(INCDIR) $(AOBJ)

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

mailalias.o:		mailalias.cc mailalias.h
	$(GPP) -c $(GPPFLAGS) $(CFLAGS) $(CPPFLAGS) mailalias.cc

ids.o:			ids.c ids.h

nulstr.o:		nulstr.c nulstr.h

mailbox.o:		mailbox.c mailbox.h

mailbox_getfrom.o:	mailbox_getfrom.c mailbox.h

mailmsghdrval.o:	mailmsghdrval.c mailmsghdrval.h

dirseen.o:		dirseen.c dirseen.h

contypevals.o:		contypevals.c contypevals.h

contentencodings.o:	contentencodings.c contentencodings.h

envmgr.o:		envmgr.c envmgr.h

outbuf.o:		outbuf.c outbuf.h

useraccdb.o:		useraccdb.c useraccdb.h

nulstr.o:		nulstr.c nulstr.h

paramfile.o:		paramfile.c paramfile.h

varsub.o:		varsub.c varsub.h

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


