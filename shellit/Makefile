# MAKEFILE

T= shellit

#SRCROOT= $(LOCAL)


LIBDIR= $(SRCROOT)/lib
BINDIR= $(SRCROOT)/bin


#CC= cc
LD= cc
LIBS= -lb -lc

INCDIRS= -I$(SRCROOT)/include

CFLAGS= -O $(INCDIRS)
LDFLAGS=

CRT0=
#CRT0= /lib/crt0.o


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


