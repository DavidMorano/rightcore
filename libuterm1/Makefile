# MAKEFILE

T= libuterm

ALL= $(T).a

SRCROOT= $(LOCAL)


BINDIR= $(SRCROOT)/bin
INCDIR= $(SRCROOT)/include
LIBDIR= $(SRCROOT)/lib
MANDIR= $(SRCROOT)/man/man1

LDCRTDIR= $(CGS_CRTDIR)
VALDIR= $(CGS_CALDIR)


CC= $(CGS_CC)
GPP= $(CGS_GPP)


GPPFLAGS= $(CGS_CCFLAGS)

CCOPTS_GCCOPTSGPP= $(CGS_CCFLAGS)
CCOPTS= $(CCOPTS_GCCALL) $(CCOPTS_GCCOPT) $(CCOPTS_GCCLIB)
#CCOPTS= $(CCOPTS_GCCALL) $(CCOPTS_GCCLIB)
#CCOPTS= $(CCOPTS_GCCALL) $(CCOPTS_GCCLIB) -g -Wstrict-aliasing -Wall

# HyperSPARC
#CCOPTS= -Kpic -xO5 -xtarget=ss20/hs22 -dalign -xdepend

# UltraSPARC
#CCOPTS= -Kpic -xO5 -xtarget=ultra -xsafe=mem -dalign -xdepend


DEF0=
DEF1=
DEF2=
DEF3=
DEF4=
DEF5=
DEF6=
DEF7= $(LF_DEFS) 

DEFS= $(DEF0) $(DEF1) $(DEF2) $(DEF3) $(DEF4) $(DEF5)


INCDIRS= -I$(INCDIR)

CPPFLAGS= $(DEFS) $(INCDIRS)

CFLAGS= $(CCOPTS) 

#LD= $(CC)
LD= cc
LD= ld


LDRPATH=

LDFLAGS= -m -R$(LIBDIR):$(LOCAL)/lib:$(CGS_RPATH)


LIBDIRS= -L$(LIBDIR) -L$(LOCAL)/lib -L$(CGS_LIBDIR)

LIB0= -luterm
LIB1= -ldam -lb
LIB2=
LIB3= -luc -lu
LIB4= $(CGS_LIBS)
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
NMFLAGS= -vs -x

CPP= cpp


INCS= uterm.h termcmd.h


LOBJ00= uterm.o uterm_readcmd.o
LOBJ01= 
LOBJ02=
LOBJ03= charq.o termcmd.o
LOBJ04=
LOBJ05=
LOBJ06=
LOBJ07=

LOBJA= $(LOBJ00) $(LOBJ01) $(LOBJ02) $(LOBJ03) 
LOBJB= $(LOBJ04) $(LOBJ05) $(LOBJ06) $(LOBJ07)

LOBJ= $(LOBJA) $(LOBJB)

LOBJS= $(LOBJ)


TOBJ00= testuterm.o whatinfo.o proginfo.o proginfo_setpiv.o
TOBJ01= 
TOBJ02= debug.o mkcleanline.o
TOBJ03= 
TOBJ04= 
TOBJ05=
TOBJ06=
TOBJ07= printhelp.o
TOBJ08=
TOBJ09=
TOBJ10= uterm.o uterm_readcmd.o
TOBJ11= charq.o termcmd.o
TOBJ12= 
TOBJ13=
TOBJ14=
TOBJ15=

TOBJA= $(TOBJ00) $(TOBJ01) $(TOBJ02) $(TOBJ03) 
TOBJB= $(TOBJ04) $(TOBJ05) $(TOBJ06) $(TOBJ07)
TOBJC= $(TOBJ08) $(TOBJ09) $(TOBJ10) $(TOBJ11) 
TOBJD= $(TOBJ12) $(TOBJ13) $(TOBJ14) $(TOBJ15) 

TOBJ= $(TOBJA) $(TOBJB) $(TOBJC) $(TOBJD)

TOBJS= $(CRTFRONT) $(TOBJ) $(CRTC) $(CRTBACK)


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


a:			$(T).a
so:			$(T).so

$(T).a:			$(LOBJ)
	$(AR) -rc $(T).a $?

$(T).so:		$(LOBJS) Makefile localmisc.h $(T).a
	$(LD) -o $@ -G $(LDFLAGS) $(LOBJS) > $(T).lm

safe:
	makesafe -v=3 -I $(INCDIR) $(LOBJ) $(TOBJ)

again:
	$(RM) $(ALL)

clean:			again
	makeclean

install:		$(ALL) Makefile install-incs
	bsdinstall $(ALL) $(LIBDIR)

install-incs:		$(INCS)
	makenewer $(INCS) -d $(INCDIR) -t $@

testuterm.x:	$(TOBJ) Makefile
	makedate -m $(T) > makedate.c
	$(CC) -c $(CFLAGS) makedate.c
	$(LD) -o $@ $(LDFLAGS) $(TOBJS) $(LIBDIRS) $(LIBS) > tesuterm.lm


testuterm.o:		testuterm.c config.h defs.h uterm.h

whatinfo.o:		whatinfo.c

proginfo.o:		defs.h

proginfo_setpiv.o:	defs.h


uterm.o:		uterm.c uterm.h ucb.h charq.h

uterm_readcmd.o:	uterm_readcmd.c uterm.h termcmd.h

charq.o:		charq.c charq.h

aiq.o:			aiq.c aiq.h

plainq.o:		plainq.c plainq.h

q.o:			q.c q.h

fdt.o:			fdt.c fdt.h

ucb.o:			ucb.c ucb.h

termcmd.o:		termcmd.c termcmd.h


utty.o:			utty.c ucb.h localmisc.h vsystem.h
	$(CC) -c $(CFLAGS) $*.c
	$(AR) -rc $(T).a $@

vs.o:			vs.c ucb.h


