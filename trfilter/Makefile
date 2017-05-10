# MAKEFILE (trfilter)

T= trfilter

SRCROOT= $(LOCAL)


LIBDIR= $(SRCROOT)/lib
BINDIR= $(SRCROOT)/bin
INCDIR= $(SRCROOT)/include


#CC= cc
LD= cc
LIBS= -lmisc -lb -lc

CFLAGS= -O -I$(INCDIR)
LDFLAGS=

#CRT0= /lib/crt0.o
CRT0=


.c.o:
	$(CC) $(CFLAGS) -c $<


OBJ= main.o


$(T):	$(OBJ) Makefile
	$(LD) $(LDFLAGS) -o $@ $(CRT0) $(OBJ) -L$(LIBDIR) $(LIBS)

strip:		$(T)
	strip $(T)

install:	$(T)
	strip $(T)
	cp $(T) $(BINDIR)

clean:
	rm -f *.o


