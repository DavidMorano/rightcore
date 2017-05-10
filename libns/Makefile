# MAKEFILE

T= libns

ALL= $(T) $(T).$(OFF)

SRCROOT= $(LOCAL)


BINDIR= $(SRCROOT)/bin
INCDIR= $(SRCROOT)/include
LIBDIR= $(SRCROOT)/lib

#LDCRTDIR= /opt/SUNWspro/WS6/lib
#LDCRTDIR= /opt/SUNWspro/SC4.0/lib
#LDCRTDIR= /opt/SUNWspro/lib


CC= gcc

CCOPTS= -O3 -fpic -mcpu=ultrasparc -mno-app-regs
#CCOPTS= -g

# HyperSPARC
#CCOPTS= -xO5 -xtarget=ss20/hs22 -dalign -xdepend

# UltraSPARC
#CCOPTS= -xO5 -xtarget=ultra -xsafe=mem -dalign -xdepend


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
#LD= cc
LD= ld

LDFLAGS= -m -R$(LIBDIR)/$(OFD):$(LIBDIR)


LIBDIRS= -L$(LIBDIR)/$(OFD) -L$(LIBDIR)

LIB0=
LIB1= -ldam -lb -luc
LIB2=
LIB3= -Bstatic -lu -Bdynamic
LIB4= -L$(GNU)/lib -lgcc
LIB5= 
LIB6= -lsecdb -lproject -lpthread -lrt -lxnet -lsocket -lnsl
LIB7= -ldl -lc

LIBS= $(LIB0) $(LIB1) $(LIB2) $(LIB3) $(LIB4) $(LIB5) $(LIB6) $(LIB7)


CRTI= $(LDCRTDIR)/crti.o
CRT1= $(LDCRTDIR)/crt1.o
CRTN= $(LDCRTDIR)/crtn.o
CRT0= $(CRTI) $(CRT1)

CRTC= makedate.o
CRT= $(CRTI) $(CRT1) $(CRTN)

LINT= lint
LINTFLAGS= -uxn -Dlint $(DEFS) $(INCDIRS)



HELP= -DCMDHELPFNAME=\"lib/vmail/cmdhelp\"

INC= config.h defs.h profile.h


SLIBS= libcurses.a vsnprintf.o

OBJ00= main.o defs.o booleval.o boxname.o cmd_delete.o cmd_undelete.o 
OBJ01= forward.o fetchfield.o help.o inter.o
OBJ02= leparse.o cmd_mlist.o mailcheck.o pipe.o
OBJ03= profile.o putfile.o reply.o rewrite.o expunge.o
OBJ04= cmd_savefile.o scanner.o search.o
OBJ05= psearch.o cmd_edit.o sdisplay.o ds.o
OBJ06= setup.o display.o
OBJ07= digit3.o timestr_scandate.o
OBJ08= abspath.o navigate.o mailbox.o termstore.o test.o whatinfo.o
OBJ09= utty.o
OBJ10= getspoolfile.o matmsg.o

OBJA= $(OBJ00) $(OBJ01) $(OBJ02) $(OBJ03) $(OBJ04) $(OBJ05) $(OBJ06) $(OBJ07)
OBJB= $(OBJ08) $(OBJ09) $(OBJ10) $(OBJ11) $(OBJ12) $(OBJ13) $(OBJ14) $(OBJ15)

OBJ= $(OBJA) $(OBJB) $(SLIBS)

OBJS= $(CRT0) $(OBJ) $(CRTC) 


default:		$(T).x

all:			$(ALL)

.c.o:			defs.h
	$(CC) -c $(CFLAGS) $<

.c.t:
	$(LINT) -u $(DEFS) $(INCDIRS) $<

.c.ln:
	$(LINT) -u $(DEFS) $(INCDIRS) $<


$(T):			$(T).sh
	cp -p $(T).sh $(T)

$(T).x:			$(OBJ) Makefile
	makedate > makedate.c
	$(CC) -c makedate.c
	$(LD) -o $@ $(LDFLAGS) $(OBJS) $(LIBDIRS) $(LIBS) $(CRTN) > $(T).lm

$(T).$(OFF) $(OFF):	$(T).x
	cp -p $(T).x $(T).$(OFF)

strip:			$(T).x
	strip $(T).x
	rm -f $(T) $(T).$(OFF)

install:		$(ALL) $(T).$(OFF)
	bsdinstall $(ALL) $(BINDIR)

again:
	-rm -f $(ALL) $(T).x $(T).$(OFF)

clean:			again
	-rm -f $(OBJ) $(ALL)

control:
	(uname -n ; date) > Control



libcurses.a:		libcurses.save
	cp -p libcurses.save libcurses.o

vsnprintf.o:		vsnprintf.save
	cp -p vsnprintf.save vsnprintf.o


main.o:			main.c $(INC)
	$(CC) -c $(CFLAGS) $(HELP) main.c

whatinfo.o:		whatinfo.c config.h

help.o:			help.c $(INC)
	$(CC) -c $(CFLAGS) help.c

*.o:			$(INC)





