# MAKEFILE

T= fixmail

SRCROOT= ${NCMP}


LIBDIR= $(SRCROOT)/lib
BINDIR= /usr/adm/bin


CC= cc

LD= ld

LIBS= -lmisc -lb -lc

CFLAGS= -O -I $(LIBDIR)/include

LDFLAGS=

CRT= /lib/crt0.o


OBJ= dmail.o

.c.o:
	$(CC) $(CFLAGS) -c $<


$(T):	$(OBJ)
	$(LD) $(LDFLAGS) -o $@ $(CRT) $(OBJ) -L$(LIBDIR) $(LIBS)

strip:		$(T)
	strip $(T)

install:	$(T)
	strip $(T)
	cp $(T) $(BINDIR)

clean:
	rm -f *.o


