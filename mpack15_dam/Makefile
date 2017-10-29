# MAKEFILE (mpack)

T= mpack

ALL= $(T)

SRCROOT= $(PCS)


BINDIR= $(SRCROOT)/bin
INCDIR= $(SRCROOT)/include
LIBDIR= $(SRCROOT)/lib
MANDIR= $(SRCROOT)/man/man1
HELPDIR= $(SRCROOT)/share/help

#LDCRTDIR= /opt/SUNWspro/WS6/lib
#LDCRTDIR= /opt/SUNWspro/SC4.0/lib
#LDCRTDIR= /opt/SUNWspro/lib


CC= gcc
GPP= g++

CCOPTS_GCCALL= -pthreads
CCOPTS_CCALL= -MT

CCOPTS_GCCOPTSPARC= -mcpu=v9
CCOPTS_GCCLIBSPARC= -mno-app-regs -fno-builtin-bcopy

CCOPTS= $(CCOPTS_GCCALL) -O $(CCOPTS_GCCOPTSPARC)
#CCOPTS= $(CCOPTS_GCCALL) -g -O
#CCOPTS= $(CCOPTS_GCCALL) -g -pg
#CCOPTS= $(CCOPTS_GCCALL) -g -Wstrict-aliasing

CCOPTS_GCCLIB= -fpic $(CCOPTS_GCCLIBSPARC)

#CCOPTS= $(CCOPTS_CCALL) -g -xs

# HyperSPARC
#CCOPTS= -xO5 -xtarget=ss20/hs22 -dalign -xdepend

# UltraSPARC
#CCOPTS= -xO5 -xtarget=ultra -xsafe=mem -dalign -xdepend

DEF0= -DOSNAME_$(OSNAME)=$(OSNUM)
DEF1=
DEF2=
DEF3=
DEF4=
DEF5=
DEF6=
DEF7= $(LF_DEFS)

DEFS= $(DEF0) $(DEF1) $(DEF2) $(DEF3) $(DEF4) $(DEF5) $(DEF6) $(DEF7)

INCDIRS= -I$(INCDIR)

CFLAGS= $(CCOPTS) $(DEFS) $(INCDIRS)

#LD= $(CC)
#LD= cc
LD= ld

LDFLAGS= -m ${LF_LDFLAGS) -R$(LIBDIR)

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

CRTC= makedate.o
CRT= $(CRT1) $(CRTI) $(CRTN)

LINT= lint
LINTFLAGS= -uxn $(DEFS) $(INCDIRS)


OBJ0= main.o whatinfo.o buildmsg.o
OBJ1= 
OBJ2=
OBJ3=
OBJ4=

OBJ= $(OBJ0) $(OBJ1) $(OBJ2) $(OBJ3) $(OBJ4)

OBJS= $(CRT0) $(OBJ) $(CRTC) 


default:		$(T).x

all:			$(ALL)

$(T):			$(T).sh
	cp -p $(T).sh $(T)

$(T).x:			$(OBJ) Makefile
	makedate > makedate.c
	$(CC) -c makedate.c
	$(LD) -o $@ $(LDFLAGS) $(OBJS) $(LIBDIRS) $(LIBS) $(CRTN) > $(T).lm

$(T).$(OFF) $(OFF):	strip
	cp -p $(T).x $(T).$(OFF)

strip:			$(T).x
	strip $(T).x
	rm -f $(T) $(T).$(OFF)

install:		$(ALL)
	bsdinstall $(ALL) $(SRCROOT)/bin

again:
	rm -f $(T).x $(ALL)

clean:			again
	rm -f *.o $(ALL)

readme:
	./mkreadme.pl unix >README.unix
	./mkreadme.pl dos >README.dos
	./mkreadme.pl os2 >README.os2
	./mkreadme.pl mac >README.mac
	./mkreadme.pl amiga >README.amiga
	./mkreadme.pl arc archimedes >README.arc

l_pack:
	#load unixpk.c encode.c codes.c magic.c unixos.c string.c \
		xmalloc.c md5c.c getopt.c

l_unpack:
	#load unixunpk.c decode.c uudecode.c codes.c unixos.c string.c \
		xmalloc.c md5c.c getopt.c

depend:
	grep '^#[ 	]*include[ 	]*"' *.[ch] | \
	sed -e 's/:[^"]*"\([^"]*\)"/:	\1/' -e 's/\.c/.o/' | \
	awk ' { if ($$1 != prev) { print rec; rec = $$0; prev = $$1; } \
		else { if (length(rec $$2) > 78) { print rec; rec = $$0; } \
		       else rec = rec " " $$2 } } \
	      END { print rec } ' > makedep
	echo '/^# DO NOT DELETE THIS LINE/+2,$$d' >eddep
	echo '$$r makedep' >>eddep
	echo 'w' >>eddep
	cp Makefile Makefile.bak
	ed - Makefile < eddep
	rm eddep makedep
	echo '' >> Makefile
	echo '# DEPENDENCIES MUST END AT END OF FILE' >> Makefile
	echo '# IF YOU PUT STUFF HERE IT WILL GO AWAY' >> Makefile
	echo '# see make depend above' >> Makefile

# DO NOT DELETE THIS LINE -- make depend uses it


codes.o:	xmalloc.h md5.h
decode.o:	xmalloc.h common.h
dosos.o:	xmalloc.h common.h
dospk.o:	version.h xmalloc.h
dosunpk.o:	version.h
macmpack.o:	macnapp.h macmpack.h version.h
macnapp.o:	macnapp.h
macnclip.o:	macnapp.h
macndlog.o:	macnapp.h
macninit.o:	macnapp.h
macnte.o:	macnapp.h
macos.o:	common.h macnapp.h macmpack.h
macpcstr.o:	macnapp.h
md5c.o:	md5.h
unixos.o:	xmalloc.h common.h
unixpk.o:	common.h version.h xmalloc.h
unixunpk.o:	version.h
uudecode.o:	xmalloc.h common.h

# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above


