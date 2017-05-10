# MAKEFILE (starlan stuff)

#ALL= slsend slget pcconv
ALL= pcconv

#SRCROOT= $(LOCAL)


BINDIR= $(HOME)/bin
LIBDIR= $(HOME)/lib
INCDIR= $(NCMP)/include


LIBS= -lb -lc
CRT0= /lib/crt0.o


# stuff

#CC= cc
CFLAGS= -O -I $(INCDIR)

AS= as
AFLAGS=

LD= ld
LDFLAGS=


# default rules

.c.o:
	$(CC) $(CFLAGS) -c $<

.s.o:
	$(AS) -o $@ $<

.c~.c:
	null "want to get $@"

.s~.s:
	null "want to get $@"

.h~.h:
	null "want to get $@"


# files

OBJ= main.o


# targets

all:	$(ALL)

slsend:	slend.o
	$(LD) -o $@ $(CRT0) slsend.o -L$(LIBDIR) $(LIBS)

pcconv:	pcconv.o
	$(LD) -o $@ $(CRT0) pcconv.o -L$(LIBDIR) $(LIBS)

install:	$(ALL)
	strip pcconv
	echo "can't install here"
#	cp $(ALL) $(BIN)
#	cp slget.sh $(BIN)/slget
#	cp slsend.sh $(BIN)/slsend

clean:
	rm -f *.o


