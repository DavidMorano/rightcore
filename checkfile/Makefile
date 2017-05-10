# MAKEFILE

ALL= checkfile reads checkdata

BINDIR= $(SRCROOT)/bin
INCDIR= $(SRCROOT)/include
LIBDIR= $(SRCROOT)/lib


CC= gcc

CCOPTS= -O

DEFINES= -D$(OS)=1
INCDIRS= -I$(INCDIR)
CFLAGS= $(CCOPTS) $(DEFINES) $(INCDIRS)

#AS= as
AFLAGS=

LD= $(CC)
LDFLAGS=
LIBDIRS= -L$(LIBDIR)
LIBS= -ldam -lb2 -lu



# files

OBJ= checkfile.o

LIB=


# target

all:	$(ALL)

.c.o:
	$(CC) -c $(CFLAGS) $<

.s.o:
	$(AS) -o $@ $<

.c~.c:
	null "want to get $@"

.s~.s:
	null "want to get $@"

.h~.h:
	null "want to get $@"


checkfile:	$(OBJ)
	$(LD) -o $@ $(OBJ) $(LIBDIRS) $(LIBS)

reads:		reads.o 
	$(LD) -o $@ reads.o $(LIBDIRS) $(LIBS)

checkdata:	checkdata.o $(LIB)
	$(LD) -o $@ checkdata.o $(LIBDIRS) $(LIBS)

clean:
	rm -f $(OBJ) $(ALL)


checkfile.o:	checkfile.c




