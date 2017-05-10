# MAKEFILE

T= rarpd

#SRCROOT= $(LOCAL)


BINDIR= $(NCMP)/bin
LIBDIR= $(NCMP)/lib
INCDIR= $(NCMP)/include


C++BIN= /usr/add-on/c++new/bin
CC= $(C++BIN)/usr/add-on/c++new/bin
LD= ld
LIBS= -lb -lmisc -lc

CFLAGS= -O -I $(INCDIR)
LDFLAGS=

CRT0= /lib/crt0.o


.c.o:
	$(CC) $(CFLAGS) -c $<


$(T):		mroff.o
	$(LD) $(LDFLAGS) -o $@ $(CRT0) mroff.o -L$(LIBDIR) $(LIBS)

install:	$(T)
	strip $(T)
	cp $(T) $(BINDIR)

clean:
	rm -f *.o 



