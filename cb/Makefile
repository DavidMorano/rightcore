# MAKEFILE

T= cb

ALL= $(T)


SRCROOT= $(LOCAL)

BINDIR= $(SRCROOT)/bin


all:		$(ALL)

$(T):		$(T).ksh
	rm -f $(T)
	cp -p $(T).ksh $(T)

install:	$(T).ksh
	rm -f $(BINDIR)/$(T).ksh
	rm -f $(BINDIR)/$(T).$(OFF)
	makenewer -r -o rmsuf $(T).ksh $(BINDIR)

clean:
	rm -f $(T)


