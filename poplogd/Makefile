# MAKEFILE

T= poplogd

ALL= $(T) $(T).ksh

SRCROOT= $(LOCAL)


BINDIR= $(SRCROOT)/bin


all:		$(ALL)

install:	$(ALL) $(T).ee
	rm -f $(T)
	cp -p $(T).ee $(T)
	makenewer -r $(ALL) $(BINDIR)

install-raw:	$(T)
	rm -f $(T)
	cp -p $(T).ksh $(T)
	makenewer -r $(T) $(BINDIR)

clean:
	rm -f $(T)



