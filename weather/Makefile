# MAKEFILE

T= weather

ALL= $(T)

SRCROOT= $(LOCAL)


BINDIR= $(SRCROOT)/bin


all:		$(ALL)

$(T):		$(T).ksh
	rm -f $(T)
	cp -p $(T).ksh $(T)

install:	$(T).ksh
	makenewer -r -o rmsuf $(T).ksh $(BINDIR)

clean:
	rm -f $(T)


