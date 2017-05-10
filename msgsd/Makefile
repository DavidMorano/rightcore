# MAKEFILE

T= msgsd

ALL= $(T)

#SRCROOT= $(LOCAL)


BINDIR= $(PCS)/sbin


all:		$(ALL)

$(T):		$(T).sh
	cp -p $(T).sh $(T)

install:	$(ALL)
	bsdinstall $(ALL) $(BINDIR)

clean:
	rm -f $(T)


