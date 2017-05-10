# MAKEFILE

T= parsedate
 
SRCROOT= $(PCS)


BINDIR= $(SRCROOT)/bin
INCDIR= $(SRCROOT)/include
LIBDIR= $(SRCROOT)/lib/$(OFD)


#CC= gcc

DEFINES= -D$(OS)=1

INCDIRS= -I$(INCDIR)

CFLAGS= $(DEFINES) $(INCDIRS)

LD= $(CC)

LDFLAGS=

LIBDIRS= -L$(LIBDIR)

LIBS= -lmisc -lb -lnews -lu -lc -lac

#CRT0= /lib/crt0.o


INC= incfile_datemsk.h

OBJ1= main.o
OBJ2= 
OBJ3=
OBJ= $(OBJ1) $(OBJ2) $(OBJ3)


ALL= $(T) $(T).x



.c.o:			$(INC)
	$(CC) -c $(CFLAGS) $<


all:			$(ALL)

$(T):			$(T).sh
	cp $(T).sh $(T)

$(T).x:			$(OBJ) Makefile
	$(LD) -o $@ $(LDFLAGS) $(CRT0) $(OBJ) $(LIBDIRS) $(LIBS)

$(T).$(OFF) $(OFF):	$(T).x
	strip $(T).x
	cp $(T).x $(T).$(OFF)

strip:			$(T).x
	strip $(T).x

install:		$(ALL) install.x

install.x:		$(ALL)
	bsdinstall $(T) $(T).x $(BINDIR)

install.$(OFF):		$(T).$(OFF)
	bsdinstall $(T) $(T).$(OFF) $(BINDIR)

clean:
	rm -f *.o $(T).x

control:
	uname -n > Control
	date >> Control


main.o:			main.c $(INC)

incfile_datemsk.h:	datemsk
	mkincfile datemsk



