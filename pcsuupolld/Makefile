# MAKEFILE

T= pcsuusched

ALL= $(T) $(T).sh

SRCROOT= $(PCS)


BINDIR= $(SRCROOT)/bin


all:		$(ALL)

$(T):		$(T).ee
	cp -p $(T).ee $(T)

install:	$(ALL)
	bsdinstall $(ALL) $(BINDIR)

clean:
	rm -f $(T)



