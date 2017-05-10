# MAKEFILE (fwm)

T= fwm

ALL= $(T)

SRCROOT= $(PCS)


# daemons of this nature go to '.../sbin/'

BINDIR= $(SRCROOT)/sbin


all:		$(ALL)

$(T):		$(T).ksh
	rm -f $(T)
	cp -p $(T).ksh $(T)

install:	$(ALL)
	makenewer -r $(ALL) $(BINDIR)

clean:
	rm -f $(T)


