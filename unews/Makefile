# MAKEFILE

T= unews

ALL= $(T)

SRCROOT= $(LOCAL)


BINDIR= $(SRCROOT)/bin
SBINDIR= $(POSTFIX)/sbin


all:		$(ALL)

$(T):		$(T).ee
	rm -f $(T)
	cp -p $(T).ee $(T)

install:	$(ALL)
	makenewer -r $(ALL) $(BINDIR)

install-raw:	$(T).ksh
	rm -f $(T)
	makecp $(T).ksh $(T)
	rm -f $(BINDIR)/$(T).ksh
	rm -f $(BINDIR)/$(T).$(OFF)
	makenewer -r $(T) $(BINDIR)
	
clean:
	rm -f $(T)


