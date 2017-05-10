# MAKEFILE

T= cplus
 

#SRCROOT= $(PCS)
#SRCROOT= /usr/add-on/pcs

BINDIR= $(SRCROOT)/bin
INCDIR= $(SRCROOT)/include
LIBDIR= $(SRCROOT)/lib


CC= g++

DEFINES= -D$(OS)=1 -DOS=\"$(OS)\" -DPROGRAMROOT=\"$(SRCROOT)\"

INCDIRS= -I$(INCDIR)

CFLAGS= -O $(DEFINES) $(INCDIRS)

LD= $(CC)

LDFLAGS=

LIBDIRS= -L$(LIBDIR)

LIBS= -lb -lu

#CRT0= /lib/crt0.o


OBJ1= main.o 
OBJ2= 
OBJ3= 
OBJ4= 

OBJ= $(OBJ1) $(OBJ2) $(OBJ3) $(OBJ4)


ALL= $(T) $(T).$(OFF)


all:		$(ALL)

.c.o:
	$(CC) -c $(CFLAGS) $<


$(T):		$(T).sh
	cp $(T).sh $(T)

$(T).x:		$(OBJ) Makefile
	compiletime > compiletime.c
	$(CC) -c $(CFLAGS) compiletime.c
	$(LD) -o $@ $(LDFLAGS) $(CRT0) $(OBJ) compiletime.o $(LIBDIRS) $(LIBS)

$(T).$(OFF) $(OFF):	$(T).x
	strip $(T).x
	cp $(T).x $(T).$(OFF)

install:	$(T) install.x
	bsdinstall $(T) $(BINDIR)

install.$(OFF):	$(T).$(OFF)
	bsdinstall $(T).$(OFF) $(BINDIR)
	chmod u+s $(BINDIR)/$(T).$(OFF)

install.x:	$(T).x
	strip $(T).x
	bsdinstall $(T).x $(BINDIR)
	chmod u+s $(BINDIR)/$(T).x

strip:		$(T).x
	strip $(T).x

again:
	rm -f $(T).x

clean:
	rm -f *.o $(T).x

control:
	uname -n > Control
	date >> Control


main.o:		main.cc
	$(CC) -c $(CFLAGS) $<



