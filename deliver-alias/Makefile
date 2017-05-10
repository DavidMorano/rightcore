# MAKEFILE

T= deliver-alias

ALL= $(T)

SRCROOT= $(LOCAL)


BINDIR= $(SRCROOT)/bin
SBINDIR= $(POSTFIX)/sbin


all:		$(ALL)

$(T):		$(T).ksh
	ln -f $(T).ksh $(T)

install:	$(ALL)
	makenewer -r $(ALL) $(SBINDIR)

clean:
	rm -f $(T)



