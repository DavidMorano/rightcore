# MAKEFILE

T= teststruct

ALL=	$(T)

#SRCROOT= $(LOCAL)


LIBDIR= $(SRCROOT)/lib
BINDIR= $(SRCROOT)/bin
INCDIR= $(SRCROOT)/include


CC= gcc
CCOPTS=
INCDIRS= -I$(INCDIR)
CFLAGS= $(CCOPTS) $(INCDIRS)

LD= cc
LDFLAGS=
LIBDIRS= -L$(LIBDIR)
LIBS= -Bstatic -ldam -lb -luc -lu -Bdynamic

#CRT0= /lib/crt0.o
CRT0=



OBJ= main.o sub.o



all:		$(ALL)

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



