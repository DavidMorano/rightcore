# MAKEFILE

T= linenumberd

ALL= $(T)

SRCROOT= $(LOCAL)


BINDIR= $(SRCROOT)/sbin


all:		$(ALL)

$(T):		$(T).ksh
	cp -p $(T).ksh $(T)

install:	$(ALL)
	bsdinstall $(ALL) $(BINDIR)

clean:
	rm -f $(T)



