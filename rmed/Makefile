# MAKEFILE

T= rmed

ALL= $(T)

SRCROOT= $(PCS)


BINDIR= $(SRCROOT)/sbin


all:		$(ALL)

$(T):		$(T).sh
	cp -p $(T).sh $(T)

install:	$(ALL)
	bsdinstall $(ALL) $(BINDIR)

clean:
	rm -f $(T)



