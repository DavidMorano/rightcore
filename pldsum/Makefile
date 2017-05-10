# MAKEFILE 'dataioup'

T= pldsum

#SRCROOT= $(LOCAL)


LIBDIR= $(SRCROOT)/lib
BINDIR= $(SRCROOT)/bin
INCDIR= $(SRCROOT)/include


INCDIRS= -I$(INCDIR)

#CC= cc

CFLAGS= -O $(INCDIRS)

AS= as

AFLAGS=

LD= cc

LDFLAGS=

#CRT0=/lib/crt0.o
CRT0=

LIBS= -lb -lc


# default rules

.c.o:
	$(CC) $(CFLAGS) -c $<

.s.o:
	$(AS) -o $@ $<

.c~.c:
	null "want to get $@"

.s~.s:
	null "want to get $@"

.h~.h:
	null "want to get $@"


# files

OBJ= main.o


# target

$(T):	main.o Makefile
	$(LD) -o $@ $(CRT0) $(OBJ) -L$(LIBDIR) $(LIBS)

clean:
	rm -f *.o

install:	$(T)
	strip $(T)
	cp $(T) $(BINDIR)

main.o:	main.c

utty.o:	utty.c


