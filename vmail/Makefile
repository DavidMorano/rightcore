# MAKEFILE (vmail)

T= vmail

ALL= $(T)

#PCS= /usr/add-on/pcs
SRCROOT= $(PCS)


BINDIR= $(SRCROOT)/bin
INCDIR= $(SRCROOT)/include
LIBDIR= $(SRCROOT)/lib
MANDIR= $(SRCROOT)/man/man1
HELPDIR= $(SRCROOT)/share/help
PLIBDIR= $(SRCROOT)/lib/vmail

LDCRTDIR= $(CGS_CRTDIR)
VALDIR= $(CGS_VALDIR)


CC= $(CGS_CC)
GPP= $(CGS_GPP)

GPPFLAGS= $(CGS_CCFLAGS)
CCOPTS_GCCALL= -pthreads -fexceptions
CCOPTS_CCALL= -MT

CCOPTS_GCCOPTSGPP= $(CGS_CCFLAGS)

CCOPTS= $(CCOPTS_GCCALL) -O3 -mcpu=ultrasparc
#CCOPTS= $(CCOPTS_GCCALL) -g -O
#CCOPTS= $(CCOPTS_GCCALL) -g -pg
#CCOPTS= $(CCOPTS_GCCALL) -g -Wstrict-aliasing
#CCOPTS= $(CCOPTS_GCCALL) -g -Wall

#CCOPTS= $(CCOPTS_CCALL) -g -xs

# PROF profiling w/ GCC
#CCOPTS= $(CCOPTS_GCCALL) -O -mcpu=ultrasparc -p -g
# GPROF profiling w/ GCC
#CCOPTS= $(CCOPTS_GCCALL) -O3 -mcpu=ultrasparc -pg -g

# HyperSPARC
#CCOPTS= -xO5 -xtarget=ss20/hs22 -dalign -xdepend

# UltraSPARC
#CCOPTS= -xO5 -xtarget=ultra -xsafe=mem -dalign -xdepend


CMDHELP= $(T).cmdhelp

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
CCFLAGS= $(GPPFLAGS) $(CFLAGS)

#LD= $(CC)
#LD= cc
LD= ld


LDRPATH= $(LOCAL)/lib:$(GNU)/lib:$(EXTRA)/lib:/usr/lib/lwp

# regular
LDFLAGS= $(LF_LDFLAGS) -R$(LIBDIR):$(LDRPATH)
# for profiling w/ PROF
LDFLAGS_P= $(LF_LDFLAGS) -L/usr/lib/libp -R$(LIBDIR):$(LDRPATH)
# for profiling w/ GPROF
LDFLAGS_G= $(LF_LDFLAGS) -L/usr/lib/libp -R$(LIBDIR):$(LDRPATH)


LIBDIRS= -L$(LIBDIR) -L$(LOCAL)/lib -L$(CGS_LIBDIR) 

LIB0=
LIB1= -lpcs -ldam -lb
LIB2=
LIB3= -luc -lu
LIB4= -liconv $(CGS_LIBS)
LIB5= 
# this has the addition of |libthread| to force linking of kernel-only threads
LIB6= -lsecdb -lproject -lpthread -lthread -lrt -lxnet -lsocket -lnsl
LIB7= -ldl -lc

LIBS= $(LIB0) $(LIB1) $(LIB2) $(LIB3) $(LIB4) $(LIB5) $(LIB6) $(LIB7)

LIBINFO= $(LIBDIRS) $(LIBS)


TLIBDIRS= -L$(LIBDIR) -L$(CGS_LIBDIR)

TLIB0=
TLIB1= -lpcs -ldam -lb
TLIB2= -ldebug
TLIB3= -luc -lu
TLIB4= -liconv $(CGS_LIBS)
TLIB5=
TLIB6= -lsecdb -lproject -lpthread -lthread -lrt -lxnet -lsocket -lnsl
TLIB7= -ldl

TLIBS= $(TLIB0) $(TLIB1) $(TLIB2) $(TLIB3) $(TLIB4) $(TLIB5) $(TLIB6) $(TLIB7)

TLIBINFO= $(TLIBDIRS) $(TLIBS)


VALUES= $(CGS_VALDIR)/$(CGS_VALUES)

CRT1= $(LDCRTDIR)/crt1.o
CRTI= $(LDCRTDIR)/crti.o
CRTBEGIN= $(LDCRTDIR)/crtbegin.o
MCRT1= $(LDCRTDIR)/mcrt1.o
GCRT1= $(LDCRTDIR)/gcrt1.o
CRTEND= $(LDCRTDIR)/crtend.o
CRTN= $(LDCRTDIR)/crtn.o

CRTFRONT= $(CRT1) $(CRTI) $(VALUES) $(CRTBEGIN)
CRTBACK= $(CRTEND) $(CRTN)

# for regular (no profiling)
CRT0= $(CRT1) $(CRTI) $(VALUES) $(CRTBEGIN)
# for 'prof(1)'
MCRT0= $(CRTI) $(MCRT1) $(VALUES) $(CRTBEGIN)
# for 'gprof(1)'
GCRT0= $(GCRT1) $(CRTI) $(GMON) $(VALUES) $(CRTBEGIN)

CRTC= makedate.o

LINT= lint
LINTFLAGS= -uxn -Dlint

NM= nm
NMFLAGS= -xs -v

CPP= cpp


INCS= config.h defs.h profile.h 


OBJ00= main.o whatinfo.o proginfo.o proginfo_setpiv.o proginfo_rootname.o
OBJ01= progopts.o progconf.o progterm.o progmailget.o proguserlist.o
OBJ02= proglog.o progvmerr.o
OBJ03= inter.o inter_trans.o
OBJ04= display.o ds.o 
OBJ05= mailmsgfile.o mailmsgviewer.o
OBJ06= timestr_scandate.o digit3.o
OBJ07= printhelp.o kshlib.o sesmsg.o msgdata.o
OBJ08=
OBJ09=
OBJ10=
OBJ11= dater.o dater_getdate.o 
OBJ12= comparse.o tmz.o envhelp.o zos.o
OBJ13= mailmsgmatenv.o mailmsgmathdr.o mailmsgfrom.o
OBJ14= mailbox.o mailbox_getfrom.o tmpmailboxes.o
OBJ15= mailmsg.o mailmsg_loadmb.o mailmsg_enver.o

OBJ16= mailmsghdrval.o mailmsghdrfold.o mbcache.o
OBJ17= td.o termstr.o uterm.o uterm_readcmd.o termcmd.o
OBJ18= keysymer.o kbdinfo.o cmdmap.o subprocs.o
OBJ19= dirseen.o ema.o useraccdb.o linefold.o
OBJ20= sigign.o
OBJ21= varray.o
OBJ22= hdrdecode.o qpdecoder.o b64decoder.o utf8decoder.o obuf.o
OBJ23= chartrans.o uiconv.o strmgr.o

OBJ24=
OBJ25=
OBJ26=
OBJ27=
OBJ28=
OBJ29=
OBJ30= headkeymat.o mkdisplayable.o sfsubstance.o
OBJ31= mkbestaddr.o mkbestfrom.o mktmpuserdir.o

OBJ32=
OBJ33= tabexpand.o termconseq.o
OBJ34= mailboxappend.o hdrextnum.o
OBJ35=
OBJ36= defproc.o sfthing.o
OBJ37= nlinecols.o
OBJ38= spawnproc.o spawncmdproc.o getprogpath.o
OBJ39= pcsgetorg.o pcsgetprog.o pcsmailcheck.o

OBJ40= initnow.o
OBJ41= wsncols.o
OBJ42= iscmdstart.o
OBJ43= strdcpycompact.o
OBJ44= straltwchar.o
OBJ45=
OBJ46=
OBJ47=

OBJ48= getsystypenum.o getproviderid.o
OBJ49=
OBJ50= sigignores.o sigdefaults.o
OBJ51=
OBJ52=
OBJ53=
OBJ54=
OBJ55=

OBJA= $(OBJ00) $(OBJ01) $(OBJ02) $(OBJ03) $(OBJ04) $(OBJ05) $(OBJ06) $(OBJ07)
OBJB= $(OBJ08) $(OBJ09) $(OBJ10) $(OBJ11) $(OBJ12) $(OBJ13) $(OBJ14) $(OBJ15)
OBJC= $(OBJ16) $(OBJ17) $(OBJ18) $(OBJ19) $(OBJ20) $(OBJ21) $(OBJ22) $(OBJ23)
OBJD= $(OBJ24) $(OBJ25) $(OBJ26) $(OBJ27) $(OBJ28) $(OBJ29) $(OBJ30) $(OBJ31)
OBJE= $(OBJ32) $(OBJ33) $(OBJ34) $(OBJ35) $(OBJ36) $(OBJ37) $(OBJ38) $(OBJ39)
OBJF= $(OBJ40) $(OBJ41) $(OBJ42) $(OBJ43) $(OBJ44) $(OBJ45) $(OBJ46) $(OBJ47)
OBJG= $(OBJ48) $(OBJ49) $(OBJ50) $(OBJ51) $(OBJ52) $(OBJ53) $(OBJ54) $(OBJ55)
OBJH= $(OBJ56) $(OBJ57) $(OBJ58) $(OBJ59) $(OBJ60) $(OBJ61) $(OBJ62) $(OBJ63)

OBJ= $(OBJA) $(OBJB) $(OBJC) $(OBJD) $(OBJE) $(OBJF) $(OBJG) $(OBJH) 

# for regular (no profiling)
OBJS= $(CRTFRONT) $(OBJ) $(CRTC) $(CRTBACK)
# for 'prof(1)'
MOBJS= $(MCRT0) $(OBJ) $(CRTC) $(CRTBACK)
# for 'gprof(1)'
GOBJS= $(GCRT0) $(OBJ) $(CRTC) $(CRTBACK)


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


$(T):			$(T).ee
	cp -p $(T).ee $(T)

$(T).x:			$(OBJ) Makefile
	makedate -m $(T) > makedate.c
	$(CC) -c makedate.c
	$(LD) -o $@ $(LDFLAGS) $(OBJS) $(LIBINFO) > $(T).lm

$(T).prof:		$(OBJ) Makefile
	makedate -m $(T) > makedate.c
	$(CC) -c makedate.c
	$(LD) -o $@ $(LDFLAGS_P) $(MOBJS) $(LIBINFO) > $(T).lm

$(T).gprof:		$(OBJ) Makefile
	makedate -m $(T) > makedate.c
	$(CC) -c makedate.c
	$(LD) -o $@ $(LDFLAGS_G) $(GOBJS) $(LIBINFO) > $(T).lm

$(T).$(OFF) $(OFF):	$(T).x
	cp -p $(T).x $(T).$(OFF)

$(T).nm nm:		$(T).x
	$(NM) $(NMFLAGS) $(T).x > $(T).nm

safe:
	makesafe -v=3 -I $(INCDIR) $(OBJ)

strip:			$(T).x
	strip $(T).x
	rm -f $(T).$(OFF) $(T)

install:		install-raw install-help install-cmdhelp

install-ee:		$(ALL) $(T).$(OFF)
	rm -f n$(T) n$(T).x n$(T).$(OFF)
	makenewer -r $(ALL) $(BINDIR)

install-raw:		$(T).x
	rm -f $(BINDIR)/$(T).$(OFF)
	makenewer -r -o rmsuf $(T).x $(BINDIR)
	-chgrp mail $(BINDIR)/$(T)
#	-chmod g+s $(BINDIR)/$(T)

install-help:		$(T).help
	-mkdir -p $(HELPDIR) 2> /dev/null
	makenewer -o rmsuf $(T).help $(HELPDIR)

install-cmdhelp:	$(T).cmdhelp
	rm -f cmdhelp
	ln -f $(T).cmdhelp cmdhelp
	makenewer cmdhelp $(PLIBDIR)

install-man:
	rm -f $(CATMAN)/cat1/$(T).1
	makenewer -m man=1 $(T).man $(MANDIR)

again:
	-rm -f $(ALL) $(T).x $(T).$(OFF) $(T).prof $(T).gprof

clean:			again
	rm -f *.ls *.ln *.i *.o *.so *.x *.lm core gmon.out

control:
	(uname -n ; date) > Control


libcurses.a:		libcurses.obj
	cp -p libcurses.obj $@

vsnprintf.o:		vsnprintf.obj
	cp -p vsnprintf.obj $@


main.o:			main.c $(INCS)

whatinfo.o:		whatinfo.c config.h

proginfo.o:		proginfo.c defs.h

proginfo_setpiv.o:	proginfo_setpiv.c defs.h

proginfo_rootname.o:	proginfo_rootname.c defs.h

progopts.o:		progopts.c $(INCS)

progconfig.o:		progconfig.c $(INCS)

proglog.o:		proglog.c $(INCS)

progterm.o:		progterm.c $(INCS) uterm.h inter.h

progmailget.o:		progmailget.c $(INCS)

proguserlist.o:		proguserlist.c $(INCS)

initnow.o:		initnow.c $(INCS)

inter.o:		inter.c inter.h $(INCS) uterm.h ds.h display.h
inter.o:		mailbox.h mailmsgfile.h mailmsgviewer.h mbcache.h
inter.o:		kbdinfo.h cmdmap.h subprocs.h

inter_trans.o:		inter_trans.c inter.h ${INCS}

display.o:		display.c display.h $(INCS) ds.h

ds.o:			ds.c ds.h $(INCS) td.h termstr.h

mailmsgfile.o:		mailmsgfile.c mailmsgfile.h

mailmsgviewer.o:	mailmsgviewer.c mailmsgviewer.h

mbdels.o:		mbdels.c mb.h $(INCS)

help.o:			help.c $(INCS)

periph.o:		periph.c $(INCS)


userinfo.o:		userinfo.c userinfo.h

vecstr_loadfile.o:	vecstr_loadfile.c

vecstr_env.o:		vecstr_env.c

vecstr.o:		vecstr.c vecstr.h

keyopt.o:		keyopt.c keyopt.h

dater.o:		dater.c dater.h

dater_getdate.o:	dater_getdate.c dater.h date.h

tmz.o:			tmz.c tmz.h

zos.o:			zos.c zos.h

comparse.o:		comparse.c comparse.h

logfile.o:		logfile.c logfile.h

logfile_userinfo:	logfile_userinfo.c logfile.h userinfo.h

uterm.o:		uterm.c uterm.h

uterm_readcmd.o:	uterm_readcmd.c uterm.h termcmd.h

charq.o:		charq.c charq.h

mailbox.o:		mailbox.c mailbox.h

mailbox_getfrom.o:	mailbox_getfrom.c mailbox.h

mailbox_fromaddr.o:	mailbox_fromaddr.c mailbox.h

mailmsghdrval.o:	mailmsghdrval.c mailmsghdrval.h

mailmsghdrfold.o:	mailmsghdrfold.c mailmsghdrfold.h

mailmsg.o:		mailmsg.c mailmsg.h

mailmsg_loadfile.o:	mailmsg_loadfile.c mailmsg.h

mailmsg_loadmb.o:	mailmsg_loadmb.c mailmsg.h

mailmsg_enver.o:	mailmsg_enver.c mailmsg_enver.h mailmsg.h 

mailmsgmatenv.o:	mailmsgmatenv.c mailmsgmatenv.h

mailmsgmathdr.o:	mailmsgmathdr.c mailmsgmathdr.h

mailmsgfrom.o:		mailmsgfrom.c mailmsgfrom.h

mbcache.o:		mbcache.c mbcache.h mailbox.h mailmsg.h

td.o:			td.c td.h termstr.h

termstr.o:		termstr.c termstr.h

dirlist.o:		dirlist.c dirlist.h

schedvar.o:		schedvar.c schedvar.h

namecache.o:		namecache.c namecache.h

lineindex.o:		lineindex.c lineindex.h

filemap.o:		filemap.c filemap.h

linefold.o:		linefold.c linefold.h

filebuf.o:		filebuf.c filebuf.h 

filebuf_writeblanks.o:	filebuf_writeblanks.c filebuf.h 

filebuf_writefill.o:	filebuf_writefill.c filebuf.h 

filebuf_writefd.o:	filebuf_writefd.c filebuf.h 

filebuf_mailsupport.o:	filebuf_mailsupport.c filebuf.h 

vecobj.o:		vecobj.c vecobj.h

varray.o:		varray.c varray.h

keysymer.o:		keysymer.c keysymer.h

kbdinfo.o:		kbdinfo.c kbdinfo.h

cmdmap.o:		cmdmap.c cmdmap.h

subprocs.o:		subprocs.c subprocs.h

vecint.o:		vecint.c vecint.h

dirseen.o:		dirseen.c dirseen.h

useraccdb.o:		useraccdb.c useraccdb.h

paramopt.o:		paramopt.c paramopt.h

envhelp.o:		envhelp.c envhelp.h

pcsconf.o:		pcsconf.c pcsconf.h

pcspoll.o:		pcspoll.c pcspoll.h

paramfile.o:		paramfile.c paramfile.h

termcmd.o:		termcmd.c termcmd.h

sighand.o:		sighand.c sighand.h

sigign.o:		sigign.c sigign.h

chartrans.o:		chartrans.c chartrans.h uiconv.h

termtrans.o:		termtrans.cc termtrans.h uiconv.h
	$(GPP) -c $(GPPFLAGS) $(CFLAGS) $(CPPFLAGS) $<

hdrdecode.o:		hdrdecode.cc hdrdecode.h uiconv.h
	$(GPP) -c $(GPPFLAGS) $(CFLAGS) $(CPPFLAGS) $<

b64decoder.o:		b64decoder.cc b64decoder.h
	$(GPP) -c $(GPPFLAGS) $(CFLAGS) $(CPPFLAGS) $<

qpdecoder.o:		qpdecoder.cc qpdecoder.h
	$(GPP) -c $(GPPFLAGS) $(CFLAGS) $(CPPFLAGS) $<

hexdecoder.o:		hexdecoder.cc hexdecoder.h
	$(GPP) -c $(GPPFLAGS) $(CFLAGS) $(CPPFLAGS) $<

utf8decoder.o:		utf8decoder.cc utf8decoder.h
	$(GPP) -c $(GPPFLAGS) $(CFLAGS) $(CPPFLAGS) $<

obuf.o:			obuf.cc obuf.h
	$(GPP) -c $(GPPFLAGS) $(CFLAGS) $(CPPFLAGS) $<

mapstrint.o:		mapstrint.c mapstrint.h

ema.o:			ema.c ema.h

lookaside.o:		lookaside.c lookaside.h

vechand.o:		vechand.c vechand.h

uiconv.o:		uiconv.c uiconv.h

date.o:			date.c date.h

strmgr.o:		strmgr.c strmgr.h

sesmsg.o:		sesmsg.c sesmsg.h

msgdata.o:		msgdata.c msgdata.h

kshlib.o:		kshlib.c kshlib.h sesmsg.h msgdata.h

progsig.o:		progsig.c progsig.h sesmsg.h msgdata.h


upt.o:			upt.c upt.h

spawnproc.o:		spawnproc.c spawnproc.h

format.o:		format.c format.h

progspawncmd.o:		progspawncmd.c spawnproc.h

mkbestaddr.o:		mkbestaddr.c ema.h


# testing

T01= testchartrans

T01OBJa= $(T01).o chartrans.o uiconv.o termtrans.o
T01OBJb= 
T01OBJc=
T01OBJd= format.o debugprintf.o debug.o

T01OBJ= $(T01OBJa) $(T01OBJb) $(T01OBJc) $(T01OBJd)

$(T01).x:		Makefile $(T01OBJ)
	$(GPP) -o $@ $(T01OBJ) $(TLIBDIRS) $(TLIBS)


T02= testsntmtime

T02OBJ= $(T02).o sntmtime.o

$(T02).x:		Makefile $(T02OBJ)
	$(GPP) -o $@ $(T02OBJ) $(TLIBDIRS) $(TLIBS)


T03= testintmax

T03OBJ= $(T03).o

$(T03).x:		Makefile $(T03OBJ)
	$(GPP) -o $@ $(T03OBJ) $(TLIBDIRS) $(TLIBS)


T04= testqpdecode

T04OBJ= $(T04).o qpdecoder.o obuf.o

$(T04).x:		Makefile $(T04OBJ)
	$(GPP) -o $@ $(T04OBJ) $(TLIBDIRS) $(TLIBS)


T05= testhdrdecode

T05OBJa= $(T05).o hdrdecode.o
T05OBJb= qpdecoder.o b64decoder.o uiconv.o chartrans.o
T05OBJc= utf8decoder.o
T05OBJd= obuf.o
T05OBJe= snwcpywidehdr.o
T05OBJf= straltwchar.o
T05OBJg=
T05OBJh=

T05OBJ0= $(T05OBJa) $(T05OBJb) $(T05OBJc) $(T05OBJd)
T05OBJ1= $(T05OBJe) $(T05OBJf) $(T05OBJg) $(T05OBJh)

T05OBJ= $(T05OBJ0) $(T05OBJ1)

$(T05).x:		Makefile $(T05OBJ)
	$(GPP) -o $@ $(T05OBJ) $(TLIBINFO)


T06= testbins

T06OBJa= $(T06).o
T06OBJb=
T06OBJc=
T06OBJd=
T06OBJe=
T06OBJf=
T06OBJg=
T06OBJh=

T06OBJ0= $(T06OBJa) $(T06OBJb) $(T06OBJc) $(T06OBJd)
T06OBJ1= $(T06OBJe) $(T06OBJf) $(T06OBJg) $(T06OBJh)

T06OBJ= $(T06OBJ0) $(T06OBJ1)

$(T06).o:		$(T06).cc
	$(GPP) -c $(GPPFLAGS) $(CFLAGS) $(CPPFLAGS) $(T06).cc

$(T06).x:		Makefile $(T06OBJ)
	$(GPP) -o $@ $(T06OBJ) $(TLIBINFO)


