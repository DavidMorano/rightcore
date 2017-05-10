# MAKEFILE

T= libac

ALL= $(T).a

SECROOT= $(LOCAL)


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
LD= ld

LDFLAGS=
LIBDIRS=
LIBS=

LINT= lint
LINTFLAGS= -uxn -Dlint

NM= nm
NMFLAGS= -x

CPP= cpp


# files

I00=
I01=
I02=
I03=
I04=
I05=
I06=
I07=

INCSA= $(I00) $(I01) $(I02) $(I03)
INCSB= $(I04) $(I05) $(I06) $(I07)

INCS= $(INCSA) $(INCSB) $(INCSC) $(INCSD)


OBJ00=
OBJ01= strlen.o strncasecmp.o strlcat.o strlcpy.o
OBJ02= isalpha.o isdigit.o 
OBJ03= dirname.o basename.o
OBJ04= 
OBJ05= mkdir.o ftime.o 
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
OBJ43=
OBJ44=
OBJ45=
OBJ46=
OBJ47=
OBJ48=
OBJ49=
OBJ50=
OBJ51=
OBJ52=
OBJ53=
OBJ54=
OBJ55=
OBJ56=
OBJ57=
OBJ58=
OBJ59=
OBJ60=
OBJ61=
OBJ62=
OBJ63=

OBJA= $(OBJ00) $(OBJ01) $(OBJ02) $(OBJ03) $(OBJ04) $(OBJ05) $(OBJ06) $(OBJ07) 
OBJB= $(OBJ08) $(OBJ09) $(OBJ10) $(OBJ11) $(OBJ12) $(OBJ13) $(OBJ14) $(OBJ15)
OBJC= $(OBJ16) $(OBJ17) $(OBJ18) $(OBJ19) $(OBJ20) $(OBJ21) $(OBJ22) $(OBJ23) 
OBJD= $(OBJ24) $(OBJ25) $(OBJ26) $(OBJ27) $(OBJ28) $(OBJ29) $(OBJ30) $(OBJ31)
OBJE= $(OBJ32) $(OBJ33) $(OBJ34) $(OBJ35) $(OBJ36) $(OBJ37) $(OBJ38) $(OBJ39)
OBJF= $(OBJ40) $(OBJ41) $(OBJ42) $(OBJ43) $(OBJ44) $(OBJ45) $(OBJ46) $(OBJ47)
OBJG= $(OBJ48) $(OBJ49) $(OBJ50) $(OBJ51) $(OBJ52) $(OBJ53) $(OBJ54) $(OBJ55)
OBJH= $(OBJ56) $(OBJ57) $(OBJ58) $(OBJ59) $(OBJ60) $(OBJ61) $(OBJ62) $(OBJ63)

OBJ= $(OBJA) $(OBJB) $(OBJC) $(OBJD) $(OBJE) $(OBJF) $(OBJG) $(OBJH)

OBJS= obja.o objb.o objc.o objd.o obje.o objf.o objg.o objh.o



.SUFFIXES:		.ls


default:		all

all:			$(ALL)

.c.o:
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $<

.c.ln:
	$(LINT) -c -u $(CPPFLAGS) $<

.c.ls:
	$(LINT) $(LINTFLAGS) $(CPPFLAGS) $<


$(T).a:			$(OBJ)
	$(AR) -rc $(T).a $?

$(T).so:		$(OBJS) Makefile localmisc.h $(T).a
	$(LD) -o $@ -G $(LDFLAGS) $(OBJS)

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

objf.o:			$(OBJF)
	$(LD) -o $@ -r $(LDFLAGS) $(OBJF)

objg.o:			$(OBJG)
	$(LD) -o $@ -r $(LDFLAGS) $(OBJG)

objh.o:			$(OBJH)
	$(LD) -o $@ -r $(LDFLAGS) $(OBJH)

again:
	$(RM) $(ALL)

clean:			again
	$(RM) *.o $(ALL)

safe:
	makesafe -v=3 -I $(INCDIR) $(OBJ)

install:		$(ALL) Makefile install.incs
	bsdinstall $(ALL) $(SRCROOT)/lib/$(OFD)

install.incs:		$(INCS)
	makenewer $(INCS) -d $(INCDIR) -t $@

control:
	(uname -n ; date) > Control


ftime.o:	ftime.mod
	cp -p ftime.mod ftime.o



