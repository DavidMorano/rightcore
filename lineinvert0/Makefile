
# check the definition for BASE for compatibility with your setup

T= lineinvert

SRCROOT= $(LOCAL)

BINDIR= $(SRCROOT)/bin
INCDIR= $(SRCROOT)/include
LIBDIR= $(SRCROOT)/lib/$(OFD)

CC= gcc
DEFINES=
INCDIRS= -I$(INCDIR)
CFLAGS= -O $(DEFINES) $(INCDIRS)

LD= $(CC)
LDFLAGS=
LIBDIRS= -L$(LIBDIR)
LIBS= -ldam -lb

#CRT0= /lib/crt0.o
CRT0=


OBJ= main.o



all:		$(T)

.c.o:
	$(CC) -c $(CFLAGS) $<


$(T):	$(OBJ) Makefile
	$(LD) -o $@ $(LDFLAGS) $(CRT0) $(OBJ) $(LIBDIRS) $(LIBS)

strip:		$(T)
	strip $(T)

install:	$(T)
	strip $(T)
	cp $(T) $(BINDIR)

clean:
	rm -f *.o



