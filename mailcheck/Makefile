# MAKEFILE (mailcheck)

T= mailcheck

ALL= $(T) $(T).ksh

SRCROOT= $(PCS)


BINDIR= $(SRCROOT)/sbin


all:		$(ALL)

$(T):		$(T).ee
	rm -f $(T)
	cp -p $(T).ee $(T)

install:	$(ALL)
	makenewer -r $(ALL) $(BINDIR)

clean:
	rm -f $(T)



