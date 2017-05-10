# MAKEFILE

T= dq

ALL= $(T)

SRCROOT= $(HOME)


BINDIR= $(SRCROOT)/bin


all:		$(ALL)

$(T):		$(T).ksh
	rm -f $(T)
	ln -f $(T).ksh $(T)

install:	$(T).ksh
	rm -f $(BINDIR)/$(T).ksh
	rm -f $(BINDIR)/$(T).$(OFF)
	makenewer -r -o rmsif $(T).ksh $(BINDIR)
	
clean:
	rm -f $(T)


