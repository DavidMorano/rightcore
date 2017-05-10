# MAKEFILE

T= fortune-info
 
ALL= $(T) $(T).$(OFD)

SRCROOT= $(NCMP)
#SRCROOT= /usr/add-on/pcs


BINDIR= $(SRCROOT)/bin
INCDIR= $(SRCROOT)/include
LIBDIR= $(SRCROOT)/lib


CC= gcc

CCOPTS= -O3 -mcpu=ultrasparc # -fpic
#CCOPTS= -xO5 -xtarget=ss20/hs22 -dalign -xdepend

DEFINES= -D$(OS)=1 -DSOLARIS=1 -DPOSIX=1 -DPROGRAMROOT=\"$(SRCROOT)\"

INCDIRS= -I$(INCDIR)

CFLAGS= $(CCOPTS) $(DEFINES) $(INCDIRS)

LD= $(CC)

LDFLAGS= -R$(LIBDIR)/$(OFD):$(LIBDIR)

LIBDIRS= -L$(LIBDIR)/$(OFD) -L$(LIBDIR)

LIBS= -ldam -lb -luc -lu -lsocket -lnsl

#CRT0= /lib/crt0.o
CRT0=
CRT1= makedate.o


OBJ1= main.o version.o procfile.o
OBJ2=
OBJ3=
OBJ4=
OBJ= $(OBJ1) $(OBJ2) $(OBJ3) $(OBJ4)




all:			$(ALL)

.c.o:
	$(CC) -c $(CFLAGS) $<


$(T):			$(T).sh
	cp $(T).sh $(T)

$(T).x:			$(OBJ) Makefile
	makedate > makedate.c
	$(CC) -c $(CFLAGS) makedate.c
	$(LD) -o $@ $(LDFLAGS) $(CRT0) $(CRT1) $(OBJ) $(LIBDIRS) $(LIBS)
	touch compiletime.o

$(T).$(OFF) $(OFF):	$(T).x
	strip $(T).x
	cp $(T).x $(T).$(OFF)

install:		$(T) install.x
	bsdinstall $(T) $(T).x $(BINDIR)

install.$(OFF):		$(T).$(OFF)
	bsdinstall $(T).$(OFF) $(BINDIR)
	chmod u+s $(BINDIR)/$(T).$(OFF)

install.x:		$(T).x
	strip $(T).x
	bsdinstall $(T).x $(BINDIR)
	chmod u+s $(BINDIR)/$(T).x

strip:			$(T).x
	strip $(T).x

clean:
	rm -f *.o $(T).x

control:
	uname -n > Control
	date >> Control

again:
	rm -f $(ALL)


compiletime.c:
	touch compiletime.c





