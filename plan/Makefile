# MAKEFILE

T= plan

ALL= $(T)

SRCROOT= $(LOCAL)


BINDIR= $(SRCROOT)/bin
FUNDIR= $(SRCROOT)/fbin
MANDIR= $(SRCROOT)/man/man1


all:		$(ALL)

$(T):		$(T).ksh
	ln -f $(T).ksh $(T)

install:	install-raw install-fun

install-raw:	$(T).ksh
	rm -f $(BINDIR)/$(T).ksh
	rm -f $(BINDIR)/$(T).$(OFF)
	makenewer -r -o rmsuf $(T).ksh $(BINDIR)

install-fun:	$(T).fun
	-chmod ugo-x $(T).fun 2> /dev/null
	makenewer -r -o rmsuf $(T).fun $(FUNDIR)

install-man:	$(T).man
	makenewer -r -s man=1 $(T).man $(MANDIR)

clean:
	rm -f $(T)


