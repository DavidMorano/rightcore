# MAKEFILE (wsconskeys)

T= wsconskeys

ALL= $(T)

SRCROOT= $(EXTRA)


BINDIR= $(SRCROOT)/bin


all:		$(ALL)

$(T):		$(T).ksh
	ln -f $(T).ksh $(T)

install:	$(T)
	makenewer $(T) $(BINDIR)

clean:
	rm -f $(T)


