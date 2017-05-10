# MAKEFILE (z)

T= z

ALL= $(T) $(T).ksh

SRCROOT= $(PCS)


BINDIR= $(SRCROOT)/bin


all:		$(ALL)

$(T):		$(T).ee
	rm -f $(T)
	cp -p $(T).ee $(T)

install:	install-raw

install-ee:	$(ALL)
	makenewer -r $(ALL) $(BINDIR)

install-raw:	$(T).ksh
	makenewer -r -o rmsuf $(T).ksh $(BINDIR)
	
clean:
	rm -f $(T)


