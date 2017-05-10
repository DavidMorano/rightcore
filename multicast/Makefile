# MAKEFILE

T= multicast

ALL= $(T) $(T).x

#SRCROOT= /usr/add-on/pcs


BINDIR= $(SRCROOT)/bin
INCDIR= $(SRCROOT)/include
LIBDIR= $(SRCROOT)/lib/$(OFD)


#CC= gcc

DEFINES= -D$(OS)=1

INCDIRS= -I$(INCDIR)

CFLAGS= -O $(DEFINES) $(INCDIRS)

LD= $(CC)

LDFLAGS=

LIBPATHS= -L$(LIBDIR)

LIBS= -ldam -lb -luc -lu -lgen -lrt2 -lpthread -lsocket -lnsl

#CRT0= /lib/crt0.o
CRT0=


.SUFFIXES:	.t


.c.o:
	$(CC) -c $(CFLAGS) $<

.c.t:
	lint -uxn $(DEFINES) $(INCDIRS) $<


OBJ1= main.o configfile.o srvfile.o watch.o job.o
OBJ2= debug.o

OBJ= $(OBJ1) $(OBJ2)



all:		$(ALL)

$(T):		$(T).sh
	cp $(T).sh $(T)

$(T).x:		$(OBJ) Makefile
	$(LD) $(LDFLAGS) -o $@ $(CRT0) $(OBJ) -L$(LIBDIR) $(LIBS)

install:	$(ALL)
	strip $(T).x
	bsdinstall $(ALL) $(BINDIR)
 
clean:
	rm -f *.o

control:
	uname -n >> Control
	date > Control


tp.o:		tp.c


tp:		tp.o
	$(LD) -o $@ $(LDFLAGS) $(CRT0) tp.o -L$(LIBDIR) $(LIBS)



main.o:		main.c configfile.h srvfile.h
	$(CC) -c -DPROGRAMROOT=\"${SRCROOT}\" $(CFLAGS) $<



main.t:		main.c

watch.t:	watch.c

srvfile.t:	srvfile.c

configfile.t:	configfile.c

quoted.t:	quoted.c

exapnd.t:	expand.c



