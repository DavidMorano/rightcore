# MAKEFILE

T= pcpio

ALL= $(T)

SRCROOT= $(LOCAL)


BINDIR= $(SRCROOT)/bin


all:		$(ALL)

$(T):		$(T).ksh
	cp -p $(T).ksh $(T)

install:	$(ALL)
	makenewer -r $(ALL) $(BINDIR)

clean:
	rm -f $(T)



