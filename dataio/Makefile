# MAKEFILE

T= dataioup


CC= gcc

CFLAGS=

AS= as

AFLAGS=

LD= ld

LDFLAGS=

CRT0= /lib/crt0.o

LIBDIR= $(HOME)/lib

LIB= $(LIBDIR)/libb.a

LIBS= -lb -lc




# files

OBJ= main.o utty.o


# target

default:	$(T)

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


dataioup:	main.o utty.o $(LIB)
	ld -o $@ $(CRT0) $(OBJ) -L$(LIBDIR) $(LIBS)

clean:
	rm -f $(T)


main.o:	main.c

utty.o:	utty.c


