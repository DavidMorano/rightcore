# MAKEFILE

T= eas

ALL= $(T) $(T).$(OFF)


BINDIR= $(SRCROOT)/bin
INCDIR= $(SRCROOT)/include
LIBDIR= $(SRCROOT)/lib


CC= gcc
CCOPTS= -O
DEFINES= -D$(OS)=1
INCDIRS= -I$(INCDIR)
CFLAGS= $(CCOPTS) $(DEFINES) $(INCDIRS)

LD= $(CC)
#LDFLAGS= -R$(LIBDIR)/$(OFD):$(LIBDIR)
LDFLAGS=
LIBDIRS= -L$(LIBDIR)/$(OFD)
LIBS= -ldam -lb -luc -lu -lsocket -lnsl

#CRT0= /lib/crt0.o
CRT0=
CRTC=
CRTN=


ALL = $(T).x $(T)

OBJ= main.o


OBJS= $(CRT0) $(OBJ) $(CRTC)



all:		$(ALL)

.c.o:
	$(CC) -c $(CFLAGS) $<

.l.c:
	echo "sorry"

.y.c:
	echo "sorry"


$(T):		$(T).sh
	cp $(T).sh $(T)

$(T).x:		$(OBJ) Makefile
	$(LD) -o $@ $(LDFLAGS) $(OBJS) $(LIBDIRS) $(LIBS) $(CRTN)

strip:		$(T).x
	strip $(T).x

$(T).$(OFF) $(OFF):	$(T).x
	strip $(T).x
	cp $(T).x $(T).$(OFF)

again:
	rm -f $(ALL)

install:	$(T).x
	bsdinstall $(ALL) $(BINDIR)
 
clean:
	rm -f *.o

lint:		main.c
	lint -uxn $(INCDIRS) main.c
	touch lint

control:
	uname -n > Control
	date >> Control



