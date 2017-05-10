# MAKEFILE

T= libut

ALL= $(T).a

SRCROOT= $(LOCAL)


BINDIR= $(SRCROOT)/bin
INCDIR= $(SRCROOT)/include
LIBDIR= $(SRCROOT)/lib


#CC= gcc

#CCOPTS= -O3 -fpic -mcpu=ultrasparc -mno-app-regs
#CCOPTS= -fpic

# HyperSPARC
#CCOPTS= -xO5 -Kpic -xtarget=ss20/hs22 -xdepend

# UltraSPARC
#CCOPTS= -xO5 -Kpic -xtarget=ultra -xsafe=mem -xdepend

# UltraSPARC-v9 (64 bits)
CCOPTS= -xO5 -xtarget=ultra -xarch=v9 -xsafe=mem -xdepend


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


LIBDIRS=

LIBS= $(LF_LIBS) -L$(GNU)/lib -lgcc


LINT= lint
LINTFLAGS= $(LF_LINTFLAGS) -uxn -Dlint

NM= nm
NMFLAGS= -sx -v

CPP= cpp

LORDER= lorder
TSORT= tsort

RM= rm -f
TOUCH= /bin/touch


INCS=


OBJ00=
OBJ01= ut_open.o ut_close.o
OBJ02= ut_bind.o ut_connect.o
OBJ03= ut_accept.o ut_listen.o 
OBJ04= ut_look.o ut_sync.o
OBJ05= ut_alloc.o ut_free.o 
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

OBJA= $(OBJ00) $(OBJ01) $(OBJ02) $(OBJ03) $(OBJ04) $(OBJ05) $(OBJ06) $(OBJ07) 
OBJB= $(OBJ08) $(OBJ09) $(OBJ10) $(OBJ11) $(OBJ12) $(OBJ13) $(OBJ14) $(OBJ15)
OBJC= $(OBJ16) $(OBJ17) $(OBJ18) $(OBJ19) $(OBJ20) $(OBJ21) $(OBJ22) $(OBJ23)
OBJD= $(OBJ24) $(OBJ25) $(OBJ26) $(OBJ27) $(OBJ28) $(OBJ29) $(OBJ30) $(OBJ31)

OBJ= $(OBJA) $(OBJB) $(OBJC) $(OBJD)

OBJS= obja.o


.SUFFIXES:		.ls .i .cx .cs


default:		all

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


$(T).a:			$(OBJ)
	$(AR) -rc $(T).a $?

$(T).so:		$(OBJS) Makefile localmisc.h $(T).a
	$(LD) -o $@ -G $(LDFLAGS) $(OBJS)

install:		$(ALL) Makefile
	ranlib $(T).a
	bsdinstall $(ALL) $(LIBDIR)

again:
	rm -f $(ALL)

clean:
	rm -f *.o $(T).a

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


field.o:		field.c field.h

veclist.o:		vechand.c vechand.h

userinfo.o:		userinfo.c userinfo.h

rfile.o:		rfile.c incfile_rfilewrite.h

incfile_rfilewrite.h:	rfilewrite
	mkincfile rfilewrite

getdate.o:		getdate.mod
	cp getdate.mod getdate.o


