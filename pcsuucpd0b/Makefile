# MAKEFILE

T= pcsuucpd
 
ALL= $(T)

SRCROOT= $(PCS)


BINDIR= $(SRCROOT)/bin
INCDIR= $(SRCROOT)/include
LIBDIR= $(SRCROOT)/lib
MANDIR= $(SRCROOT)/man/man1
HELPDIR= $(SRCROOT)/share/help

LDCRTDIR= $(CGS_CRTDIR)
VALDIR= $(CGS_BALDIR)


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
#CCOPTS= -xO4 -xtarget=ss20/hs22 -dalign -xdepend

# UltraSPARC
#CCOPTS= -xO4 -xtarget=ultra -xsafe=mem -dalign -xdepend


DEF0=
DEF1=
DEF2=
DEF3=
DEF4=
DEF5=
DEF6=
DEF7= $(LF_DEFS)

DEFS= $(DEF0) $(DEF1) $(DEF2) $(DEF3) $(DEF4) $(DEF5) $(DEF6) $(DEF7)


INCDIRS= -I$(INCDIR) -I$(NCMP)/include

CPPFLAGS= $(DEFS) $(INCDIRS)

CFLAGS= $(CCOPTS)

#LD= $(CC)
#LD= cc
LD= ld


LDRPATH=

LDFLAGS= -m $(LF_LDFLAGS) -R$(LIBDIR):$(CGS_RPATH)


LIBDIRS= -L$(LIBDIR) -L$(NCMP)/lib -L$(CGS_LIBDIR)

LIB0=
LIB1= -ldam -lb
LIB2=
LIB3= -luc -lu
LIB4= ${CGS_LIBS)
LIB5= 
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
NMFLAGS= -xs -v

CPP= cpp


INCS= config.h defs.h


OBJ00= main.o whatinfo.o proginfo.o proginfo_setpiv.o
OBJ01= progsrvargs.o progjobdir.o
OBJ02= configfile.o checklockfile.o
OBJ03= varsub_extras.o expander.o
OBJ04= watch.o mktmpdir.o checkdirs.o badback.o headkey.o
OBJ05= handle.o handle_srventry.o field_svcargs.o processargs.o execute.o
OBJ06= srventry.o inputlogin.o passwdok.o
OBJ07= printhelp.o
OBJ08=
OBJ09= varsub_dumpfd.o util.o
OBJ10= getcodedtimed.o kinfo.o
OBJ11= getfname.o findfilepath.o
OBJ12= serialbuf.o listentcp.o
OBJ13= jobdb.o builtin.o standing.o procse.o muximsg.o sysmisc.o
OBJ14= pwfile.o hostent.o
OBJ15= procfileenv.o procfilepaths.o strfieldcmp.o
OBJ16= bopensched.o bopenroot.o connection.o
OBJ17= permsched.o schedvar.o
OBJ18= srvtab.o acctab.o field_srvarg.o
OBJ19=
OBJ20= starmat.o chmods.o mkplogid.o mksublogid.o
OBJ21= snddd.o snsds.o
OBJ22=
OBJ23= msfile.o msfilee.o

OBJA= $(OBJ00) $(OBJ01) $(OBJ02) $(OBJ03) $(OBJ04) $(OBJ05) $(OBJ06) $(OBJ07) 
OBJB= $(OBJ08) $(OBJ09) $(OBJ10) $(OBJ11) $(OBJ12) $(OBJ13) $(OBJ14) $(OBJ15)
OBJC= $(OBJ16) $(OBJ17) $(OBJ18) $(OBJ19) $(OBJ20) $(OBJ21) $(OBJ22) $(OBJ23)

OBJ= $(OBJA) $(OBJB) $(OBJC)

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
	makedate -m $(T) > makedate.c
	$(CC) -c makedate.c
	$(LD) -o $@ $(LDFLAGS) $(OBJS) $(LIBINFO) > $(T).lm

$(T).$(OFF) $(OFF):	$(T).x
	cp -p $(T).x $(T).$(OFF)

$(T).nm:		$(T).x
	nm -x $(T).x > $(T).nm

safe:
	makesafe -v=3 -I $(INCDIR):$(NCMP)/include $(OBJ)

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

again:
	rm -fr $(ALL) $(T) $(T).x

clean:			again
	rm -f *.o $(ALL)

control:
	(uname -n ; date) > Control


main.o:			main.c $(INCS) srvtab.h acctab.h pwfile.h

whatinfo.o:		whatinfo.c config.h

proginfo.o:		proginfo.c $(INCS)

expander.o:		expander.c $(INCS)

builtin.o:		builtin.c builtin.h $(INCS) muximsg.h sysmisc.h

standing.o:		standing.c standing.h $(INCS) muximsg.h sysmisc.h
standing.o:		msfile.h msfilee.h kinfo.h

watch.o:		watch.c $(INCS) muximsg.h

handle.o:		handle.c $(INCS) srvtab.h pwfile.h

handle_srventry.o:	handle_srventry.c $(INCS) srvtab.h acctab.h

badback.o:		badback.c $(INCS)

timestr_nist.o:		timestr_nist.c nistinfo.h


configfile.o:		configfile.c configfile.h

jobdb.o:		jobdb.c jobdb.h

muximsg.o:		muximsg.c muximsg.h

sysmisc.o:		sysmisc.c sysmisc.h


vecstr_loadfile.o:	vecstr_loadfile.c

vecstr_env.o:		vecstr_env.c

srvtab.o:		srvtab.c srvtab.h

acctab.o:		acctab.c acctab.h

sockaddress.o:		sockaddress.c sockaddress.h

pwfile.o:		pwfile.c pwfile.h

kinfo.o:		kinfo.c kinfo.h

msfile.o:		msfile.c msfile.h msfilee.h msflag.h

msfilee.o:		msfilee.c msfilee.h

hostent.o:		hostent.c hostent.h

openportmsg.o:		openportmsg.c openportmsg.h


