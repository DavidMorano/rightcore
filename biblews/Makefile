# MAKEFILE

T= biblews

ALL= $(T)

SRCROOT= $(LOCAL)


BINDIR= $(SRCROOT)/bin


all:		$(ALL)

$(T):		$(T).ksh
	rm -f $(T)
	ln -f $(T).ksh $(T)

install:	install-raw

install-ee:	$(ALL)
	makenewer -r $(ALL) $(BINDIR)

install-raw:	$(T).ksh
	rm -f $(T)
	ln $(T).ksh $(T)
	rm -f $(BINDIR)/$(T).ksh
	rm -f $(BINDIR)/$(T).$(OFF)
	makenewer -r $(T) $(BINDIR)

again:
	rm -f $(ALL) $(T).x $(T).lm

clean:		again
	rm -f *.ls *.ln *.i *.o *.so *.x *.lm



