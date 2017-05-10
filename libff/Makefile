# MAKEFILE

T= libff

ALL= $(T).a

SRCROOT= $(LOCAL)


INCDIR= $(SRCROOT)/include
LIBDIR= $(SRCROOT)/lib


CC= gcc
GPP= g++

CCOPTS= $(CCOPTS_GCCALL) $(CCOPTS_GCCOPT) $(CCOPTS_GCCLIB)
#CCOPTS= $(CCOPTS_GCCALL) $(CCOPTS_GCCLIB)
#CCOPTS= $(CCOPTS_GCCALL) $(CCOPTS_GCCLIB) -g -Wstrict-aliasing

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

CPPFLAGS= $(DEFS) $(INCDIRS)

CFLAGS= $(CCOPTS)

#LD= $(CC)

LDFLAGS=
LIBDIRS=
LIBS=

RANLIB= ranlib

LINT= lint
LINTFLAGS= -uxn -Dlint

NM= nm
NMFLAGS= -sx

CPP= cpp

LORDER= lorder
TSORT= tsort

RM= rm -f


INCS= ffile.h


OBJ00=
OBJ01= ffopen.o 
OBJ02= ffread.o ffgetc.o ffreadline.o 
OBJ03= ffwrite.o ffputc.o ffprintf.o
OBJ04= ffseek.o fftell.o
OBJ05= ffclose.o ffflush.o
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

OBJA= $(OBJ00) $(OBJ01) $(OBJ02) $(OBJ03) $(OBJ04) $(OBJ05) $(OBJ06) $(OBJ07)
OBJB= $(OBJ08) $(OBJ09) $(OBJ10) $(OBJ11) $(OBJ12) $(OBJ13) $(OBJ14) $(OBJ15)
OBJC= $(OBJ16) $(OBJ17) $(OBJ18) $(OBJ19) $(OBJ20) $(OBJ21) $(OBJ22) $(OBJ23)

OBJ= $(OBJA) $(OBJB) $(OBJC)

OBJS= obja.o objb.o objc.o


.SUFFIXES:		.ls .i .cx .cs


default:		all

all:			$(ALL)

.c.o:			$(INCS)
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $<

.cc.o:			$(INCS)
	$(GPP) -c $(CFLAGS) $(CPPFLAGS) $<

.s.o:
	$(AS) -o $@ $(ASFLAGS) $<

.c.ln:
	$(LINT) -c -u $(CPPFLAGS) $<

.c.ls:
	$(LINT) $(LINTFLAGS) $(CPPFLAGS) $<

.c.i:
	$(CPP) $(CPPFLAGS) $< > $(*).i


$(T).a:			$(OBJ)
	$(AR) -cr $@ $?

$(T).so:		$(OBJS) Makefile $(T).a
	$(LD) -o $@ -G $(LDFLAGS) $(OBJS)

install:		$(T).a
	makenewer $(T).a $(LIBDIR)

install.$(OFF):		install

again:
	rm -f $(T).a

clean:			again
	rm -f $(T).a *.o

$(T).order order:	$(OBJ) $(T).a
	$(LORDER) $(T).a | $(TSORT) > $(T).order
	$(RM) $(T).a
	while read O ; do $(AR) -cr $(T).a $${O} ; done < $(T).order

safe:
	makesafe -v=3 -I $(INCDIR) $(OBJ)

control:
	uname -n > Control
	date >> Control


obja.o:			$(OBJA)
	$(LD) -o $@ -r $(LDFLAGS) $(OBJA)

objb.o:			$(OBJB)
	$(LD) -o $@ -r $(LDFLAGS) $(OBJB)

objc.o:			$(OBJC)
	$(LD) -o $@ -r $(LDFLAGS) $(OBJC)



