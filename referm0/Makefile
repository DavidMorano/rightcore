# MAKEFILE

T= referm

BINDIR= $(SRCROOT)/bin
INCDIR= $(SRCROOT)/include
LIBDIR= $(SRCROOT)/lib/$(OFD)


CC= gcc

INCDIRS= -I$(INCDIR)
CFLAGS= -O $(INCDIRS)

LD= $(CC)
LDFLAGS=
LIBPATHS= -L$(LIBDIR)
LIBS= -lmisc -lb

#CRT0= /lib/crt0.o
CRT0=


OBJ= main.o


.c.o:
	$(CC) -c $(CFLAGS) $<


ALL= $(T) $(T).x


all:		$(ALL)

$(T):		$(T).sh
	cp $(T).sh $(T)

$(T).x:		$(OBJ) Makefile
	$(LD) -o $@ $(LDFLAGS) $(CRT0) $(OBJ) $(LIBPATHS) $(LIBS)

$(T).elf elf:	$(T).x
	cp $(T).x $(T).elf

$(T).aout aout:	$(T).x
	cp $(T).x $(T).aout

strip:		$(T).x
	strip $(T).x

install:	$(T).x
	strip $(T).x
	cp $(T).x $(BINDIR)

clean:
	rm -f *.o $(T).x


main.o:		main.c config.h



