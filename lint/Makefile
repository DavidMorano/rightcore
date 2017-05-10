# MAKEFILE (lint)

T= lint


SRCROOT= $(NCMP)


BINDIR= $(SRCROOT)/bin
SBINDIR= $(SRCROOT)/sbin


$(T):		$(T).ksh
	rm -f $(T)
	ln -f $(T).ksh $(T)

install:	$(T).ksh
	rm -f $(SBINDIR)/$(T).ksh
	rm -f $(SBINDIR)/$(T).$(OFF)
	makenewer -r -o rmsuf $(T).ksh $(BINDIR)

clean:
	rm -f $(T)



