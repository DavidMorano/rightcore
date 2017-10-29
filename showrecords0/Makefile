# MAKEFILE

T= showrecords

#SRCROOT= $(LOCAL)


BINDIR= $(SRCROOT)/bin
INCDIR= $(SRCROOT)/include
LIBDIR= $(SRCROOT)/lib/$(OFD)


DEFINES= -D$(OS)=1

#CC= cc
INCDIRS= -I$(INCDIR)
CFLAGS= -O $(DEFINES) $(INCDIRS)

LD= $(CC)
LDFLAGS=
LIBDIRS= -L$(LIBDIR)
LIBS= -lmisc -lb


#CRT0= /lib/crt0.o


.c.o:
	$(CC) $(CFLAGS) -c $<


OBJ= main.o



$(T):		$(OBJ) Makefile
	$(LD) -o $@ $(LDFLAGS) $(CRT0) $(OBJ) $(LIBDIRS) $(LIBS)

$(T).elf elf:	$(T)
	strip $(T)
	rm -f $(T).elf
	ln $(T) $(T).elf

strip:		$(T)
	strip $(T)

install:	$(T)
	strip $(T)
	cp $(T) $(BINDIR)

clean:
	rm -f *.o


main.o:		main.c defs.h config.h



