# MAKEFILE

T= movemail

ALL= $(T)

SRCROOT= $(PCS)


BINDIR= $(SRCROOT)/bin


all:		$(ALL)

$(T):		$(T).ee
	cp -p $(T).ee $(T)

install:	$(ALL)
	bsdinstall $(T).ksh $(ALL) $(BINDIR)

clean:
	rm -f $(T)



