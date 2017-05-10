# MAKEFILE

T= groups

ALL= $(T)

SRCROOT= $(LOCAL)


BINDIR= $(SRCROOT)/bin


all:		$(ALL)

$(T):		$(T).ksh
	rm -f $(T)
	cp -p $(T).ksh $(T)

install:	$(ALL)
	makenewer -r $(ALL) $(BINDIR)

install-raw:	$(T).ksh
	rm -f $(BINDIR)/$(T).ksh
	rm -f $(BINDIR)/$(T).$(OFF)
	makenewer -r -o rmsuf $(T).ksh $(BINDIR)
	
clean:
	rm -f $(T)


