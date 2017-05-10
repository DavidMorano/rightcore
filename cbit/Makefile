# MAKEFILE

T= cbit

ALL= $(T)

SRCROOT= $(LOCAL)


BINDIR= $(SRCROOT)/bin


all:		$(ALL)

$(T):		$(T).ee
	rm -f $(T)
	ln -f $(T).ee $(T)

install:	$(T)
	rm -f $(BINDIR)/$(T).ksh
	rm -f $(BINDIR)/$(T).$(OFF)
	makenewer -o rmsuf -r $(T).ksh $(BINDIR)
	
clean:
	rm -f $(T)


