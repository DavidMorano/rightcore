# MAKEFILE

T= testpoll

ALL= $(T)

#SRCROOT= $(LOCAL)


BINDIR= $(SRCROOT)/bin
INCDIR= $(SRCROOT)/include
LIBDIR= $(SRCROOT)/lib


CC= gcc
CCOPTS=
DEFINES= -D$(OSTYPE)=1
INCDIRS= -I$(INCDIR)
CFLAGS= $(CCOPTS) $(DEFINES) $(INCDIRS)

AS= as
AFLAGS=

LD= $(CC)
LDFLAGS=
LIBDIRS= -L$(LIBDIR)
LIBS= -ldam -lb -luc -lu -lsocket -lnsl


# files

OBJ= main.o


.SUFFIXES:		.ls .i .cx .cs


all:		$(ALL)

.c.o:
	$(CC) -c $(CFLAGS) $<

.s.o:
	$(AS) -o $@ $<

.c~.c:
	null "want to get $@"

.s~.s:
	null "want to get $@"

.h~.h:
	null "want to get $@"


$(T):		$(OBJ) Makefile
	$(LD) -o $@ $(LDFLAGS) $(OBJ) $(LIBDIRS) $(LIBS)

strip:		$(T)
	strip $(T)

install:	$(T)
	strip $(T)
	cp $(T) $(BINDIR)

clean:
	rm -f *.o


main.o:		main.c


