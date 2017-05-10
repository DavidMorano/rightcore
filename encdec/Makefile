# MAKEFILE

T= encdec


BINDIR= $(SRCROOT)/bin
INCDIR= $(SRCROOT)/include
LIBDIR= $(SRCROOT)/lib


#CC= cc
DEFINES= -D$(OS)=1
INCDIRS= -I$(INCDIR)
CFLAGS= -O $(DEFINES) $(INCDIRS)

LD= $(CC)
LIBS= -lmisc -lb
LDFLAGS=

#CRT0= /lib/crt0.o
CRT0=


.c.o:
	$(CC) -c $(CFLAGS) $<


ALL= $(T) $(T).x

OBJ= encdec.o


all:			$(ALL)

$(T):			$(T).sh
	cp $(T).sh $(T)

$(T).x:			$(OBJ) Makefile
	$(LD) $(LDFLAGS) -o $@ $(CRT0) $(OBJ) -L$(LIBDIR) $(LIBS)

$(T).elf elf:		$(T).x
	strip $(T).x
	cp $(T).x $(T).elf

$(T).aout aout:		$(T).x
	strip $(T).x
	cp $(T).x $(T).aout

strip:			$(T).x
	strip $(T).x

install:		$(T).x
	strip $(T).x
	bsdinstall $(T).x $(BINDIR)

clean:
	rm -f *.o



