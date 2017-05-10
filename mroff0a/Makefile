# MAKEFILE
# check the definition for BASE for compatibility with your setup

T= mroff

SRCROOT= $(LOCAL)

BINDIR= $(SRCROOT)/bin
INCDIR= $(SRCROOT)/include
LIBDIR= $(SRCROOT)/lib/$(OFD)


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


ALL= mroff prtref

OBJ= main.o


all:		$(ALL)

.c.o:
	$(CC) -c $(CFLAGS) $<


prtref:		prtref.sh
	cp prtref.sh prtref

$(T):		$(OBJ)
	$(LD) -o $@ $(LDFLAGS) $(CRT0) $(OBJ) $(LIBDIRS) $(LIBS)

install:	$(ALL)
	strip $(T)
	cp $(ALL) $(BINDIR)

strip:		$(T)
	strip $(T)

clean:
	rm -f *.o 

control:
	(uname -n ; date) > Control


