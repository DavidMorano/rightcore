# MAKEFILE

T= looker

ALL= $(T) $(T).ksh

SRCROOT= $(LOCAL)


BINDIR= $(SRCROOT)/bin


all:		$(ALL)

$(T):		$(T).ee
	rm -f $(T)
	ln $(T).ee $(T)

install:	install-raw

install-ee:	$(ALL)
	makenewer -r $(ALL) $(BINDIR)

install-raw:	$(T).ksh
	rm -f $(BINDIR)/$(T).ksh
	rm -f $(BINDIR)/$(T).$(OFF)
	makenewer -r -o rmsuf $(T).ksh $(BINDIR)
	
clean:
	rm -f $(T)


