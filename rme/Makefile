# MAKEFILE

T= rme

ALL= $(T) $(T).ksh

SRCROOT= $(PCS)


BINDIR= $(SRCROOT)/bin


all:		$(ALL)

$(T):		$(T).ee
	cp -p $(T).ee $(T)

install:	$(ALL)
	makenewer -r $(ALL) $(BINDIR)
	-chmod g+s $(BINDIR)/$(T).ksh

clean:
	rm -f $(T)



