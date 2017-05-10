# MAKEFILE

T= pdfclean

#SRCROOT= $(LOCAL)


BINDIR= $(SRCROOT)/bin
INCDIR= $(SRCROOT)/include
LIBDIR= $(SRCROOT)/lib/$(OFD)


CC= gcc

CCOPTS= -O

DEFINES= -D$(OS)=1

INCDIRS= -I$(INCDIR)

CFLAGS= $(CCOPTS) $(DEFINES) $(INCDIRS)

LD= $(CC)

LDFLAGS=

LIBDIRS= -L$(LIBDIR)

LIBS= -ldam -lb -lu

#CRT0= /lib/crt0.o
CRT0=


OBJ= main.o

INCS= config.h defs.h


ALL= $(T) $(T).x



all:		$(ALL)

.c.o:
	$(CC) -c $(CFLAGS) $<


$(T):		$(T).sh
	cp $(T).sh $(T)

$(T).x:		$(OBJ) Makefile
	$(LD) $(LDFLAGS) -o $@ $(CRT0) $(OBJ) $(LIBDIRS) $(LIBS)

$(T).elf elf:	$(T).x
	cp $(T).x $(T).elf

$(T).aout aout:	$(T).x
	cp $(T).x $(T).elf

strip:		$(T).x
	strip $(T).x

install:	$(T).x
	strip $(T).x
	cp $(T).x $(BINDIR)
	cp $(T) $(BINDIR)

clean:
	rm -f *.o



main.o:		main.c $(INCS)




