# MAKEFILE

T= pathcomponent

ALL= $(T)

SRCROOT= $(LOCAL)


BINDIR= $(LOCAL)/bin


all:		$(ALL)

$(T):		$(T).ksh
	cp -p $(T).ksh $(T)

install:	$(ALL)
	bsdinstall $(ALL) $(BINDIR)

clean:
	rm -f $(T)



