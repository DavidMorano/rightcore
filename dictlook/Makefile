# MAKEFILE

T= dictlook

ALL= $(T)

SRCROOT= $(LOCAL)


BINDIR= $(LOCAL)/bin


all:		$(ALL)

$(T):		$(T).ksh
	rm -f $(T)
	ln -f $(T).ksh $(T)

install:	$(ALL)
	makenewer -r $(ALL) $(BINDIR)

clean:
	rm -f $(T)



