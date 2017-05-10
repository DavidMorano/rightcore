# MAKEFILE (backdisk)

T= backdisk

ALL= $(T)

SRCROOT= $(EXTRA)


BINDIR= $(SRCROOT)/bin
SBINDIR= $(SRCROOT)/sbin


all:		$(ALL)

$(T):		$(T).ksh
	rm -f $(T)
	cp -p $(T).ksh $(T)

install:	install-raw

install-raw:	$(T).ksh
	ln -f ${T}.ksh ${T}
	rm -f $(BINDIR)/$(T).ksh
	rm -f $(BINDIR)/$(T).$(OFF)
	makeinstall

clean:
	rm -f $(T)


