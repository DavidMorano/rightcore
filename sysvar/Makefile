# MAKEFILE

T= sysvar
SVST= sysvars
MKST= varmks
VIST= vars

ALL= $(T) $(SVST).so $(MKST).so $(VIST).so

SRCROOT= $(LOCAL)


BINDIR= $(SRCROOT)/bin
INCDIR= $(SRCROOT)/include
LIBDIR= $(SRCROOT)/lib
MANDIR= $(SRCROOT)/man/man1
HELPDIR= $(SRCROOT)/share/help
EXTRALIB= $(EXTRA)/lib

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

CCOPTS= $(CCOPTS_GCCALL) -O3 $(CCOPTS_GCCOPTSPARC)
#CCOPTS= $(CCOPTS_GCCALL) -g -O
#CCOPTS= $(CCOPTS_GCCALL) -g -pg
#CCOPTS= $(CCOPTS_GCCALL) -g -Wstrict-aliasing
#CCOPTS= $(CCOPTS_GCCALL) -g -Wall

CCOPTS_GCCLIB= -fpic $(CCOPTS_GCCLIBSPARC)

#CCOPTS= $(CCOPTS_CCALL) -g -xs

# HyperSPARC
# regular
#CCOPTS= -xO5 -xtarget=ss20/hs22 -xdepend
# profiling 'prof(1)'
#CCOPTS= -p -xO5 -xtarget=ss20/hs22 -xdepend
# profiling 'gprof(1)' 
#CCOPTS= -xpg -xO5 -xtarget=ss20/hs22 -xdepend

# UltraSPARC
# regular
#CCOPTS= -xO5 -xtarget=ultra -xsafe=mem -xdepend
# profiling 'prof(1)'
#CCOPTS= -p -xO5 -xtarget=ultra -xsafe=mem -xdepend
# profiling 'gprof(1)' 
#CCOPTS= -xpg -xO5 -xtarget=ultra -xsafe=mem -xdepend


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

LDRPATH= $(EXTRALIB)

# regular
LDFLAGS= $(LF_LDFLAGS) -R$(LIBDIR):$(LDRPATH)
# for profiling
LDFLAGS_P= $(LF_LDFLAGS) -L/usr/lib/libp # -R$(LIBDIR)


LIBDIRS= -L$(LIBDIR)

LIB0=
LIB1= -ldam -lb
LIB2=
LIB3= -luc -lu
LIB4= -L$(GNU)/lib -lstdc++ -lgcc_eh -lgcc
LIB5= 
LIB6= -lsecdb -lproject -lpthread -lrt -lxnet -lsocket -lnsl
LIB7= -ldl -lc

LIBS= $(LIB0) $(LIB1) $(LIB2) $(LIB3) $(LIB4) $(LIB5) $(LIB6) $(LIB7)


SLIBDIRS= -L$(LIBDIR)

SLIB0=
SLIB1= -ldam -lb
SLIB2= 
SLIB3= -luc -lu
SLIB4= -L$(GNU)/lib -lstdc++ -lgcc_eh -lgcc
SLIB5= 
#SLIB6= -lsecdb -lproject -lpthread -lrt -lxnet -lsocket -lnsl
#SLIB7= -ldl -lc

# Note:
# we can't really cheat here, there are too many symbols we need
# that are not already in any parent!
SLIBS= $(SLIB0) $(SLIB1) $(SLIB2) $(SLIB3) $(SLIB4) $(SLIB5)


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

# for regular (no profiling)
CRT0= $(CRT1) $(CRTI) $(VALUES)
# for 'prof(1)'
MCRT0= $(CRTI) $(MCRT1) $(VALUES)
# for 'gprof(1)'
GCRT0= $(CRTI) $(GCRT1) $(VALUES)

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


INCS= b_$(T).h defs.h


OBJ00= main.o whatinfo.o proginfo.o proginfo_setpiv.o 
OBJ01= b_$(T).o
OBJ02= sysvarproc.o
OBJ03= 
OBJ04= 
OBJ05= 
OBJ06=
OBJ07= printhelp.o shio.o kshlib.o maininfo.o sesmsg.o msgdata.o
OBJ08=
OBJ09= dirseen.o modload.o
OBJ10= varmk.o var.o
OBJ11= 
OBJ12=
OBJ13=
OBJ14= 
OBJ15= 

#OBJ16= varmks.o varhdr.o 
#OBJ17= vars.o hashagain.o
OBJ18=
OBJ19= 
OBJ20=
OBJ21=
OBJ22=
OBJ23=

OBJA= $(OBJ00) $(OBJ01) $(OBJ02) $(OBJ03) $(OBJ04) $(OBJ05) $(OBJ06) $(OBJ07)
OBJB= $(OBJ08) $(OBJ09) $(OBJ10) $(OBJ11) $(OBJ12) $(OBJ13) $(OBJ14) $(OBJ15)
OBJC= $(OBJ16) $(OBJ17) $(OBJ18) $(OBJ19) $(OBJ20) $(OBJ21) $(OBJ22) $(OBJ23)

OBJ= $(OBJA) $(OBJB) $(OBJC)

# for regular (no profiling)
OBJS= $(CRTFRONT) $(OBJ) $(CRTC) $(CRTBACK)
# for 'prof(1)'
MOBJS= $(MCRT0) $(OBJ) $(CRTC)
# for 'gprof(1)'
GOBJS= $(GCRT0) $(OBJ) $(CRTC)


VISOBJ= vars.o varhdr.o hashagain.o hashelf.o randlc.o mkfnamesuf.o

MKSOBJ= varmks.o varhdr.o 

SVSOBJ0= sysvars.o var.o varmk.o dirseen.o 
SVSOBJ1= sysvarproc.o spawnproc.o
SVSOBJ= $(SVSOBJ0) $(SVSOBJ1)


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
	$(LD) -o $@ $(LDFLAGS) $(OBJS) $(LIBDIRS) $(LIBS) > $(T).lm

$(T).prof:		$(OBJ) Makefile
	makedate -m $(T) > makedate.c
	$(CC) -c makedate.c
	$(LD) -o $@ $(LDFLAGS_P) $(MOBJS) $(LIBDIRS) $(LIBS) > $(T).lm

$(T).gprof:		$(OBJ) Makefile
	makedate -m $(T) > makedate.c
	$(CC) -c makedate.c
	$(LD) -o $@ $(LDFLAGS_P) $(GOBJS) $(LIBDIRS) $(LIBS) > $(T).lm

$(T).$(OFF) $(OFF):	$(T).x
	cp -p $(T).x $(T).$(OFF)

$(T).nm nm:		$(T).x
	$(NM) $(NMFLAGS) $(T).x > $(T).nm

$(T).cxref:
	$(CXREF) -c $(CXREFFLAGS) $(SRC) > $(T).cxref

so:			$(SVST).so $(MKST).so $(VIST).so 

$(SVST).so:		$(SVSOBJ) Makefile $(SVST).map
	$(LD) -G -o $@ -M $(SVST).map $(LDFLAGS) \
		$(SVSOBJ) $(SLIBDIRS) $(SLIBS)

$(MKST).so:		$(MKSOBJ) Makefile $(MKST).map
	$(LD) -G -o $@ -M $(MKST).map $(LDFLAGS) \
		$(MKSOBJ) $(SLIBDIRS) $(SLIBS)

$(VIST).so:		$(VISOBJ) Makefile $(VIST).map
	$(LD) -G -o $@ -M $(VIST).map $(LDFLAGS) \
		$(VISOBJ) $(SLIBDIRS) $(SLIBS)

safe:
	makesafe -v=3 -I $(INCDIR) $(OBJ) $(VISOBJ) $(MKSOBJ) $(SVSOBJ)

strip:			$(T).x
	strip $(T).x
	rm -f $(T).$(OFF) $(T)

install:		install-raw install-help

install-ee:		$(ALL)
	makenewer -r $(ALL) $(BINDIR)

install-so:		$(SVST).so $(MKST).so $(VIST).so 
	makenewer -r $(SVST).so $(MKST).so $(VIST).so $(LIBDIR)

install-raw:		$(T).x
	rm -f $(BINDIR)/$(T).$(OFF)
	makenewer -r -o rmsuf $(T).x $(BINDIR)

install-help:		$(T).help
	-mkdir -p $(HELPDIR) 2> /dev/null
	makenewer -o rmsuf $(T).help $(HELPDIR)

again:
	rm -f $(ALL) $(T).x $(T).lm
	rm -f $(SVST).so $(MKST).so $(VIST).so 

clean:			again
	rm -f *ls *.ln *.i *.o *.so *.x *.lm

control:
	uname -n > Control
	date >> Control


main.o:			main.c $(INCS) maininfo.h

whatinfo.o:		whatinfo.c config.h

proginfo.o:		proginfo.c defs.h

proginfo_setpiv.o:	proginfo_setpiv.c defs.h

b_$(T).o:		b_$(T).c $(INCS) varmk.h var.h

maininfo.o:		maininfo.c maininfo.h kshlib.h

kshlib.o:		kshlib.c kshlib.h sesmsg.h msgdata.h

sesmsg.o:		sesmsg.c sesmsg.h

msgdata.o:		msgdata.c msgdata.h


getutmpent.o:		getutmpent.c getutmpent.h


shio.o:			shio.c shio.h

userinfo.o:		userinfo.c userinfo.h

vecstr_loadfile.o:	vecstr_loadfile.c

vecstr_env.o:		vecstr_env.c

vecstr_adduniq.o:	vecstr_adduniq.c

keyopt.o:		keyopt.c keyopt.h

filemap.o:		filemap.c filemap.h

filebuf.o:		filebuf.c filebuf.h

realname.o:		realname.c realname.h

ipasswd.o:		ipasswd.c ipasswd.h

lastlogfile.o:		lastlogfile.c lastlogfile.h

tmpx.o:			tmpx.c tmpx.h

biblebook.o:		biblebook.c biblebook.h

bibleq.o:		bibleq.c bibleq.h bibleqs.h

bibleqs.o:		bibleqs.c bibleqs.h txtindex.h txtindexmk.h

txtindex.o:		txtindex.c txtindex.h txtindexes.h

txtindexmk.o:		txtindexmk.c txtindexmk.h txtindexmks.h

txtindexes.o:		txtindexes.c txtindexes.h txtindexhdr.h

txtindexmks.o:		txtindexmks.c txtindexmks.h txtindexhdr.h

txtindexhdr.o:		txtindexhdr.c txtindexhdr.h

offindex.o:		offindex.c offindex.h

wordfill.o:		wordfill.c wordfill.h

strstore.o:		strstore.c strstore.h

xwords.o:		xwords.c xwords.h

eigendb.o:		eigendb.c eigendb.h

vecint.o:		vecint.c vecint.h

veclong.o:		veclong.c veclong.h

vechand.o:		vechand.c vechand.h

vecobj.o:		vecobj.c vecobj.h

sysvar.o:		sysvar.c sysvar.h sysvars.h


varmk.o:		varmk.c varmk.h varmks.h dirseen.h
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<

varmks.o:		varmks.c varmks.h varhdr.h
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<

sysvars.o:		sysvars.c sysvars.h var.h dirseen.h
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<

var.o:			var.c var.h vars.h dirseen.h
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<

vars.o:			vars.c vars.h varhdr.h
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<

varhdr.o:		varhdr.c varhdr.h
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<

modload.o:		modload.c modload.h
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<

dirseen.o:		dirseen.c dirseen.h
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<

spawnproc.o:		spawnproc.c spawnproc.h
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<


sysvarproc.o:		sysvarproc.c
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<

hashagain.o:		hashagain.c
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<

hashelf.o:		hashelf.c
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<

randlc.o:		randlc.c
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<

mkfnamesuf.o:		mkfnamesuf.c
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<


