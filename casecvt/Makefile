# MAKEFILE

T= casecvt

ALL= $(T) caseupper caselower uppercase lowercase


SRCROOT= $(LOCAL)


BINDIR= $(SRCROOT)/bin


all:		$(ALL)

$(T):		$(T).ksh
	rm -f $(T)
	ln -f $(T).ksh $(T)

caselower:	$(T)
	rm -f $@
	ln -f $(T) $@

caseupper:	$(T)
	rm -f $@
	ln -f $(T) $@

lowercase:	$(T)
	rm -f $@
	ln -f $(T) $@

uppercase:	$(T)
	rm -f $@
	ln -f $(T) $@

install:	$(ALL)
	makenewer -r $(ALL) $(BINDIR)

clean:
	rm -f $(T) $(ALL)


