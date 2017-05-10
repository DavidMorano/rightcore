# MAKEFILE

T= lterm

#SRCROOT= $(LOCAL)


CC= cc

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

OBJ = main.o utty.o



# default rules

default:		$(T)

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


# target

$(T):			main.o utty.o $(LIB)
	ld -o $@ $(CRT0) $(OBJ) -L$(LIBDIR) $(LIBS)

clean:
	rm -f *.o $(T)



main.o:			main.c

utty.o:			utty.c



