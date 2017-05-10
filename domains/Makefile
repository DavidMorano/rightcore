# MAKEFILE (domains)

T= domains

ALL= $(T)

SRCROOT= $(LOCAL)


BINDIR= $(SRCROOT)/bin


all:		$(ALL)

$(T):		$(T).ee
	rm -f $(T)
	cp -p $(T).ee $(T)

install:	install-raw

install-raw:	$(T).ksh
	rm -f $(BINDIR)/$(T).ksh
	rm -f $(BINDIR)/$(T).$(OFF)
	makenewer -o rmsuf -r $(T).ksh $(BINDIR)
	
clean:
	rm -f $(T)


