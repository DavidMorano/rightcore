# MAKEFILE

T= med

ALL= $(T)

SRCROOT= $(PCS)


BINDIR= $(SRCROOT)/sbin


all:		$(ALL)

$(T):		$(T).ksh
	rm -f $(T)
	cp -p $(T).ksh $(T)

install:	$(ALL)
	makenewer -r $(ALL) $(BINDIR)

clean:
	rm -f $(T)


