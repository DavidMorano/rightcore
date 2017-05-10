# MAKEFILE

T= usend

ALL= $(T)

SRCROOT= $(PCS)


BINDIR= $(SRCROOT)/bin


all:		$(ALL)

$(T):		$(T).sh
	cp -p $(T).sh $(T)

install:	$(ALL)
	bsdinstall $(ALL) $(BINDIR)

clean:
	rm -f $(T)

control:
	( uname -n ; date '+%m/%d/%y %T' ) > Control


