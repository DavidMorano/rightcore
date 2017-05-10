# MAKEFILE

T= cdaudio

SRCROOT= $(LOCAL)

BINDIR= $(SRCROOT)/bin
INCDIR= $(SRCROOT)/include
LIBDIR= $(SRCROOT)/lib/$(OFD)


CC= gcc
CCOPTS= -O3 -mcpu=ultrasparc # -fpic

INCDIRS= -I$(SRCROOT)/include
DEFINES= -D$(OS)=1
CFLAGS= $(CCOPTS) $(DEFINES) $(INCDIRS)

LD= $(CC)
#LD= ld
LDFLAGS=
LIBDIRS= -L$(LIBDIR)
LIBS= -ldam -lb -lu


#CRT0= /lib/crt0.o
CRT0=



OBJ= main.o version.o expandline.o



all:		$(T).x $(T)

.c.o:
	$(CC) -c $(CFLAGS) $<


$(T):		$(T).sh
	cp $(T).sh $(T)

$(T).x:		$(OBJ) Makefile
	$(LD) -o $@ $(LDFLAGS) $(CRT0) $(OBJ) $(LIBDIRS) $(LIBS)

strip:		$(T).x
	strip $(T).x

install:	$(T).x
	strip $(T).x
	bsdinstall $(T).x $(BINDIR)

install.$(OFF):	$(T).$(OFF)
	strip $(T).$(OFF)
	bsdinstall $(T).$(OFF) $(BINDIR)

$(T).$(OFF) $(OFF):	$(T).x
	cp $(T).x $(T).$(OFF)
	strip $(T).$(OFF)

clean:
	rm -f *.o

lint:		main.c
	lint -uxn $(DEFINES) $(INCDIRS) main.c


main.o:		main.c config.h defs.h

version.o:	version.c config.h



