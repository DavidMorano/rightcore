# MAKEFILE

T= bbnews

ALL= $(T)

#PCS= /usr/add-on/pcs
SRCROOT= $(PCS)


BINDIR= $(SRCROOT)/bin
INCDIR= $(SRCROOT)/include
LIBDIR= $(SRCROOT)/lib
MANDIR= $(SRCROOT)/man/man1
HELPDIR= $(SRCROOT)/share/help

LDCRTDIR= $(CGS_CRTDIR)
VALDIR= $(CGS_VALDIR)


# NOTES:
#
# To 'make' for 'prof(1)' profiling, execute 'make bb.prof'.
# To 'make' for 'gprof(1)' profiling, execute 'make bb.prof'.
#
#

CC= $(CGS_CC)
GPP= $(CGS_GPP)

GPPFLAGS= $(CGS_CCFLAGS)
CCOPTS_GCCALL= -pthreads -fexceptions
CCOPTS_CCALL= -MT

CCOPTS_GCCOPTSGPP= $(CGS_CCFLAGS)
CCOPTS_GCCOPTSPARC= -mcpu=ultrasparc
CCOPTS_GCCLIBSPARC= -mno-app-regs

CCOPTS= $(CCOPTS_GCCALL) -O3 $(CCOPTS_GCCOPTSPARC)
#CCOPTS= $(CCOPTS_GCCALL) -g -O
#CCOPTS= $(CCOPTS_GCCALL) -g -pg
#CCOPTS= $(CCOPTS_GCCALL) -g -Wstrict-aliasing
#CCOPTS= $(CCOPTS_GCCALL) -g -Wall

CCOPTS_GCCLIB= -fpic $(CCOPTS_GCCLIBSPARC)

#CCOPTS= $(CCOPTS_CCALL) -g -xs

# HyperSPARC
# regular
#CCOPTS= -xO5 -xtarget=ss20/hs22 -dalign -xdepend
# profiling 'prof(1)'
#CCOPTS= -p -xO5 -xtarget=ss20/hs22 -dalign -xdepend
# profiling 'gprof(1)' 
#CCOPTS= -xpg -xO5 -xtarget=ss20/hs22 -dalign -xdepend

# UltraSPARC
# regular
#CCOPTS= -xO5 -xtarget=ultra -xsafe=mem -dalign -xdepend
# profiling 'prof(1)'
#CCOPTS= -p -xO5 -xtarget=ultra -xsafe=mem -dalign -xdepend
# profiling 'gprof(1)' 
#CCOPTS= -xpg -xO5 -xtarget=ultra -xsafe=mem -dalign -xdepend


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


LDRPATH= $(EXTRA)/lib:/usr/lib/lwp

# regular
LDFLAGS= -m $(LF_LDFLAGS) -R$(LIBDIR):$(CGS_RPATH):$(LDRPATH)
# for profiling
#LDFLAGS= $(LF_LDFLAGS) -L/usr/lib/libp # -R$(LIBDIR)


LIBDIRS= -L$(LIBDIR) -L$(CGS_LIBDIR)

LIB0= -lpcs
LIB1= -ldam -lb
LIB2=
LIB3= -luc -lu
LIB4= $(CGS_LIBS)
LIB5= 
LIB6= -lsecdb -lproject -lpthread -lthread -lrt -lxnet -lsocket -lnsl
LIB7= -ldl -lc

LIBS= $(LIB0) $(LIB1) $(LIB2) $(LIB3) $(LIB4) $(LIB5) $(LIB6) $(LIB7)

LIBINFO= $(LIBDIRS) $(LIBS)


XLIBDIRS= -L$(LIBDIR) -L$(CGS_LIBDIR)

XLIB0=
XLIB1= -ldam -lb
XLIB2= -ldebug
XLIB3= -luc -lu
XLIB4= $(CGS_LIBS)
XLIB5= 
XLIB6= -lsecdb -lproject -lpthread -lthread -lrt -lxnet -lsocket -lnsl
XLIB7= -ldl

XLIBS= $(XLIB0) $(XLIB1) $(XLIB2) $(XLIB3) $(XLIB4) $(XLIB5) $(XLIB6) $(XLIB7)

XLIBINFO= $(LIBDIRS) $(LIBS)


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

# for regular (no profiling)
CRT0= $(CRTI) $(CRT1) $(VALUES)
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


INCS= config.h defs.h mkdirlist.h dirshown.h artlist.h


OBJ00= main.o whatinfo.o proginfo.o proginfo_setpiv.o
OBJ01= proguserlist.o proglogid.o proglog.o
OBJ02= progconf.o prognewsrc.o progufname.o
OBJ03= progmode.o progng.o progmsgenv.o proghdr.o
OBJ04= mkdirlist.o dirshown.o
OBJ05= cmd_save.o cmd_output.o cmd_printout.o cmd_follow.o
OBJ06= emit_count.o emit_header.o emit_article.o emit_mailbox.o emit_test.o
OBJ07= printhelp.o 

OBJ08=
OBJ09= monthname.o getfield.o mm_getfield.o getbbopts.o hdrextid.o
OBJ10= sfcenter.o
OBJ11= bbcmp.o bbcpy.o
OBJ12= bbnewsrc.o artlist.o outbuf.o
OBJ13= 
OBJ14= matthingenv.o hmatch.o
OBJ15= dater_getbbtime.o dater.o

OBJ16= gettermlines.o
OBJ17= mailmsg_envtimes.o
OBJ18= sfbracketval.o
OBJ19=
OBJ20= pcsconf.o pcspoll.o
OBJ21=
OBJ22= openpcsdircache.o
OBJ23=

OBJ24= mailmsgfile.o mailmsgviewer.o
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

OBJ= $(OBJA) $(OBJB) $(OBJC)

# for regular (no profiling)
OBJS= $(CRTFRONT) $(OBJ) $(CRTC) $(CRTBACK)
# for 'prof(1)'
MOBJS= $(MCRT0) $(OBJ) $(CRTC)
# for 'gprof(1)'
GOBJS= $(GCRT0) $(OBJ) $(CRTC)


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
	rm -f $(T)
	cp -p $(T).ee $(T)

$(T).x:			$(OBJ) Makefile
	makedate -m $(T) > makedate.c
	$(CC) -c makedate.c
	$(LD) -o $@ $(LDFLAGS) $(OBJS) $(LIBINFO) > $(T).lm

$(T).prof:		$(OBJ) Makefile
	makedate -m $(T) > makedate.c
	$(CC) -c makedate.c
	$(LD) -o $@ $(LDFLAGS) $(MOBJS) $(LIBINFO)

$(T).gprof:		$(OBJ) Makefile
	makedate -m $(T) > makedate.c
	$(CC) -c makedate.c
	$(LD) -o $@ $(LDFLAGS) $(GOBJS) $(LIBINFO)

$(T).$(OFF) $(OFF):	$(T).x
	rm -f $(T).${OFF)
	cp -p $(T).x $(T).$(OFF)

$(T).nm nm:		$(T).x
	$(NM) $(NMFLAGS) $(T).x > $(T).nm

safe:
	makesafe -v=3 -I $(INCDIR) $(OBJ)

strip:			$(T).x
	strip $(T).x
	rm -f $(T).$(OFF) $(T)

install:		install-raw install-help

install-raw:		$(T).x
	rm -f $(BINDIR)/$(T).$(OFF)
	makenewer -r -o rmsuf $(T).x $(BINDIR)

install-help:		$(T).help
	-mkdir -p $(HELPDIR) 2> /dev/null
	makenewer -o rmsuf $(T).help $(HELPDIR)

bbpost:			bbfile.o
	ln -f bbpost.sh bbpost

bbfile:			bbfile.o
	$(LD) -o $@ $(OBJ) $(LIBDIRS) $(LIBS)

again:
	rm -fr $(ALL) $(T).x

clean:			again
	makeclean

control:
	uname -n > Control
	date >> Control

bb.cflow:
	cflow $(INCDIRS) *.c > $@


bbfile.o:
	$(CC) -c $(CFLAGS) bbfile.c


main.o:			main.c $(INCS)

whatinfo.o:		whatinfo.c config.h

proginfo.o:		proginfo.c defs.h

proginfo_setpiv.o:	proginfo_setpiv.c defs.h

proguserlist.o:		proguserlist.c defs.h

proglog.o:		proglog.c defs.h

proglogid.o:		proglogid.c defs.h

progufname.o:		progufname.c defs.h

progconfig.o:		progconfig.c $(INCS)

prognewsrc.o:		prognewsrc.c $(INCS) mkdirlist.h bbnewsrc.h

progmode.o:		progmode.c $(INCS) mkdirlist.h bbnewsrc.h

proghdr.o:		proghdr.c $(INCS) proghdr.h

progng.o:		progng.c $(INCS) mkdirlist.h bbnewsrc.h

progmsgenv.o:		progmsgenv.c $(INCS)

proglinecheck.o:	proglinecheck.c $(INCS)

gettermlines.o:		gettermlines.c $(INCS)

getbbopts.o:		getbbopts.c $(INCS)

update.o:		update.c $(INCS) bbnewsrc.h

mkdirlist.o:		mkdirlist.c $(INCS)

dirshown.o:		dirshown.c dirshown.h $(INCS) mkdirlist.h

bbnewsrc.o:		bbnewsrc.c bbnewsrc.h

artlist.o:		artlist.c artlist.h

cfbbdate.o:		cfbbdate.c $(INCS)

cmd_follow.o:		cmd_follow.c $(INCS)

cmd_output.o:		cmd_output.c $(INCS)

cmd_printout.o:		cmd_printout.c $(INCS)

cmd_reply.o:		cmd_reply.c $(INCS)

cmd_save.o:		cmd_save.c $(INCS)

emit_item.o:		emit_item.c $(INCS) headerkeys.h

emit_count.o:		emit_count.c $(INCS)

emit_header.o:		emit_header.c $(INCS)

emit_article.o:		emit_article.c $(INCS)

emit_mailbox.o:		emit_mailbox.c $(INCS)

getfield.o:		getfield.c $(INCS)

mm_getfield.o:		mm_getfield.c $(INCS)

user_match.o:		user_match.c $(INCS)

dater_getbbdate.o:	dater_getbbtime.o dater.h


pcsuserfile.o:		pcsuserfile.c pcsconf.h

openpcsdircache.o:	openpcsdircache.c openpcsdircache.h


vecstr_loadfile.o:	vecstr_loadfile.c

vecstr_env.o:		vecstr_env.c

mailmsg_envtimes.o:	mailmsg_envtimes.c mailmsg.h

dater.o:		dater.c dater.h

mailmsgfile.o:		mailmsgfile.c mailmsgfile.h

mailmsgviewer.o:	mailmsgviewer.c mailmsgviewer.h

uterm.o:		uterm.c uterm.h

pcsconf.o:		pcsconf.c pcsconf.h

pcspoll.o:		pcspoll.c pcspoll.h pcsconf.h


# testing

X01= testspawner

X01OBJ01= $(X01).o spawner.o envhelp.o
X01OBJ02= debugprintf.o strlinelen.o

X01OBJ= $(X01OBJ01) $(X01OBJ02)

X01OBJS= $(CRTFRONT) $(X01OBJ) $(CRTC) $(CRTBACK)

$(X01)-safe:
	makesafe -v=3 -I $(INCDIR) $(X01OBJ)

$(X01).x:		Makefile $(X01OBJ)
	$(CC) -o $@ $(X01OBJ) $(XLIBDIRS) $(XLIBS)

ts.x:			$(X01).x
	ln -f $(X01).x $@


X02= testopendircache

X02OBJ00= $(X02).o
X02OBJ01= openpcsdircache.o
X02OBJ02= debugprintf.o strlinelen.o

X02OBJ= $(X02OBJ00) $(X02OBJ01) $(X02OBJ02)

X02OBJS= $(CRTFRONT) $(X02OBJ) $(CRTC) $(CRTBACK)

$(X02)-safe:
	makesafe -v=3 -I $(INCDIR) $(X02OBJ)

$(X02).x:		Makefile $(X02OBJ)
	$(CC) -o $@ $(X02OBJ) $(XLIBDIRS) $(XLIBS)


