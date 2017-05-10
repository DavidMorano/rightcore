# MAKEFILE (systatd)

T= systatd

SRCROOT= $(LOCAL)


BINDIR= $(SRCROOT)/bin
SBINDIR= $(SRCROOT)/sbin
HELPDIR= $(SRCROOT)/share/help


$(T):		$(T).ksh
	rm -f $(T)
	ln -f $(T).ksh $(T)

install:	install-raw install-help

install-raw:	$(T).ksh
	rm -f $(BINDIR)/$(T).ksh
	rm -f $(BINDIR)/$(T).$(OFF)
	makenewer -r -o rmsuf $(T).ksh $(SBINDIR)

install-help:	$(T).help
	makenewer -r -o rmsuf $(T).help $(HELPDIR)

clean:
	rm -f $(T)


