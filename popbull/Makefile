# MAKEFILE

T= popbull

ALL= $(T)

SRCROOT= $(PCS)


BINDIR= $(SRCROOT)/sbin


all:		$(ALL)

$(T):		$(T).ksh
	cp -p $(T).ksh $(T)

install:	$(ALL)
	bsdinstall $(ALL) $(BINDIR)

clean:
	rm -f $(T)



