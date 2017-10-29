# MAKEFILE (finger)

T= fingerd

ALL= $(T)

SRCROOT= $(NCMP)


BINDIR= $(SRCROOT)/bin
INCDIR= $(SRCROOT)/include
LIBDIR= $(SRCROOT)/lib


CC= gcc

#CCOPTS= -O3 -mcpu=v9 # -fpic

CCOPTS= -g


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
#LD= ld
LD= cc

LDFLAGS= -R$(LIBDIR)


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


LIBCC= /opt/SUNWspro/SC4.0/lib

#CRTI= $(LIBCC)/crti.o
#CRT1= $(LIBCC)/crt1.o
#CRTN= $(LIBCC)/crtn.o
CRT0= $(CRTI) $(CRT1)

CRTC= makedate.o
CRT= $(CRTI) $(CRT1) $(CRTN)

LINT= lint
LINTFLAGS= -uxn -Dlint $(DEFS) $(INCDIRS)


HELP= -DHELPFILE=\"lib/vmail/help\"


INC= pathnames.h

OBJ= fingerd.o

OBJS= $(CRT0) $(OBJ) $(CRTC)


all:			$(ALL)

.c.o:			defs.h
	$(CC) -c $(CFLAGS) $<

.c.t:
	$(LINT) -u $(DEFS) $(INCDIRS) $<


$(T):			$(T).sh
	cp $(T).sh $(T)

$(T).x:			$(OBJ) Makefile
	makedate > makedate.c
	$(CC) -c $(CFLAGS) makedate.c
	$(LD) -o $@ $(LDFLAGS) $(OBJS) $(LIBDIRS) $(LIBS) $(CRTN)

$(T).$(OFF) $(OFF):	$(T).x
	strip $(T).x
	cp $(T).x $(T).$(OFF)

strip:			$(T).x
	strip $(T).x

install:		$(ALL) $(T).elf
	rm -f n$(T) n$(T).x n$(T).elf
	-ln $(T) n$(T) 2> /dev/null
	-ln $(T).elf n$(T).elf 2> /dev/null
	bsdinstall $(T) $(T).elf $(BINDIR)
	bsdinstall n$(T) n$(T).elf $(BINDIR)
	bsdinstall helpfile $(PCS)/lib/vmail/help

again:
	-rm -f $(ALL) $(T).x

clean:
	-rm -f $(OBJ) $(ALL) $(T).x

control:
	(uname -n ; date) > Control



main.o:			main.c $(INC)
	$(CC) -c $(CFLAGS) $(HELP) main.c

help.o:			help.c $(INC)
	$(CC) -c $(CFLAGS) help.c

*.o:			$(INC)





