# MAKEFILE (gropeo)

T= gropeo

ALL= $(T)

SRCROOT= $(LOCAL)


BINDIR= $(SRCROOT)/bin


all:		$(ALL)

$(T):		$(T).ksh
	ln -f $(T).ksh $(T)

install:	install-raw

install-raw:	$(T).ksh
	rm -f $(BINDIR)/$(T).ksh
	rm -f $(BINDIR)/$(T).$(OFF)
	makenewer -r -o rmsuf $(T).ksh $(BINDIR)
	
clean:
	rm -f $(T)


