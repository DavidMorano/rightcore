# MAKEFILE (notifier)

T= notifier

ALL= $(T)

SRCROOT= $(LOCAL)


SBINDIR= $(SRCROOT)/sbin


all:		$(ALL)

$(T):		$(T).ksh
	rm -f $(T)
	ln -f $(T).ksh $(T)

install:	$(T).ksh
	rm -f $(SBINDIR)/$(T).ksh
	rm -f $(SBINDIR)/$(T).$(OFF)
	makenewer -r -o rmsuf $(T).ksh $(SBINDIR)
	
clean:
	rm -f $(T)


