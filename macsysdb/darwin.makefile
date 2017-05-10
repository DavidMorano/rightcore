# MAKEFILE

T= sysdb



CC= gcc

CCOPTS= -O


CFLAGS= $(CCOPTS)

CPPFLAGS=

LIBDIRS=

LIBS=
#LIBS= -lsocket -lnsl



$(T):		main.o
	$(CC) -o $@ main.o $(LIBDIRS) $(LIBS)



main.o:		main.c
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $<




