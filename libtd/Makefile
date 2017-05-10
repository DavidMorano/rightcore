# MAKEFILE

T= libtd

ALL= $(T).a

SRCROOT= $(LOCAL)


BINDIR= $(SRCROOT)/bin
INCDIR= $(SRCROOT)/include
LIBDIR= $(SRCROOT)/lib

#LDCRTDIR= /opt/SUNWspro/WS6/lib
#LDCRTDIR= /opt/SUNWspro/SC5.0/lib
#LDCRTDIR= /opt/SUNWspro/SC4.0/lib
#LDCRTDIR= /opt/SUNWspro/lib
LDCRTDIR= $(SRCROOT)/lib


CC= gcc

#CCOPTS= -O3 -fpic -mcpu=ultrasparc -mno-app-regs
#CCOPTS= -mcpu=ultrasparc -fpic
CCOPTS= -g -fpic

# HyperSPARC
#CCOPTS= -xO5 -K pic -xtarget=ss20/hs22 -dalign -xdepend

# UltraSPARC
#CCOPTS= -xO5 -K pic -xtarget=ultra -xsafe=mem -dalign -xdepend


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

LDFLAGS=

LIBDIRS=

LIBS=

LINT= lint
LINTFLAGS= -uxn -Dlint

NM= nm
NMFLAGS= -sx

CPP= cpp


I00= td.h
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
OBJ01= td.o
OBJ02=
OBJ03= sbuf.o
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

OBJ64=
OBJ65=
OBJ66=
OBJ67=
OBJ68=
OBJ69=
OBJ70=
OBJ71=


OBJA= $(OBJ00) $(OBJ01) $(OBJ02) $(OBJ03) $(OBJ04) $(OBJ05) $(OBJ06) $(OBJ07) 
OBJB= $(OBJ08) $(OBJ09) $(OBJ10) $(OBJ11) $(OBJ12) $(OBJ13) $(OBJ14) $(OBJ15)
OBJC= $(OBJ16) $(OBJ17) $(OBJ18) $(OBJ19) $(OBJ20) $(OBJ21) $(OBJ22) $(OBJ23) 
OBJD= $(OBJ24) $(OBJ25) $(OBJ26) $(OBJ27) $(OBJ28) $(OBJ29) $(OBJ30) $(OBJ31)
OBJE= $(OBJ32) $(OBJ33) $(OBJ34) $(OBJ35) $(OBJ36) $(OBJ37) $(OBJ38) $(OBJ39)
OBJF= $(OBJ40) $(OBJ41) $(OBJ42) $(OBJ43) $(OBJ44) $(OBJ45) $(OBJ46) $(OBJ47)
OBJG= $(OBJ48) $(OBJ49) $(OBJ50) $(OBJ51) $(OBJ52) $(OBJ53) $(OBJ54) $(OBJ55)
OBJH= $(OBJ56) $(OBJ57) $(OBJ58) $(OBJ59) $(OBJ60) $(OBJ61) $(OBJ62) $(OBJ63)
OBJI= $(OBJ64) $(OBJ65) $(OBJ66) $(OBJ67) $(OBJ68) $(OBJ69) $(OBJ70) $(OBJ71)

OBJ= $(OBJA) $(OBJB) $(OBJC) $(OBJD) $(OBJE) $(OBJF) $(OBJG) $(OBJH) $(OBJI)

OBJS= obja.o objb.o objc.o objd.o obje.o objf.o objg.o objh.o



.SUFFIXES:		.ls .ln


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

safe:
	makesafe -v=3 -I $(INCDIR) $(OBJS)

again:
	$(RM) $(ALL)

clean:			again
	$(RM) *.o $(ALL)

install:		$(ALL) Makefile install-incs
	bsdinstall $(ALL) $(SRCROOT)/lib/$(OFD)

install-incs:		$(INCS)
	makenewer $(INCS) -d $(INCDIR) -t $@

control:
	(uname -n ; date) > Control


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



td.o:		td.c td.h termstr.h




