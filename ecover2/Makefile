# MAKEFILE

T= ecover

ALL= $(T)

HELPFILE= $(T).help

SRCROOT= $(LOCAL)


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

CCOPTS_GCCOPTSPARC= -mcpu=ultrasparc
CCOPTS_GCCLIBSPARC= -mno-app-regs

CCOPTS_GCCOPTSGPP= $(CGS_CCFLAGS)
CCOPTS= $(CCOPTS_GCCALL) -O2 $(CCOPTS_GCCOPTSPARC)
#CCOPTS= $(CCOPTS_GCCALL) -g -O
#CCOPTS= $(CCOPTS_GCCALL) -g -pg
#CCOPTS= $(CCOPTS_GCCALL) -g -Wstrict-aliasing
#CCOPTS= $(CCOPTS_GCCALL) -g -Wall

CCOPTS_GCCLIB= -fpic $(CCOPTS_GCCLIBSPARC)

#CCOPTS= $(CCOPTS_CCALL) -g -xs

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


LDRPATH=

LDFLAGS= -m $(LF_LDFLAGS) -R$(LIBDIR):$(CGS_RPATH)


LIBDIRS= -L$(LIBDIR) -L$(CGS_LIBDIR)

LIB0=
LIB1= -ldam -lb
LIB2=
LIB3= -luc -lu
LIB4= $(CGS_LIBS)
LIB5= 
LIB6= -lsecdb -lproject -lpthread -lrt -lxnet -lsocket -lnsl
LIB7= -ldl -lc

LIBS= $(LIB0) $(LIB1) $(LIB2) $(LIB3) $(LIB4) $(LIB5) $(LIB6) $(LIB7)

LIBINFO= $(LIBDIRS) $(LIBS)


TLIBDIRS= -L$(LIBDIR) -L$(CGS_LIBDIR)

TLIB0=
TLIB1= -ldam -lb
TLIB2= -ldebug
TLIB3= -luc -lu
TLIB4= $(CGS_LIBS)
TLIB5=
TLIB6= -lsecdb -lproject -lpthread -lthread -lrt -lxnet -lsocket -lnsl
TLIB7= -ldl

TLIBS= $(TLIB0) $(TLIB1) $(TLIB2) $(TLIB3) $(TLIB4) $(TLIB5) $(TLIB6) $(TLIB7)

TLIBINFO= $(TLIBDIRS) $(TLIBS)


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

CXREF= cxref
CXREFFLAGS= -R -s

CPP= cpp

LORDER= lorder
TSORT= tsort

RM= rm -f


INCS= config.h defs.h


OBJ00= main.o whatinfo.o proginfo.o proginfo_setpiv.o 
OBJ01= proglog.o
OBJ02= lightnoise.o encode.o decode.o munge.o
OBJ03= 
OBJ04= 
OBJ04= 
OBJ05=
OBJ06=
OBJ07= printhelp.o 
OBJ08= randomvar.o
OBJ09= cheaprand.o
OBJ10=
OBJ11=
OBJ12= 
OBJ13= ecinfo.o
OBJ14= ecmsg.o ecmsg_loadfile.o
OBJ15=

OBJA= $(OBJ00) $(OBJ01) $(OBJ02) $(OBJ03) $(OBJ04) $(OBJ05) $(OBJ06) $(OBJ07)
OBJB= $(OBJ08) $(OBJ09) $(OBJ10) $(OBJ11) $(OBJ12) $(OBJ13) $(OBJ14) $(OBJ15)

OBJ= $(OBJA) $(OBJB)

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
	cp -p $(T).ee $(T)

$(T).x:			$(OBJ) Makefile
	makedate -m $(T) > makedate.c
	$(CC) -c $(CFLAGS) makedate.c
	$(LD) -o $@ $(LDFLAGS) $(OBJS) $(LIBINFO) > $(T).lm

$(T).$(OFF) $(OFF):	$(T).x
	cp -p $(T).x $(T).$(OFF)

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

install-ee:		$(ALL) install-help
	makenewer -r $(ALL) $(BINDIR)

install-raw:		$(T).x
	rm -f $(BINDIR)/$(T).$(OFF)
	makenewer -r -o rmsuf $(T).x $(BINDIR)

install-help:		$(T).help
	-mkdir -p $(HELPDIR) 2> /dev/null
	makenewer -o rmsuf $(T).help $(HELPDIR)

again:
	rm -f $(ALL) $(T).x

clean:			again
	makeclean

control:
	uname -n > Control
	date >> Control


main.o:			main.c $(INCS)

whatinfo.o:		whatinfo.c config.h

proginfo.o:		proginfo.c $(INCS)

proginfo_setpiv.o:	proginfo_setpiv.c $(INCS)

proglog.o:		proglog.c proglog.h $(INCS)

lightnoise.o:		lightnoise.c $(INCS)

encode.o:		encode.c $(INCS)

decode.o:		decode.c $(INCS)

munge.o:		munge.c $(INCS)

ecinfo.o:		ecinfo.c ecinfo.h


ecmsg.o:		ecmsg.c ecmsg.h

ecmsg_loadfile.o:	ecmsg_loadfile.c ecmsg.h

vecstr_loadfile.o:	vecstr_loadfile.c

vecstr_env.o:		vecstr_env.c

randomvar.o:		randomvar.c randomvar.h

sha1.o:			sha1.c sha1.h

cksum.o:		cksum.c cksum.h


# testing

T01= testaccess

T01OBJa= $(T01).o
T01OBJb= 
T01OBJc=
T01OBJd= u_access.o

T01OBJ= $(T01OBJa) $(T01OBJb) $(T01OBJc) $(T01OBJd)

$(T01).x:		Makefile $(T01OBJ)
	$(GPP) -o $@ $(T01OBJ) $(TLIBDIRS) $(TLIBS)


