# MAKEFILE

T= prtref

#SRCROOT= $(LOCAL)


BINDIR= $(SRCROOT)/bin
INCDIR= $(SRCROOT)/include
LIBDIR= $(SRCROOT)/lib


#CC= cc

DEFINES= -D$(OS)=1

INCDIRS= -I$(INCDIR)

CFLAGS= -O $(DEFINES) $(INCDIRS)

LD= $(CC)

LIBDIRS= -L$(LIBDIR)

LIBS= -ldam -lb -lu

LDFLAGS=

#CRT0= /lib/crt0.o
CRT0=


ALL= prtref


all:			$(ALL)

.c.o:
	$(CC) -c $(CFLAGS) $<


$(T):			$(T).ksh
	cp -p $(T).ksh $(T)

install:		$(T).ksh
	makenewer -r -o rmsuf ${T}.ksh $(BINDIR}

strip:			$(T).x
	strip $(T).x
	rm -f $(T).$(OFF)

again:
	rm -f $(ALL) $(T).x

clean:			again
	rm -f *.o 

control:
	(uname -n ; date) > Control


