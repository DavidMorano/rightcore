# MAKEFILE

T= votd

SRCROOT= $(LOCAL)


BINDIR= $(SRCROOT)/bin


$(T):		$(T).ksh
	rm -f $(T)
	makecp $(T).ksh $(T)

install:	$(T).ksh
	rm -f $(BINDIR)/$(T).ksh
	rm -f $(BINDIR)/$(T).$(OFF)
	makenewer -r -o rmsuf $(T).ksh $(BINDIR)
	
clean:
	rm -f $(T)


