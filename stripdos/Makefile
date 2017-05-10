# MAKEFILE

T= stripdos

ALL= $(T)

SRCROOT= $(LOCAL)


LIBDIR= $(SRCROOT)/lib
BINDIR= $(SRCROOT)/bin
INCDIR= $(SRCROOT)/include

CC= gcc
DEFINES= -DSOLARIS=1
INCDIRS= -I$(INCDIR)
CCOPTS= -O3 -mcpu=ultrasparc # -fpic
CFLAGS= $(CCOPTS) $(DEFINES) $(INCDIRS)

LD= $(CC)
LDFLAGS= -R$(LIBDIR)/$(OFD):$(LIBDIR)
LIBDIRS= $(LIBDIR)/$(OFD)
LIBS= -ldam -lb -luc -lu -lsocket -lnsl

#CRT0= /lib/crt0.o
CRT0=


OBJ= main.o



all:		$(ALL)

.c.o:
	$(CC) $(CFLAGS) -c $<


$(T):		$(OBJ) Makefile
	$(LD) $(LDFLAGS) -o $@ $(CRT0) $(OBJ) -L$(LIBDIR) $(LIBS)

strip:		$(T)
	strip $(T)

install:	$(T)
	strip $(T)
	cp $(T) $(BINDIR)

again:
	rm -f $(ALL)

clean:
	rm -f *.o


