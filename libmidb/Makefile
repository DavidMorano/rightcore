# MAKEFILE

T= libmidb

#SRCROOT= $(LOCAL)


INCDIR= $(SRCROOT)/include
LIBDIR= $(SRCROOT)/lib/$(OFD)


# stuff

#CC= cc

#CCOPTS= -O3 -fpic -mcpu=ultrasparc -mno-app-regs

INCDIRS= -I$(INCDIR)

CFLAGS= -D$(OS)=1 $(INCDIRS)


LD= ld

LDFLAGS=

LIBPATHS= -L$(LIBDIR)

LIBS= -lmisc




.c.o:	midb.h
	$(CC) $(CFLAGS) -c $<
	ar -r $(T) $@

.c~.c:
	null "want to get $@"

.s~.s:
	null "want to get $@"

.h~.h:
	null "want to get $@"


# files

OBJ0= bopen.o bclose.o bseek.o bflush.o  bflush_i.o
OBJ1= bread.o bwrite.o bgetline.o bgetc.o bputc.o
OBJ2= bprintf.o format.o

OBJ= $(OBJ0) $(OBJ1) $(OBJ2)



$(T).a:		$(OBJ) Makefile

install:	$(T).a
	bsdinstall $(T).a $(LIBDIR)
	bsdinstall midb.h $(INCDIR)

clean:
	rm -f *.o $(T).a



