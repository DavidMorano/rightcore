# MAKEFILE

T= networks

#SRCROOT= $(LOCAL)


CC= gcc

CCOPTS= -g


CFLAGS= $(CCOPTS)

CPPFLAGS=

LIBDIRS=

#LIBS=
LIBS= -lsocket -lnsl


default:	$(T)

$(T):		main.o
	$(CC) -o $@ main.o $(LIBDIRS) $(LIBS)

clean:
	rm -f $(T) *.o


main.o:		main.c
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $<


