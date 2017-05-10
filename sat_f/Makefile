# MAKEFILE

ALL= sat_fe sat_fi

#SRCROOT= $(LOCAL)


LIBDIR= $(HOME)/lib4
BINDIR= $(HOME)/bin4


CC= cc

CFLAGS= -O -I${LIBDIR}/include

LDFLAGS=


CRT0= /lib/crt0.o

LIBS= -lb -lmisc -lc


.c.o:	sat_f.h
	$(CC) $(CFLAGS) -c $<


all:	$(ALL)


OBJA= sat_fe.o

sat_fe:	$(OBJA) Makefile
	$(LD) -o $@ $(LDFLAGS) $(CRT0) $(OBJA) -L$(LIBDIR) $(LIBS)

OBJB= sat_fi.o


sat_fi:	$(OBJB) Makefile
	$(LD) $(LDLAGS) -o $@ $(CRT0) $(OBJB) -L$(LIBDIR) $(LIBS)

strip:		$(ALL)
	strip $(ALL)

install:	$(ALL)
	strip $(ALL)
	cp $(ALL) $(BINDIR)
 
clean:
	rm -f *.o

control:
	date '+%m/%d/%y %T' > Control
	uname >> Control

deliver:
	strip $(ALL)
	cp $(ALL) ${HOME}/SUNSat/bin


