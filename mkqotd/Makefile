# MAKEFILE (mkqotd)

T= mkqotd

SRCROOT= $(LOCAL)


BINDIR= $(SRCROOT)/bin
SBINDIR= $(SRCROOT)/sbin


$(T):		$(T).ksh
	rm -f $(T)
	ln -f $(T).ksh $(T)

install:	install-sbin

install-bin:	$(T).ksh
	rm -f $(BINDIR)/$(T).ksh
	rm -f $(BINDIR)/$(T).$(OFF)
	makenewer -r -o rmsuf $(T).ksh $(BINDIR)

install-sbin:	$(T).ksh
	makenewer -r -o rmsuf $(T).ksh $(SBINDIR)

clean:
	rm -f $(T)


