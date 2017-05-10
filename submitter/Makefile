# MAKEFILE

T= submitter

ALL= $(T)

SRCROOT= $(HOME)


BINDIR= $(SRCROOT)/bin


all:		$(ALL)

$(T):		$(T).ksh
	cp -p $(T).ksh $(T)

install:	$(ALL)
	bsdinstall $(ALL) $(BINDIR)

clean:
	rm -f $(T)



