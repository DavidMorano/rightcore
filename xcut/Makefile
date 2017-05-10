# MAKEFILE 'xcut'

T= xcut

#SRCROOT= $(LOCAL)


XLIB= $(XDIR)/lib

BINDIR= ${SRCROOT}/bin

INCLUDES= -I$(SRCROOT)/lib/include -I$(XDIR)/include/X11

#CC= cc
CFLAGS= -O $(INCLUDES)

LIBDIRS= -L${SRCROOT}/lib -L$(XLIB)
LIBS= -lX11 -lb -lmisc


OBJ= main.o


$(T):	$(OBJ)
	$(CC) $(CFLAGS) -o $@ $(OBJ) $(LIBDIRS)

install:
	cp xcut $(BINDIR)

clean:
	rm -f *.o

lint:
	lint -uxn $(INCLUDES)


