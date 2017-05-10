# MAKEFILE

T= toolman-prt

#SRCROOT= $(LOCAL)


LIBDIR= $(SRCROOT)/lib
BINDIR= $(SRCROOT)/bin
INCDIR= $(SRCROOT)/include


INCDIRS= -I$(INCDIR)

#CC= cc

CFLAGS= -O $(INCDIRS)

LD= $(CC)

LDFLAGS=

LIBS= -lmisc -lb

#CRT0= /lib/crt0.o
CRT0=


OBJ= main.o


.c.o:
	$(CC) $(CFLAGS) -c $<


all:		$(T)

$(T):		$(T).sh
	cp $(T).sh $(T)

install:	$(T)
	bsdinstall $(T) $(BINDIR)

clean:
	rm -f *.o


