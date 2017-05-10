# MAKEFILE

T= checkuucplogs

ALL= $(T) $(T).ksh

SRCROOT= $(NCMP)


BINDIR= $(SRCROOT)/bin


all:		$(ALL)

$(T):		$(T).ee
	cp -p $(T).ee $(T)

install:	$(ALL)
	makenewer -r $(ALL) $(BINDIR)

clean:
	rm -f $(T)



