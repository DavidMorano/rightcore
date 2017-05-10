# MAKEFILE (libparser)

T= libparser

ALL= $(T).a

SRCROOT= $(PCS)


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


OBJ= c_cmd_mat.o c_parse.o


all:		$(T).a

.c.o:		c_parse.h
	$(CC) -c $(CFLAGS) $*.c

$(T).a:		$(OBJ)
	  ar -cr $@ $(OBJ)

clean:
	  rm -f *.o $(T).a

install:	$(T).a
	bsdinstall c_parse.h $(INCDIR)
	bsdinstall $(ALL) $(LIBDIR)

control:
	uname -n > Control
	date >> Control



