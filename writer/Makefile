# MAKEFILE

T= writer

ALL= $(T)

SRCROOT= $(LOCAL)


BINDIR= $(SRCROOT)/sbin


all:		$(ALL)

$(T):		$(T).ksh
	rm -f $(T)
	ln -f $(T).ksh $(T)

install:	$(ALL)
	makenewer -r $(ALL) $(BINDIR)

clean:
	rm -f $(T)



