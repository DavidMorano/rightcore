# MAKEFILE
# check the definition for BASE for compatibility with your setup

T= rslow 

SRCROOT=$(PCS)


BINDIR= $(SRCROOT)/bin
INCDIR= $(SRCROOT)/include
LIBDIR= $(SRCROOT)/lib/$(OFD)


#CC= cc

INCDIRS= -I$(INCDIR)

CFLAGS= -O -D$(OS)=1 $(INCDIRS)

LD= $(CC)

LDFLAGS=

LIBPATHS= -L$(LIBDIR)

LIBS= -lpcs -lmisc -lb -lsocket -lnsl

#CRT0= /lib/crt0.o


.c.o:
	$(CC) -c $(CFLAGS) $<


OBJ= main.o

ALL= $(T).x $(T)


all:		$(ALL)

$(T):		$(T).sh
	cp $(T).sh $(T)

$(T).x:		$(OBJ) Makefile
	$(LD) $(LDFLAGS) -o $@ $(CRT0) $(OBJ) $(LIBPATHS) $(LIBS)

$(T).elf elf:	$(T).x
	strip $(T).x
	cp $(T).x $(T).elf

$(T).aout aout:	$(T).x
	strip $(T).x
	cp $(T).x $(T).aout

strip:		$(T).x
	strip $(T).x

install:	$(T).x
	strip $(T).x
	cp $(T).x $(BINDIR)

clean:
	rm -f *.o $(T).x


main.o:		main.c config.h defs.h



