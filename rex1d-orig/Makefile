# MAKEFILE

T= rex

SRCROOT= $(PCS)


BINDIR= $(SRCROOT)/bin
INCDIR= $(SRCROOT)/include
LIBDIR= $(SRCROOT)/lib/$(OFD)


#CC= gcc
DEFINES= -D$(OS)=1 -DPROGRAMROOT=\"$(SRCROOT)\"
INCDIRS= -I$(INCDIR)
CFLAGS= $(DEFINES) $(INCDIRS)

LD= $(CC)
LDFLAGS=
LIBDIRS= -L$(LIBDIR)

LIBS= -lpcs -lmisc -lb -lu -lsocket -lnsl

#CRT0= /lib/crt0.o


OBJ1= main.o authfile.o hostequiv.o
OBJ2= remotes.o strnncmp.o renv.o worm.o
OBJ3= debug.o
OBJ= $(OBJ1) $(OBJ2) $(OBJ3)


ALL= $(T) $(T).$(OFF)


.c.o:
	$(CC) -c $(CFLAGS) $<


all:		$(ALL)

$(T):		$(T).sh
	cp $(T).sh $(T)

$(T).x:		$(OBJ) Makefile
	$(LD) -o $@ $(LDFLAGS) $(CRT0) $(OBJ) $(LIBDIRS) $(LIBS)

$(T).elf elf:	$(T).x
	strip $(T).x
	cp $(T).x $(T).elf

$(T).aout aout:	$(T).x
	strip $(T).x
	cp $(T).x $(T).aout

strip:		$(T).x
	strip $(T).x

install:	$(T) install.x
	bsdinstall $(T) $(BINDIR)

install.elf:	$(T).elf
	bsdinstall $(T).elf $(BINDIR)
	chmod u+s $(BINDIR)/$(T).elf

install.x:	$(T).x
	strip $(T).x
	bsdinstall $(T).x $(BINDIR)
	chmod u+s $(BINDIR)/$(T).x

clean:
	rm -f *.o $(T).x

control:
	uname -n > Control
	date >> Control


main.o:		main.c config.h defs.h

rex.o:		rex.c
	$(CC) -c $(CFLAGS) -DF_LOG=1 $<




