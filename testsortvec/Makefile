# MAKEFILE

T= testsortvec

ALL= $(T).x

#SRCROOT= /usr/add-on/pcs


BINDIR= $(SRCROOT)/bin
INCDIR= $(SRCROOT)/include
LIBDIR= $(SRCROOT)/lib


#CC= gcc
CCOPTS=
DEFINES= -D$(OS)=1
INCDIRS= -I$(INCDIR)
CFLAGS= $(CCOPTS) $(DEFINES) $(INCDIRS)

LD= $(CC)
LDFLAGS= -R$(LIBDIR)/$(OFD):$(LIBDIR)
LIBDIRS= -L$(LIBDIR)/$(OFD)
LIBS= -ldam -lb -luc -lu -lgen -lsocket -lnsl

#CRT0= /lib/crt0.o
CRT0=



OBJ0= main.o
OBJ1= sortvec.o
OBJ2=
OBJ= $(OBJ0) $(OBJ1) $(OBJ2)




.SUFFIXES:	.t



default:		$(T).x

all:			$(ALL)

.c.o:
	$(CC) -c $(CFLAGS) $<

.c.t:
	lint -uxn $(DEFINES) $(INCDIRS) $<


$(T):			$(T).sh
	cp -p $(T).sh $(T)

$(T).x:			$(OBJ) Makefile
	$(LD) -o $@ $(LDFLAGS) $(CRT0) $(OBJ) $(LIBDIRS) $(LIBS)

$(T).$(OFF) $(OFF):	$(T).x
	cp -p $(T).x $(T).$(OFF)

strip:			$(T).x
	strip $(T).x
	rm -f $(T) $(T).$(OFF)

again:
	rm -f $(ALL) $(T) $(T).x

clean:			again
	rm -f *.o

control:
	uname -n > Control
	date >> Control



main.o:		main.c localmisc.h sortvec.h

sortvec.o:	sortvec.c sortvec.h






