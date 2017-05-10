# MAKEFILE

T= testarray

ALL= $(T)

#SRCROOT= $(LOCAL)


BINDIR= $(SRCROOT)/bin
INCDIR= $(SRCROOT)/include
LIBDIR= $(SRCROOT)/lib


#CC= cc
CC=gcc


CCOPTS=
DEFINES= -D$(OSTYPE)=1 -DSUN=1
INCDIRS= -I$(SRCROOT)/include
CFLAGS= $(CCOPTS) $(DEFINES) $(INCDIRS)

LD= $(CC)
LDFLAGS=
LIBDIRS= -L$(LIBDIR)/$(OFD)

LIBS0= -lut -lpcs -ldam -lnews
LIBS1= -lb -luc
LIBS2= -lu
LIBS3= -L$(GNU)/lib -lgcc
LIBS4= -lgen -lpthread -lrt -lsocket -lnsl -lc

LIBS= $(LIBS0) $(LIBS1) $(LIBS2) $(LIBS3) $(LIBS4)



#CRT0= /lib/crt0.o
CRT0=



OBJ= main.o



all:		$(ALL)

.c.o:
	$(CC) -c $(CFLAGS) $<


$(T):		$(OBJ) Makefile
	$(LD) -o $@ $(LDFLAGS) $(OBJ) $(LIBDIRS) $(LIBS)


clean:		again
	rm -f *.o

again:
	rm -f $(ALL) $(T) $(T).x

lint:		main.c
	lint -uxn $(DEFINES) $(INCDIRS) main.c


main.o:		main.c




