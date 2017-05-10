# MAKEFILE

T= libgetchost

ALL= $(T).a

SRCROOT= $(LOCAL)


BINDIR= $(SRCROOT)/bin
INCDIR= $(SRCROOT)/include
LIBDIR= $(SRCROOT)/lib


#CC= gcc

#CCOPTS= -O3 -fpic -mcpu=ultrasparc -mno-app-regs
#CCOPTS= -fpic

# HyperSPARC
CCOPTS= -xO5 -Kpic -xtarget=ss20/hs22 -dalign -xdepend

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

CFLAGS= $(CCOPTS) $(DEFS) $(INCDIRS)

#LD= $(CC)
LDFLAGS=
LIBDIRS=
LIBS=

LINT= lint
LINTFLAGS= -uxn -Dlint $(DEFS) $(INCDIRS)


# files

OBJ00= getchost.o
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
OBJ31=
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

INCS= field.h userinfo.h vechand.h



all:		$(ALL)

.c.o:
	$(CC) -c $(CFLAGS) $<
	ar -rc $(T).a $@

.c.ln:
	$(LINT) -u $(DEFS) $(INCDIRS) $<


$(T).so:	$(OBJ) Makefile localmisc.h $(T).a
	$(LD) -o $@ -G $(LDFLAGS) $(OBJ)

$(T).a:		$(OBJ) Makefile localmisc.h

again:
	rm -f $(ALL)

clean:
	rm -f *.o $(T).so $(T).a

install:	$(T).a Makefile $(INCS)
	bsdinstall $(INCS) $(INCDIR)
	ranlib $(T).a
	bsdinstall $(T).a $(SRCROOT)/lib

install.$(OFF):	$(T).a Makefile $(INCS)
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

hdb.o:		hdb.c hdb.h



