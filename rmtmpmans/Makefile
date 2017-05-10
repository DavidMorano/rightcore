# MAKEFILE (rmtmpmans)

T= rmtmpmans

ALL= $(T)

SRCROOT= $(LOCAL)


BINDIR= $(SRCROOT)/bin


all:		$(ALL)

$(T):		$(T).ksh
	rm -f $(T)
	makecp $(T).ksh $(T)

install:	$(T).ksh
	makenewer -r -o rmsuf $(T).ksh $(BINDIR)

clean:
	rm -f $(T)


