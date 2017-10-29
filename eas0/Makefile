# MAKEFILE

T= eas

BINDIR= $(SRCROOT)/bin
INCDIR= $(SRCROOT)/include
LIBDIR= $(SRCROOT)/lib/$(OFD)


#CC= cc
INCDIRS= -I$(INCDIR)
CFLAGS= -O -D$(OS)=1 $(INCDIRS)

LD= $(CC)
LDFLAGS=
LIBDIRS= -L$(LIBDIR)
LIBS= -lmisc -lb

#CRT0= /lib/crt0.o
CRT0=


ALL = $(T).x $(T)

OBJ= main.o


.c.o:
	$(CC) -c $(CFLAGS) $<

.l.c:
	echo "sorry"

.y.c:
	echo "sorry"


all:		$(ALL)

$(T):		$(T).sh
	cp $(T).sh $(T)

$(T).x:		$(OBJ) Makefile
	$(LD) -o $@ $(LDFLAGS) $(CRT0) $(OBJ) $(LIBDIRS) $(LIBS)

strip:		$(T).x
	strip $(T).x

$(T).elf elf:	$(T).x
	strip $(T).x
	cp $(T).x $(T).elf

$(T).aout aout:	$(T).x
	strip $(T).x
	cp $(T).x $(T).aout

install:	$(T).x
	bsdinstall $(T).aout $(T).elf $(BINDIR)
 
clean:
	rm -f *.o

lint:		main.c
	lint -uxn $(INCDIRS) main.c
	touch lint



