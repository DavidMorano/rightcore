# MAKEFILE

T= pcsconf

TPC= pcsconfs

TPG= pollprog

ALL= $(T).x $(TPC).so

HELPFILE= $(T).help

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

CCOPTS= $(CCOPTS_GCCALL) -O3 $(CCOPTS_GCCOPTSPARC)
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


INCDIRS= -I$(INCDIR) -I$(LOCAL)/include

CPPFLAGS= $(DEFS) $(INCDIRS)

CFLAGS= $(CCOPTS)

#LD= $(CC)
#LD= cc
LD= ld


LDRPATH= $(EXTRA)/lib:/usr/lib/lwp

LDFLAGS= -m $(LF_LDFLAGS) -R$(LIBDIR):$(LOCAL)/lib:$(CGS_RPATH):$(LDRPATH)

LDFLAGS_SO= -m $(LF_LDFLAGS) -R$(LIBDIR):$(CGS_RPATH)


LIBDIRS= -L$(LIBDIR) -L$(LOCAL)/lib -L$(CGS_LIBDIR)

LIB0=
LIB1= -lpcs -ldam -lb
LIB2=
LIB3= -luc -lu
LIB4= -liconv $(CGS_LIBS)
LIB5= 
LIB6= -lsecdb -lproject -lpthread -lthread -lrt -lxnet -lsocket -lnsl
LIB7= -ldl -lc

LIBS= $(LIB0) $(LIB1) $(LIB2) $(LIB3) $(LIB4) $(LIB5) $(LIB6) $(LIB7)

LIBINFO= $(LIBDIRS) $(LIBS)


SLIBDIRS= -L$(LIBDIR) -L$(LOCAL)/lib -L$(CGS_LIBDIR)

SLIB0=
SLIB1= -lpcs -ldam -lb
SLIB2=
SLIB3= -luc -lu
SLIB4= $(CGS_LIBS)
SLIB5= 
SLIB6= -lsecdb -lproject -lpthread -lthread -lrt -lxnet -lsocket -lnsl
SLIB7=

SLIBS= $(SLIB0) $(SLIB1) $(SLIB2) $(SLIB3) $(SLIB4) $(SLIB5) $(SLIB6) $(SLIB7)

SLIBINFO= $(SLIBDIRS) $(SLIBS)


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
OBJ01= proglog.o proguserlist.o
OBJ02= b_$(T).o
OBJ03= 
OBJ04=
OBJ05= havenis.o checksecure.o
OBJ06=
OBJ07= printhelp.o shio.o kshlib.o maininfo.o sesmsg.o msgdata.o
OBJ08=
OBJ09= userinfo.o ids.o
OBJ10=
OBJ11= schedvar.o authfile.o 
OBJ12= pcsconf.o pcspoll.o thrcomm.o thrbase.o
OBJ13= 
OBJ14= nchr.o sfcookkey.o hasfext.o
#OBJ15= debugprintf.o debug.o nprintf.o strlinelen.o

OBJ16= getlogname.o
OBJ17=
OBJ18= localgetorg.o
OBJ19= pcsgetfacility.o
OBJ20=
OBJ21= 
OBJ22=
OBJ23=

#OBJ24= pcsconfs.o
#OBJ25= var.o varmk.o 
#OBJ26= varmks.o varhdr.o
#OBJ27=
OBJ28=
#OBJ29= onckeyalready.o onckeygetset.o checkonc.o
OBJ30=
OBJ31=

OBJA= $(OBJ00) $(OBJ01) $(OBJ02) $(OBJ03) $(OBJ04) $(OBJ05) $(OBJ06) $(OBJ07)
OBJB= $(OBJ08) $(OBJ09) $(OBJ10) $(OBJ11) $(OBJ12) $(OBJ13) $(OBJ14) $(OBJ15)
OBJC= $(OBJ16) $(OBJ17) $(OBJ18) $(OBJ19) $(OBJ20) $(OBJ21) $(OBJ22) $(OBJ23)
OBJD= $(OBJ24) $(OBJ25) $(OBJ26) $(OBJ27) $(OBJ28) $(OBJ29) $(OBJ30) $(OBJ31)

OBJ= $(OBJA) $(OBJB) $(OBJC) $(OBJD)

OBJS= $(CRTFRONT) $(OBJ) $(CRTC) $(CRTBACK)


PCOBJ00= pcsconfs.o
PCOBJ01= var.o varmk.o
PCOBJ02= varmks.o varhdr.o
PCOBJ03= prmktmpdir.o
PCOBJ04=
PCOBJ05= 
PCOBJ06= 
PCOBJ07= 

PCOBJa= $(PCOBJ00) $(PCOBJ01) $(PCOBJ02) $(PCOBJ03)
PCOBJb= $(PCOBJ04) $(PCOBJ05) $(PCOBJ06) $(PCOBJ07) 

PCOBJ= $(PCOBJa) $(PCOBJb)


TPGOBJ01= pollproger.o pollprogcheck.o
TPGOBJ02= thrbase.o thrcomm.o
TPGOBJ03=
TPGOBJ04=

TPGOBJ= $(TPGOBJ01) $(TPGOBJ02) $(TPGOBJ03) $(TPGOBJ04)


SRC= $(OBJ:.c=.o)


.SUFFIXES:		.ls .i .cx .cs


default:		$(T).x

all:			$(ALL)

so:			$(TPC).so


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
	makecp $(T).ee $(T)

$(TPC).so:		$(PCOBJ) Makefile
	$(LD) -G -o $@ $(LDFLAGS_SO) $(PCOBJ) $(SLIBINFO) > $(TPC).lm

$(TPG).so:		$(TPGOBJ) Makefile
	$(LD) -G -o $@ $(LDFLAGS_SO) $(TPGOBJ) $(SLIBINFO) > $(TPG).lm

$(T).x:			$(OBJ) Makefile
	makedate -m $(T) > makedate.c
	$(CC) -c $(CFLAGS) makedate.c
	$(LD) -o $@ $(LDFLAGS) $(OBJS) $(LIBINFO) > $(T).lm

$(T).$(OFF) $(OFF):	$(T).x
	makecp $(T).x $(T).$(OFF)

$(T).nm nm:		$(T).x
	$(NM) $(NMFLAGS) $(T).x > $(T).nm

$(T).cxref:
	$(CXREF) -c $(CXREFFLAGS) $(SRC) > $(T).cxref

safe:
	makesafe -v=3 -I $(INCDIR) $(OBJ)

strip:			$(T).x
	strip $(T).x
	rm -f $(T).$(OFF) $(T)

install:		install-raw install-help install-so

install-so:		$(TPC).so
	makenewer -r $(TPC).so $(LIBDIR)

install-ee:		$(T)
	makenewer -r $(T) $(BINDIR)

install-raw:		$(T).x
	rm -f $(BINDIR)/$(T).$(OFF)
	makenewer -r -o rmsuf $(T).x $(BINDIR)
	-chmod g+s $(BINDIR)/$(T)

install-help:		$(T).help
	-mkdir -p $(HELPDIR) 2> /dev/null
	makenewer -o rmsuf $(T).help $(HELPDIR)

again:
	rm -f $(ALL) $(T).x

clean:
	makeclean

control:
	uname -n > Control
	date >> Control


main.o:			main.c $(INCS) maininfo.h

whatinfo.o:		whatinfo.c config.h

proginfo.o:		proginfo.c defs.h

proginfo_setpiv.o:	proginfo_setpiv.c defs.h

proglog.o:		proglog.c defs.h

proguserlist.o:		proguserlist.c defs.h

proglogid.o:		proglogid.c defs.h

b_$(T).o:		b_$(T).c $(INCS)

maininfo.o:		maininfo.c maininfo.h kshlib.h

kshlib.o:		kshlib.c kshlib.h sesmsg.h msgdata.h

sesmsg.o:		sesmsg.c sesmsg.h

msgdata.o:		msgdata.c msgdata.h


pcsnames.o:		pcsnames.c

sysvarproc.o:		sysvarproc.c


shio.o:			shio.c shio.h

userinfo.o:		userinfo.c userinfo.h

vecstr_loadfile.o:	vecstr_loadfile.c

vecstr_env.o:		vecstr_env.c

filemap.o:		filemap.c filemap.h

realname.o:		realname.c realname.h

ipasswd.o:		ipasswd.c ipasswd.h

lastlogfile.o:		lastlogfile.c lastlogfile.h

scedvar.o:		schedvar.c schedvar.h

pcsconf.o:		pcsconf.c pcsconf.h

ids.o:			ids.c ids.h


pcsconfs.o:		pcsconfs.c pcsconfs.h
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<

var.o:			var.c var.h
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<

varmk.o:		varmk.c varmk.h
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<

varmks.o:		varmks.c varmks.h
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<

varhdr.o:		varhdr.c varhdr.h
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<

thrcomm.o:		thrcomm.c thrcomm.h
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<

thrbase.o:		thrbase.c thrbase.h
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<


prmktmpdir.o:		prmktmpdir.c
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<

pollproger.o:		pollproger.c
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<

pollprogcheck.o:	pollprogcheck.c
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<


