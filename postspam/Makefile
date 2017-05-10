# MAKEFILE

T= postspam

ALL= $(T)

SRCROOT= $(POSTFIX)


BINDIR= $(POSTFIX)/bin


all:		$(ALL)

$(T):		$(T).ksh
	rm -f $(T)
	ln -f $(T).ksh $(T)

install:	$(T).ksh
	makenewer -r -o rmsuf $(T).ksh $(BINDIR)

clean:
	rm -f $(T)


