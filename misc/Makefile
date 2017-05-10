# MAKEFILE (for 'misc')

T= misc

#SRCROOT= $(LOCAL)


BINDIR= $(HOME)/bin
INCDIR= $(HOME)/include
LIBDIR= $(HOME)/lib


#CC= cc

LD= cc

#LIBS= -lmisc -lb -lc

LIBS=

CFLAGS= -O -I$(LIBDIR)/include

LDFLAGS=

#CRT0= /lib/crt0.o
CRT0=


.c.o:
	$(CC) $(CFLAGS) -c $<



ALL= ff sc


all:	$(ALL)

sc:	sc.o
	$(LD) $(LDFLAGS) -o $@ $(CRT0) sc.o $(LIBS)

ff:	ff.o
	$(LD) $(LDFLAGS) -o $@ $(CRT0) ff.o -L$(LIBDIR) $(LIBS)


install:	$(ALL)
	strip $ALL
	cp $(ALL) $(BINDIR)

clean:
	rm -f *.o 


