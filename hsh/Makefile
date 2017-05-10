# MAKEFILE

T= hsh

ALL= $(T)

#SRCROOT=


BINDIR= $(SRCROOT)/bin
INCDIR= $(SRCROOT)/include
LIBDIR= $(SRCROOT)/lib


# stuff

#CC= cc
CFLAGS= -I$(INCDIR)

AS= as
AFLAGS=

LD= cc
LDFLAGS=
LIBDIRS= -L$(LIBDIR)

LIBS= -ldam -lb -lut -luc -lu -lc




# files


OBJ = main.o cmd.o editline.o





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



$(T):			$(OBJ) Makefile
	$(CC) -o $@ $(LDFLAGS) $(OBJ) $(LIBDIRS) $(LIBS)

strip:			$(T)
	strip $(T)

intstall:		$(T)
	bsdinstall $(T) $(BINDIR)

clean:
	rm -f *.o



main.o:		main.c main.h

cmd.o:		cmd.c

editline.o:	editline.c



