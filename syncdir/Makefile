# MAKEFILE

T= syncdir

ALL= $(T) $(T).ksh

SRCROOT= $(LOCAL)


BINDIR= $(SRCROOT)/bin


all:		$(ALL)

$(T):		$(T).ee
	cp -p $(T).ee $(T)

install:	$(ALL)
	bsdinstall $(ALL) $(BINDIR)

clean:
	rm -f $(T)



