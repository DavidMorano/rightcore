# MAKEFILE

T= invert

#SRCROOT= $(LOCAL)


BINDIR= $(SRCROOT)/bin
INCDIR= $(SRCROOT)/include
LIBDIR= $(SRCROOT)/lib/ELF


CC= gcc

DEFINES=

INCDIRS= -I$(INCDIR)

CFLAGS= -O $(DEFINES) -I$(INCDIR)

LD= $(CC)

LDFLAGS=

LIBDIRS= -L$(LIBDIR)

LIBS= -lmisc -lb

#CRT0= /lib/crt0.o
CRT0=


OBJ= main.o


.SUFFIXES:		.ls .i .cx .cs


all:		$(T)

.c.o:
	$(CC) -c $(CFLAGS) $<


$(T):		$(OBJ) Makefile
	$(LD) -o $@ $(LDFLAGS) $(CRT0) $(OBJ) $(LIBDIRS) $(LIBS)

strip:		$(T)
	strip $(T)

install:	$(T)
	strip $(T)
	cp $(T) $(BINDIR)

clean:
	rm -f *.o


