# MAKEFILE
#	@(#)makefile	3.4	3/23/87


T= pcnfsd

INCDIR= $(SRCROOT)/include
LIBDIR= $(SRCROOT)/lib/$(OFD)
BINDIR= $(SRCROOT)/bin


#CC= cc
INCDIRS= -I$(INCDIR)
DEFINES= -DSYSV=1 -DSVR4=1 -DSOLARIS=1
CFLAGS= -O $(DEFINES) $(INCDIRS)

LD= $(CC)
LDFLAGS=
LIBDIRS= -L$(LIBDIR)
LIBS= -lmisc -lb -lnsl


OBJ= main.o opt_nmatch.o logfile.o


all:		$(T)

$(T):		$(OBJ) Makefile
	$(LD) -o $@ $(LDFLAGS) $(OBJ) $(LIBDIRS) $(LIBS)

strip:		$(T)
	strip $(T)

lint:		main.c
	lint -uxn main.c

clean:
	rm -f $(T) *.o



main.o:		main.c config.h
	$(CC) -c $(CFLAGS) main.c



