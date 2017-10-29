# MAKEFILE

T= rsx

LIBDIR= $(SRCROOT)/lib
BINDIR= $(SRCROOT)/bin
INCDIR= $(SRCROOT)/include

INCDIRS= -I$(INCDIR)

#CC= cc
LD= cc
LIBS= -lmisc -lb -lc

CFLAGS= -O $(INCDIRS)
LDFLAGS=

#CRT0= /lib/crt0.o
CRT0=

OBJ= main.o


.c.o:
	$(CC) $(CFLAGS) -c $<


$(T):	$(OBJ) Makefile
	$(LD) $(LDFLAGS) -o $@ $(CRT0) $(OBJ) -L$(LIBDIR) $(LIBS)

strip:		$(T)
	strip $(T)

install:	$(T)
	strip $(T)
	cp $(T) $(BINDIR)

clean:
	rm -f *.o


