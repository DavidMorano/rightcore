# MAKEFILE (statmsg)

T= statmsg

ALL= $(T)

SRCROOT= $(LOCAL)


FBINDIR= $(SRCROOT)/fbin


all:		$(ALL)

install:	$(T)
	rm -f $(FBINDIR)/$(T).ksh
	rm -f $(FBINDIR)/$(T).$(OFF)
	makenewer -r $(T) $(FBINDIR)
	
clean:
	rm -f $(T)


