# MAKEFILE (libb)

T= testbfile

ALL= $(T).x

SRCROOT= $(HOME)


INCDIR= $(SRCROOT)/include
LIBDIR= $(SRCROOT)/lib
BINDIR= $(SRCROOT)/bin
MANDIR= $(SRCROOT)/man/man1
HELPDIR= $(SRCROOT)/share/help


CC= gcc
GPP= g++

CCOPTS_GCCALL= -pthreads -fexceptions
CCOPTS_CCALL= -MT

CCOPTS_GCCOPTSPARC= -mcpu=ultrasparc
CCOPTS_GCCLIBSPARC= -mno-app-regs

CCOPTS= $(CCOPTS_GCCALL) -O $(CCOPTS_GCCOPTSPARC)
#CCOPTS= $(CCOPTS_GCCALL) -g -O
#CCOPTS= $(CCOPTS_GCCALL) -g -pg
CCOPTS= $(CCOPTS_GCCALL) -g -Wstrict-aliasing

CCOPTS_GCCLIB= -fpic $(CCOPTS_GCCLIBSPARC)

#CCOPTS= $(CCOPTS_CCALL) -g -xs


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

CFLAGS= $(CCOPTS) $(LF_CFLAGS)

#LD= $(CC)

LDFLAGS= $(LF_LDFLAGS)


LIBDIRS= -L$(SRCROOT)/lib

LIB0=
LIB1= -ldam -lb
LIB2=
LIB3= -luc -lu
LIB4= -L$(GNU)/lib -lstdc++ -lgcc_eh -lgcc
LIB5= 
LIB6= -lsecdb -lproject -lpthread -lrt -lxnet -lsocket -lnsl
LIB7= -ldl -lc

LIBS= $(LIB0) $(LIB1) $(LIB2) $(LIB3) $(LIB4) $(LIB5) $(LIB6) $(LIB7)


XLIBDIRS= -L$(SRCROOT)/lib

XLIB0=
XLIB1= -ldam -lb
XLIB2= -ldebug
XLIB3= -luc -lu
XLIB4=
XLIB5= 
XLIB6= -lsecdb -lproject -lpthread -lrt -lxnet -lsocket -lnsl
XLIB7= -ldl

XLIBS= $(XLIB0) $(XLIB1) $(XLIB2) $(XLIB3) $(XLIB4) $(XLIB5) $(XLIB6) $(XLIB7)


RANLIB= ranlib

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


INCS= bfile.h


OBJ00= bopen.o bopentmp.o bopencmd.o bopenrcmd.o bopenlock.o 
OBJ01= bopenmod.o bdup.o bminmod.o
OBJ02= bread.o breadline.o breadlines.o bgetc.o 
OBJ03= bwrite.o bwriteblock.o bputc.o bprintf.o bwriteblanks.o bprintline.o
OBJ04= bcopyblock.o
OBJ05= bflush.o breserve.o
OBJ06= bcontrol.o btruncate.o brewind.o bisterm.o
OBJ07= bseek.o btell.o
OBJ08= bopensched.o bopenshell.o bopenrcmde.o
OBJ09= 
OBJ10= bfile_flush.o bfile_pagein.o bfile.o
OBJ11= mkfdfname.o 
OBJ12=
#OBJ13= debugprintf.o nprintf.o fdprintf.o strlinelen.o
OBJ14=
OBJ15= getnfile.o findfilepath.o
OBJ16= strbasename.o strwset.o
OBJ17= getpwd.o permsched.o lockfile.o
OBJ18= ctdec.o cthex.o 
OBJ19= vstrkeycmp.o strkeycmp.o
OBJ20= mkdirs.o mktmpfile.o 
OBJ21=
OBJ22= bufprintf.o format.o hasprintbad.o isprintbad.o
#OBJ23= econvert.o

OBJ24=
OBJ25=
OBJ26=
OBJ27=
OBJ28=
OBJ29=
OBJ30=
OBJ31=

OBJ32=
OBJ33=
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

OBJS= obja.o objb.o objc.o objd.o obje.o


.SUFFIXES:		.ls .i .cx .cs


default:		all

all:			$(ALL) $(T).a(bprintf.o)
 
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


$(T).a:			$(OBJ)
	$(AR) -cr $@ $?
	ranlib $@

$(T).so:		$(OBJS) bfile.map Makefile $(T).a
	$(LD) -o $@ -G -B reduce -M bfile.map \
		$(LDFLAGS) $(OBJS) $(LIBDIRS) $(LIBS)

$(T).nm:		$(T).so
	$(NM) $(NMFLAGS) $(T).so > $(T).nm

$(T).order order:	$(OBJ) $(T).a
	$(LORDER) $(T).a | $(TSORT) > $(T).order
	$(RM) $(T).a
	while read O ; do $(AR) -cr $(T).a $${O} ; done < $(T).order

install.pre:
	filefind . -s h | makenewer -af - -d $(INCDIR)

install:		$(T).a
	makenewer $(T).a $(LIBDIR)
	makenewer $(INCS) $(INCDIR)

install.$(OFF):		install

safe:
	makesafe -v=3 -I $(INCDIR) $(OBJ)

clean:			again
	rm -f $(T).a *.o

again:
	rm -f $(T).a

control:
	uname -n > Control
	date >> Control


obja.o:			$(OBJA)
	$(LD) -o $@ -r $(LDFLAGS) $(OBJA)

objb.o:			$(OBJB)
	$(LD) -o $@ -r $(LDFLAGS) $(OBJB)

objc.o:			$(OBJC)
	$(LD) -o $@ -r $(LDFLAGS) $(OBJC)

objd.o:			$(OBJD)
	$(LD) -o $@ -r $(LDFLAGS) $(OBJD)

obje.o:			$(OBJE)
	$(LD) -o $@ -r $(LDFLAGS) $(OBJE)


$(T).a(bprintf.o):	bprintf.o

$(T).a(nprintf.o):	nprintf.o

$(T).a(debugprintf.o):	debugprintf.o


# special (very special) processing for NPRINTF
nprintf_r.o:		nprintf.map Makefile nprintf.o lockfile.o format.o 
	$(LD) -r -o $@ -B reduce -M nprintf.map nprintf.o lockfile.o format.o

nprintf.o:		Makefile nprintf.c


# special (very special) processing for DEBPRINTF
debugprintf_r.o:	debugprintf.map Makefile debugprintf.o format.o 
	$(LD) -r -o $@ -B reduce -M debugprintf.map debugprintf.o format.o

debugprintf.o:		Makefile debugprintf.c


econvert.o:	econvert.obj
	rm -f econvert.o
	cp -p econvert.obj econvert.o


ids.o:			ids.c ids.h
storebuf.o:		storebuf.c storebuf.h
dupstr.o:		dupstr.c dupstr.h
snflags.o:		snflags.c snflags.h


bcontrol.o:		bcontrol.c bfile.h
bisterm.o:		bisterm.c bfile.h
bcopy.o:		bcopy.c bfile.h
bcopyblock.o:		bcopyblock.c bfile.h
bcopyfile.o:		bcopyfile.c bfile.h
bflush.o:		bflush.c bfile.h
breserve.o:		breserve.c bfile.h
bgetc.o:		bgetc.c bfile.h
bopen.o:		bopen.c bfile.h
bopencmd.o:		bopencmd.c bfile.h
bopenlock.o:		bopenlock.c bfile.h
bopenrcmd.o:		bopenrcmd.c bfile.h
bopenremote.o:		bopenremote.c bfile.h
bopensched.o:		bopensched.c bfile.h
bopentmp.o:		bopentmp.c bfile.h
bprintf.o:		bprintf.c bfile.h
bprintline.o:		bprintline.c bfile.h
bputc.o:		bputc.c bfile.h
bread.o:		bread.c bfile.h
breadline.o:		breadline.cc bfile.h
	$(GPP) -c $(GPPFLAGS) $(CFLAGS) $(CPPFLAGS) breadline.cc
breadline.o:		breadline.cc bfile.h
breadlines.o:		breadlines.c bfile.h
bseek.o:		bseek.c bfile.h
btell.o:		btell.c bfile.h
btruncate.o:		btruncate.c bfile.h
bwasteline.o:		bwasteline.c bfile.h
bwrite.o:		bwrite.c bfile.h
bwriteblock.o:		bwriteblock.c bfile.h
bwriteblanks.o:		bwriteblanks.c bfile.h
bprintcleanline.o:	bprintcleanline.c bfile.h
bprintcleanlines.o:	bprintcleanlines.c bfile.h
brewind.o:		brewind.c bfile.h
 
btell64.o:		btell64.c bfile.h
bseek64.o:		bseek64.c bfile.h

bfile.o:		bfile.c bfile.h
bdup.o:			bdup.c bfile.h

bfile_flush.o:		bfile_flush.c bfile.h
bfile_pagein.o:		bfile_pagein.c bfile.h
bfile_fnames.o:		bfile_fnames.c bfile.h

mkfdfname.o:		mkfdfname.c bfile.h


format.o:		format.c

sfbasename.o:		sfbasename.c

sfdirname.o:		sfdirname.c

strbasename.o:		strbasename.c

ctdec.o:		ctdec.c


# testing

X01= testbfile

#X01OBJ00= $(X01).o
X01OBJ00= main.o
X01OBJ01= bopen.o bread.o breadline.o bwrite.o bflush.o
X01OBJ02= bfile.o bfile_flush.o
X01OBJ03= debugprintf.o debug.o

X01OBJ= $(X01OBJ00) $(X01OBJ01) $(X01OBJ02) $(X01OBJ03)

$(X01).o:		$(X01).c bfile.h

$(X01).x:		$(X01OBJ)
	$(CC) -o $@ $(X01OBJ) $(XLIBDIRS) $(XLIBS)

main.o:			main.c bfile.h



