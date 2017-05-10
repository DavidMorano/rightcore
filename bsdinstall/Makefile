# MAKEFILE

T= bsdinstall

SRCROOT= $(LOCAL)


BINDIR= $(SRCROOT)/bin


all:		$(T)


$(T):		$(T).ksh
	rm -f $(T)
	cp -p $(T).ksh $(T)

install:	$(T)
	makenewer -r $(T) $(BINDIR)

clean:
	rm -f $(T)


