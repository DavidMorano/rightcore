# makefile to make the custom 'more' package

T= more

#SRCROOT= $(LOCAL)


BINDIR= $(SRCROOT)/bin
LIBDIR= $(SRCROOT)/lib
INCDIR= $(SRCROOT)/include

CC= cc

INCDIRS= -I$(INCDIR)

LD= cc

LIBS= -lt -lmisc -lb

CFLAGS= $(INCDIRS)

LDFLAGS=

#CRT0= /lib/crt0.o
CRT0=


HDRS= system.h localmisc.h term.h

OBJ= more.o


$(T):	$(OBJ) Makefile
	$(LD) $(LDFLAGS) -o $@ $(CRT0) $(OBJ) -L$(LIBDIR) $(LIBS)
 
install:	$(T)
	strip $(T)
	cp $(T) $(BINDIR)

clean:
	rm -f *.o

more.o:	$(HDRS)


