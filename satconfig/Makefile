# MAKEFILE

T= rsl

#SRCROOT= $(LOCAL)


LIBDIR= $(SRCROOT)/lib
BINDIR= $(SRCROOT)/bin
INCDIR= $(SRCROOT)/include


LIBS= -lb -lmisc -lc

#CC= cc
LD= cc

CFLAGS= -O -I $(INCDIR)
LDFLAGS=

#CRT0= /lib/crt0.o
CRT0=


.SUFFIXES:	.x


.c.o:
	$(CC) $(CFLAGS) -c $<

.c.x:
	lint -uxn -I $(LIBDIR)/include $<


OBJ= main.o quoted.o expand.o


$(T):	$(OBJ) Makefile
	$(LD) $(LDFLAGS) -o $@ $(CRT0) $(OBJ) -L$(LIBDIR) $(LIBS)

install:	$(T)
	strip $(T)
	cp $(T) $(BINDIR)
 
clean:
	rm -f *.o

control:
	date > Control
	uname >> Control


tp.o:	tp.c


tp:	tp.o
	$(LD) $(LDFLAGS) -o $@ $(CRT0) tp.o -L$(LIBDIR) $(LIBS)


main.o:		main.c satconfig.h

quoted.o:	quoted.c satconfig.h

expand.o:	expand.c satconfig.h


main.x:		main.c

readconfig.x:	readconfig.c

quoted.x:	quoted.c

exapnd.x:	expand.c


