# MAKEFILE

T= ft

ALL= $(T) $(T).x

#SRCROOT= $(LOCAL)


BINDIR= $(SRCROOT)/bin
INCDIR= $(SRCROOT)/include
LIBDIR= $(SRCROOT)/lib


CC= gcc

CCOPTS= -O3 -mcpu=ultrasparc # -fpic


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

LD= $(CC)

LDFLAGS=


LIBDIRS= -L$(LIBDIR)

LIB0=
LIB1= -ldam -lb -luc
LIB2=
LIB3= -Bstatic -lu -Bdynamic
LIB4= -L$(GNU)/lib -lgcc
LIB5= 
LIB6= -lsecdb -lproject -lpthread -lrt -lxnet -lsocket -lnsl
LIB7= -ldl -lc

LIBS= $(LIB0) $(LIB1) $(LIB2) $(LIB3) $(LIB4) $(LIB5) $(LIB6) $(LIB7)


LINT= lint


OBJ1= main.o process.o given_bd.o get_bds.o dirlist.o outbulls.o
OBJ2= bbcmp.o bbcpy.o
OBJ3= initialize.o update.o
OBJ4= emit_item.o emit_header.o emit_count.o
OBJ5= read_dir.o
OBJ6= cmd_save.o cmd_output.o cmd_printout.o cmd_reply.o cmd_follow.o
OBJ7= getfield.o mm_getfield.o
OBJ8= onintr.o monthname.o
OBJ9= dirshown.o
OBJ10= version.o
OBJA= $(OBJ1) $(OBJ2) $(OBJ3) $(OBJ4) $(OBJ5) $(OBJ6) $(OBJ7) $(OBJ8)
OBJB= $(OBJ9) $(OBJ10)
OBJ= $(OBJA) $(OBJB)




.SUFFIXES:	.t



default:		$(T).x

all:			$(ALL)

.c.o:
	$(CC) -c $(CFLAGS) $<

.c.t:
	lint -uxn $(DEFS) $(INCDIRS) $<


$(T):			$(T).sh
	cp -p $(T).sh $(T)

$(T).x:			$(OBJ) Makefile
	$(LD) -o $@ $(OBJ) $(LIBDIRS) $(LIBS)

$(T).$(OFF) $(OFF):	$(T).x
	strip $(T).x
	cp $(T).x $(T).$(OFF)

strip:			$(T).x
	strip $(T).x

$(T):			$(T).sh
	cp -p $(T).sh $(T)

again:
	-rm -fr $(ALL) $(T) $(T).x

clean:			again
	-rm -f *.o $(ALL)

install:		$(ALL)
	bsdinstall $(ALL) $(BINDIR)

control:
	uname -n > Control
	date >> Control

cflow:
	cflow $(INCDIRS) *.c > bb.cflow


main.o:		main.c defs.h config.h




