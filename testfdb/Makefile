# MAKEFILE

T= libfdb

ALL= $(T).a

#SRCROOT= $(LOCAL)


BINDIR= $(SRCROOT)/bin
INCDIR= $(SRCROOT)/include
LIBDIR= $(SRCROOT)/lib
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
#CCOPTS= $(CCOPTS_GCCALL) -g -Wstrict-aliasing -Wall

CCOPTS_GCCLIB= -fpic $(CCOPTS_GCCLIBSPARC)

#CCOPTS= $(CCOPTS_CCALL) -g -xs

# HyperSPARC
#CCOPTS= -xO5 -Kpic -xtarget=ss20/hs22 -dalign -xdepend

# UltraSPARC
#CCOPTS= -xO5 -Kpic -xtarget=ultra -xsafe=mem -dalign -xdepend


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

CFLAGS= $(CCOPTS) $(DEFINES) $(INCDIRS)

#LD= $(CC)
LDFLAGS=
LIBDIRS=
LIBS=

LINT= lint
LINTFLAGS= -uxn -Dlint $(DEFINES) $(INCDIRS)

# files

OBJ00= lockfile.o fdb.o
OBJ01=
OBJ02=
OBJ03=
OBJ04=
OBJ05=
OBJ06=
OBJ07=
OBJ08=
OBJ09=
OBJ10=
OBJ11=
OBJ12=
OBJ13=
OBJ14=
OBJ15=
OBJ16=
OBJ17=
OBJ18=
OBJ19=
OBJ20=
OBJ21=
OBJ22=
OBJ23=
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
OBJ40=
OBJ41=
OBJ42=

OBJA= $(OBJ00) $(OBJ01) $(OBJ02) $(OBJ03) $(OBJ04) $(OBJ05) $(OBJ06) $(OBJ07) 
OBJB= $(OBJ08) $(OBJ09) $(OBJ10) $(OBJ11) $(OBJ12) $(OBJ13) $(OBJ14) $(OBJ15)
OBJC= $(OBJ16) $(OBJ17) $(OBJ18) $(OBJ19) $(OBJ20) $(OBJ21) $(OBJ22) $(OBJ23) 
OBJD= $(OBJ24) $(OBJ25) $(OBJ26) $(OBJ27) $(OBJ28) $(OBJ29) $(OBJ30) $(OBJ31)
OBJE= $(OBJ32) $(OBJ33) $(OBJ34) $(OBJ35) $(OBJ36) $(OBJ37) $(OBJ38) $(OBJ39)
OBJF= $(OBJ40) $(OBJ41) $(OBJ42) $(OBJ43) $(OBJ44) $(OBJ45) $(OBJ46) $(OBJ47)

OBJ= $(OBJA) $(OBJB) $(OBJC) $(OBJD) $(OBJE) $(OBJF)

INCS= fdb.h


.SUFFIXES:		.ls .i .cx .cs


all:			$(ALL)

.c.o:
	$(CC) -c $(CFLAGS) $<
	ar -rc $(T).a $@

.cc.o:
	$(GPP) -c $(CFLAGS) $<

.c.ln:
	$(LINT) -u $(DEFINES) $(INCDIRS) $<


$(T).so:		$(OBJ) Makefile localmisc.h $(T).a
	$(LD) -o $@ -G $(LDFLAGS) $(OBJ)

$(T).a:			$(OBJ) Makefile localmisc.h

again:
	rm -f $(ALL)

clean:			again
	rm -f *.o $(ALL)

install:		$(T).a Makefile $(INCS)
	bsdinstall $(INCS) $(INCDIR)
	ranlib $(T).a
	bsdinstall $(T).a $(SRCROOT)/lib/$(OFD)

install.$(OFF):		$(T).a Makefile $(INCS)
	bsdinstall $(T).a $(LIBDIR)

control:
	(uname -n ; date) > Control


field.o:	field.c field.h

veclist.o:	vechand.c vechand.h

userinfo.o:	userinfo.c userinfo.h

rfile.o:	rfile.c incfile_rfilewrite.h

incfile_rfilewrite.h:	rfilewrite
	mkincfile rfilewrite

getdate.o:	getdate.mod
	cp getdate.mod getdate.o

fdb.o:		fdb.c fdb.h localmisc.h


