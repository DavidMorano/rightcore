# MAKEFILE 

T= rmailbridge

ALL= $(T) $(T).ksh

SRCROOT= $(PCS)


BINDIR= $(SRCROOT)/bin


all:		$(ALL)

$(T):		$(T).ee
	rm -f $(T)
	ln $(T).ee $(T)

install:	install-raw

install-ee:	$(ALL)
	makenewer -r $(ALL) $(BINDIR)

install-raw:	$(T).ksh
	rm -f $(T)
	ln $(T).ksh $(T)
	rm -f $(BINDIR)/$(T).ksh
	rm -f $(BINDIR)/$(T).$(OFF)
	makenewer -r $(T) $(BINDIR)
	
clean:
	rm -f $(T)



