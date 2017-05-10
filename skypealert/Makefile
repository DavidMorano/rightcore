# MAKEFILE


T= skypealert

ALL= $(T)

SRCROOT= $(LOCAL)


BINDIR= $(SRCROOT)/bin


all:		$(ALL)

$(T):		$(T).ksh
	rm -f $(T)
	ln -f $(T).ksh $(T)

install:	$(T)
	makenewer -r -o rmsuf $(ALL).ksh $(BINDIR)

clean:
	rm -f $(T)



