# MAKEFILE

T= xterminal

ALL= $(T)

SRCROOT= $(LOCAL)


BINDIR= $(LOCAL)/bin


all:		$(ALL)

$(T):		$(T).sh
	cp -p $(T).sh $(T)

install:	$(ALL)
	bsdinstall $(ALL) $(BINDIR)

clean:
	rm -f $(T)


