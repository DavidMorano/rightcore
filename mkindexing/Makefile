# MAKEFILE

T= mkindexing

TIST= txtindexes

MKST= txtindexmks

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


INCDIRS= -I$(INCDIR)

CPPFLAGS= $(DEFS) $(INCDIRS)

CFLAGS= $(CCOPTS)

#LD= $(CC)
#LD= cc
LD= ld


LDRPATH= $(EXTRA)/lib

LDFLAGS= $(LF_LDFLAGS) -R$(LIBDIR):$(CGS_RPATH)

LDFLAGS_SO= $(LF_LDFLAGS) $(LDFLAGS) -R$(LDRPATH):$(CGS_RPATH)


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


SLIBDIRS= -L$(LIBDIR) -L$(CGS_LIBDIR)

SLIB0=
SLIB1= -ldam -lb
SLIB2=
SLIB3= -luc -lu
SLIB4= $(CGS_LIBS)
SLIB5=
SLIB6= -lsecdb -lproject -lpthread -lrt -lxnet -lsocket -lnsl
SLIB7= -ldl

SLIBA= $(SLIB0) $(SLIB1) $(SLIB2) $(SLIB3) 
SLIBB= $(SLIB4) $(SLIB5) $(SLIB6) $(SLIB7)

SLIBS= $(SLIBA) $(SLIBB)

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
NMFLAGS= -xs

CXREF= cxref
CXREFFLAGS= -R -s

CPP= cpp

LORDER= lorder
TSORT= tsort

RM= rm -f


INCS= config.h defs.h mkcmds.h


OBJ00= main.o whatinfo.o proginfo.o proginfo_setpiv.o 
OBJ01= proglog.o proguserlist.o progids.o progeigen.o
OBJ02= progkey.o proginv.o progquery.o progtagprint.o 
OBJ03= progkeyer.o progtagprinter.o
OBJ04= mkcmds.o mktagfname.o sfill.o
OBJ05= 
OBJ06= keys.o taginfo.o outfmt.o biblecite.o
OBJ07= printhelp.o 
OBJ08=
OBJ09=
OBJ10= eigendb.o rtags.o fsi.o biblebook.o
OBJ11= textlook.o searchkeys.o xwords.o
OBJ12= txtindexmk.o txtindex.o
OBJ13=
OBJ14= makedate_get.o
OBJ15=

# optional to include shared-objects directly
#OBJ16= txtindexhdr.o offindex.o
#OBJ17= strtabfind.o 
#OBJ18= 
#OBJ19= txtindexmks.o txtindexes.o
#OBJ20=
#OBJ21=
#OBJ22=
#OBJ23=

OBJA= $(OBJ00) $(OBJ01) $(OBJ02) $(OBJ03) $(OBJ04) $(OBJ05) $(OBJ06) $(OBJ07)
OBJB= $(OBJ08) $(OBJ09) $(OBJ10) $(OBJ11) $(OBJ12) $(OBJ13) $(OBJ14) $(OBJ15)
OBJC= $(OBJ16) $(OBJ17) $(OBJ18) $(OBJ19) $(OBJ20) $(OBJ21) $(OBJ22) $(OBJ23)

OBJ= $(OBJA) $(OBJB) $(OBJC)

OBJS= $(CRTFRONT) $(OBJ) $(CRTC) $(CRTBACK)

SOBJ= txtindexes.o txtindexhdr.o offindex.o strtabfind.o

TISOBJ= txtindexes.o txtindexhdr.o offindex.o strtabfind.o

MKSOBJ= txtindexmks.o txtindexhdr.o strtabfind.o osetint.o


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

.c.cx:
	$(CXREF) -C $(CXREFFLAGS) $(CPPFLAGS) $<

.c.cs:
	$(CXREF) $(CXREFFLAGS) $(CPPFLAGS) -o $(*).cs $<


$(T):			$(T).ee
	rm -f $(T)
	cp -p $(T).ee $(T)

so:			$(TIST).so $(MKST).so 

$(T).x:			$(OBJ) Makefile
	rm -f $(T).x
	makedate $(T) > makedate.c
	$(CC) -c $(CFLAGS) makedate.c
	$(LD) -o $@ -m $(LDFLAGS) $(OBJS) $(LIBINFO) > $(T).lm

$(T).$(OFF) $(OFF):	$(T).x
	cp -p $(T).x $(T).$(OFF)

$(T).nm nm:		$(T).x
	$(NM) $(NMFLAGS) $(T).x > $(T).nm

$(T).cxref:
	$(CXREF) -c $(CXREFFLAGS) $(SRC) > $(T).cxref

$(TIST).so:		$(TISOBJ) Makefile $(TIST).map
	$(LD) -G -o $@ -M $(TIST).map $(LDFLAGS_SO) \
		$(TISOBJ) $(SLIBINFO) > $(TIST).lm

$(MKST).so:		$(MKSOBJ) Makefile $(MKST).map
	$(LD) -G -o $@ -M $(MKST).map $(LDFLAGS_SO) \
		$(MKSOBJ) $(SLIBINFO) > $(MKST).lm

safe:
	makesafe -v=3 -I $(INCDIR) $(OBJ) $(SOBJ) $(TISOBJ) $(MKSOBJ)

strip:			$(T).x
	strip $(T).x
	rm -f $(T).$(OFF) $(T)

install:		install-raw install-help

install-ee:		$(ALL)
	makenewer -r $(ALL) $(BINDIR)

install-raw:		$(T).x
	rm -f $(T)
	ln $(T).x $(T)
	makeinstall-all $(BINDIR)

install-so:		$(TIST).so $(MKST).so 
	makenewer -r $(TIST).so $(MKST).so $(LIBDIR)

install-help:
	-mkdir -p $(HELPDIR) 2> /dev/null
	makeinstall-help $(HELPDIR)

install-man:
	makeinstall-man ${MANDIR}

again:
	rm -f $(ALL) $(T).x $(T).lm $(TIST).so $(MKST).so 

clean:			again
	rm -f *.ls *.ln *.i *.o *.so *.x *.lm 

control:
	uname -n > Control
	date >> Control


main.o:			main.c $(INCS)

whatinfo.o:		whatinfo.c config.h

proginfo.o:		proginfo.c defs.h

proginfo_setpiv.o:	proginfo_setpiv.c defs.h

proglog.o:		proglog.c proglog.h $(INCS)

proguserlist.o:		proguserlist.c proguserlist.h $(INCS)

progids.o:		progids.c progids.h $(INCS)

progeigen.o:		progeigen.c $(INCS)

mkkey.o:		mkkey.c $(INCS)

mkinv.o:		mkinv.c $(INCS)

mkquery.o:		mkquery.c $(INCS)

mktagprint.o:		mktagprint.c $(INCS) outfmt.h

mkaudit.o:		mkaudit.c $(INCS)

progkey.o:		progkey.c $(INCS) xwords.h

progtagprint.o:		progtagprint.c $(INCS) outfmt.h sfill.h

progkeyer.o:		progkey.c $(INCS) keys.h xwords.h

progtagprinter.o:	progtagprinter.c $(INCS) outfmt.h taginfo.h sfill.h

keys.o:			keys.c keys.h $(INCS)

mkhashfile.o:		mkhashfile.c $(INCS)

auditdb.o:		auditdb.c $(INCS)


upt.o:			upt.c upt.h

outfmt.o:		outfmt.c outfmt.h

biblecite.o:		biblecite.c biblecite.h

mkcmds.o:		mkcmds.c mkcmds.h

sfill.o:		sfill.c sfill.h


strtabfind.o:		strtabfind.c
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<


vecstr_loadfile.o:	vecstr_loadfile.c

vecstr_env.o:		vecstr_env.c

eigendb.o:		eigendb.c eigendb.h

filemap.o:		filemap.c filemap.h

vecint.o:		vecint.c vecint.h

xwords.o:		xwords.c xwords.h

rtags.o:		rtags.c rtags.h

taginfo.o:		taginfo.c taginfo.h

txtindexmk.o:		txtindexmk.c txtindexmk.h txtindexmks.h

txtindex.o:		txtindex.c txtindex.h txtindexes.h

txtindeximkwait.o:	txtindexmkwait.c txtindexhdr.h

txtindexmks.o:		txtindexmks.c txtindexmks.h txtindexhdr.h
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<

txtindexes.o:		txtindexes.c txtindexes.h txtindexhdr.h
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<

txtindexhdr.o:		txtindexhdr.c txtindexhdr.h
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<

offindex.o:		offindex.c offindex.h
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<

wordfill.o:		wordfill.c wordfill.h

strstore.o:		strstore.c strstore.h

fsi.o:			fsi.c fsi.h

dirseen.o:		dirseen.c dirseen.h

textlook.o:		textlook.c textlook.h

searchkeys.o:		searchkeys.c searchkeys.h

ciq.o:			ciq.c ciq.h

modload.o:		modload.c modload.h

vecint.o:		vecint.c vecint.h

setint.o:		setint.cc setint.h
	$(GPP) -c $(GPPFLAGS) $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<

osetint.o:		osetint.cc osetint.h
	$(GPP) -c $(GPPFLAGS) $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<


