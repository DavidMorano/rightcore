# MAKEFILE

T= mmpost

ALL= $(T)

#PCS= $(HW)/add-on/pcs


BINDIR= $(PCS)/bin
LIBDIR= $(PCS)/lib
INCDIR= $(PCS)/include

CC= gcc
CCOPTS= -O
DEFINES=
INCDIRS= -I$(INCDIR)
CFLAGS= $(CCOPTS) $(DEFINES) $(INCDIRS)

LD= $(CC)
LDFLAGS=
LIBDIRS= -L$(LIBDIR)
LIBS= -lpcs -ldam -lb -lu

#CRT0= /lib/crt0.o
CRT0=



OBJ= main.o whatinfo.o




default:	$(T)

all:		$(ALL)

.c.o:
	$(CC) -c $(CFLAGS) $<


$(T):		$(OBJ) Makefile
	$(LD) -o $@ $(LDFLAGS) $(CRT0) $(OBJ) $(LIBDIRS) $(LIBS)

strip:		$(T)
	strip $(T)

install:	$(T)
	strip $(T)
	install $(T) $(BINDIR)

again:
	rm -f $(ALL) $(T).x

clean:
	rm -f *.o



