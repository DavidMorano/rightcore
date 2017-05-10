# MAKEFILE

T= postdeliver

ALL= $(T)

SRCROOT= $(LOCAL)


BINDIR= $(SRCROOT)/bin
SBINDIR= $(POSTFIX)/sbin


all:		$(ALL)

$(T):		$(T).ksh
	rm -f $(T)
	ln -f $(T).ksh $(T)

install:	$(T).ksh
	makenewer -r -o rmsuf $(T).ksh $(SBINDIR)

clean:
	rm -f $(T)


